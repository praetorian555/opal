#pragma once

#include "opal/threading/atomic.h"

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/expected.h"
#include "opal/container/shared-ptr.h"
#include "opal/error-codes.h"
#include "opal/threading/cpu-pause.h"
#include "opal/type-traits.h"

namespace Opal
{
namespace Impl
{
/**
 * Lock-free single-producer single-consumer bounded queue.
 * Capacity is rounded up to the next power of two.
 * @tparam T Type of data stored in the queue. Must be default constructable.
 * @tparam UseSignaling If true, uses Atomic::Wait/Notify to block when the queue is full/empty.
 *         If false, busy-waits using a CPU pause instruction. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
    requires Opal::DefaultConstructable<T>
class QueueSPSC
{
public:
    static_assert(Atomic<size_t>::k_is_always_lock_free, "Type size_t is not atomic on this platform!");

    explicit QueueSPSC(size_t capacity, AllocatorBase* allocator = nullptr)
        : m_capacity(GetNextPowerOf2(capacity)), m_data(m_capacity, allocator)
    {
        OPAL_VERIFY(capacity > 0, "QueueSPSC capacity must be greater than zero");
        OPAL_VERIFY(m_data.GetAllocator()->IsThreadSafe(), "QueueSPSC allocator must be thread safe");
        m_write_idx.Store<MemoryOrder::Relaxed>(0);
        m_read_idx.Store<MemoryOrder::Relaxed>(0);
    }

    void Push(const T& item)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Relaxed>();
        // Acquire: the slot about to be written is one the consumer may still have been reading on the previous lap, and its
        // release of m_read_idx is what orders that read before this write.
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();

        while (write_idx - read_idx == m_capacity)
        {
            WaitForPop(read_idx);
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = item;  // Do copy

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        SignalPushed();
    }

    bool TryPush(const T& item)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Relaxed>();
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();

        if (write_idx - read_idx == m_capacity)
        {
            return false;
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = item;  // Do copy

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        SignalPushed();
        return true;
    }

    void Push(T&& item)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Relaxed>();
        // Acquire: the slot about to be written is one the consumer may still have been reading on the previous lap, and its
        // release of m_read_idx is what orders that read before this write.
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();

        while (write_idx - read_idx == m_capacity)
        {
            WaitForPop(read_idx);
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = std::move(item);  // Do move

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        SignalPushed();
    }

    template <typename... Args>
    void PushWithEmplace(const Args&... args)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Relaxed>();
        // Acquire: the slot about to be written is one the consumer may still have been reading on the previous lap, and its
        // release of m_read_idx is what orders that read before this write.
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();

        while (write_idx - read_idx == m_capacity)
        {
            WaitForPop(read_idx);
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        EmplaceAt(m_data[bound_write_idx], args...);

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        SignalPushed();
    }

    template <typename... Args>
    bool TryPushWithEmplace(const Args&... args)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Relaxed>();
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();

        if (write_idx - read_idx == m_capacity)
        {
            return false;
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        EmplaceAt(m_data[bound_write_idx], args...);

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        SignalPushed();
        return true;
    }

    T Pop()
    {
        T result;
        for (u32 round = 0; !TryPop(result); round++)
        {
            WaitForPush(round);
        }
        return result;
    }

    /**
     * Pop an item, blocking while the queue is empty and open.
     * @param result Receives the item.
     * @return True if an item was popped, false if the queue is empty and closed. Items pushed before Close are always popped
     *         before this reports the queue closed.
     */
    bool PopUnlessClosed(T& result)
    {
        for (u32 round = 0;; round++)
        {
            if (TryPop(result))
            {
                return true;
            }
            if (IsClosed())
            {
                return TryPop(result);
            }
            WaitForPush(round);
        }
    }

    bool TryPop(T& result)
    {
        size_t read_idx = m_read_idx.Load<MemoryOrder::Relaxed>();
        size_t write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
        if (write_idx - read_idx == 0)
        {
            return false;
        }
        size_t bound_read_idx = read_idx & (m_capacity - 1);
        result = std::move(m_data[bound_read_idx]);
        m_read_idx.Store<MemoryOrder::Release>(read_idx + 1);
        SignalPopped();
        return true;
    }

    /** Mark the queue closed and wake a consumer blocked in PopUnlessClosed. Items already in it can still be popped. */
    void Close()
    {
        m_is_closed.Store<MemoryOrder::Release>(true);
        if constexpr (UseSignaling)
        {
            m_push_count.FetchAdd<MemoryOrder::Release>(1);
            m_push_count.NotifyAll();
        }
    }

    /** @return True once Close has been called. */
    [[nodiscard]] bool IsClosed() const { return m_is_closed.Load<MemoryOrder::Acquire>(); }

private:
    // With signaling, how many times a waiter polls, pausing twice as long each time, before it goes to sleep.
    static constexpr u32 k_spin_rounds_before_sleep = 8;

    // A waiter registers in its flag, fences, then re-checks before it sleeps; the other side publishes, fences, then reads the
    // flag. The two fences make sure one of them sees the other, so a wakeup is only paid for when someone sleeps and never lost.
    // The fence is asymmetric: the side about to sleep pays for it, and every push and pop gets away with the light half.

    /** Wake the consumer if it is asleep waiting for an item. Called right after an item was published. */
    void SignalPushed()
    {
        if constexpr (UseSignaling)
        {
            AsymmetricThreadFenceLight();
            if (m_consumer_waiting.Load<MemoryOrder::Relaxed>() != 0)
            {
                m_push_count.FetchAdd<MemoryOrder::Release>(1);
                m_push_count.NotifyOne();
            }
        }
    }

    /** Wake the producer if it is asleep waiting for room. Called right after an item was popped. */
    void SignalPopped()
    {
        if constexpr (UseSignaling)
        {
            AsymmetricThreadFenceLight();
            if (m_producer_waiting.Load<MemoryOrder::Relaxed>() != 0)
            {
                m_read_idx.NotifyOne();
            }
        }
    }

    /**
     * Block until something may have been pushed or the queue closed. Returns at once if the queue is no longer empty.
     * @param round How many times the caller has already waited for this item. With signaling, the first few rounds spin.
     */
    void WaitForPush(u32 round)
    {
        if constexpr (UseSignaling)
        {
            if (round < k_spin_rounds_before_sleep)
            {
                for (u32 i = 0; i < (1u << round); i++)
                {
                    CpuPause();
                }
                return;
            }
            m_consumer_waiting.Store<MemoryOrder::Relaxed>(1);
            AsymmetricThreadFenceHeavy();
            const u32 push_count = m_push_count.Load<MemoryOrder::Acquire>();
            if (m_write_idx.Load<MemoryOrder::Acquire>() == m_read_idx.Load<MemoryOrder::Relaxed>() && !IsClosed())
            {
                m_push_count.Wait<MemoryOrder::Acquire>(push_count);
            }
            m_consumer_waiting.Store<MemoryOrder::Relaxed>(0);
        }
        else
        {
            CpuPause();
        }
    }

    /** Block until the consumer may have popped since read_idx was read. */
    void WaitForPop(size_t read_idx)
    {
        if constexpr (UseSignaling)
        {
            m_producer_waiting.Store<MemoryOrder::Relaxed>(1);
            AsymmetricThreadFenceHeavy();
            if (m_read_idx.Load<MemoryOrder::Relaxed>() == read_idx)
            {
                m_read_idx.Wait<MemoryOrder::Acquire>(read_idx);
            }
            m_producer_waiting.Store<MemoryOrder::Relaxed>(0);
        }
        else
        {
            CpuPause();
        }
    }

    /** Replace the value in a slot with one constructed from args. */
    template <typename... Args>
    static void EmplaceAt(T& slot, const Args&... args)
    {
        if constexpr (std::is_nothrow_constructible_v<T, const Args&...>)
        {
            slot.~T();
            new (&slot) T(args...);
        }
        else
        {
            slot = T(args...);
        }
    }

    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_write_idx;
    Atomic<u32> m_push_count = 0;
    Atomic<u32> m_consumer_waiting = 0;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_read_idx;
    Atomic<u32> m_producer_waiting = 0;
    OPAL_END_DISABLE_WARNINGS

    size_t m_capacity = 0;
    DynamicArray<T> m_data = nullptr;
    Atomic<bool> m_is_closed = false;
};
}  // namespace Impl

/**
 * Producer end of a single-producer single-consumer communication channel.
 * It can't be copied, only moved.
 * @tparam T Type of data that is being sent.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct TransmitterSPSC
{
    TransmitterSPSC() = default;
    explicit TransmitterSPSC(SharedPtr<Impl::QueueSPSC<T, UseSignaling>>&& q) : m_queue(std::move(q)) {}
    ~TransmitterSPSC() = default;

    TransmitterSPSC(const TransmitterSPSC& q) = delete;
    TransmitterSPSC& operator=(const TransmitterSPSC& q) = delete;

    TransmitterSPSC(TransmitterSPSC&& other) noexcept : m_queue(std::move(other.m_queue)) {}

    TransmitterSPSC& operator=(TransmitterSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        return *this;
    }

    bool operator==(const TransmitterSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(std::move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

    /**
     * Marks the channel as closed and wakes any receiver blocked in Receive(). Items sent before this are still received;
     * once they are drained, Receive() and TryReceive() return ErrorCode::ChannelClosed without blocking.
     */
    void Close() { m_queue->Close(); }

private:
    SharedPtr<Impl::QueueSPSC<T, UseSignaling>> m_queue;
};

/**
 * Consumer end of a single-producer single-consumer communication channel.
 * It can't be copied, only moved.
 * @tparam T Type of data that is being received.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ReceiverSPSC
{
    ReceiverSPSC() = default;
    explicit ReceiverSPSC(SharedPtr<Impl::QueueSPSC<T, UseSignaling>>&& q) : m_queue(std::move(q)) {}
    ~ReceiverSPSC() = default;

    ReceiverSPSC(const ReceiverSPSC& q) = delete;
    ReceiverSPSC& operator=(const ReceiverSPSC& q) = delete;

    ReceiverSPSC(ReceiverSPSC&& other) noexcept : m_queue(std::move(other.m_queue)) {}
    ReceiverSPSC& operator=(ReceiverSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        return *this;
    }

    /**
     * Blocks until an item is available or the channel is closed.
     * Items already in the queue are drained first. Only after the queue is empty
     * and the channel has been closed does this return ErrorCode::ChannelClosed.
     */
    Expected<T, ErrorCode> Receive()
    {
        T result;
        if (m_queue->PopUnlessClosed(result))
        {
            return Expected<T, ErrorCode>(std::move(result));
        }
        return Expected<T, ErrorCode>(ErrorCode::ChannelClosed);
    }

    /**
     * Non-blocking receive. Items already in the queue are drained first.
     * Only after the queue is empty and the channel has been closed does this
     * return ErrorCode::ChannelClosed.
     */
    ErrorCode TryReceive(T& result)
    {
        if (m_queue->TryPop(result))
        {
            return ErrorCode::Success;
        }
        if (m_queue->IsClosed())
        {
            return m_queue->TryPop(result) ? ErrorCode::Success : ErrorCode::ChannelClosed;
        }
        return ErrorCode::ChannelEmpty;
    }

    bool operator==(const ReceiverSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueSPSC<T, UseSignaling>> m_queue;
};

/**
 * One-way, thread-safe communication channel with one producer and one consumer.
 * Use the transmitter field to send data and the receiver field to receive it.
 * The transmitter and receiver can be moved to separate threads.
 * @tparam T Type of data to be sent over the channel.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ChannelSPSC
{
    TransmitterSPSC<T, UseSignaling> transmitter;
    ReceiverSPSC<T, UseSignaling> receiver;

    ChannelSPSC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        SharedPtr<Impl::QueueSPSC<T, UseSignaling>> q(allocator, capacity, allocator);
        transmitter = TransmitterSPSC<T, UseSignaling>(q.Clone());
        receiver = ReceiverSPSC<T, UseSignaling>(std::move(q));
    }
};

}  // namespace Opal

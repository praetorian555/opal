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
            if constexpr (UseSignaling)
            {
                m_read_idx.Wait<MemoryOrder::Acquire>(read_idx);
            }
            else
            {
                CpuPause();
            }
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = item;  // Do copy

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        if constexpr (UseSignaling)
        {
            m_write_idx.NotifyOne();
        }
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
        if constexpr (UseSignaling)
        {
            m_write_idx.NotifyOne();
        }
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
            if constexpr (UseSignaling)
            {
                m_read_idx.Wait<MemoryOrder::Acquire>(read_idx);
            }
            else
            {
                CpuPause();
            }
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = std::move(item);  // Do move

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        if constexpr (UseSignaling)
        {
            m_write_idx.NotifyOne();
        }
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
            if constexpr (UseSignaling)
            {
                m_read_idx.Wait<MemoryOrder::Acquire>(read_idx);
            }
            else
            {
                CpuPause();
            }
            read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        new (&m_data[bound_write_idx]) T(args...);

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        if constexpr (UseSignaling)
        {
            m_write_idx.NotifyOne();
        }
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
        new (&m_data[bound_write_idx]) T(args...);

        m_write_idx.Store<MemoryOrder::Release>(write_idx + 1);
        if constexpr (UseSignaling)
        {
            m_write_idx.NotifyOne();
        }
        return true;
    }

    T Pop()
    {
        size_t read_idx = m_read_idx.Load<MemoryOrder::Relaxed>();
        size_t write_idx = m_write_idx.Load<MemoryOrder::Acquire>();

        while (write_idx - read_idx == 0)
        {
            if constexpr (UseSignaling)
            {
                m_write_idx.Wait<MemoryOrder::Acquire>(write_idx);
            }
            else
            {
                CpuPause();
            }
            write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
        }

        size_t bound_read_idx = read_idx & (m_capacity - 1);
        T result = std::move(m_data[bound_read_idx]);
        m_read_idx.Store<MemoryOrder::Release>(read_idx + 1);
        if constexpr (UseSignaling)
        {
            m_read_idx.NotifyOne();
        }
        return result;
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
        if constexpr (UseSignaling)
        {
            m_read_idx.NotifyOne();
        }
        return true;
    }

private:
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_write_idx;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_read_idx;
    OPAL_END_DISABLE_WARNINGS

    size_t m_capacity = 0;
    DynamicArray<T> m_data = nullptr;
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
    TransmitterSPSC(SharedPtr<Impl::QueueSPSC<T, UseSignaling>>&& q, Ref<Atomic<bool>> is_closed)
        : m_queue(std::move(q)), m_is_closed(std::move(is_closed))
    {
    }
    ~TransmitterSPSC() = default;

    TransmitterSPSC(const TransmitterSPSC& q) = delete;
    TransmitterSPSC& operator=(const TransmitterSPSC& q) = delete;

    TransmitterSPSC(TransmitterSPSC&& other) noexcept
        : m_queue(std::move(other.m_queue)), m_is_closed(std::move(other.m_is_closed))
    {
    }

    TransmitterSPSC& operator=(TransmitterSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        m_is_closed = std::move(other.m_is_closed);
        return *this;
    }

    bool operator==(const TransmitterSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(std::move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

    /**
     * Marks the channel as closed. After this, Receive() and TryReceive() on the
     * corresponding receiver will return ErrorCode::ChannelClosed without blocking.
     */
    void Close() const { m_is_closed->Store<MemoryOrder::Release>(true); }

private:
    SharedPtr<Impl::QueueSPSC<T, UseSignaling>> m_queue;
    Ref<Atomic<bool>> m_is_closed;
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
    ReceiverSPSC(SharedPtr<Impl::QueueSPSC<T, UseSignaling>>&& q, Ref<Atomic<bool>> is_closed)
        : m_queue(std::move(q)), m_is_closed(std::move(is_closed))
    {
    }
    ~ReceiverSPSC() = default;

    ReceiverSPSC(const ReceiverSPSC& q) = delete;
    ReceiverSPSC& operator=(const ReceiverSPSC& q) = delete;

    ReceiverSPSC(ReceiverSPSC&& other) noexcept
        : m_queue(std::move(other.m_queue)), m_is_closed(std::move(other.m_is_closed))
    {
    }
    ReceiverSPSC& operator=(ReceiverSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        m_is_closed = std::move(other.m_is_closed);
        return *this;
    }

    /**
     * Blocks until an item is available and returns it.
     * Items already in the queue are drained first. Only after the queue is empty
     * and the channel has been closed does this return ErrorCode::ChannelClosed.
     */
    Expected<T, ErrorCode> Receive()
    {
        T result;
        if (m_queue->TryPop(result))
        {
            return Expected<T, ErrorCode>(std::move(result));
        }
        if (m_is_closed->Load<MemoryOrder::Acquire>())
        {
            return Expected<T, ErrorCode>(ErrorCode::ChannelClosed);
        }
        return Expected<T, ErrorCode>(m_queue->Pop());
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
        if (m_is_closed->Load<MemoryOrder::Acquire>())
        {
            return ErrorCode::ChannelClosed;
        }
        return ErrorCode::ChannelEmpty;
    }

    bool operator==(const ReceiverSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueSPSC<T, UseSignaling>> m_queue;
    Ref<Atomic<bool>> m_is_closed;
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
    Atomic<bool> m_is_closed = false;

    ChannelSPSC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        SharedPtr<Impl::QueueSPSC<T, UseSignaling>> q(allocator, capacity, allocator);
        transmitter = TransmitterSPSC<T, UseSignaling>(q.Clone(), m_is_closed);
        receiver = ReceiverSPSC<T, UseSignaling>(std::move(q), m_is_closed);
    }
};

}  // namespace Opal

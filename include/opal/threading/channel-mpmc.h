#pragma once

#include "opal/threading/atomic.h"

#include "opal/allocator.h"
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

template <typename T>
struct QueueMPMCSlot
{
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> turn = 0;
    alignas(OPAL_CACHE_LINE_SIZE) T data;
    OPAL_END_DISABLE_WARNINGS
};

/**
 * Lock-free multiple-producer multiple-consumer bounded queue.
 * @tparam T Type of data stored in the queue. Must be default constructable, and either copy assignable or clonable.
 * @tparam UseSignaling If true, uses Atomic::Wait/Notify to block when waiting for a slot's turn.
 *         If false, busy-waits using a CPU pause instruction. Defaults to false.
 */
template <typename T, bool UseSignaling = false>
    requires Opal::DefaultConstructable<T> && (Opal::CopyAssignable<T> || Opal::Clonable<T>)
class QueueMPMC
{
public:
    QueueMPMC(size_t capacity, AllocatorBase* allocator = nullptr) : m_data(capacity, allocator), m_capacity(capacity) {}

    void Push(const T& data)
    {
        size_t write_idx = m_write_idx.FetchAdd<MemoryOrder::Relaxed>(1);
        size_t slot_idx = write_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        while (2 * turn != current_turn)
        {
            if constexpr (UseSignaling)
            {
                slot.turn.template Wait<MemoryOrder::Relaxed>(current_turn);
            }
            else
            {
                CpuPause();
            }
            current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        }
        if constexpr (Opal::CopyAssignable<T>)
        {
            slot.data = data;
        }
        else
        {
            slot.data = data.Clone();
        }
        slot.turn.template Store<MemoryOrder::Release>(2 * turn + 1);
        if constexpr (UseSignaling)
        {
            slot.turn.NotifyAll();
        }
    }

    bool TryPush(const T& data)
    {
        size_t write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
        while (true)
        {
            size_t slot_idx = write_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn != slot.turn.template Load<MemoryOrder::Acquire>())
            {
                // Slot is not empty, we are most likely full
                const size_t prev_write_idx = write_idx;
                write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
                if (write_idx == prev_write_idx)
                {
                    // Nothing changed since the time we loaded the write_idx
                    // report that we are full
                    return false;
                }
            }
            else
            {
                if (m_write_idx.CompareExchangeStrong(write_idx, write_idx + 1))
                {
                    // If write_idx didn't change since last check, consume it and increment the
                    // atomic
                    if constexpr (Opal::CopyAssignable<T>)
                    {
                        slot.data = data;
                    }
                    else
                    {
                        slot.data = data.Clone();
                    }
                    slot.turn.template Store<MemoryOrder::Release>(2 * turn + 1);
                    if constexpr (UseSignaling)
                    {
                        slot.turn.NotifyAll();
                    }
                    return true;
                }
            }
        }
    }

    T Pop()
    {
        size_t read_idx = m_read_idx.FetchAdd<MemoryOrder::Relaxed>(1);
        size_t slot_idx = read_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        while (2 * turn + 1 != current_turn)
        {
            if constexpr (UseSignaling)
            {
                slot.turn.template Wait<MemoryOrder::Relaxed>(current_turn);
            }
            else
            {
                CpuPause();
            }
            current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        }
        T result = std::move(slot.data);
        slot.turn.template Store<MemoryOrder::Release>(2 * turn + 2);
        if constexpr (UseSignaling)
        {
            slot.turn.NotifyAll();
        }
        return result;
    }

    bool TryPop(T& result)
    {
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        while (true)
        {
            size_t slot_idx = read_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn + 1 != slot.turn.template Load<MemoryOrder::Acquire>())
            {
                const size_t prev_read_idx = read_idx;
                read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
                if (prev_read_idx == read_idx)
                {
                    return false;
                }
            }
            else
            {
                if (m_read_idx.CompareExchangeStrong(read_idx, read_idx + 1))
                {
                    result = std::move(slot.data);
                    slot.turn.template Store<MemoryOrder::Release>(2 * turn + 2);
                    if constexpr (UseSignaling)
                    {
                        slot.turn.NotifyAll();
                    }
                    return true;
                }
            }
        }
    }

    /** @return How many items have ever been pushed. Only grows, so a caller can compare two readings of it. */
    [[nodiscard]] size_t GetPushCount() const { return m_write_idx.Load<MemoryOrder::Acquire>(); }

private:
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_write_idx = 0;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_read_idx = 0;
    OPAL_END_DISABLE_WARNINGS
    DynamicArray<QueueMPMCSlot<T>> m_data;
    size_t m_capacity = 0;
};

}  // namespace Impl

/**
 * Producer end of a multiple-producer multiple-consumer communication channel.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being sent.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to false.
 */
template <typename T, bool UseSignaling = false>
struct TransmitterMPMC
{
    TransmitterMPMC() = default;
    TransmitterMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling>>&& q, Ref<Atomic<bool>> is_closed)
        : m_queue(std::move(q)), m_is_closed(std::move(is_closed))
    {
    }
    ~TransmitterMPMC() = default;

    TransmitterMPMC(const TransmitterMPMC& q) = delete;
    TransmitterMPMC& operator=(const TransmitterMPMC& q) = delete;

    TransmitterMPMC(TransmitterMPMC&& other) noexcept
        : m_queue(std::move(other.m_queue)), m_is_closed(std::move(other.m_is_closed))
    {
    }
    TransmitterMPMC& operator=(TransmitterMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        m_is_closed = std::move(other.m_is_closed);
        return *this;
    }

    TransmitterMPMC Clone() const { return TransmitterMPMC{m_queue.Clone(), m_is_closed.Clone()}; }

    bool operator==(const TransmitterMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(std::move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

    /** @return How many items have ever been sent through this channel, by any transmitter sharing it. */
    [[nodiscard]] size_t GetSendCount() const { return m_queue->GetPushCount(); }

    /**
     * Marks the channel as closed. After this, Receive() and TryReceive() on the
     * corresponding receiver will return ErrorCode::ChannelClosed without blocking.
     */
    void Close() const { m_is_closed->Store<MemoryOrder::Release>(true); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling>> m_queue;
    Ref<Atomic<bool>> m_is_closed;
};

/**
 * Consumer end of a multiple-producer multiple-consumer communication channel.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being received.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to false.
 */
template <typename T, bool UseSignaling = false>
struct ReceiverMPMC
{
    ReceiverMPMC() = default;
    ReceiverMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling>>&& q, Ref<Atomic<bool>> is_closed)
        : m_queue(std::move(q)), m_is_closed(std::move(is_closed))
    {
    }
    ~ReceiverMPMC() = default;

    ReceiverMPMC(const ReceiverMPMC& q) = delete;
    ReceiverMPMC& operator=(const ReceiverMPMC& q) = delete;

    ReceiverMPMC(ReceiverMPMC&& other) noexcept
        : m_queue(std::move(other.m_queue)), m_is_closed(std::move(other.m_is_closed))
    {
    }
    ReceiverMPMC& operator=(ReceiverMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        m_is_closed = std::move(other.m_is_closed);
        return *this;
    }

    ReceiverMPMC Clone() const { return ReceiverMPMC{m_queue.Clone(), m_is_closed.Clone()}; }

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

    bool operator==(const ReceiverMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling>> m_queue;
    Ref<Atomic<bool>> m_is_closed;
};

/**
 * One-way, thread-safe communication channel with multiple producers and multiple consumers.
 * Use the transmitter field to send data and the receiver field to receive it.
 * @tparam T Type of data to be sent over the channel.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to false.
 */
template <typename T, bool UseSignaling = false>
struct ChannelMPMC
{
    TransmitterMPMC<T, UseSignaling> transmitter;
    ReceiverMPMC<T, UseSignaling> receiver;
    Atomic<bool> m_is_closed = false;

    explicit ChannelMPMC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        capacity = GetNextPowerOf2(capacity);
        SharedPtr<Impl::QueueMPMC<T, UseSignaling>> q(allocator, capacity, allocator);
        transmitter = TransmitterMPMC<T, UseSignaling>(q.Clone(), m_is_closed);
        receiver = ReceiverMPMC<T, UseSignaling>(std::move(q), m_is_closed);
    }
};

}  // namespace Opal

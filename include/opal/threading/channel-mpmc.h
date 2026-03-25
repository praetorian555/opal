#pragma once

#include <atomic>

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
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<size_t> turn = 0;
    alignas(OPAL_CACHE_LINE_SIZE) T data;
    OPAL_END_DISABLE_WARNINGS
};

/**
 * Lock-free multiple-producer multiple-consumer bounded queue.
 * @tparam T Type of data stored in the queue. Must be default constructable.
 * @tparam UseSignaling If true, uses std::atomic::wait/notify to block when waiting for a slot's turn.
 *         If false, busy-waits using a CPU pause instruction. Defaults to false.
 */
template <typename T, bool UseSignaling = false>
    requires Opal::DefaultConstructable<T>
class QueueMPMC
{
public:
    QueueMPMC(size_t capacity, AllocatorBase* allocator = nullptr) : m_data(capacity, allocator), m_capacity(capacity) {}

    void Push(const T& data)
    {
        size_t write_idx = m_write_idx.fetch_add(1, std::memory_order_relaxed);
        size_t slot_idx = write_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.load(std::memory_order_acquire);
        while (2 * turn != current_turn)
        {
            if constexpr (UseSignaling)
            {
                slot.turn.wait(current_turn, std::memory_order_relaxed);
            }
            else
            {
                CpuPause();
            }
            current_turn = slot.turn.load(std::memory_order_acquire);
        }
        if constexpr (Opal::CopyAssignable<T>)
        {
            slot.data = data;
        }
        else if constexpr (Opal::Clonable<T>)
        {
            slot.data = data.Clone();
        }
        else
        {
            throw Exception("Data type can't be copied!");
        }
        slot.turn.store(2 * turn + 1, std::memory_order_release);
        if constexpr (UseSignaling)
        {
            slot.turn.notify_all();
        }
    }

    bool TryPush(const T& data)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_acquire);
        while (true)
        {
            size_t slot_idx = write_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn != slot.turn.load(std::memory_order_acquire))
            {
                // Slot is not empty, we are most likely full
                const size_t prev_write_idx = write_idx;
                write_idx = m_write_idx.load(std::memory_order_acquire);
                if (write_idx == prev_write_idx)
                {
                    // Nothing changed since the time we loaded the write_idx
                    // report that we are full
                    return false;
                }
            }
            else
            {
                if (m_write_idx.compare_exchange_strong(write_idx, write_idx + 1))
                {
                    // If write_idx didn't change since last check, consume it and increment the
                    // atomic
                    if constexpr (Opal::CopyAssignable<T>)
                    {
                        slot.data = data;
                    }
                    else if constexpr (Opal::Clonable<T>)
                    {
                        slot.data = data.Clone();
                    }
                    else
                    {
                        throw Exception("Data type can't be copied!");
                    }
                    slot.turn.store(2 * turn + 1, std::memory_order_release);
                    if constexpr (UseSignaling)
                    {
                        slot.turn.notify_all();
                    }
                    return true;
                }
            }
        }
    }

    T Pop()
    {
        size_t read_idx = m_read_idx.fetch_add(1, std::memory_order_relaxed);
        size_t slot_idx = read_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.load(std::memory_order_acquire);
        while (2 * turn + 1 != current_turn)
        {
            if constexpr (UseSignaling)
            {
                slot.turn.wait(current_turn, std::memory_order_relaxed);
            }
            else
            {
                CpuPause();
            }
            current_turn = slot.turn.load(std::memory_order_acquire);
        }
        T result = std::move(slot.data);
        slot.turn.store(2 * turn + 2, std::memory_order_release);
        if constexpr (UseSignaling)
        {
            slot.turn.notify_all();
        }
        return result;
    }

    bool TryPop(T& result)
    {
        size_t read_idx = m_read_idx.load(std::memory_order_acquire);
        while (true)
        {
            size_t slot_idx = read_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn + 1 != slot.turn.load(std::memory_order_acquire))
            {
                const size_t prev_read_idx = read_idx;
                read_idx = m_read_idx.load(std::memory_order_acquire);
                if (prev_read_idx == read_idx)
                {
                    return false;
                }
            }
            else
            {
                if (m_read_idx.compare_exchange_strong(read_idx, read_idx + 1))
                {
                    result = std::move(slot.data);
                    slot.turn.store(2 * turn + 2, std::memory_order_release);
                    if constexpr (UseSignaling)
                    {
                        slot.turn.notify_all();
                    }
                    return true;
                }
            }
        }
    }

private:
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<size_t> m_write_idx = 0;
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<size_t> m_read_idx = 0;
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
    TransmitterMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling>>&& q, Ref<std::atomic<bool>> is_closed)
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

    /**
     * Marks the channel as closed. After this, Receive() and TryReceive() on the
     * corresponding receiver will return ErrorCode::ChannelClosed without blocking.
     */
    void Close() const { m_is_closed->store(true, std::memory_order_release); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling>> m_queue;
    Ref<std::atomic<bool>> m_is_closed;
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
    ReceiverMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling>>&& q, Ref<std::atomic<bool>> is_closed)
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
        if (m_is_closed->load(std::memory_order_acquire))
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
        if (m_is_closed->load(std::memory_order_acquire))
        {
            return ErrorCode::ChannelClosed;
        }
        return ErrorCode::ChannelEmpty;
    }

    bool operator==(const ReceiverMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling>> m_queue;
    Ref<std::atomic<bool>> m_is_closed;
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
    std::atomic<bool> m_is_closed = false;

    explicit ChannelMPMC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        capacity = GetNextPowerOf2(capacity);
        SharedPtr<Impl::QueueMPMC<T, UseSignaling>> q(allocator, capacity, allocator);
        transmitter = TransmitterMPMC<T, UseSignaling>(q.Clone(), m_is_closed);
        receiver = ReceiverMPMC<T, UseSignaling>(std::move(q), m_is_closed);
    }
};

}  // namespace Opal

#pragma once

#include <atomic>

#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/shared-ptr.h"
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

template <typename T>
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
        while (2 * turn != slot.turn.load(std::memory_order_acquire))
        {
        }
        slot.data = data;
        slot.turn.store(2 * turn + 1, std::memory_order_release);
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
                    slot.data = data;
                    slot.turn.store(2 * turn + 1, std::memory_order_release);
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
        while (2 * turn + 1 != slot.turn.load(std::memory_order_acquire))
        {
        }
        T result = Move(slot.data);
        slot.turn.store(2 * turn + 2, std::memory_order_release);
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
                    result = Move(slot.data);
                    slot.turn.store(2 * turn + 2, std::memory_order_release);
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
 * Part of the multiple-producer multiple-consumer communication channel used for sending data.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being sent.
 */
template <typename T>
struct TransmitterMPMC
{
    TransmitterMPMC() = default;
    explicit TransmitterMPMC(SharedPtr<Impl::QueueMPMC<T>>&& q) : m_queue(Move(q)) {}
    ~TransmitterMPMC() = default;

    TransmitterMPMC(const TransmitterMPMC<T>& q) = delete;
    TransmitterMPMC& operator=(const TransmitterMPMC<T>& q) = delete;

    TransmitterMPMC(TransmitterMPMC&& other) noexcept : m_queue(Move(other.m_queue)) {}
    TransmitterMPMC& operator=(TransmitterMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = Move(other.m_queue);
        return *this;
    }

    TransmitterMPMC Clone() const { return {m_queue->Clone()}; }

    bool operator==(const TransmitterMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(Move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

private:
    SharedPtr<Impl::QueueMPMC<T>> m_queue;
};

/**
 * Part of the single-producer single-consumer communication channel used for receiving data.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being received.
 */
template <typename T>
struct ReceiverMPMC
{
    ReceiverMPMC() = default;
    explicit ReceiverMPMC(SharedPtr<Impl::QueueMPMC<T>>&& q) : m_queue(Move(q)) {}
    ~ReceiverMPMC() = default;

    ReceiverMPMC(const ReceiverMPMC<T>& q) = delete;
    ReceiverMPMC& operator=(const ReceiverMPMC<T>& q) = delete;

    ReceiverMPMC(ReceiverMPMC&& other) noexcept : m_queue(Move(other.m_queue)) {}
    ReceiverMPMC& operator=(ReceiverMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = Move(other.m_queue);
        return *this;
    }

    ReceiverMPMC Clone() const { return {m_queue->Clone()}; }

    T Receive() { return m_queue->Pop(); }
    bool TryReceive(T& result) { return m_queue->TryPop(result); }

    bool operator==(const ReceiverMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueMPMC<T>> m_queue;
};

/**
 * Represents one-way, thread-safe communication channel that can have multiple producers and
 * multiple consumer. To send data you need to use transmitter field and to receive it you need
 * to use receiver field.
 * @tparam T Type of data to be sent over the channel.
 */
template <typename T>
struct ChannelMPMC
{
    TransmitterMPMC<T> transmitter;
    ReceiverMPMC<T> receiver;

    explicit ChannelMPMC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        capacity = GetNextPowerOf2(capacity);
        SharedPtr<Impl::QueueMPMC<T>> q(allocator, capacity, allocator);
        transmitter = TransmitterMPMC<T>(q.Clone());
        receiver = ReceiverMPMC<T>(Move(q));
    }
};

}  // namespace Opal

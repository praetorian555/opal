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
    requires Opal::DefaultConstructable<T>
class QueueSPSC
{
public:
    static_assert(std::atomic<size_t>::is_always_lock_free, "Type size_t is not atomic on this platform!");

    explicit QueueSPSC(size_t capacity, AllocatorBase* allocator = nullptr)
        : m_capacity(GetNextPowerOf2(capacity)), m_data(m_capacity, allocator)
    {
        OPAL_ASSERT(m_data.GetAllocator()->IsThreadSafe(), "Allocator must be thread safe!");
        m_write_idx.store(0, std::memory_order_relaxed);
        m_read_idx.store(0, std::memory_order_relaxed);
    }

    void Push(const T& item)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_relaxed);
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        while (write_idx - read_idx == m_capacity)
        {
            // We are full, busy wait
            read_idx = m_read_idx.load(std::memory_order_relaxed);
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = item;  // Do copy

        m_write_idx.store(write_idx + 1, std::memory_order_release);
    }

    bool TryPush(const T& item)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_relaxed);
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        if (write_idx - read_idx == m_capacity)
        {
            // We are full, busy wait
            return false;
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = item;  // Do copy

        m_write_idx.store(write_idx + 1, std::memory_order_release);
        return true;
    }

    void Push(T&& item)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_relaxed);
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        while (write_idx - read_idx == m_capacity)
        {
            // We are full, busy wait
            read_idx = m_read_idx.load(std::memory_order_relaxed);
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        m_data[bound_write_idx] = Move(item);  // Do move

        m_write_idx.store(write_idx + 1, std::memory_order_release);
    }

    template <typename... Args>
    void PushWithEmplace(const Args&... args)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_relaxed);
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        while (write_idx - read_idx == m_capacity)
        {
            // We are full, busy wait
            read_idx = m_read_idx.load(std::memory_order_relaxed);
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        new (&m_data[bound_write_idx]) T(args...);

        m_write_idx.store(write_idx + 1, std::memory_order_release);
    }

    template <typename... Args>
    bool TryPushWithEmplace(const Args&... args)
    {
        size_t write_idx = m_write_idx.load(std::memory_order_relaxed);
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        if (write_idx - read_idx == m_capacity)
        {
            return false;
        }

        size_t bound_write_idx = write_idx & (m_capacity - 1);
        new (&m_data[bound_write_idx]) T(args...);

        m_write_idx.store(write_idx + 1, std::memory_order_release);
        return true;
    }

    T Pop()
    {
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);
        size_t write_idx = m_write_idx.load(std::memory_order_acquire);
        while (write_idx - read_idx == 0)
        {
            // We are empty, busy wait
            write_idx = m_write_idx.load(std::memory_order_acquire);
        }
        size_t bound_read_idx = read_idx & (m_capacity - 1);
        T result = Move(m_data[bound_read_idx]);
        m_read_idx.store(read_idx + 1, std::memory_order_release);
        return result;
    }

    bool TryPop(T& result)
    {
        size_t read_idx = m_read_idx.load(std::memory_order_relaxed);
        size_t write_idx = m_write_idx.load(std::memory_order_acquire);
        if (write_idx - read_idx == 0)
        {
            return false;
        }
        size_t bound_read_idx = read_idx & (m_capacity - 1);
        result = Move(m_data[bound_read_idx]);
        m_read_idx.store(read_idx + 1, std::memory_order_release);
        return true;
    }

private:
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<size_t> m_write_idx;
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<size_t> m_read_idx;
    OPAL_END_DISABLE_WARNINGS

    size_t m_capacity = 0;
    DynamicArray<T> m_data = nullptr;
};
}  // namespace Impl

/**
 * Part of the single-producer single-consumer communication channel used for sending data.
 * It can't be copied only moved.
 * @tparam T Type of data that is being sent.
 */
template <typename T>
struct TransmitterSPSC
{
    TransmitterSPSC() = default;
    TransmitterSPSC(SharedPtr<Impl::QueueSPSC<T>>&& q) : m_queue(Move(q)) {}
    ~TransmitterSPSC() = default;

    TransmitterSPSC(const TransmitterSPSC<T>& q) = delete;
    TransmitterSPSC& operator=(const TransmitterSPSC<T>& q) = delete;

    TransmitterSPSC(TransmitterSPSC&& other) noexcept : m_queue(Move(other.m_queue)) {}

    TransmitterSPSC& operator=(TransmitterSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = Move(other.m_queue);
        return *this;
    }

    bool operator==(const TransmitterSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(Move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

private:
    SharedPtr<Impl::QueueSPSC<T>> m_queue;
};

/**
 * Part of the single-producer single-consumer communication channel used for receiving data.
 * It can't be copied, only moved.
 * @tparam T Type of data that is being received.
 */
template <typename T>
struct ReceiverSPSC
{
    ReceiverSPSC() = default;
    ReceiverSPSC(SharedPtr<Impl::QueueSPSC<T>>&& q) : m_queue(Move(q)) {}
    ~ReceiverSPSC() = default;

    ReceiverSPSC(const ReceiverSPSC<T>& q) = delete;
    ReceiverSPSC& operator=(const ReceiverSPSC<T>& q) = delete;

    ReceiverSPSC(ReceiverSPSC&& other) noexcept : m_queue(Move(other.m_queue)) {}
    ReceiverSPSC& operator=(ReceiverSPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = Move(other.m_queue);
        return *this;
    }

    T Receive() { return m_queue->Pop(); }
    bool TryReceive(T& result) { return m_queue->TryPop(result); }

    bool operator==(const ReceiverSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueSPSC<T>> m_queue;
};

/**
 * Represents one-way, thread-safe communication channel that can have one producer and one consumer.
 * To send data you need to use transmitter field and to receive it you need to use receiver field.
 * @tparam T Type of data to be sent over the channel.
 */
template <typename T>
struct ChannelSPSC
{
    TransmitterSPSC<T> transmitter;
    ReceiverSPSC<T> receiver;

    ChannelSPSC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        SharedPtr<Impl::QueueSPSC<T>> q(allocator, capacity, allocator);
        transmitter = TransmitterSPSC<T>(q.Clone());
        receiver = ReceiverSPSC<T>(Move(q));
    }
};

}  // namespace Opal

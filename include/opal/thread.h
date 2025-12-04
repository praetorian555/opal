#pragma once

#include <new>
#include <thread>
#include <tuple>
#include <type_traits>

#include "container/dynamic-array.h"
#include "container/shared-ptr.h"
#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/type-traits.h"

#define OPAL_CACHE_LINE_SIZE (64)

namespace Opal
{

namespace Impl
{

struct OPAL_EXPORT ThreadDataBase
{
    AllocatorBase* allocator = nullptr;

    ThreadDataBase(AllocatorBase* a) : allocator(a) {}
    virtual ~ThreadDataBase() = default;
    virtual void Invoke() = 0;
};

template <typename Function, typename... Args>
struct ThreadData : ThreadDataBase
{
    Function function;
    std::tuple<std::decay_t<Args>...> args;

    explicit ThreadData(AllocatorBase* alloc, Function&& f, Args&&... a)
        : ThreadDataBase(alloc), function(std::forward<Function>(f)), args(std::forward<Args>(a)...)
    {
    }

    void Invoke() override { std::apply(function, std::move(args)); }
};

}  // namespace Impl

struct OPAL_EXPORT ThreadHandle
{
    void* native_handle = nullptr;
    void* native_id = nullptr;

    bool operator==(const ThreadHandle& other) const;
};

namespace Impl
{
ThreadHandle OPAL_EXPORT CreateThread(ThreadDataBase* data);
}  // namespace Impl

/**
 * Creates and starts a new thread using the @p function for a body and @p for function arguments.
 * @tparam Function Type of function used for a thread body.
 * @tparam Args Variable list of types of the function arguments.
 * @param function Function used for the thread body.
 * @param args Arguments passed to the function.
 * @return Returns a thread handle in case of a success.
 * @throw Exception when thread creation fails.
 */
template <typename Function, typename... Args>
ThreadHandle CreateThread(Function&& function, Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    OPAL_ASSERT(allocator->IsThreadSafe(), "Allocator must be thread safe!");

    Impl::ThreadDataBase* data =
        Opal::New<Impl::ThreadData<Function, Args...>>(allocator, allocator, std::forward<Function>(function), std::forward<Args>(args)...);
    ThreadHandle handle = Impl::CreateThread(data);
    if (handle.native_handle == nullptr)
    {
        Delete(allocator, data);
    }
    return handle;
}

/**
 * Wait for a thread to exit.
 * @param handle Handle of a thread to wait for.
 */
void OPAL_EXPORT JoinThread(ThreadHandle handle);

/**
 * Get a thread handle of a current thread.
 * @return Thread handle.
 */
ThreadHandle OPAL_EXPORT GetCurrentThreadHandle();

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

private:
    SharedPtr<Impl::QueueSPSC<T>> m_queue;
};

/**
 * Part of the single-producer single-consumer communication channel used for receiving data.
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

    bool operator==(const ReceiverSPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueSPSC<T>> m_queue;
};

/**
 * Represents one-way, thread-safe communication channel that can have one producer and one consumer.
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

template <typename T>
struct Mutex;

template <typename T>
struct MutexGuard
{
    MutexGuard(struct Mutex<T>* lock, T* object) : m_lock(lock), m_object(object) {}

    ~MutexGuard() { m_lock->Unlock(); }

    T* Deref() { return m_object; }

private:
    struct Mutex<T>* m_lock = nullptr;
    T* m_object = nullptr;
};

namespace Impl
{
struct OPAL_EXPORT PureMutex
{
    PureMutex();
    ~PureMutex();

    void Lock();
    void Unlock();

private:
    void* m_native_handle;
    AllocatorBase* m_allocator = nullptr;
};
}  // namespace Impl

template <typename T>
struct Mutex
{
    Mutex(T&& object) : m_object(Move(object)) {}

    template <typename... Args>
    Mutex(Args&&... args) : m_object(std::forward<Args>(args)...)
    {
    }

    MutexGuard<T> Lock()
    {
        m_pure_mutex.Lock();
        return {this, &m_object};
    }

    void Unlock()
    {
        m_pure_mutex.Unlock();
    }

private:
    friend struct MutexGuard<T>;

    T m_object;
    Impl::PureMutex m_pure_mutex;
};

}  // namespace Opal

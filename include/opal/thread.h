#pragma once

#include <thread>
#include <tuple>
#include <type_traits>
#include <new>

#include "container/dynamic-array.h"
#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/type-traits.h"

#if defined(__cpp_lib_hardware_interference_size)
#define OPAL_CACHE_LINE_SIZE (std::hardware_destructive_interference_size)
#else
#define OPAL_CACHE_LINE_SIZE (64)
#endif

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

    void Invoke() override { function(std::get<std::decay_t<Args>>(args)...); }
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

template <typename Function, typename... Args>
ThreadHandle CreateThread(Function&& function, Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    OPAL_ASSERT(allocator->IsThreadSafe(), "Allocator must be thread safe!");

    Impl::ThreadDataBase* data = Opal::New<Impl::ThreadData<Function, Args...>>(allocator, allocator, std::forward<Function>(function),
                                                                                std::forward<Args>(args)...);
    ThreadHandle handle = Impl::CreateThread(data);
    if (handle.native_handle == nullptr)
    {
        Delete(allocator, data);
    }
    return handle;
}

void OPAL_EXPORT JoinThread(ThreadHandle handle);

ThreadHandle OPAL_EXPORT GetCurrentThreadHandle();

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

    template <typename ...Args>
    void PushWithEmplace(const Args& ...args)
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

}  // namespace Opal

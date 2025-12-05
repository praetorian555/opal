#pragma once

#include <new>
#include <thread>
#include <tuple>
#include <type_traits>

#include "../container/dynamic-array.h"
#include "../container/shared-ptr.h"
#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/type-traits.h"

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

}  // namespace Opal

#pragma once

#include <thread>
#include <tuple>
#include <type_traits>

#include "allocator.h"
#include "assert.h"
#include "opal/defines.h"

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
    OPAL_ASSERT(GetDefaultAllocator()->IsThreadSafe(), "Allocator must be thread safe!");

    Impl::ThreadDataBase* data =
        Opal::New<Impl::ThreadData<Function, Args...>>(GetDefaultAllocator(), std::forward<Function>(function), std::forward<Args>(args)...);
    ThreadHandle handle = Impl::CreateThread(data);
    if (handle.native_handle == nullptr)
    {
        Delete(data);
    }
    return handle;
}

void OPAL_EXPORT JoinThread(ThreadHandle handle);

ThreadHandle OPAL_EXPORT GetCurrentThreadHandle();

}  // namespace Opal

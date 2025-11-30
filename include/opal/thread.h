#pragma once

#include <tuple>
#include <type_traits>

#include "allocator.h"
#include "opal/defines.h"

namespace Opal
{

namespace Impl
{

struct OPAL_EXPORT ThreadDataBase
{
    virtual ~ThreadDataBase() = default;
    virtual void Invoke() = 0;
};

template <typename Function, typename... Args>
struct ThreadData : ThreadDataBase
{
    Function function;
    std::tuple<std::decay_t<Args>...> args;

    explicit ThreadData(Function&& f, Args&&... a) : function(std::forward<Function>(f)), args(std::forward<Args>(a)...) {}

    void Invoke() override { function(std::get<std::decay_t<Args>>(args)...); }
};

}  // namespace Impl

struct OPAL_EXPORT ThreadHandle
{
    void* native_handle = nullptr;
    Impl::ThreadDataBase* data = nullptr;
};

namespace Impl
{
ThreadHandle OPAL_EXPORT CreateThread(ThreadDataBase* data);
}  // namespace Impl

template <typename Function, typename... Args>
ThreadHandle CreateThread(Function&& function, Args&&... args)
{
    Impl::ThreadDataBase* data = Opal::New<Impl::ThreadData<Function, Args...>>(std::forward<Function>(function), std::forward<Args>(args)...);
    ThreadHandle handle = Impl::CreateThread(data);
    if (handle.native_handle == nullptr)
    {
        Delete(data);
    }
    handle.data = data;
    return handle;
}

void OPAL_EXPORT JoinThread(ThreadHandle handle);

}  // namespace Opal
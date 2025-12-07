#pragma once

#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Thread-safe primitive that is used to signal events between threads.
 * Notify API should be called while associated mutex is locked but its not required.
 */
class OPAL_EXPORT ConditionVariable
{
public:
    explicit ConditionVariable(AllocatorBase* allocator = nullptr);
    ~ConditionVariable();

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    ConditionVariable(ConditionVariable&& other) noexcept;
    ConditionVariable& operator=(ConditionVariable&& other) noexcept;

    void NotifyOne();
    void NotifyAll();

    template <typename T>
    T* Wait(MutexGuard<T>& mutex_guard)
    {
        Wait(mutex_guard.GetNativeHandle());
        return mutex_guard.Deref();
    }

private:
    void Wait(void* native_mutex_handle);

    void* m_native_handle = nullptr;
    AllocatorBase* m_allocator = nullptr;
};

}
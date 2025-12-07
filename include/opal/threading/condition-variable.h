#pragma once

#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

class OPAL_EXPORT ConditionVariable
{
public:
    ConditionVariable();
    ~ConditionVariable();

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
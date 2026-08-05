#pragma once

#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Thread-safe primitive that is used to signal events between threads.
 * Notify API should be called while associated mutex is locked but its not required.
 * Can be neither copied nor moved: a thread already inside Wait is parked on this object, and relocating it would leave that
 * thread unreachable by any later notification.
 */
class OPAL_EXPORT ConditionVariable
{
public:
    explicit ConditionVariable(AllocatorBase* allocator = nullptr);
    ~ConditionVariable();

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    ConditionVariable(ConditionVariable&&) = delete;
    ConditionVariable& operator=(ConditionVariable&&) = delete;

    void NotifyOne();
    void NotifyAll();

    template <typename T>
    T* Wait(MutexGuard<T>& mutex_guard)
    {
        Wait(mutex_guard.GetNativeHandle());
        return mutex_guard.Deref();
    }

    /**
     * Wait until the predicate holds. The predicate is tested before the first wait, so a notification that arrived before this
     * call is not lost, and again on every wake, so a spurious one does not return early.
     * @param mutex_guard Guard holding the mutex lock.
     * @param predicate Callable taking T& and returning something contextually convertible to bool. Called with the mutex held.
     * @return Pointer to the protected data.
     */
    template <typename T, typename Predicate>
    T* Wait(MutexGuard<T>& mutex_guard, Predicate&& predicate)
    {
        while (!predicate(*mutex_guard.Deref()))
        {
            Wait(mutex_guard.GetNativeHandle());
        }
        return mutex_guard.Deref();
    }

    /**
     * Wait for the condition variable to be signaled or for the timeout to expire.
     * @param mutex_guard Guard holding the mutex lock.
     * @param timeout_ms Timeout in milliseconds.
     * @return true if signaled, false if timed out.
     */
    template <typename T>
    bool WaitFor(MutexGuard<T>& mutex_guard, u64 timeout_ms)
    {
        return WaitFor(mutex_guard.GetNativeHandle(), timeout_ms);
    }

private:
    void Wait(void* native_mutex_handle);
    bool WaitFor(void* native_mutex_handle, u64 timeout_ms);

    void* m_native_handle = nullptr;
    AllocatorBase* m_allocator = nullptr;
};

}
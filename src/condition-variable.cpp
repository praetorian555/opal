#include "opal/threading/condition-variable.h"

#include <new>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_POSIX)
#include "pthread.h"
#include <time.h>
#endif

namespace
{
#if defined(OPAL_PLATFORM_WINDOWS)
using NativeConditionVariable = CONDITION_VARIABLE;
using NativeMutex = SRWLOCK;
#elif defined(OPAL_PLATFORM_POSIX)
using NativeConditionVariable = pthread_cond_t;
using NativeMutex = pthread_mutex_t;
#else
#error "Platform not supported"
#endif

NativeConditionVariable* GetNative(void* storage)
{
    return static_cast<NativeConditionVariable*>(storage);
}
}  // namespace

Opal::ConditionVariable::ConditionVariable()
{
    static_assert(sizeof(NativeConditionVariable) <= k_storage_size, "Native condition variable does not fit its storage");
    static_assert(alignof(NativeConditionVariable) <= k_storage_alignment, "Native condition variable is underaligned in its storage");
#if defined(OPAL_PLATFORM_WINDOWS)
    InitializeConditionVariable(new (m_storage) CONDITION_VARIABLE);
#elif defined(OPAL_PLATFORM_MACOS)
    // WaitFor waits for a relative time, so there is no clock to choose.
    pthread_cond_init(new (m_storage) pthread_cond_t, nullptr);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_condattr_t attributes;
    pthread_condattr_init(&attributes);
    pthread_condattr_setclock(&attributes, CLOCK_MONOTONIC);
    pthread_cond_init(new (m_storage) pthread_cond_t, &attributes);
    pthread_condattr_destroy(&attributes);
#endif
}

Opal::ConditionVariable::~ConditionVariable()
{
#if defined(OPAL_PLATFORM_POSIX)
    pthread_cond_destroy(GetNative(m_storage));
#endif
}

void Opal::ConditionVariable::NotifyOne()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeConditionVariable(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_cond_signal(GetNative(m_storage));
#endif
}

void Opal::ConditionVariable::NotifyAll()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeAllConditionVariable(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_cond_broadcast(GetNative(m_storage));
#endif
}

void Opal::ConditionVariable::Wait(void* native_mutex_handle)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SleepConditionVariableSRW(GetNative(m_storage), static_cast<NativeMutex*>(native_mutex_handle), INFINITE, 0);
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_cond_wait(GetNative(m_storage), static_cast<NativeMutex*>(native_mutex_handle));
#endif
}

bool Opal::ConditionVariable::WaitFor(void* native_mutex_handle, u64 timeout_ms)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    constexpr u64 k_longest_wait_ms = INFINITE - 1;
    u64 remaining_ms = timeout_ms;
    while (true)
    {
        const u64 wait_ms = remaining_ms < k_longest_wait_ms ? remaining_ms : k_longest_wait_ms;
        if (SleepConditionVariableSRW(GetNative(m_storage), static_cast<NativeMutex*>(native_mutex_handle), static_cast<DWORD>(wait_ms), 0) !=
            0)
        {
            return true;
        }
        remaining_ms -= wait_ms;
        if (remaining_ms == 0)
        {
            return false;
        }
    }
#elif defined(OPAL_PLATFORM_MACOS)
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(timeout_ms / 1000);
    ts.tv_nsec = static_cast<long>((timeout_ms % 1000) * 1000000);
    return pthread_cond_timedwait_relative_np(GetNative(m_storage), static_cast<NativeMutex*>(native_mutex_handle), &ts) == 0;
#elif defined(OPAL_PLATFORM_LINUX)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += static_cast<time_t>(timeout_ms / 1000);
    ts.tv_nsec += static_cast<long>((timeout_ms % 1000) * 1000000);
    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }
    return pthread_cond_timedwait(GetNative(m_storage), static_cast<NativeMutex*>(native_mutex_handle), &ts) == 0;
#endif
}

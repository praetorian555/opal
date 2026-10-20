#include "opal/threading/condition-variable.h"

#include "opal/exceptions.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include "pthread.h"
#include <time.h>
#endif

Opal::ConditionVariable::ConditionVariable(AllocatorBase* allocator)
{
    m_allocator = allocator;
    if (m_allocator == nullptr)
    {
        m_allocator = GetDefaultAllocator();
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    CONDITION_VARIABLE* condition_variable = New<CONDITION_VARIABLE>(m_allocator);
    if (condition_variable == nullptr)
    {
        OPAL_RAISE(OutOfMemoryException(m_allocator->GetName(), sizeof(CONDITION_VARIABLE)));
    }
    InitializeConditionVariable(condition_variable);
    m_native_handle = reinterpret_cast<void*>(condition_variable);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_t* cond = New<pthread_cond_t>(m_allocator);
    if (cond == nullptr)
    {
        OPAL_RAISE(OutOfMemoryException(m_allocator->GetName(), sizeof(pthread_cond_t)));
    }
    pthread_condattr_t attributes;
    pthread_condattr_init(&attributes);
    pthread_condattr_setclock(&attributes, CLOCK_MONOTONIC);
    pthread_cond_init(cond, &attributes);
    pthread_condattr_destroy(&attributes);
    m_native_handle = reinterpret_cast<void*>(cond);
#else
#error "Platform not supported"
#endif
}

Opal::ConditionVariable::~ConditionVariable()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    CONDITION_VARIABLE* condition_variable = static_cast<CONDITION_VARIABLE*>(m_native_handle);
    Delete(m_allocator, condition_variable);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_t* cond = static_cast<pthread_cond_t*>(m_native_handle);
    pthread_cond_destroy(cond);
    Delete(m_allocator, cond);
#else
#error "Platform not supported"
#endif
    m_native_handle = nullptr;
}

void Opal::ConditionVariable::NotifyOne()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeConditionVariable(static_cast<CONDITION_VARIABLE*>(m_native_handle));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_signal(static_cast<pthread_cond_t*>(m_native_handle));
#else
#error "Platform not supported"
#endif
}

void Opal::ConditionVariable::NotifyAll()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeAllConditionVariable(static_cast<CONDITION_VARIABLE*>(m_native_handle));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_broadcast(static_cast<pthread_cond_t*>(m_native_handle));
#else
#error "Platform not supported"
#endif
}

void Opal::ConditionVariable::Wait(void* native_mutex_handle)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SleepConditionVariableSRW(static_cast<CONDITION_VARIABLE*>(m_native_handle), static_cast<SRWLOCK*>(native_mutex_handle), INFINITE, 0);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_wait(static_cast<pthread_cond_t*>(m_native_handle), static_cast<pthread_mutex_t*>(native_mutex_handle));
#else
#error "Platform not supported"
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
        if (SleepConditionVariableSRW(static_cast<CONDITION_VARIABLE*>(m_native_handle), static_cast<SRWLOCK*>(native_mutex_handle),
                                      static_cast<DWORD>(wait_ms), 0) != 0)
        {
            return true;
        }
        remaining_ms -= wait_ms;
        if (remaining_ms == 0)
        {
            return false;
        }
    }
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
    const int result =
        pthread_cond_timedwait(static_cast<pthread_cond_t*>(m_native_handle), static_cast<pthread_mutex_t*>(native_mutex_handle), &ts);
    return result == 0;
#else
#error "Platform not supported"
#endif
}

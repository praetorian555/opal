#include "opal/threading/condition-variable.h"

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
    InitializeConditionVariable(condition_variable);
    m_native_handle = reinterpret_cast<void*>(condition_variable);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_t* cond = New<pthread_cond_t>(m_allocator);
    pthread_cond_init(cond, nullptr);
    m_native_handle = reinterpret_cast<void*>(cond);
#else
    throw NotImplementedException(__FUNCTION__);
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
    throw Opal::NotImplementedException(__FUNCTION__);
#endif
}

Opal::ConditionVariable::ConditionVariable(ConditionVariable&& other) noexcept
{
    if (this == &other)
    {
        return;
    }
    m_native_handle = other.m_native_handle;
    m_allocator = other.m_allocator;
    other.m_native_handle = nullptr;
    other.m_allocator = nullptr;
}

Opal::ConditionVariable& Opal::ConditionVariable::operator=(ConditionVariable&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (m_native_handle != nullptr)
    {
        this->~ConditionVariable();
    }
    m_native_handle = other.m_native_handle;
    m_allocator = other.m_allocator;
    other.m_native_handle = nullptr;
    return *this;
}

void Opal::ConditionVariable::NotifyOne()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeConditionVariable(static_cast<CONDITION_VARIABLE*>(m_native_handle));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_signal(static_cast<pthread_cond_t*>(m_native_handle));
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::ConditionVariable::NotifyAll()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeAllConditionVariable(static_cast<CONDITION_VARIABLE*>(m_native_handle));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_broadcast(static_cast<pthread_cond_t*>(m_native_handle));
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::ConditionVariable::Wait(void* native_mutex_handle)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SleepConditionVariableCS(static_cast<CONDITION_VARIABLE*>(m_native_handle), static_cast<CRITICAL_SECTION*>(native_mutex_handle),
                             INFINITE);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_cond_wait(static_cast<pthread_cond_t*>(m_native_handle), static_cast<pthread_mutex_t*>(native_mutex_handle));
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

bool Opal::ConditionVariable::WaitFor(void* native_mutex_handle, u64 timeout_ms)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const BOOL result = SleepConditionVariableCS(static_cast<CONDITION_VARIABLE*>(m_native_handle),
                                                 static_cast<CRITICAL_SECTION*>(native_mutex_handle),
                                                 static_cast<DWORD>(timeout_ms));
    return result != 0;
#elif defined(OPAL_PLATFORM_LINUX)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
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
    throw NotImplementedException(__FUNCTION__);
#endif
}

#include "opal/threading/mutex.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>
#endif

Opal::Impl::PureMutex::PureMutex()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    m_allocator = GetDefaultAllocator();
    LPCRITICAL_SECTION critical_section = New<CRITICAL_SECTION>(m_allocator);
    InitializeCriticalSection(critical_section);
    m_native_handle = reinterpret_cast<void*>(critical_section);
#elif defined(OPAL_PLATFORM_LINUX)
    m_allocator = GetDefaultAllocator();
    pthread_mutex_t* mutex = New<pthread_mutex_t>(m_allocator);
    pthread_mutex_init(mutex, nullptr);
    m_native_handle = reinterpret_cast<void*>(mutex);
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

Opal::Impl::PureMutex::~PureMutex()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    if (m_native_handle != nullptr)
    {
        LPCRITICAL_SECTION critical_section = reinterpret_cast<LPCRITICAL_SECTION>(m_native_handle);
        DeleteCriticalSection(critical_section);
        Delete(m_allocator, m_native_handle);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    if (m_native_handle != nullptr)
    {
        pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
        pthread_mutex_destroy(mutex);
        Delete(m_allocator, mutex);
    }
#else
    throw NotImplementedException(__FUNCTION__)
#endif
}

void Opal::Impl::PureMutex::Lock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    if (m_native_handle != nullptr)
    {
        LPCRITICAL_SECTION critical_section = reinterpret_cast<LPCRITICAL_SECTION>(m_native_handle);
        EnterCriticalSection(critical_section);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    if (m_native_handle != nullptr)
    {
        pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
        pthread_mutex_lock(mutex);
    }
#else
    throw NotImplementedException(__FUNCTION__)
#endif
}

void Opal::Impl::PureMutex::Unlock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    if (m_native_handle != nullptr)
    {
        LPCRITICAL_SECTION critical_section = reinterpret_cast<LPCRITICAL_SECTION>(m_native_handle);
        LeaveCriticalSection(critical_section);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    if (m_native_handle != nullptr)
    {
        pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
        pthread_mutex_unlock(mutex);
    }
#else
    throw NotImplementedException(__FUNCTION__)
#endif
}
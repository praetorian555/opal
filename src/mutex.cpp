#include "opal/threading/mutex.h"

#include "opal/exceptions.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>
#endif

Opal::Impl::PureMutex::PureMutex()
{
    m_allocator = GetDefaultAllocator();
#if defined(OPAL_PLATFORM_WINDOWS)
    SRWLOCK* lock = New<SRWLOCK>(m_allocator);
    if (lock == nullptr)
    {
        OPAL_RAISE(OutOfMemoryException(m_allocator->GetName(), sizeof(SRWLOCK)));
    }
    InitializeSRWLock(lock);
    m_native_handle = reinterpret_cast<void*>(lock);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_mutex_t* mutex = New<pthread_mutex_t>(m_allocator);
    if (mutex == nullptr)
    {
        OPAL_RAISE(OutOfMemoryException(m_allocator->GetName(), sizeof(pthread_mutex_t)));
    }
    pthread_mutex_init(mutex, nullptr);
    m_native_handle = reinterpret_cast<void*>(mutex);
#else
#error "Platform not supported"
#endif
}

Opal::Impl::PureMutex::~PureMutex()
{
    if (m_native_handle == nullptr)
    {
        return;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    // An SRWLOCK has no destroy call.
    SRWLOCK* lock = reinterpret_cast<SRWLOCK*>(m_native_handle);
    Delete(m_allocator, lock);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
    pthread_mutex_destroy(mutex);
    Delete(m_allocator, mutex);
#else
#error "Platform not supported"
#endif
    m_native_handle = nullptr;
}

bool Opal::Impl::PureMutex::TryLock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SRWLOCK* lock = reinterpret_cast<SRWLOCK*>(m_native_handle);
    return TryAcquireSRWLockExclusive(lock) != 0;
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
    return pthread_mutex_trylock(mutex) == 0;
#else
#error "Platform not supported"
#endif
}

void Opal::Impl::PureMutex::Lock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SRWLOCK* lock = reinterpret_cast<SRWLOCK*>(m_native_handle);
    AcquireSRWLockExclusive(lock);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
    pthread_mutex_lock(mutex);
#else
#error "Platform not supported"
#endif
}

void Opal::Impl::PureMutex::Unlock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    SRWLOCK* lock = reinterpret_cast<SRWLOCK*>(m_native_handle);
    ReleaseSRWLockExclusive(lock);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(m_native_handle);
    pthread_mutex_unlock(mutex);
#else
#error "Platform not supported"
#endif
}

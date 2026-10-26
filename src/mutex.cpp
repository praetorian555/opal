#include "opal/threading/mutex.h"

#include <new>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_POSIX)
#include <pthread.h>
#endif

namespace
{
#if defined(OPAL_PLATFORM_WINDOWS)
using NativeMutex = SRWLOCK;
#elif defined(OPAL_PLATFORM_POSIX)
using NativeMutex = pthread_mutex_t;
#else
#error "Platform not supported"
#endif

static_assert(sizeof(NativeMutex) <= Opal::Impl::PureMutex::k_storage_size, "Native mutex does not fit its storage");
static_assert(alignof(NativeMutex) <= Opal::Impl::PureMutex::k_storage_alignment, "Native mutex is underaligned in its storage");

NativeMutex* GetNative(void* storage)
{
    return static_cast<NativeMutex*>(storage);
}
}  // namespace

Opal::Impl::PureMutex::PureMutex()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    InitializeSRWLock(new (m_storage) SRWLOCK);
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_mutex_init(new (m_storage) pthread_mutex_t, nullptr);
#endif
}

Opal::Impl::PureMutex::~PureMutex()
{
#if defined(OPAL_PLATFORM_POSIX)
    pthread_mutex_destroy(GetNative(m_storage));
#endif
}

bool Opal::Impl::PureMutex::TryLock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return TryAcquireSRWLockExclusive(GetNative(m_storage)) != 0;
#elif defined(OPAL_PLATFORM_POSIX)
    return pthread_mutex_trylock(GetNative(m_storage)) == 0;
#endif
}

void Opal::Impl::PureMutex::Lock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    AcquireSRWLockExclusive(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_mutex_lock(GetNative(m_storage));
#endif
}

void Opal::Impl::PureMutex::Unlock()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    ReleaseSRWLockExclusive(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_mutex_unlock(GetNative(m_storage));
#endif
}

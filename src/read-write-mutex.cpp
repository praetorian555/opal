#include "opal/threading/read-write-mutex.h"

#include <new>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>
#endif

namespace
{
#if defined(OPAL_PLATFORM_WINDOWS)
using NativeLock = SRWLOCK;
#elif defined(OPAL_PLATFORM_LINUX)
using NativeLock = pthread_rwlock_t;
#else
#error "Platform not supported"
#endif

static_assert(sizeof(NativeLock) <= Opal::Impl::PureReadWriteMutex::k_storage_size, "Native read-write lock does not fit its storage");
static_assert(alignof(NativeLock) <= Opal::Impl::PureReadWriteMutex::k_storage_alignment,
              "Native read-write lock is underaligned in its storage");

NativeLock* GetNative(Opal::u8* storage)
{
    return reinterpret_cast<NativeLock*>(storage);
}
}  // namespace

Opal::Impl::PureReadWriteMutex::PureReadWriteMutex()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    InitializeSRWLock(new (m_storage) SRWLOCK);
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_rwlockattr_t attributes;
    pthread_rwlockattr_init(&attributes);
#if defined(__GLIBC__) || (defined(__ANDROID_API__) && __ANDROID_API__ >= 23)
    pthread_rwlockattr_setkind_np(&attributes, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
#endif
    pthread_rwlock_init(new (m_storage) pthread_rwlock_t, &attributes);
    pthread_rwlockattr_destroy(&attributes);
#endif
}

Opal::Impl::PureReadWriteMutex::~PureReadWriteMutex()
{
#if defined(OPAL_PLATFORM_LINUX)
    pthread_rwlock_destroy(GetNative(m_storage));
#endif
}

void Opal::Impl::PureReadWriteMutex::LockRead()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    AcquireSRWLockShared(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_rwlock_rdlock(GetNative(m_storage));
#endif
}

bool Opal::Impl::PureReadWriteMutex::TryLockRead()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return TryAcquireSRWLockShared(GetNative(m_storage)) != 0;
#elif defined(OPAL_PLATFORM_LINUX)
    return pthread_rwlock_tryrdlock(GetNative(m_storage)) == 0;
#endif
}

void Opal::Impl::PureReadWriteMutex::UnlockRead()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    ReleaseSRWLockShared(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_rwlock_unlock(GetNative(m_storage));
#endif
}

void Opal::Impl::PureReadWriteMutex::LockWrite()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    AcquireSRWLockExclusive(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_rwlock_wrlock(GetNative(m_storage));
#endif
}

bool Opal::Impl::PureReadWriteMutex::TryLockWrite()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return TryAcquireSRWLockExclusive(GetNative(m_storage)) != 0;
#elif defined(OPAL_PLATFORM_LINUX)
    return pthread_rwlock_trywrlock(GetNative(m_storage)) == 0;
#endif
}

void Opal::Impl::PureReadWriteMutex::UnlockWrite()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    ReleaseSRWLockExclusive(GetNative(m_storage));
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_rwlock_unlock(GetNative(m_storage));
#endif
}

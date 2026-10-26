#pragma once

#include <type_traits>
#include <utility>

#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/types.h"

namespace Opal
{

namespace Impl
{
/**
 * Lock that many threads can hold for reading at once, or one thread for writing. Holds the OS lock in place, so it neither
 * allocates nor fails to construct. Neither mode is recursive.
 */
class PureReadWriteMutex
{
public:
    PureReadWriteMutex();
    ~PureReadWriteMutex();

    PureReadWriteMutex(const PureReadWriteMutex&) = delete;
    PureReadWriteMutex& operator=(const PureReadWriteMutex&) = delete;

    PureReadWriteMutex(PureReadWriteMutex&&) = delete;
    PureReadWriteMutex& operator=(PureReadWriteMutex&&) = delete;

    void LockRead();
    bool TryLockRead();
    void UnlockRead();

    void LockWrite();
    bool TryLockWrite();
    void UnlockWrite();

#if defined(OPAL_PLATFORM_MACOS)
    static constexpr size_t k_storage_size = 200;
#else
    static constexpr size_t k_storage_size = 64;
#endif
    static constexpr size_t k_storage_alignment = 8;

private:
    alignas(k_storage_alignment) u8 m_storage[k_storage_size];
};
}  // namespace Impl

/**
 * Object returned when a ReadWriteMutex is locked for reading. Gives const access to the data while the lock is held, and
 * releases it when destroyed.
 * @tparam T Type of secured data.
 */
template <typename T>
class ReadGuard
{
public:
    ReadGuard(Impl::PureReadWriteMutex* lock, const T* object) : m_lock(lock), m_object(object) {}
    ~ReadGuard()
    {
        if (m_lock != nullptr)
        {
            m_lock->UnlockRead();
        }
    }

    ReadGuard(const ReadGuard&) = delete;
    ReadGuard& operator=(const ReadGuard&) = delete;

    ReadGuard(ReadGuard&& other) noexcept : m_lock(other.m_lock), m_object(other.m_object)
    {
        other.m_lock = nullptr;
        other.m_object = nullptr;
    }
    ReadGuard& operator=(ReadGuard&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        if (m_lock != nullptr)
        {
            m_lock->UnlockRead();
        }
        m_lock = other.m_lock;
        m_object = other.m_object;
        other.m_lock = nullptr;
        other.m_object = nullptr;
        return *this;
    }

    /** @return Pointer to the protected data. Ends the program if this guard has been moved from and holds no lock. */
    const T* Deref() const
    {
        OPAL_VERIFY(m_object != nullptr, "ReadGuard used after being moved from");
        return m_object;
    }

    const T& operator*() const { return *Deref(); }
    const T* operator->() const { return Deref(); }

private:
    Impl::PureReadWriteMutex* m_lock = nullptr;
    const T* m_object = nullptr;
};

/**
 * Object returned when a ReadWriteMutex is locked for writing. Gives exclusive access to the data while the lock is held, and
 * releases it when destroyed.
 * @tparam T Type of secured data.
 */
template <typename T>
class WriteGuard
{
public:
    WriteGuard(Impl::PureReadWriteMutex* lock, T* object) : m_lock(lock), m_object(object) {}
    ~WriteGuard()
    {
        if (m_lock != nullptr)
        {
            m_lock->UnlockWrite();
        }
    }

    WriteGuard(const WriteGuard&) = delete;
    WriteGuard& operator=(const WriteGuard&) = delete;

    WriteGuard(WriteGuard&& other) noexcept : m_lock(other.m_lock), m_object(other.m_object)
    {
        other.m_lock = nullptr;
        other.m_object = nullptr;
    }
    WriteGuard& operator=(WriteGuard&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        if (m_lock != nullptr)
        {
            m_lock->UnlockWrite();
        }
        m_lock = other.m_lock;
        m_object = other.m_object;
        other.m_lock = nullptr;
        other.m_object = nullptr;
        return *this;
    }

    /** @return Pointer to the protected data. Ends the program if this guard has been moved from and holds no lock. */
    T* Deref()
    {
        OPAL_VERIFY(m_object != nullptr, "WriteGuard used after being moved from");
        return m_object;
    }
    const T* Deref() const
    {
        OPAL_VERIFY(m_object != nullptr, "WriteGuard used after being moved from");
        return m_object;
    }

    T& operator*() { return *Deref(); }
    const T& operator*() const { return *Deref(); }
    T* operator->() { return Deref(); }
    const T* operator->() const { return Deref(); }

private:
    Impl::PureReadWriteMutex* m_lock = nullptr;
    T* m_object = nullptr;
};

/**
 * Wrapper around an object of type T that lets many threads read it at once, or one thread write it. The data is reachable only
 * through a ReadGuard, which hands out const access, or a WriteGuard.
 *
 * On Linux and Android a thread waiting to write holds off readers that arrive after it, so a steady stream of readers cannot
 * starve it; Windows makes no fairness promise either way. Neither lock is recursive: a thread that already holds the lock in
 * either mode and locks it again can deadlock, reads included, since a writer may have queued in between.
 * Can be neither copied nor moved, for the same reason as Mutex.
 * @tparam T Type of data to secure access to.
 */
template <typename T>
class ReadWriteMutex
{
public:
    explicit ReadWriteMutex(T&& object) : m_object(std::move(object)) {}

    /** Constructs the protected object in place. Never a better match for a ReadWriteMutex than the deleted copy constructor is. */
    template <typename... Args>
        requires(sizeof...(Args) != 1 || !(std::is_same_v<std::remove_cvref_t<Args>, ReadWriteMutex> || ...))
    explicit ReadWriteMutex(Args&&... args) : m_object(std::forward<Args>(args)...)
    {
    }

    ReadWriteMutex(const ReadWriteMutex&) = delete;
    ReadWriteMutex& operator=(const ReadWriteMutex&) = delete;

    ReadWriteMutex(ReadWriteMutex&&) = delete;
    ReadWriteMutex& operator=(ReadWriteMutex&&) = delete;

    /** Block until no thread holds the lock for writing, then hold it for reading. */
    ReadGuard<T> LockRead()
    {
        m_lock.LockRead();
        return {&m_lock, &m_object};
    }

    /** Block until no thread holds the lock in either mode, then hold it for writing. */
    WriteGuard<T> LockWrite()
    {
        m_lock.LockWrite();
        return {&m_lock, &m_object};
    }

    /** @return A read guard, or false if a thread holds the lock for writing. */
    Expected<ReadGuard<T>, bool> TryLockRead()
    {
        if (m_lock.TryLockRead())
        {
            return Expected<ReadGuard<T>, bool>(ReadGuard<T>{&m_lock, &m_object});
        }
        return Expected<ReadGuard<T>, bool>(false);
    }

    /** @return A write guard, or false if any thread holds the lock. */
    Expected<WriteGuard<T>, bool> TryLockWrite()
    {
        if (m_lock.TryLockWrite())
        {
            return Expected<WriteGuard<T>, bool>(WriteGuard<T>{&m_lock, &m_object});
        }
        return Expected<WriteGuard<T>, bool>(false);
    }

private:
    T m_object;
    Impl::PureReadWriteMutex m_lock;
};

}  // namespace Opal

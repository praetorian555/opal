#pragma once

#include <type_traits>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/export.h"

namespace Opal
{

template <typename T>
struct Mutex;

/**
 * Object returned when mutex is locked. Can be used to access underlying data,
 * but only while the mutex is locked.
 * @tparam T Type of secured data.
 */
template <typename T>
struct MutexGuard
{
    MutexGuard(struct Mutex<T>* lock, T* object) : m_lock(lock), m_object(object) {}
    ~MutexGuard()
    {
        if (m_lock != nullptr)
        {
            m_lock->Unlock();
        }
    }

    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;

    MutexGuard(MutexGuard&& other) noexcept : m_lock(other.m_lock), m_object(other.m_object)
    {
        other.m_lock = nullptr;
        other.m_object = nullptr;
    }
    MutexGuard& operator=(MutexGuard&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        if (m_lock != nullptr)
        {
            m_lock->Unlock();
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
        OPAL_VERIFY(m_object != nullptr, "MutexGuard used after being moved from");
        return m_object;
    }
    const T* Deref() const
    {
        OPAL_VERIFY(m_object != nullptr, "MutexGuard used after being moved from");
        return m_object;
    }

    T& operator*() { return *Deref(); }
    const T& operator*() const { return *Deref(); }
    T* operator->() { return Deref(); }
    const T* operator->() const { return Deref(); }

    void* GetNativeHandle() { return m_lock->GetNativeHandle(); }

private:
    struct Mutex<T>* m_lock = nullptr;
    T* m_object = nullptr;
};

namespace Impl
{
struct OPAL_EXPORT PureMutex
{
    PureMutex();
    ~PureMutex();

    PureMutex(const PureMutex&) = delete;
    PureMutex& operator=(const PureMutex&) = delete;

    PureMutex(PureMutex&&) = delete;
    PureMutex& operator=(PureMutex&&) = delete;

    void Lock();
    bool TryLock();
    void Unlock();

    void* GetNativeHandle() { return m_native_handle; }

private:
    void* m_native_handle = nullptr;
    AllocatorBase* m_allocator = nullptr;
};
}  // namespace Impl

/**
 * Wrapper around object of type T used to secure exclusive access to it.
 * Can be neither copied nor moved: a live MutexGuard points at this object and at the data it protects, so relocating either
 * would leave the guard unlocking and dereferencing storage that has been moved out of.
 * @tparam T Type of data to secure access to.
 */
template <typename T>
struct Mutex
{
    explicit Mutex(T&& object) : m_object(Move(object)) {}

    /** Constructs the protected object in place. Never a better match for a Mutex than the deleted copy constructor is. */
    template <typename... Args>
        requires(sizeof...(Args) != 1 || !(std::is_same_v<std::remove_cvref_t<Args>, Mutex> || ...))
    explicit Mutex(Args&&... args) : m_object(std::forward<Args>(args)...)
    {
    }

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    Mutex(Mutex&&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    MutexGuard<T> Lock()
    {
        m_pure_mutex.Lock();
        return {this, &m_object};
    }

    Expected<MutexGuard<T>, bool> TryLock()
    {
        if (m_pure_mutex.TryLock())
        {
            return Expected<MutexGuard<T>, bool>(MutexGuard<T>{this, &m_object});
        }
        return Expected<MutexGuard<T>, bool>(false);
    }

    void* GetNativeHandle() { return m_pure_mutex.GetNativeHandle(); }

private:
    friend struct MutexGuard<T>;

    // Private: the guard owns the unlock. A caller that unlocked here would leave the guard to unlock a second time.
    void Unlock() { m_pure_mutex.Unlock(); }

    T m_object;
    Impl::PureMutex m_pure_mutex;
};

}  // namespace Opal
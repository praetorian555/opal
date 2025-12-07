#pragma once

#include "opal/allocator.h"
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
    ~MutexGuard() { m_lock->Unlock(); }

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
        m_lock = other.m_lock;
        m_object = other.m_object;
        other.m_lock = nullptr;
        other.m_object = nullptr;
        return *this;
    }

    T* Deref() { return m_object; }

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

    PureMutex(PureMutex&& other) noexcept;
    PureMutex& operator=(PureMutex&& other) noexcept;

    void Lock();
    void Unlock();

    void* GetNativeHandle() { return m_native_handle; }

private:
    void* m_native_handle = nullptr;
    AllocatorBase* m_allocator = nullptr;
};
}  // namespace Impl

/**
 * Wrapper around object of type T used to secure exclusive access to it.
 * Can't be copied, only moved.
 * @tparam T Type of data to secure access to.
 */
template <typename T>
struct Mutex
{
    explicit Mutex(T&& object) : m_object(Move(object)) {}

    template <typename... Args>
    explicit Mutex(Args&&... args) : m_object(std::forward<Args>(args)...)
    {
    }

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    Mutex(Mutex&& other) noexcept : m_object(Move(other.m_object)), m_pure_mutex(Move(other.m_pure_mutex)) {}
    Mutex& operator=(Mutex&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        m_object = Move(other.m_object);
        m_pure_mutex = Move(other.m_pure_mutex);
        return *this;
    }

    MutexGuard<T> Lock()
    {
        m_pure_mutex.Lock();
        return {this, &m_object};
    }

    void Unlock() { m_pure_mutex.Unlock(); }

    void* GetNativeHandle() { return m_pure_mutex.GetNativeHandle(); }

private:
    friend struct MutexGuard<T>;

    T m_object;
    Impl::PureMutex m_pure_mutex;
};

}  // namespace Opal
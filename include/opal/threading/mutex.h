#pragma once

#include "opal/allocator.h"
#include "opal/export.h"

namespace Opal
{

template <typename T>
struct Mutex;

template <typename T>
struct MutexGuard
{
    MutexGuard(struct Mutex<T>* lock, T* object) : m_lock(lock), m_object(object) {}

    ~MutexGuard() { m_lock->Unlock(); }

    T* Deref() { return m_object; }

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

    void Lock();
    void Unlock();

private:
    void* m_native_handle;
    AllocatorBase* m_allocator = nullptr;
};
}  // namespace Impl

template <typename T>
struct Mutex
{
    explicit Mutex(T&& object) : m_object(Move(object)) {}

    template <typename... Args>
    explicit Mutex(Args&&... args) : m_object(std::forward<Args>(args)...)
    {
    }

    MutexGuard<T> Lock()
    {
        m_pure_mutex.Lock();
        return {this, &m_object};
    }

    void Unlock() { m_pure_mutex.Unlock(); }

private:
    friend struct MutexGuard<T>;

    T m_object;
    Impl::PureMutex m_pure_mutex;
};

}  // namespace Opal
#pragma once

#include <atomic>

#include "opal/allocator.h"
#include "opal/defines.h"

namespace Opal
{

/**
 * Thread-safe, reference counting wrapper around objects of the desired type.
 * Object that is wrapped is created when shared pointer is constructed. It is not possible to pass in a pointer to object
 * not created inside the shared pointer.
 * When last shared pointer that points to the same data is destroyed the data is destroyed as well.
 * Shared pointer can only be copied using the Clone API. It can be moved.
 * @tparam T Object type stored inside the shared pointer.
 */
template <typename T>
class SharedPtr
{
public:
    SharedPtr() = default;

    template <typename ...Args>
    SharedPtr(AllocatorBase* allocator, Args&&... args)
    {
        if (allocator == nullptr)
        {
            allocator = GetDefaultAllocator();
        }
        OPAL_ASSERT(allocator && allocator->IsThreadSafe(), "Allocator should be thread-safe");
        m_object = New<T>(allocator, std::forward<Args>(args)...);
        m_refcount = New<std::atomic<size_t>>(allocator);
        m_refcount->store(1, std::memory_order_relaxed);
        m_allocator = allocator;
    }

    SharedPtr(AllocatorBase* allocator, T* object)
    {
        if (allocator == nullptr)
        {
            allocator = GetDefaultAllocator();
        }
        if (object == nullptr)
        {
            return;
        }
        OPAL_ASSERT(allocator && allocator->IsThreadSafe(), "Allocator should be thread-safe");
        m_object = object;
        m_refcount = New<std::atomic<size_t>>(allocator);
        m_refcount->store(1, std::memory_order_relaxed);
        m_allocator = allocator;
    }

    ~SharedPtr()
    {
        Reset();
    }

    SharedPtr(const SharedPtr&) = delete;
    SharedPtr& operator=(const SharedPtr&) = delete;

    SharedPtr(SharedPtr&& other) noexcept : m_object(other.m_object), m_allocator(other.m_allocator), m_refcount(other.m_refcount)
    {
        other.m_object = nullptr;
        other.m_allocator = nullptr;
        other.m_refcount = nullptr;
    }

    template <typename U>
        requires Convertible<U*, T*>
    explicit SharedPtr(SharedPtr<U>&& other)  noexcept : m_object(static_cast<T*>(other.m_object)), m_allocator(other.m_allocator), m_refcount(other.m_refcount)
    {
        other.m_object = nullptr;
        other.m_allocator = nullptr;
        other.m_refcount = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        Reset();
        m_object = other.m_object;
        m_refcount = other.m_refcount;
        m_allocator = other.m_allocator;
        other.m_object = nullptr;
        other.m_refcount = nullptr;
        other.m_allocator = nullptr;
        return *this;
    }

    T* operator->() { return m_object; }
    const T* operator->() const { return m_object; }

    SharedPtr Clone() const
    {
        SharedPtr clone;
        if (!IsValid())
        {
            return clone;
        }
        clone.m_object = m_object;
        clone.m_refcount = m_refcount;
        clone.m_refcount->fetch_add(1, std::memory_order_relaxed);
        clone.m_allocator = m_allocator;
        return clone;
    }

    void Reset()
    {
        if (m_refcount != nullptr && m_refcount->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            Delete(m_allocator, m_object);
            Delete(m_allocator, m_refcount);
        }
        m_object = nullptr;
        m_refcount = nullptr;
        m_allocator = nullptr;
    }

    [[nodiscard]] bool IsValid() const
    {
        return m_object != nullptr;
    }

    bool operator==(const SharedPtr& other) const
    {
        return m_object == other.m_object;
    }

    T* Get() { return m_object; }
    const T* Get() const { return m_object; }

    Ref<T> GetRef() { return Ref<T>(m_object); }
    Ref<const T> GetRef() const { return Ref<const T>(m_object); }

private:
    template <typename U>
    friend class SharedPtr;

    T* m_object = nullptr;
    AllocatorBase* m_allocator = nullptr;
    std::atomic<size_t>* m_refcount = nullptr;
};

}  // namespace Opal

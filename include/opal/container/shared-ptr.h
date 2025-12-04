#pragma once

#include <atomic>

#include "opal/allocator.h"
#include "opal/defines.h"

namespace Opal
{

template <typename T>
class SharedPtr
{
public:
    SharedPtr() = default;

    template <typename ...Args>
    SharedPtr(AllocatorBase* allocator, Args&&... args)
    {
        OPAL_ASSERT(allocator && allocator->IsThreadSafe(), "Allocator should be thread-safe");
        m_object = New<T>(allocator, std::forward<Args>(args)...);
        m_refcount = New<std::atomic<size_t>>(allocator);
        m_refcount->store(1, std::memory_order_relaxed);
    }

    ~SharedPtr()
    {
        if (m_refcount != nullptr && m_refcount->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            Delete(m_allocator, m_object);
            m_object = nullptr;
            Delete(m_allocator, m_refcount);
            m_refcount = nullptr;
        }
    }

    SharedPtr(const SharedPtr&) = delete;
    SharedPtr& operator=(const SharedPtr&) = delete;

    SharedPtr(SharedPtr&& other) noexcept : m_refcount(other.m_refcount), m_object(other.m_object)
    {
        other.m_object = nullptr;
        other.m_refcount = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        this->~SharedPtr();
        m_object = other.m_object;
        m_refcount = other.m_refcount;
        other.m_object = nullptr;
        other.m_refcount = nullptr;
        return *this;
    }

    SharedPtr Clone()
    {
        SharedPtr clone;
        clone.m_object = m_object;
        clone.m_refcount = m_refcount;
        clone.m_refcount->fetch_add(1, std::memory_order_relaxed);
        clone.m_allocator = m_allocator;
        return clone;
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

private:
    T* m_object = nullptr;
    AllocatorBase* m_allocator = nullptr;
    std::atomic<size_t>* m_refcount = nullptr;
};

}  // namespace Opal

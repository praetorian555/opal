#pragma once

#include <utility>

#include "opal/allocator.h"
#include "opal/type-traits.h"

namespace Opal
{

/**
 * Ownership wrapper around object of type T.
 * Internal object can only be created in the constructor of the scope pointer and this pointer is responsible for the object's
 * lifetime.
 * @tparam T
 */
template <typename T>
class ScopePtr
{
public:
    ScopePtr() = default;

    template <typename U>
        requires Convertible<U*, T*>
    ScopePtr(AllocatorBase* allocator, U* object)
        : m_ptr(static_cast<U*>(object)), m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
    {
    }

    template <typename... Args>
    explicit ScopePtr(AllocatorBase* allocator, Args&&... args) : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
    {
        m_ptr = New<T>(m_allocator, std::forward<Args>(args)...);
    }

    ScopePtr(const ScopePtr& other) = delete;
    ScopePtr& operator=(const ScopePtr& other) = delete;

    ScopePtr(ScopePtr&& other) noexcept : m_ptr(other.m_ptr), m_allocator(other.m_allocator)
    {
        other.m_ptr = nullptr;
        other.m_allocator = nullptr;
    }

    template <typename U>
        requires Convertible<U*, T*>
    ScopePtr(ScopePtr<U>&& other) noexcept
    {
        m_ptr = static_cast<T*>(other.m_ptr);
        m_allocator = other.GetAllocator();
        other.m_ptr = nullptr;
        other.m_allocator = nullptr;
    }

    ScopePtr& operator=(ScopePtr&& other)
    {
        if (this != &other)
        {
            if (m_ptr != nullptr)
            {
                Delete(m_allocator, m_ptr);
            }
            m_ptr = other.m_ptr;
            m_allocator = other.m_allocator;
            other.m_ptr = nullptr;
            other.m_allocator = nullptr;
        }
        return *this;
    }

    ~ScopePtr()
    {
        if (m_ptr != nullptr)
        {
            Delete(m_allocator, m_ptr);
        }
    }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* Get() const { return m_ptr; }
    [[nodiscard]] AllocatorBase* GetAllocator() const { return m_allocator; }

    void Reset()
    {
        if (m_ptr != nullptr)
        {
            Delete(m_allocator, m_ptr);
        }
        m_ptr = nullptr;
        m_allocator = nullptr;
    }

    [[nodiscard]] bool IsValid() const { return m_ptr != nullptr; }

    explicit operator bool() const { return IsValid(); }

    bool operator==(const ScopePtr& other) const { return m_ptr == other.m_ptr; }
    bool operator==(const T* ptr) const { return m_ptr == ptr; }
    bool operator==(decltype(nullptr)) const { return m_ptr == nullptr; }

private:
    template <typename U>
    friend class ScopePtr;

    T* m_ptr = nullptr;
    AllocatorBase* m_allocator = nullptr;
};

}  // namespace Opal

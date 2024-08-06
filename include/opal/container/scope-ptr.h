#pragma once

// TODO: See if we can implement std::forward
#include <utility>

#include "opal/allocator.h"
#include "opal/defines.h"
#include "opal/assert.h"
#include "opal/type-traits.h"

namespace Opal
{

template <typename T, typename Allocator = AllocatorBase>
class ScopePtr
{
public:
    ScopePtr() : m_ptr(nullptr), m_allocator(GetDefaultAllocator()) {}

    explicit ScopePtr(T* ptr, Allocator* allocator = nullptr)
        : m_ptr(ptr), m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
    {
    }

    ScopePtr(const ScopePtr& other) = delete;
    ScopePtr& operator=(const ScopePtr& other) = delete;

    ScopePtr(ScopePtr&& other)  noexcept : m_ptr(other.m_ptr), m_allocator(other.m_allocator)
    {
        other.m_ptr = nullptr;
        other.m_allocator = nullptr;
    }

    template <typename U, typename OtherAllocator = AllocatorBase>
        requires Convertible<U*, T*> && Convertible<Allocator*, AllocatorBase*>
    ScopePtr(ScopePtr<U, OtherAllocator>&& other)  noexcept
    {
        m_ptr = static_cast<T*>(other.Release());
        m_allocator = static_cast<AllocatorBase*>(other.GetAllocator());
    }

    ScopePtr& operator=(ScopePtr&& other)
    {
        if (this != &other)
        {
            if (m_ptr != nullptr)
            {
                m_ptr->~T();
                OPAL_ASSERT(m_allocator != nullptr, "Allocator is null!");
                m_allocator->Free(m_ptr);
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
            m_ptr->~T();
            OPAL_ASSERT(m_allocator != nullptr, "Allocator is null!");
            m_allocator->Free(m_ptr);
        }
    }

    T* operator->() const { return m_ptr; }

    T& operator*() const { return *m_ptr; }

    T* Get() const { return m_ptr; }
    [[nodiscard]] AllocatorBase* GetAllocator() const { return m_allocator; }

    void Reset(T* ptr, Allocator* allocator = nullptr)
    {
        if (m_ptr != nullptr)
        {
            m_ptr->~T();
            OPAL_ASSERT(m_allocator != nullptr, "Allocator is null!");
            m_allocator->Free(m_ptr);
        }
        m_ptr = ptr;
        m_allocator = allocator;
    }

    T* Release()
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    [[nodiscard]] bool IsValid() const { return m_ptr != nullptr; }

    explicit operator bool() const { return IsValid(); }

    bool operator==(const ScopePtr& other) const { return m_ptr == other.m_ptr; }
    bool operator==(const T* ptr) const { return m_ptr == ptr; }
    bool operator==(nullptr_t) const { return m_ptr == nullptr; }

private:
    T* m_ptr;
    AllocatorBase* m_allocator;
};

/**
 * Allocates memory using the provided allocator and constructs an object of type T in that memory.
 * @tparam T Type of the object to be constructed.
 * @tparam Allocator Type of the allocator to be used for memory allocation.
 * @tparam Args Types of the arguments to be passed to the constructor of the object.
 * @param allocator Allocator to be used for memory allocation.
 * @param args Arguments to be passed to the constructor of the object.
 * @return A unique pointer to the object of type T.
 */
template <typename T, typename Allocator, typename... Args>
ScopePtr<T> MakeScoped(Allocator* allocator, Args&&... args)
{
    void* memory = nullptr;
    if (allocator == nullptr)
    {
        AllocatorBase* default_allocator = GetDefaultAllocator();
        memory = default_allocator->Alloc(sizeof(T), 8);
    }
    else
    {
        memory = allocator->Alloc(sizeof(T), 8);
    }
    return Opal::ScopePtr<T>{new (memory) T(std::forward<Args>(args)...), allocator};
}

/**
 * Allocates memory using MallocAllocator and constructs an object of type T in that memory.
 * @tparam T Type of the object to be constructed.
 * @tparam Args Types of the arguments to be passed to the constructor of T.
 * @param args Arguments to be passed to the constructor of T.
 * @return A unique pointer to the object of type T.
 */
template <typename T, typename... Args>
ScopePtr<T> MakeDefaultScoped(Args&&... args)
{
    void* memory = nullptr;
    AllocatorBase* default_allocator = GetDefaultAllocator();
    memory = default_allocator->Alloc(sizeof(T), 8);
    return Opal::ScopePtr<T>{new (memory) T(std::forward<Args>(args)...), default_allocator};
}

}  // namespace Opal

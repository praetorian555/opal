#pragma once

#include <atomic>

#include "opal/allocator.h"
#include "opal/exceptions.h"

namespace Opal
{

enum class ThreadingPolicy : u8
{
    ThreadSafe,
    SingleThread,
};

namespace Impl
{

template <ThreadingPolicy Policy>
struct RefCountType;

template <>
struct RefCountType<ThreadingPolicy::ThreadSafe>
{
    using Type = std::atomic<size_t>;

    static void Store(Type* refcount, size_t value) { refcount->store(value, std::memory_order_relaxed); }
    static void Increment(Type* refcount) { refcount->fetch_add(1, std::memory_order_relaxed); }
    static size_t DecrementAndGet(Type* refcount) { return refcount->fetch_sub(1, std::memory_order_acq_rel); }
};

template <>
struct RefCountType<ThreadingPolicy::SingleThread>
{
    using Type = size_t;

    static void Store(Type* refcount, size_t value) { *refcount = value; }
    static void Increment(Type* refcount) { ++(*refcount); }
    static size_t DecrementAndGet(Type* refcount)
    {
        const size_t prev = *refcount;
        --(*refcount);
        return prev;
    }
};

}  // namespace Impl

/**
 * Reference counting wrapper around objects of the desired type.
 *
 * The wrapped object is constructed when the shared pointer is created. When the last shared pointer
 * that refers to the same object is destroyed or reset, the object is destroyed as well.
 *
 * Copy construction and copy assignment are deleted. Use Clone() to create a new shared pointer that
 * shares ownership. Move construction and move assignment are supported.
 *
 * @tparam T Object type stored inside the shared pointer.
 * @tparam Policy Threading policy that controls how reference counting is performed. When set to
 *         ThreadingPolicy::ThreadSafe (the default), the reference count is an atomic variable and
 *         the allocator must be thread-safe. When set to ThreadingPolicy::SingleThread, the reference
 *         count is a plain integer, avoiding the overhead of atomic operations.
 */
template <typename T, ThreadingPolicy Policy = ThreadingPolicy::ThreadSafe>
class SharedPtr
{
    using RefCountOps = Impl::RefCountType<Policy>;
    using RefCountT = typename RefCountOps::Type;

public:
    /** Default constructor. Creates an invalid shared pointer that holds no object. */
    SharedPtr() = default;

    /**
     * Constructs a shared pointer and the managed object in-place.
     * @param allocator Allocator used for the object and reference count. If nullptr, the default allocator is used.
     * @param args Arguments forwarded to the constructor of T.
     * @throws InvalidArgumentException If Policy is ThreadSafe and the allocator is not thread-safe.
     */
    template <typename... Args>
    SharedPtr(AllocatorBase* allocator, Args&&... args)
    {
        if (allocator == nullptr)
        {
            allocator = GetDefaultAllocator();
        }
        if constexpr (Policy == ThreadingPolicy::ThreadSafe)
        {
            if (!allocator->IsThreadSafe())
            {
                throw InvalidArgumentException("SharedPtr", "Allocator should be thread-safe");
            }
        }
        m_object = New<T>(allocator, std::forward<Args>(args)...);
        m_refcount = New<RefCountT>(allocator);
        RefCountOps::Store(m_refcount, 1);
        m_allocator = allocator;
    }

    /**
     * Constructs a shared pointer that takes ownership of an existing raw pointer.
     * If the object pointer is nullptr, the shared pointer is left in an invalid state.
     * @param allocator Allocator used for the reference count and for destroying the object. If nullptr, the default
     *        allocator is used.
     * @param object Raw pointer to the object to manage. Must have been allocated with the same allocator.
     * @throws InvalidArgumentException If Policy is ThreadSafe and the allocator is not thread-safe.
     */
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
        if constexpr (Policy == ThreadingPolicy::ThreadSafe)
        {
            if (!allocator->IsThreadSafe())
            {
                throw InvalidArgumentException("SharedPtr", "Allocator should be thread-safe");
            }
        }
        m_object = object;
        m_refcount = New<RefCountT>(allocator);
        RefCountOps::Store(m_refcount, 1);
        m_allocator = allocator;
    }

    /** Destructor. Decrements the reference count and destroys the managed object if this was the last owner. */
    ~SharedPtr() { Reset(); }

    SharedPtr(const SharedPtr&) = delete;
    SharedPtr& operator=(const SharedPtr&) = delete;

    /** Move constructor. Transfers ownership from other, leaving other in an invalid state. */
    SharedPtr(SharedPtr&& other) noexcept : m_object(other.m_object), m_allocator(other.m_allocator), m_refcount(other.m_refcount)
    {
        other.m_object = nullptr;
        other.m_allocator = nullptr;
        other.m_refcount = nullptr;
    }

    /**
     * Converting move constructor. Transfers ownership from a SharedPtr of a convertible type.
     * Both shared pointers must use the same threading policy.
     * @tparam U Source type, must be convertible to T*.
     */
    template <typename U>
        requires Convertible<U*, T*>
    explicit SharedPtr(SharedPtr<U, Policy>&& other) noexcept
        : m_object(static_cast<T*>(other.m_object)), m_allocator(other.m_allocator), m_refcount(other.m_refcount)
    {
        other.m_object = nullptr;
        other.m_allocator = nullptr;
        other.m_refcount = nullptr;
    }

    /** Move assignment operator. Releases the current object (if any) and transfers ownership from other. */
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

    /** Returns a raw pointer to the managed object. */
    T* operator->() { return m_object; }
    /** Returns a const raw pointer to the managed object. */
    const T* operator->() const { return m_object; }

    /**
     * Creates a new SharedPtr that shares ownership of the managed object. The reference count is incremented.
     * If this shared pointer is invalid, the returned clone is also invalid.
     * @return A new SharedPtr that points to the same object.
     */
    SharedPtr Clone() const
    {
        SharedPtr clone;
        if (!IsValid())
        {
            return clone;
        }
        clone.m_object = m_object;
        clone.m_refcount = m_refcount;
        RefCountOps::Increment(m_refcount);
        clone.m_allocator = m_allocator;
        return clone;
    }

    /**
     * Releases ownership of the managed object. If this was the last owner, the object is destroyed.
     * After this call, the shared pointer is in an invalid state.
     */
    void Reset()
    {
        if (m_refcount != nullptr && RefCountOps::DecrementAndGet(m_refcount) == 1)
        {
            Delete(m_allocator, m_object);
            Delete(m_allocator, m_refcount);
        }
        m_object = nullptr;
        m_refcount = nullptr;
        m_allocator = nullptr;
    }

    /**
     * Checks whether this shared pointer manages a valid object.
     * @return True if the managed object pointer is not nullptr.
     */
    [[nodiscard]] bool IsValid() const { return m_object != nullptr; }

    /**
     * Compares two shared pointers for equality based on the managed object pointer.
     * Two shared pointers are equal if they point to the same object.
     */
    bool operator==(const SharedPtr& other) const { return m_object == other.m_object; }

    /** Returns a raw pointer to the managed object. */
    T* Get() { return m_object; }
    /** Returns a const raw pointer to the managed object. */
    const T* Get() const { return m_object; }

    /** Returns a Ref wrapper around the managed object. */
    Ref<T> GetRef() { return Ref<T>(m_object); }
    /** Returns a const Ref wrapper around the managed object. */
    Ref<const T> GetRef() const { return Ref<const T>(m_object); }

private:
    template <typename U, ThreadingPolicy P>
    friend class SharedPtr;

    T* m_object = nullptr;
    AllocatorBase* m_allocator = nullptr;
    RefCountT* m_refcount = nullptr;
};

/**
 * Constructs an object of type Derived and returns a SharedPtr<Base>.
 * Useful for creating shared pointers to polymorphic types without manual New/cast.
 * @tparam Base Base type for the returned SharedPtr.
 * @tparam Derived Derived type to construct. Must be convertible to Base*. Defaults to Base.
 * @tparam Policy Threading policy for the SharedPtr.
 * @param allocator Allocator used for the object and reference count. If nullptr, the default allocator is used.
 * @param args Arguments forwarded to the constructor of Derived.
 * @return SharedPtr<Base, Policy> owning the newly constructed Derived object.
 */
template <typename Base, typename Derived = Base, ThreadingPolicy Policy = ThreadingPolicy::ThreadSafe, typename... Args>
    requires Convertible<Derived*, Base*>
SharedPtr<Base, Policy> MakeShared(AllocatorBase* allocator, Args&&... args)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    Derived* object = New<Derived>(allocator, std::forward<Args>(args)...);
    return SharedPtr<Base, Policy>(allocator, static_cast<Base*>(object));
}

}  // namespace Opal

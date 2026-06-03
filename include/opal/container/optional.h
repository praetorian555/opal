#pragma once

#include <new>

#include "opal/assert.h"
#include "opal/common.h"
#include "opal/hash.h"
#include "opal/type-traits.h"

namespace Opal
{

/**
 * Tag type used to request in-place construction of the contained value.
 * Use the k_in_place constant: Optional<T>(k_in_place, args...).
 */
struct InPlaceTag
{
    explicit InPlaceTag() = default;
};
inline constexpr InPlaceTag k_in_place{};

/**
 * Tag type used to explicitly construct an empty Optional or to reset one via assignment.
 * Use the k_null_opt constant: Optional<T> value = k_null_opt;
 */
struct NullOptTag
{
    explicit NullOptTag() = default;
};
inline constexpr NullOptTag k_null_opt{};

/**
 * Container that holds either a single value of type T or nothing. When empty no instance of T is
 * constructed and no heap allocation is performed; storage for T lives inline inside the Optional.
 *
 * Copy is deleted following the project's "Clone instead of Copy" pattern; use Clone() for a deep
 * copy or Move() to transfer.
 */
template <typename T>
class Optional
{
public:
    using ValueType = T;

    Optional();
    Optional(NullOptTag);
    Optional(const T& value);
    Optional(T&& value);

    template <typename... Args>
    explicit Optional(InPlaceTag, Args&&... args);

    Optional(const Optional& other) = delete;
    Optional& operator=(const Optional& other) = delete;

    Optional(Optional&& other) noexcept;
    Optional& operator=(Optional&& other) noexcept;
    Optional& operator=(NullOptTag);

    ~Optional();

    [[nodiscard]] bool HasValue() const;
    explicit operator bool() const;

    T& GetValue() &;
    const T& GetValue() const&;
    T&& GetValue() &&;
    T GetValueOr(const T& default_value) const;

    T& operator*() &;
    const T& operator*() const&;
    T* operator->();
    const T* operator->() const;

    /** Destroys the current value (if any) and constructs a new one in place. Returns the value. */
    template <typename... Args>
    T& Emplace(Args&&... args);

    /** Destroys the contained value (if any) and leaves the Optional empty. */
    void Reset();

    [[nodiscard]] Optional Clone(struct AllocatorBase* allocator = nullptr) const;

    bool operator==(const Optional& other) const;
    bool operator!=(const Optional& other) const;
    bool operator==(NullOptTag) const;
    bool operator!=(NullOptTag) const;

private:
    union
    {
        T m_value;
    };
    bool m_has_value;
};

/**
 * Reference specialization. Holds a non-owning pointer to a T (or nothing). Mirrors Expected<T&>:
 * copy is deleted, move transfers the reference and empties the source.
 */
template <typename T>
class Optional<T&>
{
public:
    using ValueType = T;

    Optional();
    Optional(NullOptTag);
    Optional(T& value);

    Optional(const Optional& other) = delete;
    Optional& operator=(const Optional& other) = delete;

    Optional(Optional&& other) noexcept;
    Optional& operator=(Optional&& other) noexcept;
    Optional& operator=(NullOptTag);

    ~Optional() = default;

    [[nodiscard]] bool HasValue() const;
    explicit operator bool() const;

    T& GetValue() const;
    T& GetValueOr(T& default_value) const;

    T& operator*() const;
    T* operator->() const;

    void Reset();

    [[nodiscard]] Optional Clone(struct AllocatorBase* allocator = nullptr) const;

    bool operator==(const Optional& other) const;
    bool operator!=(const Optional& other) const;
    bool operator==(NullOptTag) const;
    bool operator!=(NullOptTag) const;

private:
    T* m_ptr;
};

/** Hash support so Optional can be used as a HashMap/HashSet key when T is hashable. */
template <typename T>
struct Hasher<Optional<T>>
{
    u64 operator()(const Optional<T>& value) const
    {
        if (!value.HasValue())
        {
            return k_empty_hash;
        }
        return Hasher<T>{}(value.GetValue());
    }

private:
    static constexpr u64 k_empty_hash = 0xa5a5'a5a5'a5a5'a5a5ull;
};

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

template <typename T>
Opal::Optional<T>::Optional() : m_has_value(false)
{
}

template <typename T>
Opal::Optional<T>::Optional(NullOptTag) : m_has_value(false)
{
}

template <typename T>
Opal::Optional<T>::Optional(const T& value) : m_value(value), m_has_value(true)
{
}

template <typename T>
Opal::Optional<T>::Optional(T&& value) : m_value(Move(value)), m_has_value(true)
{
}

template <typename T>
template <typename... Args>
Opal::Optional<T>::Optional(InPlaceTag, Args&&... args) : m_value(static_cast<Args&&>(args)...), m_has_value(true)
{
}

template <typename T>
Opal::Optional<T>::Optional(Optional&& other) noexcept : m_has_value(other.m_has_value)
{
    if (m_has_value)
    {
        new (&m_value) T(Move(other.m_value));
    }
}

template <typename T>
Opal::Optional<T>& Opal::Optional<T>::operator=(Optional&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (other.m_has_value)
    {
        if (m_has_value)
        {
            m_value = Move(other.m_value);
        }
        else
        {
            new (&m_value) T(Move(other.m_value));
            m_has_value = true;
        }
    }
    else
    {
        Reset();
    }
    return *this;
}

template <typename T>
Opal::Optional<T>& Opal::Optional<T>::operator=(NullOptTag)
{
    Reset();
    return *this;
}

template <typename T>
Opal::Optional<T>::~Optional()
{
    if (m_has_value)
    {
        m_value.~T();
    }
}

template <typename T>
bool Opal::Optional<T>::HasValue() const
{
    return m_has_value;
}

template <typename T>
Opal::Optional<T>::operator bool() const
{
    return m_has_value;
}

template <typename T>
T& Opal::Optional<T>::GetValue() &
{
    OPAL_ASSERT(m_has_value, "Optional does not have a value");
    return m_value;
}

template <typename T>
const T& Opal::Optional<T>::GetValue() const&
{
    OPAL_ASSERT(m_has_value, "Optional does not have a value");
    return m_value;
}

template <typename T>
T&& Opal::Optional<T>::GetValue() &&
{
    OPAL_ASSERT(m_has_value, "Optional does not have a value");
    return Move(m_value);
}

template <typename T>
T Opal::Optional<T>::GetValueOr(const T& default_value) const
{
    if (m_has_value)
    {
        return m_value;
    }
    return default_value;
}

template <typename T>
T& Opal::Optional<T>::operator*() &
{
    return GetValue();
}

template <typename T>
const T& Opal::Optional<T>::operator*() const&
{
    return GetValue();
}

template <typename T>
T* Opal::Optional<T>::operator->()
{
    OPAL_ASSERT(m_has_value, "Optional does not have a value");
    return &m_value;
}

template <typename T>
const T* Opal::Optional<T>::operator->() const
{
    OPAL_ASSERT(m_has_value, "Optional does not have a value");
    return &m_value;
}

template <typename T>
template <typename... Args>
T& Opal::Optional<T>::Emplace(Args&&... args)
{
    Reset();
    new (&m_value) T(static_cast<Args&&>(args)...);
    m_has_value = true;
    return m_value;
}

template <typename T>
void Opal::Optional<T>::Reset()
{
    if (m_has_value)
    {
        m_value.~T();
        m_has_value = false;
    }
}

template <typename T>
Opal::Optional<T> Opal::Optional<T>::Clone(AllocatorBase* allocator) const
{
    if (!m_has_value)
    {
        return Optional();
    }
    return Optional(k_in_place, Opal::Clone(m_value, allocator));
}

template <typename T>
bool Opal::Optional<T>::operator==(const Optional& other) const
{
    if (m_has_value != other.m_has_value)
    {
        return false;
    }
    if (!m_has_value)
    {
        return true;
    }
    return m_value == other.m_value;
}

template <typename T>
bool Opal::Optional<T>::operator!=(const Optional& other) const
{
    return !(*this == other);
}

template <typename T>
bool Opal::Optional<T>::operator==(NullOptTag) const
{
    return !m_has_value;
}

template <typename T>
bool Opal::Optional<T>::operator!=(NullOptTag) const
{
    return m_has_value;
}

/** Reference specialization implementation ******************************************************/

template <typename T>
Opal::Optional<T&>::Optional() : m_ptr(nullptr)
{
}

template <typename T>
Opal::Optional<T&>::Optional(NullOptTag) : m_ptr(nullptr)
{
}

template <typename T>
Opal::Optional<T&>::Optional(T& value) : m_ptr(&value)
{
}

template <typename T>
Opal::Optional<T&>::Optional(Optional&& other) noexcept : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

template <typename T>
Opal::Optional<T&>& Opal::Optional<T&>::operator=(Optional&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
    return *this;
}

template <typename T>
Opal::Optional<T&>& Opal::Optional<T&>::operator=(NullOptTag)
{
    m_ptr = nullptr;
    return *this;
}

template <typename T>
bool Opal::Optional<T&>::HasValue() const
{
    return m_ptr != nullptr;
}

template <typename T>
Opal::Optional<T&>::operator bool() const
{
    return m_ptr != nullptr;
}

template <typename T>
T& Opal::Optional<T&>::GetValue() const
{
    OPAL_ASSERT(m_ptr != nullptr, "Optional does not have a value");
    return *m_ptr;
}

template <typename T>
T& Opal::Optional<T&>::GetValueOr(T& default_value) const
{
    if (m_ptr != nullptr)
    {
        return *m_ptr;
    }
    return default_value;
}

template <typename T>
T& Opal::Optional<T&>::operator*() const
{
    return GetValue();
}

template <typename T>
T* Opal::Optional<T&>::operator->() const
{
    OPAL_ASSERT(m_ptr != nullptr, "Optional does not have a value");
    return m_ptr;
}

template <typename T>
void Opal::Optional<T&>::Reset()
{
    m_ptr = nullptr;
}

template <typename T>
Opal::Optional<T&> Opal::Optional<T&>::Clone(AllocatorBase*) const
{
    Optional result;
    result.m_ptr = m_ptr;
    return result;
}

template <typename T>
bool Opal::Optional<T&>::operator==(const Optional& other) const
{
    return m_ptr == other.m_ptr;
}

template <typename T>
bool Opal::Optional<T&>::operator!=(const Optional& other) const
{
    return m_ptr != other.m_ptr;
}

template <typename T>
bool Opal::Optional<T&>::operator==(NullOptTag) const
{
    return m_ptr == nullptr;
}

template <typename T>
bool Opal::Optional<T&>::operator!=(NullOptTag) const
{
    return m_ptr != nullptr;
}
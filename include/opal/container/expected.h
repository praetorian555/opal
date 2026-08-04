#pragma once

#include <new>
#include <utility>

#include "opal/assert.h"
#include "opal/type-traits.h"

namespace Opal
{

/**
 * Holds either a value or an error, decided at construction. There is no empty state and no default constructor: an Expected that
 * holds neither would report success with a value nobody produced.
 */
template <typename T, typename E>
class Expected
{
public:
    Expected() = delete;
    explicit Expected(T value);
    explicit Expected(E error);
    Expected(Expected&& other) noexcept;

    ~Expected();

    Expected& operator=(Expected&& other) noexcept;

    [[nodiscard]] bool HasValue() const;

    /** @note Asserts when there is no value. Check HasValue first, or use GetValueOr. */
    T& GetValue() &;
    const T& GetValue() const &;
    T&& GetValue() &&;
    T GetValueOr(const T& default_value) const;

    /** @note Asserts when there is no error. Check HasValue first, or use GetErrorOr. */
    E& GetError();
    const E& GetError() const;
    E GetErrorOr(const E& default_error) const;

    /**
     * Call @p func with the value and return what it returns, or pass the error through untouched.
     * @param func Takes the value by rvalue and returns an Expected with the same error type.
     */
    template <typename Func>
    auto AndThen(Func&& func) &&;

    /**
     * Call @p func with the value and wrap the result in an Expected, or pass the error through untouched.
     * @param func Takes the value by rvalue and returns a plain value.
     */
    template <typename Func>
    auto Map(Func&& func) &&;

private:
    union
    {
        T m_value;
        E m_error;
    };
    bool m_has_value;
};

template <typename T, typename E>
class Expected<T&, E>
{
public:
    Expected() = delete;
    explicit Expected(T& value);
    explicit Expected(E error);
    Expected(Expected&& other) noexcept;

    ~Expected();

    Expected& operator=(Expected&& other) noexcept;

    [[nodiscard]] bool HasValue() const;

    /** @note Asserts when there is no value. Check HasValue first, or use GetValueOr. */
    T& GetValue();
    const T& GetValue() const;
    T GetValueOr(const T& default_value) const;

    /** @note Asserts when there is no error. Check HasValue first, or use GetErrorOr. */
    E& GetError();
    const E& GetError() const;
    E GetErrorOr(const E& default_error) const;

private:
    union
    {
        T* m_value;
        E m_error;
    };
    bool m_has_value;
};

}  // namespace Opal

/** Implementation *******************************************************************************/

template <typename T, typename E>
Opal::Expected<T, E>::Expected(T value) : m_value(std::move(value)), m_has_value(true)
{
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(E error) : m_error(std::move(error)), m_has_value(false)
{
}

template <typename T, typename E>
Opal::Expected<T&, E>::Expected(T& value) : m_value(&value), m_has_value(true)
{
}

template <typename T, typename E>
Opal::Expected<T&, E>::Expected(E error) : m_error(std::move(error)), m_has_value(false)
{
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(Expected&& other) noexcept : m_has_value(other.m_has_value)
{
    // Placement new, not assignment: neither union member has been constructed yet, and assigning over one runs T's or E's
    // assignment operator on storage it never owned.
    if (other.m_has_value)
    {
        new (&m_value) T(Move(other.m_value));
    }
    else
    {
        new (&m_error) E(Move(other.m_error));
    }
}

template <typename T, typename E>
Opal::Expected<T&, E>::Expected(Expected&& other) noexcept : m_has_value(other.m_has_value)
{
    if (other.m_has_value)
    {
        m_value = other.m_value;
        other.m_value = nullptr;
    }
    else
    {
        new (&m_error) E(Move(other.m_error));
    }
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected::~Expected()
{
    if (m_has_value)
    {
        m_value.~T();
    }
    else
    {
        m_error.~E();
    }
}

template <typename T, typename E>
Opal::Expected<T&, E>::Expected::~Expected()
{
    if (!m_has_value)
    {
        m_error.~E();
    }
}

template <typename T, typename E>
Opal::Expected<T, E>& Opal::Expected<T, E>::operator=(Expected&& other) noexcept
{
    if (other.m_has_value)
    {
        if (m_has_value)
        {
            m_value = Move(other.m_value);
        }
        else
        {
            m_error.~E();
            new (&m_value) T(Move(other.m_value));
        }
    }
    else
    {
        if (!m_has_value)
        {
            m_error = Move(other.m_error);
        }
        else
        {
            m_value.~T();
            new (&m_error) E(Move(other.m_error));
        }
    }
    m_has_value = other.m_has_value;
    return *this;
}

template <typename T, typename E>
Opal::Expected<T&, E>& Opal::Expected<T&, E>::operator=(Expected&& other) noexcept
{
    if (other.m_has_value)
    {
        if (m_has_value)
        {
            m_value = other.m_value;
        }
        else
        {
            m_error.~E();
            m_value = other.m_value;
        }
    }
    else
    {
        if (!m_has_value)
        {
            m_error = Move(other.m_error);
        }
        else
        {
            new (&m_error) E(Move(other.m_error));
        }
    }
    m_has_value = other.m_has_value;
    return *this;
}

template <typename T, typename E>
bool Opal::Expected<T, E>::HasValue() const
{
    return m_has_value;
}

template <typename T, typename E>
bool Opal::Expected<T&, E>::HasValue() const
{
    return m_has_value;
}

template <typename T, typename E>
T& Opal::Expected<T, E>::GetValue() &
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    return m_value;
}

template <typename T, typename E>
T& Opal::Expected<T&, E>::GetValue()
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    return *m_value;
}

template <typename T, typename E>
const T& Opal::Expected<T, E>::GetValue() const &
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    return m_value;
}

template <typename T, typename E>
const T& Opal::Expected<T&, E>::GetValue() const
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    return *m_value;
}

template <typename T, typename E>
T&& Opal::Expected<T, E>::GetValue() &&
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    return std::move(m_value);
}

template <typename T, typename E>
T Opal::Expected<T, E>::GetValueOr(const T& default_value) const
{
    if (m_has_value)
    {
        return GetValue();
    }
    return default_value;
}

template <typename T, typename E>
T Opal::Expected<T&, E>::GetValueOr(const T& default_value) const
{
    if (m_has_value)
    {
        return GetValue();
    }
    return default_value;
}

template <typename T, typename E>
const E& Opal::Expected<T, E>::GetError() const
{
    OPAL_ASSERT(!m_has_value, "Expected does not have an error");
    return m_error;
}

template <typename T, typename E>
const E& Opal::Expected<T&, E>::GetError() const
{
    OPAL_ASSERT(!m_has_value, "Expected does not have an error");
    return m_error;
}

template <typename T, typename E>
E& Opal::Expected<T, E>::GetError()
{
    OPAL_ASSERT(!m_has_value, "Expected does not have an error");
    return m_error;
}

template <typename T, typename E>
E& Opal::Expected<T&, E>::GetError()
{
    OPAL_ASSERT(!m_has_value, "Expected does not have an error");
    return m_error;
}

template <typename T, typename E>
E Opal::Expected<T, E>::GetErrorOr(const E& default_error) const
{
    if (m_has_value)
    {
        return default_error;
    }
    return m_error;
}

template <typename T, typename E>
E Opal::Expected<T&, E>::GetErrorOr(const E& default_error) const
{
    if (m_has_value)
    {
        return default_error;
    }
    return m_error;
}

template <typename T, typename E>
template <typename Func>
auto Opal::Expected<T, E>::AndThen(Func&& func) &&
{
    using ResultType = decltype(func(Move(m_value)));
    if (!m_has_value)
    {
        return ResultType(Move(m_error));
    }
    return func(Move(m_value));
}

template <typename T, typename E>
template <typename Func>
auto Opal::Expected<T, E>::Map(Func&& func) &&
{
    using ResultType = Expected<decltype(func(Move(m_value))), E>;
    if (!m_has_value)
    {
        return ResultType(Move(m_error));
    }
    return ResultType(func(Move(m_value)));
}

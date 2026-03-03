#pragma once

#include <utility>

#include "opal/assert.h"
#include "opal/type-traits.h"

namespace Opal
{

template <typename T, typename E>
class Expected
{
public:
    Expected();
    explicit Expected(T value);
    explicit Expected(E error);
    Expected(Expected&& other) noexcept;

    ~Expected();

    Expected& operator=(Expected&& other) noexcept;

    [[nodiscard]] bool HasValue() const;
    T& GetValue();
    const T& GetValue() const;
    T GetValueOr(const T& default_value) const;

    E& GetError();
    const E& GetError() const;

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
    T& GetValue();
    const T& GetValue() const;
    T GetValueOr(const T& default_value) const;

    E& GetError();
    const E& GetError() const;

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
Opal::Expected<T, E>::Expected() : m_value(), m_has_value(true)
{
}

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
    if (other.m_has_value)
    {
        m_value = Move(other.m_value);
    }
    else
    {
        m_error = Move(other.m_error);
    }
}

template <typename T, typename E>
Opal::Expected<T&, E>::Expected(Expected&& other) noexcept : m_has_value(other.m_has_value)
{
    if (other.m_has_value)
    {
        m_value = Move(other.m_value);
        other.m_value = nullptr;
    }
    else
    {
        m_error = Move(other.m_error);
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
T& Opal::Expected<T, E>::GetValue()
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
const T& Opal::Expected<T, E>::GetValue() const
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

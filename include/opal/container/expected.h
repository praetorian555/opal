#pragma once

#include "opal/assert.h"
#include "opal/type-traits.h"

namespace Opal
{

template <typename T, typename E>
class Expected
{
public:
    static constexpr bool k_is_reference_value = Opal::k_is_reference_value<T>;
    using PrivateValueType = ConditionalType<k_is_reference_value, Opal::RemoveReferenceType<T>*, T>;
    using PublicValueType = ConditionalType<k_is_reference_value, T, T&>;

    Expected();
    explicit Expected(const T& value);
    explicit Expected(const E& error);
    Expected(const Expected& other);
    Expected(Expected&& other) noexcept;

    ~Expected();

    Expected& operator=(const Expected& other);
    Expected& operator=(Expected&& other) noexcept;

    bool HasValue() const;
    PublicValueType GetValue();
    const PublicValueType GetValue() const;
    T GetValueOr(const T& default_value) const;

    E& GetError();
    const E& GetError() const;

private:
    union
    {
        PrivateValueType m_value;
        E m_error;
    };
    bool m_has_value;
};

}  // namespace Opal

/** Implementation *******************************************************************************/

template <typename T, typename E>
Opal::Expected<T, E>::Expected()
{
    if constexpr (k_is_reference_value)
    {
        new (&m_error) E();
        m_has_value = false;
    }
    else
    {
        new (&m_value) T();
        m_has_value = true;
    }
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(const T& value)
{
    if constexpr (k_is_reference_value)
    {
        m_value = &value;
    }
    else
    {
        m_value = value;
    }
    m_has_value = true;
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(const E& error) : m_error(error), m_has_value(false)
{
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(const Expected& other)
{
    if (other.m_has_value)
    {
        m_value = other.m_value;
    }
    else
    {
        m_error = other.m_error;
    }
    m_has_value = other.m_has_value;
}

template <typename T, typename E>
Opal::Expected<T, E>::Expected(Expected&& other) noexcept
{
    if (other.m_has_value)
    {
        m_value = Move(other.m_value);
        if constexpr (k_is_reference_value)
        {
            other.m_value = nullptr;
        }
        else
        {
            other.m_value = PrivateValueType();
        }
    }
    else
    {
        m_error = Move(other.m_error);
    }
    m_has_value = other.m_has_value;
    other.m_has_value = !k_is_reference_value;
}

template <typename T, typename E>
Opal::Expected<T, E>::~Expected()
{
    if (m_has_value || (!m_has_value && k_is_reference_value))
    {
        if constexpr (!k_is_reference_value)
        {
            m_value.~T();
        }
    }
    else
    {
        m_error.~E();
    }
}

template <typename T, typename E>
Opal::Expected<T, E>& Opal::Expected<T, E>::operator=(const Expected& other)
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
            if constexpr (k_is_reference_value)
            {
                m_value = other.m_value;
            }
            else
            {
                new (&m_value) T(other.m_value);
            }
        }
    }
    else
    {
        if (m_has_value)
        {
            if constexpr (k_is_reference_value)
            {
                m_value = nullptr;
            }
            else
            {
                m_value.~T();
            }
            new (&m_error) E(other.m_error);
        }
        else
        {
            m_error = other.m_error;
        }
    }
    m_has_value = other.m_has_value;
    return *this;
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
            if constexpr (k_is_reference_value)
            {
                m_value = Move(other.m_value);
                other.m_value = nullptr;
            }
            else
            {
                new (&m_value) T(Move(other.m_value));
                other.m_value = T();
            }
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
            if constexpr (k_is_reference_value)
            {
                m_value = nullptr;
            }
            else
            {
                m_value.~T();
            }
            new (&m_error) E(Move(other.m_error));
        }
    }
    m_has_value = other.m_has_value;
    other.m_has_value = !k_is_reference_value;
    return *this;
}

template <typename T, typename E>
bool Opal::Expected<T, E>::HasValue() const
{
    return m_has_value;
}

template <typename T, typename E>
Opal::Expected<T, E>::PublicValueType Opal::Expected<T, E>::GetValue()
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    if constexpr (k_is_reference_value)
    {
        return *m_value;
    }
    else
    {
        return m_value;
    }
}

template <typename T, typename E>
const typename Opal::Expected<T, E>::PublicValueType Opal::Expected<T, E>::GetValue() const
{
    OPAL_ASSERT(m_has_value, "Expected does not have a value");
    if constexpr (k_is_reference_value)
    {
        return *m_value;
    }
    else
    {
        return m_value;
    }
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
const E& Opal::Expected<T, E>::GetError() const
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

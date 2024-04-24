#pragma once

#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename T>
class Span
{
public:
    using SizeType = u64;

    Span() = default;

    template <typename InputIt>
    Span(InputIt first, SizeType count);

    template <typename InputIt>
    Span(InputIt first, InputIt last);

    template <u64 N>
    Span(T (&array)[N]);

    Span(const Span& other) = default;
    Span(Span&& other) noexcept = default;

    ~Span() = default;

    Span& operator=(const Span& other) = default;
    Span& operator=(Span&& other) noexcept = default;

    bool operator==(const Span& other) const;

    T* GetData() { return m_data; }
    [[nodiscard]] const T* GetData() const { return m_data; }

    [[nodiscard]] SizeType GetSize() const { return m_size; }

    Expected<T&, ErrorCode> At(SizeType index);
    Expected<const T&, ErrorCode> At(SizeType index) const;

    T& operator[](SizeType index) { return m_data[index]; }
    const T& operator[](SizeType index) const { return m_data[index]; }

    Expected<T&, ErrorCode> Front();
    Expected<const T&, ErrorCode> Front() const;

    Expected<T&, ErrorCode> Back();
    Expected<const T&, ErrorCode> Back() const;

    Expected<Span<T>, ErrorCode> SubSpan(SizeType offset, SizeType count) const;

private:
    T* m_data = nullptr;
    SizeType m_size = 0;
};

template <typename T>
Span<const u8> AsBytes(Span<T> span);

template <typename T>
Span<u8> AsWritableBytes(Span<T> span);

}  // namespace Opal

template <typename T>
template <typename InputIt>
Opal::Span<T>::Span(InputIt first, SizeType count) : m_data(&(*first)), m_size(count)
{
}

template <typename T>
template <typename InputIt>
Opal::Span<T>::Span(InputIt first, InputIt last) : m_size(last - first), m_data(&(*first))
{
}

template <typename T>
template <Opal::u64 N>
Opal::Span<T>::Span(T (&array)[N]) : m_data(array), m_size(N)
{
}

template <typename T>
bool Opal::Span<T>::operator==(const Span& other) const
{
    return m_data == other.m_data && m_size == other.m_size;
}

template <typename T>
Opal::Expected<T&, Opal::ErrorCode> Opal::Span<T>::At(SizeType index)
{
    if (index >= m_size)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[index]);
}

template <typename T>
Opal::Expected<const T&, Opal::ErrorCode> Opal::Span<T>::At(SizeType index) const
{
    if (index >= m_size)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[index]);
}

template <typename T>
Opal::Expected<T&, Opal::ErrorCode> Opal::Span<T>::Front()
{
    if (m_size == 0)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[0]);
}

template <typename T>
Opal::Expected<const T&, Opal::ErrorCode> Opal::Span<T>::Front() const
{
    if (m_size == 0)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[0]);
}

template <typename T>
Opal::Expected<T&, Opal::ErrorCode> Opal::Span<T>::Back()
{
    if (m_size == 0)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[m_size - 1]);
}

template <typename T>
Opal::Expected<const T&, Opal::ErrorCode> Opal::Span<T>::Back() const
{
    if (m_size == 0)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[m_size - 1]);
}

template <typename T>
Opal::Span<const Opal::u8> Opal::AsBytes(Span<T> span)
{
    return Span<const u8>(reinterpret_cast<const u8*>(span.GetData()), span.GetSize() * sizeof(T));
}

template <typename T>
Opal::Span<Opal::u8> Opal::AsWritableBytes(Opal::Span<T> span)
{
    return Span<u8>(reinterpret_cast<u8*>(span.GetData()), span.GetSize() * sizeof(T));
}

template <typename T>
Opal::Expected<Opal::Span<T>, Opal::ErrorCode> Opal::Span<T>::SubSpan(SizeType offset, SizeType count) const
{
    if (offset + count > m_size)
    {
        return Expected<Span<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<Span<T>, ErrorCode>(Span<T>(m_data + offset, count));
}

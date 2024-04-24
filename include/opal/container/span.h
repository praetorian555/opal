#pragma once

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

    T* GetData() { return m_data; }
    const T* GetData() const { return m_data; }

    [[nodiscard]] SizeType GetSize() const { return m_size; }

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
Opal::Span<const Opal::u8> Opal::AsBytes(Span<T> span)
{
    return Span<const u8>(reinterpret_cast<const u8*>(span.GetData()), span.GetSize() * sizeof(T));
}

template <typename T>
Opal::Span<Opal::u8> Opal::AsWritableBytes(Opal::Span<T> span)
{
    return Span<u8>(reinterpret_cast<u8*>(span.GetData()), span.GetSize() * sizeof(T));
}

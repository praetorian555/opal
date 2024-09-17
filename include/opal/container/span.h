#pragma once

#include "opal/container/expected.h"
#include "opal/container/iterator.h"
#include "opal/error-codes.h"
#include "opal/types.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wsign-conversion)

namespace Opal
{

template <typename MySpan>
class SpanIterator
{
public:
    using value_type = typename MySpan::value_type;
    using difference_type = typename MySpan::difference_type;
    using reference = typename MySpan::reference;
    using pointer = typename MySpan::pointer;

    SpanIterator() = default;
    explicit SpanIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const SpanIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const SpanIterator& other) const;
    bool operator>=(const SpanIterator& other) const;
    bool operator<(const SpanIterator& other) const;
    bool operator<=(const SpanIterator& other) const;

    SpanIterator& operator++();
    SpanIterator operator++(int);
    SpanIterator& operator--();
    SpanIterator operator--(int);

    SpanIterator operator+(difference_type n) const;
    SpanIterator operator-(difference_type n) const;
    SpanIterator& operator+=(difference_type n);
    SpanIterator& operator-=(difference_type n);

    difference_type operator-(const SpanIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MySpan>
SpanIterator<MySpan> operator+(typename SpanIterator<MySpan>::difference_type n, const SpanIterator<MySpan>& it);

template <typename MySpan>
class SpanConstIterator
{
public:
    using value_type = typename MySpan::value_type;
    using difference_type = typename MySpan::difference_type;
    using reference = typename MySpan::reference;
    using pointer = typename MySpan::pointer;

    SpanConstIterator() = default;
    explicit SpanConstIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const SpanConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const SpanConstIterator& other) const;
    bool operator>=(const SpanConstIterator& other) const;
    bool operator<(const SpanConstIterator& other) const;
    bool operator<=(const SpanConstIterator& other) const;

    SpanConstIterator& operator++();
    SpanConstIterator operator++(int);
    SpanConstIterator& operator--();
    SpanConstIterator operator--(int);

    SpanConstIterator operator+(difference_type n) const;
    SpanConstIterator operator-(difference_type n) const;
    SpanConstIterator& operator+=(difference_type n);
    SpanConstIterator& operator-=(difference_type n);

    difference_type operator-(const SpanConstIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MySpan>
SpanConstIterator<MySpan> operator+(typename SpanConstIterator<MySpan>::difference_type n, const SpanConstIterator<MySpan>& it);

template <typename T>
class Span
{
public:
    using value_type = T;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = SpanIterator<Span<T>>;
    using const_iterator = SpanConstIterator<Span<T>>;

    Span() = default;

    template <typename InputIt>
    Span(InputIt first, size_type count);

    template <typename InputIt>
    Span(InputIt first, InputIt last);

    template <u64 N>
    Span(T (&array)[N]);

    template <typename Container>
        requires Range<Container> && (Opal::SameAs<T, typename Opal::ValueTypeGetter<Container>::Type> ||
                                      Opal::SameAs<typename Opal::RemoveConst<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
    explicit Span(Container& container);

    Span(const Span& other) = default;
    Span(Span&& other) noexcept = default;

    ~Span() = default;

    Span& operator=(const Span& other) = default;
    Span& operator=(Span&& other) noexcept = default;

    bool operator==(const Span& other) const;

    T* GetData() { return m_data; }
    [[nodiscard]] const T* GetData() const { return m_data; }

    [[nodiscard]] size_type GetSize() const { return m_size; }

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    Expected<T&, ErrorCode> At(size_type index);
    [[nodiscard]] Expected<const T&, ErrorCode> At(size_type index) const;

    T& operator[](size_type index) { return m_data[index]; }
    const T& operator[](size_type index) const { return m_data[index]; }

    Expected<T&, ErrorCode> Front();
    [[nodiscard]] Expected<const T&, ErrorCode> Front() const;

    Expected<T&, ErrorCode> Back();
    [[nodiscard]] Expected<const T&, ErrorCode> Back() const;

    [[nodiscard]] Expected<Span<T>, ErrorCode> SubSpan(size_type offset, size_type count) const;

    // Compatible with std::begin and std::end
    iterator begin() { return iterator(m_data); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(m_data); }
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }

    iterator end() { return iterator(m_data + m_size); }
    [[nodiscard]] const_iterator end() const { return const_iterator(m_data + m_size); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(m_data + m_size); }

private:
    T* m_data = nullptr;
    size_type m_size = 0;
};

/**
 * @brief Converts object to a span of bytes.
 * @tparam T Type of object.
 * @param object Object to convert.
 * @return Span of bytes that can't be modified.
 */
template <typename T>
Span<const u8> AsBytes(T& object);

/**
 * @brief Converts object to a span of bytes.
 * @tparam T Type of object.
 * @param object Object to convert.
 * @return Span of bytes that can be modified.
 */
template <typename T>
Span<u8> AsWritableBytes(T& object);

/**
 * @brief Converts container to a span of bytes.
 * @tparam Container Type of container.
 * @param container Container to convert.
 * @return Span of bytes that can't be modified.
 */
template <typename Container>
    requires Opal::Range<Container>
Span<const u8> AsBytes(Container& container);

/**
 * @brief Converts container to a span of bytes.
 * @tparam Container Type of container.
 * @param container Container to convert.
 * @return Span of bytes that can be modified.
 */
template <typename Container>
    requires Opal::Range<Container>
Span<u8> AsWritableBytes(Container& container);

}  // namespace Opal

#define TEMPLATE_HEADER template <typename T>
#define CLASS_HEADER Opal::Span<T>

TEMPLATE_HEADER
template <typename InputIt>
CLASS_HEADER::Span(InputIt first, size_type count) : m_data(&(*first)), m_size(count)
{
}

TEMPLATE_HEADER
template <typename InputIt>
CLASS_HEADER::Span(InputIt first, InputIt last) : m_data(&(*first)), m_size(static_cast<size_type>(last - first))
{
}

TEMPLATE_HEADER
template <Opal::u64 N>
CLASS_HEADER::Span(T (&array)[N]) : m_data(array), m_size(N)
{
}

TEMPLATE_HEADER
template <typename Container>
    requires Opal::Range<Container> && (Opal::SameAs<T, typename Opal::ValueTypeGetter<Container>::Type> ||
                                        Opal::SameAs<typename Opal::RemoveConst<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
CLASS_HEADER::Span(Container& container)
{
    // TODO: Check if underlying array is contiguous
    if (Opal::begin(container) == Opal::end(container))
    {
        m_data = nullptr;
        m_size = 0;
        return;
    }
    m_data = &(*container.begin());
    m_size = static_cast<size_type>(container.end() - container.begin());
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const Span& other) const
{
    return m_data == other.m_data && m_size == other.m_size;
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::At(size_type index)
{
    if (index >= m_size)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::At(size_type index) const
{
    if (index >= m_size)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::Front()
{
    if (m_size == 0)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    if (m_size == 0)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::Back()
{
    if (m_size == 0)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    if (m_size == 0)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[m_size - 1]);
}

template <typename T>
Opal::Span<const Opal::u8> Opal::AsBytes(T& object)
{
    return {reinterpret_cast<const u8*>(&object), sizeof(T)};
}

template <typename T>
Opal::Span<Opal::u8> Opal::AsWritableBytes(T& object)
{
    return {reinterpret_cast<u8*>(&object), sizeof(T)};
}

template <typename Container>
    requires Opal::Range<Container>
Opal::Span<const Opal::u8> Opal::AsBytes(Container& container)
{
    auto data = &(*container.begin());
    Opal::u64 size = sizeof(typename Container::value_type) * (container.end() - container.begin());
    return {reinterpret_cast<const u8*>(data), size};
}

template <typename Container>
    requires Opal::Range<Container>
Opal::Span<Opal::u8> Opal::AsWritableBytes(Container& container)
{
    auto data = &(*container.begin());
    Opal::u64 size = sizeof(typename Container::value_type) * (container.end() - container.begin());
    return {reinterpret_cast<u8*>(data), size};
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::SubSpan(size_type offset, size_type count) const
{
    if (offset + count > m_size)
    {
        return Expected<Span<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<Span<T>, ErrorCode>(Span<T>(m_data + offset, count));
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MySpan>
#define CLASS_HEADER Opal::SpanIterator<MySpan>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const SpanIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const SpanIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const SpanIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const SpanIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    SpanIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    SpanIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return SpanIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return SpanIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const SpanIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename SpanIterator<MySpan>::difference_type n, const SpanIterator<MySpan>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MySpan>
#define CLASS_HEADER Opal::SpanConstIterator<MySpan>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const SpanConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const SpanConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const SpanConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const SpanConstIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    SpanConstIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    SpanConstIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return SpanConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return SpanConstIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const SpanConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename SpanConstIterator<MySpan>::difference_type n, const SpanConstIterator<MySpan>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

OPAL_END_DISABLE_WARNINGS

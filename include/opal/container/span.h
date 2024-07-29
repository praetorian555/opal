#pragma once

#include "opal/container/expected.h"
#include "opal/container/iterator.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MySpan>
class SpanIterator
{
public:
    using ValueType = typename MySpan::ValueType;
    using ReferenceType = typename MySpan::ReferenceType;
    using PointerType = typename MySpan::PointerType;
    using DifferenceType = typename MySpan::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    SpanIterator() = default;
    explicit SpanIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const SpanIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const SpanIterator& other) const;
    bool operator>=(const SpanIterator& other) const;
    bool operator<(const SpanIterator& other) const;
    bool operator<=(const SpanIterator& other) const;

    SpanIterator& operator++();
    SpanIterator operator++(int);
    SpanIterator& operator--();
    SpanIterator operator--(int);

    SpanIterator operator+(DifferenceType n) const;
    SpanIterator operator-(DifferenceType n) const;
    SpanIterator& operator+=(DifferenceType n);
    SpanIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const SpanIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MySpan>
SpanIterator<MySpan> operator+(typename SpanIterator<MySpan>::DifferenceType n, const SpanIterator<MySpan>& it);

template <typename MySpan>
class SpanConstIterator
{
public:
    using ValueType = typename MySpan::ValueType;
    using ReferenceType = typename MySpan::ConstReferenceType;
    using PointerType = typename MySpan::PointerType;
    using DifferenceType = typename MySpan::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    SpanConstIterator() = default;
    explicit SpanConstIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const SpanConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const SpanConstIterator& other) const;
    bool operator>=(const SpanConstIterator& other) const;
    bool operator<(const SpanConstIterator& other) const;
    bool operator<=(const SpanConstIterator& other) const;

    SpanConstIterator& operator++();
    SpanConstIterator operator++(int);
    SpanConstIterator& operator--();
    SpanConstIterator operator--(int);

    SpanConstIterator operator+(DifferenceType n) const;
    SpanConstIterator operator-(DifferenceType n) const;
    SpanConstIterator& operator+=(DifferenceType n);
    SpanConstIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const SpanConstIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MySpan>
SpanConstIterator<MySpan> operator+(typename SpanConstIterator<MySpan>::DifferenceType n, const SpanConstIterator<MySpan>& it);

template <typename T>
class Span
{
public:
    using ValueType = T;
    using ReferenceType = T&;
    using ConstReferenceType = const T&;
    using PointerType = T*;
    using SizeType = u64;
    using DifferenceType = i64;
    using IteratorType = SpanIterator<Span<T>>;
    using ConstIteratorType = SpanConstIterator<Span<T>>;

    using value_type = ValueType;
    using reference = ReferenceType;
    using const_reference = ConstReferenceType;
    using pointer = PointerType;

    Span() = default;

    template <typename InputIt>
    Span(InputIt first, SizeType count);

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

    [[nodiscard]] SizeType GetSize() const { return m_size; }

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    Expected<T&, ErrorCode> At(SizeType index);
    [[nodiscard]] Expected<const T&, ErrorCode> At(SizeType index) const;

    T& operator[](SizeType index) { return m_data[index]; }
    const T& operator[](SizeType index) const { return m_data[index]; }

    Expected<T&, ErrorCode> Front();
    [[nodiscard]] Expected<const T&, ErrorCode> Front() const;

    Expected<T&, ErrorCode> Back();
    [[nodiscard]] Expected<const T&, ErrorCode> Back() const;

    [[nodiscard]] Expected<Span<T>, ErrorCode> SubSpan(SizeType offset, SizeType count) const;

    // Iterators
    IteratorType Begin() { return IteratorType(m_data); }
    IteratorType End() { return IteratorType(m_data + m_size); }
    [[nodiscard]] ConstIteratorType Begin() const { return ConstIteratorType(m_data); }
    [[nodiscard]] ConstIteratorType End() const { return ConstIteratorType(m_data + m_size); }
    [[nodiscard]] ConstIteratorType ConstBegin() const { return ConstIteratorType(m_data); }
    [[nodiscard]] ConstIteratorType ConstEnd() const { return ConstIteratorType(m_data + m_size); }

    // Compatible with std::begin and std::end
    IteratorType begin() { return IteratorType(m_data); }
    IteratorType end() { return IteratorType(m_data + m_size); }
    [[nodiscard]] ConstIteratorType begin() const { return ConstIteratorType(m_data); }
    [[nodiscard]] ConstIteratorType end() const { return ConstIteratorType(m_data + m_size); }

private:
    T* m_data = nullptr;
    SizeType m_size = 0;
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
CLASS_HEADER::Span(InputIt first, SizeType count) : m_data(&(*first)), m_size(count)
{
}

TEMPLATE_HEADER
template <typename InputIt>
CLASS_HEADER::Span(InputIt first, InputIt last) : m_size(last - first), m_data(&(*first))
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
    m_data = &(*container.begin());
    m_size = container.end() - container.begin();
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const Span& other) const
{
    return m_data == other.m_data && m_size == other.m_size;
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::At(SizeType index)
{
    if (index >= m_size)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::At(SizeType index) const
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
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::SubSpan(SizeType offset, SizeType count) const
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
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return SpanIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return SpanIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const SpanIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename SpanIterator<MySpan>::DifferenceType n, const SpanIterator<MySpan>& it)
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
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return SpanConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return SpanConstIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const SpanConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename SpanConstIterator<MySpan>::DifferenceType n, const SpanConstIterator<MySpan>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

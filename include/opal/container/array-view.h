#pragma once

#include "opal/casts.h"
#include "opal/container/expected.h"
#include "opal/container/iterator.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

/*************************************************************************************************/
/** Iterator API *********************************************************************************/
/*************************************************************************************************/

template <typename MySpan>
class ArrayViewIterator
{
public:
    using value_type = typename MySpan::value_type;
    using difference_type = typename MySpan::difference_type;
    using reference = typename MySpan::reference;
    using pointer = typename MySpan::pointer;

    ArrayViewIterator() = default;
    explicit ArrayViewIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const ArrayViewIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const ArrayViewIterator& other) const;
    bool operator>=(const ArrayViewIterator& other) const;
    bool operator<(const ArrayViewIterator& other) const;
    bool operator<=(const ArrayViewIterator& other) const;

    ArrayViewIterator& operator++();
    ArrayViewIterator operator++(int);
    ArrayViewIterator& operator--();
    ArrayViewIterator operator--(int);

    ArrayViewIterator operator+(difference_type n) const;
    ArrayViewIterator operator-(difference_type n) const;
    ArrayViewIterator& operator+=(difference_type n);
    ArrayViewIterator& operator-=(difference_type n);

    difference_type operator-(const ArrayViewIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MySpan>
ArrayViewIterator<MySpan> operator+(typename ArrayViewIterator<MySpan>::difference_type n, const ArrayViewIterator<MySpan>& it);

/*************************************************************************************************/
/** Const Iterator API ***************************************************************************/
/*************************************************************************************************/

template <typename MySpan>
class ArrayViewConstIterator
{
public:
    using value_type = typename MySpan::value_type;
    using difference_type = typename MySpan::difference_type;
    using reference = typename MySpan::reference;
    using pointer = typename MySpan::pointer;

    ArrayViewConstIterator() = default;
    explicit ArrayViewConstIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const ArrayViewConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const ArrayViewConstIterator& other) const;
    bool operator>=(const ArrayViewConstIterator& other) const;
    bool operator<(const ArrayViewConstIterator& other) const;
    bool operator<=(const ArrayViewConstIterator& other) const;

    ArrayViewConstIterator& operator++();
    ArrayViewConstIterator operator++(int);
    ArrayViewConstIterator& operator--();
    ArrayViewConstIterator operator--(int);

    ArrayViewConstIterator operator+(difference_type n) const;
    ArrayViewConstIterator operator-(difference_type n) const;
    ArrayViewConstIterator& operator+=(difference_type n);
    ArrayViewConstIterator& operator-=(difference_type n);

    difference_type operator-(const ArrayViewConstIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MySpan>
ArrayViewConstIterator<MySpan> operator+(typename ArrayViewConstIterator<MySpan>::difference_type n,
                                         const ArrayViewConstIterator<MySpan>& it);

/**
 * Represents a non-owning view of contiguous sequence of elements.
 */
template <typename T>
class ArrayView
{
public:
    using value_type = T;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = ArrayViewIterator<ArrayView<T>>;
    using const_iterator = ArrayViewConstIterator<ArrayView<T>>;

    ArrayView() = default;

    /**
     * Construct a span from an iterator and a count.
     * @tparam InputIt Type of iterator. Must be a random access iterator.
     * @param first Iterator to the first element.
     * @param count Number of elements.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ArrayView(InputIt first, size_type count);

    /**
     * Construct a span from start and end iterators.
     * @tparam InputIt Type of iterator. Must be a random access iterator.
     * @param first Iterator to the first element.
     * @param last Iterator to the element after the last element.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ArrayView(InputIt first, InputIt last);

    /**
     * Construct a span from an array.
     * @tparam N Size of the array.
     * @param array Pointer to the first element of the array.
     */
    template <u64 N>
    ArrayView(T (&array)[N]);

    /**
     * Construct a span from a container.
     * @tparam Container Type of container. Must be a range. The value type of the container must match T, and if T is mutable then value
     * type of the container must be mutable as well.
     * @param container Container to construct the span from.
     */
    template <typename Container>
        requires Range<Container> && (Opal::SameAs<T, typename Opal::ValueTypeGetter<Container>::Type> ||
                                      Opal::SameAs<typename Opal::RemoveConst<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
    ArrayView(Container& container);

    ArrayView(const ArrayView& other) = default;
    ArrayView(ArrayView&& other) noexcept = default;

    ~ArrayView() = default;

    ArrayView& operator=(const ArrayView& other) = default;
    ArrayView& operator=(ArrayView&& other) noexcept = default;

    /**
     * Compare two spans for equality. Two spans are equal if they point to the same data and have the same size.
     * @param other Span to compare with.
     * @return True if the spans are equal, false otherwise.
     */
    bool operator==(const ArrayView& other) const;

    T* GetData() { return m_data; }
    [[nodiscard]] const T* GetData() const { return m_data; }

    [[nodiscard]] size_type GetSize() const { return m_size; }

    /**
     * Check if the span is empty.
     * @return True if the span is empty, false otherwise.
     */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Get a reference to the element at the specified index.
     * @param index Index of the element.
     * @return Reference to the element. If the index is out of bounds, an ErrorCode::OutOfBounds is returned.
     */
    Expected<T&, ErrorCode> At(size_type index);
    [[nodiscard]] Expected<const T&, ErrorCode> At(size_type index) const;

    /**
     * Get a reference to the element at the specified index. No bounds checking is performed.
     * @param index Index of the element.
     * @return Reference to the element.
     */
    T& operator[](size_type index) { return m_data[index]; }
    const T& operator[](size_type index) const { return m_data[index]; }

    /**
     * Get a reference to the first element.
     * @return Reference to the first element. If the span is empty, an ErrorCode::OutOfBounds is returned.
     */
    Expected<T&, ErrorCode> Front();
    [[nodiscard]] Expected<const T&, ErrorCode> Front() const;

    /**
     * Get a reference to the last element.
     * @return Reference to the last element. If the span is empty, an ErrorCode::OutOfBounds is returned.
     */
    Expected<T&, ErrorCode> Back();
    [[nodiscard]] Expected<const T&, ErrorCode> Back() const;

    /**
     * Get a sub span of the span.
     * @param offset Offset of the sub span.
     * @param count Number of elements in the sub span.
     * @return Sub span. If the offset or count are out of bounds, an ErrorCode::OutOfBounds is returned.
     */
    [[nodiscard]] Expected<ArrayView<T>, ErrorCode> SubSpan(size_type offset, size_type count) const;

    /** Iterator API - Compatible with standard library. */

    /**
     * Get an iterator to the first element.
     * @return Iterator to the first element.
     */
    iterator begin() { return iterator(m_data); }

    /**
     * Get a const iterator to the first element.
     * @return Const iterator to the first element.
     */
    [[nodiscard]] const_iterator begin() const { return const_iterator(m_data); }

    /**
     * Get a const iterator to the first element.
     * @return Const iterator to the first element.
     */
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }

    /**
     * Get an iterator to the element after the last element.
     * @return Iterator to the element after the last element.
     */
    iterator end() { return iterator(m_data + m_size); }

    /**
     * Get a const iterator to the element after the last element.
     * @return Const iterator to the element after the last element.
     */
    [[nodiscard]] const_iterator end() const { return const_iterator(m_data + m_size); }

    /**
     * Get a const iterator to the element after the last element.
     * @return Const iterator to the element after the last element.
     */
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
ArrayView<const u8> AsBytes(T& object);

/**
 * @brief Converts object to a span of bytes.
 * @tparam T Type of object.
 * @param object Object to convert.
 * @return Span of bytes that can be modified.
 */
template <typename T>
ArrayView<u8> AsWritableBytes(T& object);

/**
 * @brief Converts container to a span of bytes.
 * @tparam Container Type of container.
 * @param container Container to convert.
 * @return Span of bytes that can't be modified.
 */
template <typename Container>
    requires Opal::Range<Container>
ArrayView<const u8> AsBytes(Container& container);

/**
 * @brief Converts container to a span of bytes.
 * @tparam Container Type of container.
 * @param container Container to convert.
 * @return Span of bytes that can be modified.
 */
template <typename Container>
    requires Opal::Range<Container>
ArrayView<u8> AsWritableBytes(Container& container);

}  // namespace Opal

#define TEMPLATE_HEADER template <typename T>
#define CLASS_HEADER Opal::ArrayView<T>

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::ArrayView(InputIt first, size_type count) : m_data(&(*first)), m_size(count)
{
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::ArrayView(InputIt first, InputIt last) : m_data(&(*first)), m_size(static_cast<size_type>(last - first))
{
}

TEMPLATE_HEADER
template <Opal::u64 N>
CLASS_HEADER::ArrayView(T (&array)[N]) : m_data(array), m_size(N)
{
}

TEMPLATE_HEADER
template <typename Container>
    requires Opal::Range<Container> && (Opal::SameAs<T, typename Opal::ValueTypeGetter<Container>::Type> ||
                                        Opal::SameAs<typename Opal::RemoveConst<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
CLASS_HEADER::ArrayView(Container& container)
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
bool CLASS_HEADER::operator==(const ArrayView& other) const
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
Opal::ArrayView<const Opal::u8> Opal::AsBytes(T& object)
{
    return {reinterpret_cast<const u8*>(&object), sizeof(T)};
}

template <typename T>
Opal::ArrayView<Opal::u8> Opal::AsWritableBytes(T& object)
{
    return {reinterpret_cast<u8*>(&object), sizeof(T)};
}

template <typename Container>
    requires Opal::Range<Container>
Opal::ArrayView<const Opal::u8> Opal::AsBytes(Container& container)
{
    if (container.empty())
    {
        return {};
    }
    auto data = &(*container.begin());
    using value_type = Container::value_type;
    using size_type = Container::size_type;
    Opal::u64 size = sizeof(value_type) * Narrow<size_type>(container.end() - container.begin());
    return {reinterpret_cast<const u8*>(data), size};
}

template <typename Container>
    requires Opal::Range<Container>
Opal::ArrayView<Opal::u8> Opal::AsWritableBytes(Container& container)
{
    if (container.empty())
    {
        return {};
    }
    auto data = &(*container.begin());
    using value_type = Container::value_type;
    using size_type = Container::size_type;
    Opal::u64 size = sizeof(value_type) * Narrow<size_type>(container.end() - container.begin());
    return {reinterpret_cast<u8*>(data), size};
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::SubSpan(size_type offset, size_type count) const
{
    if (offset + count > m_size)
    {
        return Expected<ArrayView<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<ArrayView<T>, ErrorCode>(ArrayView<T>(m_data + offset, count));
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MySpan>
#define CLASS_HEADER Opal::ArrayViewIterator<MySpan>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const ArrayViewIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const ArrayViewIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const ArrayViewIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const ArrayViewIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    ++m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    ArrayViewIterator temp = *this;
    ++m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    ArrayViewIterator temp = *this;
    --m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return ArrayViewIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return ArrayViewIterator(m_ptr - n);
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
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const ArrayViewIterator& other) const
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
CLASS_HEADER Opal::operator+(typename ArrayViewIterator<MySpan>::difference_type n, const ArrayViewIterator<MySpan>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MySpan>
#define CLASS_HEADER Opal::ArrayViewConstIterator<MySpan>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const ArrayViewConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const ArrayViewConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const ArrayViewConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const ArrayViewConstIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    ++m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    ArrayViewConstIterator temp = *this;
    ++m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    ArrayViewConstIterator temp = *this;
    --m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return ArrayViewConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return ArrayViewConstIterator(m_ptr - n);
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
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const ArrayViewConstIterator& other) const
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
CLASS_HEADER Opal::operator+(typename ArrayViewConstIterator<MySpan>::difference_type n, const ArrayViewConstIterator<MySpan>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

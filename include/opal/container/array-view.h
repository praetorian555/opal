#pragma once

#include <iterator>

#include "opal/assert.h"
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
    using reference = typename MySpan::const_reference;
    using const_reference = typename MySpan::const_reference;
    using pointer = typename MySpan::const_pointer;

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
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type k_npos = static_cast<size_type>(-1);

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
                                      Opal::SameAs<typename Opal::RemoveConstVolatile<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
    ArrayView(Container& container);

    ArrayView(const ArrayView& other) = default;
    ArrayView(ArrayView&& other) noexcept = default;

    ~ArrayView() = default;

    ArrayView& operator=(const ArrayView& other) = default;
    ArrayView& operator=(ArrayView&& other) noexcept = default;

    /**
     * Compare two spans for equality.
     * @param other Span to compare with.
     * @return True if both spans hold the same number of elements and those elements compare equal.
     */
    bool operator==(const ArrayView& other) const;

    T* GetData() { return m_data; }
    [[nodiscard]] const T* GetData() const { return m_data; }

    [[nodiscard]] size_type GetSize() const { return m_size; }

    /** @return Number of bytes the viewed elements occupy. */
    [[nodiscard]] size_type GetSizeInBytes() const { return m_size * sizeof(T); }

    /**
     * Check if the span is empty.
     * @return True if the span is empty, false otherwise.
     */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Get a reference to the element at the specified index. No index bounds checking outside of debug builds.
     * @param index Index of the element.
     * @return Reference to the element.
     */
    reference At(size_type index);
    [[nodiscard]] const_reference At(size_type index) const;

    /**
     * Get a reference to the element at the specified index.
     * @param index Index of the element.
     * @return Reference to the element.
     * @note An out of range index is a caller mistake, not a runtime outcome: the check runs in every build and ends the program
     *       through the contract violation handler. Use TryAt when the index is not known to be in range.
     */
    reference operator[](size_type index);
    const_reference operator[](size_type index) const;

    /**
     * Get a reference to the element at the specified index, for callers that did not check the size first.
     * @param index Index of the element.
     * @return Reference to the element, or ErrorCode::OutOfBounds.
     */
    [[nodiscard]] Expected<T&, ErrorCode> TryAt(size_type index);
    [[nodiscard]] Expected<const T&, ErrorCode> TryAt(size_type index) const;

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
     * @param count Number of elements in the sub span. Defaults to k_npos, which is the rest of the span.
     * @return Sub span. If the offset or count are out of bounds, an ErrorCode::OutOfBounds is returned.
     */
    [[nodiscard]] Expected<ArrayView<T>, ErrorCode> SubSpan(size_type offset, size_type count = k_npos) const;

    /**
     * Get a span over the first elements.
     * @param count Number of elements.
     * @return Sub span. If count is larger than the span, an ErrorCode::OutOfBounds is returned.
     */
    [[nodiscard]] Expected<ArrayView<T>, ErrorCode> First(size_type count) const;

    /**
     * Get a span over the last elements.
     * @param count Number of elements.
     * @return Sub span. If count is larger than the span, an ErrorCode::OutOfBounds is returned.
     */
    [[nodiscard]] Expected<ArrayView<T>, ErrorCode> Last(size_type count) const;

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

    /**
     * Get an iterator to the last element, walking towards the first.
     * @return Reverse iterator to the last element.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    /**
     * Get an iterator to the element before the first.
     * @return Reverse iterator to the element before the first.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

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
CLASS_HEADER::ArrayView(InputIt first, size_type count) : m_size(count)
{
    // The iterator is dereferenced only when there is an element to point at. first can be an end iterator when count is zero.
    m_data = count == 0 ? nullptr : &(*first);
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::ArrayView(InputIt first, InputIt last) : m_size(static_cast<size_type>(last - first))
{
    m_data = m_size == 0 ? nullptr : &(*first);
}

TEMPLATE_HEADER
template <Opal::u64 N>
CLASS_HEADER::ArrayView(T (&array)[N]) : m_data(array), m_size(N)
{
}

TEMPLATE_HEADER
template <typename Container>
    requires Opal::Range<Container> && (Opal::SameAs<T, typename Opal::ValueTypeGetter<Container>::Type> ||
                                        Opal::SameAs<typename Opal::RemoveConstVolatile<T>::Type, typename Opal::ValueTypeGetter<Container>::Type>)
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
    if (m_size != other.m_size)
    {
        return false;
    }
    if (m_data == other.m_data)
    {
        return true;
    }
    for (size_type i = 0; i < m_size; ++i)
    {
        if (!(m_data[i] == other.m_data[i]))
        {
            return false;
        }
    }
    return true;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::At(size_type index)
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::At(size_type index) const
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](size_type index)
{
    OPAL_VERIFY(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::operator[](size_type index) const
{
    OPAL_VERIFY(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::TryAt(size_type index)
{
    if (index >= m_size) [[unlikely]]
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::TryAt(size_type index) const
{
    if (index >= m_size) [[unlikely]]
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
    // Compared without adding, since offset + count wraps for a large count and would let the check pass.
    if (offset > m_size)
    {
        return Expected<ArrayView<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    const size_type remaining = m_size - offset;
    if (count == k_npos)
    {
        count = remaining;
    }
    else if (count > remaining)
    {
        return Expected<ArrayView<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<ArrayView<T>, ErrorCode>(ArrayView<T>(m_data + offset, count));
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::First(size_type count) const
{
    return SubSpan(0, count);
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::Last(size_type count) const
{
    if (count > m_size)
    {
        return Expected<ArrayView<T>, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return SubSpan(m_size - count, count);
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

#pragma once

#include <initializer_list>

#include "opal/assert.h"
#include "opal/common.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wsign-conversion)

namespace Opal
{

template <typename MyArray>
class InPlaceArrayIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::reference;
    using pointer = typename MyArray::pointer;
};

template <typename MyArray>
class InPlaceArrayConstIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::reference;
    using pointer = typename MyArray::pointer;
};

/**
 * A fixed-size array that stores its elements in-place.
 * @tparam T The type of the elements.
 * @tparam N The number of elements in the array.
 */
template <typename T, u64 N>
class InPlaceArray
{
public:
    using value_type = T;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = InPlaceArrayIterator<InPlaceArray<T, N>>;
    using const_iterator = InPlaceArrayConstIterator<InPlaceArray<T, N>>;

    static_assert(N > 0, "Array size must be greater than 0");

    /**
     * Default constructor. No initialization is performed.
     */
    InPlaceArray() = default;

    /**
     * Construct an array with the given initializer list. If the size of the list is less than N, the remaining
     * elements are default initialized.
     * @param list The initializer list.
     */
    InPlaceArray(std::initializer_list<T> list);

    ~InPlaceArray() = default;

    [[nodiscard]] size_type GetSize() const { return N; }

    [[nodiscard]] pointer GetData() { return m_data; }
    [[nodiscard]] const_pointer GetData() const { return m_data; }

    /**
     * Check if two arrays are equal. Two arrays are equal if all their elements are equal and they have the same amount of elements.
     * @param other The other array.
     * @return True if the arrays are equal, false otherwise.
     */
    bool operator==(const InPlaceArray& other) const;

    /**
     * Access an element of the array. If the index is out of bounds, an assertion is triggered in debug builds.
     * @param index The index of the element.
     * @return A reference to the element.
     */
    T& operator[](size_type index);
    const T& operator[](size_type index) const;

    /**
     * Access an element of the array.
     * @param index The index of the element.
     * @return A reference to the element if the index is in bounds, an ErrorCode::OutOfBounds otherwise.
     */
    Expected<T&, ErrorCode> At(size_type index);
    Expected<const T&, ErrorCode> At(size_type index) const;

    /**
     * Get reference to the first element of the array.
     * @return Returns a reference to the first element if the array is not empty, an ErrorCode::OutOfBounds otherwise.
     */
    T& Front();
    const T& Front() const;

    /**
     * Get reference to the last element of the array.
     * @return Returns a reference to the last element if the array is not empty, an ErrorCode::OutOfBounds otherwise.
     */
    T& Back();
    const T& Back() const;

    /**
     * Fill the array with the given value.
     * @param value The value to fill the array with.
     */
    void Fill(const T& value);

private:
    T m_data[N];
};

template <typename T, u64 N>
void Swap(InPlaceArray<T, N>& lhs, InPlaceArray<T, N>& rhs);

}  // namespace Opal

template <typename T, Opal::u64 N>
Opal::InPlaceArray<T, N>::InPlaceArray(std::initializer_list<T> list)
{
    const T* start = list.begin();
    const T* end = list.end();
    u64 size = end - start;
    OPAL_ASSERT(size <= N, "Initializer list size is greater than the array size");

    for (u64 i = 0; i < size; ++i)
    {
        m_data[i] = start[i];
    }
    for (u64 i = size; i < N; ++i)
    {
        m_data[i] = T();
    }
}

template <typename T, Opal::u64 N>
bool Opal::InPlaceArray<T, N>::operator==(const InPlaceArray& other) const
{
    for (u64 i = 0; i < N; ++i)
    {
        if (m_data[i] != other.m_data[i])
        {
            return false;
        }
    }
    return true;
}

template <typename T, Opal::u64 N>
T& Opal::InPlaceArray<T, N>::operator[](size_type index)
{
    OPAL_ASSERT(index < N, "Index out of bounds");
    return m_data[index];
}

template <typename T, Opal::u64 N>
const T& Opal::InPlaceArray<T, N>::operator[](size_type index) const
{
    OPAL_ASSERT(index < N, "Index out of bounds");
    return m_data[index];
}

template <typename T, Opal::u64 N>
Opal::Expected<T&, Opal::ErrorCode> Opal::InPlaceArray<T, N>::At(InPlaceArray::size_type index)
{
    if (index < N)
    {
        return Expected<T&, ErrorCode>(m_data[index]);
    }
    return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
}

template <typename T, Opal::u64 N>
Opal::Expected<const T&, Opal::ErrorCode> Opal::InPlaceArray<T, N>::At(InPlaceArray::size_type index) const
{
    if (index < N)
    {
        return Expected<const T&, ErrorCode>(m_data[index]);
    }
    return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
}

template <typename T, Opal::u64 N>
T& Opal::InPlaceArray<T, N>::Front()
{
    return m_data[0];
}

template <typename T, Opal::u64 N>
const T& Opal::InPlaceArray<T, N>::Front() const
{
    return m_data[0];
}

template <typename T, Opal::u64 N>
T& Opal::InPlaceArray<T, N>::Back()
{
    return m_data[N - 1];
}

template <typename T, Opal::u64 N>
const T& Opal::InPlaceArray<T, N>::Back() const
{
    return m_data[N - 1];
}

template <typename T, Opal::u64 N>
void Opal::Swap(InPlaceArray<T, N>& lhs, InPlaceArray<T, N>& rhs)
{
    for (u64 i = 0; i < N; ++i)
    {
        Swap(lhs[i], rhs[i]);
    }
}

template <typename T, Opal::u64 N>
void Opal::InPlaceArray<T, N>::Fill(const T& value)
{
    for (u64 i = 0; i < N; ++i)
    {
        m_data[i] = value;
    }
}

OPAL_END_DISABLE_WARNINGS

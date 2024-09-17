#pragma once

#include <initializer_list>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wsign-conversion)
OPAL_DISABLE_WARNING(-Wimplicit-int-float-conversion)

namespace Opal
{

/*************************************************************************************************/
/** Iterator API *********************************************************************************/
/*************************************************************************************************/

template <typename MyArray>
class DynamicArrayIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::reference;
    using pointer = typename MyArray::pointer;

    DynamicArrayIterator() = default;
    explicit DynamicArrayIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const DynamicArrayIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayIterator& other) const;
    bool operator>=(const DynamicArrayIterator& other) const;
    bool operator<(const DynamicArrayIterator& other) const;
    bool operator<=(const DynamicArrayIterator& other) const;

    DynamicArrayIterator& operator++();
    DynamicArrayIterator operator++(int);
    DynamicArrayIterator& operator--();
    DynamicArrayIterator operator--(int);

    DynamicArrayIterator operator+(difference_type n) const;
    DynamicArrayIterator operator-(difference_type n) const;
    DynamicArrayIterator& operator+=(difference_type n);
    DynamicArrayIterator& operator-=(difference_type n);

    difference_type operator-(const DynamicArrayIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MyArray>
DynamicArrayIterator<MyArray> operator+(typename DynamicArrayIterator<MyArray>::difference_type n, const DynamicArrayIterator<MyArray>& it);

/*************************************************************************************************/
/** Const Iterator API ***************************************************************************/
/*************************************************************************************************/

template <typename MyArray>
class DynamicArrayConstIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::reference;
    using pointer = typename MyArray::pointer;

    DynamicArrayConstIterator() = default;
    explicit DynamicArrayConstIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const DynamicArrayConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayConstIterator& other) const;
    bool operator>=(const DynamicArrayConstIterator& other) const;
    bool operator<(const DynamicArrayConstIterator& other) const;
    bool operator<=(const DynamicArrayConstIterator& other) const;

    DynamicArrayConstIterator& operator++();
    DynamicArrayConstIterator operator++(int);
    DynamicArrayConstIterator& operator--();
    DynamicArrayConstIterator operator--(int);

    DynamicArrayConstIterator operator+(difference_type n) const;
    DynamicArrayConstIterator operator-(difference_type n) const;
    DynamicArrayConstIterator& operator+=(difference_type n);
    DynamicArrayConstIterator& operator-=(difference_type n);

    difference_type operator-(const DynamicArrayConstIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MyArray>
DynamicArrayConstIterator<MyArray> operator+(typename DynamicArrayConstIterator<MyArray>::difference_type n, const DynamicArrayConstIterator<MyArray>& it);

/**
 * Represents continuous memory storage on the heap that can dynamically grow in size. Similar to std::vector.
 */
template <typename T, typename Allocator = AllocatorBase>
class DynamicArray
{
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = DynamicArrayIterator<DynamicArray<T, Allocator>>;
    using const_iterator = DynamicArrayConstIterator<DynamicArray<T, Allocator>>;

    static_assert(!k_is_reference_value<value_type>, "Value type must not be a reference");
    static_assert(!k_is_const_value<value_type>, "Value type must not be const");

    // Constructors
    DynamicArray(Allocator* allocator = nullptr);
    explicit DynamicArray(size_type count, Allocator* allocator = nullptr);
    DynamicArray(size_type count, const T& default_value, Allocator* allocator = nullptr);
    DynamicArray(T* data, size_type count, Allocator* allocator = nullptr);
    DynamicArray(const DynamicArray& other, Allocator* allocator = nullptr);
    DynamicArray(DynamicArray&& other) noexcept;
    DynamicArray(const std::initializer_list<T>& init_list, Allocator* allocator = nullptr);

    ~DynamicArray();

    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other) noexcept;

    bool operator==(const DynamicArray& other) const;

    /**
     * Clears the array and adds `count` new elements with value `value`.
     * @param count How many new elements to add.
     * @param value Value of the new elements.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Assign(size_type count, const T& value);

    /**
     * Clears the array and adds new elements based on the input iterator range.
     * @tparam InputIt Input iterator type.
     * @param start Start of the range, inclusive.
     * @param end end of the range, exclusive.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::BadInput if start > end, ErrorCode::OutOfMemory if
     * memory allocation failed.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ErrorCode Assign(InputIt start, InputIt end);

    /**
     * Get a reference to the element at specified index.
     * @param index Index of the element in the array.
     * @return Returns reference to the element or ErrorCode::OutOfBounds if the index is out of bounds.
     */
    Expected<reference, ErrorCode> At(size_type index);
    Expected<const_reference, ErrorCode> At(size_type index) const;

    /**
     * Get a reference to the element at specified index. No index bounds checking.
     * @param index Index of the element in the array.
     * @return Reference to the element.
     */
    reference operator[](size_type index);
    const_reference operator[](size_type index) const;

    /**
     * Get a reference to the first element in the array.
     * @return Reference to the first element or ErrorCode::OutOfBounds if the array is empty.
     */
    Expected<reference, ErrorCode> Front();
    Expected<const_reference, ErrorCode> Front() const;

    /**
     * Get a reference to the last element in the array.
     * @return Reference to the last element or ErrorCode::OutOfBounds if the array is empty.
     */
    Expected<reference, ErrorCode> Back();
    Expected<const_reference, ErrorCode> Back() const;

    T* GetData();
    const T* GetData() const;

    [[nodiscard]] size_type GetCapacity() const;
    [[nodiscard]] size_type GetSize() const;

    Allocator* GetAllocator() const { return m_allocator; }

    /**
     * Check if the array is empty.
     * @return True if the array is empty, false otherwise.
     */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    /**
     * Increase the capacity of the array to a value `new_capacity` if its greater then current capacity, otherwise do nothing.
     * @param new_capacity New capacity of the array.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Reserve(size_type new_capacity);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are default constructed.
     * @param new_size New size of the array.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(size_type new_size);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are copy constructed from
     * `default_value`.
     * @param new_size New size of the array.
     * @param default_value Value to copy construct new elements from.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(size_type new_size, const T& default_value);

    /**
     * Clear the array and set its size to 0. Does not deallocate memory.
     */
    void Clear();

    /**
     * Add a new element to the end of the array. If the array is full, it will be resized.
     * @param value Value of the new element.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode PushBack(const T& value);
    ErrorCode PushBack(T&& value);

    /**
     * Remove the last element from the array.
     */
    void PopBack();

    /**
     * Insert a new element at the specified position.
     * @param position Iterator pointing to the position where the new element should be inserted.
     * @param value Value of the new element.
     * @return Iterator pointing to the newly inserted element or ErrorCode::OutOfBounds if the position is invalid,
     * ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<iterator, ErrorCode> Insert(const_iterator position, const T& value);
    Expected<iterator, ErrorCode> Insert(const_iterator position, T&& value);

    /**
     * Insert `count` new elements with value `value` at the specified position.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be cend() to insert at the
     * end.
     * @param count How many new elements to insert.
     * @param value Value of the new elements.
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid,
     * ErrorCode::BadInput if count is 0, ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<iterator, ErrorCode> Insert(const_iterator position, size_type count, const T& value);

    /**
     * Insert new elements from the range [start, end) at the specified position.
     * @tparam InputIt Input iterator type.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be cend() to insert at the
     * end.
     * @param start Start of the range, inclusive.
     * @param end_it end of the range, exclusive.
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid,
     * ErrorCode::BadInput if start >= end, ErrorCode::OutOfMemory if memory allocation failed.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    Expected<iterator, ErrorCode> Insert(const_iterator position, InputIt start, InputIt end_it);

    /**
     * Erase the element at the specified position. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the element following the erased element or ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<iterator, ErrorCode> Erase(const_iterator position);
    Expected<iterator, ErrorCode> Erase(iterator position);

    /**
     * Erase the element at the specified position by swapping it with the last element. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the new element at the position or ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<iterator, ErrorCode> EraseWithSwap(const_iterator position);
    Expected<iterator, ErrorCode> EraseWithSwap(iterator position);

    /**
     * Erase elements in the range [start, end). Does not deallocate memory.
     * @param start_it Iterator pointing to the first element to erase.
     * @param end_it Iterator pointing to the element following the last element to erase.
     * @return Iterator pointing to the element following the last erased element or ErrorCode::BadInput if start > end,
     * ErrorCode::OutOfBounds if start or end are out of bounds.
     */
    Expected<iterator, ErrorCode> Erase(const_iterator start_it, const_iterator end_it);
    Expected<iterator, ErrorCode> Erase(iterator start_it, iterator end_it);

    /** Iterator API - Compatible with standard library. */

    /**
     * Get an iterator pointing to the first element in the array.
     * @return Iterator pointing to the first element.
     */
    iterator begin() { return iterator(m_data); }

    /**
     * Get a const iterator pointing to the first element in the array.
     * @return Const iterator pointing to the first element.
     */
    const_iterator begin() const { return const_iterator(m_data); }

    /**
     * Get a const iterator pointing to the first element in the array.
     * @return Const iterator pointing to the first element.
     */
    const_iterator cbegin() const { return const_iterator(m_data); }

    /**
     * Get an iterator pointing to the element following the last element in the array.
     * @return Iterator pointing to the element following the last element.
     */
    iterator end() { return iterator(m_data + m_size); }

    /**
     * Get a const iterator pointing to the element following the last element in the array.
     * @return Const iterator pointing to the element following the last element.
     */
    const_iterator end() const { return const_iterator(m_data + m_size); }

    /**
     * Get a const iterator pointing to the element following the last element in the array.
     * @return Const iterator pointing to the element following the last element.
     */
    const_iterator cend() const { return const_iterator(m_data + m_size); }

private:
    T* Allocate(size_type count);
    void Deallocate(T* ptr);

    static constexpr f64 k_resize_factor = 1.5;

    Allocator* m_allocator = nullptr;
    size_type m_capacity = 0;
    size_type m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

#define TEMPLATE_HEADER template <typename T, typename Allocator>
#define CLASS_HEADER Opal::DynamicArray<T, Allocator>

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(Allocator* allocator) : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for Array");
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, const T& default_value, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for Array");
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(T* data, size_type count, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        m_data = nullptr;
        OPAL_ASSERT(false, "Invalid size for Array");
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for Array");
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(const DynamicArray& other, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    if (m_capacity == 0)
    {
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for Array");
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(DynamicArray&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(const std::initializer_list<T>& init_list, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    size_type count = init_list.size();
    if (count > m_capacity)
    {
        m_data = Allocate(count);
        if (m_data == nullptr)
        {
            return;
        }
        m_capacity = count;
    }
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(*(init_list.begin() + i));  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray::~DynamicArray()
{
    if (m_data != nullptr)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
        Deallocate(m_data);
    }
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(const DynamicArray& other)
{
    if (this == &other)
    {
        return *this;
    }
    if (m_data != nullptr)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    if (m_allocator != other.m_allocator)
    {
        Deallocate(m_data);
        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
        m_allocator = other.m_allocator;
    }
    if (m_capacity < other.m_size)
    {
        Deallocate(m_data);
        m_capacity = other.m_size;
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_size = 0;
            OPAL_ASSERT(false, "Failed to allocate memory for Array");
            return *this;
        }
    }
    m_size = other.m_size;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(DynamicArray&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (m_data != nullptr)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
        Deallocate(m_data);
        m_data = nullptr;
    }
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    m_data = other.m_data;
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
    return *this;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const DynamicArray& other) const
{
    if (m_size != other.m_size)
    {
        return false;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        if (m_data[i] != other.m_data[i])
        {
            return false;
        }
    }
    return true;
}

TEMPLATE_HEADER
inline CLASS_HEADER::size_type CLASS_HEADER::GetCapacity() const
{
    return m_capacity;
}

TEMPLATE_HEADER
inline CLASS_HEADER::size_type CLASS_HEADER::GetSize() const
{
    return m_size;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(DynamicArray::size_type count, const T& value)
{
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = count;
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
    }
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(value);  // Invokes copy constructor on allocated memory
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Assign(InputIt start, InputIt end)
{
    if (start > end)
    {
        return ErrorCode::BadInput;
    }
    size_type count = static_cast<size_type>(end - start);
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = count;
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
    }
    m_size = count;
    InputIt current = start;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(*(current + i));  // Invokes copy constructor on allocated memory
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::reference, Opal::ErrorCode> CLASS_HEADER::At(size_type index)
{
    using ReturnType = Expected<reference, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::const_reference, Opal::ErrorCode> CLASS_HEADER::At(size_type index) const
{
    using ReturnType = Expected<const_reference, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](DynamicArray::size_type index)
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::operator[](DynamicArray::size_type index) const
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::reference, Opal::ErrorCode> CLASS_HEADER::Front()
{
    using ReturnType = Expected<reference, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::const_reference, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    using ReturnType = Expected<const_reference, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::reference, Opal::ErrorCode> CLASS_HEADER::Back()
{
    using ReturnType = Expected<reference, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::const_reference, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    using ReturnType = Expected<const_reference, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
inline T* CLASS_HEADER::GetData()
{
    return m_data;
}

TEMPLATE_HEADER
inline const T* CLASS_HEADER::GetData() const
{
    return m_data;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Reserve(DynamicArray::size_type new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return ErrorCode::Success;
    }
    T* new_data = Allocate(new_capacity);
    if (new_data == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(DynamicArray::size_type new_size)
{
    return Resize(new_size, T());
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(DynamicArray::size_type new_size, const T& default_value)
{
    if (new_size == m_size)
    {
        return ErrorCode::Success;
    }
    if (new_size < m_size)
    {
        for (size_type i = new_size; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
        m_size = new_size;
    }
    else
    {
        if (new_size > m_capacity)
        {
            ErrorCode err = Reserve(new_size);
            if (err != ErrorCode::Success)
            {
                return err;
            }
        }
        for (size_type i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    m_size = 0;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        const size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    new (&m_data[m_size]) T(value);  // Invokes copy constructor on allocated memory
    m_size++;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        const size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    new (&m_data[m_size]) T(Move(value));  // Invokes move constructor on allocated memory
    m_size++;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::PopBack()
{
    if (m_size == 0)
    {
        return;
    }
    m_data[m_size - 1].~T();  // Invokes destructor on allocated memory
    m_size--;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(const_iterator position, const T& value)
{
    if (position < cbegin() || position > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    size_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        const size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<iterator, ErrorCode>(err);
        }
    }
    iterator it = end() - 1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + 1) = Move(*it);
        --it;
    }
    *mut_position = value;
    m_size++;
    return Expected<iterator, ErrorCode>(mut_position);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(DynamicArray::const_iterator position, T&& value)
{
    if (position < cbegin() || position > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    size_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        const size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<iterator, ErrorCode>(err);
        }
    }
    iterator it = end() - 1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + 1) = Move(*it);
        --it;
    }
    *mut_position = Move(value);
    m_size++;
    return Expected<iterator, ErrorCode>(mut_position);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(DynamicArray::const_iterator position,
                                                                                          DynamicArray::size_type count, const T& value)
{
    if (position < cbegin() || position > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::BadInput);
    }
    size_type pos_offset = position - cbegin();
    if (m_size + count > m_capacity)
    {
        size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<iterator, ErrorCode>(err);
        }
    }
    iterator it = end() - 1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + count) = Move(*it);
        --it;
    }
    iterator return_it = mut_position;
    for (size_type i = 0; i < count; i++)
    {
        *mut_position = value;
        ++mut_position;
    }
    m_size += count;
    return Expected<iterator, ErrorCode>(return_it);
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(const_iterator position, InputIt start,
                                                                                          InputIt end_it)
{
    if (position < cbegin() || position > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start >= end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::BadInput);
    }
    size_type pos_offset = position - cbegin();
    size_type count = static_cast<size_type>(end_it - start);
    if (m_size + count > m_capacity)
    {
        size_type new_capacity = static_cast<size_type>((m_capacity * k_resize_factor) + 1.0);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<iterator, ErrorCode>(err);
        }
    }
    iterator it = end() -  1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + count) = Move(*it);
        --it;
    }
    iterator return_it = mut_position;
    for (InputIt current = start; current < end_it; ++current)
    {
        *mut_position = *current;
        ++mut_position;
    }
    m_size += count;
    return Expected<iterator, ErrorCode>(return_it);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(DynamicArray::const_iterator position)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (position < cbegin() || position >= cend())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    size_type pos_offset = position - cbegin();
    iterator mut_position = begin() + pos_offset;
    (*mut_position).~T();  // Invokes destructor on allocated memory
    while (mut_position < end() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    m_size--;
    return ReturnType{begin() + pos_offset};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator position)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (position < begin() || position >= end())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    size_type pos_offset = position - begin();
    iterator mut_position = begin() + pos_offset;
    (*mut_position).~T();  // Invokes destructor on allocated memory
    while (mut_position < end() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    m_size--;
    return ReturnType{begin() + pos_offset};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::EraseWithSwap(DynamicArray::const_iterator position)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (position < cbegin() || position >= cend())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    iterator mut_position = begin() + (position - cbegin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != end() - 1)
    {
        *mut_position = Move(*(end() - 1));
        m_size--;
        return ReturnType{mut_position};
    }
    m_size--;
    return ReturnType{end()};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::EraseWithSwap(iterator position)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (position < begin() || position >= end())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    iterator mut_position = begin() + (position - begin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != end() - 1)
    {
        *mut_position = Move(*(end() - 1));
        m_size--;
        return ReturnType{mut_position};
    }
    m_size--;
    return ReturnType{end()};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(DynamicArray::const_iterator start_it,
                                                                                         DynamicArray::const_iterator end_it)
{
    if (start_it > end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::BadInput);
    }
    if (start_it < cbegin() || end_it > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start_it == end_it)
    {
        return Expected<iterator, ErrorCode>(begin() + (start_it - cbegin()));
    }
    const size_type start_offset = start_it - cbegin();
    const size_type end_offset = end_it - cbegin();
    for (size_type i = start_offset; i < end_offset; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    iterator mut_start = begin() + start_offset;
    iterator mut_end = begin() + end_offset;
    while (mut_end < end())
    {
        *mut_start = Move(*mut_end);
        ++mut_start;
        ++mut_end;
    }
    m_size += start_offset - end_offset;
    using ReturnType = Expected<iterator, ErrorCode>;
    return ReturnType{begin() + start_offset};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator start_it, iterator end_it)
{
    if (start_it > end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::BadInput);
    }
    if (start_it < begin() || end_it > end())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start_it == end_it)
    {
        return Expected<iterator, ErrorCode>(begin() + (start_it - begin()));
    }
    const size_type start_offset = start_it - begin();
    const size_type end_offset = end_it - begin();
    for (size_type i = start_offset; i < end_offset; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    iterator mut_start = begin() + start_offset;
    iterator mut_end = begin() + end_offset;
    while (mut_end < end())
    {
        *mut_start = Move(*mut_end);
        ++mut_start;
        ++mut_end;
    }
    m_size += start_offset - end_offset;
    using ReturnType = Expected<iterator, ErrorCode>;
    return ReturnType{begin() + start_offset};
}

TEMPLATE_HEADER
T* CLASS_HEADER::Allocate(size_type count)
{
    OPAL_ASSERT(m_allocator, "Allocator should never be null!");
    constexpr u64 k_alignment = 8;
    const size_type bytes_to_allocate = count * sizeof(T);
    return static_cast<T*>(m_allocator->Alloc(bytes_to_allocate, k_alignment));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(T* ptr)
{
    OPAL_ASSERT(m_allocator, "Allocator should never be null!");
    m_allocator->Free(ptr);
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray>
#define CLASS_HEADER Opal::DynamicArrayIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DynamicArrayIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DynamicArrayIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DynamicArrayIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DynamicArrayIterator& other) const
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
    DynamicArrayIterator temp = *this;
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
    DynamicArrayIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return DynamicArrayIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return DynamicArrayIterator(m_ptr - n);
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
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const DynamicArrayIterator& other) const
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
    OPAL_ASSERT(m_ptr, "Dereferencing null pointer");
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename DynamicArrayIterator<MyArray>::difference_type n, const DynamicArrayIterator<MyArray>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray>
#define CLASS_HEADER Opal::DynamicArrayConstIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DynamicArrayConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DynamicArrayConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DynamicArrayConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DynamicArrayConstIterator& other) const
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
    DynamicArrayConstIterator temp = *this;
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
    DynamicArrayConstIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return DynamicArrayConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return DynamicArrayConstIterator(m_ptr - n);
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
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const DynamicArrayConstIterator& other) const
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
CLASS_HEADER Opal::operator+(typename DynamicArrayConstIterator<MyArray>::difference_type n, const DynamicArrayConstIterator<MyArray>& it)
{
    return it + n;
}

OPAL_END_DISABLE_WARNINGS

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

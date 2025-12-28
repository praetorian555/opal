#pragma once

#include <cstring>
#include <initializer_list>
#include <new>

#include "opal/allocator.h"
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
DynamicArrayConstIterator<MyArray> operator+(typename DynamicArrayConstIterator<MyArray>::difference_type n,
                                             const DynamicArrayConstIterator<MyArray>& it);

/**
 * Represents continuous memory storage on the heap that can dynamically grow in size. Similar to std::vector.
 */
template <typename T>
class DynamicArray
{
public:
    using value_type = T;
    using allocator_type = AllocatorBase;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = DynamicArrayIterator<DynamicArray>;
    using const_iterator = DynamicArrayConstIterator<DynamicArray>;

    static_assert(!k_is_reference_value<value_type>, "Value type must not be a reference");
    static_assert(!k_is_const_value<value_type>, "Value type must not be const");

    /**
     * Default constructor.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` default constructed elements.
     * @param count Number of elements to construct.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    explicit DynamicArray(size_type count, allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` elements with value `default_value`.
     * @param count Number of elements to construct.
     * @param default_value Value of the elements.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(size_type count, const T& default_value, allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` elements copied from `data`.
     * @param data Source data.
     * @param count Number of elements to copy.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(const T* data, size_type count, allocator_type* allocator = nullptr);

    /**
     * Copy constructor.
     * @param other Source array.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(const DynamicArray& other, allocator_type* allocator = nullptr);

    /**
     * Move constructor.
     * @param other Source array.
     */
    DynamicArray(DynamicArray&& other) noexcept;

    /**
     * Construct an array with elements from the initializer list.
     * @param init_list Initializer list.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(const std::initializer_list<T>& init_list, allocator_type* allocator = nullptr);

    ~DynamicArray();

    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other) noexcept;

    bool operator==(const DynamicArray& other) const;

    /**
     * Clears the array and adds `count` new elements with value `value`.
     * @param count How many new elements to add.
     * @param value Value of the new elements.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Assign(size_type count, const T& value);

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
     * @return Returns a reference to the element in the array at the given index.
     * @throw OutOfBoundsException when index is out of bounds.
     */
    reference At(size_type index);
    const_reference At(size_type index) const;

    /**
     * Get a reference to the element at specified index. No index bounds checking.
     * @param index Index of the element in the array.
     * @return Reference to the element.
     */
    reference operator[](size_type index);
    const_reference operator[](size_type index) const;

    /**
     * Get a reference to the first element in the array.
     * @return Reference to the first element.
     * @throw OutOfBoundsException when array is empty.
     */
    reference Front();
    const_reference Front() const;

    /**
     * Get a reference to the last element in the array.
     * @return Reference to the last element.
     * @throw OutOfBoundsException when array is empty.
     */
    reference Back();
    const_reference Back() const;

    T* GetData();
    const T* GetData() const;

    [[nodiscard]] size_type GetCapacity() const;
    [[nodiscard]] size_type GetSize() const;

    allocator_type* GetAllocator() const { return m_allocator; }
    void SetAllocator(allocator_type* allocator);

    /**
     * Check if the array is empty.
     * @return True if the array is empty, false otherwise.
     */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Increase the capacity of the array to a value `new_capacity` if its greater then current capacity, otherwise do nothing.
     * @param new_capacity New capacity of the array.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Reserve(size_type new_capacity);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are default constructed.
     * @param new_size New size of the array.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Resize(size_type new_size);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are copy constructed from
     * `default_value`.
     * @param new_size New size of the array.
     * @param default_value Value to copy construct new elements from.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Resize(size_type new_size, const T& default_value);

    /**
     * Clear the array and set its size to 0. Does not deallocate memory.
     */
    void Clear();

    /**
     * Add a new element to the end of the array. If the array is full, it will be resized.
     * @param value Value of the new element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void PushBack(const T& value);
    void PushBack(T&& value);

    template <typename ContainerClass>
        requires Range<ContainerClass>
    void Append(const ContainerClass& container);

    template <typename ContainerClass>
        requires Range<ContainerClass>
    void Append(ContainerClass&& container);

    /**
     * Remove the last element from the array.
     */
    void PopBack();

    /**
     * Insert a new element at the specified position.
     * @param position Iterator pointing to the position where the new element should be inserted.
     * @param value Value of the new element.
     * @return Iterator pointing to the newly inserted element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     */
    iterator Insert(const_iterator position, const T& value);
    iterator Insert(const_iterator position, T&& value);

    /**
     * Insert `count` new elements with value `value` at the specified position.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be cend() to insert at the
     * end.
     * @param count How many new elements to insert.
     * @param value Value of the new elements.
     * @return Iterator pointing to the first newly inserted element or @p position if no element is inserted.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     */
    iterator Insert(const_iterator position, size_type count, const T& value);

    /**
     * Insert new elements from the range [@p start_it, @p end_it) at the specified position.
     * @tparam InputIt Input iterator type.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be @ref cend to insert at the
     * end.
     * @param start_it Start of the range, inclusive.
     * @param end_it end of the range, exclusive.
     * @return Iterator pointing to the first newly inserted element or @p position if no element is inserted.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     * @throw InvalidArgumentException if @p start_it is greater than @p end_it.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    iterator Insert(const_iterator position, InputIt start_it, InputIt end_it);

    /**
     * Erase the element at the specified position. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the element following the erased element. Returns @ref end() if @p position is out of bounds.
     */
    iterator Erase(const_iterator position);
    iterator Erase(iterator position);

    /**
     * Erase the element at the specified position by swapping it with the last element. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the new element at the @pos position, or @ref end() if @p position is out of bounds.
     */
    iterator EraseWithSwap(const_iterator position);
    iterator EraseWithSwap(iterator position);

    /**
     * Erase elements in the range [start, end). Does not deallocate memory.
     * @param start_it Iterator pointing to the first element to erase.
     * @param end_it Iterator pointing to the element following the last element to erase.
     * @return Iterator pointing to the element following the last erased element or ErrorCode::BadInput if start > end,
     * ErrorCode::OutOfBounds if start or end are out of bounds.
     */
    Expected<iterator, ErrorCode> Erase(const_iterator start_it, const_iterator end_it);
    Expected<iterator, ErrorCode> Erase(iterator start_it, iterator end_it);

    /**
     * Remove an element in the array matching the value argument. Do nothing if element is not in the array. The order of the elements
     * stays the same.
     * @param value Value to find. Uses equality operator of the type T.
     */
    void Remove(const T& value);

    /**
     * Remove an element in the array matching the value argument. The order of the elements does not stay the same.
     * @param value Value to find. Uses equality operator of the type T.
     */
    void RemoveWithSwap(const T& value);

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

    size_type GetNextCapacity(size_type current_capacity) const;

    static constexpr f64 k_resize_factor = 1.5;

    allocator_type* m_allocator = nullptr;
    size_type m_capacity = 0;
    size_type m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

#define TEMPLATE_HEADER template <typename T>
#define CLASS_HEADER Opal::DynamicArray<T>

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(allocator_type* allocator) : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, const T& default_value, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(const T* data, size_type count, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, data, count * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(const DynamicArray& other, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (other.m_capacity == 0)
    {
        return;
    }
    m_data = Allocate(other.m_capacity);
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, other.m_data, m_size * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
        }
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
CLASS_HEADER::DynamicArray(const std::initializer_list<T>& init_list, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    size_type count = init_list.size();
    if (count > m_capacity)
    {
        m_data = Allocate(count);
        m_capacity = count;
    }
    m_size = count;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, init_list.begin(), count * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(*(init_list.begin() + i));  // Invokes copy constructor on allocated memory
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray::~DynamicArray()
{
    if (m_data != nullptr)
    {
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        Deallocate(m_data);
        m_data = nullptr;
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
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
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
    }
    m_size = other.m_size;
    if constexpr (IsPOD<T>)
    {
        if (m_size > 0)
        {
            memcpy(m_data, other.m_data, m_size * sizeof(T));
        }
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
        }
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
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        Deallocate(m_data);
        m_data = nullptr;
    }
    m_allocator = other.m_allocator;
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
    if (m_size == 0)
    {
        return true;
    }
    if constexpr (IsPOD<T>)
    {
        return memcmp(m_data, other.m_data, sizeof(T)) == 0;
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            if (m_data[i] != other.m_data[i])
            {
                return false;
            }
        }
        return true;
    }
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
void CLASS_HEADER::SetAllocator(allocator_type* allocator)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    if (m_allocator == allocator)
    {
        return;
    }
    T* new_data = static_cast<T*>(allocator->Alloc(m_size * sizeof(T), alignof(T)));
    if constexpr (IsPOD<T>)
    {
        memcpy(new_data, m_data, m_size * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            // Invoke move constructor
            new (&new_data[i]) T(Move(m_data[i]));
        }
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = m_size;
    m_allocator = allocator;
}

TEMPLATE_HEADER
void CLASS_HEADER::Assign(size_type count, const T& value)
{
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = count;
        m_data = Allocate(m_capacity);
    }
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(value);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Assign(InputIt start, InputIt end)
{
    if (start > end) [[unlikely]]
    {
        return ErrorCode::InvalidArgument;
    }
    size_type count = static_cast<size_type>(end - start);
    if (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = count;
        m_data = Allocate(m_capacity);
    }
    m_size = count;
    InputIt current = start;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, &(*current), m_size * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            new (&m_data[i]) T(*(current + Narrow<difference_type>(i)));  // Invokes copy constructor on allocated memory
        }
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::At(size_type index)
{
    if (index >= m_size) [[unlikely]]
    {
        throw OutOfBoundsException(index, 0, m_size - 1);
    }
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::At(size_type index) const
{
    if (index >= m_size) [[unlikely]]
    {
        throw OutOfBoundsException(index, 0, m_size - 1);
    }
    return m_data[index];
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
typename CLASS_HEADER::reference CLASS_HEADER::Front()
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[0];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::Front() const
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[0];
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::Back()
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[m_size - 1];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::Back() const
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[m_size - 1];
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
void CLASS_HEADER::Reserve(DynamicArray::size_type new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return;
    }
    T* new_data = Allocate(new_capacity);
    if constexpr (IsPOD<T>)
    {
        if (m_size > 0)
        {
            memcpy(new_data, m_data, sizeof(T) * m_size);
        }
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
        }
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
}

TEMPLATE_HEADER
void CLASS_HEADER::Resize(DynamicArray::size_type new_size)
{
    Resize(new_size, T());
}

TEMPLATE_HEADER
void CLASS_HEADER::Resize(DynamicArray::size_type new_size, const T& default_value)
{
    if (new_size == m_size)
    {
        return;
    }
    if (new_size < m_size)
    {
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = new_size; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        m_size = new_size;
    }
    else
    {
        if (new_size > m_capacity)
        {
            Reserve(new_size);
        }
        for (size_type i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
    return;
}

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    m_size = 0;
}

TEMPLATE_HEADER
void CLASS_HEADER::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        const size_type new_capacity = GetNextCapacity(m_capacity);
        Reserve(new_capacity);
    }
    new (&m_data[m_size]) T(value);  // Invokes copy constructor on allocated memory
    m_size++;
}

TEMPLATE_HEADER
void CLASS_HEADER::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        const size_type new_capacity = GetNextCapacity(m_capacity);
        Reserve(new_capacity);
    }
    new (&m_data[m_size]) T(Move(value));  // Invokes move constructor on allocated memory
    m_size++;
}

TEMPLATE_HEADER
template <typename ContainerClass>
    requires Opal::Range<ContainerClass>
void CLASS_HEADER::Append(const ContainerClass& container)
{
    for (const auto& element : container)
    {
        PushBack(element);
    }
}

TEMPLATE_HEADER
template <typename ContainerClass>
    requires Opal::Range<ContainerClass>
void CLASS_HEADER::Append(ContainerClass&& container)
{
    for (auto& element : container)
    {
        PushBack(std::move(element));
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::PopBack()
{
    if (m_size == 0)
    {
        return;
    }
    if constexpr (!IsPOD<T>)
    {
        m_data[m_size - 1].~T();  // Invokes destructor on allocated memory
    }
    m_size--;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, const T& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), 0, cend() - cbegin() + 1);
    }
    difference_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        const size_type new_capacity = GetNextCapacity(m_capacity);
        Reserve(new_capacity);
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
    return mut_position;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(DynamicArray::const_iterator position, T&& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), 0, cend() - cbegin() + 1);
    }
    difference_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        const size_type new_capacity = GetNextCapacity(m_capacity);
        Reserve(new_capacity);
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
    return mut_position;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, size_type count, const T& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), 0, cend() - cbegin() - 1);
    }
    if (count == 0)
    {
        return begin() + (position - cbegin());
    }
    difference_type pos_offset = position - cbegin();
    if (m_size + count > m_capacity)
    {
        size_type new_capacity = GetNextCapacity(m_capacity);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        Reserve(new_capacity);
    }
    iterator it = end() - 1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + Narrow<difference_type>(count)) = Move(*it);
        --it;
    }
    iterator return_it = mut_position;
    for (size_type i = 0; i < count; i++)
    {
        *mut_position = value;
        ++mut_position;
    }
    m_size += count;
    return return_it;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, InputIt start_it, InputIt end_it)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), 0, cend() - cbegin() - 1);
    }
    if (start_it > end_it)
    {
        throw InvalidArgumentException(__FUNCTION__, "end_it - start_it", static_cast<i64>(end_it - start_it));
    }
    difference_type pos_offset = position - cbegin();
    size_type count = static_cast<size_type>(end_it - start_it);
    if (m_size + count > m_capacity)
    {
        size_type new_capacity = GetNextCapacity(m_capacity);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        Reserve(new_capacity);
    }
    iterator it = end() - 1;
    iterator mut_position = begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + Narrow<difference_type>(count)) = Move(*it);
        --it;
    }
    iterator return_it = mut_position;
    for (InputIt current = start_it; current < end_it; ++current)
    {
        *mut_position = *current;
        ++mut_position;
    }
    m_size += count;
    return return_it;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Erase(const_iterator position)
{
    if (position < cbegin() || position >= cend())
    {
        return end();
    }
    difference_type pos_offset = position - cbegin();
    iterator mut_position = begin() + pos_offset;
    (*mut_position).~T();  // Invokes destructor on allocated memory
    while (mut_position < end() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    m_size--;
    return begin() + pos_offset;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Erase(iterator position)
{
    if (position < begin() || position >= end())
    {
        return end();
    }
    difference_type pos_offset = position - begin();
    iterator mut_position = begin() + pos_offset;
    (*mut_position).~T();  // Invokes destructor on allocated memory
    while (mut_position < end() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    m_size--;
    return begin() + pos_offset;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::EraseWithSwap(DynamicArray::const_iterator position)
{
    if (position < cbegin() || position >= cend())
    {
        return end();
    }
    iterator mut_position = begin() + (position - cbegin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != end() - 1)
    {
        *mut_position = Move(*(end() - 1));
        m_size--;
        return mut_position;
    }
    m_size--;
    return end();
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::EraseWithSwap(iterator position)
{
    if (position < begin() || position >= end())
    {
        return end();
    }
    iterator mut_position = begin() + (position - begin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != end() - 1)
    {
        *mut_position = Move(*(end() - 1));
        m_size--;
        return mut_position;
    }
    m_size--;
    return end();
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(DynamicArray::const_iterator start_it,
                                                                                     DynamicArray::const_iterator end_it)
{
    if (start_it > end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::InvalidArgument);
    }
    if (start_it < cbegin() || end_it > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start_it == end_it)
    {
        return Expected<iterator, ErrorCode>(begin() + (start_it - cbegin()));
    }
    const difference_type start_offset = start_it - cbegin();
    const difference_type end_offset = end_it - cbegin();
    for (difference_type i = start_offset; i < end_offset; ++i)
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
    m_size += Narrow<size_type>(start_offset - end_offset);
    using ReturnType = Expected<iterator, ErrorCode>;
    return ReturnType{begin() + start_offset};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator start_it, iterator end_it)
{
    if (start_it > end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::InvalidArgument);
    }
    if (start_it < begin() || end_it > end())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start_it == end_it)
    {
        return Expected<iterator, ErrorCode>(begin() + (start_it - begin()));
    }
    const difference_type start_offset = start_it - begin();
    const difference_type end_offset = end_it - begin();
    for (difference_type i = start_offset; i < end_offset; ++i)
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
    m_size += Narrow<size_type>(start_offset - end_offset);
    using ReturnType = Expected<iterator, ErrorCode>;
    return ReturnType{begin() + start_offset};
}

TEMPLATE_HEADER
void CLASS_HEADER::Remove(const T& value)
{
    iterator it = begin();
    while (it != end())
    {
        if (*it == value)
        {
            Erase(it);
            return;
        }
        ++it;
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::RemoveWithSwap(const T& value)
{
    iterator it = begin();
    while (it != end())
    {
        if (*it == value)
        {
            EraseWithSwap(it);
            return;
        }
        ++it;
    }
}

TEMPLATE_HEADER
T* CLASS_HEADER::Allocate(size_type count)
{
    OPAL_ASSERT(m_allocator, "Allocator should never be null!");
    constexpr u64 k_alignment = alignof(T);
    const size_type bytes_to_allocate = count * sizeof(T);
    return static_cast<T*>(m_allocator->Alloc(bytes_to_allocate, k_alignment));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(T* ptr)
{
    if (m_allocator)
    {
        m_allocator->Free(ptr);
    }
}

TEMPLATE_HEADER
typename CLASS_HEADER::size_type CLASS_HEADER::GetNextCapacity(size_type current_capacity) const
{
    if (current_capacity == 0)
    {
        return 1;
    }
    return static_cast<size_type>((Narrow<f64>(m_capacity) * k_resize_factor) + 1.0);
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

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

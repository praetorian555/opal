#pragma once

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyDynamicArray>
class DynamicArrayIterator
{
public:
    using ValueType = typename MyDynamicArray::ValueType;
    using ReferenceType = typename MyDynamicArray::ReferenceType;
    using PointerType = typename MyDynamicArray::PointerType;
    using DifferenceType = typename MyDynamicArray::DifferenceType;

    DynamicArrayIterator() = default;
    explicit DynamicArrayIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const DynamicArrayIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayIterator& other) const;
    bool operator>=(const DynamicArrayIterator& other) const;
    bool operator<(const DynamicArrayIterator& other) const;
    bool operator<=(const DynamicArrayIterator& other) const;

    DynamicArrayIterator& operator++();
    DynamicArrayIterator operator++(int);
    DynamicArrayIterator& operator--();
    DynamicArrayIterator operator--(int);

    DynamicArrayIterator operator+(DifferenceType n) const;
    DynamicArrayIterator operator-(DifferenceType n) const;
    DynamicArrayIterator& operator+=(DifferenceType n);
    DynamicArrayIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const DynamicArrayIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyDynamicArray>
DynamicArrayIterator<MyDynamicArray> operator+(typename DynamicArrayIterator<MyDynamicArray>::DifferenceType n,
                                               const DynamicArrayIterator<MyDynamicArray>& it);

template <typename MyDynamicArray>
class DynamicArrayConstIterator
{
public:
    using ValueType = typename MyDynamicArray::ValueType;
    using ReferenceType = typename MyDynamicArray::ConstReferenceType;
    using PointerType = typename MyDynamicArray::PointerType;
    using DifferenceType = typename MyDynamicArray::DifferenceType;

    DynamicArrayConstIterator() = default;
    explicit DynamicArrayConstIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const DynamicArrayConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayConstIterator& other) const;
    bool operator>=(const DynamicArrayConstIterator& other) const;
    bool operator<(const DynamicArrayConstIterator& other) const;
    bool operator<=(const DynamicArrayConstIterator& other) const;

    DynamicArrayConstIterator& operator++();
    DynamicArrayConstIterator operator++(int);
    DynamicArrayConstIterator& operator--();
    DynamicArrayConstIterator operator--(int);

    DynamicArrayConstIterator operator+(DifferenceType n) const;
    DynamicArrayConstIterator operator-(DifferenceType n) const;
    DynamicArrayConstIterator& operator+=(DifferenceType n);
    DynamicArrayConstIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const DynamicArrayConstIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyDynamicArray>
DynamicArrayConstIterator<MyDynamicArray> operator+(typename DynamicArrayConstIterator<MyDynamicArray>::DifferenceType n,
                                                    const DynamicArrayConstIterator<MyDynamicArray>& it);

/**
 * Represents continuous memory storage that can dynamically grow in size.
 */
template <typename T, typename Allocator = DefaultAllocator>
class DynamicArray
{
public:
    using ValueType = T;
    using ReferenceType = T&;
    using ConstReferenceType = const T&;
    using PointerType = T*;
    using SizeType = u64;
    using AllocatorType = Allocator;
    using DifferenceType = i64;
    using IteratorType = DynamicArrayIterator<DynamicArray<T, Allocator>>;
    using ConstIteratorType = DynamicArrayConstIterator<DynamicArray<T, Allocator>>;

    static_assert(!k_is_reference_value<ValueType>, "Value type must be a reference");
    static_assert(!k_is_const_value<ValueType>, "Value type must not be const");

    // Constructors
    DynamicArray();
    explicit DynamicArray(const Allocator& allocator);
    explicit DynamicArray(Allocator&& allocator);
    explicit DynamicArray(SizeType count);
    DynamicArray(SizeType count, const Allocator& allocator);
    DynamicArray(SizeType count, Allocator&& allocator);
    DynamicArray(SizeType count, const T& default_value);
    DynamicArray(SizeType count, const T& default_value, const Allocator& allocator);
    DynamicArray(SizeType count, const T& default_value, Allocator&& allocator);
    DynamicArray(T* data, SizeType count);
    DynamicArray(T* data, SizeType count, const Allocator& allocator);
    DynamicArray(T* data, SizeType count, Allocator&& allocator);
    DynamicArray(const DynamicArray& other);
    DynamicArray(const DynamicArray& other, const Allocator& allocator);
    DynamicArray(const DynamicArray& other, Allocator&& allocator);
    DynamicArray(DynamicArray&& other) noexcept;
    DynamicArray(DynamicArray&& other, const Allocator& allocator) noexcept;
    DynamicArray(DynamicArray&& other, Allocator&& allocator) noexcept;

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
    ErrorCode Assign(SizeType count, const T& value);

    /**
     * Clears the array and adds new elements based on the input iterator range.
     * @tparam InputIt Input iterator type.
     * @param start Start of the range, inclusive.
     * @param end End of the range, exclusive.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::BadInput if start > end, ErrorCode::OutOfMemory if
     * memory allocation failed.
     */
    template <typename InputIt>
    ErrorCode AssignIt(InputIt start, InputIt end);

    /**
     * Get a reference to the element at specified index.
     * @param index Index of the element in the array.
     * @return Returns reference to the element or ErrorCode::OutOfBounds if the index is out of bounds.
     */
    Expected<ReferenceType, ErrorCode> At(SizeType index);
    Expected<ConstReferenceType, ErrorCode> At(SizeType index) const;

    /**
     * Get a reference to the element at specified index. No index bounds checking.
     * @param index Index of the element in the array.
     * @return Reference to the element.
     */
    ReferenceType operator[](SizeType index);
    ConstReferenceType operator[](SizeType index) const;

    /**
     * Get a reference to the first element in the array.
     * @return Reference to the first element or ErrorCode::OutOfBounds if the array is empty.
     */
    Expected<ReferenceType, ErrorCode> Front();
    Expected<ConstReferenceType, ErrorCode> Front() const;

    /**
     * Get a reference to the last element in the array.
     * @return Reference to the last element or ErrorCode::OutOfBounds if the array is empty.
     */
    Expected<ReferenceType, ErrorCode> Back();
    Expected<ConstReferenceType, ErrorCode> Back() const;

    T* GetData();
    const T* GetData() const;

    [[nodiscard]] SizeType GetCapacity() const;
    [[nodiscard]] SizeType GetSize() const;

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    /**
     * Increase the capacity of the array to a value `new_capacity` if its greater then current capacity, otherwise do nothing.
     * @param new_capacity New capacity of the array.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Reserve(SizeType new_capacity);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are default constructed.
     * @param new_size New size of the array.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(SizeType new_size);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are copy constructed from
     * `default_value`.
     * @param new_size New size of the array.
     * @param default_value Value to copy construct new elements from.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(SizeType new_size, const T& default_value);

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
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType position, const T& value);
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType position, T&& value);

    /**
     * Insert `count` new elements with value `value` at the specified position.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be ConstEnd() to insert at the
     * end.
     * @param count How many new elements to insert.
     * @param value Value of the new elements.
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid, ErrorCode::BadInput
     * if count is 0, ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType position, SizeType count, const T& value);

    /**
     * Insert new elements from the range [start, end) at the specified position.
     * @tparam InputIt Input iterator type.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be ConstEnd() to insert at the
     * end.
     * @param start Start of the range, inclusive.
     * @param end End of the range, exclusive.
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid, ErrorCode::BadInput
     * if start >= end, ErrorCode::OutOfMemory if memory allocation failed.
     */
    template <typename InputIt>
    Expected<IteratorType, ErrorCode> InsertIt(ConstIteratorType position, InputIt start, InputIt end);

    /**
     * Erase the element at the specified position. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the element following the erased element or ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType position);

    /**
     * Erase elements in the range [start, end). Does not deallocate memory.
     * @param start Iterator pointing to the first element to erase.
     * @param end Iterator pointing to the element following the last element to erase.
     * @return Iterator pointing to the element following the last erased element or ErrorCode::BadInput if start > end,
     * ErrorCode::OutOfBounds if start or end are out of bounds.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType start, ConstIteratorType end);

    // Iterators
    IteratorType Begin() { return IteratorType(m_data); }
    IteratorType End() { return IteratorType(m_data + m_size); }
    ConstIteratorType ConstBegin() const { return ConstIteratorType(m_data); }
    ConstIteratorType ConstEnd() const { return ConstIteratorType(m_data + m_size); }

    // Compatible with std::begin and std::end
    IteratorType begin() { return IteratorType(m_data); }
    IteratorType end() { return IteratorType(m_data + m_size); }
    ConstIteratorType begin() const { return ConstIteratorType(m_data); }
    ConstIteratorType end() const { return ConstIteratorType(m_data + m_size); }

private:
    T* Allocate(SizeType count);
    void Deallocate(T* ptr);

    static constexpr SizeType k_default_capacity = 4;
    static constexpr f64 k_resize_factor = 1.5;

    Allocator m_allocator;
    SizeType m_capacity = k_default_capacity;
    SizeType m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/***************************************** Implementation *****************************************/

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray()
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const Allocator& allocator) : m_allocator(allocator)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(Allocator&& allocator) : m_allocator(Move(allocator))
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count, const Allocator& allocator) : m_allocator(allocator)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count, Allocator&& allocator) : m_allocator(Move(allocator))
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count, const T& default_value)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count, const T& default_value, const Allocator& allocator) : m_allocator(allocator)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count, const T& default_value, Allocator&& allocator) : m_allocator(Move(allocator))
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(T* data, SizeType count) : m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        m_data = nullptr;
        OPAL_ASSERT(false, "Invalid size for DynamicArray");
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(T* data, SizeType count, const Allocator& allocator)
    : m_allocator(allocator), m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        m_data = nullptr;
        OPAL_ASSERT(false, "Invalid size for DynamicArray");
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(T* data, SizeType count, Allocator&& allocator)
    : m_allocator(Move(allocator)), m_capacity(count), m_size(count)
{
    if (m_capacity == 0)
    {
        m_data = nullptr;
        OPAL_ASSERT(false, "Invalid size for DynamicArray");
        return;
    }
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const DynamicArray& other)
    : m_allocator(other.m_allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const DynamicArray& other, const Allocator& allocator)
    : m_allocator(allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const DynamicArray& other, Allocator&& allocator)
    : m_allocator(allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(DynamicArray&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(DynamicArray&& other, const Allocator& allocator) noexcept
    : m_allocator(allocator), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(DynamicArray&& other, Allocator&& allocator) noexcept
    : m_allocator(allocator), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::~DynamicArray()
{
    if (m_data != nullptr)
    {
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
        Deallocate(m_data);
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>& Opal::DynamicArray<T, Allocator>::operator=(const DynamicArray& other)
{
    if (this == &other)
    {
        return *this;
    }
    if (m_data != nullptr)
    {
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    if (m_capacity < other.m_size)
    {
        Deallocate(m_data);
        m_capacity = other.m_size;
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_size = 0;
            OPAL_ASSERT(false, "Failed to allocate memory for DynamicArray");
            return *this;
        }
    }
    m_size = other.m_size;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
    return *this;
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>& Opal::DynamicArray<T, Allocator>::operator=(DynamicArray&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (m_data != nullptr)
    {
        for (SizeType i = 0; i < m_size; i++)
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

template <typename T, typename Allocator>
bool Opal::DynamicArray<T, Allocator>::operator==(const DynamicArray& other) const
{
    if (m_size != other.m_size)
    {
        return false;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        if (m_data[i] != other.m_data[i])
        {
            return false;
        }
    }
    return true;
}

template <typename T, typename Allocator>
inline Opal::DynamicArray<T, Allocator>::SizeType Opal::DynamicArray<T, Allocator>::GetCapacity() const
{
    return m_capacity;
}

template <typename T, typename Allocator>
inline Opal::DynamicArray<T, Allocator>::SizeType Opal::DynamicArray<T, Allocator>::GetSize() const
{
    return m_size;
}

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::Assign(DynamicArray::SizeType count, const T& value)
{
    for (SizeType i = 0; i < m_size; i++)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(value);  // Invokes copy constructor on allocated memory
    }
    return ErrorCode::Success;
}

template <typename T, typename Allocator>
template <typename InputIt>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::AssignIt(InputIt start, InputIt end)
{
    if (start > end)
    {
        return ErrorCode::BadInput;
    }
    SizeType count = static_cast<SizeType>(end - start);
    for (SizeType i = 0; i < m_size; i++)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(*(current + i));  // Invokes copy constructor on allocated memory
    }
    return ErrorCode::Success;
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ReferenceType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::At(
    SizeType index)
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ConstReferenceType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::At(
    SizeType index) const
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

template <typename T, typename Allocator>
typename Opal::DynamicArray<T, Allocator>::ReferenceType Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::operator[](
    DynamicArray::SizeType index)
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

template <typename T, typename Allocator>
typename Opal::DynamicArray<T, Allocator>::ConstReferenceType Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::operator[](
    DynamicArray::SizeType index) const
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ReferenceType, Opal::ErrorCode>
Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::Front()
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ConstReferenceType, Opal::ErrorCode>
Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::Front() const
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ReferenceType, Opal::ErrorCode>
Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::Back()
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::ConstReferenceType, Opal::ErrorCode>
Opal::DynamicArray<T, Allocator>::DynamicArray<T, Allocator>::Back() const
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

template <typename T, typename Allocator>
inline T* Opal::DynamicArray<T, Allocator>::GetData()
{
    return m_data;
}

template <typename T, typename Allocator>
inline const T* Opal::DynamicArray<T, Allocator>::GetData() const
{
    return m_data;
}

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::Reserve(DynamicArray::SizeType new_capacity)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
    return ErrorCode::Success;
}

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::Resize(DynamicArray::SizeType new_size)
{
    return Resize(new_size, T());
}

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::Resize(DynamicArray::SizeType new_size, const T& default_value)
{
    if (new_size == m_size)
    {
        return ErrorCode::Success;
    }
    if (new_size < m_size)
    {
        for (SizeType i = new_size; i < m_size; i++)
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
        for (SizeType i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
    return ErrorCode::Success;
}

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::Clear()
{
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    m_size = 0;
}

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        const SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
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

template <typename T, typename Allocator>
Opal::ErrorCode Opal::DynamicArray<T, Allocator>::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        const SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
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

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::PopBack()
{
    if (m_size == 0)
    {
        return;
    }
    m_data[m_size - 1].~T();  // Invokes destructor on allocated memory
    m_size--;
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::Insert(
    ConstIteratorType position, const T& value)
{
    if (position < ConstBegin() || position > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    SizeType pos_offset = position - ConstBegin();
    if (m_size == m_capacity)
    {
        const SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    IteratorType it = End();
    IteratorType mut_position = Begin() + pos_offset;
    while (it > mut_position)
    {
        *it = Move(*(it - 1));
        --it;
    }
    *mut_position = value;
    m_size++;
    return Expected<IteratorType, ErrorCode>(mut_position);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::Insert(
    DynamicArray::ConstIteratorType position, T&& value)
{
    if (position < ConstBegin() || position > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    SizeType pos_offset = position - ConstBegin();
    if (m_size == m_capacity)
    {
        const SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    IteratorType it = End();
    IteratorType mut_position = Begin() + pos_offset;
    while (it > mut_position)
    {
        *it = Move(*(it - 1));
        --it;
    }
    *mut_position = Move(value);
    m_size++;
    return Expected<IteratorType, ErrorCode>(mut_position);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::Insert(
    DynamicArray::ConstIteratorType position, DynamicArray::SizeType count, const T& value)
{
    if (position < ConstBegin() || position > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    SizeType pos_offset = position - ConstBegin();
    if (m_size + count > m_capacity)
    {
        SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    IteratorType it = End() + count - 1;
    IteratorType mut_position = Begin() + pos_offset;
    for (SizeType i = 0; i < count; i++)
    {
        *it = Move(*(it - count));
        --it;
    }
    IteratorType return_it = mut_position;
    for (SizeType i = 0; i < count; i++)
    {
        *mut_position = value;
        ++mut_position;
    }
    m_size += count;
    return Expected<IteratorType, ErrorCode>(return_it);
}

template <typename T, typename Allocator>
template <typename InputIt>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::InsertIt(
    DynamicArray::ConstIteratorType position, InputIt start, InputIt end)
{
    if (position < ConstBegin() || position > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start >= end)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    SizeType pos_offset = position - ConstBegin();
    SizeType count = static_cast<SizeType>(end - start);
    if (m_size + count > m_capacity)
    {
        SizeType new_capacity = static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        ErrorCode err = Reserve(new_capacity);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    IteratorType it = End() + count - 1;
    IteratorType mut_position = Begin() + pos_offset;
    for (SizeType i = 0; i < count; i++)
    {
        *it = Move(*(it - count));
        --it;
    }
    IteratorType return_it = mut_position;
    for (InputIt current = start; current < end; ++current)
    {
        *mut_position = *current;
        ++mut_position;
    }
    m_size += count;
    return Expected<IteratorType, ErrorCode>(return_it);
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::Erase(
    DynamicArray::ConstIteratorType position)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (position < ConstBegin() || position >= ConstEnd())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    SizeType pos_offset = position - ConstBegin();
    IteratorType mut_position = Begin() + pos_offset;
    (*mut_position).~T();  // Invokes destructor on allocated memory
    while (mut_position < End() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    m_size--;
    return ReturnType{Begin() + pos_offset};
}

template <typename T, typename Allocator>
Opal::Expected<typename Opal::DynamicArray<T, Allocator>::IteratorType, Opal::ErrorCode> Opal::DynamicArray<T, Allocator>::Erase(
    DynamicArray::ConstIteratorType start, DynamicArray::ConstIteratorType end)
{
    if (start > end)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    if (start < ConstBegin() || end > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start == end)
    {
        return Expected<IteratorType, ErrorCode>(Begin() + (start - ConstBegin()));
    }
    const SizeType start_offset = start - ConstBegin();
    const SizeType end_offset = end - ConstBegin();
    for (SizeType i = start_offset; i < end_offset; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    IteratorType mut_start = Begin() + start_offset;
    IteratorType mut_end = Begin() + end_offset;
    while (mut_end < End())
    {
        *mut_start = Move(*mut_end);
        ++mut_start;
        ++mut_end;
    }
    m_size += start_offset - end_offset;
    using ReturnType = Expected<IteratorType, ErrorCode>;
    return ReturnType{Begin() + start_offset};
}

template <typename T, typename Allocator>
T* Opal::DynamicArray<T, Allocator>::Allocate(SizeType count)
{
    return static_cast<T*>(m_allocator.Allocate(count * sizeof(T)));
}

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::Deallocate(T* ptr)
{
    m_allocator.Deallocate(ptr);
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayIterator<MyDynamicArray>::operator>(const DynamicArrayIterator& other) const
{
    return m_ptr > other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayIterator<MyDynamicArray>::operator>=(const DynamicArrayIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayIterator<MyDynamicArray>::operator<(const DynamicArrayIterator& other) const
{
    return m_ptr < other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayIterator<MyDynamicArray>::operator<=(const DynamicArrayIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray>& Opal::DynamicArrayIterator<MyDynamicArray>::operator++()
{
    m_ptr++;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray> Opal::DynamicArrayIterator<MyDynamicArray>::operator++(int)
{
    DynamicArrayIterator temp = *this;
    m_ptr++;
    return temp;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray>& Opal::DynamicArrayIterator<MyDynamicArray>::operator--()
{
    m_ptr--;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray> Opal::DynamicArrayIterator<MyDynamicArray>::operator--(int)
{
    DynamicArrayIterator temp = *this;
    m_ptr--;
    return temp;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray> Opal::DynamicArrayIterator<MyDynamicArray>::operator+(DifferenceType n) const
{
    return DynamicArrayIterator(m_ptr + n);
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray> Opal::DynamicArrayIterator<MyDynamicArray>::operator-(DifferenceType n) const
{
    return DynamicArrayIterator(m_ptr - n);
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray>& Opal::DynamicArrayIterator<MyDynamicArray>::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray>& Opal::DynamicArrayIterator<MyDynamicArray>::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayIterator<MyDynamicArray>::DifferenceType Opal::DynamicArrayIterator<MyDynamicArray>::operator-(
    const DynamicArrayIterator& other) const
{
    return m_ptr - other.m_ptr;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayIterator<MyDynamicArray>::ReferenceType Opal::DynamicArrayIterator<MyDynamicArray>::operator[](
    DifferenceType n) const
{
    return *(m_ptr + n);
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayIterator<MyDynamicArray>::ReferenceType Opal::DynamicArrayIterator<MyDynamicArray>::operator*() const
{
    return *m_ptr;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayIterator<MyDynamicArray>::PointerType Opal::DynamicArrayIterator<MyDynamicArray>::operator->() const
{
    return m_ptr;
}

template <typename MyDynamicArray>
Opal::DynamicArrayIterator<MyDynamicArray> Opal::operator+(typename DynamicArrayIterator<MyDynamicArray>::DifferenceType n,
                                                           const DynamicArrayIterator<MyDynamicArray>& it)
{
    return it + n;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayConstIterator<MyDynamicArray>::operator>(const DynamicArrayConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayConstIterator<MyDynamicArray>::operator>=(const DynamicArrayConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayConstIterator<MyDynamicArray>::operator<(const DynamicArrayConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

template <typename MyDynamicArray>
bool Opal::DynamicArrayConstIterator<MyDynamicArray>::operator<=(const DynamicArrayConstIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray>& Opal::DynamicArrayConstIterator<MyDynamicArray>::operator++()
{
    m_ptr++;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray> Opal::DynamicArrayConstIterator<MyDynamicArray>::operator++(int)
{
    DynamicArrayConstIterator temp = *this;
    m_ptr++;
    return temp;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray>& Opal::DynamicArrayConstIterator<MyDynamicArray>::operator--()
{
    m_ptr--;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray> Opal::DynamicArrayConstIterator<MyDynamicArray>::operator--(int)
{
    DynamicArrayConstIterator temp = *this;
    m_ptr--;
    return temp;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray> Opal::DynamicArrayConstIterator<MyDynamicArray>::operator+(DifferenceType n) const
{
    return DynamicArrayConstIterator(m_ptr + n);
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray> Opal::DynamicArrayConstIterator<MyDynamicArray>::operator-(DifferenceType n) const
{
    return DynamicArrayConstIterator(m_ptr - n);
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray>& Opal::DynamicArrayConstIterator<MyDynamicArray>::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray>& Opal::DynamicArrayConstIterator<MyDynamicArray>::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayConstIterator<MyDynamicArray>::DifferenceType Opal::DynamicArrayConstIterator<MyDynamicArray>::operator-(
    const DynamicArrayConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayConstIterator<MyDynamicArray>::ReferenceType Opal::DynamicArrayConstIterator<MyDynamicArray>::operator[](
    DifferenceType n) const
{
    return *(m_ptr + n);
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayConstIterator<MyDynamicArray>::ReferenceType Opal::DynamicArrayConstIterator<MyDynamicArray>::operator*() const
{
    return *m_ptr;
}

template <typename MyDynamicArray>
typename Opal::DynamicArrayConstIterator<MyDynamicArray>::PointerType Opal::DynamicArrayConstIterator<MyDynamicArray>::operator->() const
{
    return m_ptr;
}

template <typename MyDynamicArray>
Opal::DynamicArrayConstIterator<MyDynamicArray> Opal::operator+(typename DynamicArrayConstIterator<MyDynamicArray>::DifferenceType n,
                                                                const DynamicArrayConstIterator<MyDynamicArray>& it)
{
    return it + n;
}

#pragma once

#include <initializer_list>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyArray>
class ArrayIterator
{
public:
    using ValueType = typename MyArray::ValueType;
    using ReferenceType = typename MyArray::ReferenceType;
    using PointerType = typename MyArray::PointerType;
    using DifferenceType = typename MyArray::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    ArrayIterator() = default;
    explicit ArrayIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const ArrayIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const ArrayIterator& other) const;
    bool operator>=(const ArrayIterator& other) const;
    bool operator<(const ArrayIterator& other) const;
    bool operator<=(const ArrayIterator& other) const;

    ArrayIterator& operator++();
    ArrayIterator operator++(int);
    ArrayIterator& operator--();
    ArrayIterator operator--(int);

    ArrayIterator operator+(DifferenceType n) const;
    ArrayIterator operator-(DifferenceType n) const;
    ArrayIterator& operator+=(DifferenceType n);
    ArrayIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const ArrayIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyArray>
ArrayIterator<MyArray> operator+(typename ArrayIterator<MyArray>::DifferenceType n, const ArrayIterator<MyArray>& it);

template <typename MyArray>
class ArrayConstIterator
{
public:
    using ValueType = typename MyArray::ValueType;
    using ReferenceType = typename MyArray::ConstReferenceType;
    using PointerType = typename MyArray::PointerType;
    using DifferenceType = typename MyArray::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    ArrayConstIterator() = default;
    explicit ArrayConstIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const ArrayConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const ArrayConstIterator& other) const;
    bool operator>=(const ArrayConstIterator& other) const;
    bool operator<(const ArrayConstIterator& other) const;
    bool operator<=(const ArrayConstIterator& other) const;

    ArrayConstIterator& operator++();
    ArrayConstIterator operator++(int);
    ArrayConstIterator& operator--();
    ArrayConstIterator operator--(int);

    ArrayConstIterator operator+(DifferenceType n) const;
    ArrayConstIterator operator-(DifferenceType n) const;
    ArrayConstIterator& operator+=(DifferenceType n);
    ArrayConstIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const ArrayConstIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyArray>
ArrayConstIterator<MyArray> operator+(typename ArrayConstIterator<MyArray>::DifferenceType n, const ArrayConstIterator<MyArray>& it);

/**
 * Represents continuous memory storage on the heap that can dynamically grow in size. Similar to std::vector.
 */
template <typename T, typename Allocator = AllocatorBase>
class Array
{
public:
    using ValueType = T;
    using ReferenceType = T&;
    using ConstReferenceType = const T&;
    using PointerType = T*;
    using SizeType = u64;
    using AllocatorType = Allocator;
    using DifferenceType = i64;
    using IteratorType = ArrayIterator<Array<T, Allocator>>;
    using ConstIteratorType = ArrayConstIterator<Array<T, Allocator>>;

    using value_type = ValueType;

    static_assert(!k_is_reference_value<ValueType>, "Value type must not be a reference");
    static_assert(!k_is_const_value<ValueType>, "Value type must not be const");

    // Constructors
    Array(Allocator* allocator = nullptr);
    explicit Array(SizeType count, Allocator* allocator = nullptr);
    Array(SizeType count, const T& default_value, Allocator* allocator = nullptr);
    Array(T* data, SizeType count, Allocator* allocator = nullptr);
    Array(const Array& other, Allocator* allocator = nullptr);
    Array(Array&& other) noexcept;
    Array(const std::initializer_list<T>& init_list, Allocator* allocator = nullptr);

    ~Array();

    Array& operator=(const Array& other);
    Array& operator=(Array&& other) noexcept;

    bool operator==(const Array& other) const;

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
        requires RandomAccessIterator<InputIt>
    ErrorCode Assign(InputIt start, InputIt end);

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

    Allocator* GetAllocator() const { return m_allocator; }

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
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid,
     * ErrorCode::BadInput if count is 0, ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType position, SizeType count, const T& value);

    /**
     * Insert new elements from the range [start, end) at the specified position.
     * @tparam InputIt Input iterator type.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be ConstEnd() to insert at the
     * end.
     * @param start Start of the range, inclusive.
     * @param end End of the range, exclusive.
     * @return Iterator pointing to the first newly inserted element or ErrorCode::OutOfBounds if the position is invalid,
     * ErrorCode::BadInput if start >= end, ErrorCode::OutOfMemory if memory allocation failed.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType position, InputIt start, InputIt end);

    /**
     * Erase the element at the specified position. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the element following the erased element or ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType position);
    Expected<IteratorType, ErrorCode> Erase(IteratorType position);

    /**
     * Erase the element at the specified position by swapping it with the last element. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the new element at the position or ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<IteratorType, ErrorCode> EraseWithSwap(ConstIteratorType position);
    Expected<IteratorType, ErrorCode> EraseWithSwap(IteratorType position);

    /**
     * Erase elements in the range [start, end). Does not deallocate memory.
     * @param start Iterator pointing to the first element to erase.
     * @param end Iterator pointing to the element following the last element to erase.
     * @return Iterator pointing to the element following the last erased element or ErrorCode::BadInput if start > end,
     * ErrorCode::OutOfBounds if start or end are out of bounds.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType start, ConstIteratorType end);
    Expected<IteratorType, ErrorCode> Erase(IteratorType start, IteratorType end);

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

    static constexpr f64 k_resize_factor = 1.5;

    Allocator* m_allocator = nullptr;
    SizeType m_capacity = 0;
    SizeType m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/***************************************** Implementation *****************************************/

#define TEMPLATE_HEADER template <typename T, typename Allocator>
#define CLASS_HEADER Opal::Array<T, Allocator>

TEMPLATE_HEADER
CLASS_HEADER::Array(Allocator* allocator) : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::Array(SizeType count, Allocator* allocator)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Array(SizeType count, const T& default_value, Allocator* allocator)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Array(T* data, SizeType count, Allocator* allocator)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Array(const Array& other, Allocator* allocator)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Array(Array&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
CLASS_HEADER::Array(const std::initializer_list<T>& init_list, Allocator* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    SizeType count = init_list.size();
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(*(init_list.begin() + i));  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Array::~Array()
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

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(const Array& other)
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
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(Array&& other) noexcept
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

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const Array& other) const
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

TEMPLATE_HEADER
inline CLASS_HEADER::SizeType CLASS_HEADER::GetCapacity() const
{
    return m_capacity;
}

TEMPLATE_HEADER
inline CLASS_HEADER::SizeType CLASS_HEADER::GetSize() const
{
    return m_size;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(Array::SizeType count, const T& value)
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

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Assign(InputIt start, InputIt end)
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

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ReferenceType, Opal::ErrorCode> CLASS_HEADER::At(SizeType index)
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ConstReferenceType, Opal::ErrorCode> CLASS_HEADER::At(SizeType index) const
{
    using ReturnType = Expected<ConstReferenceType, ErrorCode>;
    if (index >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[index]);
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](Array::SizeType index)
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::ConstReferenceType CLASS_HEADER::operator[](Array::SizeType index) const
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ReferenceType, Opal::ErrorCode> CLASS_HEADER::Front()
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ConstReferenceType, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    using ReturnType = Expected<ConstReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ReferenceType, Opal::ErrorCode> CLASS_HEADER::Back()
{
    using ReturnType = Expected<ReferenceType, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::ConstReferenceType, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    using ReturnType = Expected<ConstReferenceType, ErrorCode>;
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
Opal::ErrorCode CLASS_HEADER::Reserve(Array::SizeType new_capacity)
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

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(Array::SizeType new_size)
{
    return Resize(new_size, T());
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(Array::SizeType new_size, const T& default_value)
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

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    for (SizeType i = 0; i < m_size; i++)
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

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushBack(T&& value)
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
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(ConstIteratorType position, const T& value)
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
    IteratorType it = End() - 1;
    IteratorType mut_position = Begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + 1) = Move(*it);
        --it;
    }
    *mut_position = value;
    m_size++;
    return Expected<IteratorType, ErrorCode>(mut_position);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(Array::ConstIteratorType position, T&& value)
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
    IteratorType it = End() - 1;
    IteratorType mut_position = Begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + 1) = Move(*it);
        --it;
    }
    *mut_position = Move(value);
    m_size++;
    return Expected<IteratorType, ErrorCode>(mut_position);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(Array::ConstIteratorType position,
                                                                                          Array::SizeType count, const T& value)
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
    IteratorType it = End() - 1;
    IteratorType mut_position = Begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + count) = Move(*it);
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

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(ConstIteratorType position, InputIt start,
                                                                                          InputIt end)
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
    IteratorType it = End() -  1;
    IteratorType mut_position = Begin() + pos_offset;
    while (it >= mut_position)
    {
        *(it + count) = Move(*it);
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

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(Array::ConstIteratorType position)
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

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType position)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (position < Begin() || position >= End())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    SizeType pos_offset = position - Begin();
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

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::EraseWithSwap(Array::ConstIteratorType position)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (position < ConstBegin() || position >= ConstEnd())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    IteratorType mut_position = Begin() + (position - ConstBegin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != End() - 1)
    {
        *mut_position = Move(*(End() - 1));
        m_size--;
        return ReturnType{mut_position};
    }
    m_size--;
    return ReturnType{End()};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::EraseWithSwap(IteratorType position)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (position < Begin() || position >= End())
    {
        return ReturnType{ErrorCode::OutOfBounds};
    }
    IteratorType mut_position = Begin() + (position - Begin());
    (*mut_position).~T();  // Invokes destructor on allocated memory
    if (mut_position != End() - 1)
    {
        *mut_position = Move(*(End() - 1));
        m_size--;
        return ReturnType{mut_position};
    }
    m_size--;
    return ReturnType{End()};
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(Array::ConstIteratorType start,
                                                                                         Array::ConstIteratorType end)
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

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType start, IteratorType end)
{
    if (start > end)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    if (start < Begin() || end > End())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start == end)
    {
        return Expected<IteratorType, ErrorCode>(Begin() + (start - Begin()));
    }
    const SizeType start_offset = start - Begin();
    const SizeType end_offset = end - Begin();
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

TEMPLATE_HEADER
T* CLASS_HEADER::Allocate(SizeType count)
{
    OPAL_ASSERT(m_allocator, "Allocator should never be null!");
    constexpr u64 k_alignment = 8;
    const SizeType bytes_to_allocate = count * sizeof(T);
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
#define CLASS_HEADER Opal::ArrayIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const ArrayIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const ArrayIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const ArrayIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const ArrayIterator& other) const
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
    ArrayIterator temp = *this;
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
    ArrayIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return ArrayIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return ArrayIterator(m_ptr - n);
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
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const ArrayIterator& other) const
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
    OPAL_ASSERT(m_ptr, "Dereferencing null pointer");
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename ArrayIterator<MyArray>::DifferenceType n, const ArrayIterator<MyArray>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray>
#define CLASS_HEADER Opal::ArrayConstIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const ArrayConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const ArrayConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const ArrayConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const ArrayConstIterator& other) const
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
    ArrayConstIterator temp = *this;
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
    ArrayConstIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return ArrayConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return ArrayConstIterator(m_ptr - n);
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
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const ArrayConstIterator& other) const
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
CLASS_HEADER Opal::operator+(typename ArrayConstIterator<MyArray>::DifferenceType n, const ArrayConstIterator<MyArray>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

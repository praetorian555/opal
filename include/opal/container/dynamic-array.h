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

    DynamicArray();
    explicit DynamicArray(const Allocator& allocator);
    explicit DynamicArray(Allocator&& allocator);
    explicit DynamicArray(SizeType count);
    DynamicArray(SizeType count, const Allocator& allocator);
    DynamicArray(SizeType count, Allocator&& allocator);
    DynamicArray(SizeType count, const T& default_value);
    DynamicArray(SizeType count, const T& default_value, const Allocator& allocator);
    DynamicArray(SizeType count, const T& default_value, Allocator&& allocator);
    // TODO: Implement constructors that take iterators as input
    // TODO: Implement constructors that take std::initializer_list as input
    DynamicArray(const DynamicArray& other);
    DynamicArray(DynamicArray&& other) noexcept;

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
     * @param return ErrorCode::Success if the operation was successful, ErrorCode::BadInput if start > end, ErrorCode::OutOfMemory if
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

    ReferenceType operator[](SizeType index);
    ConstReferenceType operator[](SizeType index) const;

    Expected<ReferenceType, ErrorCode> Front();
    Expected<ConstReferenceType, ErrorCode> Front() const;

    Expected<ReferenceType, ErrorCode> Back();
    Expected<ConstReferenceType, ErrorCode> Back() const;

    T* GetData();
    const T* GetData() const;

    [[nodiscard]] SizeType GetCapacity() const;
    [[nodiscard]] SizeType GetSize() const;

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    void Reserve(SizeType new_capacity);

    void Resize(SizeType new_size);
    void Resize(SizeType new_size, const T& default_value);

    void Clear();

    void PushBack(const T& value);
    void PushBack(T&& value);

    void PopBack();

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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const Allocator& allocator) : m_allocator(allocator)
{
    m_data = Allocate(m_capacity);
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(Allocator&& allocator) : m_allocator(Move(allocator))
{
    m_data = Allocate(m_capacity);
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(SizeType count)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count);
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(const DynamicArray& other) : m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = Allocate(m_capacity);
    OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T, typename Allocator>
Opal::DynamicArray<T, Allocator>::DynamicArray(DynamicArray&& other) noexcept
    : m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
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
        OPAL_ASSERT(m_data != nullptr, "Failed to allocate memory for DynamicArray");
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
void Opal::DynamicArray<T, Allocator>::Reserve(DynamicArray::SizeType new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return;
    }
    T* new_data = Allocate(new_capacity);
    OPAL_ASSERT(new_data != nullptr, "Failed to allocate memory for DynamicArray");
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
}

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::Resize(DynamicArray::SizeType new_size)
{
    Resize(new_size, T());
}

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::Resize(DynamicArray::SizeType new_size, const T& default_value)
{
    if (new_size == m_size)
    {
        return;
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
            Reserve(new_size);
        }
        for (SizeType i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
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
void Opal::DynamicArray<T, Allocator>::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        Reserve(static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0));
    }
    new (&m_data[m_size]) T(value);  // Invokes copy constructor on allocated memory
    m_size++;
}

template <typename T, typename Allocator>
void Opal::DynamicArray<T, Allocator>::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        Reserve(static_cast<SizeType>((m_capacity * k_resize_factor) + 1.0));
    }
    new (&m_data[m_size]) T(Move(value));  // Invokes move constructor on allocated memory
    m_size++;
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

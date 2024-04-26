#pragma once

#include "opal/allocator.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyDeque>
class DequeIterator
{
public:
    using ValueType = typename MyDeque::ValueType;
    using ReferenceType = typename MyDeque::ReferenceType;
    using PointerType = typename MyDeque::PointerType;
    using DifferenceType = typename MyDeque::DifferenceType;
    using SizeType = typename MyDeque::SizeType;

    explicit DequeIterator(PointerType data, SizeType capacity, SizeType first, SizeType index);

    bool operator==(const DequeIterator& other) const;
    bool operator>(const DequeIterator& other) const;
    bool operator>=(const DequeIterator& other) const;
    bool operator<(const DequeIterator& other) const;
    bool operator<=(const DequeIterator& other) const;

    DequeIterator& operator++();
    DequeIterator operator++(int);
    DequeIterator& operator--();
    DequeIterator operator--(int);

    DequeIterator operator+(DifferenceType n) const;
    DequeIterator operator-(DifferenceType n) const;
    DequeIterator& operator+=(DifferenceType n);
    DequeIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const DequeIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_data = nullptr;
    SizeType m_capacity = 0;
    SizeType m_first = 0;
    SizeType m_index = 0;
};

template <typename MyDeque>
DequeIterator<MyDeque> operator+(typename DequeIterator<MyDeque>::DifferenceType n, const DequeIterator<MyDeque>& it);

template <typename MyDeque>
class DequeConstIterator
{
public:
    using ValueType = typename MyDeque::ValueType;
    using ReferenceType = typename MyDeque::ConstReferenceType;
    using PointerType = typename MyDeque::PointerType const;
    using DifferenceType = typename MyDeque::DifferenceType;
    using SizeType = typename MyDeque::SizeType;

    DequeConstIterator() = default;
    explicit DequeConstIterator(PointerType data, SizeType capacity, SizeType first, SizeType index);

    bool operator==(const DequeConstIterator& other) const;
    bool operator>(const DequeConstIterator& other) const;
    bool operator>=(const DequeConstIterator& other) const;
    bool operator<(const DequeConstIterator& other) const;
    bool operator<=(const DequeConstIterator& other) const;

    DequeConstIterator& operator++();
    DequeConstIterator operator++(int);
    DequeConstIterator& operator--();
    DequeConstIterator operator--(int);

    DequeConstIterator operator+(DifferenceType n) const;
    DequeConstIterator operator-(DifferenceType n) const;
    DequeConstIterator& operator+=(DifferenceType n);
    DequeConstIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const DequeConstIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_data = nullptr;
    SizeType m_capacity = 0;
    SizeType m_first = 0;
    SizeType m_index = 0;
};

template <typename MyDeque>
DequeConstIterator<MyDeque> operator+(typename DequeConstIterator<MyDeque>::DifferenceType n, const DequeConstIterator<MyDeque>& it);

/**
 * Data structure that provides constant time insertion and deletion at both ends of the container. It also allows
 * constant time random access to elements.
 *
 * Capacity will always be a power of 2.
 */
template <typename T, typename Allocator = DefaultAllocator>
class Deque
{
public:
    using ValueType = T;
    using ReferenceType = T&;
    using ConstReferenceType = const T&;
    using PointerType = T*;
    using SizeType = u64;
    using AllocatorType = Allocator;
    using DifferenceType = i64;
    using IteratorType = DequeIterator<Deque<T, Allocator>>;
    using ConstIteratorType = DequeConstIterator<Deque<T, Allocator>>;

    static_assert(!k_is_reference_value<ValueType>, "Value type must not be a reference");
    static_assert(!k_is_const_value<ValueType>, "Value type must not be const");

    Deque();
    explicit Deque(const AllocatorType& allocator);
    explicit Deque(AllocatorType&& allocator);
    explicit Deque(SizeType count);
    Deque(SizeType count, const AllocatorType& allocator);
    Deque(SizeType count, AllocatorType&& allocator);
    Deque(SizeType count, const T& value);
    Deque(SizeType count, const T& value, const AllocatorType& allocator);
    Deque(SizeType count, const T& value, AllocatorType&& allocator);
    Deque(const Deque& other);
    Deque(Deque&& other) noexcept;

    ~Deque();

    Deque& operator=(const Deque& other);
    Deque& operator=(Deque&& other) noexcept;

    bool operator==(const Deque& other) const;

    /**
     * Clears the container and assigns new values to it that are default constructed.
     * @param count Number of elements to assign.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Assign(SizeType count);

    /**
     * Clears the container and assigns new values to it that are copies of the provided value.
     * @param count Number of elements to assign.
     * @param value Value to assign.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Assign(SizeType count, const T& value);

    /**
     * Clears the container and assigns new values to it that are copies of the values in the range [first, last).
     * @tparam InputIt Type of the input iterator.
     * @param first Iterator to the first element in the range.
     * @param last Iterator to the element past the last element in the range.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::BadInput if first > last, ErrorCode::OutOfMemory if memory
     * allocation failed.
     */
    template <typename InputIt>
    ErrorCode AssignIt(InputIt first, InputIt last);

    /**
     * Accesses the element at the specified index.
     * @param index Index of the element to access.
     * @return Expected containing a reference to the element if the index is valid, ErrorCode::OutOfBounds otherwise.
     */
    Expected<T&, ErrorCode> At(SizeType index);
    Expected<const T&, ErrorCode> At(SizeType index) const;

    /**
     * Accesses the element at the specified index, with no error checking.
     * @param index Index of the element to access.
     * @return Reference to the element at the specified index.
     */
    T& operator[](SizeType index);
    const T& operator[](SizeType index) const;

    /**
     * Accesses the first element in the container.
     * @return Expected containing a reference to the first element if the container is not empty, ErrorCode::OutOfBounds otherwise.
     */
    Expected<T&, ErrorCode> Front();
    Expected<const T&, ErrorCode> Front() const;

    /**
     * Accesses the last element in the container.
     * @return Expected containing a reference to the last element if the container is not empty, ErrorCode::OutOfBounds otherwise.
     */
    Expected<T&, ErrorCode> Back();
    Expected<const T&, ErrorCode> Back() const;

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] SizeType GetSize() const;
    [[nodiscard]] SizeType GetCapacity() const;

    /**
     * Increases the capacity of the container to a value that's greater than or equal to new_capacity. The new capacity will be a power
     * of 2.
     * @param new_capacity New capacity of the container.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Reserve(SizeType new_capacity);

    /**
     * Resizes the container to contain count elements. If count is greater than the current size, the new elements are default-initialized.
     * @param count New size of the container.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(SizeType count);

    /**
     * Resizes the container to contain count elements. If count is greater than the current size, the new elements are copies of the
     * provided value.
     * @param count New size of the container.
     * @param value Value to copy.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Resize(SizeType count, const T& value);

    /**
     * Appends a new element to the end of the container.
     * @param value Value to append.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode PushBack(const T& value);
    ErrorCode PushBack(T&& value);

    /**
     * Prepends a new element to the beginning of the container.
     * @param value Value to prepend.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode PushFront(const T& value);
    ErrorCode PushFront(T&& value);

    /**
     * Inserts a new element at the specified position.
     * @param pos Iterator to the position to insert the element at.
     * @param value Value to insert.
     * @return Expected containing an iterator to the inserted element if the operation was successful, ErrorCode::OutOfBounds if the
     * position is invalid, ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, const T& value);
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, T&& value);

    /**
     * Inserts count copies of the provided value at the specified position.
     * @param pos Iterator to the position to insert the elements at.
     * @param count Number of elements to insert.
     * @param value Value to insert.
     * @return Expected containing an iterator to the first inserted element if the operation was successful, ErrorCode::OutOfBounds if the
     * position is invalid, ErrorCode::OutOfMemory if memory allocation failed.
     */
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, SizeType count, const T& value);

    /**
     * Inserts elements from the range [first, last) at the specified position.
     * @tparam InputIt Type of the input iterator.
     * @param pos Iterator to the position to insert the elements at.
     * @param first Iterator to the first element in the range.
     * @param last Iterator to the element past the last element in the range.
     * @return Expected containing an iterator to the first inserted element if the operation was successful, ErrorCode::OutOfBounds if the
     * position is invalid, ErrorCode::BadInput if first > last, ErrorCode::OutOfMemory if memory allocation failed.
     */
    template <typename InputIt>
    Expected<IteratorType, ErrorCode> InsertIt(ConstIteratorType pos, InputIt first, InputIt last);

    /**
     * Removes the last element from the container.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfBounds if the container is empty.
     */
    ErrorCode PopBack();

    /**
     * Removes the first element from the container.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfBounds if the container is empty.
     */
    ErrorCode PopFront();

    /**
     * Removes all the elements from the container. No deallocation is performed.
     */
    void Clear();

    /**
     * Removes the element at the specified position.
     * @param pos Iterator to the position to remove the element at.
     * @return Expected containing an iterator to the element after the removed element if the operation was successful,
     * ErrorCode::OutOfBounds if the position is invalid.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType pos);
    Expected<IteratorType, ErrorCode> Erase(IteratorType pos);

    /**
     * Removes elements in the range [first, last).
     * @param first Iterator to the first element in the range.
     * @param last Iterator to the element past the last element in the range.
     * @return Expected containing an iterator to the element after the last removed element if the operation was successful,
     * ErrorCode::BadInput if first > last, ErrorCode::OutOfBounds if the range is invalid.
     */
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType first, ConstIteratorType last);
    Expected<IteratorType, ErrorCode> Erase(IteratorType first, IteratorType last);

    IteratorType Begin() { return IteratorType(m_data, m_capacity, m_first, 0); }
    ConstIteratorType Begin() const { return ConstIteratorType(m_data, m_capacity, m_first, 0); }
    ConstIteratorType ConstBegin() const { return ConstIteratorType(m_data, m_capacity, m_first, 0); }
    IteratorType End() { return IteratorType(m_data, m_capacity, m_first, m_size); }
    ConstIteratorType End() const { return ConstIteratorType(m_data, m_capacity, m_first, m_size); }
    ConstIteratorType ConstEnd() const { return ConstIteratorType(m_data, m_capacity, m_first, m_size); }

    // Compatible with std::begin and std::end
    IteratorType begin() { return IteratorType(m_data, m_capacity, m_first, 0); }
    IteratorType end() { return IteratorType(m_data, m_capacity, m_first, m_size); }
    ConstIteratorType begin() const { return ConstIteratorType(m_data, m_capacity, m_first, 0); }
    ConstIteratorType end() const { return ConstIteratorType(m_data, m_capacity, m_first, m_size); }

private:
    void Initialize(const T& value);

    T* Allocate(SizeType count);
    void Deallocate(T* data);

    static constexpr bool IsPowerOf2(u64 value) { return (value != 0) && ((value & (value - 1)) == 0); }

    static constexpr u64 NextPowerOf2(u64 value)
    {
        if (value == 0)
        {
            return 1;
        }
        if (IsPowerOf2(value))
        {
            return value;
        }

        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        value++;
        return value;
    }

    static constexpr u64 Min(u64 a, u64 b) { return a < b ? a : b; }

    static constexpr u64 Max(u64 a, u64 b) { return a > b ? a : b; }

    constexpr static SizeType k_default_capacity = 0;

    AllocatorType m_allocator;
    SizeType m_capacity = 0;
    SizeType m_size = 0;
    SizeType m_first = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

#define TEMPLATE_HEADER template <typename T, typename Allocator>
#define CLASS_HEADER Opal::Deque<T, Allocator>

TEMPLATE_HEADER
CLASS_HEADER::Deque() : m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(const AllocatorType& allocator) : m_allocator(allocator), m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(AllocatorType&& allocator) : m_allocator(Move(allocator)), m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count) : m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count, const AllocatorType& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count, AllocatorType&& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count, const T& value) : m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count, const T& value, const AllocatorType& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(SizeType count, const T& value, AllocatorType&& allocator)
    : m_allocator(Move(allocator)), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(const Deque& other)
    : m_allocator(other.m_allocator), m_capacity(other.m_capacity), m_size(other.m_size), m_first(other.m_first)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        new (m_data + i) T(other.m_data[i]);
        ++i;
        i &= (m_capacity - 1);
    }
}

TEMPLATE_HEADER
CLASS_HEADER::Deque(Deque&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_first(other.m_first), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_first = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
CLASS_HEADER::~Deque()
{
    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    if (m_data)
    {
        Deallocate(m_data);
    }
    m_size = 0;
    m_capacity = 0;
    m_first = 0;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(const Deque& other)
{
    if (this != &other)
    {
        for (SizeType i = m_first, count = 0; count < m_size; count++)
        {
            m_data[i].~T();
            ++i;
            i &= (m_capacity - 1);
        }
        if (m_capacity != other.m_capacity)
        {
            Deallocate(m_data);
        }

        m_allocator = other.m_allocator;
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_first = other.m_first;

        m_data = Allocate(m_capacity);
        for (SizeType i = m_first, count = 0; count < m_size; count++)
        {
            new (m_data + i) T(other.m_data[i]);
            ++i;
            i &= (m_capacity - 1);
        }
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(Deque&& other) noexcept
{
    if (this != &other)
    {
        for (SizeType i = m_first, count = 0; count < m_size; count++)
        {
            m_data[i].~T();
            ++i;
            i &= (m_capacity - 1);
        }
        if (m_data)
        {
            Deallocate(m_data);
        }

        m_allocator = Move(other.m_allocator);
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_first = other.m_first;
        m_data = other.m_data;

        other.m_capacity = 0;
        other.m_size = 0;
        other.m_first = 0;
        other.m_data = nullptr;
    }
    return *this;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const Deque& other) const
{
    if (m_size != other.m_size)
    {
        return false;
    }
    if (m_capacity != other.m_capacity)
    {
        return false;
    }

    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        if (m_data[i] != other.m_data[i])
        {
            return false;
        }
        ++i;
        i &= (m_capacity - 1);
    }
    return true;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(SizeType count)
{
    return Assign(count, T());
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(SizeType count, const T& value)
{
    if (count == 0)
    {
        return ErrorCode::Success;
    }
    for (SizeType i = m_first, iter_count = 0; iter_count < m_size; iter_count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = NextPowerOf2(count);
        m_data = Allocate(m_capacity);
        if (!m_data)
        {
            return ErrorCode::OutOfMemory;
        }
    }
    m_size = count;
    m_first = 0;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (m_data + i) T(value);
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
template <typename InputIt>
Opal::ErrorCode CLASS_HEADER::AssignIt(InputIt first, InputIt last)
{
    if (first > last)
    {
        return ErrorCode::BadInput;
    }
    if (first == last)
    {
        return ErrorCode::Success;
    }
    const SizeType count = last - first;
    for (SizeType i = m_first, iter_count = 0; iter_count < m_size; iter_count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    if (count > m_capacity)
    {
        Deallocate(m_data);
        m_capacity = NextPowerOf2(count);
        m_data = Allocate(m_capacity);
        if (!m_data)
        {
            return ErrorCode::OutOfMemory;
        }
    }
    m_size = count;
    m_first = 0;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (m_data + i) T(*first);
        ++first;
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::At(SizeType index)
{
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    if (final_index >= last)
    {
        return Expected<T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<T&, ErrorCode>(m_data[final_index]);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::At(SizeType index) const
{
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    if (final_index >= last)
    {
        return Expected<const T&, ErrorCode>(ErrorCode::OutOfBounds);
    }
    return Expected<const T&, ErrorCode>(m_data[final_index]);
}

TEMPLATE_HEADER
T& CLASS_HEADER::operator[](SizeType index)
{
    return m_data[(m_first + index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
const T& CLASS_HEADER::operator[](SizeType index) const
{
    return m_data[(m_first + index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::Front()
{
    return At(0);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    return At(0);
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> CLASS_HEADER::Back()
{
    return At(m_size - 1);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    return At(m_size - 1);
}

TEMPLATE_HEADER
bool CLASS_HEADER::IsEmpty() const
{
    return m_size == 0;
}

TEMPLATE_HEADER
typename CLASS_HEADER::SizeType CLASS_HEADER::GetSize() const
{
    return m_size;
}

TEMPLATE_HEADER
typename CLASS_HEADER::SizeType CLASS_HEADER::GetCapacity() const
{
    return m_capacity;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Reserve(SizeType new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return ErrorCode::Success;
    }
    new_capacity = NextPowerOf2(new_capacity);
    T* new_data = Allocate(new_capacity);
    if (!new_data)
    {
        return ErrorCode::OutOfMemory;
    }
    SizeType new_it = 0;
    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        new (new_data + new_it) T(Move(m_data[i]));
        ++i;
        i &= (m_capacity - 1);
        ++new_it;
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
    m_first = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(SizeType count)
{
    return Resize(count, T());
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(SizeType count, const T& value)
{
    if (count == 0)
    {
        Clear();
        return ErrorCode::Success;
    }
    if (count == m_size)
    {
        return ErrorCode::Success;
    }
    if (count < m_size)
    {
        const SizeType first = (m_first + count) & (m_capacity - 1);
        for (SizeType i = first, iter_count = 0; iter_count < m_size; iter_count++)
        {
            m_data[i].~T();
            ++i;
            i &= (m_capacity - 1);
        }
        m_size = count;
        return ErrorCode::Success;
    }
    if (count > m_capacity)
    {
        ErrorCode err = Reserve(NextPowerOf2(count));
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    const SizeType first = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = first, iter_count = m_size; iter_count < count; iter_count++)
    {
        new (m_data + i) T(value);
        ++i;
        i &= (m_capacity - 1);
    }
    m_size = count;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    new (m_data + last) T(value);
    ++m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    new (m_data + last) T(Move(value));
    ++m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushFront(const T& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    m_first = (m_first + m_capacity - 1) & (m_capacity - 1);
    new (m_data + m_first) T(value);
    ++m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PushFront(T&& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return err;
        }
    }
    m_first = (m_first + m_capacity - 1) & (m_capacity - 1);
    new (m_data + m_first) T(Move(value));
    ++m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(ConstIteratorType pos, const T& value)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    const SizeType index = pos - ConstBegin();
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = last, count = 0; count < m_size - index; count++)
    {
        const SizeType prev = (i + m_capacity - 1) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[prev]));
        --i;
        i &= (m_capacity - 1);
    }
    new (m_data + final_index) T(value);
    ++m_size;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(ConstIteratorType pos, T&& value)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity > 0 ? m_capacity * 2 : 1);
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    const SizeType index = pos - ConstBegin();
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = last, count = 0; count < m_size - index; count++)
    {
        const SizeType prev = (i + m_capacity - 1) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[prev]));
        --i;
        i &= (m_capacity - 1);
    }
    new (m_data + final_index) T(Move(value));
    ++m_size;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Insert(ConstIteratorType pos, SizeType count,
                                                                                                      const T& value)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, pos - ConstBegin()));
    }
    if (m_size + count > m_capacity)
    {
        ErrorCode err = Reserve(Max(m_capacity * 2, m_size + count));
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    const SizeType insert_index = (m_first + (pos - ConstBegin())) & (m_capacity - 1);
    const SizeType last = (m_first + m_size - 1) & (m_capacity - 1);
    const SizeType count_to_move = m_size - (pos - ConstBegin());
    for (SizeType i = last, iter_count = 0; iter_count < count_to_move; iter_count++)
    {
        const SizeType next = (i + count) & (m_capacity - 1);
        new (m_data + next) T(Move(m_data[i]));
        --i;
        i &= (m_capacity - 1);
    }
    for (SizeType i = insert_index, iter_count = 0; iter_count < count; iter_count++)
    {
        new (m_data + i) T(value);
        ++i;
        i &= (m_capacity - 1);
    }
    m_size += count;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, pos - ConstBegin()));
}

TEMPLATE_HEADER
template <typename InputIt>
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::InsertIt(ConstIteratorType pos,
                                                                                                        InputIt first, InputIt last)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    const SizeType count = last - first;
    if (count == 0)
    {
        return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, pos - ConstBegin()));
    }
    if (m_size + count > m_capacity)
    {
        ErrorCode err = Reserve(Max(m_capacity * 2, m_size + count));
        if (err != ErrorCode::Success)
        {
            return Expected<IteratorType, ErrorCode>(err);
        }
    }
    const SizeType insert_index = (m_first + (pos - ConstBegin())) & (m_capacity - 1);
    const SizeType move_start_index = (m_first + m_size - 1) & (m_capacity - 1);
    const SizeType count_to_move = m_size - (pos - ConstBegin());
    for (SizeType i = move_start_index, iter_count = 0; iter_count < count_to_move; iter_count++)
    {
        const SizeType next = (i + count) & (m_capacity - 1);
        new (m_data + next) T(Move(m_data[i]));
        --i;
        i &= (m_capacity - 1);
    }
    for (SizeType i = insert_index, iter_count = 0; iter_count < count; iter_count++)
    {
        new (m_data + i) T(*first);
        ++i;
        ++first;
        i &= (m_capacity - 1);
    }
    m_size += count;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, pos - ConstBegin()));
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PopBack()
{
    if (m_size == 0)
    {
        return ErrorCode::OutOfBounds;
    }
    const SizeType last = (m_first + m_size - 1) & (m_capacity - 1);
    m_data[last].~T();
    --m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::PopFront()
{
    if (m_size == 0)
    {
        return ErrorCode::OutOfBounds;
    }
    m_data[m_first].~T();
    m_first = (m_first + 1) & (m_capacity - 1);
    --m_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    m_size = 0;
    m_first = 0;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(ConstIteratorType pos)
{
    if (pos < ConstBegin() || pos >= ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    const SizeType index = pos - ConstBegin();
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    m_data[final_index].~T();
    for (SizeType i = final_index, count = 0; count < m_size - index; count++)
    {
        const SizeType next = (i + 1) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[next]));
        ++i;
        i &= (m_capacity - 1);
    }
    --m_size;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, index == m_size ? index - 1 : index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType pos)
{
    if (pos < Begin() || pos >= End())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    const SizeType index = pos - Begin();
    const SizeType final_index = (m_first + index) & (m_capacity - 1);
    m_data[final_index].~T();
    for (SizeType i = final_index, count = 0; count < m_size - index; count++)
    {
        const SizeType next = (i + 1) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[next]));
        ++i;
        i &= (m_capacity - 1);
    }
    --m_size;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, index == m_size ? index - 1 : index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(ConstIteratorType first,
                                                                                                     ConstIteratorType last)
{
    if (first < ConstBegin() || first > ConstEnd() || last < ConstBegin() || last > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    if (first == last)
    {
        return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, first - ConstBegin()));
    }
    const SizeType first_index = first - ConstBegin();
    const SizeType last_index = last - ConstBegin();
    const SizeType count = last_index - first_index;
    const SizeType first_final_index = (m_first + first_index) & (m_capacity - 1);
    for (SizeType i = first_final_index, iter_count = 0; iter_count < count; iter_count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    const SizeType count_to_move = m_size - last_index;
    for (SizeType i = first_final_index, iter_count = 0; iter_count < count_to_move; iter_count++)
    {
        const SizeType prev = (i + count) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[prev]));
        --i;
        i &= (m_capacity - 1);
    }
    m_size -= count;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, first_index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType first, IteratorType last)
{
    if (first < Begin() || first > End() || last < Begin() || last > End())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::BadInput);
    }
    if (first == last)
    {
        return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, first - Begin()));
    }
    const SizeType first_index = first - Begin();
    const SizeType last_index = last - Begin();
    const SizeType count = last_index - first_index;
    const SizeType first_final_index = (m_first + first_index) & (m_capacity - 1);
    for (SizeType i = first_final_index, iter_count = 0; iter_count < count; iter_count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    const SizeType count_to_move = m_size - last_index;
    for (SizeType i = first_final_index, iter_count = 0; iter_count < count_to_move; iter_count++)
    {
        const SizeType prev = (i + count) & (m_capacity - 1);
        new (m_data + i) T(Move(m_data[prev]));
        --i;
        i &= (m_capacity - 1);
    }
    m_size -= count;
    return Expected<IteratorType, ErrorCode>(IteratorType(m_data, m_capacity, m_first, first_index));
}

TEMPLATE_HEADER
void CLASS_HEADER::Initialize(const T& value)
{
    if (m_capacity == 0)
    {
        return;
    }
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        new (m_data + i) T(value);
    }
}

TEMPLATE_HEADER
T* CLASS_HEADER::Allocate(SizeType count)
{
    return static_cast<T*>(m_allocator.Allocate(count * sizeof(T), alignof(T)));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(T* data)
{
    m_allocator.Deallocate(data);
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyDeque>
#define CLASS_HEADER Opal::DequeIterator<MyDeque>

TEMPLATE_HEADER
CLASS_HEADER::DequeIterator(PointerType data, SizeType capacity, SizeType first, SizeType index)
    : m_data(data), m_capacity(capacity), m_first(first), m_index(index)
{
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const DequeIterator& other) const
{
    return m_data == other.m_data && m_capacity == other.m_capacity && m_first == other.m_first && m_index == other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index > other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index >= other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index < other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index <= other.m_index;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    ++m_index;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    DequeIterator copy = *this;
    ++m_index;
    return copy;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_index;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    DequeIterator copy = *this;
    --m_index;
    return copy;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return DequeIterator(m_data, m_capacity, m_first, m_index + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return DequeIterator(m_data, m_capacity, m_first, m_index - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_index += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_index -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index - other.m_index;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return m_data[(m_first + m_index + n) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return &m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename CLASS_HEADER::DifferenceType n, const DequeIterator<MyDeque>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyDeque>
#define CLASS_HEADER Opal::DequeConstIterator<MyDeque>

TEMPLATE_HEADER
CLASS_HEADER::DequeConstIterator(PointerType data, SizeType capacity, SizeType first, SizeType index)
    : m_data(data), m_capacity(capacity), m_first(first), m_index(index)
{
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const DequeConstIterator& other) const
{
    return m_data == other.m_data && m_capacity == other.m_capacity && m_first == other.m_first && m_index == other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index > other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index >= other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index < other.m_index;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index <= other.m_index;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    ++m_index;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    DequeConstIterator copy = *this;
    ++m_index;
    return copy;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_index;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    DequeConstIterator copy = *this;
    --m_index;
    return copy;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return DequeConstIterator(m_data, m_capacity, m_first, m_index + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return DequeConstIterator(m_data, m_capacity, m_first, m_index - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_index += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_index -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index - other.m_index;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return m_data[(m_first + m_index + n) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return &m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename CLASS_HEADER::DifferenceType n, const DequeConstIterator<MyDeque>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

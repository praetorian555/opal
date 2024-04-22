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

    // TODO: Add docs

    ErrorCode Assign(SizeType count);
    ErrorCode Assign(SizeType count, const T& value);

    template <typename InputIt>
    ErrorCode AssignIt(InputIt first, InputIt last);

    Expected<T&, ErrorCode> At(SizeType index);
    Expected<const T&, ErrorCode> At(SizeType index) const;

    T& operator[](SizeType index);
    const T& operator[](SizeType index) const;

    Expected<T&, ErrorCode> Front();
    Expected<const T&, ErrorCode> Front() const;

    Expected<T&, ErrorCode> Back();
    Expected<const T&, ErrorCode> Back() const;

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] SizeType GetSize() const;
    [[nodiscard]] SizeType GetCapacity() const;

    ErrorCode Reserve(SizeType new_capacity);

    ErrorCode Resize(SizeType count);
    ErrorCode Resize(SizeType count, const T& value);

    ErrorCode PushBack(const T& value);
    ErrorCode PushBack(T&& value);
    ErrorCode PushFront(const T& value);
    ErrorCode PushFront(T&& value);

    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, const T& value);
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, T&& value);
    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, SizeType count, const T& value);

    template <typename InputIt>
    Expected<IteratorType, ErrorCode> InsertIt(ConstIteratorType pos, InputIt first, InputIt last);

    ErrorCode PopBack();
    ErrorCode PopFront();

    void Clear();

    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType pos);
    Expected<IteratorType, ErrorCode> Erase(IteratorType pos);
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

    constexpr static SizeType k_default_capacity = 4;

    AllocatorType m_allocator;
    SizeType m_capacity = 0;
    SizeType m_size = 0;
    SizeType m_first = 0;
    T* m_data;
};

}  // namespace Opal

#define TEMPLATE_HEADER template <typename T, typename Allocator>
#define TEMPLATE_NAMESPACE Opal::Deque<T, Allocator>

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque() : m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(const AllocatorType& allocator) : m_allocator(allocator), m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(AllocatorType&& allocator) : m_allocator(Move(allocator)), m_capacity(k_default_capacity)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count) : m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count, const AllocatorType& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count, AllocatorType&& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(T());
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count, const T& value) : m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count, const T& value, const AllocatorType& allocator)
    : m_allocator(allocator), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(SizeType count, const T& value, AllocatorType&& allocator)
    : m_allocator(Move(allocator)), m_capacity(Max(k_default_capacity, NextPowerOf2(count))), m_size(count)
{
    Initialize(value);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::Deque(const Deque& other)
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
TEMPLATE_NAMESPACE::Deque(Deque&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_first(other.m_first), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_first = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::~Deque()
{
    for (SizeType i = m_first, count = 0; count < m_size; count++)
    {
        m_data[i].~T();
        ++i;
        i &= (m_capacity - 1);
    }
    Deallocate(m_data);
    m_size = 0;
    m_capacity = 0;
    m_first = 0;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator=(const Deque& other)
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
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator=(Deque&& other) noexcept
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
bool TEMPLATE_NAMESPACE::operator==(const Deque& other) const
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
Opal::ErrorCode TEMPLATE_NAMESPACE::Assign(SizeType count)
{
    return Assign(count, T());
}

TEMPLATE_HEADER
Opal::ErrorCode TEMPLATE_NAMESPACE::Assign(SizeType count, const T& value)
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
Opal::ErrorCode TEMPLATE_NAMESPACE::AssignIt(InputIt first, InputIt last)
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
Opal::Expected<T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::At(SizeType index)
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
Opal::Expected<const T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::At(SizeType index) const
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
T& TEMPLATE_NAMESPACE::operator[](SizeType index)
{
    return m_data[(m_first + index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
const T& TEMPLATE_NAMESPACE::operator[](SizeType index) const
{
    return m_data[(m_first + index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::Front()
{
    return At(0);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::Front() const
{
    return At(0);
}

TEMPLATE_HEADER
Opal::Expected<T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::Back()
{
    return At(m_size - 1);
}

TEMPLATE_HEADER
Opal::Expected<const T&, Opal::ErrorCode> TEMPLATE_NAMESPACE::Back() const
{
    return At(m_size - 1);
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::IsEmpty() const
{
    return m_size == 0;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::SizeType TEMPLATE_NAMESPACE::GetSize() const
{
    return m_size;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::SizeType TEMPLATE_NAMESPACE::GetCapacity() const
{
    return m_capacity;
}

TEMPLATE_HEADER
Opal::ErrorCode TEMPLATE_NAMESPACE::Reserve(SizeType new_capacity)
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
Opal::ErrorCode TEMPLATE_NAMESPACE::Resize(SizeType count)
{
    return Resize(count, T());
}

TEMPLATE_HEADER
Opal::ErrorCode TEMPLATE_NAMESPACE::Resize(SizeType count, const T& value)
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PushFront(const T& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PushFront(T&& value)
{
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Insert(ConstIteratorType pos, const T& value)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Insert(ConstIteratorType pos, T&& value)
{
    if (pos < ConstBegin() || pos > ConstEnd())
    {
        return Expected<IteratorType, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (m_size == m_capacity)
    {
        ErrorCode err = Reserve(m_capacity * 2);
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Insert(ConstIteratorType pos, SizeType count,
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::InsertIt(ConstIteratorType pos,
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PopBack()
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
Opal::ErrorCode TEMPLATE_NAMESPACE::PopFront()
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
void TEMPLATE_NAMESPACE::Clear()
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Erase(ConstIteratorType pos)
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Erase(IteratorType pos)
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Erase(ConstIteratorType first,
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
Opal::Expected<typename TEMPLATE_NAMESPACE::IteratorType, Opal::ErrorCode> TEMPLATE_NAMESPACE::Erase(IteratorType first,
                                                                                                     IteratorType last)
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
void TEMPLATE_NAMESPACE::Initialize(const T& value)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        new (m_data + i) T(value);
    }
}

TEMPLATE_HEADER
T* TEMPLATE_NAMESPACE::Allocate(SizeType count)
{
    return static_cast<T*>(m_allocator.Allocate(count * sizeof(T), alignof(T)));
}

TEMPLATE_HEADER
void TEMPLATE_NAMESPACE::Deallocate(T* data)
{
    m_allocator.Deallocate(data);
}

#undef TEMPLATE_HEADER
#undef TEMPLATE_NAMESPACE

#define TEMPLATE_HEADER template <typename MyDeque>
#define TEMPLATE_NAMESPACE Opal::DequeIterator<MyDeque>

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::DequeIterator(PointerType data, SizeType capacity, SizeType first, SizeType index)
    : m_data(data), m_capacity(capacity), m_first(first), m_index(index)
{
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator==(const DequeIterator& other) const
{
    return m_data == other.m_data && m_capacity == other.m_capacity && m_first == other.m_first && m_index == other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator>(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index > other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator>=(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index >= other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator<(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index < other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator<=(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index <= other.m_index;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator++()
{
    ++m_index;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator++(int)
{
    DequeIterator copy = *this;
    ++m_index;
    return copy;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator--()
{
    --m_index;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator--(int)
{
    DequeIterator copy = *this;
    --m_index;
    return copy;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator+(DifferenceType n) const
{
    return DequeIterator(m_data, m_capacity, m_first, m_index + n);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator-(DifferenceType n) const
{
    return DequeIterator(m_data, m_capacity, m_first, m_index - n);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator+=(DifferenceType n)
{
    m_index += n;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator-=(DifferenceType n)
{
    m_index -= n;
    return *this;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::DifferenceType TEMPLATE_NAMESPACE::operator-(const DequeIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index - other.m_index;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::ReferenceType TEMPLATE_NAMESPACE::operator[](DifferenceType n) const
{
    return m_data[(m_first + m_index + n) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::ReferenceType TEMPLATE_NAMESPACE::operator*() const
{
    return m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::PointerType TEMPLATE_NAMESPACE::operator->() const
{
    return &m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE Opal::operator+(typename TEMPLATE_NAMESPACE::DifferenceType n, const DequeIterator<MyDeque>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef TEMPLATE_NAMESPACE

#define TEMPLATE_HEADER template <typename MyDeque>
#define TEMPLATE_NAMESPACE Opal::DequeConstIterator<MyDeque>

TEMPLATE_HEADER
TEMPLATE_NAMESPACE::DequeConstIterator(PointerType data, SizeType capacity, SizeType first, SizeType index)
    : m_data(data), m_capacity(capacity), m_first(first), m_index(index)
{
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator==(const DequeConstIterator& other) const
{
    return m_data == other.m_data && m_capacity == other.m_capacity && m_first == other.m_first && m_index == other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator>(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index > other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator>=(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index >= other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator<(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index < other.m_index;
}

TEMPLATE_HEADER
bool TEMPLATE_NAMESPACE::operator<=(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index <= other.m_index;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator++()
{
    ++m_index;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator++(int)
{
    DequeConstIterator copy = *this;
    ++m_index;
    return copy;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator--()
{
    --m_index;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator--(int)
{
    DequeConstIterator copy = *this;
    --m_index;
    return copy;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator+(DifferenceType n) const
{
    return DequeConstIterator(m_data, m_capacity, m_first, m_index + n);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE TEMPLATE_NAMESPACE::operator-(DifferenceType n) const
{
    return DequeConstIterator(m_data, m_capacity, m_first, m_index - n);
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator+=(DifferenceType n)
{
    m_index += n;
    return *this;
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE& TEMPLATE_NAMESPACE::operator-=(DifferenceType n)
{
    m_index -= n;
    return *this;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::DifferenceType TEMPLATE_NAMESPACE::operator-(const DequeConstIterator& other) const
{
    OPAL_ASSERT(m_data == other.m_data, "Iterators are not comparable");
    return m_index - other.m_index;
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::ReferenceType TEMPLATE_NAMESPACE::operator[](DifferenceType n) const
{
    return m_data[(m_first + m_index + n) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::ReferenceType TEMPLATE_NAMESPACE::operator*() const
{
    return m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
typename TEMPLATE_NAMESPACE::PointerType TEMPLATE_NAMESPACE::operator->() const
{
    return &m_data[(m_first + m_index) & (m_capacity - 1)];
}

TEMPLATE_HEADER
TEMPLATE_NAMESPACE Opal::operator+(typename TEMPLATE_NAMESPACE::DifferenceType n, const DequeConstIterator<MyDeque>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef TEMPLATE_NAMESPACE

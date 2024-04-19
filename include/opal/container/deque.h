#pragma once

#include "opal/allocator.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

//template <typename MyDeque>
//class DequeIterator
//{
//};
//
//template <typename MyDeque>
//class DequeConstIterator
//{
//};

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
    using SizeType = u64;
    using AllocatorType = Allocator;
//    using IteratorType = DequeIterator<Deque<T, Allocator>>;
//    using ConstIteratorType = DequeConstIterator<Deque<T, Allocator>>;

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

    ErrorCode Resize(SizeType count);
    ErrorCode Resize(SizeType count, const T& value);

    ErrorCode PushBack(const T& value);
    ErrorCode PushBack(T&& value);
    ErrorCode PushFront(const T& value);
    ErrorCode PushFront(T&& value);

//    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, const T& value);
//    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, T&& value);
//    Expected<IteratorType, ErrorCode> Insert(ConstIteratorType pos, SizeType count, const T& value);
//
//    template <typename InputIt>
//    Expected<IteratorType, ErrorCode> InsertIt(ConstIteratorType pos, InputIt first, InputIt last);

    ErrorCode PopBack();
    ErrorCode PopFront();

    void Clear();

//    ErrorCode Erase(ConstIteratorType pos);
//    ErrorCode Erase(IteratorType pos);
//    ErrorCode Erase(ConstIteratorType first, ConstIteratorType last);
//    ErrorCode Erase(IteratorType first, IteratorType last);

//    IteratorType Begin();
//    ConstIteratorType Begin() const;
//    ConstIteratorType ConstBegin() const;
//    IteratorType End();
//    ConstIteratorType End() const;
//    ConstIteratorType ConstEnd() const;
//
//    // Compatible with std::begin and std::end
//    IteratorType begin();
//    IteratorType end();
//    ConstIteratorType begin() const;
//    ConstIteratorType end() const;

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

private:
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
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = m_first; i != last;)
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
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = m_first; i != last;)
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
        SizeType last = (m_first + m_size) & (m_capacity - 1);
        for (SizeType i = m_first; i != last;)
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
        last = (m_first + m_size) & (m_capacity - 1);
        for (SizeType i = m_first; i != last;)
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
        const SizeType last = (m_first + m_size) & (m_capacity - 1);
        for (SizeType i = m_first; i != last;)
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
    for (SizeType i = m_first, last = (m_first + m_size) & (m_capacity - 1); i != last;)
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
    const SizeType last = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = m_first; i != last;)
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
    const SizeType last_local = (m_first + m_size) & (m_capacity - 1);
    for (SizeType i = m_first; i != last_local;)
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

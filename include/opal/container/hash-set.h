#pragma once

#include <unordered_set>

#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/error-codes.h"
#include "opal/hash.h"

namespace Opal
{

template <typename HashSetType>
class HashSetIterator
{
public:
    using hash_set_type = HashSetType;
    using value_type = typename hash_set_type::value_type;
    using difference_type = typename hash_set_type::difference_type;
    using reference = typename hash_set_type::reference;
    using pointer = typename hash_set_type::pointer;

    HashSetIterator() = default;
    HashSetIterator(hash_set_type* hash_set, u64 index);

    bool operator==(const HashSetIterator& other) const;
    bool operator>(const HashSetIterator& other) const;
    bool operator>=(const HashSetIterator& other) const;
    bool operator<(const HashSetIterator& other) const;
    bool operator<=(const HashSetIterator& other) const;

    HashSetIterator& operator++();
    HashSetIterator operator++(int);
    HashSetIterator& operator--();
    HashSetIterator operator--(int);

    HashSetIterator operator+(difference_type n) const;
    HashSetIterator operator-(difference_type n) const;
    HashSetIterator& operator+=(difference_type n);
    HashSetIterator& operator-=(difference_type n);

    difference_type operator-(const HashSetIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    HashSetType* m_hash_set = nullptr;
    u64 m_index = 0;
};

template <typename HashSetType>
class HashSetConstIterator
{
public:
    using hash_set_type = HashSetType;
    using value_type = typename hash_set_type::value_type;
    using difference_type = typename hash_set_type::difference_type;
    using reference = typename hash_set_type::reference;
    using pointer = typename hash_set_type::pointer;

    HashSetConstIterator() = default;
    HashSetConstIterator(hash_set_type* hash_set, u64 index);

    bool operator==(const HashSetConstIterator& other) const;
    bool operator>(const HashSetConstIterator& other) const;
    bool operator>=(const HashSetConstIterator& other) const;
    bool operator<(const HashSetConstIterator& other) const;
    bool operator<=(const HashSetConstIterator& other) const;

    HashSetConstIterator& operator++();
    HashSetConstIterator operator++(int);
    HashSetConstIterator& operator--();
    HashSetConstIterator operator--(int);

    HashSetConstIterator operator+(difference_type n) const;
    HashSetConstIterator operator-(difference_type n) const;
    HashSetConstIterator& operator+=(difference_type n);
    HashSetConstIterator& operator-=(difference_type n);

    difference_type operator-(const HashSetConstIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    HashSetType* m_hash_set = nullptr;
    u64 m_index = 0;
};

template <typename KeyType, typename AllocatorType = AllocatorBase>
class HashSet
{
public:
    using key_type = KeyType;
    using value_type = KeyType;
    using size_type = u64;
    using difference_type = i64;
    using allocator_type = AllocatorType;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = HashSetIterator<HashSet<key_type, allocator_type>>;
    using const_iterator = HashSetConstIterator<HashSet<key_type, allocator_type>>;

    constexpr u64 k_group_width = 16;
    constexpr i8 k_control_bitmask_empty = -128;   // 0b10000000;
    constexpr i8 k_control_bitmask_deleted = -2;   // 0b11111110;
    constexpr i8 k_control_bitmask_sentinel = -1;  // 0b11111111;

    explicit HashSet(size_type capacity, allocator_type* allocator = nullptr);

    HashSet(const HashSet& other);
    HashSet(HashSet&& other) noexcept;

    HashSet& operator=(const HashSet& other);
    HashSet& operator=(HashSet&& other) noexcept;

    ErrorCode Reserve(size_type capacity);

    ~HashSet();

    iterator Find(const key_type& key);
    const_iterator Find(const key_type& key) const;

    ErrorCode Insert(const key_type& key, const value_type& value);
    ErrorCode Insert(key_type&& key, value_type&& value);

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

private:
    static u64 GetNextPowerOf2MinusOne(u64 value);
    [[nodiscard]] bool IsControlEmpty(i8 control) const { return control == k_control_bitmask_empty; }
    [[nodiscard]] bool IsControlDeleted(i8 control) const { return control == k_control_bitmask_deleted; }
    [[nodiscard]] static bool IsControlFull(i8 control) { return control >= 0; }
    [[nodiscard]] bool IsControlEmptyOrDeleted(i8 control) const { return control < k_control_bitmask_sentinel; }
    static u64 CalculateHash(const key_type& key) { return CalculateHashFromObject(key); }
    [[nodiscard]] u64 GetHash1(u64 hash) const { return (hash >> 7) & (reinterpret_cast<u64>(m_control_bytes) >> 12); }
    static u64 GetHash2(u64 hash) { return hash & 0x7f; }

    allocator_type* m_allocator = nullptr;
    u8* m_control_bytes = nullptr;
    key_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::HashSet(size_type capacity, allocator_type* allocator)
    : m_allocator(allocator != nullptr ? allocator : &GetDefaultAllocator())
{
    Reserve(capacity < 4 ? 4 : capacity);
}

template <typename KeyType, typename AllocatorType>
Opal::ErrorCode Opal::HashSet<KeyType, AllocatorType>::Reserve(size_type capacity)
{
    // TODO: Handle move of data
    m_capacity = GetNextPowerOf2MinusOne(capacity);
    u64 size_to_allocate = m_capacity + k_group_width + (m_capacity * sizeof(key_type));

    m_control_bytes = static_cast<u8*>(m_allocator->Allocate(size_to_allocate));
    m_slots = reinterpret_cast<key_type*>(m_control_bytes + m_capacity + k_group_width);

    return ErrorCode::Success;
}
template <typename KeyType, typename AllocatorType>
typename Opal::HashSet<KeyType, AllocatorType>::iterator Opal::HashSet<KeyType, AllocatorType>::Find(const key_type& key)
{
    u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash) & m_capacity;
    while (true)
    {
        u8* group = m_control_bytes + offset;
        u64 hash2 = GetHash2(hash);
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
        for (const u32 i : match_mask)
        {
            if (m_slots[offset + i] == key)
            {
                return iterator(this, offset + i);
            }
        }
        if (GetGroupMatchEmpty(group))
        {
            break;
        }
        offset = (offset + k_group_width) & m_capacity;
    }
    return end();
}

template <typename KeyType, typename AllocatorType>
Opal::u64 Opal::HashSet<KeyType, AllocatorType>::GetNextPowerOf2MinusOne(u64 value)
{
    return value != 0 ? ~u64{} >> CountLeadingZeros(value) : 1;
}

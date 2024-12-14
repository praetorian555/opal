#pragma once

#include <unordered_set>

#include "opal/defines.h"

#if defined(OPAL_COMPILER_CLANG) || defined(OPAL_COMPILER_GCC)
#include <emmintrin.h>
#endif

#include "opal/export.h"
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
    HashSetIterator(hash_set_type* hash_set, u64 index) : m_hash_set(hash_set), m_index(index) {}

    bool operator==(const HashSetIterator& other) const { return m_hash_set == other.m_hash_set && m_index == other.m_index; }
    bool operator>(const HashSetIterator& other) const { return m_hash_set == other.m_hash_set && m_index > other.m_index; }
    bool operator>=(const HashSetIterator& other) const { return m_hash_set == other.m_hash_set && m_index >= other.m_index; }
    bool operator<(const HashSetIterator& other) const { return m_hash_set == other.m_hash_set && m_index < other.m_index; }
    bool operator<=(const HashSetIterator& other) const { return m_hash_set == other.m_hash_set && m_index <= other.m_index; }

    HashSetIterator& operator++()
    {
        auto next_it = m_hash_set->FindNextIterator(*this);
        m_index = next_it.GetIndex();
        return *this;
    }
    HashSetIterator operator++(int)
    {
        auto next_it = m_hash_set->FindNextIterator(*this);
        auto rtn_it = *this;
        m_index = next_it.GetIndex();
        return rtn_it;
    }

    reference operator*() const
    {
        return m_hash_set->GetKey(m_index);
    }

    pointer operator->() const
    {
        return &m_hash_set->GetKey(m_index);
    }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

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
    HashSetConstIterator(hash_set_type* hash_set, u64 index) : m_hash_set(hash_set), m_index(index) {}

    bool operator==(const HashSetConstIterator& other) const { return m_hash_set == other.m_hash_set && m_index == other.m_index; }
    bool operator>(const HashSetConstIterator& other) const { return m_hash_set == other.m_hash_set && m_index > other.m_index; }
    bool operator>=(const HashSetConstIterator& other) const { return m_hash_set == other.m_hash_set && m_index >= other.m_index; }
    bool operator<(const HashSetConstIterator& other) const { return m_hash_set == other.m_hash_set && m_index < other.m_index; }
    bool operator<=(const HashSetConstIterator& other) const { return m_hash_set == other.m_hash_set && m_index <= other.m_index; }

    HashSetConstIterator& operator++()
    {
        auto next_it = m_hash_set->FindNextIterator(*this);
        m_index = next_it.GetIndex();
        return *this;
    }
    HashSetConstIterator operator++(int)
    {
        auto next_it = m_hash_set->FindNextIterator(*this);
        auto rtn_it = *this;
        m_index = next_it.GetIndex();
        return rtn_it;
    }

    reference operator*() const
    {
        return m_hash_set->GetKey(m_index);
    }

    pointer operator->() const
    {
        return &m_hash_set->GetKey(m_index);
    }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

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

    constexpr static u64 k_group_width = 16;
    constexpr static i8 k_control_bitmask_empty = -128;   // 0b10000000;
    constexpr static i8 k_control_bitmask_deleted = -2;   // 0b11111110;
    constexpr static i8 k_control_bitmask_sentinel = -1;  // 0b11111111;

    explicit HashSet(size_type capacity, allocator_type* allocator = nullptr);

    HashSet(const HashSet& other);
    HashSet(HashSet&& other) noexcept;

    HashSet& operator=(const HashSet& other);
    HashSet& operator=(HashSet&& other) noexcept;

    ErrorCode Reserve(size_type capacity);

    ~HashSet();

    u64 GetSize() const { return m_size; }
    u64 GetCapacity() const { return m_capacity; }

    iterator Find(const key_type& key);
    const_iterator Find(const key_type& key) const;
    bool Contains(const key_type& key) const;

    ErrorCode Insert(const key_type& key);
    ErrorCode Insert(key_type&& key);

    iterator begin() { return FindFirstIterator(); }
    const_iterator begin() const { return FindFirstIterator(); }
    const_iterator cbegin() const { return FindFirstIterator(); }
    iterator end() { return iterator{this, ~u64{}}; }
    const_iterator end() const { return const_iterator{this, ~u64{}}; }
    const_iterator cend() const { return const_iterator{this, ~u64{}}; }

    iterator FindFirstIterator();
    const_iterator FindFirstIterator() const;

    iterator FindNextIterator(iterator pos);
    const_iterator FindNextIterator(const_iterator pos) const;

    key_type& GetKey(u64 index);
    const key_type& GetKey(u64 index) const;

private:
    static u64 GetNextPowerOf2MinusOne(u64 value);
    [[nodiscard]] static bool IsControlFull(i8 control) { return control >= 0; }
    static u64 CalculateHash(const key_type& key) { return CalculateHashFromObject(key); }
    [[nodiscard]] u64 GetHash1(u64 hash, void* seed) const { return (hash >> 7) & (reinterpret_cast<u64>(seed) >> 12); }
    static i8 GetHash2(u64 hash) { return hash & 0x7f; }
    static BitMask<u32> GetGroupMatch(const i8* group, i8 pattern);
    static BitMask<u32> GetGroupMatchEmpty(const i8* group);
    static BitMask<u32> GetGroupNotFull(const i8* group);
    void SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity);
    static u64 GetGrowthThreshold(u64 capacity) { return (capacity * 7) / 8; }

    allocator_type* m_allocator = nullptr;
    i8* m_control_bytes = nullptr;
    key_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::HashSet(size_type capacity, allocator_type* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    Reserve(capacity < 4 ? 4 : capacity);
}

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::~HashSet()
{
    m_allocator->Free(m_control_bytes);
}

template <typename KeyType, typename AllocatorType>
Opal::ErrorCode Opal::HashSet<KeyType, AllocatorType>::Reserve(size_type capacity)
{
    u64 new_capacity = GetNextPowerOf2MinusOne(capacity);
    u64 new_size = 0;
    u64 size_to_allocate = new_capacity + k_group_width + (new_capacity * sizeof(key_type));

    i8* new_control_bytes = static_cast<i8*>(m_allocator->Alloc(size_to_allocate, 16u));
    memset(new_control_bytes, k_control_bitmask_empty, new_capacity + k_group_width);
    new_control_bytes[new_capacity] = k_control_bitmask_sentinel;
    key_type* new_slots = reinterpret_cast<key_type*>(new_control_bytes + new_capacity + k_group_width);
    memset(new_slots, 0, new_capacity * sizeof(key_type));

    if (m_capacity > 0)
    {
        for (key_type& key : *this)
        {
            u64 hash = CalculateHash(key);
            u64 offset = GetHash1(hash, new_control_bytes) & new_capacity;
            while (true)
            {
                i8* group = new_control_bytes + offset;
                BitMask<u32> not_full_mask = GetGroupNotFull(group);
                if (not_full_mask)
                {
                    u64 slot_index = offset + not_full_mask.GetLowestSetBitIndex();
                    slot_index &= new_capacity;
                    SetControlByte(slot_index, GetHash2(hash), new_control_bytes, new_capacity);
                    new_slots[slot_index] = key;
                    new_size++;
                    break;
                }
                offset = (offset + k_group_width) & new_capacity;
            }
        }
    }

    m_allocator->Free(m_control_bytes);

    m_capacity = new_capacity;
    m_size = new_size;
    m_growth_left = m_capacity - m_size;
    m_control_bytes = new_control_bytes;
    m_slots = new_slots;

    return ErrorCode::Success;
}
template <typename KeyType, typename AllocatorType>
typename Opal::HashSet<KeyType, AllocatorType>::iterator Opal::HashSet<KeyType, AllocatorType>::Find(const key_type& key)
{
    u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        i8 hash2 = GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
        // Iterate over matches by index, if the key from one of them matches we found our target
        for (const u32 i : match_mask)
        {
            const u64 pos = (offset + i) & m_capacity;
            if (m_slots[pos] == key)
            {
                return iterator(this, pos);
            }
        }
        // Since we didn't find a match in this group, and it has at least one empty slot, we are done
        if (GetGroupMatchEmpty(group))
        {
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    return end();
}

template <typename KeyType, typename AllocatorType>
typename Opal::HashSet<KeyType, AllocatorType>::const_iterator Opal::HashSet<KeyType, AllocatorType>::Find(const key_type& key) const
{
    u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        i8 hash2 = GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
        // Iterate over matches by index, if the key from one of them matches we found our target
        for (const u32 i : match_mask)
        {
            const u64 pos = (offset + i) & m_capacity;
            if (m_slots[pos] == key)
            {
                return const_iterator(this, pos);
            }
        }
        // Since we didn't find a match in this group, and it has at least one empty slot, we are done
        if (GetGroupMatchEmpty(group))
        {
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    return cend();
}

template <typename KeyType, typename AllocatorType>
bool Opal::HashSet<KeyType, AllocatorType>::Contains(const key_type& key) const
{
    return Find(key) != cend();
}

template <typename KeyType, typename AllocatorType>
Opal::ErrorCode Opal::HashSet<KeyType, AllocatorType>::Insert(const key_type& key)
{
    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        Reserve(m_capacity + 1);
    }
    u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        i8 hash2 = GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
        // Iterate over matches by index, if the key from one of them matches we found our target
        for (const u32 i : match_mask)
        {
            const u64 pos = (offset + i) & m_capacity;
            if (m_slots[pos] == key)
            {
                return ErrorCode::Success;
            }
        }
        // Since we didn't find a match in this group, and it has at least one empty slot, we are done
        if (GetGroupMatchEmpty(group))
        {
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    // The key is not in the set, find the first free slot to put it in
    offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        const BitMask<u32> not_full_mask = GetGroupNotFull(group);
        if (not_full_mask)
        {
            const u64 slot_index = (offset + not_full_mask.GetLowestSetBitIndex()) & m_capacity;
            SetControlByte(slot_index, GetHash2(hash), m_control_bytes, m_capacity);
            m_slots[slot_index] = key;
            m_size++;
            m_growth_left--;
            return ErrorCode::Success;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    return ErrorCode::InsufficientSpace;
}

template <typename KeyType, typename AllocatorType>
Opal::ErrorCode Opal::HashSet<KeyType, AllocatorType>::Insert(key_type&& key)
{
    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        Reserve(GetNextPowerOf2MinusOne(m_capacity + 1));
    }
    u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        i8 hash2 = GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
        // Iterate over matches by index, if the key from one of them matches we found our target
        for (const u32 i : match_mask)
        {
            const u64 pos = (offset + i) & m_capacity;
            if (m_slots[pos] == key)
            {
                return ErrorCode::Success;
            }
        }
        // Since we didn't find a match in this group, and it has at least one empty slot, we are done
        if (GetGroupMatchEmpty(group))
        {
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    // The key is not in the set, find the first free slot to put it in
    offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        const BitMask<u32> not_full_mask = GetGroupNotFull(group);
        if (not_full_mask)
        {
            const u64 slot_index = (offset + not_full_mask.GetLowestSetBitIndex()) & m_capacity;
            SetControlByte(slot_index, GetHash2(hash));
            m_slots[slot_index] = key;
            m_size++;
            m_growth_left--;
            return ErrorCode::Success;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    return ErrorCode::InsufficientSpace;
}

template <typename KeyType, typename AllocatorType>
Opal::u64 Opal::HashSet<KeyType, AllocatorType>::GetNextPowerOf2MinusOne(u64 value)
{
    return value != 0 ? ~u64{} >> CountLeadingZeros(value) : 1;
}

template <typename KeyType, typename AllocatorType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType, AllocatorType>::GetGroupMatch(const i8* group, i8 pattern)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i match = _mm_set1_epi8(pattern);
    // First compare byte for byte from ctrl and match. If bytes are equal empty 0xFF, if bytes are not equal emit 0x00.
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl))));
}

template <typename KeyType, typename AllocatorType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType, AllocatorType>::GetGroupMatchEmpty(const i8* group)
{
    return GetGroupMatch(group, k_control_bitmask_empty);
}

template <typename KeyType, typename AllocatorType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType, AllocatorType>::GetGroupNotFull(const i8* group)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i special = _mm_set1_epi8(k_control_bitmask_sentinel);
    // First compare byte for byte from special and ctrl. If special byte is greater than ctrl byte emit 0xFF, or 0x00 otherwise..
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, ctrl))));
}

template <typename KeyType, typename AllocatorType>
void Opal::HashSet<KeyType, AllocatorType>::SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity)
{
    control_bytes[index] = hash2;
    constexpr u64 k_cloned_bytes_count = k_group_width - 1;
    // Here we use underflow of unsigned integers to set cloned bytes at the end of m_control_bytes array. If byte targeted by index
    // is not in first 15 bytes we just set that byte again.
    control_bytes[((index - k_cloned_bytes_count) & capacity) + (k_cloned_bytes_count & capacity)] = hash2;
}

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::iterator Opal::HashSet<KeyType, AllocatorType>::FindFirstIterator()
{
    u64 index = 0;
    while (m_control_bytes[index] != k_control_bitmask_sentinel)
    {
        if (IsControlFull(m_control_bytes[index]))
        {
            return iterator(this, index);
        }
        index++;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::const_iterator Opal::HashSet<KeyType, AllocatorType>::FindFirstIterator() const
{
    u64 index = 0;
    while (m_control_bytes[index] != k_control_bitmask_sentinel)
    {
        if (IsControlFull(m_control_bytes[index]))
        {
            return const_iterator(this, index);
        }
        index++;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::iterator Opal::HashSet<KeyType, AllocatorType>::FindNextIterator(HashSet::iterator pos)
{
    if (pos == end())
    {
        return end();
    }
    u64 index = pos.GetIndex() + 1;
    while (m_control_bytes[index] != k_control_bitmask_sentinel)
    {
        if (IsControlFull(m_control_bytes[index]))
        {
            return iterator(this, index);
        }
        index++;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType, typename AllocatorType>
Opal::HashSet<KeyType, AllocatorType>::const_iterator Opal::HashSet<KeyType, AllocatorType>::FindNextIterator(HashSet::const_iterator pos) const
{
    if (pos == cend())
    {
        return cend();
    }
    u64 index = pos.GetIndex() + 1;
    while (m_control_bytes[index] != k_control_bitmask_sentinel)
    {
        if (IsControlFull(m_control_bytes[index]))
        {
            return const_iterator(this, index);
        }
        index++;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType, typename AllocatorType>
typename Opal::HashSet<KeyType, AllocatorType>::key_type& Opal::HashSet<KeyType, AllocatorType>::GetKey(u64 index)
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType, typename AllocatorType>
const typename Opal::HashSet<KeyType, AllocatorType>::key_type& Opal::HashSet<KeyType, AllocatorType>::GetKey(u64 index) const
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

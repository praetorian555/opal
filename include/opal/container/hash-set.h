#pragma once

#include <emmintrin.h>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/error-codes.h"
#include "opal/exceptions.h"
#include "opal/hash.h"
#include "opal/common.h"

namespace Opal
{

/**
 * @brief Forward iterator over the keys of a HashSet, in unspecified order.
 * @tparam HashSetType Set this iterator refers into.
 */
template <typename HashSetType>
class HashSetIterator
{
public:
    using hash_set_type = HashSetType;
    using value_type = typename hash_set_type::value_type;
    using difference_type = typename hash_set_type::difference_type;
    // Keys are handed out read only, see the note on HashSet.
    using reference = typename hash_set_type::const_reference;
    using pointer = typename hash_set_type::const_pointer;

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

    reference operator*() const { return m_hash_set->GetKey(m_index); }

    pointer operator->() const { return &m_hash_set->GetKey(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    HashSetType* m_hash_set = nullptr;
    u64 m_index = 0;
};

/**
 * @brief Forward iterator over the keys of a const HashSet, in unspecified order.
 * @tparam HashSetType Set this iterator refers into.
 */
template <typename HashSetType>
class HashSetConstIterator
{
public:
    using hash_set_type = HashSetType;
    using value_type = typename hash_set_type::value_type;
    using difference_type = typename hash_set_type::difference_type;
    using reference = typename hash_set_type::const_reference;
    using const_reference = typename hash_set_type::const_reference;
    using pointer = typename hash_set_type::const_pointer;

    HashSetConstIterator() = default;
    HashSetConstIterator(const hash_set_type* hash_set, u64 index) : m_hash_set(hash_set), m_index(index) {}

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

    const_reference operator*() const { return m_hash_set->GetKey(m_index); }

    pointer operator->() const { return &m_hash_set->GetKey(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    const HashSetType* m_hash_set = nullptr;
    u64 m_index = 0;
};

/**
 * @brief Unordered collection of unique keys, with average constant time lookup, insertion and removal.
 *
 * `KeyType` needs an equality operator and a `Hasher` specialization. Keys are read only once inserted, since a key that changed would no
 * longer be findable. Any insertion that makes the set grow invalidates every iterator into it, as does Reserve.
 *
 * @tparam KeyType Type of the stored keys.
 */
template <typename KeyType>
class HashSet
{
public:
    using key_type = KeyType;
    using value_type = KeyType;
    using size_type = u64;
    using difference_type = i64;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = HashSetIterator<HashSet>;
    using const_iterator = HashSetConstIterator<HashSet>;

    constexpr static u64 k_group_width = 16;
    // Special value to indicate that the control byte is empty
    constexpr static i8 k_control_bitmask_empty = -128;  // 0b10000000;
    // Special value to indicate that the control byte was deleted
    constexpr static i8 k_control_bitmask_deleted = -2;  // 0b11111110;
    // Special value to indicate that we reach the end of the control bytes
    constexpr static i8 k_control_bitmask_sentinel = -1;  // 0b11111111;

    constexpr static u64 k_default_capacity = 4;

    /**
     * Creates a set able to hold at least `capacity` keys before it has to grow.
     * @param capacity How many keys the set should make room for. Smaller requests are raised to a usable minimum.
     * @param allocator Allocator to use, or nullptr to use the default one.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    explicit HashSet(size_type capacity = k_default_capacity, AllocatorBase* allocator = nullptr);

    HashSet(const HashSet& other) = delete;
    HashSet(HashSet&& other) noexcept;
    HashSet& operator=(const HashSet& other) = delete;
    HashSet& operator=(HashSet&& other) noexcept;

    ~HashSet();

    /**
     * Makes room for at least `capacity` keys. Never makes the set smaller than what it already holds, and does nothing useful when the
     * room is already there. Invalidates every iterator into the set.
     * @param capacity How many keys the set should make room for.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Reserve(size_type capacity);

    /**
     * Creates a set holding a copy of every key in this one.
     * @param allocator Allocator the copy should use, or nullptr to use the same one as this set. Copied keys are given the same allocator.
     * @return The new set.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    HashSet Clone(AllocatorBase* allocator = nullptr) const;

    /** @return Number of keys in the set. */
    [[nodiscard]] u64 GetSize() const { return m_size; }
    /** @return How many keys the set can hold before it has to grow. */
    [[nodiscard]] u64 GetCapacity() const { return m_capacity; }
    /** @return How many more keys can be inserted before the set has to grow. Erasing a key does not give this budget back. */
    [[nodiscard]] u64 GetGrowthLeft() const { return m_growth_left; }
    /** @return True when the set holds no keys. */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    /** @return True when the set holds no keys. */
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Looks for a key.
     * @param key Key to look for.
     * @return Iterator to the key, or end() when the set does not hold it.
     */
    iterator Find(const key_type& key);
    const_iterator Find(const key_type& key) const;

    /**
     * @param key Key to look for.
     * @return True when the set holds the key.
     */
    bool Contains(const key_type& key) const;

    /**
     * Adds a key. A key equal to one already in the set replaces it.
     * @param key Key to add.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Insert(const key_type& key) requires IsPOD<KeyType>;
    ErrorCode Insert(key_type&& key);

    /**
     * Removes a key.
     * @param key Key to remove.
     * @return ErrorCode::Success if the key was removed, ErrorCode::InvalidArgument if the set does not hold it.
     */
    ErrorCode Erase(const key_type& key);

    /**
     * Removes the key an iterator refers to.
     * @param it Iterator into this set.
     * @return ErrorCode::Success if the key was removed, ErrorCode::OutOfBounds if the iterator is not in [begin(), end()),
     * ErrorCode::InvalidArgument if it does not refer to a key.
     */
    ErrorCode Erase(iterator it);
    ErrorCode Erase(const_iterator it);

    /**
     * Removes every key in [first, last).
     * @param first Iterator to the first key to remove.
     * @param last Iterator past the last key to remove.
     * @return ErrorCode::Success if the keys were removed, ErrorCode::OutOfBounds if the range is not a valid range of this set,
     * ErrorCode::InvalidArgument if the range covers a position that does not refer to a key.
     */
    ErrorCode Erase(iterator first, iterator last);
    ErrorCode Erase(const_iterator first, const_iterator last);

    /** Removes every key. Keeps the room the set has already made. */
    void Clear();

    /**
     * Copies every key into an array. The order is unspecified.
     * @return Array holding a copy of every key.
     */
    DynamicArray<key_type> ToArray() const;

    iterator begin() { return FindFirstIterator(); }
    const_iterator begin() const { return FindFirstIterator(); }
    const_iterator cbegin() const { return FindFirstIterator(); }
    iterator end() { return iterator{this, ~u64{}}; }
    const_iterator end() const { return const_iterator{this, ~u64{}}; }
    const_iterator cend() const { return const_iterator{this, ~u64{}}; }

private:
    friend class HashSetIterator<HashSet>;
    friend class HashSetConstIterator<HashSet>;

    iterator FindFirstIterator();
    const_iterator FindFirstIterator() const;

    iterator FindNextIterator(iterator pos);
    const_iterator FindNextIterator(const_iterator pos) const;

    const key_type& GetKey(u64 index) const;

    static u64 GetNextPowerOf2MinusOne(u64 value);
    [[nodiscard]] static bool IsControlFull(i8 control) { return control >= 0; }
    static u64 CalculateHash(const key_type& key) { Hasher<key_type> hasher; return hasher(key); }
    [[nodiscard]] u64 GetHash1(u64 hash, void* seed) const { return (hash >> 7) ^ (reinterpret_cast<u64>(seed) >> 12); }
    static i8 GetHash2(u64 hash) { return hash & 0x7f; }
    static BitMask<u32> GetGroupMatch(const i8* group, i8 pattern);
    static BitMask<u32> GetGroupMatchEmpty(const i8* group);
    static BitMask<u32> GetGroupNotFull(const i8* group);
    void SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity);
    static u64 GetGrowthThreshold(u64 capacity) { return (capacity * 7) / 8; }
    bool FindIndex(const key_type& key, u64& out_index) const;
    void OccupySlot(const key_type& key, u64 index) requires IsPOD<KeyType>;
    void OccupySlot(key_type&& key, u64 index);
    void DeleteSlot(u64 index);
    void DestroyAllKeys();
    ErrorCode Grow();

    AllocatorBase* m_allocator = nullptr;
    i8* m_control_bytes = nullptr;
    key_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal

template <typename KeyType>
Opal::HashSet<KeyType>::HashSet(size_type capacity, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    if (Reserve(capacity) != ErrorCode::Success)
    {
        throw OutOfMemoryException(m_allocator->GetName(), capacity * sizeof(key_type));
    }
}

template <typename KeyType>
Opal::HashSet<KeyType>::HashSet(HashSet&& other) noexcept
    : m_allocator(other.m_allocator),
      m_control_bytes(other.m_control_bytes),
      m_slots(other.m_slots),
      m_capacity(other.m_capacity),
      m_size(other.m_size),
      m_growth_left(other.m_growth_left)
{
    other.m_control_bytes = nullptr;
    other.m_slots = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_growth_left = 0;
}

template <typename KeyType>
Opal::HashSet<KeyType>& Opal::HashSet<KeyType>::operator=(HashSet&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    DestroyAllKeys();
    m_allocator->Free(m_control_bytes);

    m_allocator = other.m_allocator;
    m_control_bytes = other.m_control_bytes;
    m_slots = other.m_slots;
    m_growth_left = other.m_growth_left;
    m_size = other.m_size;
    m_capacity = other.m_capacity;

    other.m_control_bytes = nullptr;
    other.m_slots = nullptr;
    other.m_growth_left = 0;
    other.m_size = 0;
    other.m_capacity = 0;

    return *this;
}

template <typename KeyType>
Opal::HashSet<KeyType>::~HashSet()
{
    DestroyAllKeys();
    m_allocator->Free(m_control_bytes);
}

template <typename KeyType>
void Opal::HashSet<KeyType>::DestroyAllKeys()
{
    if (m_control_bytes == nullptr)
    {
        return;
    }
    for (u64 i = 0; i < m_capacity; ++i)
    {
        if (IsControlFull(m_control_bytes[i]))
        {
            m_slots[i].~key_type();
        }
    }
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Reserve(size_type capacity)
{
    u64 new_capacity = GetNextPowerOf2MinusOne(capacity < k_default_capacity ? k_default_capacity : capacity);
    // Probing walks until it meets a slot that was never occupied, so a table that cannot hold the keys already in the set with room to
    // spare would corrupt them and never terminate. Raise the request rather than honour it.
    while (GetGrowthThreshold(new_capacity) < m_size)
    {
        new_capacity = (new_capacity << 1) | 1;
    }
    u64 new_size = 0;
    // Control bytes and keys share one allocation, so the block has to satisfy whichever of the two needs the stricter alignment, and the
    // keys have to start at an offset that keeps it.
    constexpr u64 k_slot_alignment = alignof(key_type) > 16 ? alignof(key_type) : 16;
    const u64 control_bytes_size = GetNextPowerOf2MinusOne(new_capacity + k_group_width) + 1;
    const u64 slots_offset = (control_bytes_size + k_slot_alignment - 1) & ~(k_slot_alignment - 1);
    const u64 size_to_allocate = slots_offset + (new_capacity * sizeof(key_type));

    i8* new_control_bytes = static_cast<i8*>(m_allocator->Alloc(size_to_allocate, k_slot_alignment));
    if (new_control_bytes == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    memset(new_control_bytes, k_control_bitmask_empty, control_bytes_size);
    new_control_bytes[new_capacity] = k_control_bitmask_sentinel;
    key_type* new_slots = reinterpret_cast<key_type*>(new_control_bytes + slots_offset);

    if (m_control_bytes != nullptr)
    {
        // Walks the slots directly rather than through an iterator, since rehashing moves out of every key it visits and iterators only
        // hand out const keys.
        for (u64 i = 0; i < m_capacity; ++i)
        {
            if (!IsControlFull(m_control_bytes[i]))
            {
                continue;
            }
            key_type& key = m_slots[i];
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
                    new (&new_slots[slot_index]) key_type(Move(key));  // Invokes move constructor on allocated memory
                    new_size++;
                    break;
                }
                offset = (offset + k_group_width) & new_capacity;
            }
            key.~key_type();  // Invokes destructor on the slot we just moved out of
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

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Grow()
{
    // Erasing a key frees a slot but not the growth budget, since the slot it leaves behind still has to be probed through. When those
    // deleted slots, rather than live keys, are what filled the table, reallocate at the same capacity to drop them instead of growing.
    if (m_size + 1 <= GetGrowthThreshold(m_capacity) / 2)
    {
        return Reserve(m_capacity);
    }
    return Reserve(m_capacity + 1);
}

template <typename KeyType>
Opal::HashSet<KeyType> Opal::HashSet<KeyType>::Clone(AllocatorBase* allocator) const
{
    allocator = allocator == nullptr ? m_allocator : allocator;
    HashSet clone(m_capacity, allocator);
    for (const key_type& key : *this)
    {
        clone.Insert(Opal::Clone(key, allocator));
    }
    return clone;
}

template <typename KeyType>
bool Opal::HashSet<KeyType>::FindIndex(const key_type& key, u64& out_index) const
{
    if (m_control_bytes == nullptr)
    {
        return false;
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
                out_index = pos;
                return true;
            }
        }
        // Key doesn't seem to exist so return first available slot position
        BitMask<u32> empty_group = GetGroupMatchEmpty(group);
        if (empty_group)
        {
            out_index = (offset + empty_group.GetLowestSetBitIndex()) & m_capacity;
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
    }
    return false;
}

template <typename KeyType>
typename Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::Find(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        return iterator(this, index);
    }
    return end();
}

template <typename KeyType>
typename Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::Find(const key_type& key) const
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        return const_iterator(this, index);
    }
    return cend();
}

template <typename KeyType>
bool Opal::HashSet<KeyType>::Contains(const key_type& key) const
{
    return Find(key) != cend();
}

template <typename KeyType>
void Opal::HashSet<KeyType>::OccupySlot(const key_type& key, u64 index) requires IsPOD<KeyType>
{
    const u64 hash = CalculateHash(key);
    SetControlByte(index, GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index]) key_type(key);  // Invokes copy constructor on allocated memory
    m_size++;
    m_growth_left--;
}

template <typename KeyType>
void Opal::HashSet<KeyType>::OccupySlot(key_type&& key, u64 index)
{
    const u64 hash = CalculateHash(key);
    SetControlByte(index, GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index]) key_type(Move(key));  // Invokes move constructor on allocated memory
    m_size++;
    m_growth_left--;
}

template <typename KeyType>
void Opal::HashSet<KeyType>::DeleteSlot(u64 index)
{
    m_slots[index].~key_type();
    m_size--;
    SetControlByte(index, k_control_bitmask_deleted, m_control_bytes, m_capacity);
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Insert(const key_type& key) requires IsPOD<KeyType>
{
    u64 index = 0;
    const bool has_key = FindIndex(key, index);
    if (has_key)
    {
        m_slots[index] = key;
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(key, index);
    return ErrorCode::Success;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Insert(key_type&& key)
{
    u64 index = 0;
    const bool has_key = FindIndex(key, index);
    if (has_key)
    {
        m_slots[index] = std::move(key);
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(std::move(key), index);
    return ErrorCode::Success;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(iterator it)
{
    if (it < begin() || it >= end())
    {
        return ErrorCode::OutOfBounds;
    }
    const u64 index = it.GetIndex();
    if (IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(const_iterator it)
{
    if (it < cbegin() || it >= cend())
    {
        return ErrorCode::OutOfBounds;
    }
    const u64 index = it.GetIndex();
    if (IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(iterator first, iterator last)
{
    if (first < begin() || first > end())
    {
        return ErrorCode::OutOfBounds;
    }
    if (last < first || last > end())
    {
        return ErrorCode::OutOfBounds;
    }
    for (auto it = first; it != last; ++it)
    {
        const u64 index = it.GetIndex();
        if (IsControlFull(m_control_bytes[index]))
        {
            DeleteSlot(index);
        }
        else
        {
            return ErrorCode::InvalidArgument;
        }
    }
    return ErrorCode::Success;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(const_iterator first, const_iterator last)
{
    if (first < cbegin() || first > cend())
    {
        return ErrorCode::OutOfBounds;
    }
    if (last < first || last > cend())
    {
        return ErrorCode::OutOfBounds;
    }
    for (auto it = first; it != last; ++it)
    {
        const u64 index = it.GetIndex();
        if (IsControlFull(m_control_bytes[index]))
        {
            DeleteSlot(index);
        }
        else
        {
            return ErrorCode::InvalidArgument;
        }
    }
    return ErrorCode::Success;
}

template <typename KeyType>
void Opal::HashSet<KeyType>::Clear()
{
    if (m_control_bytes == nullptr)
    {
        return;
    }
    DestroyAllKeys();
    memset(m_control_bytes, k_control_bitmask_empty, m_capacity + k_group_width);
    m_control_bytes[m_capacity] = k_control_bitmask_sentinel;
    m_growth_left = m_capacity;
    m_size = 0;
}

template <typename KeyType>
Opal::u64 Opal::HashSet<KeyType>::GetNextPowerOf2MinusOne(u64 value)
{
    return value != 0 ? ~u64{} >> CountLeadingZeros(value) : 1;
}

template <typename KeyType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType>::GetGroupMatch(const i8* group, i8 pattern)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i match = _mm_set1_epi8(pattern);
    // First compare byte for byte from ctrl and match. If bytes are equal emit 0xFF, if bytes are not equal emit 0x00.
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl))));
}

template <typename KeyType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType>::GetGroupMatchEmpty(const i8* group)
{
    return GetGroupMatch(group, k_control_bitmask_empty);
}

template <typename KeyType>
Opal::BitMask<Opal::u32> Opal::HashSet<KeyType>::GetGroupNotFull(const i8* group)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i special = _mm_set1_epi8(k_control_bitmask_sentinel);
    // First compare byte for byte from special and ctrl. If special byte is greater than ctrl byte emit 0xFF, or 0x00 otherwise..
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, ctrl))));
}

template <typename KeyType>
void Opal::HashSet<KeyType>::SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity)
{
    // Here we simply set the control byte at the index to hash2.
    control_bytes[index] = hash2;
    constexpr u64 k_cloned_bytes_count = k_group_width - 1;
    // We need to figure out if we need to clone the byte at the end of the group. If byte is in the first
    // k_group_width - 1 we use unsigned integer underflow to set the cloned byte at the end of the group.
    // If byte is not in the first k_group_width - 1 we just set the byte again.
    control_bytes[((index - k_cloned_bytes_count) & capacity) + (k_cloned_bytes_count & capacity)] = hash2;
}

template <typename KeyType>
Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::FindFirstIterator()
{
    if (m_control_bytes == nullptr)
    {
        return iterator(this, ~u64{});
    }
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

template <typename KeyType>
Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::FindFirstIterator() const
{
    if (m_control_bytes == nullptr)
    {
        return const_iterator(this, ~u64{});
    }
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

template <typename KeyType>
Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::FindNextIterator(iterator pos)
{
    if (pos == end() || m_control_bytes == nullptr)
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
        ++index;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType>
Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::FindNextIterator(
    HashSet::const_iterator pos) const
{
    if (pos == cend() || m_control_bytes == nullptr)
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
        ++index;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType>
const typename Opal::HashSet<KeyType>::key_type& Opal::HashSet<KeyType>::GetKey(u64 index) const
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType>
Opal::DynamicArray<KeyType> Opal::HashSet<KeyType>::ToArray() const
{
    DynamicArray<key_type> result;
    result.Reserve(m_size);
    for (const key_type& key : *this)
    {
        result.PushBack(Opal::Clone(key, m_allocator));
    }
    return result;
}

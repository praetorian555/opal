#pragma once

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/clonable-base.h"
#include "opal/common.h"
#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-table-base.h"
#include "opal/error-codes.h"
#include "opal/exceptions.h"
#include "opal/hash.h"

namespace Opal
{

/**
 * @brief A key and a value held together. What a HashMap stores and what its iterators refer to.
 * @tparam KeyType Type of the key.
 * @tparam ValueType Type of the value.
 */
template <typename KeyType, typename ValueType>
struct Pair
{
    KeyType key;
    ValueType value;

    Pair Clone(AllocatorBase* allocator = nullptr) const
    {
        return Pair{Opal::Clone(key, allocator), Opal::Clone(value, allocator)};
    }
};

/**
 * @brief Forward iterator over the pairs of a HashMap, in unspecified order.
 * @tparam HashMapClass Map this iterator refers into.
 */
template <typename HashMapClass>
class HashMapIterator
{
public:
    using hash_set_type = HashMapClass;
    using key_type = HashMapClass::key_type;
    using value_type = hash_set_type::value_type;
    using pair_type = hash_set_type::pair_type;
    using difference_type = hash_set_type::difference_type;
    using reference = hash_set_type::reference;
    using pointer = hash_set_type::pointer;

    HashMapIterator() = default;
    HashMapIterator(hash_set_type* hash_set, u64 index) : m_hash_map(hash_set), m_index(index) {}

    bool operator==(const HashMapIterator& other) const { return m_hash_map == other.m_hash_map && m_index == other.m_index; }
    bool operator>(const HashMapIterator& other) const { return m_hash_map == other.m_hash_map && m_index > other.m_index; }
    bool operator>=(const HashMapIterator& other) const { return m_hash_map == other.m_hash_map && m_index >= other.m_index; }
    bool operator<(const HashMapIterator& other) const { return m_hash_map == other.m_hash_map && m_index < other.m_index; }
    bool operator<=(const HashMapIterator& other) const { return m_hash_map == other.m_hash_map && m_index <= other.m_index; }

    HashMapIterator& operator++()
    {
        auto next_it = m_hash_map->FindNextIterator(*this);
        m_index = next_it.GetIndex();
        return *this;
    }
    HashMapIterator operator++(int)
    {
        auto next_it = m_hash_map->FindNextIterator(*this);
        auto rtn_it = *this;
        m_index = next_it.GetIndex();
        return rtn_it;
    }

    // The pair is handed out whole so that values stay writable, which leaves the key writable too. Do not write it, see the note on
    // HashMap. GetKey is the accessor that enforces the rule.
    pair_type& operator*() const { return m_hash_map->Get(m_index); }
    pair_type* operator->() const { return &m_hash_map->Get(m_index); }
    const key_type& GetKey() const { return m_hash_map->GetKey(m_index); }
    value_type& GetValue() const { return m_hash_map->GetValue(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    HashMapClass* m_hash_map = nullptr;
    u64 m_index = 0;
};

/**
 * @brief Forward iterator over the pairs of a const HashMap, in unspecified order.
 * @tparam HashMapClass Map this iterator refers into.
 */
template <typename HashMapClass>
class HashMapConstIterator
{
public:
    using hash_set_type = HashMapClass;
    using key_type = HashMapClass::key_type;
    using value_type = hash_set_type::value_type;
    using pair_type = hash_set_type::pair_type;
    using difference_type = hash_set_type::difference_type;
    using reference = hash_set_type::reference;
    using const_reference = hash_set_type::const_reference;
    using pointer = hash_set_type::pointer;

    HashMapConstIterator() = default;
    HashMapConstIterator(const hash_set_type* hash_set, u64 index) : m_hash_map(hash_set), m_index(index) {}

    bool operator==(const HashMapConstIterator& other) const { return m_hash_map == other.m_hash_map && m_index == other.m_index; }
    bool operator>(const HashMapConstIterator& other) const { return m_hash_map == other.m_hash_map && m_index > other.m_index; }
    bool operator>=(const HashMapConstIterator& other) const { return m_hash_map == other.m_hash_map && m_index >= other.m_index; }
    bool operator<(const HashMapConstIterator& other) const { return m_hash_map == other.m_hash_map && m_index < other.m_index; }
    bool operator<=(const HashMapConstIterator& other) const { return m_hash_map == other.m_hash_map && m_index <= other.m_index; }

    HashMapConstIterator& operator++()
    {
        auto next_it = m_hash_map->FindNextIterator(*this);
        m_index = next_it.GetIndex();
        return *this;
    }
    HashMapConstIterator operator++(int)
    {
        auto next_it = m_hash_map->FindNextIterator(*this);
        auto rtn_it = *this;
        m_index = next_it.GetIndex();
        return rtn_it;
    }

    const pair_type& operator*() const { return m_hash_map->Get(m_index); }
    const pair_type* operator->() const { return &m_hash_map->Get(m_index); }
    const key_type& GetKey() const { return m_hash_map->GetKey(m_index); }
    const value_type& GetValue() const { return m_hash_map->GetValue(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    const HashMapClass* m_hash_map = nullptr;
    u64 m_index = 0;
};

/**
 * @brief Unordered collection of values stored under unique keys, with average constant time lookup, insertion and removal.
 *
 * `KeyType` needs an equality operator and a `Hasher` specialization. Values may be written through an iterator, keys may not: a key that
 * changed would no longer be findable. Any insertion that makes the map grow invalidates every iterator into it, as does Reserve.
 *
 * @tparam KeyType Type of the stored keys.
 * @tparam ValueType Type of the stored values.
 */
template <typename KeyType, typename ValueType>
class HashMap
{
public:
    using key_type = KeyType;
    using value_type = ValueType;
    using pair_type = Pair<KeyType, ValueType>;
    using size_type = u64;
    using difference_type = i64;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = HashMapIterator<HashMap>;
    using const_iterator = HashMapConstIterator<HashMap>;

    /** Smallest number of pairs a map is built for. */
    constexpr static u64 k_default_capacity = Impl::k_default_capacity;

    /**
     * Creates a map able to hold at least `capacity` pairs before it has to grow.
     * @param capacity How many pairs the map should make room for. Smaller requests are raised to a usable minimum.
     * @param allocator Allocator to use, or nullptr to use the default one.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    explicit HashMap(size_type capacity = k_default_capacity, AllocatorBase* allocator = nullptr);

    /**
     * Creates a map holding a copy of every given pair. Later pairs replace earlier ones with an equal key.
     * @param pairs Pairs to copy in. Copies are given the map's allocator.
     * @param allocator Allocator to use, or nullptr to use the default one.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    HashMap(const ArrayView<Pair<KeyType, ValueType>>& pairs, AllocatorBase* allocator = nullptr);
    HashMap(std::initializer_list<pair_type> pairs, AllocatorBase* allocator = nullptr);

    HashMap(const HashMap& other) = delete;
    HashMap(HashMap&& other) noexcept;

    ~HashMap();

    HashMap& operator=(const HashMap& other) = delete;
    HashMap& operator=(HashMap&& other) noexcept;

    /**
     * Creates a map holding a copy of every pair in this one.
     * @param allocator Allocator the copy should use, or nullptr to use the same one as this map. Copied keys and values are given the
     * same allocator.
     * @return The new map.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    HashMap Clone(AllocatorBase* allocator = nullptr) const;

    /**
     * Makes room for at least `capacity` pairs. Never makes the map smaller than what it already holds, and does nothing useful when the
     * room is already there. Invalidates every iterator into the map.
     * @param capacity How many pairs the map should make room for.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Reserve(size_type capacity);

    /** @return Number of pairs in the map. */
    [[nodiscard]] u64 GetSize() const { return m_size; }
    /** @return How many pairs the map can hold before it has to grow. */
    [[nodiscard]] u64 GetCapacity() const { return m_capacity; }
    /** @return How many more pairs can be inserted before the map has to grow. Erasing a pair does not give this budget back. */
    [[nodiscard]] u64 GetGrowthLeft() const { return m_growth_left; }
    /** @return True when the map holds no pairs. */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    /** @return True when the map holds no pairs. */
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Looks for the pair with the given key.
     * @param key Key to look for.
     * @return Iterator to the pair, or end() when the map does not hold the key.
     */
    iterator Find(const key_type& key);
    const_iterator Find(const key_type& key) const;

    /**
     * @param key Key to look for.
     * @return True when the map holds the key.
     */
    bool Contains(const key_type& key) const;

    /**
     * Reads the value stored under a key. Check with Contains or Find when the key might be absent.
     * @param key Key to look for.
     * @return The value stored under the key.
     * @throw OutOfBoundsException when the map does not hold the key.
     */
    value_type& GetValue(const key_type& key);
    const value_type& GetValue(const key_type& key) const;

    /**
     * Stores a value under a key, replacing the pair already stored under an equal key.
     * @param key Key to store under.
     * @param value Value to store.
     * @return ErrorCode::Success if the operation was successful, ErrorCode::OutOfMemory if memory allocation failed.
     */
    ErrorCode Insert(const key_type& key, const value_type& value)
        requires(IsPOD<key_type> && IsPOD<value_type>);
    ErrorCode Insert(key_type&& key, value_type&& value);
    ErrorCode Insert(const key_type& key, value_type&& value)
        requires IsPOD<key_type>;
    ErrorCode Insert(key_type&& key, const value_type& value)
        requires IsPOD<value_type>;

    /**
     * Removes the pair stored under a key.
     * @param key Key to remove.
     * @return ErrorCode::Success if the pair was removed, ErrorCode::InvalidArgument if the map does not hold the key.
     */
    ErrorCode Erase(const key_type& key);

    /**
     * Removes the pair an iterator refers to.
     * @param it Iterator into this map.
     * @return ErrorCode::Success if the pair was removed, ErrorCode::OutOfBounds if the iterator is not in [begin(), end()),
     * ErrorCode::InvalidArgument if it does not refer to a pair.
     */
    ErrorCode Erase(iterator it);
    ErrorCode Erase(const_iterator it);

    /**
     * Removes every pair in [first, last).
     * @param first Iterator to the first pair to remove.
     * @param last Iterator past the last pair to remove.
     * @return ErrorCode::Success if the pairs were removed, ErrorCode::OutOfBounds if the range is not a valid range of this map,
     * ErrorCode::InvalidArgument if the range covers a position that does not refer to a pair.
     */
    ErrorCode Erase(iterator first, iterator last);
    ErrorCode Erase(const_iterator first, const_iterator last);

    /** Removes every pair. Keeps the room the map has already made. */
    void Clear();

    /**
     * Copies the contents into an array. The order is unspecified, and is the same for all three.
     * @return Array holding a copy of every pair, key or value.
     */
    DynamicArray<pair_type> ToArray() const;
    DynamicArray<key_type> ToArrayOfKeys() const;
    DynamicArray<value_type> ToArrayOfValues() const;

    iterator begin() { return FindFirstIterator(); }
    const_iterator begin() const { return FindFirstIterator(); }
    const_iterator cbegin() const { return FindFirstIterator(); }
    iterator end() { return iterator{this, ~u64{}}; }
    const_iterator end() const { return const_iterator{this, ~u64{}}; }
    const_iterator cend() const { return const_iterator{this, ~u64{}}; }

private:
    friend class HashMapIterator<HashMap>;
    friend class HashMapConstIterator<HashMap>;

    iterator FindFirstIterator();
    const_iterator FindFirstIterator() const;

    iterator FindNextIterator(iterator pos);
    const_iterator FindNextIterator(const_iterator pos) const;

    pair_type& Get(u64 index);
    const pair_type& Get(u64 index) const;
    key_type& GetKey(u64 index);
    const key_type& GetKey(u64 index) const;
    value_type& GetValue(u64 index);
    const value_type& GetValue(u64 index) const;

    static u64 CalculateHash(const key_type& key)
    {
        Hasher<key_type> hasher;
        return hasher(key);
    }
    bool FindIndex(const key_type& key, u64& out_index) const;
    void OccupySlot(const key_type& key, const value_type& value, u64 index)
        requires(IsPOD<key_type> && IsPOD<value_type>);
    void OccupySlot(key_type&& key, value_type&& value, u64 index);
    void OccupySlot(const key_type& key, value_type&& value, u64 index)
        requires IsPOD<key_type>;
    void OccupySlot(key_type&& key, const value_type& value, u64 index)
        requires IsPOD<value_type>;
    void DeleteSlot(u64 index);
    void DestroyAllPairs();
    ErrorCode Grow();

    AllocatorBase* m_allocator = nullptr;
    i8* m_control_bytes = nullptr;
    pair_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal






template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(size_type capacity, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    if (Reserve(capacity) != ErrorCode::Success)
    {
        throw OutOfMemoryException(m_allocator->GetName(), capacity * sizeof(pair_type));
    }
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(const ArrayView<Pair<KeyType, ValueType>>& pairs, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    if (Reserve(pairs.GetSize()) != ErrorCode::Success)
    {
        throw OutOfMemoryException(m_allocator->GetName(), pairs.GetSize() * sizeof(pair_type));
    }
    for (const auto& pair : pairs)
    {
        if (Insert(Opal::Clone(pair.key, m_allocator), Opal::Clone(pair.value, m_allocator)) != ErrorCode::Success)
        {
            throw OutOfMemoryException(m_allocator->GetName(), pairs.GetSize() * sizeof(pair_type));
        }
    }
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(std::initializer_list<pair_type> pairs, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    if (Reserve(pairs.size()) != ErrorCode::Success)
    {
        throw OutOfMemoryException(m_allocator->GetName(), static_cast<u64>(pairs.size() * sizeof(pair_type)));
    }
    for (const auto& pair : pairs)
    {
        if (Insert(Opal::Clone(pair.key, m_allocator), Opal::Clone(pair.value, m_allocator)) != ErrorCode::Success)
        {
            throw OutOfMemoryException(m_allocator->GetName(), static_cast<u64>(pairs.size() * sizeof(pair_type)));
        }
    }
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(HashMap&& other) noexcept
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

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>& Opal::HashMap<KeyType, ValueType>::operator=(HashMap&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    DestroyAllPairs();
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

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::~HashMap()
{
    DestroyAllPairs();
    m_allocator->Free(m_control_bytes);
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::DestroyAllPairs()
{
    if (m_control_bytes == nullptr)
    {
        return;
    }
    for (u64 i = 0; i < m_capacity; ++i)
    {
        if (Impl::IsControlFull(m_control_bytes[i]))
        {
            m_slots[i].key.~KeyType();
            m_slots[i].value.~ValueType();
        }
    }
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType> Opal::HashMap<KeyType, ValueType>::Clone(AllocatorBase* allocator) const
{
    allocator = allocator == nullptr ? m_allocator : allocator;
    HashMap clone(m_capacity, allocator);
    for (const auto& pair : *this)
    {
        clone.Insert(Opal::Clone(pair.key, allocator), Opal::Clone(pair.value, allocator));
    }
    return clone;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Reserve(size_type capacity)
{
    u64 new_capacity = Impl::GetNextPowerOf2MinusOne(capacity < k_default_capacity ? k_default_capacity : capacity);
    // Probing walks until it meets a slot that was never occupied, so a table that cannot hold the pairs already in the map with room to
    // spare would corrupt them and never terminate. Raise the request rather than honour it.
    while (Impl::GetGrowthThreshold(new_capacity) < m_size)
    {
        new_capacity = (new_capacity << 1) | 1;
    }
    u64 new_size = 0;
    // Control bytes and pairs share one allocation, so the block has to satisfy whichever of the two needs the stricter alignment, and the
    // pairs have to start at an offset that keeps it.
    constexpr u64 k_slot_alignment = alignof(pair_type) > 16 ? alignof(pair_type) : 16;
    const u64 control_bytes_size = Impl::GetNextPowerOf2MinusOne(new_capacity + Impl::k_group_width) + 1;
    const u64 slots_offset = (control_bytes_size + k_slot_alignment - 1) & ~(k_slot_alignment - 1);
    const u64 size_to_allocate = slots_offset + (new_capacity * sizeof(pair_type));

    i8* new_control_bytes = static_cast<i8*>(m_allocator->Alloc(size_to_allocate, k_slot_alignment));
    if (new_control_bytes == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    memset(new_control_bytes, Impl::k_control_empty, control_bytes_size);
    new_control_bytes[new_capacity] = Impl::k_control_sentinel;
    pair_type* new_slots = reinterpret_cast<pair_type*>(new_control_bytes + slots_offset);

    if (m_capacity > 0)
    {
        for (pair_type& pair : *this)
        {
            const u64 hash = CalculateHash(pair.key);
            u64 offset = Impl::GetHash1(hash, new_control_bytes) & new_capacity;
            while (true)
            {
                const i8* group = new_control_bytes + offset;
                if (BitMask<u32> not_full_mask = Impl::GetGroupNotFull(group))
                {
                    u64 slot_index = offset + not_full_mask.GetLowestSetBitIndex();
                    slot_index &= new_capacity;
                    Impl::SetControlByte(slot_index, Impl::GetHash2(hash), new_control_bytes, new_capacity);
                    // Invokes move constructors on allocated memory
                    new (&new_slots[slot_index].key) KeyType(Move(pair.key));
                    new (&new_slots[slot_index].value) ValueType(Move(pair.value));
                    new_size++;
                    break;
                }
                offset = (offset + Impl::k_group_width) & new_capacity;
            }
            // Invokes destructors on the slot we just moved out of
            pair.key.~KeyType();
            pair.value.~ValueType();
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

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Grow()
{
    // Erasing a pair frees a slot but not the growth budget, since the slot it leaves behind still has to be probed through. When those
    // deleted slots, rather than live pairs, are what filled the table, reallocate at the same capacity to drop them instead of growing.
    if (m_size + 1 <= Impl::GetGrowthThreshold(m_capacity) / 2)
    {
        return Reserve(m_capacity);
    }
    return Reserve(m_capacity + 1);
}

template <typename KeyType, typename ValueType>
bool Opal::HashMap<KeyType, ValueType>::FindIndex(const key_type& key, u64& out_index) const
{
    if (m_control_bytes == nullptr)
    {
        return false;
    }
    const u64 hash = CalculateHash(key);
    u64 offset = Impl::GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        const i8* group = m_control_bytes + offset;
        const i8 hash2 = Impl::GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = Impl::GetGroupMatch(group, hash2);
        // Iterate over matches by index, if the key from one of them matches we found our target
        for (const u32 i : match_mask)
        {
            const u64 pos = (offset + i) & m_capacity;
            if (m_slots[pos].key == key)
            {
                out_index = pos;
                return true;
            }
        }
        // Key doesn't seem to exist so return first available slot position
        if (BitMask<u32> empty_group = Impl::GetGroupMatchEmpty(group))
        {
            out_index = (offset + empty_group.GetLowestSetBitIndex()) & m_capacity;
            break;
        }
        // Move to the next group
        offset = (offset + Impl::k_group_width) & m_capacity;
    }
    return false;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::iterator Opal::HashMap<KeyType, ValueType>::Find(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        return iterator(this, index);
    }
    return end();
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::const_iterator Opal::HashMap<KeyType, ValueType>::Find(const key_type& key) const
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        return const_iterator(this, index);
    }
    return cend();
}

template <typename KeyType, typename ValueType>
bool Opal::HashMap<KeyType, ValueType>::Contains(const key_type& key) const
{
    return Find(key) != cend();
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::OccupySlot(const key_type& key, const value_type& value, u64 index)
    requires(IsPOD<key_type> && IsPOD<value_type>)
{
    const u64 hash = CalculateHash(key);
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index].key) KeyType(key);
    new (&m_slots[index].value) ValueType(value);
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::OccupySlot(key_type&& key, value_type&& value, u64 index)
{
    const u64 hash = CalculateHash(key);
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index].key) KeyType(Move(key));
    new (&m_slots[index].value) ValueType(Move(value));
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::OccupySlot(const key_type& key, value_type&& value, u64 index)
    requires IsPOD<key_type>
{
    const u64 hash = CalculateHash(key);
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index].key) KeyType(key);
    new (&m_slots[index].value) ValueType(Move(value));
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::OccupySlot(key_type&& key, const value_type& value, u64 index)
    requires IsPOD<value_type>
{
    const u64 hash = CalculateHash(key);
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index].key) KeyType(Move(key));
    new (&m_slots[index].value) ValueType(value);
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::DeleteSlot(u64 index)
{
    m_slots[index].key.~key_type();
    m_slots[index].value.~value_type();
    m_size--;
    Impl::SetControlByte(index, Impl::k_control_deleted, m_control_bytes, m_capacity);
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(const key_type& key)
{
    iterator it = Find(key);
    if (it != end())
    {
        return it.GetValue();
    }
    throw OutOfBoundsException("Key not found");
}

template <typename KeyType, typename ValueType>
const typename Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(const key_type& key) const
{
    const_iterator it = Find(key);
    if (it != cend())
    {
        return it.GetValue();
    }
    throw OutOfBoundsException("Key not found");
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Insert(const key_type& key, const value_type& value)
    requires(IsPOD<key_type> && IsPOD<value_type>)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = key;
        m_slots[index].value = value;
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= Impl::GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(key, value, index);
    return ErrorCode::Success;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Insert(key_type&& key, value_type&& value)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = Move(key);
        m_slots[index].value = Move(value);
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= Impl::GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(Move(key), Move(value), index);
    return ErrorCode::Success;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Insert(const key_type& key, value_type&& value)
    requires IsPOD<key_type>
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = key;
        m_slots[index].value = Move(value);
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= Impl::GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(key, Move(value), index);
    return ErrorCode::Success;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Insert(key_type&& key, const value_type& value)
    requires IsPOD<value_type>
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = Move(key);
        m_slots[index].value = value;
        return ErrorCode::Success;
    }

    if (m_capacity - m_growth_left >= Impl::GetGrowthThreshold(m_capacity))
    {
        const ErrorCode status = Grow();
        if (status != ErrorCode::Success)
        {
            return status;
        }
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(Move(key), value, index);
    return ErrorCode::Success;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Erase(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Erase(iterator it)
{
    if (it < begin() || it >= end())
    {
        return ErrorCode::OutOfBounds;
    }
    const u64 index = it.GetIndex();
    if (Impl::IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Erase(const_iterator it)
{
    if (it < cbegin() || it >= cend())
    {
        return ErrorCode::OutOfBounds;
    }
    const u64 index = it.GetIndex();
    if (Impl::IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Erase(iterator first, iterator last)
{
    if (first < begin() || first > end() || last < first || last > end())
    {
        return ErrorCode::OutOfBounds;
    }
    for (auto it = first; it != last; ++it)
    {
        const u64 index = it.GetIndex();
        if (Impl::IsControlFull(m_control_bytes[index]))
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

template <typename KeyType, typename ValueType>
Opal::ErrorCode Opal::HashMap<KeyType, ValueType>::Erase(const_iterator first, const_iterator last)
{
    if (first < cbegin() || first > cend() || last < first || last > cend())
    {
        return ErrorCode::OutOfBounds;
    }
    for (auto it = first; it != last; ++it)
    {
        const u64 index = it.GetIndex();
        if (Impl::IsControlFull(m_control_bytes[index]))
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

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Clear()
{
    if (m_control_bytes == nullptr)
    {
        return;
    }
    DestroyAllPairs();
    memset(m_control_bytes, Impl::k_control_empty, m_capacity + Impl::k_group_width);
    m_control_bytes[m_capacity] = Impl::k_control_sentinel;
    m_growth_left = m_capacity;
    m_size = 0;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<Opal::Pair<KeyType, ValueType>> Opal::HashMap<KeyType, ValueType>::ToArray() const
{
    DynamicArray<pair_type> result;
    result.Reserve(m_size);
    for (const auto& pair : *this)
    {
        result.PushBack(Opal::Clone(pair, m_allocator));
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<KeyType> Opal::HashMap<KeyType, ValueType>::ToArrayOfKeys() const
{
    DynamicArray<key_type> result;
    result.Reserve(m_size);
    for (const auto& pair : *this)
    {
        result.PushBack(Opal::Clone(pair.key, m_allocator));
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<ValueType> Opal::HashMap<KeyType, ValueType>::ToArrayOfValues() const
{
    DynamicArray<value_type> result;
    result.Reserve(m_size);
    for (const auto& pair : *this)
    {
        result.PushBack(Opal::Clone(pair.value, m_allocator));
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::iterator Opal::HashMap<KeyType, ValueType>::FindFirstIterator()
{
    if (m_control_bytes == nullptr)
    {
        return iterator(this, ~u64{});
    }
    u64 index = 0;
    while (m_control_bytes[index] != Impl::k_control_sentinel)
    {
        if (Impl::IsControlFull(m_control_bytes[index]))
        {
            return iterator(this, index);
        }
        index++;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::const_iterator Opal::HashMap<KeyType, ValueType>::FindFirstIterator() const
{
    if (m_control_bytes == nullptr)
    {
        return const_iterator(this, ~u64{});
    }
    u64 index = 0;
    while (m_control_bytes[index] != Impl::k_control_sentinel)
    {
        if (Impl::IsControlFull(m_control_bytes[index]))
        {
            return const_iterator(this, index);
        }
        index++;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::iterator Opal::HashMap<KeyType, ValueType>::FindNextIterator(HashMap::iterator pos)
{
    if (pos == end() || m_control_bytes == nullptr)
    {
        return end();
    }
    u64 index = pos.GetIndex() + 1;
    while (m_control_bytes[index] != Impl::k_control_sentinel)
    {
        if (Impl::IsControlFull(m_control_bytes[index]))
        {
            return iterator(this, index);
        }
        ++index;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::const_iterator Opal::HashMap<KeyType, ValueType>::FindNextIterator(HashMap::const_iterator pos) const
{
    if (pos == cend() || m_control_bytes == nullptr)
    {
        return cend();
    }
    u64 index = pos.GetIndex() + 1;
    while (m_control_bytes[index] != Impl::k_control_sentinel)
    {
        if (Impl::IsControlFull(m_control_bytes[index]))
        {
            return const_iterator(this, index);
        }
        ++index;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::pair_type& Opal::HashMap<KeyType, ValueType>::Get(u64 index)
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType, typename ValueType>
const typename Opal::HashMap<KeyType, ValueType>::pair_type& Opal::HashMap<KeyType, ValueType>::Get(u64 index) const
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType, typename ValueType>
typename Opal::HashMap<KeyType, ValueType>::key_type& Opal::HashMap<KeyType, ValueType>::GetKey(u64 index)
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].key;
}

template <typename KeyType, typename ValueType>
const typename Opal::HashMap<KeyType, ValueType>::key_type& Opal::HashMap<KeyType, ValueType>::GetKey(u64 index) const
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].key;
}

template <typename KeyType, typename ValueType>
typename Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(u64 index)
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].value;
}

template <typename KeyType, typename ValueType>
const typename Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(u64 index) const
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].value;
}

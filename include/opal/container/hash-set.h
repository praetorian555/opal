#pragma once

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-table-base.h"
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

    /** Smallest number of keys a set is built for. */
    constexpr static u64 k_default_capacity = Impl::k_default_capacity;

    /**
     * Creates a set able to hold at least `capacity` keys before it has to grow.
     * @param capacity How many keys the set should make room for. Smaller requests are raised to a usable minimum.
     * @param allocator Allocator to use, or nullptr to use the default one.
     * @throw OutOfMemoryException when the allocator runs out of memory.
     */
    explicit HashSet(size_type capacity = k_default_capacity, AllocatorBase* allocator = nullptr);

    /**
     * Build a set the way the matching constructor does, reporting a failed allocation instead of throwing it. Use this when the
     * allocator is budgeted and running out is an outcome to branch on.
     * @return The set, or ErrorCode::OutOfMemory.
     */
    [[nodiscard]] static Expected<HashSet, ErrorCode> Create(size_type capacity = k_default_capacity, AllocatorBase* allocator = nullptr);

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

    static u64 CalculateHash(const key_type& key)
    {
        Hasher<key_type> hasher;
        return hasher(key);
    }
    bool FindIndex(const key_type& key, u64 hash, u64& out_index) const;
    void OccupySlot(const key_type& key, u64 hash, u64 index) requires IsPOD<KeyType>;
    void OccupySlot(key_type&& key, u64 hash, u64 index);
    void DeleteSlot(u64 index);
    void DestroyAllKeys();
    ErrorCode Grow();

    // Builds a set that owns nothing and has not touched an allocator, for the factory to fill in. Reserve(0) is not a
    // substitute: it still asks the allocator for the smallest table and fails on a budgeted one.
    struct EmptyTag
    {
    };
    HashSet(EmptyTag, AllocatorBase* allocator) : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator()) {}

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
        OPAL_RAISE(OutOfMemoryException(m_allocator->GetName(), capacity * sizeof(key_type)));
    }
}

template <typename KeyType>
Opal::Expected<Opal::HashSet<KeyType>, Opal::ErrorCode> Opal::HashSet<KeyType>::Create(size_type capacity, AllocatorBase* allocator)
{
    using Result = Expected<HashSet, ErrorCode>;
    HashSet set{EmptyTag{}, allocator};
    const ErrorCode error = set.Reserve(capacity);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Result(error);
    }
    return Result(Move(set));
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
        if (Impl::IsControlFull(m_control_bytes[i]))
        {
            m_slots[i].~key_type();
        }
    }
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Reserve(size_type capacity)
{
    u64 new_capacity = Impl::GetNextPowerOf2MinusOne(capacity < k_default_capacity ? k_default_capacity : capacity);
    // Probing walks until it meets a slot that was never occupied, so a table that cannot hold the keys already in the set with room to
    // spare would corrupt them and never terminate. Raise the request rather than honour it.
    while (Impl::GetGrowthThreshold(new_capacity) < m_size)
    {
        new_capacity = (new_capacity << 1) | 1;
    }
    u64 new_size = 0;
    // Control bytes and keys share one allocation, so the block has to satisfy whichever of the two needs the stricter alignment, and the
    // keys have to start at an offset that keeps it.
    constexpr u64 k_slot_alignment = alignof(key_type) > 16 ? alignof(key_type) : 16;
    const u64 control_bytes_size = Impl::GetNextPowerOf2MinusOne(new_capacity + Impl::k_group_width) + 1;
    const u64 slots_offset = (control_bytes_size + k_slot_alignment - 1) & ~(k_slot_alignment - 1);
    const u64 size_to_allocate = slots_offset + (new_capacity * sizeof(key_type));

    i8* new_control_bytes = static_cast<i8*>(m_allocator->Alloc(size_to_allocate, k_slot_alignment));
    if (new_control_bytes == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    memset(new_control_bytes, Impl::k_control_empty, control_bytes_size);
    new_control_bytes[new_capacity] = Impl::k_control_sentinel;
    key_type* new_slots = reinterpret_cast<key_type*>(new_control_bytes + slots_offset);

    if (m_control_bytes != nullptr)
    {
        // Walks the slots directly rather than through an iterator, since rehashing moves out of every key it visits and iterators only
        // hand out const keys.
        for (u64 i = 0; i < m_capacity; ++i)
        {
            if (!Impl::IsControlFull(m_control_bytes[i]))
            {
                continue;
            }
            key_type& key = m_slots[i];
            u64 hash = CalculateHash(key);
            u64 offset = Impl::GetHash1(hash, new_control_bytes) & new_capacity;
            while (true)
            {
                i8* group = new_control_bytes + offset;
                BitMask<u32> not_full_mask = Impl::GetGroupNotFull(group);
                if (not_full_mask)
                {
                    u64 slot_index = offset + not_full_mask.GetLowestSetBitIndex();
                    slot_index &= new_capacity;
                    Impl::SetControlByte(slot_index, Impl::GetHash2(hash), new_control_bytes, new_capacity);
                    new (&new_slots[slot_index]) key_type(Move(key));  // Invokes move constructor on allocated memory
                    new_size++;
                    break;
                }
                offset = (offset + Impl::k_group_width) & new_capacity;
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
    if (m_size + 1 <= Impl::GetGrowthThreshold(m_capacity) / 2)
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
bool Opal::HashSet<KeyType>::FindIndex(const key_type& key, u64 hash, u64& out_index) const
{
    if (m_control_bytes == nullptr)
    {
        return false;
    }
    u64 offset = Impl::GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        i8* group = m_control_bytes + offset;
        i8 hash2 = Impl::GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = Impl::GetGroupMatch(group, hash2);
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
        BitMask<u32> empty_group = Impl::GetGroupMatchEmpty(group);
        if (empty_group)
        {
            out_index = (offset + empty_group.GetLowestSetBitIndex()) & m_capacity;
            break;
        }
        // Move to the next group
        offset = (offset + Impl::k_group_width) & m_capacity;
    }
    return false;
}

template <typename KeyType>
typename Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::Find(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, CalculateHash(key), index))
    {
        return iterator(this, index);
    }
    return end();
}

template <typename KeyType>
typename Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::Find(const key_type& key) const
{
    u64 index = 0;
    if (FindIndex(key, CalculateHash(key), index))
    {
        return const_iterator(this, index);
    }
    return cend();
}

template <typename KeyType>
bool Opal::HashSet<KeyType>::Contains(const key_type& key) const
{
    u64 index = 0;
    return FindIndex(key, CalculateHash(key), index);
}

template <typename KeyType>
void Opal::HashSet<KeyType>::OccupySlot(const key_type& key, u64 hash, u64 index) requires IsPOD<KeyType>
{
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index]) key_type(key);  // Invokes copy constructor on allocated memory
    m_size++;
    m_growth_left--;
}

template <typename KeyType>
void Opal::HashSet<KeyType>::OccupySlot(key_type&& key, u64 hash, u64 index)
{
    Impl::SetControlByte(index, Impl::GetHash2(hash), m_control_bytes, m_capacity);
    new (&m_slots[index]) key_type(Move(key));  // Invokes move constructor on allocated memory
    m_size++;
    m_growth_left--;
}

template <typename KeyType>
void Opal::HashSet<KeyType>::DeleteSlot(u64 index)
{
    m_slots[index].~key_type();
    m_size--;
    Impl::SetControlByte(index, Impl::k_control_deleted, m_control_bytes, m_capacity);
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Insert(const key_type& key) requires IsPOD<KeyType>
{
    const u64 hash = CalculateHash(key);
    u64 index = 0;
    const bool has_key = FindIndex(key, hash, index);
    if (has_key)
    {
        m_slots[index] = key;
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
        FindIndex(key, hash, index);
    }

    OccupySlot(key, hash, index);
    return ErrorCode::Success;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Insert(key_type&& key)
{
    const u64 hash = CalculateHash(key);
    u64 index = 0;
    const bool has_key = FindIndex(key, hash, index);
    if (has_key)
    {
        m_slots[index] = Move(key);
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
        FindIndex(key, hash, index);
    }

    OccupySlot(Move(key), hash, index);
    return ErrorCode::Success;
}

template <typename KeyType>
Opal::ErrorCode Opal::HashSet<KeyType>::Erase(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, CalculateHash(key), index))
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
    if (Impl::IsControlFull(m_control_bytes[index]))
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
    if (Impl::IsControlFull(m_control_bytes[index]))
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

template <typename KeyType>
void Opal::HashSet<KeyType>::Clear()
{
    if (m_control_bytes == nullptr)
    {
        return;
    }
    DestroyAllKeys();
    memset(m_control_bytes, Impl::k_control_empty, m_capacity + Impl::k_group_width);
    m_control_bytes[m_capacity] = Impl::k_control_sentinel;
    m_growth_left = m_capacity;
    m_size = 0;
}






template <typename KeyType>
Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::FindFirstIterator()
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

template <typename KeyType>
Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::FindFirstIterator() const
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

template <typename KeyType>
Opal::HashSet<KeyType>::iterator Opal::HashSet<KeyType>::FindNextIterator(iterator pos)
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

template <typename KeyType>
Opal::HashSet<KeyType>::const_iterator Opal::HashSet<KeyType>::FindNextIterator(
    HashSet::const_iterator pos) const
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

template <typename KeyType>
const typename Opal::HashSet<KeyType>::key_type& Opal::HashSet<KeyType>::GetKey(u64 index) const
{
    OPAL_ASSERT(Impl::IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
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

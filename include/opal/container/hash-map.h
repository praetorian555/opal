#pragma once

#include <emmintrin.h>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/hash.h"

namespace Opal
{

template <typename KeyType, typename ValueType>
struct Pair
{
    KeyType key;
    ValueType value;
};

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

    pair_type& operator*() const { return m_hash_map->Get(m_index); }
    pair_type* operator->() const { return &m_hash_map->Get(m_index); }
    key_type& GetKey() { return m_hash_map->GetKey(m_index); }
    value_type& GetValue() { return m_hash_map->GetValue(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    HashMapClass* m_hash_map = nullptr;
    u64 m_index = 0;
};

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
    pair_type* operator->() const { return &m_hash_map->Get(m_index); }
    const key_type& GetKey() const { return m_hash_map->GetKey(m_index); }
    const value_type& GetValue() const { return m_hash_map->GetValue(m_index); }

    [[nodiscard]] u64 GetIndex() const { return m_index; }

private:
    const HashMapClass* m_hash_map = nullptr;
    u64 m_index = 0;
};

namespace Impl
{
}  // namespace Impl

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

    constexpr static u64 k_group_width = 16;
    // Special value to indicate that the control byte is empty
    constexpr static i8 k_control_bitmask_empty = -128;  // 0b10000000;
    // Special value to indicate that the control byte was deleted
    constexpr static i8 k_control_bitmask_deleted = -2;  // 0b11111110;
    // Special value to indicate that we reach the end of the control bytes
    constexpr static i8 k_control_bitmask_sentinel = -1;  // 0b11111111;

    constexpr static u64 k_default_capacity = 4;

    explicit HashMap(size_type capacity = k_default_capacity, AllocatorBase* allocator = nullptr);
    HashMap(const ArrayView<Pair<KeyType, ValueType>>& pairs, AllocatorBase* allocator = nullptr);
    HashMap(std::initializer_list<pair_type> pairs, AllocatorBase* allocator = nullptr);

    HashMap(const HashMap& other) = delete;
    HashMap(HashMap&& other) noexcept;

    ~HashMap();

    HashMap& operator=(const HashMap& other) = delete;
    HashMap& operator=(HashMap&& other) noexcept;

    HashMap Clone() const;

    void Reserve(size_type capacity);

    [[nodiscard]] u64 GetSize() const { return m_size; }
    [[nodiscard]] u64 GetCapacity() const { return m_capacity; }
    [[nodiscard]] u64 GetGrowthLeft() const { return m_growth_left; }

    iterator Find(const key_type& key);
    const_iterator Find(const key_type& key) const;
    bool Contains(const key_type& key) const;

    value_type& GetValue(const key_type& key);
    const value_type& GetValue(const key_type& key) const;

    void Insert(const key_type& key, const value_type& value);
    void Insert(key_type&& key, value_type&& value);

    void Erase(const key_type& key);
    void Erase(iterator it);
    void Erase(const_iterator it);

    void Clear();

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

    static u64 GetNextPowerOf2MinusOne(u64 value);
    [[nodiscard]] static bool IsControlFull(i8 control) { return control >= 0; }
    static u64 CalculateHash(const key_type& key)
    {
        Hasher<key_type> hasher;
        return hasher(key);
    }
    [[nodiscard]] u64 GetHash1(u64 hash, void* seed) const { return (hash >> 7) ^ (reinterpret_cast<u64>(seed) >> 12); }
    static i8 GetHash2(u64 hash) { return static_cast<i8>(hash & 0x000000000000007f); }
    static BitMask<u32> GetGroupMatch(const i8* group, i8 pattern);
    static BitMask<u32> GetGroupMatchEmpty(const i8* group);
    static BitMask<u32> GetGroupNotFull(const i8* group);
    void SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity);
    static u64 GetGrowthThreshold(u64 capacity) { return (capacity * 7) / 8; }
    bool FindIndex(const key_type& key, u64& out_index) const;
    void OccupySlot(const key_type& key, const value_type& value, u64 index);
    void OccupySlot(key_type&& key, value_type&& value, u64 index);
    void DeleteSlot(u64 index);

    AllocatorBase* m_allocator = nullptr;
    i8* m_control_bytes = nullptr;
    pair_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal

template <typename KeyType, typename ValueType>
Opal::u64 Opal::HashMap<KeyType, ValueType>::GetNextPowerOf2MinusOne(u64 value)
{
    return value != 0 ? ~u64{} >> CountLeadingZeros(value) : 1;
}

template <typename KeyType, typename ValueType>
Opal::BitMask<Opal::u32> Opal::HashMap<KeyType, ValueType>::GetGroupMatch(const i8* group, i8 pattern)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i match = _mm_set1_epi8(pattern);
    // First compare byte for byte from ctrl and match. If bytes are equal emit 0xFF, if bytes are not equal emit 0x00.
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl))));
}

template <typename KeyType, typename ValueType>
Opal::BitMask<Opal::u32> Opal::HashMap<KeyType, ValueType>::GetGroupMatchEmpty(const i8* group)
{
    return GetGroupMatch(group, k_control_bitmask_empty);
}

template <typename KeyType, typename ValueType>
Opal::BitMask<Opal::u32> Opal::HashMap<KeyType, ValueType>::GetGroupNotFull(const i8* group)
{
    // Converts data pointed by void* pointer to __128i, pointer does not have to be aligned to any particular boundary
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Creates __m128i by repeating pattern byte 16 times
    const __m128i special = _mm_set1_epi8(k_control_bitmask_sentinel);
    // First compare byte for byte from special and ctrl. If special byte is greater than ctrl byte emit 0xFF, or 0x00 otherwise..
    // After the compare take the highest bit of every byte in the result and pack it into 32 bit value (only using lower 16 bits).
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, ctrl))));
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::SetControlByte(u64 index, i8 hash2, i8* control_bytes, u64 capacity)
{
    // Here we simply set the control byte at the index to hash2.
    control_bytes[index] = hash2;
    constexpr u64 k_cloned_bytes_count = k_group_width - 1;
    // We need to figure out if we need to clone the byte at the end of the group. If byte is in the first
    // k_group_width - 1 we use unsigned integer underflow to set the cloned byte at the end of the group.
    // If byte is not in the first k_group_width - 1 we just set the byte again.
    control_bytes[((index - k_cloned_bytes_count) & capacity) + (k_cloned_bytes_count & capacity)] = hash2;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(size_type capacity, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    Reserve(capacity < k_default_capacity ? k_default_capacity : capacity);
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(const ArrayView<Pair<KeyType, ValueType>>& pairs, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    Reserve(pairs.GetSize());
    for (auto& pair : pairs)
    {
        Insert(pair.key, pair.value);
    }
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::HashMap(std::initializer_list<pair_type> pairs, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
{
    Reserve(pairs.size());
    for (auto& pair : pairs)
    {
        Insert(pair.key, pair.value);
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
    other.Reserve(4);
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>& Opal::HashMap<KeyType, ValueType>::operator=(HashMap&& other) noexcept
{
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
    other.Reserve(4);

    return *this;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::~HashMap()
{
    m_allocator->Free(m_control_bytes);
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType> Opal::HashMap<KeyType, ValueType>::Clone() const
{
    HashMap clone(m_capacity, m_allocator);
    for (const auto& pair : *this)
    {
        clone.Insert(pair.key, pair.value);
    }
    return clone;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Reserve(size_type capacity)
{
    const u64 new_capacity = GetNextPowerOf2MinusOne(capacity);
    u64 new_size = 0;
    // Since we are storing control bytes and the keys in the same memory block we need to make sure
    // that keys start at the address that is aligned with their size.
    const u64 control_bytes_size = GetNextPowerOf2MinusOne(new_capacity + k_group_width) + 1;
    u64 size_to_allocate = control_bytes_size + (new_capacity * sizeof(pair_type));

    i8* new_control_bytes = static_cast<i8*>(m_allocator->Alloc(size_to_allocate, 16u));
    if (new_control_bytes == nullptr)
    {
        throw OutOfMemoryException(m_allocator->GetName(), size_to_allocate);
    }
    memset(new_control_bytes, k_control_bitmask_empty, control_bytes_size);
    new_control_bytes[new_capacity] = k_control_bitmask_sentinel;
    pair_type* new_slots = reinterpret_cast<pair_type*>(new_control_bytes + control_bytes_size);

    // We have just allocated this memory, no need to trigger expansive constructors since
    // new objects will be moved into this memory before use
#ifdef OPAL_COMPILER_GCC
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_WARNING("-Wclass-memaccess")
#endif  // OPAL_COMPILER_GCC
    memset(new_slots, 0, new_capacity * sizeof(pair_type));
#ifdef OPAL_COMPILER_GCC
    OPAL_END_DISABLE_WARNINGS
#endif  // OPAL_COMPILER_GCC

    if (m_capacity > 0)
    {
        for (pair_type& pair : *this)
        {
            const u64 hash = CalculateHash(pair.key);
            u64 offset = GetHash1(hash, new_control_bytes) & new_capacity;
            while (true)
            {
                const i8* group = new_control_bytes + offset;
                if (BitMask<u32> not_full_mask = GetGroupNotFull(group))
                {
                    u64 slot_index = offset + not_full_mask.GetLowestSetBitIndex();
                    slot_index &= new_capacity;
                    SetControlByte(slot_index, GetHash2(hash), new_control_bytes, new_capacity);
                    new_slots[slot_index] = Move(pair);
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
}

template <typename KeyType, typename ValueType>
bool Opal::HashMap<KeyType, ValueType>::FindIndex(const key_type& key, u64& out_index) const
{
    const u64 hash = CalculateHash(key);
    u64 offset = GetHash1(hash, m_control_bytes) & m_capacity;
    while (true)
    {
        const i8* group = m_control_bytes + offset;
        const i8 hash2 = GetHash2(hash);
        // Get slots in this group that match lower 7-bits of the hash
        const BitMask<u32> match_mask = GetGroupMatch(group, hash2);
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
        if (BitMask<u32> empty_group = GetGroupMatchEmpty(group))
        {
            out_index = (offset + empty_group.GetLowestSetBitIndex()) & m_capacity;
            break;
        }
        // Move to the next group
        offset = (offset + k_group_width) & m_capacity;
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
{
    const u64 hash = CalculateHash(key);
    SetControlByte(index, GetHash2(hash), m_control_bytes, m_capacity);
    m_slots[index].key = key;
    m_slots[index].value = value;
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::OccupySlot(key_type&& key, value_type&& value, u64 index)
{
    const u64 hash = CalculateHash(key);
    SetControlByte(index, GetHash2(hash), m_control_bytes, m_capacity);
    m_slots[index].key = Move(key);
    m_slots[index].value = Move(value);
    m_size++;
    m_growth_left--;
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::DeleteSlot(u64 index)
{
    m_slots[index].key.~key_type();
    m_slots[index].value.~value_type();
    m_size--;
    SetControlByte(index, k_control_bitmask_deleted, m_control_bytes, m_capacity);
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
const Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(const key_type& key) const
{
    const_iterator it = Find(key);
    if (it != cend())
    {
        return it.GetValue();
    }
    throw OutOfBoundsException("Key not found");
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Insert(const key_type& key, const value_type& value)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = key;
        m_slots[index].value = value;
        return;
    }

    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        Reserve(m_capacity + 1);

        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(key, value, index);
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Insert(key_type&& key, value_type&& value)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        m_slots[index].key = Move(key);
        m_slots[index].value = Move(value);
        return;
    }

    if (m_capacity - m_growth_left >= GetGrowthThreshold(m_capacity))
    {
        Reserve(m_capacity + 1);
        // We have to get the index again since we rehashed the table
        FindIndex(key, index);
    }

    OccupySlot(key, value, index);
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Erase(const key_type& key)
{
    u64 index = 0;
    if (FindIndex(key, index))
    {
        DeleteSlot(index);
    }
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Erase(iterator it)
{
    if (it < begin() || it >= end())
    {
        return;
    }
    const u64 index = it.GetIndex();
    if (IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
    }
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Erase(const_iterator it)
{
    if (it < cbegin() || it >= cend())
    {
        return;
    }
    const u64 index = it.GetIndex();
    if (IsControlFull(m_control_bytes[index]))
    {
        DeleteSlot(index);
    }
}

template <typename KeyType, typename ValueType>
void Opal::HashMap<KeyType, ValueType>::Clear()
{
    for (auto it = begin(); it != end(); ++it)
    {
        it.GetKey().~key_type();
        it.GetValue().~value_type();
    }
    memset(m_control_bytes, k_control_bitmask_empty, m_capacity + k_group_width);
    m_control_bytes[m_capacity] = k_control_bitmask_sentinel;
    m_growth_left = m_capacity;
    m_size = 0;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<Opal::Pair<KeyType, ValueType>> Opal::HashMap<KeyType, ValueType>::ToArray() const
{
    DynamicArray<pair_type> result;
    result.Reserve(m_size);
    for (auto& pair : *this)
    {
        result.PushBack(pair);
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<KeyType> Opal::HashMap<KeyType, ValueType>::ToArrayOfKeys() const
{
    DynamicArray<key_type> result;
    result.Reserve(m_size);
    for (auto& pair : *this)
    {
        result.PushBack(pair.key);
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::DynamicArray<ValueType> Opal::HashMap<KeyType, ValueType>::ToArrayOfValues() const
{
    DynamicArray<value_type> result;
    result.Reserve(m_size);
    for (auto& pair : *this)
    {
        result.PushBack(pair.value);
    }
    return result;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::iterator Opal::HashMap<KeyType, ValueType>::FindFirstIterator()
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

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::const_iterator Opal::HashMap<KeyType, ValueType>::FindFirstIterator() const
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

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::iterator Opal::HashMap<KeyType, ValueType>::FindNextIterator(HashMap::iterator pos)
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
        ++index;
    }
    return iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::const_iterator Opal::HashMap<KeyType, ValueType>::FindNextIterator(HashMap::const_iterator pos) const
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
        ++index;
    }
    return const_iterator(this, ~u64{});
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::pair_type& Opal::HashMap<KeyType, ValueType>::Get(u64 index)
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType, typename ValueType>
const Opal::HashMap<KeyType, ValueType>::pair_type& Opal::HashMap<KeyType, ValueType>::Get(u64 index) const
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index];
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::key_type& Opal::HashMap<KeyType, ValueType>::GetKey(u64 index)
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].key;
}

template <typename KeyType, typename ValueType>
const Opal::HashMap<KeyType, ValueType>::key_type& Opal::HashMap<KeyType, ValueType>::GetKey(u64 index) const
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].key;
}

template <typename KeyType, typename ValueType>
Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(u64 index)
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].value;
}

template <typename KeyType, typename ValueType>
const Opal::HashMap<KeyType, ValueType>::value_type& Opal::HashMap<KeyType, ValueType>::GetValue(u64 index) const
{
    OPAL_ASSERT(IsControlFull(m_control_bytes[index]), "There is no valid key at this index!");
    return m_slots[index].value;
}

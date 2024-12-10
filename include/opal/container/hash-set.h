#pragma once

#include <unordered_set>

#include "opal/allocator.h"
#include "opal/error-codes.h"
#include "string-hash.h"

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

    explicit HashSet(size_type capacity, allocator_type* allocator = nullptr);

    HashSet(const HashSet& other);
    HashSet(HashSet&& other) noexcept;

    HashSet& operator=(const HashSet& other);
    HashSet& operator=(HashSet&& other) noexcept;

    ~HashSet();

    ErrorCode Insert(const key_type& key, const value_type& value);
    ErrorCode Insert(key_type&& key, value_type&& value);

private:
    allocator_type* m_allocator = nullptr;
    u8* m_control_bytes = nullptr;
    key_type* m_slots = nullptr;
    u64 m_capacity = 0;
    u64 m_size = 0;
    u64 m_growth_left = 0;
};

}  // namespace Opal

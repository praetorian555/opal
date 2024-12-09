#pragma once

#include "opal/container/string.h"
#include <unordered_map>

namespace Opal
{

template <typename MyString>
struct Hash
{
    size_t operator()(const MyString& str) const
    {
        // FNV-1a HASH
        constexpr u64 k_fnv_offset_basis = 14695981039346656037ULL;
        constexpr u64 k_fnv_prime = 1099511628211ULL;

        u64 hash = k_fnv_offset_basis;
        for (typename MyString::value_type c : str)
        {
            hash = hash ^ static_cast<u64>(c);
            hash = hash * k_fnv_prime;
        }
        return hash;
    }
};

} // namespace Opal

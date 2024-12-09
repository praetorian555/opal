#pragma once

#include "opal/container/string.h"
#include "opal/hash.h"
#include <unordered_map>

namespace Opal
{

template <typename MyString>
struct Hash
{
    size_t operator()(const MyString& str) const
    {
        return CalculateHashFromPointerArray(reinterpret_cast<const u8*>(str.GetData()), str.GetSize());
    }
};

} // namespace Opal

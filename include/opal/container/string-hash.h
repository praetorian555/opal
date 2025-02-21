#pragma once

#include "opal/container/string.h"
#include "opal/hash.h"

namespace Opal
{

template <typename MyString>
struct StringHash
{
    size_t operator()(const MyString& str) const
    {
        return Hash::CalcRawArray(reinterpret_cast<const u8*>(str.GetData()), str.GetSize());
    }
};

} // namespace Opal

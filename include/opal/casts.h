#pragma once

#include "opal/defines.h"
#include "opal/exceptions.h"

namespace Opal
{
template <typename To, typename From>
    requires IntegralOrFloatingPoint<To> && IntegralOrFloatingPoint<From>
To Narrow(const From& from)
{
#if defined(OPAL_DEBUG)
    To to = static_cast<To>(from);
    From back_to_from = static_cast<From>(to);
    if (from != back_to_from)
    {
        throw Exception("Narrow cast failed!");
    }
    return to;
#else
    return static_cast<To>(from);
#endif
}
}  // namespace Opal

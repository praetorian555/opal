#pragma once

#include "opal/assert.h"
#include "opal/defines.h"
#include "opal/exceptions.h"

namespace Opal
{
/**
 * Checked narrowing cast between integral and floating-point types. Preferred over static_cast
 * for conversions that may lose data (e.g. i64 -> i32, f64 -> f32, i32 -> u8).
 *
 * In debug builds, validates losslessness by round-tripping the value (cast To, then back to From).
 * Ends the program through the contract violation handler if the round-trip produces a different value, indicating data loss.
 * In release builds, compiles down to a plain static_cast with no overhead.
 *
 * @tparam To   Target type (must satisfy IntegralOrFloatingPoint).
 * @tparam From Source type (must satisfy IntegralOrFloatingPoint).
 * @param from  Value to convert.
 * @return      The converted value.
 * @note Ends the program through the contract violation handler when the conversion is lossy. Debug builds only.
 */
template <typename To, typename From>
    requires IntegralOrFloatingPoint<To> && IntegralOrFloatingPoint<From>
constexpr To Narrow(const From& from)
{
#if defined(OPAL_DEBUG)
    To to = static_cast<To>(from);
    From back_to_from = static_cast<From>(to);
    OPAL_VERIFY(from == back_to_from, "Narrow cast is lossy");
    return to;
#else
    return static_cast<To>(from);
#endif
}
}  // namespace Opal

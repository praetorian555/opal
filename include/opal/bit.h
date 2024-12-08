#pragma once

#include "opal/types.h"

namespace Opal
{

/**
 * @brief Count the number of leading zeros in a 32-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
u32 CountLeadingZeros(u32 value);

/**
 * @brief Count the number of leading zeros in a 64-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
u64 CountLeadingZeros(u64 value);

/**
 * @brief Count the number of trailing zeros in a 32-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
u32 CountTrailingZeros(u32 value);

/**
 * @brief Count the number of trailing zeros in a 64-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
u64 CountTrailingZeros(u64 value);

}  // namespace Opal

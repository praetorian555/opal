#pragma once

#include "opal/types.h"
#include "opal/export.h"

namespace Opal
{

/**
 * @brief Count the number of leading zeros in a 32-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
OPAL_EXPORT u32 CountLeadingZeros(u32 value);

/**
 * @brief Count the number of leading zeros in a 64-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
OPAL_EXPORT u64 CountLeadingZeros(u64 value);

/**
 * @brief Count the number of trailing zeros in a 32-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
OPAL_EXPORT u32 CountTrailingZeros(u32 value);

/**
 * @brief Count the number of trailing zeros in a 64-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
OPAL_EXPORT u64 CountTrailingZeros(u64 value);

template <typename T>
u32 GetBitWidth();

/**
 * Wrapper around a simple bitmask that allows the user to iterate over indices of
 * set bits.
 * @tparam T Type used for bitmask.
 */
template <typename T>
class BitMask
{
public:
    explicit BitMask(T bit_mask) : m_bit_mask(bit_mask) {}

    [[nodiscard]] u32 GetLowestSetBitIndex() const { return static_cast<u32>(CountTrailingZeros(m_bit_mask)); }
    [[nodiscard]] u32 GetHighestSetBitIndex() const { return static_cast<u32>(GetBitWidth<T>() - CountLeadingZeros(m_bit_mask) - 1); }

    // Move to next, more significant bit
    BitMask operator++()
    {
        // This will clear least significant set bit
        m_bit_mask &= (m_bit_mask - 1);
        return *this;
    }

    u32 operator*() const { return GetLowestSetBitIndex(); }

    explicit operator bool() const { return m_bit_mask != 0; }

    BitMask begin() const { return *this; }
    BitMask end() const { return BitMask(0); }

    bool operator==(const BitMask& other) const { return m_bit_mask == other.m_bit_mask; }

private:
    T m_bit_mask;
};

}  // namespace Opal

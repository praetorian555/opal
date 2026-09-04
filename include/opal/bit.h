#pragma once

#include "opal/types.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <intrin.h>
#endif

namespace Opal
{

/**
 * @brief Count the number of leading zeros in a 32-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
inline u32 CountLeadingZeros(u32 value);

/**
 * @brief Count the number of leading zeros in a 64-bit integer.
 * @param value The value to count the leading zeros of.
 * @return The number of leading zeros in the value.
 */
inline u64 CountLeadingZeros(u64 value);

/**
 * @brief Count the number of trailing zeros in a 32-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
inline u32 CountTrailingZeros(u32 value);

/**
 * @brief Count the number of trailing zeros in a 64-bit integer.
 * @param value The value to count the trailing zeros of.
 * @return The number of trailing zeros in the value.
 */
inline u64 CountTrailingZeros(u64 value);

/**
 * @brief Counts the number of set bits in the 32-bit or 64-bit integer.
 * @param value 32-bit integer or 64-bit integer.
 * @return The number of set bits.
 */
inline u32 CountSetBits(u32 value);
inline u64 CountSetBits(u64 value);

/**
 * Calculates the next number greater then given value that is the power of 2.
 * @param value Reference number.
 * @return Value that is power of 2. If the value is 0 or 1, it returns 1.
 */
inline u64 GetNextPowerOf2(u64 value);

/**
 * Get number of bits type has.
 * @tparam T Type to check.
 * @return Number of bits.
 */
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
    BitMask() : m_bit_mask(static_cast<T>(0)) {}
    explicit BitMask(T bit_mask) : m_bit_mask(bit_mask) {}

    [[nodiscard]] u32 GetLowestSetBitIndex() const { return static_cast<u32>(CountTrailingZeros(m_bit_mask)); }
    [[nodiscard]] u32 GetHighestSetBitIndex() const { return static_cast<u32>(GetBitWidth<T>() - CountLeadingZeros(m_bit_mask) - 1); }

    [[nodiscard]] u32 GetTrailingZeros() const { return static_cast<u32>(CountTrailingZeros(m_bit_mask)); }
    [[nodiscard]] u32 GetLeadingZeros() const { return static_cast<u32>(CountLeadingZeros(m_bit_mask)); }

    [[nodiscard]] u32 GetSetBitCount() const { return static_cast<u32>(CountSetBits(m_bit_mask)); }

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

/** Implementation *******************************************************************************/

inline Opal::u32 Opal::CountLeadingZeros(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __lzcnt(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u32>(__builtin_clz(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u64 Opal::CountLeadingZeros(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __lzcnt64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u64>(__builtin_clzll(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u32 Opal::CountTrailingZeros(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return _tzcnt_u32(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u32>(__builtin_ctz(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u64 Opal::CountTrailingZeros(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return _tzcnt_u64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u64>(__builtin_ctzll(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u32 Opal::CountSetBits(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __popcnt(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u32>(__builtin_popcount(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u64 Opal::CountSetBits(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __popcnt64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u64>(__builtin_popcountll(value));
#else
#error "Compiler not supported"
#endif
}

inline Opal::u64 Opal::GetNextPowerOf2(u64 value)
{
    return value <= 1 ? 1ull : 1ull << (64ull - CountLeadingZeros(value - 1));
}

template <>
inline Opal::u32 Opal::GetBitWidth<Opal::u8>()
{
    return 8;
}

template <>
inline Opal::u32 Opal::GetBitWidth<Opal::u16>()
{
    return 16;
}

template <>
inline Opal::u32 Opal::GetBitWidth<Opal::u32>()
{
    return 32;
}

template <>
inline Opal::u32 Opal::GetBitWidth<Opal::u64>()
{
    return 64;
}

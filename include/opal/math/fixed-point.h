#pragma once

#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/math-base.h"
#include "opal/math-constants.h"
#include "opal/type-traits.h"
#include "opal/types.h"

#if defined(OPAL_COMPILER_MSVC)
#include <intrin.h>
#endif

namespace Opal
{

/**
 * @brief Concept that checks if a type can back a FixedPoint value.
 * @tparam T The type to be evaluated.
 */
template <typename T>
concept FixedPointStorage = AnyOf<T, i8, i16, i32, i64>;

// ------------------------------------------------------------------------------------------------
// Internal helpers.
// ------------------------------------------------------------------------------------------------

namespace Impl
{

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wpedantic")

/** Signed type wide enough to hold the exact product of two T values. Falls back to T when there is none. */
template <typename T>
struct FixedPointWide
{
    using Type = T;
};

template <>
struct FixedPointWide<i8>
{
    using Type = i16;
};

template <>
struct FixedPointWide<i16>
{
    using Type = i32;
};

template <>
struct FixedPointWide<i32>
{
    using Type = i64;
};

#if !defined(OPAL_COMPILER_MSVC)
template <>
struct FixedPointWide<i64>
{
    using Type = __int128;
};
#endif

OPAL_END_DISABLE_WARNINGS

/** True when FixedPointWide<T>::Type really is wider than T. */
template <typename T>
inline constexpr bool k_fixed_point_has_wide_value = true;

#if defined(OPAL_COMPILER_MSVC)
template <>
inline constexpr bool k_fixed_point_has_wide_value<i64> = false;
#endif

/** Unsigned type with the same width as T, used for the wrapping arithmetic the signed types are not allowed to do. */
template <typename T>
struct FixedPointUnsigned
{
    using Type = T;
};

template <>
struct FixedPointUnsigned<i8>
{
    using Type = u8;
};

template <>
struct FixedPointUnsigned<i16>
{
    using Type = u16;
};

template <>
struct FixedPointUnsigned<i32>
{
    using Type = u32;
};

template <>
struct FixedPointUnsigned<i64>
{
    using Type = u64;
};

/** Number of bits in T. */
template <typename T>
inline constexpr u32 k_fixed_point_bit_count_value = static_cast<u32>(sizeof(T) * 8);

/** Most negative value T can hold. */
template <FixedPointStorage T>
inline constexpr T k_fixed_point_lowest_value =
    static_cast<T>(static_cast<typename FixedPointUnsigned<T>::Type>(1) << (k_fixed_point_bit_count_value<T> - 1));

/** Largest value T can hold. */
template <FixedPointStorage T>
inline constexpr T k_fixed_point_highest_value = static_cast<T>(~k_fixed_point_lowest_value<T>);

/** Two raised to the given exponent. */
constexpr f64 FixedPointPow2(u32 exponent)
{
    f64 result = 1.0;
    for (u32 i = 0; i < exponent; ++i)
    {
        result *= 2.0;
    }
    return result;
}

/** Largest number of fractional bits T can give the constant without losing its integer part. */
template <FixedPointStorage T>
constexpr u32 FixedPointConstantShift(f64 value)
{
    const f64 magnitude = value < 0.0 ? -value : value;
    const f64 limit = FixedPointPow2(k_fixed_point_bit_count_value<T> - 1);
    u32 shift = 0;
    while (shift + 1 < k_fixed_point_bit_count_value<T> && magnitude * FixedPointPow2(shift + 1) < limit)
    {
        ++shift;
    }
    return shift;
}

/** Rounds value scaled by 2^shift to the nearest T. */
template <FixedPointStorage T>
constexpr T FixedPointScale(f64 value, u32 shift)
{
    const f64 scaled = value * FixedPointPow2(shift);
    return static_cast<T>(scaled + (scaled < 0.0 ? -0.5 : 0.5));
}

/** Adds two raw values. Wraps on overflow; debug builds report it. */
template <FixedPointStorage T>
constexpr T FixedPointAdd(T a, T b)
{
    using Unsigned = typename FixedPointUnsigned<T>::Type;
    const T result = static_cast<T>(static_cast<Unsigned>(static_cast<Unsigned>(a) + static_cast<Unsigned>(b)));
    OPAL_ASSERT(((a ^ result) & (b ^ result)) >= 0, "Fixed point addition overflowed");
    return result;
}

/** Subtracts two raw values. Wraps on overflow; debug builds report it. */
template <FixedPointStorage T>
constexpr T FixedPointSub(T a, T b)
{
    using Unsigned = typename FixedPointUnsigned<T>::Type;
    const T result = static_cast<T>(static_cast<Unsigned>(static_cast<Unsigned>(a) - static_cast<Unsigned>(b)));
    OPAL_ASSERT(((a ^ b) & (a ^ result)) >= 0, "Fixed point subtraction overflowed");
    return result;
}

/** Negates a raw value. Wraps on overflow; debug builds report it. */
template <FixedPointStorage T>
constexpr T FixedPointNegate(T a)
{
    using Unsigned = typename FixedPointUnsigned<T>::Type;
    OPAL_ASSERT(a != k_fixed_point_lowest_value<T>, "Fixed point negation overflowed");
    return static_cast<T>(static_cast<Unsigned>(0) - static_cast<Unsigned>(a));
}

/**
 * Computes (a * b) >> shift over the exact product, rounding halves up.
 * @note The caller guarantees the outcome fits in T. Debug builds report it when it does not.
 */
template <FixedPointStorage T>
T FixedPointMulShift(T a, T b, u32 shift)
{
    if constexpr (k_fixed_point_has_wide_value<T>)
    {
        using Wide = typename FixedPointWide<T>::Type;
        const Wide bias = shift == 0 ? static_cast<Wide>(0) : static_cast<Wide>(static_cast<Wide>(1) << (shift - 1));
        const Wide result = static_cast<Wide>((static_cast<Wide>(a) * static_cast<Wide>(b) + bias) >> shift);
        OPAL_ASSERT(
            result >= static_cast<Wide>(k_fixed_point_lowest_value<T>) && result <= static_cast<Wide>(k_fixed_point_highest_value<T>),
            "Fixed point multiplication overflowed");
        return static_cast<T>(result);
    }
    else
    {
#if defined(OPAL_COMPILER_MSVC)
        i64 high = 0;
        u64 low = static_cast<u64>(_mul128(a, b, &high));
        if (shift > 0)
        {
            const u64 biased_low = low + (1ULL << (shift - 1));
            if (biased_low < low)
            {
                ++high;
            }
            low = biased_low;
        }
        const i64 result = static_cast<i64>(__shiftright128(low, static_cast<u64>(high), static_cast<u8>(shift)));
        OPAL_ASSERT((high >> shift) == (result >> 63), "Fixed point multiplication overflowed");
        return result;
#else
        static_assert(k_always_false_value<T>, "No wide type for this storage type");
        return static_cast<T>(0);
#endif
    }
}

/**
 * Computes (a << shift) / b over the exact numerator, truncating towards zero.
 * @note The caller guarantees b is not zero and that the outcome fits in T. Debug builds report it when it does not.
 */
template <FixedPointStorage T>
T FixedPointDivShift(T a, T b, u32 shift)
{
    OPAL_ASSERT(b != 0, "Fixed point division by zero");
    if constexpr (k_fixed_point_has_wide_value<T>)
    {
        using Wide = typename FixedPointWide<T>::Type;
        const Wide result = static_cast<Wide>((static_cast<Wide>(a) << shift) / static_cast<Wide>(b));
        OPAL_ASSERT(
            result >= static_cast<Wide>(k_fixed_point_lowest_value<T>) && result <= static_cast<Wide>(k_fixed_point_highest_value<T>),
            "Fixed point division overflowed");
        return static_cast<T>(result);
    }
    else
    {
#if defined(OPAL_COMPILER_MSVC)
        const u64 low = static_cast<u64>(a) << shift;
        const i64 high = shift == 0 ? (a >> 63) : (a >> (64 - shift));
#if defined(OPAL_DEBUG)
        // _div128 raises a hardware exception when the quotient does not fit, so the check has to run before it.
        // The quotient fits when the numerator magnitude shifted down 63 bits stays under the divisor magnitude,
        // plus the one extra value the negative side has room for.
        u64 magnitude_low = low;
        u64 magnitude_high = static_cast<u64>(high);
        if (high < 0)
        {
            magnitude_low = 0ULL - low;
            magnitude_high = ~magnitude_high + (low == 0 ? 1ULL : 0ULL);
        }
        const u64 divisor_magnitude = b < 0 ? 0ULL - static_cast<u64>(b) : static_cast<u64>(b);
        const u64 quotient_magnitude_high = (magnitude_high << 1) | (magnitude_low >> 63);
        const u64 numerator_low_bits = magnitude_low & ((1ULL << 63) - 1ULL);
        const bool is_negative = (a < 0) != (b < 0);
        OPAL_ASSERT(quotient_magnitude_high < divisor_magnitude ||
                        (is_negative && quotient_magnitude_high == divisor_magnitude && numerator_low_bits < divisor_magnitude),
                    "Fixed point division overflowed");
#endif
        i64 remainder = 0;
        return _div128(high, static_cast<i64>(low), b, &remainder);
#else
        static_assert(k_always_false_value<T>, "No wide type for this storage type");
        return static_cast<T>(0);
#endif
    }
}

/** Zero based index of the most significant set bit. The caller guarantees the value is positive. */
template <FixedPointStorage T>
u32 FixedPointHighestSetBit(T value)
{
    OPAL_ASSERT(value > 0, "Fixed point bit scan of a non positive value");
    if constexpr (sizeof(T) <= 4)
    {
        return 31u - CountLeadingZeros(static_cast<u32>(value));
    }
    else
    {
        return 63u - static_cast<u32>(CountLeadingZeros(static_cast<u64>(value)));
    }
}

}  // namespace Impl

// ------------------------------------------------------------------------------------------------
// FixedPoint.
// ------------------------------------------------------------------------------------------------

/**
 * @brief A real number held as an integer scaled by a power of two.
 *
 * Arithmetic is exact integer arithmetic, so the same operands produce the same bits on every platform, compiler and
 * optimization level. That is what separates it from f32 and f64 and what makes it usable for lockstep simulation and
 * replays. In exchange the range is small and fixed, and there are no infinities and no NaN.
 *
 * Results that do not fit wrap around, the way the underlying integer does, except in division: a quotient that does
 * not fit, or a zero divisor, is a contract violation with no defined result. Debug builds report all of these instead.
 *
 * Construction from an integer is exact and implicit. Construction from a floating point value rounds to the nearest
 * representable value and is explicit, since it loses precision.
 *
 * @tparam T Storage type. The sign bit and the fractional bits leave the rest of it for the integer part.
 * @tparam k_frac_bits Number of fractional bits, which fixes the resolution at 2^-k_frac_bits.
 */
template <FixedPointStorage T, u32 k_frac_bits>
struct FixedPoint
{
    static_assert(k_frac_bits > 0, "A fixed point value needs at least one fractional bit");
    static_assert(k_frac_bits <= Impl::k_fixed_point_bit_count_value<T> - 2, "A fixed point value needs an integer bit besides the sign");

    using storage_type = T;

    static constexpr u32 k_frac_bits_value = k_frac_bits;
    static constexpr u32 k_bit_count_value = Impl::k_fixed_point_bit_count_value<T>;

    /** The scaled integer. The value it stands for is raw * 2^-k_frac_bits. */
    T raw;

    /** Default constructor. No initialization is performed. */
    constexpr FixedPoint() = default;

    /**
     * Constructs an exact value from an integer.
     * @param value The integer to represent. Must fit the integer part.
     */
    template <Integral U>
    constexpr FixedPoint(U value);

    /**
     * Constructs the representable value nearest to a floating point number.
     * @param value The number to represent. Must fit the range of the type.
     */
    template <FloatingPoint U>
    constexpr explicit FixedPoint(U value);

    /** @return The value whose scaled integer is the one given. */
    [[nodiscard]] static constexpr FixedPoint FromRaw(T raw_value);

    /** @return Zero. */
    [[nodiscard]] static constexpr FixedPoint Zero();

    /** @return One. */
    [[nodiscard]] static constexpr FixedPoint One();

    /** @return The most negative representable value. */
    [[nodiscard]] static constexpr FixedPoint Min();

    /** @return The largest representable value. */
    [[nodiscard]] static constexpr FixedPoint Max();

    /** @return The smallest representable value greater than zero. */
    [[nodiscard]] static constexpr FixedPoint Epsilon();

    /** @return Pi, rounded to the nearest representable value. */
    [[nodiscard]] static constexpr FixedPoint Pi();

    /** @return Two pi, rounded to the nearest representable value. */
    [[nodiscard]] static constexpr FixedPoint TwoPi();

    /** @return Half of pi, rounded to the nearest representable value. */
    [[nodiscard]] static constexpr FixedPoint HalfPi();

    /** @return The value as a single precision float. */
    constexpr explicit operator f32() const;

    /** @return The value as a double precision float. */
    constexpr explicit operator f64() const;

    /** @return The integer part, truncated towards zero. */
    constexpr explicit operator T() const;

    constexpr FixedPoint operator+() const;
    constexpr FixedPoint operator-() const;

    FixedPoint& operator+=(const FixedPoint& other);
    FixedPoint& operator-=(const FixedPoint& other);
    FixedPoint& operator*=(const FixedPoint& other);
    FixedPoint& operator/=(const FixedPoint& other);

    // The binary operators are friends declared in place so that an integer on either side converts. That is what lets
    // generic code written for f32, down to expressions as small as 1 - t, compile unchanged for this type.

    friend constexpr bool operator==(const FixedPoint& a, const FixedPoint& b) { return a.raw == b.raw; }
    friend constexpr bool operator!=(const FixedPoint& a, const FixedPoint& b) { return a.raw != b.raw; }
    friend constexpr bool operator<(const FixedPoint& a, const FixedPoint& b) { return a.raw < b.raw; }
    friend constexpr bool operator<=(const FixedPoint& a, const FixedPoint& b) { return a.raw <= b.raw; }
    friend constexpr bool operator>(const FixedPoint& a, const FixedPoint& b) { return a.raw > b.raw; }
    friend constexpr bool operator>=(const FixedPoint& a, const FixedPoint& b) { return a.raw >= b.raw; }

    friend constexpr FixedPoint operator+(const FixedPoint& a, const FixedPoint& b)
    {
        return FromRaw(Impl::FixedPointAdd<T>(a.raw, b.raw));
    }

    friend constexpr FixedPoint operator-(const FixedPoint& a, const FixedPoint& b)
    {
        return FromRaw(Impl::FixedPointSub<T>(a.raw, b.raw));
    }

    friend FixedPoint operator*(const FixedPoint& a, const FixedPoint& b)
    {
        return FromRaw(Impl::FixedPointMulShift<T>(a.raw, b.raw, k_frac_bits));
    }

    friend FixedPoint operator/(const FixedPoint& a, const FixedPoint& b)
    {
        return FromRaw(Impl::FixedPointDivShift<T>(a.raw, b.raw, k_frac_bits));
    }
};

/** Signed 32-bit fixed point with 16 fractional bits. Spans about -32768 to 32768 with a resolution of about 1.5e-5. */
using Fixed32 = FixedPoint<i32, 16>;

/** Signed 64-bit fixed point with 32 fractional bits. Spans about -2.1e9 to 2.1e9 with a resolution of about 2.3e-10. */
using Fixed64 = FixedPoint<i64, 32>;

template <FixedPointStorage T, u32 k_frac_bits>
inline constexpr bool k_is_fixed_point_value<FixedPoint<T, k_frac_bits>> = true;

// ------------------------------------------------------------------------------------------------
// Fixed point overloads of the math functions.
// ------------------------------------------------------------------------------------------------

/**
 * @brief Returns the largest integer value not greater than the given one.
 * @param value The value to take the floor of.
 * @return The floor of the given value.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] constexpr FixedPoint<T, k_frac_bits> Floor(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the smallest integer value not less than the given one.
 * @param value The value to take the ceil of.
 * @return The ceil of the given value.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] constexpr FixedPoint<T, k_frac_bits> Ceil(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the rounded value.
 * @param value The value to round.
 * @return The rounded value. A value exactly halfway between two integers rounds away from zero.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] constexpr FixedPoint<T, k_frac_bits> Round(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the remainder of a / b.
 * @param a The dividend.
 * @param b The divisor. Must not be zero.
 * @return The remainder of a / b. The result has the same sign as a.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] constexpr FixedPoint<T, k_frac_bits> Mod(FixedPoint<T, k_frac_bits> a, FixedPoint<T, k_frac_bits> b);

/**
 * @brief Returns the square root of the given value.
 * @param value The value to take the square root of. Must not be negative.
 * @return The square root, accurate to within one resolution step. Zero for a negative value.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Sqrt(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the linear interpolation between the given values.
 * @param t The interpolation factor.
 * @param p0 The first value.
 * @param p1 The second value.
 * @return The linear interpolation between the given values. Exact at t equal to zero and one.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Lerp(FixedPoint<T, k_frac_bits> t, FixedPoint<T, k_frac_bits> p0, FixedPoint<T, k_frac_bits> p1);

/**
 * @brief Converts the given degrees to radians.
 * @param degrees The degrees to convert.
 * @return The given degrees in radians.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Radians(FixedPoint<T, k_frac_bits> degrees);

/**
 * @brief Converts the given radians to degrees.
 * @param radians The radians to convert.
 * @return The given radians in degrees.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Degrees(FixedPoint<T, k_frac_bits> radians);

/**
 * @brief Returns the sine of the given value.
 * @param radians The value to take the sine of in radians. Any magnitude is accepted, though a large one carries the
 * loss of precision that comes with holding it in this type.
 * @return The sine of the given value, never outside the range of minus one to one.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Sin(FixedPoint<T, k_frac_bits> radians);

/**
 * @brief Returns the cosine of the given value.
 * @param radians The value to take the cosine of in radians. Any magnitude is accepted, though a large one carries the
 * loss of precision that comes with holding it in this type.
 * @return The cosine of the given value, never outside the range of minus one to one.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Cos(FixedPoint<T, k_frac_bits> radians);

/**
 * @brief Returns the tangent of the given value.
 * @param radians The value to take the tangent of in radians.
 * @return The tangent of the given value. Where the tangent has a pole, the largest representable value of matching
 * sign is returned.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Tan(FixedPoint<T, k_frac_bits> radians);

/**
 * @brief Returns the angle between the positive x axis and the point (x, y).
 * @param y The y coordinate of the point.
 * @param x The x coordinate of the point.
 * @return The angle in radians, in the range of minus pi to pi. Zero when both coordinates are zero. The type needs
 * three integer bits to hold the result.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> ArcTan2(FixedPoint<T, k_frac_bits> y, FixedPoint<T, k_frac_bits> x);

/**
 * @brief Returns the arc tangent of the given value.
 * @param value The value to take the arc tangent of.
 * @return The arc tangent of the given value in radians, in the range of minus half pi to half pi.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> ArcTan(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the arc sine of the given value.
 * @param value The value to take the arc sine of. Must be in the range of minus one to one.
 * @return The arc sine of the given value in radians, in the range of minus half pi to half pi. Accuracy falls off as
 * the value approaches either end of its range, where the result turns on a difference of two nearly equal numbers.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> ArcSin(FixedPoint<T, k_frac_bits> value);

/**
 * @brief Returns the arc cosine of the given value.
 * @param value The value to take the arc cosine of. Must be in the range of minus one to one.
 * @return The arc cosine of the given value in radians, in the range of zero to pi. Accuracy falls off as the value
 * approaches either end of its range.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> ArcCos(FixedPoint<T, k_frac_bits> value);

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
template <Opal::Integral U>
constexpr Opal::FixedPoint<T, k_frac_bits>::FixedPoint(U value)
{
    using Unsigned = typename Impl::FixedPointUnsigned<T>::Type;
    OPAL_ASSERT(static_cast<i64>(value) >= (static_cast<i64>(Impl::k_fixed_point_lowest_value<T>) >> k_frac_bits) &&
                    static_cast<i64>(value) <= (static_cast<i64>(Impl::k_fixed_point_highest_value<T>) >> k_frac_bits),
                "Fixed point integer is out of range");
    raw = static_cast<T>(static_cast<Unsigned>(static_cast<Unsigned>(value) << k_frac_bits));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
template <Opal::FloatingPoint U>
constexpr Opal::FixedPoint<T, k_frac_bits>::FixedPoint(U value)
{
    const f64 scaled = static_cast<f64>(value) * Impl::FixedPointPow2(k_frac_bits);
    OPAL_ASSERT(scaled >= -Impl::FixedPointPow2(k_bit_count_value - 1) && scaled < Impl::FixedPointPow2(k_bit_count_value - 1),
                "Fixed point value is out of range");
    raw = static_cast<T>(scaled + (scaled < 0.0 ? -0.5 : 0.5));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::FromRaw(T raw_value)
{
    FixedPoint result;
    result.raw = raw_value;
    return result;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::Zero()
{
    return FromRaw(static_cast<T>(0));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::One()
{
    return FromRaw(static_cast<T>(static_cast<T>(1) << k_frac_bits));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::Min()
{
    return FromRaw(Impl::k_fixed_point_lowest_value<T>);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::Max()
{
    return FromRaw(Impl::k_fixed_point_highest_value<T>);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::Epsilon()
{
    return FromRaw(static_cast<T>(1));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::Pi()
{
    return FixedPoint(k_pi_double);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::TwoPi()
{
    return FixedPoint(2.0 * k_pi_double);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::HalfPi()
{
    return FixedPoint(0.5 * k_pi_double);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits>::operator Opal::f32() const
{
    return static_cast<f32>(static_cast<f64>(raw) / Impl::FixedPointPow2(k_frac_bits));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits>::operator Opal::f64() const
{
    return static_cast<f64>(raw) / Impl::FixedPointPow2(k_frac_bits);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits>::operator T() const
{
    const T bias = raw < 0 ? static_cast<T>((static_cast<T>(1) << k_frac_bits) - 1) : static_cast<T>(0);
    return static_cast<T>(static_cast<T>(raw + bias) >> k_frac_bits);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::operator+() const
{
    return *this;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::FixedPoint<T, k_frac_bits>::operator-() const
{
    return FromRaw(Impl::FixedPointNegate<T>(raw));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits>& Opal::FixedPoint<T, k_frac_bits>::operator+=(const FixedPoint& other)
{
    raw = Impl::FixedPointAdd<T>(raw, other.raw);
    return *this;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits>& Opal::FixedPoint<T, k_frac_bits>::operator-=(const FixedPoint& other)
{
    raw = Impl::FixedPointSub<T>(raw, other.raw);
    return *this;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits>& Opal::FixedPoint<T, k_frac_bits>::operator*=(const FixedPoint& other)
{
    raw = Impl::FixedPointMulShift<T>(raw, other.raw, k_frac_bits);
    return *this;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits>& Opal::FixedPoint<T, k_frac_bits>::operator/=(const FixedPoint& other)
{
    raw = Impl::FixedPointDivShift<T>(raw, other.raw, k_frac_bits);
    return *this;
}

namespace Opal::Impl
{

/** Evaluates the sine of a quarter turn scaled by f, with f in [0, 1]. Argument and result are raw values. */
template <FixedPointStorage T, u32 k_frac_bits>
T FixedPointSinQuarterTurn(T f)
{
    constexpr T k_c1 = FixedPointScale<T>(1.5707963267948966, k_frac_bits);
    constexpr T k_c3 = FixedPointScale<T>(-0.6459640975062462, k_frac_bits);
    constexpr T k_c5 = FixedPointScale<T>(0.0796926262461670, k_frac_bits);
    constexpr T k_c7 = FixedPointScale<T>(-0.0046817541353187, k_frac_bits);
    constexpr T k_c9 = FixedPointScale<T>(0.0001568986005013, k_frac_bits);

    const T f_squared = FixedPointMulShift<T>(f, f, k_frac_bits);
    T accumulator = k_c9;
    accumulator = static_cast<T>(k_c7 + FixedPointMulShift<T>(f_squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c5 + FixedPointMulShift<T>(f_squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c3 + FixedPointMulShift<T>(f_squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c1 + FixedPointMulShift<T>(f_squared, accumulator, k_frac_bits));
    return FixedPointMulShift<T>(f, accumulator, k_frac_bits);
}

/** Evaluates the sine of an angle given in turns rather than radians. Argument and result are raw values. */
template <FixedPointStorage T, u32 k_frac_bits>
T FixedPointSinFromTurns(T turns)
{
    static_assert(k_frac_bits >= 4, "Fixed point trigonometry needs at least four fractional bits");

    constexpr T k_one = static_cast<T>(static_cast<T>(1) << k_frac_bits);
    constexpr T k_turn_mask = static_cast<T>(k_one - 1);
    constexpr T k_quadrant_mask = static_cast<T>((static_cast<T>(1) << (k_frac_bits - 2)) - 1);

    const T turn = static_cast<T>(turns & k_turn_mask);
    const u32 quadrant = static_cast<u32>(turn >> (k_frac_bits - 2));
    T quarter = static_cast<T>(static_cast<T>(turn & k_quadrant_mask) << 2);
    if (quadrant == 1 || quadrant == 3)
    {
        quarter = static_cast<T>(k_one - quarter);
    }

    T result = FixedPointSinQuarterTurn<T, k_frac_bits>(quarter);
    if (quadrant >= 2)
    {
        result = static_cast<T>(-result);
    }
    if (result > k_one)
    {
        result = k_one;
    }
    if (result < static_cast<T>(-k_one))
    {
        result = static_cast<T>(-k_one);
    }
    return result;
}

/** Converts an angle in radians to the same angle in turns. Argument and result are raw values. */
template <FixedPointStorage T, u32 k_frac_bits>
T FixedPointRadiansToTurns(T radians)
{
    constexpr u32 k_shift = FixedPointConstantShift<T>(k_inv_2pi_double);
    constexpr T k_inv_two_pi = FixedPointScale<T>(k_inv_2pi_double, k_shift);
    return FixedPointMulShift<T>(radians, k_inv_two_pi, k_shift);
}

/**
 * Evaluates the arc tangent of a ratio whose magnitude is at most one. Argument and result are raw values.
 *
 * A ratio above the tangent of a twelfth of a turn is folded below it first, which is what keeps the series short
 * enough that its truncation stays under one resolution step at every supported width.
 */
template <FixedPointStorage T, u32 k_frac_bits>
T FixedPointArcTanUnitRatio(T ratio)
{
    static_assert(k_frac_bits + 3 <= k_fixed_point_bit_count_value<T>,
                  "Fixed point inverse trigonometry needs three integer bits to hold pi");

    constexpr T k_one = static_cast<T>(static_cast<T>(1) << k_frac_bits);
    constexpr T k_tan_twelfth_turn = FixedPointScale<T>(0.2679491924311227064725, k_frac_bits);
    constexpr T k_sqrt_three = FixedPointScale<T>(1.7320508075688772935274, k_frac_bits);
    constexpr T k_sixth_pi = FixedPointScale<T>(0.5235987755982988730771, k_frac_bits);

    constexpr T k_c1 = FixedPointScale<T>(1.0, k_frac_bits);
    constexpr T k_c3 = FixedPointScale<T>(-1.0 / 3.0, k_frac_bits);
    constexpr T k_c5 = FixedPointScale<T>(1.0 / 5.0, k_frac_bits);
    constexpr T k_c7 = FixedPointScale<T>(-1.0 / 7.0, k_frac_bits);
    constexpr T k_c9 = FixedPointScale<T>(1.0 / 9.0, k_frac_bits);
    constexpr T k_c11 = FixedPointScale<T>(-1.0 / 11.0, k_frac_bits);
    constexpr T k_c13 = FixedPointScale<T>(1.0 / 13.0, k_frac_bits);
    constexpr T k_c15 = FixedPointScale<T>(-1.0 / 15.0, k_frac_bits);

    const bool is_negative = ratio < 0;
    T magnitude = is_negative ? FixedPointNegate<T>(ratio) : ratio;

    T offset = static_cast<T>(0);
    if (magnitude > k_tan_twelfth_turn)
    {
        const T numerator = FixedPointSub<T>(FixedPointMulShift<T>(magnitude, k_sqrt_three, k_frac_bits), k_one);
        const T denominator = FixedPointAdd<T>(magnitude, k_sqrt_three);
        magnitude = FixedPointDivShift<T>(numerator, denominator, k_frac_bits);
        offset = k_sixth_pi;
    }

    const T squared = FixedPointMulShift<T>(magnitude, magnitude, k_frac_bits);
    T accumulator = k_c15;
    accumulator = static_cast<T>(k_c13 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c11 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c9 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c7 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c5 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c3 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));
    accumulator = static_cast<T>(k_c1 + FixedPointMulShift<T>(squared, accumulator, k_frac_bits));

    const T result = FixedPointAdd<T>(offset, FixedPointMulShift<T>(magnitude, accumulator, k_frac_bits));
    return is_negative ? FixedPointNegate<T>(result) : result;
}

/** Evaluates the angle of the point (x, y) measured from the positive x axis. Arguments and result are raw values. */
template <FixedPointStorage T, u32 k_frac_bits>
T FixedPointArcTan2(T y, T x)
{
    constexpr T k_half_pi = FixedPointScale<T>(0.5 * k_pi_double, k_frac_bits);
    constexpr T k_whole_pi = FixedPointScale<T>(k_pi_double, k_frac_bits);

    if (x == 0 && y == 0)
    {
        return static_cast<T>(0);
    }

    const T y_magnitude = y < 0 ? FixedPointNegate<T>(y) : y;
    const T x_magnitude = x < 0 ? FixedPointNegate<T>(x) : x;

    T angle;
    if (x_magnitude >= y_magnitude)
    {
        const T ratio = FixedPointDivShift<T>(y_magnitude, x_magnitude, k_frac_bits);
        angle = FixedPointArcTanUnitRatio<T, k_frac_bits>(ratio);
    }
    else
    {
        const T ratio = FixedPointDivShift<T>(x_magnitude, y_magnitude, k_frac_bits);
        angle = FixedPointSub<T>(k_half_pi, FixedPointArcTanUnitRatio<T, k_frac_bits>(ratio));
    }

    if (x < 0)
    {
        angle = FixedPointSub<T>(k_whole_pi, angle);
    }
    return y < 0 ? FixedPointNegate<T>(angle) : angle;
}

}  // namespace Opal::Impl

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::Floor(FixedPoint<T, k_frac_bits> value)
{
    constexpr T k_frac_mask = static_cast<T>((static_cast<T>(1) << k_frac_bits) - 1);
    return FixedPoint<T, k_frac_bits>::FromRaw(static_cast<T>(value.raw & ~k_frac_mask));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::Ceil(FixedPoint<T, k_frac_bits> value)
{
    constexpr T k_frac_mask = static_cast<T>((static_cast<T>(1) << k_frac_bits) - 1);
    const T carried = Impl::FixedPointAdd<T>(value.raw, k_frac_mask);
    return FixedPoint<T, k_frac_bits>::FromRaw(static_cast<T>(carried & ~k_frac_mask));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::Round(FixedPoint<T, k_frac_bits> value)
{
    constexpr T k_frac_mask = static_cast<T>((static_cast<T>(1) << k_frac_bits) - 1);
    constexpr T k_half = static_cast<T>(static_cast<T>(1) << (k_frac_bits - 1));
    if ((value.raw & k_frac_mask) == 0)
    {
        return value;
    }
    if (value.raw >= 0)
    {
        return Floor(FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointAdd<T>(value.raw, k_half)));
    }
    return Ceil(FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointSub<T>(value.raw, k_half)));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
constexpr Opal::FixedPoint<T, k_frac_bits> Opal::Mod(FixedPoint<T, k_frac_bits> a, FixedPoint<T, k_frac_bits> b)
{
    OPAL_ASSERT(b.raw != 0, "Fixed point modulo by zero");
    // Every remainder by one raw step is zero, and saying so keeps the hardware away from the one pair it traps on,
    // the most negative value over minus one step.
    if (b.raw == static_cast<T>(1) || b.raw == static_cast<T>(-1))
    {
        return FixedPoint<T, k_frac_bits>::Zero();
    }
    return FixedPoint<T, k_frac_bits>::FromRaw(static_cast<T>(a.raw % b.raw));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Sqrt(FixedPoint<T, k_frac_bits> value)
{
    OPAL_ASSERT(value.raw >= 0, "Square root of a negative fixed point value");
    if (value.raw <= 0)
    {
        return FixedPoint<T, k_frac_bits>::Zero();
    }

    // The iteration converges only from above the root. When the starting power of two does not fit in T, the largest
    // representable value still sits above every root the type can produce, so it takes over as the starting point.
    constexpr u32 k_highest_guess_shift = FixedPoint<T, k_frac_bits>::k_bit_count_value - 2;
    const u32 highest_bit = Impl::FixedPointHighestSetBit<T>(value.raw);
    const u32 guess_shift = (highest_bit + k_frac_bits) / 2 + 1;

    T estimate = guess_shift > k_highest_guess_shift ? Impl::k_fixed_point_highest_value<T>
                                                     : static_cast<T>(static_cast<T>(1) << guess_shift);
    while (true)
    {
        const T quotient = Impl::FixedPointDivShift<T>(value.raw, estimate, k_frac_bits);
        const T next =
            static_cast<T>(static_cast<T>(estimate >> 1) + static_cast<T>(quotient >> 1) + static_cast<T>((estimate & 1) & (quotient & 1)));
        if (next >= estimate)
        {
            break;
        }
        estimate = next;
    }
    return FixedPoint<T, k_frac_bits>::FromRaw(estimate);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Lerp(FixedPoint<T, k_frac_bits> t, FixedPoint<T, k_frac_bits> p0, FixedPoint<T, k_frac_bits> p1)
{
    return (1 - t) * p0 + t * p1;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Radians(FixedPoint<T, k_frac_bits> degrees)
{
    constexpr f64 k_radians_per_degree = k_pi_double / 180.0;
    constexpr u32 k_shift = Impl::FixedPointConstantShift<T>(k_radians_per_degree);
    constexpr T k_factor = Impl::FixedPointScale<T>(k_radians_per_degree, k_shift);
    return FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointMulShift<T>(degrees.raw, k_factor, k_shift));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Degrees(FixedPoint<T, k_frac_bits> radians)
{
    constexpr f64 k_degrees_per_radian = 180.0 / k_pi_double;
    constexpr u32 k_shift = Impl::FixedPointConstantShift<T>(k_degrees_per_radian);
    constexpr T k_factor = Impl::FixedPointScale<T>(k_degrees_per_radian, k_shift);
    return FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointMulShift<T>(radians.raw, k_factor, k_shift));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Sin(FixedPoint<T, k_frac_bits> radians)
{
    const T turns = Impl::FixedPointRadiansToTurns<T, k_frac_bits>(radians.raw);
    return FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointSinFromTurns<T, k_frac_bits>(turns));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Cos(FixedPoint<T, k_frac_bits> radians)
{
    using Unsigned = typename Impl::FixedPointUnsigned<T>::Type;
    constexpr T k_quarter_turn = static_cast<T>(static_cast<T>(1) << (k_frac_bits - 2));
    const T turns = Impl::FixedPointRadiansToTurns<T, k_frac_bits>(radians.raw);
    const T shifted = static_cast<T>(static_cast<Unsigned>(static_cast<Unsigned>(turns) + static_cast<Unsigned>(k_quarter_turn)));
    return FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointSinFromTurns<T, k_frac_bits>(shifted));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Tan(FixedPoint<T, k_frac_bits> radians)
{
    const FixedPoint<T, k_frac_bits> sine = Sin(radians);
    const FixedPoint<T, k_frac_bits> cosine = Cos(radians);
    const T sine_magnitude = sine.raw < 0 ? static_cast<T>(-sine.raw) : sine.raw;
    const T cosine_magnitude = cosine.raw < 0 ? static_cast<T>(-cosine.raw) : cosine.raw;
    const T representable = cosine_magnitude == 0
                                ? static_cast<T>(0)
                                : Impl::FixedPointMulShift<T>(Impl::k_fixed_point_highest_value<T>, cosine_magnitude, k_frac_bits);
    if (sine_magnitude > representable)
    {
        const bool is_negative = (sine.raw < 0) != (cosine.raw < 0);
        return is_negative ? FixedPoint<T, k_frac_bits>::Min() : FixedPoint<T, k_frac_bits>::Max();
    }
    return sine / cosine;
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::ArcTan2(FixedPoint<T, k_frac_bits> y, FixedPoint<T, k_frac_bits> x)
{
    return FixedPoint<T, k_frac_bits>::FromRaw(Impl::FixedPointArcTan2<T, k_frac_bits>(y.raw, x.raw));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::ArcTan(FixedPoint<T, k_frac_bits> value)
{
    return ArcTan2(value, FixedPoint<T, k_frac_bits>::One());
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::ArcSin(FixedPoint<T, k_frac_bits> value)
{
    using Fixed = FixedPoint<T, k_frac_bits>;
    OPAL_ASSERT(value >= Fixed(-1) && value <= Fixed(1), "Arc sine of a fixed point value outside of minus one to one");
    // A value at either end of the range squares to a hair over one once rounding is in play, so what is left of the
    // radicand has to be held at zero for the root to stay defined.
    const Fixed radicand = 1 - value * value;
    return ArcTan2(value, Sqrt(radicand.raw < 0 ? Fixed::Zero() : radicand));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::ArcCos(FixedPoint<T, k_frac_bits> value)
{
    return FixedPoint<T, k_frac_bits>::HalfPi() - ArcSin(value);
}

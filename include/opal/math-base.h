#pragma once

#include <cmath>

#include "opal/type-traits.h"
#include "opal/math-constants.h"

namespace Opal
{

/**
 * @brief Returns the absolute value of the given value.
 * @tparam T Value type.
 * @param value The value to take the absolute value of.
 * @return The absolute value of the given value.
 */
template <typename T>
T Abs(T value);

/**
 * @brief Checks if the given values are equal within the given epsilon.
 * @tparam T Value type.
 * @param a First value.
 * @param b Second value.
 * @param epsilon Epsilon.
 * @return True if the values are equal within the given epsilon, false otherwise.
 */
template <typename T>
bool IsEqual(T a, T b, T epsilon);

/**
 * @brief Returns the minimum of the given values.
 * @tparam T Value type.
 * @param A First value.
 * @param B Second value.
 * @return Returns the minimum of the given values.
 */
template <typename T>
T Min(T A, T B);

/**
 * @brief Returns the maximum of the given values.
 * @tparam T Value type.
 * @param A First value.
 * @param B Second value.
 * @return Returns the maximum of the given values.
 */
template <typename T>
T Max(T A, T B);

/**
 * Checks if the given value is NaN.
 * @tparam T Value type.
 * @param value The value to check.
 * @return True if the given value is NaN, false otherwise.
 */
template <typename T>
bool IsNaN(T value);

/**
 * @brief Checks if the given value is finite.
 * @tparam T Value type.
 * @param value The value to check.
 * @return True if the given value is finite, false otherwise.
 */
template <typename T>
bool IsFinite(T value);

/**
 * @brief Converts the given degrees to radians.
 * @tparam T Value type. Must be a floating point type.
 * @param degrees The degrees to convert.
 * @return The given degrees in radians.
 */
template <FloatingPoint T>
T Radians(T degrees);

/**
 * @brief Converts the given radians to degrees.
 * @tparam T Value type. Must be a floating point type.
 * @param radians The radians to convert.
 * @return The given radians in degrees.
 */
template <FloatingPoint T>
T Degrees(T radians);

/**
 * Clamps the given value between the given low and high values.
 * @tparam T Value type.
 * @param value Value to clamp.
 * @param low Low value.
 * @param high High value.
 * @return The clamped value.
 */
template <typename T>
T Clamp(T value, T low, T high);

/**
 * @brief Returns the remainder of A / B.
 * @param a The dividend.
 * @param b The divisor.
 * @return The remainder of A / B. Result will have the same sign as A. If A or B is NAN or B is 0,
 * NAN is returned.
 */
template <typename T>
T Mod(T a, T b);

/**
 * @brief Returns the square root of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param value The value to take the square root of.
 * @return The square root of the given value.
 */
template <FloatingPoint T>
T Sqrt(T value);

/**
 * Returns the linear interpolation between the given values.
 * @tparam T Value type. Must be a floating point type.
 * @param t The interpolation factor.
 * @param p0 The first value.
 * @param p1 The second value.
 * @return The linear interpolation between the given values.
 */
template <FloatingPoint T>
T Lerp(T t, T p0, T p1);

/**
 * @brief Returns the rounded value.
 * @tparam T Value type. Must be a floating point type.
 * @param value The value to round.
 * @return The rounded value. If the value is exactly halfway between two integers, the rounding is
 * done away from zero.
 */
template <FloatingPoint T>
T Round(T value);

/**
 * @brief Returns the floor of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param value The value to take the floor of.
 * @return The floor of the given value.
 */
template <FloatingPoint T>
T Floor(T value);

/**
 * @brief Returns the ceil of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param value The value to take the ceil of.
 * @return The ceil of the given value.
 */
template <FloatingPoint T>
T Ceil(T value);

/**
 * @brief Returns the sine of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param radians The value to take the sine of in radians.
 * @return The sine of the given value.
 */
template <FloatingPoint T>
T Sin(T radians);

/**
 * @brief Returns the cosine of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param radians The value to take the cosine of in radians.
 * @return The cosine of the given value.
 */
template <FloatingPoint T>
T Cos(T radians);

/**
 * @brief Returns the tangent of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param radians The value to take the tangent of in radians.
 * @return The tangent of the given value.
 */
template <FloatingPoint T>
T Tan(T radians);

/**
 * @brief Returns the base raised to the exponent power.
 * @tparam T Value type. Must be a floating point type.
 * @param base The base.
 * @param exponent The exponent.
 * @return The base raised to the exponent power.
 */
template <FloatingPoint T>
T Power(T base, T exponent);

/**
 * @brief Returns the logarithm base e of the given value.
 * @tparam T Value type. Must be a floating point type.
 * @param value The value to take the logarithm base e of.
 * @return The logarithm base e of the given value. If value is 1 it returns 0. If value is 0 it
 * returns negative infinity. If value is positive infinity, it returns positive infinity. If value
 * is negative, negative infinity or NaN, it returns NaN.
 */
template <FloatingPoint T>
T LogNatural(T value);

/**
 * @brief Returns the logarithm base 2 of the given value.
 * @param value The value to take the logarithm base 2 of.
 * @return The logarithm base 2 of the given value. If value is 1 it returns 0. If value is 0 it
 * returns negative infinity. If value is positive infinity, it returns positive infinity. If value
 * is negative, negative infinity or NaN, it returns NaN.
 */
template <FloatingPoint T>
T Log2(T value);

}  // namespace Opal

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
T Opal::Abs(T value)
{
    return value >= 0 ? value : -value;
}

template <typename T>
bool Opal::IsEqual(T a, T b, T epsilon)
{
    return Abs(a - b) <= epsilon;
}

template <typename T>
T Opal::Min(T A, T B)
{
    return A < B ? A : B;
}

template <typename T>
T Opal::Max(T A, T B)
{
    return A > B ? A : B;
}

template <typename T>
bool Opal::IsNaN(T value)
{
    return value != value;
}

template <typename T>
bool Opal::IsFinite(T value)
{
    return std::isfinite(value);
}

template <Opal::FloatingPoint T>
T Opal::Radians(T degrees)
{
    constexpr T k_half_circle_degrees = static_cast<T>(180.0);
    constexpr T k_pi = static_cast<T>(k_pi_double);
    return (k_pi / k_half_circle_degrees) * degrees;
}

template <Opal::FloatingPoint T>
T Opal::Degrees(T radians)
{
    constexpr T k_half_circle_degrees = static_cast<T>(180.0);
    constexpr T k_pi = static_cast<T>(k_pi_double);
    return (k_half_circle_degrees / k_pi) * radians;
}

template <typename T>
T Opal::Clamp(T value, T low, T high)
{
    return Min(Max(value, low), high);
}

template <typename T>
T Opal::Mod(T a, T b)
{
    if constexpr (std::is_integral_v<T>)
    {
        return a % b;
    }
    else
    {
        if (IsNaN(a) || IsNaN(b) || b == 0)
        {
            return static_cast<T>(k_nan_double);
        }
        return std::fmod(a, b);
    }
}

template <Opal::FloatingPoint T>
T Opal::Sqrt(T value)
{
    return std::sqrt(value);
}

template <Opal::FloatingPoint T>
T Opal::Lerp(T t, T p0, T p1)
{
    return (1 - t) * p0 + t * p1;
}

template <Opal::FloatingPoint T>
T Opal::Round(T value)
{
    return std::round(value);
}

template <Opal::FloatingPoint T>
T Opal::Floor(T value)
{
    return std::floor(value);
}

template <Opal::FloatingPoint T>
T Opal::Ceil(T value)
{
    return std::ceil(value);
}

template <Opal::FloatingPoint T>
T Opal::Sin(T radians)
{
    return std::sin(radians);
}

template <Opal::FloatingPoint T>
T Opal::Cos(T radians)
{
    return std::cos(radians);
}

template <Opal::FloatingPoint T>
T Opal::Tan(T radians)
{
    return std::tan(radians);
}

template <Opal::FloatingPoint T>
T Opal::Power(T base, T exponent)
{
    return std::pow(base, exponent);
}

template <Opal::FloatingPoint T>
T Opal::LogNatural(T value)
{
    return std::log(value);
}

template <Opal::FloatingPoint T>
T Opal::Log2(T value)
{
    constexpr T k_inv_log2 =
        static_cast<T>(1.4426950408889634073599246810018921374266459541529859341354494069);
    return std::log(value) * k_inv_log2;
}

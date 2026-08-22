#pragma once

#include "opal/assert.h"
#include "opal/math/fixed-point.h"
#include "opal/math/normal3.h"
#include "opal/math/point2.h"
#include "opal/math/point3.h"
#include "opal/math/point4.h"
#include "opal/math/vector2.h"
#include "opal/math/vector3.h"
#include "opal/math/vector4.h"

// The vector and point types are already generic enough to hold a FixedPoint, and most of what they offer works on one as
// it stands. What does not are the handful of operations that reach for a square root: those return a double and compute
// it with the C library, which gives up both the type and the reproducibility that are the whole point of fixed point.
// This header replaces exactly those.

namespace Opal
{

/**
 * Returns the length of the vector.
 * @param vec The vector to get the length of.
 * @return The length of the vector. The squared length has to be representable, which for a vector of n components
 * bounds each of them by the square root of the largest representable value divided by n.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Length(const Vector2<FixedPoint<T, k_frac_bits>>& vec);

/** @see Length(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Length(const Vector3<FixedPoint<T, k_frac_bits>>& vec);

/** @see Length(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Length(const Vector4<FixedPoint<T, k_frac_bits>>& vec);

/** @see Length(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Length(const Normal3<FixedPoint<T, k_frac_bits>>& n);

/**
 * Returns the normalized vector.
 * @param vec The vector to normalize. Must not be zero, and its squared length has to be representable.
 * @return The normalized vector. Its length is one to within a few resolution steps.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] Vector2<FixedPoint<T, k_frac_bits>> Normalize(const Vector2<FixedPoint<T, k_frac_bits>>& vec);

/** @see Normalize(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] Vector3<FixedPoint<T, k_frac_bits>> Normalize(const Vector3<FixedPoint<T, k_frac_bits>>& vec);

/** @see Normalize(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] Vector4<FixedPoint<T, k_frac_bits>> Normalize(const Vector4<FixedPoint<T, k_frac_bits>>& vec);

/** @see Normalize(const Vector2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] Normal3<FixedPoint<T, k_frac_bits>> Normalize(const Normal3<FixedPoint<T, k_frac_bits>>& n);

/**
 * Returns the distance between two points.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The distance between the two points. The squared distance has to be representable.
 */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Distance(const Point2<FixedPoint<T, k_frac_bits>>& p1,
                                                  const Point2<FixedPoint<T, k_frac_bits>>& p2);

/** @see Distance(const Point2&, const Point2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Distance(const Point3<FixedPoint<T, k_frac_bits>>& p1,
                                                  const Point3<FixedPoint<T, k_frac_bits>>& p2);

/** @see Distance(const Point2&, const Point2&) */
template <FixedPointStorage T, u32 k_frac_bits>
[[nodiscard]] FixedPoint<T, k_frac_bits> Distance(const Point4<FixedPoint<T, k_frac_bits>>& p1,
                                                  const Point4<FixedPoint<T, k_frac_bits>>& p2);

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Length(const Vector2<FixedPoint<T, k_frac_bits>>& vec)
{
    return Opal::Sqrt(Opal::LengthSquared(vec));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Length(const Vector3<FixedPoint<T, k_frac_bits>>& vec)
{
    return Opal::Sqrt(Opal::LengthSquared(vec));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Length(const Vector4<FixedPoint<T, k_frac_bits>>& vec)
{
    return Opal::Sqrt(Opal::LengthSquared(vec));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Length(const Normal3<FixedPoint<T, k_frac_bits>>& n)
{
    return Opal::Sqrt(Opal::LengthSquared(n));
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::Vector2<Opal::FixedPoint<T, k_frac_bits>> Opal::Normalize(const Vector2<FixedPoint<T, k_frac_bits>>& vec)
{
    const FixedPoint<T, k_frac_bits> length = Opal::Length(vec);
    OPAL_ASSERT(length.raw > 0, "Can't normalize zero vector");
    return {vec.x / length, vec.y / length};
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::Vector3<Opal::FixedPoint<T, k_frac_bits>> Opal::Normalize(const Vector3<FixedPoint<T, k_frac_bits>>& vec)
{
    const FixedPoint<T, k_frac_bits> length = Opal::Length(vec);
    OPAL_ASSERT(length.raw > 0, "Can't normalize zero vector");
    return {vec.x / length, vec.y / length, vec.z / length};
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::Vector4<Opal::FixedPoint<T, k_frac_bits>> Opal::Normalize(const Vector4<FixedPoint<T, k_frac_bits>>& vec)
{
    const FixedPoint<T, k_frac_bits> length = Opal::Length(vec);
    OPAL_ASSERT(length.raw > 0, "Can't normalize zero vector");
    return {vec.x / length, vec.y / length, vec.z / length, vec.w / length};
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::Normal3<Opal::FixedPoint<T, k_frac_bits>> Opal::Normalize(const Normal3<FixedPoint<T, k_frac_bits>>& n)
{
    const FixedPoint<T, k_frac_bits> length = Opal::Length(n);
    OPAL_ASSERT(length.raw > 0, "Can't normalize zero normal");
    return {n.x / length, n.y / length, n.z / length};
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Distance(const Point2<FixedPoint<T, k_frac_bits>>& p1, const Point2<FixedPoint<T, k_frac_bits>>& p2)
{
    return Opal::Length(p1 - p2);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Distance(const Point3<FixedPoint<T, k_frac_bits>>& p1, const Point3<FixedPoint<T, k_frac_bits>>& p2)
{
    return Opal::Length(p1 - p2);
}

template <Opal::FixedPointStorage T, Opal::u32 k_frac_bits>
Opal::FixedPoint<T, k_frac_bits> Opal::Distance(const Point4<FixedPoint<T, k_frac_bits>>& p1, const Point4<FixedPoint<T, k_frac_bits>>& p2)
{
    return Opal::Length(p1 - p2);
}

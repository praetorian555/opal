#pragma once

#include "opal/math-base.h"
#include "opal/math/point2.h"
#include "opal/math/vector2.h"

namespace Opal
{

/**
 * Axis aligned bounding box in 2D.
 */
template <typename T>
struct Bounds2
{
    Point2<T> min;
    Point2<T> max;

    /**
     * Default constructor. No initialization is done.
     */
    constexpr Bounds2();

    /**
     * Construct consisting of a single point.
     * @param p The point to initialize the bounds with.
     */
    constexpr explicit Bounds2(const Point2<T>& p);

    /**
     * Constructs a bounding box by specifying any two points. The points do not need to be in any
     * particular order.
     * @param p1 First point.
     * @param p2 Second point.
     */
    Bounds2(const Point2<T>& p1, const Point2<T>& p2);

    /**
     * Access min or max by index.
     * @param index 0 for min, 1 for max.
     * @return The min or max point.
     */
    Point2<T>& operator[](int32_t index);
    const Point2<T>& operator[](int32_t index) const;

    /** Operators. */
    bool operator==(const Bounds2& other) const;
    bool operator!=(const Bounds2& other) const;
};

/**
 * Returns the corner of the bounding box specified by the mask.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param mask Bit mask specifying the corner. The least significant bit is the x coordinate, the
 * second least significant bit is the y coordinate. A value of 0 means the minimum corner, a value
 * of 1 means the maximum corner.
 * @return The corner of the bounding box specified by the mask.
 */
template <typename T>
[[nodiscard]] Point2<T> Corner(const Bounds2<T>& b, uint8_t mask);

/**
 * Returns the diagonal of the bounding box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @return The diagonal of the bounding box pointing from min to max point.
 */
template <typename T>
[[nodiscard]] Vector2<T> Diagonal(const Bounds2<T>& b);

/**
 * Calculates the surface area of the bounding box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @return The surface area of the bounding box.
 */
template <typename T>
[[nodiscard]] T SurfaceArea(const Bounds2<T>& b);

/**
 * Returns the index of the axis with the maximum extent.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @return The index of the axis with the maximum extent.
 */
template <typename T>
[[nodiscard]] int32_t MaximumExtent(const Bounds2<T>& b);

/**
 * Calculate linear interpolation between the min and max point of the bounding box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param t The interpolation factor. 0 returns the min point, 1 returns the max point.
 * @return The interpolated point.
 */
template <FloatingPoint T>
[[nodiscard]] Point2<T> Lerp(const Bounds2<T>& b, const Point2<T>& t);

/**
 * Calculate the offset of a point from the minimum corner of the bounding box scaled by
 * the inverse of the bounding box extent.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param p The point.
 * @return The offset of the point from the minimum corner of the bounding box scaled by the inverse
 * of the bounding box extent.
 */
template <FloatingPoint T>
[[nodiscard]] Vector2<T> Offset(const Bounds2<T>& b, const Point2<T>& p);

/**
 * Calculates a sphere that bounds the bounding box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param out_center The center of the sphere.
 * @param out_radius The radius of the sphere.
 */
template <FloatingPoint T>
void BoundingSphere(const Bounds2<T>& b, Point2<T>& out_center, T& out_radius);

/**
 * Calculates the extent of the bounding box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @return The extent of the bounding box.
 */
template <typename T>
[[nodiscard]] Vector2<T> Extent(const Bounds2<T>& b);

/**
 * Calculates the union of a bounding box and a point.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param p The point.
 * @return The union of the bounding box and the point.
 */
template <typename T>
[[nodiscard]] Bounds2<T> Union(const Bounds2<T>& b, const Point2<T>& p);

/**
 * Calculates the union of two bounding boxes.
 * @tparam T Data type of the bounding boxes.
 * @param b1 First bounding box.
 * @param b2 Second bounding box.
 * @return The union of the two bounding boxes.
 */
template <typename T>
[[nodiscard]] Bounds2<T> Union(const Bounds2<T>& b1, const Bounds2<T>& b2);

/**
 * Expands the bounding box by a constant amount in all directions.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param delta The amount to expand the bounding box by.
 * @return The expanded bounding box.
 */
template <typename T>
[[nodiscard]] Bounds2<T> Expand(const Bounds2<T>& b, T delta);

/**
 * Checks if two bounding boxes overlap.
 * @tparam T Data type of the bounding boxes.
 * @param b1 First bounding box.
 * @param b2 Second bounding box.
 * @return True if the bounding boxes overlap, false otherwise.
 */
template <typename T>
bool Overlaps(const Bounds2<T>& b1, const Bounds2<T>& b2);

/**
 * Calculates the intersection of two bounding boxes.
 * @tparam T Data type of the bounding boxes.
 * @param b1 First bounding box.
 * @param b2 Second bounding box.
 * @return The intersection of the two bounding boxes. If the bounding boxes do not overlap, the
 * result is undefined. Use Overlaps() to check if the bounding boxes overlap.
 */
template <typename T>
[[nodiscard]] Bounds2<T> Intersect(const Bounds2<T>& b1, const Bounds2<T>& b2);

/**
 * Checks if a point is inside a bounding box. Note that the point is not counted if it is on the
 * upper boundary of the box.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param p The point.
 * @return True if the point is inside the bounding box, false otherwise.
 */
template <typename T>
bool Inside(const Bounds2<T>& b, const Point2<T>& p);

/**
 * Checks if a point is inside a bounding box. Note that the point is counted if it is on the
 * upper boundary of the box as well.
 * @tparam T Data type of the bounding box.
 * @param b The bounding box.
 * @param p The point.
 * @return True if the point is inside the bounding box, false otherwise.
 */
template <typename T>
bool InsideInclusive(const Bounds2<T>& b, const Point2<T>& p);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr Opal::Bounds2<T>::Bounds2()
{
    // Do nothing.
}

template <typename T>
constexpr Opal::Bounds2<T>::Bounds2(const Point2<T>& p) : min(p), max(p)
{
}

template <typename T>
Opal::Bounds2<T>::Bounds2(const Point2<T>& p1, const Point2<T>& p2)
{
    min = Point2<T>(Opal::Min(p1.x, p2.x), Opal::Min(p1.y, p2.y));
    max = Point2<T>(Opal::Max(p1.x, p2.x), Opal::Max(p1.y, p2.y));
}

template <typename T>
Opal::Point2<T>& Opal::Bounds2<T>::operator[](int32_t index)
{
    return (&min)[index];
}

template <typename T>
const Opal::Point2<T>& Opal::Bounds2<T>::operator[](int32_t index) const
{
    return (&min)[index];
}

template <typename T>
bool Opal::Bounds2<T>::operator==(const Bounds2& other) const
{
    return min == other.min && max == other.max;
}

template <typename T>
bool Opal::Bounds2<T>::operator!=(const Bounds2& other) const
{
    return min != other.min || max != other.max;
}

template <typename T>
Opal::Point2<T> Opal::Corner(const Bounds2<T>& b, uint8_t mask)
{
    return Point2<T>((mask & 1) ? b.max.x : b.min.x, (mask & 2) ? b.max.y : b.min.y);
}

template <typename T>
Opal::Vector2<T> Opal::Diagonal(const Bounds2<T>& b)
{
    return b.max - b.min;
}

template <typename T>
T Opal::SurfaceArea(const Bounds2<T>& b)
{
    const Vector2<T> diag = Diagonal(b);
    return diag.x * diag.y;
}

template <typename T>
int32_t Opal::MaximumExtent(const Bounds2<T>& b)
{
    const Vector2<T> diag = Diagonal(b);
    return (diag.x > diag.y) ? 0 : 1;
}

template <Opal::FloatingPoint T>
Opal::Point2<T> Opal::Lerp(const Bounds2<T>& b, const Point2<T>& t)
{
    return Point2<T>(Opal::Lerp(t.x, b.min.x, b.max.x), Opal::Lerp(t.y, b.min.y, b.max.y));
}

template <Opal::FloatingPoint T>
Opal::Vector2<T> Opal::Offset(const Bounds2<T>& b, const Point2<T>& p)
{
    Vector2<T> o = p - b.min;
    if (b.max.x > b.min.x)
    {
        o.x /= b.max.x - b.min.x;
    }
    if (b.max.y > b.min.y)
    {
        o.y /= b.max.y - b.min.y;
    }
    return o;
}

template <Opal::FloatingPoint T>
void Opal::BoundingSphere(const Bounds2<T>& b, Point2<T>& out_center, T& out_radius)
{
    out_center = b.min + (b.max - b.min) / static_cast<T>(2);
    out_radius = Inside(b, out_center) ? static_cast<T>(Distance(out_center, b.max)) : 0;
}

template <typename T>
Opal::Vector2<T> Opal::Extent(const Bounds2<T>& b)
{
    return Opal::Abs(b.max - b.min);
}

template <typename T>
Opal::Bounds2<T> Opal::Union(const Bounds2<T>& b, const Point2<T>& p)
{
    return Bounds2<T>(Point2<T>(Opal::Min(b.min.x, p.x), Opal::Min(b.min.y, p.y)),
                      Point2<T>(Opal::Max(b.max.x, p.x), Opal::Max(b.max.y, p.y)));
}

template <typename T>
Opal::Bounds2<T> Opal::Union(const Bounds2<T>& b1, const Bounds2<T>& b2)
{
    return Bounds2<T>(Point2<T>(Opal::Min(b1.min.x, b2.min.x), Opal::Min(b1.min.y, b2.min.y)),
                      Point2<T>(Opal::Max(b1.max.x, b2.max.x), Opal::Max(b1.max.y, b2.max.y)));
}

template <typename T>
Opal::Bounds2<T> Opal::Expand(const Bounds2<T>& b, T delta)
{
    return Bounds2<T>(b.min - Vector2<T>(delta, delta), b.max + Vector2<T>(delta, delta));
}

template <typename T>
bool Opal::Overlaps(const Bounds2<T>& b1, const Bounds2<T>& b2)
{
    bool x = (b1.max.x >= b2.min.x) && (b1.min.x <= b2.max.x);
    bool y = (b1.max.y >= b2.min.y) && (b1.min.y <= b2.max.y);
    return x && y;
}

template <typename T>
Opal::Bounds2<T> Opal::Intersect(const Bounds2<T>& b1, const Bounds2<T>& b2)
{
    return Bounds2<T>(Point2<T>(Opal::Max(b1.min.x, b2.min.x), Opal::Max(b1.min.y, b2.min.y)),
                      Point2<T>(Opal::Min(b1.max.x, b2.max.x), Opal::Min(b1.max.y, b2.max.y)));
}

template <typename T>
bool Opal::Inside(const Bounds2<T>& b, const Point2<T>& p)
{
    return (p.x >= b.min.x && p.x < b.max.x && p.y >= b.min.y && p.y < b.max.y);
}

template <typename T>
bool Opal::InsideInclusive(const Bounds2<T>& b, const Point2<T>& p)
{
    return (p.x >= b.min.x && p.x <= b.max.x && p.y >= b.min.y && p.y <= b.max.y);
}

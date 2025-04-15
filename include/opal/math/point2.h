#pragma once

#include "opal/math/vector2.h"

namespace Opal
{

template <typename T>
struct Point2
{
    union
    {
        struct
        {
            T x, y;
        };
        struct
        {
            T r, g;
        };
        struct
        {
            T s, t;
        };
        T data[2];
    };

    /**
     * Default constructor. No initialization is performed.
     */
    constexpr Point2();

    /**
     * Constructs a point where all components are initialized to the same value.
     * @param value The value to initialize the components to.
     */
    constexpr explicit Point2(T value);

    /**
     * Constructs a point from the given components.
     * @param xx The x component.
     * @param yy The y component.
     */
    constexpr Point2(T xx, T yy);

    /**
     * Returns a point with all components set to zero.
     * @return A point with all components set to zero.
     */
    static Point2 Zero();

    /** Operators **/
    T& operator[](int index);
    const T& operator[](int index) const;

    bool operator==(const Point2& other) const;
    bool operator!=(const Point2& other) const;

    Point2 operator+(const Vector2<T>& vec) const;
    Point2& operator+=(const Vector2<T>& vec);

    Point2 operator-(const Vector2<T>& vec) const;
    Vector2<T> operator-(const Point2& vec) const;
    Point2& operator-=(const Vector2<T>& vec);

    Point2 operator-() const;

    template <typename U>
    Point2 operator*(U scalar) const;
    template <typename U>
    Point2& operator*=(U scalar);
    template <typename U>
    Point2 operator/(U scalar) const;
    template <typename U>
    Point2& operator/=(U scalar);
};

template <typename T, typename U>
Point2<T> operator*(U scalar, const Point2<T>& p);

/**
 * Checks if any of the components are NaN or infinite value.
 * @return True if any of the components are NaN or infinite value, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNonFinite(const Point2<T>& p);

/**
 * Checks if any of the components are NaN.
 * @return True if any of the components are NaN, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNaN(const Point2<T>& p);

/**
 * Returns a new point with the absolute value of each component.
 * @tparam T The type of the point components.
 * @param p The point to get the absolute value of.
 * @return A new point with the absolute value of each component.
 */
template <typename T>
[[nodiscard]] Point2<T> Abs(const Point2<T>& p);

/**
 * Converts the point to euclidean space by dividing each component by the w component.
 * @tparam T The type of the point components.
 * @param p The point to convert.
 * @return The point in euclidean space.
 */
template <typename T>
[[nodiscard]] Point2<T> ToEuclidean(const Point2<T>& p);

/**
 * Checks if the points are equal within a given epsilon.
 * @tparam T The type of the point components.
 * @param p1 The first point.
 * @param p2 The second point.
 * @param epsilon The epsilon to use.
 * @return True if the points are equal within a given epsilon, false otherwise.
 */
template <typename T>
bool IsEqual(const Point2<T>& p1, const Point2<T>& p2, T epsilon);

/**
 * Calculates the distance between two points.
 * @tparam T The type of the point components.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The distance between the two points.
 */
template <typename T>
double Distance(const Point2<T>& p1, const Point2<T>& p2);

/**
 * Calculates the squared distance between two points.
 * @tparam T The type of the point components.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The squared distance between the two points.
 */
template <typename T>
T DistanceSquared(const Point2<T>& p1, const Point2<T>& p2);

/**
 * Calculates the linear interpolation between two points.
 * @tparam T The type of the point components.
 * @param t The interpolation factor.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The interpolated point.
 */
template <typename T>
Point2<T> Lerp(T t, const Point2<T>& p1, const Point2<T>& p2);

/**
 * Returns the new point with the minimum components of the two points.
 * @tparam T The type of the point components.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The new point with the minimum components of the two points.
 */
template <typename T>
Point2<T> Min(const Point2<T>& p1, const Point2<T>& p2);

/**
 * Returns the new point with the maximum components of the two points.
 * @tparam T The type of the point components.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The new point with the maximum components of the two points.
 */
template <typename T>
Point2<T> Max(const Point2<T>& p1, const Point2<T>& p2);

/**
 * Return the new point with permuted components.
 * @tparam T The type of the point components.
 * @param p The point to permute.
 * @param x The index of the new x component.
 * @param y The index of the new y component.
 * @return The new point with permuted components.
 */
template <typename T>
Point2<T> Permute(const Point2<T>& p, int x, int y);

/**
 * Returns the new point with each component floored.
 * @tparam T The type of the point components.
 * @param p The point to floor.
 * @return The new point with each component floored.
 */
template <typename T>
Point2<T> Floor(const Point2<T>& p);

/**
 * Returns the new point with each component ceilinged.
 * @tparam T The type of the point components.
 * @param p The point to ceiling.
 * @return The new point with each component ceilinged.
 */
template <typename T>
Point2<T> Ceil(const Point2<T>& p);

/**
 * Returns the new point with each component rounded.
 * @tparam T The type of the point components.
 * @param p The point to round.
 * @return The new point with each component rounded.
 */
template <typename T>
Point2<T> Round(const Point2<T>& p);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr Opal::Point2<T>::Point2()
{
    // No initialization
}

template <typename T>
constexpr Opal::Point2<T>::Point2(T value) : x(value), y(value)
{
}

template <typename T>
constexpr Opal::Point2<T>::Point2(T xx, T yy) : x(xx), y(yy)
{
}

template <typename T>
Opal::Point2<T> Opal::Point2<T>::Zero()
{
    return {0, 0};
}

template <typename T>
T& Opal::Point2<T>::operator[](int index)
{
    return data[index];
}

template <typename T>
const T& Opal::Point2<T>::operator[](int index) const
{
    return data[index];
}

template <typename T>
bool Opal::Point2<T>::operator==(const Point2& other) const
{
    return x == other.x && y == other.y;
}

template <typename T>
bool Opal::Point2<T>::operator!=(const Point2& other) const
{
    return !(*this == other);
}

template <typename T>
Opal::Point2<T> Opal::Point2<T>::operator+(const Vector2<T>& vec) const
{
    return {x + vec.x, y + vec.y};
}

template <typename T>
Opal::Point2<T>& Opal::Point2<T>::operator+=(const Vector2<T>& vec)
{
    x += vec.x;
    y += vec.y;
    return *this;
}

template <typename T>
Opal::Point2<T> Opal::Point2<T>::operator-(const Vector2<T>& vec) const
{
    return {x - vec.x, y - vec.y};
}

template <typename T>
Opal::Vector2<T> Opal::Point2<T>::operator-(const Point2& vec) const
{
    return {x - vec.x, y - vec.y};
}

template <typename T>
Opal::Point2<T>& Opal::Point2<T>::operator-=(const Vector2<T>& vec)
{
    x -= vec.x;
    y -= vec.y;
    return *this;
}

template <typename T>
Opal::Point2<T> Opal::Point2<T>::operator-() const
{
    return {-x, -y};
}

template <typename T, typename U>
Opal::Point2<T> Opal::operator*(U scalar, const Point2<T>& p)
{
    return p * scalar;
}

template <typename T>
template <typename U>
Opal::Point2<T> Opal::Point2<T>::operator*(U scalar) const
{
    T sc = static_cast<T>(scalar);
    return {x * sc, y * sc};
}

template <typename T>
template <typename U>
Opal::Point2<T>& Opal::Point2<T>::operator*=(U scalar)
{
    T sc = static_cast<T>(scalar);
    x *= sc;
    y *= sc;
    return *this;
}

template <typename T>
template <typename U>
Opal::Point2<T> Opal::Point2<T>::operator/(U scalar) const
{
    if constexpr (std::is_integral_v<T>)
    {
        T sc = static_cast<T>(scalar);
        return {x / sc, y / sc};
    }
    else
    {
        T sc = static_cast<T>(1 / scalar);
        return {x * sc, y * sc};
    }
}

template <typename T>
template <typename U>
Opal::Point2<T>& Opal::Point2<T>::operator/=(U scalar)
{
    if constexpr (std::is_integral_v<T>)
    {
        T sc = static_cast<T>(scalar);
        x /= sc;
        y /= sc;
        return *this;
    }
    else
    {
        T sc = static_cast<T>(1 / scalar);
        x *= sc;
        y *= sc;
        return *this;
    }
}

template <typename T>
bool Opal::ContainsNonFinite(const Point2<T>& p)
{
    return !Opal::IsFinite(p.x) || !Opal::IsFinite(p.y);
}

template <typename T>
bool Opal::ContainsNaN(const Point2<T>& p)
{
    return Opal::IsNaN(p.x) || Opal::IsNaN(p.y);
}

template <typename T>
Opal::Point2<T> Opal::Abs(const Point2<T>& p)
{
    return {Opal::Abs(p.x), Opal::Abs(p.y)};
}

template <typename T>
bool Opal::IsEqual(const Point2<T>& p1, const Point2<T>& p2, T epsilon)
{
    return Opal::Abs(p1.x - p2.x) <= epsilon && Opal::Abs(p1.y - p2.y) <= epsilon;
}

template <typename T>
double Opal::Distance(const Point2<T>& p1, const Point2<T>& p2)
{
    return Length(p1 - p2);
}

template <typename T>
T Opal::DistanceSquared(const Point2<T>& p1, const Point2<T>& p2)
{
    return LengthSquared(p1 - p2);
}

template <typename T>
Opal::Point2<T> Opal::Lerp(T t, const Point2<T>& p1, const Point2<T>& p2)
{
    return p1 + t * (p2 - p1);
}

template <typename T>
Opal::Point2<T> Opal::Min(const Point2<T>& p1, const Point2<T>& p2)
{
    return {Opal::Min(p1.x, p2.x), Opal::Min(p1.y, p2.y)};
}

template <typename T>
Opal::Point2<T> Opal::Max(const Point2<T>& p1, const Point2<T>& p2)
{
    return {Opal::Max(p1.x, p2.x), Opal::Max(p1.y, p2.y)};
}

template <typename T>
Opal::Point2<T> Opal::Permute(const Point2<T>& p, int x, int y)
{
    return {p[x], p[y]};
}

template <typename T>
Opal::Point2<T> Opal::Floor(const Point2<T>& p)
{
    if constexpr (std::is_integral_v<T>)
    {
        return p;
    }
    if constexpr (std::is_floating_point_v<T>)
    {
        return {Opal::Floor(p.x), Opal::Floor(p.y)};
    }
}

template <typename T>
Opal::Point2<T> Opal::Ceil(const Point2<T>& p)
{
    if constexpr (std::is_integral_v<T>)
    {
        return p;
    }
    if constexpr (std::is_floating_point_v<T>)
    {
        return {Opal::Ceil(p.x), Opal::Ceil(p.y)};
    }
}

template <typename T>
Opal::Point2<T> Opal::Round(const Point2<T>& p)
{
    if constexpr (std::is_integral_v<T>)
    {
        return p;
    }
    if constexpr (std::is_floating_point_v<T>)
    {
        return {Opal::Round(p.x), Opal::Round(p.y)};
    }
}

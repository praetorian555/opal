#pragma once

#include "opal/math-base.h"
#include "opal/assert.h"

namespace Opal
{

template <typename T>
struct Vector2
{
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_WARNING("-Wpedantic")
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
    OPAL_END_DISABLE_WARNINGS

    /**
     * Default constructor. No initialization is performed.
     */
    constexpr Vector2();

    /**
     * Constructs a vector where all components are initialized to the same value.
     * @param value The value to initialize the components to.
     */
    constexpr explicit Vector2(T value);

    /**
     * Constructs a vector from the given components.
     * @param xx The x component.
     * @param yy The y component.
     */
    constexpr Vector2(T xx, T yy);

    /**
     * Returns a vector with all components set to zero.
     * @return A vector with all components set to zero.
     */
    static Vector2 Zero();

    /** Operators **/
    T& operator[](int index);
    const T& operator[](int index) const;

    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;

    Vector2 operator+(const Vector2& other) const;
    Vector2& operator+=(const Vector2& other);
    Vector2 operator-(const Vector2& other) const;
    Vector2& operator-=(const Vector2& other);

    Vector2 operator*(const Vector2& other) const;
    Vector2& operator*=(const Vector2& other);

    Vector2 operator-() const;

    template <typename U>
    Vector2 operator*(U scalar) const;
    template <typename U>
    Vector2& operator*=(U scalar);
    template <typename U>
    Vector2 operator/(U scalar) const;
    template <typename U>
    Vector2& operator/=(U scalar);
};

template <typename T, typename U>
Vector2<T> operator*(U scalar, const Vector2<T>& vec);

/**
 * Checks if any of the components are NaN or infinite value.
 * @return True if any of the components are NaN or infinite value, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNonFinite(const Vector2<T>& vec);

/**
 * Checks if any of the components are NaN.
 * @return True if any of the components are NaN, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNaN(const Vector2<T>& vec);

/**
 * Returns a new vector with the absolute value of each component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the absolute value of.
 * @return A new vector with the absolute value of each component.
 */
template <typename T>
[[nodiscard]] Vector2<T> Abs(const Vector2<T>& vec);

/**
 * Returns the dot product of vector with himself.
 * @tparam T The type of the vector components.
 * @param vec  The vector to get the dot product of.
 * @return The dot product of vector with himself.
 */
template <typename T>
[[nodiscard]] T LengthSquared(const Vector2<T>& vec);

/**
 * Returns the length of the vector.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the length of.
 * @return The length of the vector.
 */
template <typename T>
[[nodiscard]] double Length(const Vector2<T>& vec);

/**
 * Checks if the vectors are equal within a given epsilon.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @param epsilon The epsilon to use.
 * @return True if the vectors are equal within a given epsilon, false otherwise.
 */
template <typename T>
bool IsEqual(const Vector2<T>& vec1, const Vector2<T>& vec2, T epsilon);

/**
 * Returns the linear interpolation between two vectors.
 * @tparam T The type of the vector components.
 * @param t The interpolation factor.
 * @param vec1 Start vector.
 * @param vec2 End vector.
 * @return The linear interpolation between two vectors.
 */
template <typename T>
Vector2<T> Lerp(T t, const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Returns the dot product of two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The dot product of two vectors.
 */
template <typename T>
T Dot(const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Returns the absolute dot product of two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The absolute dot product of two vectors.
 */
template <typename T>
T AbsDot(const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Returns the cross product of two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The cross product of two vectors.
 */
template <typename T>
T Cross(const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Returns the normalized vector.
 * @tparam T The type of the vector components.
 * @param vec The vector to normalize.
 * @return The normalized vector.
 */
template <typename T>
Vector2<T> Normalize(const Vector2<T>& vec);

/**
 * Returns the new vector with the minimum components of the two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The new vector with the minimum components of the two vectors.
 */
template <typename T>
Vector2<T> Min(const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Returns the new vector with the maximum components of the two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The new vector with the maximum components of the two vectors.
 */
template <typename T>
Vector2<T> Max(const Vector2<T>& vec1, const Vector2<T>& vec2);

/**
 * Return the new vector with permuted components.
 * @tparam T The type of the vector components.
 * @param vec The vector to permute.
 * @param x The index of the new x component.
 * @param y The index of the new y component.
 * @return The new vector with permuted components.
 */
template <typename T>
Vector2<T> Permute(const Vector2<T>& vec, int x, int y);

/**
 * Returns the new vector with all the components clamped.
 * @tparam T The type of the vector components.
 * @param vec The vector to clamp.
 * @param low Bottom clamp value.
 * @param high Top clamp value.
 * @return The new vector with all the components clamped.
 */
template <typename T>
Vector2<T> Clamp(const Vector2<T>& vec, T low, T high);

/**
 * Reflect the incidence vector around the normal.
 * @param incidence The incidence vector.
 * @param normal The normal vector.
 * @return The reflected vector.
 */
template <typename T>
Vector2<T> Reflect(const Vector2<T>& incidence, const Vector2<T>& normal);

/**
 * Returns the value of the smallest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the smallest component of.
 * @return The value of the smallest component.
 */
template <typename T>
T MinComponent(const Vector2<T>& vec);

/**
 * Returns the value of the largest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the largest component of.
 * @return The value of the largest component.
 */
template <typename T>
T MaxComponent(const Vector2<T>& vec);

/**
 * Returns the index of the smallest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the index of the smallest component of.
 * @return The index of the smallest component.
 */
template <typename T>
int MinDimension(const Vector2<T>& vec);

/**
 * Returns the index of the largest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the index of the largest component of.
 * @return The index of the largest component.
 */
template <typename T>
int MaxDimension(const Vector2<T>& vec);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr Opal::Vector2<T>::Vector2()
{
    // No initialization
}

template <typename T>
constexpr Opal::Vector2<T>::Vector2(T value) : x(value), y(value)
{
}

template <typename T>
constexpr Opal::Vector2<T>::Vector2(T xx, T yy) : x(xx), y(yy)
{
}

template <typename T>
Opal::Vector2<T> Opal::Vector2<T>::Zero()
{
    return {0, 0};
}

template <typename T>
T& Opal::Vector2<T>::operator[](int index)
{
    return data[index];
}

template <typename T>
const T& Opal::Vector2<T>::operator[](int index) const
{
    return data[index];
}

template <typename T>
bool Opal::Vector2<T>::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}

template <typename T>
bool Opal::Vector2<T>::operator!=(const Vector2& other) const
{
    return !(*this == other);
}

template <typename T>
Opal::Vector2<T> Opal::Vector2<T>::operator+(const Vector2& other) const
{
    return {x + other.x, y + other.y};
}

template <typename T>
Opal::Vector2<T>& Opal::Vector2<T>::operator+=(const Vector2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

template <typename T>
Opal::Vector2<T> Opal::Vector2<T>::operator-(const Vector2& other) const
{
    return {x - other.x, y - other.y};
}

template <typename T>
Opal::Vector2<T>& Opal::Vector2<T>::operator-=(const Vector2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

template <typename T>
Opal::Vector2<T> Opal::Vector2<T>::operator*(const Vector2& other) const
{
    return {x * other.x, y * other.y};
}

template <typename T, typename U>
Opal::Vector2<T> Opal::operator*(U scalar, const Vector2<T>& vec)
{
    return vec * scalar;
}

template <typename T>
Opal::Vector2<T>& Opal::Vector2<T>::operator*=(const Vector2& other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

template <typename T>
Opal::Vector2<T> Opal::Vector2<T>::operator-() const
{
    return {-x, -y};
}

template <typename T>
template <typename U>
Opal::Vector2<T> Opal::Vector2<T>::operator*(U scalar) const
{
    T sc = static_cast<T>(scalar);
    return {x * sc, y * sc};
}

template <typename T>
template <typename U>
Opal::Vector2<T>& Opal::Vector2<T>::operator*=(U scalar)
{
    T sc = static_cast<T>(scalar);
    x *= sc;
    y *= sc;
    return *this;
}

template <typename T>
template <typename U>
Opal::Vector2<T> Opal::Vector2<T>::operator/(U scalar) const
{
    if constexpr (std::is_integral_v<T>)
    {
        T sc = static_cast<T>(scalar);
        return {x / sc, y / sc};
    }
    else
    {
        T sc = static_cast<T>(1) / static_cast<T>(scalar);
        return {x * sc, y * sc};
    }
}

template <typename T>
template <typename U>
Opal::Vector2<T>& Opal::Vector2<T>::operator/=(U scalar)
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
        T sc = static_cast<T>(1) / static_cast<T>(scalar);
        x *= sc;
        y *= sc;
        return *this;
    }
}

template <typename T>
bool Opal::ContainsNonFinite(const Vector2<T>& vec)
{
    return !Opal::IsFinite(vec.x) || !Opal::IsFinite(vec.y);
}

template <typename T>
bool Opal::ContainsNaN(const Vector2<T>& vec)
{
    return Opal::IsNaN(vec.x) || Opal::IsNaN(vec.y);
}

template <typename T>
Opal::Vector2<T> Opal::Abs(const Vector2<T>& vec)
{
    return {Opal::Abs(vec.x), Opal::Abs(vec.y)};
}

template <typename T>
T Opal::LengthSquared(const Vector2<T>& vec)
{
    return vec.x * vec.x + vec.y * vec.y;
}

template <typename T>
double Opal::Length(const Vector2<T>& vec)
{
    return Opal::Sqrt(static_cast<double>(LengthSquared(vec)));
}

template <typename T>
bool Opal::IsEqual(const Vector2<T>& vec1, const Vector2<T>& vec2, T epsilon)
{
    return Opal::Abs(vec1.x - vec2.x) <= epsilon && Opal::Abs(vec1.y - vec2.y) <= epsilon;
}

template <typename T>
Opal::Vector2<T> Opal::Lerp(T t, const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return (1 - t) * vec1 + t * vec2;
}

template <typename T>
T Opal::Dot(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

template <typename T>
T Opal::AbsDot(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return Opal::Abs(Dot(vec1, vec2));
}

template <typename T>
T Opal::Cross(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return vec1.x * vec2.y - vec1.y * vec2.x;
}

template <typename T>
Opal::Vector2<T> Opal::Normalize(const Vector2<T>& vec)
{
    double length = Length(vec);
    OPAL_ASSERT(length > 0, "Can't normalize zero vector");
    return vec / length;
}

template <typename T>
Opal::Vector2<T> Opal::Min(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return {Opal::Min(vec1.x, vec2.x), Opal::Min(vec1.y, vec2.y)};
}

template <typename T>
Opal::Vector2<T> Opal::Max(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return {Opal::Max(vec1.x, vec2.x), Opal::Max(vec1.y, vec2.y)};
}

template <typename T>
Opal::Vector2<T> Opal::Permute(const Vector2<T>& vec, int x, int y)
{
    return {vec[x], vec[y]};
}

template <typename T>
Opal::Vector2<T> Opal::Clamp(const Vector2<T>& vec, T low, T high)
{
    return {Opal::Clamp(vec.x, low, high), Opal::Clamp(vec.y, low, high)};
}

template <typename T>
T Opal::MinComponent(const Vector2<T>& vec)
{
    return Opal::Min(vec.x, vec.y);
}

template <typename T>
T Opal::MaxComponent(const Vector2<T>& vec)
{
    return Opal::Max(vec.x, vec.y);
}

template <typename T>
int Opal::MinDimension(const Vector2<T>& vec)
{
    return vec.x < vec.y ? 0 : 1;
}

template <typename T>
int Opal::MaxDimension(const Vector2<T>& vec)
{
    return vec.x > vec.y ? 0 : 1;
}

template <typename T>
Opal::Vector2<T> Opal::Reflect(const Vector2<T>& incidence, const Vector2<T>& normal)
{
    assert(Dot(incidence, normal) >= 0);
    return 2 * Dot(incidence, normal) * normal - incidence;
}

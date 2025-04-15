#pragma once

#include "opal/math-base.h"
#include "opal/assert.h"

namespace Opal
{

template <typename T>
struct Vector4
{
    union
    {
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
        struct
        {
            T s, t, p, q;
        };
        T data[4];
    };

    /**
     * Default constructor. No initialization is performed.
     */
    constexpr Vector4();

    /**
     * Constructs a vector where all components are initialized to the same value.
     * @param value The value to initialize the components to.
     */
    constexpr explicit Vector4(T value);

    /**
     * Constructs a vector from the given components.
     * @param xx The x component.
     * @param yy The y component.
     * @param zz The z component.
     * @param ww The w component.
     */
    constexpr Vector4(T xx, T yy, T zz, T ww);

    /**
     * Returns vector with all components set to zero.
     * @return Vector with all components set to zero.
     */
    static Vector4 Zero();

    /** Operators **/
    T& operator[](int index);
    const T& operator[](int index) const;

    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;

    Vector4 operator+(const Vector4& other) const;
    Vector4& operator+=(const Vector4& other);
    Vector4 operator-(const Vector4& other) const;
    Vector4& operator-=(const Vector4& other);

    Vector4 operator*(const Vector4& other) const;
    Vector4& operator*=(const Vector4& other);

    Vector4 operator-() const;

    template <typename U>
    Vector4 operator*(U scalar) const;
    template <typename U>
    Vector4& operator*=(U scalar);
    template <typename U>
    Vector4 operator/(U scalar) const;
    template <typename U>
    Vector4& operator/=(U scalar);
};

template <typename T, typename U>
Vector4<T> operator*(U scalar, const Vector4<T>& vec);

/**
 * Checks if any of the components are NaN or infinite value.
 * @return True if any of the components are NaN or infinite value, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNonFinite(const Vector4<T>& vec);

/**
 * Checks if any of the components are NaN.
 * @return True if any of the components are NaN, false otherwise.
 */
template <typename T>
[[nodiscard]] bool ContainsNaN(const Vector4<T>& vec);

/**
 * Returns a new vector with the absolute value of each component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the absolute value of.
 * @return A new vector with the absolute value of each component.
 */
template <typename T>
[[nodiscard]] Vector4<T> Abs(const Vector4<T>& vec);

/**
 * Returns the dot product of vector with himself.
 * @tparam T The type of the vector components.
 * @param vec  The vector to get the dot product of.
 * @return The dot product of vector with himself.
 */
template <typename T>
[[nodiscard]] T LengthSquared(const Vector4<T>& vec);

/**
 * Returns the length of the vector.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the length of.
 * @return The length of the vector.
 */
template <typename T>
[[nodiscard]] double Length(const Vector4<T>& vec);

/**
 * Checks if the vectors are equal within a given epsilon.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @param epsilon The epsilon to use.
 * @return True if the vectors are equal within a given epsilon, false otherwise.
 */
template <typename T>
bool IsEqual(const Vector4<T>& vec1, const Vector4<T>& vec2, T epsilon);

/**
 * Returns the linear interpolation of two vectors.
 * @tparam T The type of the vector components.
 * @param t The interpolation factor.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The linear interpolation of two vectors.
 */
template <typename T>
Vector4<T> Lerp(T t, const Vector4<T>& vec1, const Vector4<T>& vec2);

/**
 * Returns the dot product of two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The dot product of two vectors.
 */
template <typename T>
T Dot(const Vector4<T>& vec1, const Vector4<T>& vec2);

/**
 * Returns the absolute dot product of two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The absolute dot product of two vectors.
 */
template <typename T>
T AbsDot(const Vector4<T>& vec1, const Vector4<T>& vec2);

/**
 * Returns the normalized vector.
 * @tparam T The type of the vector components.
 * @param vec The vector to normalize.
 * @return The normalized vector.
 */
template <typename T>
Vector4<T> Normalize(const Vector4<T>& vec);

/**
 * Returns the new vector with the minimum components of the two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The new vector with the minimum components of the two vectors.
 */
template <typename T>
Vector4<T> Min(const Vector4<T>& vec1, const Vector4<T>& vec2);

/**
 * Returns the new vector with the maximum components of the two vectors.
 * @tparam T The type of the vector components.
 * @param vec1 The first vector.
 * @param vec2 The second vector.
 * @return The new vector with the maximum components of the two vectors.
 */
template <typename T>
Vector4<T> Max(const Vector4<T>& vec1, const Vector4<T>& vec2);

/**
 * Return the new vector with permuted components.
 * @tparam T The type of the vector components.
 * @param vec The vector to permute.
 * @param x The index of the new x component.
 * @param y The index of the new y component.
 * @param z The index of the new z component.
 * @param w The index of the new w component.
 * @return The new vector with permuted components.
 */
template <typename T>
Vector4<T> Permute(const Vector4<T>& vec, int x, int y, int z, int w);

/**
 * Returns the new vector with all the components clamped.
 * @tparam T The type of the vector components.
 * @param vec The vector to clamp.
 * @param low Bottom clamp value.
 * @param high Top clamp value.
 * @return The new vector with all the components clamped.
 */
template <typename T>
Vector4<T> Clamp(const Vector4<T>& vec, T low, T high);

/**
 * Returns the value of the smallest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the smallest component of.
 * @return The value of the smallest component.
 */
template <typename T>
T MinComponent(const Vector4<T>& vec);

/**
 * Returns the value of the largest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the largest component of.
 * @return The value of the largest component.
 */
template <typename T>
T MaxComponent(const Vector4<T>& vec);

/**
 * Returns the index of the smallest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the index of the smallest component of.
 * @return The index of the smallest component.
 */
template <typename T>
int MinDimension(const Vector4<T>& vec);

/**
 * Returns the index of the largest component.
 * @tparam T The type of the vector components.
 * @param vec The vector to get the index of the largest component of.
 * @return The index of the largest component.
 */
template <typename T>
int MaxDimension(const Vector4<T>& vec);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr Opal::Vector4<T>::Vector4()
{
    // No initialization
}

template <typename T>
constexpr Opal::Vector4<T>::Vector4(T value) : x(value), y(value), z(value), w(value)
{
}

template <typename T>
constexpr Opal::Vector4<T>::Vector4(T xx, T yy, T zz, T ww) : x(xx), y(yy), z(zz), w(ww)
{
}

template <typename T>
Opal::Vector4<T> Opal::Vector4<T>::Zero()
{
    return {0, 0, 0, 0};
}

template <typename T>
T& Opal::Vector4<T>::operator[](int index)
{
    return data[index];
}

template <typename T>
const T& Opal::Vector4<T>::operator[](int index) const
{
    return data[index];
}

template <typename T>
bool Opal::Vector4<T>::operator==(const Vector4& other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

template <typename T>
bool Opal::Vector4<T>::operator!=(const Vector4& other) const
{
    return !(*this == other);
}

template <typename T>
Opal::Vector4<T> Opal::Vector4<T>::operator+(const Vector4& other) const
{
    return {x + other.x, y + other.y, z + other.z, w + other.w};
}

template <typename T>
Opal::Vector4<T>& Opal::Vector4<T>::operator+=(const Vector4& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

template <typename T>
Opal::Vector4<T> Opal::Vector4<T>::operator-(const Vector4& other) const
{
    return {x - other.x, y - other.y, z - other.z, w - other.w};
}

template <typename T>
Opal::Vector4<T>& Opal::Vector4<T>::operator-=(const Vector4& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

template <typename T>
Opal::Vector4<T> Opal::Vector4<T>::operator*(const Vector4& other) const
{
    return {x * other.x, y * other.y, z * other.z, w * other.w};
}

template <typename T, typename U>
Opal::Vector4<T> Opal::operator*(U scalar, const Vector4<T>& vec)
{
    return vec * scalar;
}

template <typename T>
Opal::Vector4<T>& Opal::Vector4<T>::operator*=(const Vector4& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
    return *this;
}

template <typename T>
Opal::Vector4<T> Opal::Vector4<T>::operator-() const
{
    return {-x, -y, -z, -w};
}

template <typename T>
template <typename U>
Opal::Vector4<T> Opal::Vector4<T>::operator*(U scalar) const
{
    T sc = static_cast<T>(scalar);
    return {x * sc, y * sc, z * sc, w * sc};
}

template <typename T>
template <typename U>
Opal::Vector4<T>& Opal::Vector4<T>::operator*=(U scalar)
{
    T sc = static_cast<T>(scalar);
    x *= sc;
    y *= sc;
    z *= sc;
    w *= sc;
    return *this;
}

template <typename T>
template <typename U>
Opal::Vector4<T> Opal::Vector4<T>::operator/(U scalar) const
{
    if constexpr (std::is_integral_v<T>)
    {
        T sc = static_cast<T>(scalar);
        return {x / sc, y / sc, z / sc, w / sc};
    }
    else
    {
        T sc = static_cast<T>(1 / scalar);
        return {x * sc, y * sc, z * sc, w * sc};
    }
}

template <typename T>
template <typename U>
Opal::Vector4<T>& Opal::Vector4<T>::operator/=(U scalar)
{
    if constexpr (std::is_integral_v<T>)
    {
        T sc = static_cast<T>(scalar);
        x /= sc;
        y /= sc;
        z /= sc;
        w /= sc;
        return *this;
    }
    else
    {
        T sc = static_cast<T>(1 / scalar);
        x *= sc;
        y *= sc;
        z *= sc;
        w *= sc;
        return *this;
    }
}

template <typename T>
bool Opal::ContainsNonFinite(const Vector4<T>& vec)
{
    return !Opal::IsFinite(vec.x) || !Opal::IsFinite(vec.y) || !Opal::IsFinite(vec.z) ||
           !Opal::IsFinite(vec.w);
}

template <typename T>
bool Opal::ContainsNaN(const Vector4<T>& vec)
{
    return Opal::IsNaN(vec.x) || Opal::IsNaN(vec.y) || Opal::IsNaN(vec.z) || Opal::IsNaN(vec.w);
}

template <typename T>
Opal::Vector4<T> Opal::Abs(const Vector4<T>& vec)
{
    return {Opal::Abs(vec.x), Opal::Abs(vec.y), Opal::Abs(vec.z), Opal::Abs(vec.w)};
}

template <typename T>
T Opal::LengthSquared(const Vector4<T>& vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
}

template <typename T>
double Opal::Length(const Vector4<T>& vec)
{
    return Opal::Sqrt(static_cast<double>(LengthSquared(vec)));
}

template <typename T>
bool Opal::IsEqual(const Vector4<T>& vec1, const Vector4<T>& vec2, T epsilon)
{
    return Opal::Abs(vec1.x - vec2.x) <= epsilon && Opal::Abs(vec1.y - vec2.y) <= epsilon &&
           Opal::Abs(vec1.z - vec2.z) <= epsilon && Opal::Abs(vec1.w - vec2.w) <= epsilon;
}

template <typename T>
Opal::Vector4<T> Opal::Lerp(T t, const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return vec1 + (vec2 - vec1) * t;
}

template <typename T>
T Opal::Dot(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

template <typename T>
T Opal::AbsDot(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return Opal::Abs(Dot(vec1, vec2));
}

template <typename T>
Opal::Vector4<T> Opal::Normalize(const Vector4<T>& vec)
{
    double length = Length(vec);
    OPAL_ASSERT(length > 0, "Can't normalize a zero vector.");
    return vec / length;
}

template <typename T>
Opal::Vector4<T> Opal::Min(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return {Opal::Min(vec1.x, vec2.x), Opal::Min(vec1.y, vec2.y), Opal::Min(vec1.z, vec2.z),
            Opal::Min(vec1.w, vec2.w)};
}

template <typename T>
Opal::Vector4<T> Opal::Max(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return {Opal::Max(vec1.x, vec2.x), Opal::Max(vec1.y, vec2.y), Opal::Max(vec1.z, vec2.z),
            Opal::Max(vec1.w, vec2.w)};
}

template <typename T>
Opal::Vector4<T> Opal::Permute(const Vector4<T>& vec, int x, int y, int z, int w)
{
    return {vec[x], vec[y], vec[z], vec[w]};
}

template <typename T>
Opal::Vector4<T> Opal::Clamp(const Vector4<T>& vec, T low, T high)
{
    return {Opal::Clamp(vec.x, low, high), Opal::Clamp(vec.y, low, high),
            Opal::Clamp(vec.z, low, high), Opal::Clamp(vec.w, low, high)};
}

template <typename T>
T Opal::MinComponent(const Vector4<T>& vec)
{
    return Opal::Min(Opal::Min(vec.x, vec.y), Opal::Min(vec.z, vec.w));
}

template <typename T>
T Opal::MaxComponent(const Vector4<T>& vec)
{
    return Opal::Max(Opal::Max(vec.x, vec.y), Opal::Max(vec.z, vec.w));
}

template <typename T>
int Opal::MinDimension(const Vector4<T>& vec)
{
    return vec.x < vec.y ? (vec.x < vec.z ? (vec.x < vec.w ? 0 : 3) : (vec.z < vec.w ? 2 : 3))
                         : (vec.y < vec.z ? (vec.y < vec.w ? 1 : 3) : (vec.z < vec.w ? 2 : 3));
}

template <typename T>
int Opal::MaxDimension(const Vector4<T>& vec)
{
    return vec.x > vec.y ? (vec.x > vec.z ? (vec.x > vec.w ? 0 : 3) : (vec.z > vec.w ? 2 : 3))
                         : (vec.y > vec.z ? (vec.y > vec.w ? 1 : 3) : (vec.z > vec.w ? 2 : 3));
}

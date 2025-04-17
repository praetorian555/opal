#pragma once

#include "opal/math/matrix.h"
#include "opal/math/point3.h"
#include "opal/math/vector3.h"

namespace Opal
{

template <FloatingPoint T>
class Quaternion
{
public:
    Vector3<T> vec;
    T w;

    /**
     * Default constructor. No initialization is done.
     */
    Quaternion();

    /**
     * @brief Construct a Quaternion using four T values.
     * @param w - The T part of the Quaternion.
     * @param x - The i part of the Quaternion.
     * @param y - The j part of the Quaternion.
     * @param z - The k part of the Quaternion.
     */
    Quaternion(T w, T x, T y, T z);

    /**
     * @brief Construct a Quaternion from a matrix. This uses only the upper left 3x3 part of the
     * matrix.
     * @param transform - The transform to construct the Quaternion from.
     */
    explicit Quaternion(const Matrix4x4<T>& transform);

    template <Opal::FloatingPoint U>
    Quaternion(const Quaternion<U>& other);

    /**
     * @brief Construct a Quaternion from an axis and an angle.
     * @param axis - The axis to rotate around. It doesn't have to be normalized.
     * @param angle_degrees - The angle to rotate by in degrees.
     * @return The Quaternion representing the rotation.
     */
    static Quaternion FromAxisAngleDegrees(const Vector3<T>& axis, T angle_degrees);

    /**
     * @brief Construct a Quaternion from an axis and an angle.
     * @param axis - The axis to rotate around. It doesn't have to be normalized.
     * @param angle_radians - The angle to rotate by in radians.
     * @return The Quaternion representing the rotation.
     */
    static Quaternion FromAxisAngleRadians(const Vector3<T>& axis, T angle_radians);

    /**
     * @brief Construct an identity Quaternion. This is a Quaternion with all components set to zero
     * except for the real component which is set to one. It represents no rotation.
     */
    static Quaternion Identity();

    /**
     * Quaternion with all components set to zero.
     */
    static Quaternion Zero();

    /** Operator overloads. */

    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;

    Quaternion& operator+=(const Quaternion& other);
    Quaternion& operator-=(const Quaternion& other);
    Quaternion& operator*=(const Quaternion& other);
    Quaternion& operator*=(T scalar);
    Quaternion& operator/=(T scalar);

    Quaternion operator*(T scalar) const;
    Quaternion operator/(T scalar) const;
};

template <Opal::FloatingPoint T>
bool IsEqual(const Quaternion<T>& q1, const Quaternion<T>& q2, T epsilon);

template <Opal::FloatingPoint T>
Quaternion<T> operator+(const Quaternion<T>& q);

template <Opal::FloatingPoint T>
Quaternion<T> operator-(const Quaternion<T>& q);

template <Opal::FloatingPoint T>
Quaternion<T> operator+(const Quaternion<T>& q1, const Quaternion<T>& q2);
template <Opal::FloatingPoint T>
Quaternion<T> operator-(const Quaternion<T>& q1, const Quaternion<T>& q2);
template <Opal::FloatingPoint T>
Quaternion<T> operator*(const Quaternion<T>& q1, const Quaternion<T>& q2);
template <Opal::FloatingPoint T>
Quaternion<T> operator*(T scalar, const Quaternion<T>& q);

/**
 * Rotate vector by this Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q Quaternion to rotate by.
 * @param vec Vector to rotate.
 * @return Returns a new Quaternion.
 */
template <Opal::FloatingPoint T>
Vector3<T> operator*(const Quaternion<T>& q, const Vector3<T>& vec);

/**
 * Rotate point by this Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q Quaternion to rotate by.
 * @param p point to rotate.
 * @return Returns a new Quaternion.
 */
template <Opal::FloatingPoint T>
Point3<T> operator*(const Quaternion<T>& q, const Point3<T>& p);

/**
 * Checks if any of the components are NaN or infinite value.
 * @param q Quaternion to check.
 * @return True if any of the components are NaN or infinite value, false otherwise.
 */
template <Opal::FloatingPoint T>
[[nodiscard]] bool ContainsNonFinite(const Quaternion<T>& q);

/**
 * Checks if any of the components are NaN.
 * @param q Quaternion to check.
 * @return True if any of the components are NaN, false otherwise.
 */
template <Opal::FloatingPoint T>
[[nodiscard]] bool ContainsNaN(const Quaternion<T>& q);

/**
 * Calculate the length squared of a Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q Quaternion to calculate the length squared of.
 * @return Returns the length squared of the Quaternion.
 */
template <Opal::FloatingPoint T>
[[nodiscard]] T LengthSquared(const Quaternion<T>& q);

/**
 * Calculate the length of a Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q Quaternion to calculate the length of.
 * @return Returns the length of the Quaternion.
 */
template <Opal::FloatingPoint T>
[[nodiscard]] T Length(const Quaternion<T>& q);

/**
 * Calculate the dot product of two Quaternions.
 * @tparam T Type of the Quaternions.
 * @param q1 First Quaternion.
 * @param q2 Second Quaternion.
 * @return Returns the dot product of the two Quaternions.
 */
template <Opal::FloatingPoint T>
T Dot(const Quaternion<T>& q1, const Quaternion<T>& q2);

/**
 * Normalize a Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q Quaternion to normalize. Can't have a magnitude of 0.
 * @return Returns the normalized Quaternion in a new object.
 */
template <Opal::FloatingPoint T>
Quaternion<T> Normalize(const Quaternion<T>& q);

/**
 * Perform linear interpolation between two Quaternions.
 * @tparam T Type of the Quaternions.
 * @param param Parameter to interpolate between the two Quaternions. Should be between 0 and 1.
 * @param q1 First Quaternion.
 * @param q2 Second Quaternion.
 * @return Returns the interpolated Quaternion in a new object.
 * @note This operation is commutative but results in a non-constant angular velocity. If you want
 * absolute precision and constant angular velocity, use Slerp instead.
 * @see Slerp
 */
template <Opal::FloatingPoint T>
Quaternion<T> Lerp(T param, const Quaternion<T>& q1, const Quaternion<T>& q2);

/**
 * Perform spherical linear interpolation between two Quaternions.
 * @tparam T Type of the Quaternions.
 * @param q1 First Quaternion.
 * @param q2 Second Quaternion.
 * @param param Parameter to interpolate between the two Quaternions. Should be between 0 and 1.
 * @return Returns the interpolated Quaternion in a new object.
 * @note This operation is not commutative but provides constant angular velocity. If you want less
 * precision and commutative interpolation but can live with non-constant angular velocity, use Lerp
 * instead.
 * @see Lerp
 */
template <Opal::FloatingPoint T>
Quaternion<T> Slerp(T param, const Quaternion<T>& q1, const Quaternion<T>& q2);

/**
 * @brief Get the conjugate of a Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q - The Quaternion to get the conjugate of.
 * @return The conjugate of the Quaternion.
 */
template <Opal::FloatingPoint T>
Quaternion<T> Conjugate(const Quaternion<T>& q);

/**
 * @brief Get the inverse of a Quaternion.
 * @tparam T Type of the Quaternion.
 * @param q - The Quaternion to get the inverse of.
 * @return The inverse of the Quaternion.
 */
template <Opal::FloatingPoint T>
Quaternion<T> Inverse(const Quaternion<T>& q);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <Opal::FloatingPoint T>
Opal::Quaternion<T>::Quaternion()
{
    // Do nothing.
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>::Quaternion(T w, T x, T y, T z) : vec(x, y, z), w(w)
{
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>::Quaternion(const Matrix4x4<T>& transform)
{
    const T trace = transform.elements[0][0] + transform.elements[1][1] + transform.elements[2][2] +
                    transform.elements[3][3];

    if (trace > static_cast<T>(0.0))
    {
        w = Opal::Sqrt(trace) / 2;
        const T scalar = 1 / (4 * w);
        vec.x = (transform.elements[2][1] - transform.elements[1][2]) * scalar;
        vec.y = (transform.elements[0][2] - transform.elements[2][0]) * scalar;
        vec.z = (transform.elements[1][0] - transform.elements[0][1]) * scalar;
        return;
    }

    const int Next[3] = {1, 2, 0};

    // Figure out who is largest
    int i = 0;
    if (transform.elements[1][1] > transform.elements[0][0])
    {
        i = 1;
    }
    if (transform.elements[2][2] > transform.elements[i][i])
    {
        i = 2;
    }
    const int j = Next[i];
    const int k = Next[j];

    T scalar = Opal::Sqrt(
        (transform.elements[i][i] - (transform.elements[j][j] + transform.elements[k][k])) +
        static_cast<T>(1.0));
    T dir[3];
    dir[i] = scalar * static_cast<T>(0.5);
    if (scalar != 0.f)
    {
        scalar = static_cast<T>(0.5) / scalar;
    }
    w = (transform.elements[k][j] - transform.elements[j][k]) * scalar;
    dir[j] = (transform.elements[j][i] + transform.elements[i][j]) * scalar;
    dir[k] = (transform.elements[k][i] + transform.elements[i][k]) * scalar;
    vec.x = dir[0];
    vec.y = dir[1];
    vec.z = dir[2];
}

template <Opal::FloatingPoint T>
template <Opal::FloatingPoint U>
Opal::Quaternion<T>::Quaternion(const Quaternion<U>& other)
    : vec(static_cast<T>(other.vec.x), static_cast<T>(other.vec.y), static_cast<T>(other.vec.z)),
      w(static_cast<T>(other.w))
{
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::FromAxisAngleDegrees(const Vector3<T>& axis,
                                                              T angle_degrees)
{
    return FromAxisAngleRadians(axis, Radians(angle_degrees));
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::FromAxisAngleRadians(const Vector3<T>& axis,
                                                              T angle_radians)
{
    const T s = Sin(angle_radians / 2);
    const T c = Cos(angle_radians / 2);
    const Vector3 vec = Normalize(axis);
    return {c, vec.x * s, vec.y * s, vec.z * s};
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::Identity()
{
    return {1, 0, 0, 0};
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::Zero()
{
    return {0, 0, 0, 0};
}

template <Opal::FloatingPoint T>
bool Opal::Quaternion<T>::operator==(const Quaternion& other) const
{
    return vec == other.vec && w == other.w;
}

template <Opal::FloatingPoint T>
bool Opal::Quaternion<T>::operator!=(const Quaternion& other) const
{
    return !(*this == other);
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>& Opal::Quaternion<T>::operator+=(const Quaternion& other)
{
    vec += other.vec;
    w += other.w;
    return *this;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>& Opal::Quaternion<T>::operator-=(const Quaternion& other)
{
    vec -= other.vec;
    w -= other.w;
    return *this;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>& Opal::Quaternion<T>::operator*=(const Quaternion& other)
{
    Quaternion q;
    q.vec.x = w * other.vec.x + vec.x * other.w + vec.y * other.vec.z - vec.z * other.vec.y;
    q.vec.y = w * other.vec.y + vec.y * other.w + vec.z * other.vec.x - vec.x * other.vec.z;
    q.vec.z = w * other.vec.z + vec.z * other.w + vec.x * other.vec.y - vec.y * other.vec.x;
    q.w = w * other.w - Dot(vec, other.vec);
    *this = q;
    return *this;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>& Opal::Quaternion<T>::operator*=(T scalar)
{
    vec *= scalar;
    w *= scalar;
    return *this;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T>& Opal::Quaternion<T>::operator/=(T scalar)
{
    vec /= scalar;
    w /= scalar;
    return *this;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::operator*(T scalar) const
{
    Quaternion q = *this;
    q *= scalar;
    return q;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Quaternion<T>::operator/(T scalar) const
{
    Quaternion q = *this;
    q /= scalar;
    return q;
}

template <Opal::FloatingPoint T>
bool Opal::IsEqual(const Quaternion<T>& q1, const Quaternion<T>& q2, T epsilon)
{
    return IsEqual(q1.vec, q2.vec, epsilon) && IsEqual(q1.w, q2.w, epsilon);
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator+(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    Quaternion<T> q = q1;
    q += q2;
    return q;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator-(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    Quaternion<T> q = q1;
    q -= q2;
    return q;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator*(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    Quaternion<T> q = q1;
    q *= q2;
    return q;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator*(T scalar, const Quaternion<T>& q)
{
    Quaternion<T> result = q;
    result *= scalar;
    return result;
}

template <Opal::FloatingPoint T>
Opal::Vector3<T> Opal::operator*(const Quaternion<T>& q, const Vector3<T>& vec)
{
    const Quaternion<T> qp(0, vec.x, vec.y, vec.z);
    const Quaternion<T> result = q * qp * Inverse(q);
    return {result.vec.x, result.vec.y, result.vec.z};
}

template <Opal::FloatingPoint T>
Opal::Point3<T> Opal::operator*(const Quaternion<T>& q, const Point3<T>& p)
{
    const Quaternion<T> qp(0, p.x, p.y, p.z);
    const Quaternion<T> result = q * qp * Inverse(q);
    return {result.vec.x, result.vec.y, result.vec.z};
}

template <Opal::FloatingPoint T>
bool Opal::ContainsNonFinite(const Quaternion<T>& q)
{
    return !std::isfinite(q.vec.x) || !std::isfinite(q.vec.y) || !std::isfinite(q.vec.z) ||
           !std::isfinite(q.w);
}

template <Opal::FloatingPoint T>
bool Opal::ContainsNaN(const Quaternion<T>& q)
{
    return std::isnan(q.vec.x) || std::isnan(q.vec.y) || std::isnan(q.vec.z) || std::isnan(q.w);
}

template <Opal::FloatingPoint T>
T Opal::LengthSquared(const Quaternion<T>& q)
{
    return q.vec.x * q.vec.x + q.vec.y * q.vec.y + q.vec.z * q.vec.z + q.w * q.w;
}

template <Opal::FloatingPoint T>
T Opal::Length(const Quaternion<T>& q)
{
    return Opal::Sqrt(LengthSquared(q));
}

template <Opal::FloatingPoint T>
T Opal::Dot(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    return q1.vec.x * q2.vec.x + q1.vec.y * q2.vec.y + q1.vec.z * q2.vec.z + q1.w * q2.w;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Normalize(const Quaternion<T>& q)
{
    const T length = Length(q);
    if (length == 0)
    {
        return q;
    }
    return q / length;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Lerp(T param, const Quaternion<T>& q1, const Quaternion<T>& q2)
{
#if _DEBUG
    constexpr T k_epsilon = static_cast<T>(0.0001);
    assert(IsEqual(Length(q1), static_cast<T>(1.0), k_epsilon));
    assert(IsEqual(Length(q2), static_cast<T>(1.0), k_epsilon));
#endif

    const Quaternion q3 = q1 * (1 - param) + q2 * param;
    return Normalize(q3);
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Slerp(T param, const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    // Implementation based on: Understanding Slerp, Then Not Using It, Jonathan Blow
    // http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/

    constexpr T k_epsilon = static_cast<T>(0.0001);
    assert(IsEqual(Length(q1), static_cast<T>(1.0), k_epsilon));
    assert(IsEqual(Length(q2), static_cast<T>(1.0), k_epsilon));

    const T cos_theta0 = Dot(q1, q2);
    if (cos_theta0 > static_cast<T>(1.0) - k_epsilon)
    {
        return Lerp(param, q1, q2);
    }

    const T theta0 = std::acos(cos_theta0);
    const T theta = theta0 * param;

    Quaternion q3 = q2 - q1 * cos_theta0;
    q3 = Normalize(q3);

    return q1 * Opal::Cos(theta) + q3 * Opal::Sin(theta);
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Conjugate(const Quaternion<T>& q)
{
    Quaternion<T> ret = q;
    ret.vec = -ret.vec;
    return ret;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::Inverse(const Quaternion<T>& q)
{
    const T length_squared = LengthSquared(q);
    assert(length_squared != 0);
    return Conjugate(q) / length_squared;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator+(const Quaternion<T>& q)
{
    return q;
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::operator-(const Quaternion<T>& q)
{
    return {-q.w, -q.vec.x, -q.vec.y, -q.vec.z};
}

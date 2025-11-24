#pragma once

#include "opal/math-base.h"
#include "opal/math/quaternion.h"
#include "opal/math/vector3.h"

namespace Opal
{

template <FloatingPoint T>
struct DualQuaternion
{
    Quaternion<T> real;
    Quaternion<T> dual;

    ~DualQuaternion() = default;

    DualQuaternion();
    DualQuaternion(const Quaternion<T>& in_real, const Quaternion<T>& in_dual);
    explicit DualQuaternion(const Quaternion<T>& orientation);
    explicit DualQuaternion(const Vector3<T>& translation);
    DualQuaternion(const Quaternion<T>& orientation, const Vector3<T>& translation);

    DualQuaternion(const DualQuaternion<T>& other) = default;
    template <Opal::FloatingPoint U>
    explicit DualQuaternion(const DualQuaternion<U>& other);

    DualQuaternion<T>& operator=(const DualQuaternion<T>& other) = default;
    template <Opal::FloatingPoint U>
    DualQuaternion<T>& operator=(const DualQuaternion<U>& other);

    DualQuaternion(DualQuaternion&& other) = delete;
    DualQuaternion& operator=(DualQuaternion&& other) = delete;

    static DualQuaternion Identity();

    template <typename U>
    DualQuaternion<T>& operator*=(U scalar);

    template <typename U>
    DualQuaternion<T>& operator/=(U scalar);

    bool operator==(const DualQuaternion<T>& other) const;
    bool operator!=(const DualQuaternion<T>& other) const;

    Quaternion<T> GetOrientation() const;
    Vector3<T> GetTranslation() const;
};

template <Opal::FloatingPoint T>
bool IsEqual(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs, T epsilon);

template <Opal::FloatingPoint T>
DualQuaternion<T> operator+(const DualQuaternion<T>& dq);

template <Opal::FloatingPoint T>
DualQuaternion<T> operator-(const DualQuaternion<T>& dq);

template <Opal::FloatingPoint T>
DualQuaternion<T> operator+(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs);

template <Opal::FloatingPoint T>
DualQuaternion<T> operator*(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs);

template <Opal::FloatingPoint T>
Point3<T> operator*(const DualQuaternion<T>& dq, const Point3<T>& point);

template <Opal::FloatingPoint T, typename U>
DualQuaternion<T> operator*(const DualQuaternion<T>& dq, U scalar);

template <Opal::FloatingPoint T, typename U>
DualQuaternion<T> operator*(U scalar, const DualQuaternion<T>& dq);

template <Opal::FloatingPoint T, typename U>
DualQuaternion<T> operator/(const DualQuaternion<T>& dq, U scalar);

template <Opal::FloatingPoint T>
DualQuaternion<T> Conjugate(const DualQuaternion<T>& dq);

template <Opal::FloatingPoint T>
DualQuaternion<T> Inverse(const DualQuaternion<T>& dq);

}  // namespace Opal

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T>::DualQuaternion()
{
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T>::DualQuaternion(const Quaternion<T>& in_real, const Quaternion<T>& in_dual) : real(in_real), dual(in_dual)
{
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T>::DualQuaternion(const Quaternion<T>& orientation) : real(orientation), dual(0, 0, 0, 0)
{
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T>::DualQuaternion(const Vector3<T>& translation)
    : real(1, 0, 0, 0), dual(0, translation.x / 2, translation.y / 2, translation.z / 2)
{
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T>::DualQuaternion(const Quaternion<T>& orientation, const Vector3<T>& translation)
    : real(orientation),
      // If r is a rotation quaternion and t is a translation quaternion, then the dual quaternion
      // representing the transformation is given by q = r + 0.5 * t * r
      dual(static_cast<T>(0.5) * Quaternion<T>(0, translation.x, translation.y, translation.z) * orientation)
{
}

template <Opal::FloatingPoint T>
template <Opal::FloatingPoint U>
Opal::DualQuaternion<T>::DualQuaternion(const DualQuaternion<U>& other) : real(other.real), dual(other.dual)
{
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::DualQuaternion<T>::Identity()
{
    return DualQuaternion<T>(Quaternion<T>(1, 0, 0, 0), Quaternion<T>(0, 0, 0, 0));
}

template <Opal::FloatingPoint T>
template <Opal::FloatingPoint U>
Opal::DualQuaternion<T>& Opal::DualQuaternion<T>::operator=(const DualQuaternion<U>& other)
{
    real = other.real;
    dual = other.dual;
    return *this;
}

template <Opal::FloatingPoint T>
template <typename U>
Opal::DualQuaternion<T>& Opal::DualQuaternion<T>::operator*=(U scalar)
{
    real *= static_cast<T>(scalar);
    dual *= static_cast<T>(scalar);
    return *this;
}

template <Opal::FloatingPoint T>
template <typename U>
Opal::DualQuaternion<T>& Opal::DualQuaternion<T>::operator/=(U scalar)
{
    real /= static_cast<T>(scalar);
    dual /= static_cast<T>(scalar);
    return *this;
}

template <Opal::FloatingPoint T>
bool Opal::DualQuaternion<T>::operator==(const DualQuaternion<T>& other) const
{
    return real == other.real && dual == other.dual;
}

template <Opal::FloatingPoint T>
bool Opal::DualQuaternion<T>::operator!=(const DualQuaternion<T>& other) const
{
    return !(*this == other);
}

template <Opal::FloatingPoint T>
Opal::Quaternion<T> Opal::DualQuaternion<T>::GetOrientation() const
{
    return real;
}

template <Opal::FloatingPoint T>
Opal::Vector3<T> Opal::DualQuaternion<T>::GetTranslation() const
{
    Quaternion<T> result = dual * Conjugate(real);
    return static_cast<T>(2) * result.vec;
}

template <Opal::FloatingPoint T>
bool Opal::IsEqual(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs, T epsilon)
{
    return IsEqual(lhs.real, rhs.real, epsilon) && IsEqual(lhs.dual, rhs.dual, epsilon);
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::operator+(const DualQuaternion<T>& dq)
{
    return dq;
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::operator-(const DualQuaternion<T>& dq)
{
    return {-dq.real, -dq.dual};
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::operator+(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs)
{
    return DualQuaternion<T>(lhs.real + rhs.real, lhs.dual + rhs.dual);
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::operator*(const DualQuaternion<T>& lhs, const DualQuaternion<T>& rhs)
{
    return DualQuaternion<T>(lhs.real * rhs.real, lhs.real * rhs.dual + lhs.dual * rhs.real);
}

template <Opal::FloatingPoint T>
Opal::Point3<T> Opal::operator*(const DualQuaternion<T>& dq, const Point3<T>& point)
{
    const Quaternion<T> p(0, point.x, point.y, point.z);
    Quaternion<T> result = dq.real * p * Conjugate(dq.real) + static_cast<T>(2) * dq.dual * Conjugate(dq.real);
    return Point3<T>(result.vec.x, result.vec.y, result.vec.z);
}

template <Opal::FloatingPoint T, typename U>
Opal::DualQuaternion<T> Opal::operator*(const DualQuaternion<T>& dq, U scalar)
{
    return DualQuaternion<T>(dq.real * scalar, dq.dual * scalar);
}

template <Opal::FloatingPoint T, typename U>
Opal::DualQuaternion<T> Opal::operator*(U scalar, const DualQuaternion<T>& dq)
{
    return DualQuaternion<T>(dq.real * scalar, dq.dual * scalar);
}

template <Opal::FloatingPoint T, typename U>
Opal::DualQuaternion<T> Opal::operator/(const DualQuaternion<T>& dq, U scalar)
{
    OPAL_ASSERT(scalar != 0, "Division by zero");
    return DualQuaternion<T>(dq.real / scalar, dq.dual / scalar);
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::Conjugate(const DualQuaternion<T>& dq)
{
    return DualQuaternion<T>(Conjugate(dq.real), Conjugate(dq.dual));
}

template <Opal::FloatingPoint T>
Opal::DualQuaternion<T> Opal::Inverse(const DualQuaternion<T>& dq)
{
    const Quaternion<T> real = Conjugate(dq.real);
    const Quaternion<T> dual = Conjugate(dq.dual);
    return DualQuaternion<T>(real, dual + (real * (static_cast<T>(-2.0f) * Dot(real, dual))));
}

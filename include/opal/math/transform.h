#pragma once

#include "opal/math/matrix.h"
#include "opal/math/point4.h"
#include "opal/math/quaternion.h"
#include "opal/math/rotator.h"

namespace Opal
{

/**
 * Create an identity transform.
 * @tparam T The data type.
 * @return The identity transform.
 */
template <typename T>
Matrix4x4<T> Identity();

/**
 * Create a translation matrix.
 * @tparam T The data type.
 * @param delta The translation vector.
 * @return The translation matrix.
 */
template <typename T>
Matrix4x4<T> Translate(const Point3<T>& delta);

/**
 * Create a translation matrix.
 * @tparam T The data type.
 * @param delta The translation vector.
 * @return The translation matrix.
 */
template <typename T>
Matrix4x4<T> Translate(const Vector3<T>& delta);

/**
 * Create a scale matrix.
 * @tparam T The data type.
 * @param x Amount to scale along the x-axis.
 * @param y Amount to scale along the y-axis.
 * @param z Amount to scale along the z-axis.
 * @return The scale matrix.
 */
template <typename T>
Matrix4x4<T> Scale(T x, T y, T z);

/**
 * Create a scale matrix.
 * @tparam T The data type.
 * @param scalar The amount to scale.
 * @return The scale matrix.
 */
template <typename T>
Matrix4x4<T> Scale(T scalar);

/**
 * Create a rotation matrix around the x-axis.
 * @tparam T The data type.
 * @param angle_degrees The angle in degrees.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> RotateX(T angle_degrees);

/**
 * Create a rotation matrix around the y-axis.
 * @tparam T The data type.
 * @param angle_degrees The angle in degrees.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> RotateY(T angle_degrees);

/**
 * Create a rotation matrix around the z-axis.
 * @tparam T The data type.
 * @param angle_degrees The angle in degrees.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> RotateZ(T angle_degrees);

/**
 * Create a rotation matrix around an arbitrary axis.
 * @tparam T The data type.
 * @param angle_degrees The angle in degrees.
 * @param axis The axis to rotate around.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> Rotate(T angle_degrees, const Vector3<T>& axis);

/**
 * Create a rotation matrix from a quaternion.
 * @tparam T The data type.
 * @param q The quaternion.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> Rotate(const Quaternion<T>& q);

/**
 * Create a rotation matrix from a rotator. This applies angle around x then around y and then
 * around z axis.
 * @tparam T The data type.
 * @param rot The rotator.
 * @return The rotation matrix.
 */
template <typename T>
Matrix4x4<T> Rotate(const Rotator<T>& rot);

/**
 * Create a rotation and translation matrix from a rotator and a translation vector.
 * @tparam T The data type.
 * @param rot Rotator. This applies angle around x then around y and then around z axis.
 * @param t Translation vector. This is applied after the rotation.
 * @return The rotation and translation matrix.
 */
template <typename T>
Matrix4x4<T> RotateAndTranslate(const Rotator<T>& rot, const Point3<T>& t);

/**
 * Create a rotation and translation matrix from a rotator and a translation vector.
 * @tparam T The data type.
 * @param rot Rotator. This applies angle around x then around y and then around z axis.
 * @param t Translation vector. This is applied after the rotation.
 * @return The rotation and translation matrix.
 */
template <typename T>
Matrix4x4<T> RotateAndTranslate(const Rotator<T>& rot, const Vector3<T>& t);

/**
 *  Create a world to camera (view) transform for a right-handed coordinate system.
 * @tparam T The data type.
 * @param eye The position of the camera.
 * @param target The position the camera is looking at.
 * @param up The up vector.
 * @return The view matrix.
 */
template <typename T>
Matrix4x4<T> LookAt_RH(const Point3<T>& eye, const Point3<T>& target, const Vector3<T>& up);

/**
 * Create a world to camera (view) transform for a left-handed coordinate system.
 * @tparam T The data type.
 * @param eye The position of the camera.
 * @param target The position the camera is looking at.
 * @param up The up vector.
 * @return The view matrix.
 */
template <typename T>
Matrix4x4<T> LookAt_LH(const Point3<T>& eye, const Point3<T>& target, const Vector3<T>& up);

}  // namespace Math

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <typename T>
Opal::Matrix4x4<T> Opal::Identity()
{
    return Matrix4x4<T>(static_cast<T>(1.0f));
}

template <typename T>
Opal::Matrix4x4<T> Opal::Translate(const Point3<T>& delta)
{
    Matrix4x4<T> result = Identity<T>();
    result.elements[0][3] = delta.x;
    result.elements[1][3] = delta.y;
    result.elements[2][3] = delta.z;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Translate(const Vector3<T>& delta)
{
    Matrix4x4<T> result = Identity<T>();
    result.elements[0][3] = delta.x;
    result.elements[1][3] = delta.y;
    result.elements[2][3] = delta.z;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Scale(T x, T y, T z)
{
    Matrix4x4<T> result = Identity<T>();
    result.elements[0][0] = x;
    result.elements[1][1] = y;
    result.elements[2][2] = z;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Scale(T scalar)
{
    return Scale(scalar, scalar, scalar);
}

template <typename T>
Opal::Matrix4x4<T> Opal::RotateX(T angle_degrees)
{
    const T angle_radians = Radians(angle_degrees);
    const T cos = std::cos(angle_radians);
    const T sin = std::sin(angle_radians);

    Matrix4x4<T> result = Identity<T>();
    result.elements[1][1] = cos;
    result.elements[1][2] = -sin;
    result.elements[2][1] = sin;
    result.elements[2][2] = cos;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::RotateY(T angle_degrees)
{
    const T angle_radians = Radians(angle_degrees);
    const T cos = std::cos(angle_radians);
    const T sin = std::sin(angle_radians);

    Matrix4x4<T> result = Identity<T>();
    result.elements[0][0] = cos;
    result.elements[0][2] = sin;
    result.elements[2][0] = -sin;
    result.elements[2][2] = cos;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::RotateZ(T angle_degrees)
{
    const T angle_radians = Radians(angle_degrees);
    const T cos = std::cos(angle_radians);
    const T sin = std::sin(angle_radians);

    Matrix4x4<T> result = Identity<T>();
    result.elements[0][0] = cos;
    result.elements[0][1] = -sin;
    result.elements[1][0] = sin;
    result.elements[1][1] = cos;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Rotate(T angle_degrees, const Vector3<T>& axis)
{
    const T angle_radians = Radians(angle_degrees);
    const T cos = std::cos(angle_radians);
    const T sin = std::sin(angle_radians);
    const T one_minus_cos = 1 - cos;

    const Vector3<T> norm_axis = Normalize(axis);
    const T x = norm_axis.x;
    const T y = norm_axis.y;
    const T z = norm_axis.z;

    Matrix4x4<T> result = Identity<T>();
    result.elements[0][0] = x * x * one_minus_cos + cos;
    result.elements[0][1] = x * y * one_minus_cos - z * sin;
    result.elements[0][2] = x * z * one_minus_cos + y * sin;
    result.elements[1][0] = y * x * one_minus_cos + z * sin;
    result.elements[1][1] = y * y * one_minus_cos + cos;
    result.elements[1][2] = y * z * one_minus_cos - x * sin;
    result.elements[2][0] = z * x * one_minus_cos - y * sin;
    result.elements[2][1] = z * y * one_minus_cos + x * sin;
    result.elements[2][2] = z * z * one_minus_cos + cos;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Rotate(const Quaternion<T>& q)
{
    const T x = q.vec.x;
    const T y = q.vec.y;
    const T z = q.vec.z;
    const T w = q.w;

    Matrix4x4<T> result = Identity<T>();
    result.elements[0][0] = 1 - 2 * y * y - 2 * z * z;
    result.elements[0][1] = 2 * x * y - 2 * z * w;
    result.elements[0][2] = 2 * x * z + 2 * y * w;
    result.elements[1][0] = 2 * x * y + 2 * z * w;
    result.elements[1][1] = 1 - 2 * x * x - 2 * z * z;
    result.elements[1][2] = 2 * y * z - 2 * x * w;
    result.elements[2][0] = 2 * x * z - 2 * y * w;
    result.elements[2][1] = 2 * y * z + 2 * x * w;
    result.elements[2][2] = 1 - 2 * x * x - 2 * y * y;
    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::Rotate(const Rotator<T>& rot)
{
    return RotateZ(rot.pitch) * RotateY(rot.yaw) * RotateX(rot.roll);
}

template <typename T>
Opal::Matrix4x4<T> Opal::RotateAndTranslate(const Rotator<T>& rot, const Point3<T>& t)
{
    return Translate(t) * Rotate(rot);
}

template <typename T>
Opal::Matrix4x4<T> Opal::RotateAndTranslate(const Rotator<T>& rot, const Vector3<T>& t)
{
    return Translate(t) * Rotate(rot);
}

template <typename T>
Opal::Matrix4x4<T> Opal::LookAt_RH(const Point3<T>& eye,
                                   const Point3<T>& target,
                                   const Vector3<T>& up)
{
    const Vector3<T> forward = Normalize(eye - target);
    const Vector3<T> right = Normalize(Cross(up, forward));
    const Vector3<T> new_up = Cross(forward, right);
    const Vector3<T> eye_vector = eye - Point3<T>::Zero();

    Matrix4x4<T> result(1.0f);
    result(0, 0) = right.x;
    result(0, 1) = right.y;
    result(0, 2) = right.z;
    result(1, 0) = new_up.x;
    result(1, 1) = new_up.y;
    result(1, 2) = new_up.z;
    result(2, 0) = forward.x;
    result(2, 1) = forward.y;
    result(2, 2) = forward.z;
    result(0, 3) = -Dot(right, eye_vector);
    result(1, 3) = -Dot(new_up, eye_vector);
    result(2, 3) = -Dot(forward, eye_vector);

    return result;
}

template <typename T>
Opal::Matrix4x4<T> Opal::LookAt_LH(const Point3<T>& eye,
                                   const Point3<T>& target,
                                   const Vector3<T>& up)
{
    const Vector3<T> forward = Normalize(target - eye);
    const Vector3<T> right = Normalize(Cross(up, forward));
    const Vector3<T> new_up = Cross(forward, right);
    const Vector3<T> eye_vector = eye - Point3<T>::Zero();

    Matrix4x4<T> result(1.0f);
    result(0, 0) = right.x;
    result(0, 1) = right.y;
    result(0, 2) = right.z;
    result(1, 0) = new_up.x;
    result(1, 1) = new_up.y;
    result(1, 2) = new_up.z;
    result(2, 0) = forward.x;
    result(2, 1) = forward.y;
    result(2, 2) = forward.z;
    result(0, 3) = -Dot(right, eye_vector);
    result(1, 3) = -Dot(new_up, eye_vector);
    result(2, 3) = -Dot(forward, eye_vector);

    return result;
}


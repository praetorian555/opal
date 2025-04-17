#pragma once

#include <ostream>
#include <string>

#include "opal/math/dual-quaternion.h"
#include "opal/math/point2.h"
#include "opal/math/quaternion.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Point2<T>& value)
{
    os << "Point2(x=" << value.x << ", y=" << value.y << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Point3<T>& value)
{
    os << "Point3(x=" << value.x << ", y=" << value.y << ", z=" << value.z << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Point4<T>& value)
{
    os << "Point4(x=" << value.x << ", y=" << value.y << ", z=" << value.z << ", w=" << value.w << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Vector2<T>& value)
{
    os << "Vector2(x=" << value.x << ", y=" << value.y << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Vector3<T>& value)
{
    os << "Vector3(x=" << value.x << ", y=" << value.y << ", z=" << value.z << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Vector4<T>& value)
{
    os << "Vector4(x=" << value.x << ", y=" << value.y << ", z=" << value.z << ", w=" << value.w << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::Quaternion<T>& value)
{
    os << "Quaternion(w=" << value.w << ", x=" << value.vec.x << ", y=" << value.vec.y << ", z=" << value.vec.z << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Opal::DualQuaternion<T>& value)
{
    os << "DualQuaternion(real=" << value.real << ", dual=" << value.dual << ")";
    return os;
}

// clang-format off
OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS
// clang-format on

template <typename T>
struct CompareQuaternion : Catch::Matchers::MatcherBase<Opal::Quaternion<T>>
{
    Opal::Quaternion<T> m_q;
    T m_epsilon;

    CompareQuaternion(const Opal::Quaternion<T>& q, T epsilon) : m_q(q), m_epsilon(epsilon) {}

    bool match(const Opal::Quaternion<T>& actual) const override { return IsEqual(actual, m_q, m_epsilon); }

    [[nodiscard]] std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_q << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_QUATERNION(actual, expected, epsilon) CHECK_THAT(actual, CompareQuaternion(expected, epsilon))
#define CHECK_QUATERNION_FALSE(actual, expected, epsilon) CHECK_THAT_FALSE(actual, CompareQuaternion(expected, epsilon))

template <typename T>
struct ComparePoint3 : Catch::Matchers::MatcherGenericBase
{
    Opal::Point3<T> m_p;
    T m_epsilon;

    ComparePoint3(const Opal::Point3<T>& p, T epsilon) : m_p(p), m_epsilon(epsilon) {}

    bool match(const Opal::Point3<T>& actual) const { return IsEqual(actual, m_p, m_epsilon); }

    std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_p << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_POINT3(actual, expected, epsilon) CHECK_THAT(actual, ComparePoint3(expected, epsilon))

template <typename T>
struct ComparePoint4 : Catch::Matchers::MatcherBase<Opal::Point4<T>>
{
    Opal::Point4<T> m_p;
    T m_epsilon;

    ComparePoint4(const Opal::Point4<T>& p, T epsilon) : m_p(p), m_epsilon(epsilon) {}

    bool match(const Opal::Point4<T>& actual) const override { return IsEqual(actual, m_p, m_epsilon); }

    [[nodiscard]] std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_p << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_POINT4(actual, expected, epsilon) CHECK_THAT(actual, ComparePoint4(expected, epsilon))

template <typename T>
struct CompareVector3 : Catch::Matchers::MatcherBase<Opal::Vector3<T>>
{
    Opal::Vector3<T> m_v;
    T m_epsilon;

    CompareVector3(const Opal::Vector3<T>& v, T epsilon) : m_v(v), m_epsilon(epsilon) {}

    bool match(const Opal::Vector3<T>& actual) const override { return IsEqual(actual, m_v, m_epsilon); }

    [[nodiscard]] std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_v << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_VECTOR3(actual, expected, epsilon) CHECK_THAT(actual, CompareVector3(expected, epsilon))

template <typename T>
struct CompareVector4 : Catch::Matchers::MatcherBase<Opal::Vector4<T>>
{
    Opal::Vector4<T> m_v;
    T m_epsilon;

    CompareVector4(const Opal::Vector4<T>& v, T epsilon) : m_v(v), m_epsilon(epsilon) {}

    bool match(const Opal::Vector4<T>& actual) const override { return IsEqual(actual, m_v, m_epsilon); }

    [[nodiscard]] std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_v << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_VECTOR4(actual, expected, epsilon) CHECK_THAT(actual, CompareVector4(expected, epsilon))

template <typename T>
struct CompareDualQuaternion : Catch::Matchers::MatcherBase<Opal::DualQuaternion<T>>
{
    Opal::DualQuaternion<T> m_dq;
    T m_epsilon;

    CompareDualQuaternion(const Opal::DualQuaternion<T>& dq, T epsilon) : m_dq(dq), m_epsilon(epsilon) {}

    bool match(const Opal::DualQuaternion<T>& actual) const override { return IsEqual(actual, m_dq, m_epsilon); }

    [[nodiscard]] std::string describe() const override
    {
        std::ostringstream ss;
        ss << "is equal to " << m_dq << " within " << m_epsilon;
        return ss.str();
    }
};

#define CHECK_DUAL_QUATERNION(actual, expected, epsilon) CHECK_THAT(actual, CompareDualQuaternion(expected, epsilon))

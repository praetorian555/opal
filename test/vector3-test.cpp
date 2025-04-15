#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/vector3.h"

using Vec3f = Opal::Vector3<float>;
using Vec3d = Opal::Vector3<double>;
using Vec3i = Opal::Vector3<int>;

TEST_CASE("Vector3 constructor", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        CHECK(vec1.x == 1);;
        CHECK(vec1.y == 2);;
        CHECK(vec1.z == 3);;

        const Vec3f vec2(1);
        CHECK(vec2.x == 1);;
        CHECK(vec2.y == 1);;
        CHECK(vec2.z == 1);;
    }
    {
        const Vec3d vec1(1, 2, 3);
        CHECK(vec1.x == 1);;
        CHECK(vec1.y == 2);;
        CHECK(vec1.z == 3);;

        const Vec3d vec2(1);
        CHECK(vec2.x == 1);;
        CHECK(vec2.y == 1);;
        CHECK(vec2.z == 1);;
    }
    {
        const Vec3i vec1(1, 2, 3);
        CHECK(vec1.x == 1);;
        CHECK(vec1.y == 2);;
        CHECK(vec1.z == 3);;

        const Vec3i vec2(1);
        CHECK(vec2.x == 1);;
        CHECK(vec2.y == 1);;
        CHECK(vec2.z == 1);;
    }
}

TEST_CASE("Vector3 non finite", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Vector3 NaN", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec3d vec1(1, 2, 4);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Vector3 comparisons", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(1, 2, 3);
        const Vec3f vec3(1, 2, 4);
        const Vec3f vec4(1, 2, 4);
        const Vec3f vec5(1, 3, 3);
        const Vec3f vec6(2, 2, 3);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 == vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 != vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(1, 2, 3);
        const Vec3d vec3(1, 2, 4);
        const Vec3d vec4(1, 2, 4);
        const Vec3d vec5(1, 3, 3);
        const Vec3d vec6(2, 2, 3);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 == vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 != vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(1, 2, 3);
        const Vec3i vec3(1, 2, 4);
        const Vec3i vec4(1, 2, 4);
        const Vec3i vec5(1, 3, 3);
        const Vec3i vec6(2, 2, 3);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 == vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 != vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
}

TEST_CASE("Vector3 addition", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);

        Vec3f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 8.0f);;

        vec3 += vec1;
        CHECK(vec3.x == 5.0f);;
        CHECK(vec3.y == 8.0f);;
        CHECK(vec3.z == 11.0f);;
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);

        Vec3d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);;
        CHECK(vec3.y == 6.0);;
        CHECK(vec3.z == 8.0);;

        vec3 += vec1;
        CHECK(vec3.x == 5.0);;
        CHECK(vec3.y == 8.0);;
        CHECK(vec3.z == 11.0);;
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);

        Vec3i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);;
        CHECK(vec3.y == 6);;
        CHECK(vec3.z == 8);;

        vec3 += vec1;
        CHECK(vec3.x == 5);;
        CHECK(vec3.y == 8);;
        CHECK(vec3.z == 11);;
    }
}

TEST_CASE("Vector3T subtraction", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);

        Vec3f vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 2.0f);;
        CHECK(vec3.z == 2.0f);;

        vec3 -= vec1;
        CHECK(vec3.x == 1.0f);;
        CHECK(vec3.y == 0.0f);;
        CHECK(vec3.z == -1.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);

        Vec3d vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0);;
        CHECK(vec3.y == 2.0);;
        CHECK(vec3.z == 2.0);;

        vec3 -= vec1;
        CHECK(vec3.x == 1.0);;
        CHECK(vec3.y == 0.0);;
        CHECK(vec3.z == -1.0);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);

        Vec3i vec3 = vec2 - vec1;
        CHECK(vec3.x == 2);;
        CHECK(vec3.y == 2);;
        CHECK(vec3.z == 2);;

        vec3 -= vec1;
        CHECK(vec3.x == 1);;
        CHECK(vec3.y == 0);;
        CHECK(vec3.z == -1);
    }
}

TEST_CASE("Vector3 multiplication with scalar", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);

        Vec3f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        const Vec3f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);;
        CHECK(vec3.y == 40.0f);;
        CHECK(vec3.z == 60.0f);;
    }
    {
        const Vec3d vec1(1, 2, 3);

        Vec3d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        const Vec3d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);;
        CHECK(vec3.y == 40.0f);;
        CHECK(vec3.z == 60.0f);;
    }
    {
        const Vec3i vec1(1, 2, 3);

        Vec3i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        const Vec3i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);;
        CHECK(vec3.y == 40.0f);;
        CHECK(vec3.z == 60.0f);;
    }
}

TEST_CASE("Vector3 multiplication", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        Vec3f vec2(2, 3, 4);

        const Vec3f vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 12.0f);;

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);;
        CHECK(vec2.y == 6.0f);;
        CHECK(vec2.z == 12.0f);;
    }
    {
        const Vec3d vec1(1, 2, 3);
        Vec3d vec2(2, 3, 4);

        const Vec3d vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 12.0f);;

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);;
        CHECK(vec2.y == 6.0f);;
        CHECK(vec2.z == 12.0f);;
    }
    {
        const Vec3i vec1(1, 2, 3);
        Vec3i vec2(2, 3, 4);

        const Vec3i vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 12.0f);;

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);;
        CHECK(vec2.y == 6.0f);;
        CHECK(vec2.z == 12.0f);;
    }
}

TEST_CASE("Vector3 division with scalar", "[math][vector3]")
{
    {
        const Vec3f vec1(20.0f, 40.0f, 60.0f);

        Vec3f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;
    }
    {
        const Vec3d vec1(20.0f, 40.0f, 60.0f);

        Vec3d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;
    }
    {
        const Vec3i vec1(20, 40, 60);

        Vec3i vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;
    }
}

TEST_CASE("Vector3 negation", "[math][vector3]")
{
    {
        const Vec3f vec(5, -10, -15);
        const Vec3f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);;
        CHECK(neg.z == 15.0f);;
    }
    {
        const Vec3d vec(5, -10, -15);
        const Vec3d neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);;
        CHECK(neg.z == 15.0f);;
    }
    {
        const Vec3i vec(5, -10, -15);
        const Vec3i neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);;
        CHECK(neg.z == 15.0f);;
    }
}

TEST_CASE("Vector3 abs", "[math][vector3]")
{
    {
        const Vec3f vec(5, -10, -15);
        const Vec3f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);;
        CHECK(a.y == 10.0f);;
        CHECK(a.z == 15.0f);;
    }
    {
        const Vec3d vec(5, -10, -15);
        const Vec3d a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);;
        CHECK(a.y == 10.0f);;
        CHECK(a.z == 15.0f);;
    }
    {
        const Vec3i vec(5, -10, -15);
        const Vec3i a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);;
        CHECK(a.y == 10.0f);;
        CHECK(a.z == 15.0f);;
    }
}

TEST_CASE("Vector3 length", "[math][vector3]")
{
    {
        const Vec3f v1(3, 4, 5);

        CHECK(Opal::Length(v1) == std::sqrt(50.0));
        CHECK(Opal::LengthSquared(v1) == 50);
    }
    {
        const Vec3d v1(3, 4, 5);

        CHECK(Opal::Length(v1) == std::sqrt(50.0));
        CHECK(Opal::LengthSquared(v1) == 50);
    }
    {
        const Vec3i v1(3, 4, 5);

        CHECK(Opal::Length(v1) == std::sqrt(50.0));
        CHECK(Opal::LengthSquared(v1) == 50);
    }
}

TEST_CASE("Vector3 lerp", "[math][vector3]")
{
    {
        const Vec3f v1(1, 2, 3);
        const Vec3f v2(3, 4, 5);

        const Vec3f v3 = Opal::Lerp(0.5f, v1, v2);
        CHECK(v3.x == 2.0f);;
        CHECK(v3.y == 3.0f);;
        CHECK(v3.z == 4.0f);;
    }
    {
        const Vec3d v1(1, 2, 3);
        const Vec3d v2(3, 4, 5);

        const Vec3d v3 = Opal::Lerp(0.5, v1, v2);
        CHECK(v3.x == 2.0f);;
        CHECK(v3.y == 3.0f);;
        CHECK(v3.z == 4.0f);;
    }
}

TEST_CASE("Vector3 dot", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);
        const Vec3f vec3(-3, -4, -5);

        CHECK(Dot(vec1, vec2) == 26.0f);
        CHECK(AbsDot(vec1, vec3) == 26.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);
        const Vec3d vec3(-3, -4, -5);

        CHECK(Dot(vec1, vec2) == 26.0f);
        CHECK(AbsDot(vec1, vec3) == 26.0f);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);
        const Vec3i vec3(-3, -4, -5);

        CHECK(Dot(vec1, vec2) == 26.0f);
        CHECK(AbsDot(vec1, vec3) == 26.0f);
    }
}

TEST_CASE("Vector3 cross product", "[math][vector3]")
{
    {
        const Vec3f x(1, 0, 0);
        const Vec3f y(0, 1, 0);
        const Vec3f z(0, 0, 1);

        const Vec3f c = Cross(x, y);
        CHECK(c == z);
    }
    {
        const Vec3d x(1, 0, 0);
        const Vec3d y(0, 1, 0);
        const Vec3d z(0, 0, 1);

        const Vec3d c = Cross(x, y);
        CHECK(c == z);
    }
    {
        const Vec3i x(1, 0, 0);
        const Vec3i y(0, 1, 0);
        const Vec3i z(0, 0, 1);

        const Vec3i c = Cross(x, y);
        CHECK(c == z);
    }
}

TEST_CASE("Vector3 cross product in 2D", "[math][vector3]")
{
    {
        const Vec3f vec1(0, 1, 1);
        const Vec3f vec2(1, 0, 2);

        CHECK(-1 == Cross2D(vec1, vec2));
    }
    {
        const Vec3d vec1(0, 1, 1);
        const Vec3d vec2(1, 0, 2);

        CHECK(-1 == Cross2D(vec1, vec2));
    }
    {
        const Vec3i vec1(0, 1, 1);
        const Vec3i vec2(1, 0, 2);

        CHECK(-1 == Cross2D(vec1, vec2));
    }
}

TEST_CASE("Vector3 normalize", "[math][vector3]")
{
    {
        const Vec3f vec(2, 3, 5);
        const Vec3f norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
    {
        const Vec3d vec(2, 3, 5);
        const Vec3d norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
}

TEST_CASE("Vector3 misc", "[math][vector3]")
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, -2, 5);
        const Vec3f min = Min(vec1, vec2);
        const Vec3f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);;
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);;
        CHECK(max.x == 3.0f);;
        CHECK(max.y == 2.0f);;
        CHECK(max.z == 5.0f);;

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);;
        CHECK(perm.y == 3.0f);;
        CHECK(perm.z == 1.0f);;
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, -2, 5);
        const Vec3d min = Min(vec1, vec2);
        const Vec3d max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);;
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);;
        CHECK(max.x == 3.0f);;
        CHECK(max.y == 2.0f);;
        CHECK(max.z == 5.0f);;

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);;
        CHECK(perm.y == 3.0f);;
        CHECK(perm.z == 1.0f);;
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, -2, 5);
        const Vec3i min = Min(vec1, vec2);
        const Vec3i max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);;
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);;
        CHECK(max.x == 3.0f);;
        CHECK(max.y == 2.0f);;
        CHECK(max.z == 5.0f);;

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);;
        CHECK(perm.y == 3.0f);;
        CHECK(perm.z == 1.0f);;
    }
}

TEST_CASE("Vector3 clamp", "[math][vector3]")
{
    {
        const Vec3f v1(2, -5, 10);
        const Vec3f v2 = Opal::Clamp(v1, 0.0f, 5.0f);
        CHECK(v2.x == 2);;
        CHECK(v2.y == 0);;
        CHECK(v2.z == 5);;
    }
    {
        const Vec3d v1(2, -5, 10);
        const Vec3d v2 = Opal::Clamp(v1, 0.0, 5.0);
        CHECK(v2.x == 2);;
        CHECK(v2.y == 0);;
        CHECK(v2.z == 5);;
    }
    {
        const Vec3i v1(2, -5, 10);
        const Vec3i v2 = Opal::Clamp(v1, 0, 5);
        CHECK(v2.x == 2);;
        CHECK(v2.y == 0);;
        CHECK(v2.z == 5);;
    }
}

TEST_CASE("Vector3 reflection", "[math][vector3]")
{
    {
        const Vec3f incidence(1, 0, 0);
        const Vec3f normal(0, 1, 0);
        const Vec3f reflection = Opal::Reflect(incidence, normal);
        CHECK(reflection.x == -1.0f);
        CHECK(reflection.y == 0.0f);;
        CHECK(reflection.z == 0.0f);;
    }
    {
        const Vec3d incidence(1, 0, 0);
        const Vec3d normal(0, 1, 0);
        const Vec3d reflection = Opal::Reflect(incidence, normal);
        CHECK(reflection.x == -1.0f);
        CHECK(reflection.y == 0.0f);;
        CHECK(reflection.z == 0.0f);;
    }
}

TEST_CASE("Vector3 comparison with IsEqual", "[math][vector3]")
{
    {
        const Vec3f v1(1, 2, 3);
        const Vec3f v2(2, 3, 4);
        const Vec3f v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Vec3d v1(1, 2, 3);
        const Vec3d v2(2, 3, 4);
        const Vec3d v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Vec3i v1(1, 2, 3);
        const Vec3i v2(2, 3, 4);
        const Vec3i v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Vector3 min and max component", "[math][vector3]")
{
    {
        const Vec3f v(1, 2, 3);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 3);
    }
    {
        const Vec3d v(1, 2, 3);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 3);
    }
    {
        const Vec3i v(1, 2, 3);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 3);
    }
}

TEST_CASE("Vector3 min and max dimension", "[math][vector3]")
{
    {
        const Vec3f v(1, 2, 3);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 2);
    }
    {
        const Vec3d v(1, 2, 3);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 2);
    }
    {
        const Vec3i v(1, 2, 3);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 2);
    }
}

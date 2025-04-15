#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/normal3.h"

using Normal3f = Opal::Normal3<float>;
using Normal3d = Opal::Normal3<double>;
using Vec3f = Opal::Vector3<float>;
using Vec3d = Opal::Vector3<double>;

TEST_CASE("Normal3 constructor", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);

        const Normal3f vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);

        const Normal3f vec3(Vec3f(1, 2, 3));
        CHECK(vec3.x == 1);
        CHECK(vec3.y == 2);
        CHECK(vec3.z == 3);
    }
    {
        const Normal3d vec1(1, 2, 3);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);

        const Normal3d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);

        const Normal3d vec3(Vec3d(1, 2, 3));
        CHECK(vec3.x == 1);
        CHECK(vec3.y == 2);
        CHECK(vec3.z == 3);
    }
}

TEST_CASE("Normal3 non-finite", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Normal3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Normal3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Normal3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Normal3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Normal3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Normal3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Normal3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Normal3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Normal3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Normal3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Normal3 NaN", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Normal3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Normal3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Normal3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Normal3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Normal3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Normal3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Normal3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Normal3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Normal3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Normal3d vec1(1, 2, 4);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Normal3 comparisons", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        const Normal3f vec2(1, 2, 3);
        const Normal3f vec3(1, 2, 4);
        const Normal3f vec4(1, 2, 4);
        const Normal3f vec5(1, 3, 3);
        const Normal3f vec6(2, 2, 3);

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
        const Normal3d vec1(1, 2, 3);
        const Normal3d vec2(1, 2, 3);
        const Normal3d vec3(1, 2, 4);
        const Normal3d vec4(1, 2, 4);
        const Normal3d vec5(1, 3, 3);
        const Normal3d vec6(2, 2, 3);

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

TEST_CASE("Normal3 addition", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        const Normal3f vec2(3, 4, 5);

        Normal3f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);
        CHECK(vec3.y == 6.0f);
        CHECK(vec3.z == 8.0f);

        vec3 += vec1;
        CHECK(vec3.x == 5.0f);
        CHECK(vec3.y == 8.0f);
        CHECK(vec3.z == 11.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);
        const Normal3d vec2(3, 4, 5);

        Normal3d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);
        CHECK(vec3.y == 6.0);
        CHECK(vec3.z == 8.0);

        vec3 += vec1;
        CHECK(vec3.x == 5.0);
        CHECK(vec3.y == 8.0);
        CHECK(vec3.z == 11.0);
    }
}

TEST_CASE("Normal3 subtraction", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        const Normal3f vec2(3, 4, 5);

        Normal3f vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 2.0f);
        CHECK(vec3.z == 2.0f);

        vec3 -= vec1;
        CHECK(vec3.x == 1.0f);
        CHECK(vec3.y == 0.0f);
        CHECK(vec3.z == -1.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);
        const Normal3d vec2(3, 4, 5);

        Normal3d vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0);
        CHECK(vec3.y == 2.0);
        CHECK(vec3.z == 2.0);

        vec3 -= vec1;
        CHECK(vec3.x == 1.0);
        CHECK(vec3.y == 0.0);
        CHECK(vec3.z == -1.0);
    }
}

TEST_CASE("Normal3 multiplication by scalar", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);

        Normal3f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        const Normal3f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);

        Normal3d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        const Normal3d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
    }
}

TEST_CASE("Normal3 multiplication", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        Normal3f vec2(2, 3, 4);

        const Normal3f vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 6.0f);
        CHECK(vec3.z == 12.0f);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);
        CHECK(vec2.y == 6.0f);
        CHECK(vec2.z == 12.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);
        Normal3d vec2(2, 3, 4);

        const Normal3d vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 6.0f);
        CHECK(vec3.z == 12.0f);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);
        CHECK(vec2.y == 6.0f);
        CHECK(vec2.z == 12.0f);
    }
}

TEST_CASE("Normal3 division by scalar", "[math][normal3]")
{
    {
        const Normal3f vec1(20.0f, 40.0f, 60.0f);

        Normal3f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
    }
    {
        const Normal3d vec1(20.0f, 40.0f, 60.0f);

        Normal3d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
    }
}

TEST_CASE("Normal3 negation", "[math][normal3]")
{
    {
        const Normal3f vec(5, -10, -15);
        const Normal3f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
    }
    {
        const Normal3d vec(5, -10, -15);
        const Normal3d neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
    }
}

TEST_CASE("Normal3 abs", "[math][normal3]")
{
    {
        const Normal3f vec(5, -10, -15);
        const Normal3f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
    }
    {
        const Normal3d vec(5, -10, -15);
        const Normal3d a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
    }
}

TEST_CASE("Normal3 length", "[math][normal3]")
{
    {
        const Normal3f v1(3, 4, 5);

        CHECK(Opal::Length(v1) == std::sqrt(50.0));
        CHECK(Opal::LengthSquared(v1) == 50);
    }
    {
        const Normal3d v1(3, 4, 5);

        CHECK(Opal::Length(v1) == std::sqrt(50.0));
        CHECK(Opal::LengthSquared(v1) == 50);
    }
}

TEST_CASE("Normal3 dot product", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        const Normal3f vec2(3, 4, 5);
        const Normal3f vec3(-3, -4, -5);
        const Vec3f vec4(3, 4, 5);
        const Vec3f vec5(-3, -4, -5);

        CHECK(Dot(vec1, vec2) == 26.0f);
        CHECK(Dot(vec1, vec4) == 26.0f);
        CHECK(Dot(vec4, vec1) == 26.0f);
        CHECK(AbsDot(vec1, vec3) == 26.0f);
        CHECK(AbsDot(vec1, vec5) == 26.0f);
        CHECK(AbsDot(vec5, vec1) == 26.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);
        const Normal3d vec2(3, 4, 5);
        const Normal3d vec3(-3, -4, -5);
        const Vec3d vec4(3, 4, 5);
        const Vec3d vec5(-3, -4, -5);

        CHECK(Dot(vec1, vec2) == 26.0f);
        CHECK(Dot(vec1, vec4) == 26.0f);
        CHECK(Dot(vec4, vec1) == 26.0f);
        CHECK(AbsDot(vec1, vec3) == 26.0f);
        CHECK(AbsDot(vec1, vec5) == 26.0f);
        CHECK(AbsDot(vec5, vec1) == 26.0f);
    }
}

TEST_CASE("Normal3 normalize", "[math][normal3]")
{
    {
        const Normal3f vec(2, 3, 5);
        const Normal3f norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
    {
        const Normal3d vec(2, 3, 5);
        const Normal3d norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
}

TEST_CASE("Normal3 misc", "[math][normal3]")
{
    {
        const Normal3f vec1(1, 2, 3);
        const Normal3f vec2(3, -2, 5);
        const Normal3f min = Min(vec1, vec2);
        const Normal3f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);

        const Normal3f vec(1, 2, 3);
        const Normal3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 1.0f);
    }
    {
        const Normal3d vec1(1, 2, 3);
        const Normal3d vec2(3, -2, 5);
        const Normal3d min = Min(vec1, vec2);
        const Normal3d max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);

        const Normal3f vec(1, 2, 3);
        const Normal3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 1.0f);
    }
}

TEST_CASE("Normal3 comparison with IsEqual", "[math][normal3]")
{
    {
        const Normal3f v1(1, 2, 3);
        const Normal3f v2(2, 3, 4);
        const Normal3f v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Normal3d v1(1, 2, 3);
        const Normal3d v2(2, 3, 4);
        const Normal3d v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
}

TEST_CASE("Normal3 min and max component", "[math][normal3]")
{
    {
        const Normal3f v(1, 2, 3);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 3);
    }
    {
        const Normal3d v(1, 2, 3);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 3);
    }
}

TEST_CASE("Normal3 min and max dimension", "[math][normal3]")
{
    {
        const Normal3f v(1, 2, 3);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 2);
    }
    {
        const Normal3d v(1, 2, 3);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 2);
    }
}

TEST_CASE("Normal3 face forward", "[math][normal3]")
{
    {
        const Normal3f n1(1, 2, 3);
        const Vec3f v1(1, 2, 5);
        const Vec3f v2(-1, -2, -5);

        CHECK(FaceForward(n1, v1) == n1);
        CHECK(FaceForward(n1, v2) == -n1);
    }
    {
        const Normal3d n1(1, 2, 3);
        const Vec3d v1(1, 2, 5);
        const Vec3d v2(-1, -2, -5);

        CHECK(FaceForward(n1, v1) == n1);
        CHECK(FaceForward(n1, v2) == -n1);
    }
}

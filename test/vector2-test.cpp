#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/vector2.h"

using Vec2f = Opal::Vector2<float>;
using Vec2d = Opal::Vector2<double>;
using Vec2i = Opal::Vector2<int>;

TEST_CASE("Vector2 constructor", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);

        const Vec2f vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
    }
    {
        const Vec2d vec1(1, 2);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);

        const Vec2d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
    }
    {
        const Vec2i vec1(1, 2);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);

        const Vec2i vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
    }
}

TEST_CASE("Vector2 non-inifinte", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec2f vec2(1, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec2f vec3(1, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec2f vec4(1, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec2f vec5(1, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec2d vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec2d vec2(1, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec2d vec3(1, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec2d vec4(1, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec2d vec5(1, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec2d vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Vector2 NaN", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec2f vec2(1, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec2f vec3(1, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec2f vec4(1, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec2f vec5(1, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec2d vec1(1, 2);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec2d vec2(1, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec2d vec3(1, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec2d vec4(1, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec2d vec5(1, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec2d vec1(1, 2);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Vector2 equality", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        const Vec2f vec2(1, 2);
        const Vec2f vec3(1, 3);
        const Vec2f vec4(1, 4);
        const Vec2f vec5(1, 5);
        const Vec2f vec6(2, 6);

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
        const Vec2d vec1(1, 2);
        const Vec2d vec2(1, 2);
        const Vec2d vec3(1, 3);
        const Vec2d vec4(1, 4);
        const Vec2d vec5(1, 5);
        const Vec2d vec6(2, 6);

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
        const Vec2i vec1(1, 2);
        const Vec2i vec2(1, 2);
        const Vec2i vec3(1, 3);
        const Vec2i vec4(1, 4);
        const Vec2i vec5(1, 5);
        const Vec2i vec6(2, 6);

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

TEST_CASE("Vector2 addition", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        const Vec2f vec2(3, 4);

        Vec2f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);
        CHECK(vec3.y == 6.0f);

        vec3 += vec1;
        CHECK(vec3.x == 5.0f);
        CHECK(vec3.y == 8.0f);
    }
    {
        const Vec2d vec1(1, 2);
        const Vec2d vec2(3, 4);

        Vec2d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);
        CHECK(vec3.y == 6.0);

        vec3 += vec1;
        CHECK(vec3.x == 5.0);
        CHECK(vec3.y == 8.0);
    }
    {
        const Vec2i vec1(1, 2);
        const Vec2i vec2(3, 4);

        Vec2i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);
        CHECK(vec3.y == 6);

        vec3 += vec1;
        CHECK(vec3.x == 5);
        CHECK(vec3.y == 8);
    }
}

TEST_CASE("Vector2 subtraction", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        const Vec2f vec2(3, 4);

        Vec2f vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 2.0f);

        vec3 -= vec1;
        CHECK(vec3.x == 1.0f);
        CHECK(vec3.y == 0.0f);
    }
    {
        const Vec2d vec1(1, 2);
        const Vec2d vec2(3, 4);

        Vec2d vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0);
        CHECK(vec3.y == 2.0);

        vec3 -= vec1;
        CHECK(vec3.x == 1.0);
        CHECK(vec3.y == 0.0);
    }
    {
        const Vec2i vec1(1, 2);
        const Vec2i vec2(3, 4);

        Vec2i vec3 = vec2 - vec1;
        CHECK(vec3.x == 2);
        CHECK(vec3.y == 2);

        vec3 -= vec1;
        CHECK(vec3.x == 1);
        CHECK(vec3.y == 0);
    }
}

TEST_CASE("Vector2 scalar multiplication", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);

        Vec2f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        const Vec2f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
    }
    {
        const Vec2d vec1(1, 2);

        Vec2d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        const Vec2d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
    }
    {
        const Vec2i vec1(1, 2);

        Vec2i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        const Vec2i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
    }
}

TEST_CASE("Vector2 per cell multiplication", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        Vec2f vec2(2, 3);

        const Vec2f vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 6.0f);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);
        CHECK(vec2.y == 6.0f);
    }
    {
        const Vec2d vec1(1, 2);
        Vec2d vec2(2, 3);

        const Vec2d vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 6.0f);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);
        CHECK(vec2.y == 6.0f);
    }
    {
        const Vec2i vec1(1, 2);
        Vec2i vec2(2, 3);

        const Vec2i vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);
        CHECK(vec3.y == 6.0f);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);
        CHECK(vec2.y == 6.0f);
    }
}

TEST_CASE("Vector2 scalar division", "[math][vector2]")
{
    {
        const Vec2f vec1(20.0f, 40.0f);

        Vec2f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
    }
    {
        const Vec2d vec1(20.0f, 40.0f);

        Vec2d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
    }
    {
        const Vec2i vec1(20, 40);

        Vec2i vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
    }
}

TEST_CASE("Vector2 negation", "[math][vector2]")
{
    {
        const Vec2f vec(5, -10);
        const Vec2f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
    }
    {
        const Vec2d vec(5, -10);
        const Vec2d neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
    }
    {
        const Vec2i vec(5, -10);
        const Vec2i neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
    }
}

TEST_CASE("Vector2 abs", "[math][vector2]")
{
    {
        const Vec2f vec(5, -10);
        const Vec2f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
    }
    {
        const Vec2d vec(5, -10);
        const Vec2d a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
    }
    {
        const Vec2i vec(5, -10);
        const Vec2i a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
    }
}

TEST_CASE("Vector2 length", "[math][vector2]")
{
    {
        const Vec2f v1(3, 4);

        CHECK(Opal::Length(v1) == std::sqrt(25.0));
        CHECK(Opal::LengthSquared(v1) == 25);
    }
    {
        const Vec2d v1(3, 4);

        CHECK(Opal::Length(v1) == std::sqrt(25.0));
        CHECK(Opal::LengthSquared(v1) == 25);
    }
    {
        const Vec2i v1(3, 4);

        CHECK(Opal::Length(v1) == std::sqrt(25.0));
        CHECK(Opal::LengthSquared(v1) == 25);
    }
}

TEST_CASE("Vector2 lerp", "[math][vector2]")
{
    {
        const Vec2f v1(1, 2);
        const Vec2f v2(3, 4);

        const Vec2f v3 = Opal::Lerp(0.5f, v1, v2);
        CHECK(v3.x == 2.0f);
        CHECK(v3.y == 3.0f);
    }
    {
        const Vec2d v1(1, 2);
        const Vec2d v2(3, 4);

        const Vec2d v3 = Opal::Lerp(0.5, v1, v2);
        CHECK(v3.x == 2.0f);
        CHECK(v3.y == 3.0f);
    }
}

TEST_CASE("Vector2 dot product", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        const Vec2f vec2(3, 4);
        const Vec2f vec3(-3, -4);

        CHECK(Dot(vec1, vec2) == 11.0f);
        CHECK(AbsDot(vec1, vec3) == 11.0f);
    }
    {
        const Vec2d vec1(1, 2);
        const Vec2d vec2(3, 4);
        const Vec2d vec3(-3, -4);

        CHECK(Dot(vec1, vec2) == 11.0f);
        CHECK(AbsDot(vec1, vec3) == 11.0f);
    }
    {
        const Vec2i vec1(1, 2);
        const Vec2i vec2(3, 4);
        const Vec2i vec3(-3, -4);

        CHECK(Dot(vec1, vec2) == 11.0f);
        CHECK(AbsDot(vec1, vec3) == 11.0f);
    }
}

TEST_CASE("Vector2 cross product", "[math][vector2]")
{
    {
        const Vec2f x(1, 2);
        const Vec2f y(2, 1);

        const float c = Cross(x, y);
        CHECK(c == -3.0f);
    }
    {
        const Vec2d x(1, 2);
        const Vec2d y(2, 1);

        const double c = Cross(x, y);
        CHECK(c == -3.0);
    }
    {
        const Vec2i x(1, 2);
        const Vec2i y(2, 1);

        const int c = Cross(x, y);
        CHECK(c == -3);
    }
}

TEST_CASE("Vector2 normalize", "[math][vector2]")
{
    {
        const Vec2f vec(2, 3);
        const Vec2f norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
    {
        const Vec2d vec(2, 3);
        const Vec2d norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
}

TEST_CASE("Vector2 min, max and permute", "[math][vector2]")
{
    {
        const Vec2f vec1(1, 2);
        const Vec2f vec2(3, -2);
        const Vec2f min = Min(vec1, vec2);
        const Vec2f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);

        const Vec2f vec(1, 2);
        const Vec2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
    {
        const Vec2d vec1(1, 2);
        const Vec2d vec2(3, -2);
        const Vec2d min = Min(vec1, vec2);
        const Vec2d max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);

        const Vec2f vec(1, 2);
        const Vec2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
    {
        const Vec2i vec1(1, 2);
        const Vec2i vec2(3, -2);
        const Vec2i min = Min(vec1, vec2);
        const Vec2i max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);

        const Vec2f vec(1, 2);
        const Vec2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
}

TEST_CASE("Vector2 clamp", "[math][vector2]")
{
    {
        const Vec2f v1(2, -5);
        const Vec2f v2 = Opal::Clamp(v1, 0.0f, 5.0f);
        CHECK(v2.x == 2);
        CHECK(v2.y == 0);
    }
    {
        const Vec2d v1(2, -5);
        const Vec2d v2 = Opal::Clamp(v1, 0.0, 5.0);
        CHECK(v2.x == 2);
        CHECK(v2.y == 0);
    }
    {
        const Vec2i v1(2, -5);
        const Vec2i v2 = Opal::Clamp(v1, 0, 5);
        CHECK(v2.x == 2);
        CHECK(v2.y == 0);
    }
}

TEST_CASE("Vector2 reflection", "[math][vector2]")
{
    {
        const Vec2f incidence(1, 0);
        const Vec2f normal(0, 1);
        const Vec2f reflection = Opal::Reflect(incidence, normal);
        CHECK(reflection.x == -1.0f);
        CHECK(reflection.y == 0.0f);
    }
    {
        const Vec2d incidence(1, 0);
        const Vec2d normal(0, 1);
        const Vec2d reflection = Opal::Reflect(incidence, normal);
        CHECK(reflection.x == -1.0f);
        CHECK(reflection.y == 0.0f);
    }
}

TEST_CASE("Vector2 comparison with IsEqual", "[math][vector2]")
{
    {
        const Vec2f v1(1, 2);
        const Vec2f v2(2, 3);
        const Vec2f v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Vec2d v1(1, 2);
        const Vec2d v2(2, 3);
        const Vec2d v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Vec2i v1(1, 2);
        const Vec2i v2(2, 3);
        const Vec2i v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Vector2 min and max component", "[math][vector2]")
{
    {
        const Vec2f v(1, 2);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 2);
    }
    {
        const Vec2d v(1, 2);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 2);
    }
    {
        const Vec2i v(1, 2);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 2);
    }
}

TEST_CASE("Vector2 min and max dimension", "[math][vector2]")
{
    {
        const Vec2f v(1, 2);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 1);
    }
    {
        const Vec2d v(1, 2);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 1);
    }
    {
        const Vec2i v(1, 2);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 1);
    }
}

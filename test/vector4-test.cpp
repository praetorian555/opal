#include "test-helpers.h"

#include "opal/math/vector4.h"

using Vec4f = Opal::Vector4<float>;
using Vec4d = Opal::Vector4<double>;
using Vec4i = Opal::Vector4<int>;

TEST_CASE("Vector4 constructor", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1.0f);
        CHECK(vec1.y == 2.0f);
        CHECK(vec1.z == 3.0f);
        CHECK(vec1.w == 4.0f);

        const Vec4f vec2(1);
        CHECK(vec2.x == 1.0f);
        CHECK(vec2.y == 1.0f);
        CHECK(vec2.z == 1.0f);
        CHECK(vec2.w == 1.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);
        CHECK(vec1.w == 4);

        const Vec4d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
        CHECK(vec2.w == 1);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1);;
        CHECK(vec1.y == 2);;
        CHECK(vec1.z == 3);;
        CHECK(vec1.w == 4);;

        const Vec4i vec2(1);
        CHECK(vec2.x == 1);;
        CHECK(vec2.y == 1);;
        CHECK(vec2.z == 1);;
        CHECK(vec2.w == 1);;
    }
}

TEST_CASE("Vector4 non-finite", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Vec4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Vec4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Vec4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Vec4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Vector4 NaN", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));

        const Vec4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Vec4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Vec4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Vec4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Vector4 comparison", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(1, 2, 3, 4);
        const Vec4f vec3(1, 2, 3, 5);
        const Vec4f vec4(1, 2, 4, 4);
        const Vec4f vec5(1, 3, 3, 4);
        const Vec4f vec6(2, 2, 3, 4);

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
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(1, 2, 3, 4);
        const Vec4d vec3(1, 2, 3, 5);
        const Vec4d vec4(1, 2, 4, 4);
        const Vec4d vec5(1, 3, 3, 4);
        const Vec4d vec6(2, 2, 3, 4);

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
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(1, 2, 3, 4);
        const Vec4i vec3(1, 2, 3, 5);
        const Vec4i vec4(1, 2, 4, 4);
        const Vec4i vec5(1, 3, 3, 4);
        const Vec4i vec6(2, 2, 3, 4);

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

TEST_CASE("Vector4 addition", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);

        Vec4f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 8.0f);;
        CHECK(vec3.w == 10.0f);;

        vec3 += vec1;
        CHECK(vec3.x == 5.0f);;
        CHECK(vec3.y == 8.0f);;
        CHECK(vec3.z == 11.0f);;
        CHECK(vec3.w == 14.0f);;
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);

        Vec4d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);;
        CHECK(vec3.y == 6.0);;
        CHECK(vec3.z == 8.0);;
        CHECK(vec3.w == 10.0);;

        vec3 += vec1;
        CHECK(vec3.x == 5.0);;
        CHECK(vec3.y == 8.0);;
        CHECK(vec3.z == 11.0);;
        CHECK(vec3.w == 14.0);;
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);

        Vec4i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);;
        CHECK(vec3.y == 6);;
        CHECK(vec3.z == 8);;
        CHECK(vec3.w == 10);;

        vec3 += vec1;
        CHECK(vec3.x == 5);;
        CHECK(vec3.y == 8);;
        CHECK(vec3.z == 11);;
        CHECK(vec3.w == 14);;
    }
}

TEST_CASE("Vector4 subtraction", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);

        Vec4f vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 2.0f);;
        CHECK(vec3.z == 2.0f);;
        CHECK(vec3.w == 2.0f);;

        vec3 -= vec1;
        CHECK(vec3.x == 1.0f);;
        CHECK(vec3.y == 0.0f);;
        CHECK(vec3.z == -1.0f);
        CHECK(vec3.w == -2.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);

        Vec4d vec3 = vec2 - vec1;
        CHECK(vec3.x == 2.0);;
        CHECK(vec3.y == 2.0);;
        CHECK(vec3.z == 2.0);;
        CHECK(vec3.w == 2.0);;

        vec3 -= vec1;
        CHECK(vec3.x == 1.0);;
        CHECK(vec3.y == 0.0);;
        CHECK(vec3.z == -1.0);
        CHECK(vec3.w == -2.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);

        Vec4i vec3 = vec2 - vec1;
        CHECK(vec3.x == 2);;
        CHECK(vec3.y == 2);;
        CHECK(vec3.z == 2);;
        CHECK(vec3.w == 2);;

        vec3 -= vec1;
        CHECK(vec3.x == 1);;
        CHECK(vec3.y == 0);;
        CHECK(vec3.z == -1);
        CHECK(vec3.w == -2);
    }
}

TEST_CASE("Vector4 multiplication by scalar", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);

        Vec4f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;
        CHECK(vec2.w == 20.0f);;

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;
        CHECK(vec2.w == 40.0f);;

        const Vec4f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);;
        CHECK(vec3.y == 40.0f);;
        CHECK(vec3.z == 60.0f);;
        CHECK(vec3.w == 80.0f);;
    }
    {
        const Vec4d vec1(1, 2, 3, 4);

        Vec4d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        const Vec4d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
        CHECK(vec3.z == 60.0);
        CHECK(vec3.w == 80.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);

        Vec4i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        const Vec4i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
        CHECK(vec3.z == 60.0);
        CHECK(vec3.w == 80.0);
    }
}

TEST_CASE("Vector4 multiplication", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        Vec4f vec2(2, 3, 4, 5);

        const Vec4f vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0f);;
        CHECK(vec3.y == 6.0f);;
        CHECK(vec3.z == 12.0f);;
        CHECK(vec3.w == 20.0f);;

        vec2 *= vec1;
        CHECK(vec2.x == 2.0f);;
        CHECK(vec2.y == 6.0f);;
        CHECK(vec2.z == 12.0f);;
        CHECK(vec2.w == 20.0f);;
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        Vec4d vec2(2, 3, 4, 5);

        const Vec4d vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0);
        CHECK(vec3.y == 6.0);
        CHECK(vec3.z == 12.0);
        CHECK(vec3.w == 20.0);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0);
        CHECK(vec2.y == 6.0);
        CHECK(vec2.z == 12.0);
        CHECK(vec2.w == 20.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(2, 3, 4, 5);

        const Vec4i vec3 = vec1 * vec2;
        CHECK(vec3.x == 2.0);
        CHECK(vec3.y == 6.0);
        CHECK(vec3.z == 12.0);
        CHECK(vec3.w == 20.0);

        vec2 *= vec1;
        CHECK(vec2.x == 2.0);
        CHECK(vec2.y == 6.0);
        CHECK(vec2.z == 12.0);
        CHECK(vec2.w == 20.0);
    }
}

TEST_CASE("Vector4 division by scalar", "[math][vector4]")
{
    {
        const Vec4f vec1(20.0f, 40.0f, 60.0f, 80);

        Vec4f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);;
        CHECK(vec2.y == 20.0f);;
        CHECK(vec2.z == 30.0f);;
        CHECK(vec2.w == 40.0f);;

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);;
        CHECK(vec2.y == 10.0f);;
        CHECK(vec2.z == 15.0f);;
        CHECK(vec2.w == 20.0f);;
    }
    {
        const Vec4d vec1(20.0, 40.0, 60.0, 80.0);

        Vec4d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);
    }
    {
        const Vec4i vec1(20, 40, 60, 80);

        Vec4i vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);
    }
}

TEST_CASE("Vector4 negation", "[math][vector4]")
{
    {
        const Vec4f vec(5, -10, -15, 20);
        const Vec4f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);;
        CHECK(neg.z == 15.0f);;
        CHECK(neg.w == -20.0f);
    }
    {
        const Vec4d vec(5, -10, -15, 20);
        const Vec4d neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
        CHECK(neg.z == 15.0);
        CHECK(neg.w == -20.0);
    }
    {
        const Vec4i vec(5, -10, -15, 20);
        const Vec4i neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
        CHECK(neg.z == 15.0);
        CHECK(neg.w == -20.0);
    }
}

TEST_CASE("Vector4 abs", "[math][vector4]")
{
    {
        const Vec4f vec(5, -10, -15, 20);
        const Vec4f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
        CHECK(a.w == 20.0f);
    }
    {
        const Vec4d vec(5, -10, -15, 20);
        const Vec4d a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
        CHECK(a.z == 15.0);
        CHECK(a.w == 20.0);
    }
    {
        const Vec4i vec(5, -10, -15, 20);
        const Vec4i a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
        CHECK(a.z == 15.0);
        CHECK(a.w == 20.0);
    }
}

TEST_CASE("Vector4 length", "[math][vector4]")
{
    {
        const Vec4f v1(3, 4, 5, 6);

        CHECK(Opal::Length(v1) == std::sqrt(86.0));
        CHECK(Opal::LengthSquared(v1) == 86.0f);
    }
    {
        const Vec4d v1(3, 4, 5, 6);

        CHECK(Opal::Length(v1) == std::sqrt(86.0));
        CHECK(Opal::LengthSquared(v1) == 86);
    }
    {
        const Vec4i v1(3, 4, 5, 6);

        CHECK(Opal::Length(v1) == std::sqrt(86.0));
        CHECK(Opal::LengthSquared(v1) == 86);
    }
}

TEST_CASE("Vector4 lerp", "[math][vector4]")
{
    {
        const Vec4f v1(1, 2, 3, 4);
        const Vec4f v2(3, 4, 5, 6);

        const Vec4f v3 = Opal::Lerp(0.5f, v1, v2);
        CHECK(v3.x == 2.0f);;
        CHECK(v3.y == 3.0f);;
        CHECK(v3.z == 4.0f);;
        CHECK(v3.w == 5.0f);;
    }
    {
        const Vec4d v1(1, 2, 3, 4);
        const Vec4d v2(3, 4, 5, 6);

        const Vec4d v3 = Opal::Lerp(0.5, v1, v2);
        CHECK(v3.x == 2.0);
        CHECK(v3.y == 3.0);
        CHECK(v3.z == 4.0);
        CHECK(v3.w == 5.0);
    }
}

TEST_CASE("Vector4 dot product", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);
        const Vec4f vec3(-3, -4, -5, -6);

        CHECK(Dot(vec1, vec2) == 50.0f);
        CHECK(AbsDot(vec1, vec3) == 50.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);
        const Vec4d vec3(-3, -4, -5, -6);

        CHECK(Dot(vec1, vec2) == 50.0);
        CHECK(AbsDot(vec1, vec3) == 50.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);
        const Vec4i vec3(-3, -4, -5, -6);

        CHECK(Dot(vec1, vec2) == 50.0);
        CHECK(AbsDot(vec1, vec3) == 50.0);
    }
}

TEST_CASE("Vector4 normalize", "[math][vector4]")
{
    {
        const Vec4f vec(2, 3, 5, 6);
        const Vec4f norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
    {
        const Vec4d vec(2, 3, 5, 6);
        const Vec4d norm = Normalize(vec);
        CHECK(Opal::IsEqual(Opal::Length(norm), 1.0, 0.00001));
    }
}

TEST_CASE("Vector4 misc", "[math][vector4]")
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, -2, 5, -3);
        const Vec4f min = Min(vec1, vec2);
        const Vec4f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);;
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);;
        CHECK(min.w == -3.0f);
        CHECK(max.x == 3.0f);;
        CHECK(max.y == 2.0f);;
        CHECK(max.z == 5.0f);;
        CHECK(max.w == 4.0f);;

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);;
        CHECK(perm.y == 3.0f);;
        CHECK(perm.z == 4.0f);;
        CHECK(perm.w == 1.0f);;
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, -2, 5, -3);
        const Vec4d min = Min(vec1, vec2);
        const Vec4d max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(min.z == 3.0);
        CHECK(min.w == -3.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);
        CHECK(max.z == 5.0);
        CHECK(max.w == 4.0);

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 4.0f);
        CHECK(perm.w == 1.0f);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, -2, 5, -3);
        const Vec4i min = Min(vec1, vec2);
        const Vec4i max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(min.z == 3.0);
        CHECK(min.w == -3.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);
        CHECK(max.z == 5.0);
        CHECK(max.w == 4.0);

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);;
        CHECK(perm.y == 3.0f);;
        CHECK(perm.z == 4.0f);;
        CHECK(perm.w == 1.0f);;
    }
}

TEST_CASE("Vector4 clamp", "[math][vector4]")
{
    {
        const Vec4f v1(2, -5, 10, -4);
        const Vec4f v2 = Opal::Clamp(v1, 0.0f, 5.0f);
        CHECK(v2.x == 2.0f);
        CHECK(v2.y == 0.0f);
        CHECK(v2.z == 5.0f);
        CHECK(v2.w == 0.0f);
    }
    {
        const Vec4d v1(2, -5, 10, -4);
        const Vec4d v2 = Opal::Clamp(v1, 0.0, 5.0);
        CHECK(v2.x == 2);
        CHECK(v2.y == 0);
        CHECK(v2.z == 5);
        CHECK(v2.w == 0);
    }
    {
        const Vec4i v1(2, -5, 10, -4);
        const Vec4i v2 = Opal::Clamp(v1, 0, 5);
        CHECK(v2.x == 2);
        CHECK(v2.y == 0);
        CHECK(v2.z == 5);
        CHECK(v2.w == 0);
    }
}

TEST_CASE("Vector4 comparison with IsEqual", "[math][vector4]")
{
    {
        const Vec4f v1(1, 2, 3, 4);
        const Vec4f v2(2, 3, 4, 5);
        const Vec4f v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Vec4d v1(1, 2, 3, 4);
        const Vec4d v2(2, 3, 4, 5);
        const Vec4d v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Vec4i v1(1, 2, 3, 4);
        const Vec4i v2(2, 3, 4, 5);
        const Vec4i v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Vector4 min and max component", "[math][vector4]")
{
    {
        const Vec4f v(1, 2, 3, 4);
        CHECK(Opal::MinComponent(v) == 1.0f);
        CHECK(Opal::MaxComponent(v) == 4.0f);
    }
    {
        const Vec4d v(1, 2, 3, 4);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 4);
    }
    {
        const Vec4i v(1, 2, 3, 4);
        CHECK(Opal::MinComponent(v) == 1);
        CHECK(Opal::MaxComponent(v) == 4);
    }
}

TEST_CASE("Vector4 min and max dimensions", "[math][vector4]")
{
    {
        const Vec4f v(1, 2, 3, 4);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 3);
    }
    {
        const Vec4d v(1, 2, 3, 4);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 3);
    }
    {
        const Vec4i v(1, 2, 3, 4);
        CHECK(Opal::MinDimension(v) == 0);
        CHECK(Opal::MaxDimension(v) == 3);
    }
}

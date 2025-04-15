#include <gtest/gtest.h>

#include "math/vector4.h"

using Vec4f = Math::Vector4<float>;
using Vec4d = Math::Vector4<double>;
using Vec4i = Math::Vector4<int>;

TEST(Vector4Tests, Construction)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);
        EXPECT_EQ(vec1.w, 4);

        const Vec4f vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
        EXPECT_EQ(vec2.w, 1);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);
        EXPECT_EQ(vec1.w, 4);

        const Vec4d vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
        EXPECT_EQ(vec2.w, 1);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);
        EXPECT_EQ(vec1.w, 4);

        const Vec4i vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
        EXPECT_EQ(vec2.w, 1);
    }
}

TEST(Vector4Tests, ContainsNonFinite)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNonFinite(vec1));

        const Vec4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec2));

        const Vec4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec3));

        const Vec4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec4));

        const Vec4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNonFinite(vec1));

        const Vec4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec2));

        const Vec4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec3));

        const Vec4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec4));

        const Vec4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNonFinite(vec1));
    }
}

TEST(Vector4Tests, ContainsNaN)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNaN(vec1));

        const Vec4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        EXPECT_FALSE(ContainsNaN(vec2));

        const Vec4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        EXPECT_FALSE(ContainsNaN(vec3));

        const Vec4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec4));

        const Vec4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNaN(vec1));

        const Vec4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        EXPECT_FALSE(ContainsNaN(vec2));

        const Vec4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        EXPECT_FALSE(ContainsNaN(vec3));

        const Vec4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec4));

        const Vec4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec5));
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        EXPECT_FALSE(ContainsNaN(vec1));
    }
}

TEST(Vector4Tests, Comparisons)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(1, 2, 3, 4);
        const Vec4f vec3(1, 2, 3, 5);
        const Vec4f vec4(1, 2, 4, 4);
        const Vec4f vec5(1, 3, 3, 4);
        const Vec4f vec6(2, 2, 3, 4);

        EXPECT_TRUE(vec1 == vec2);
        EXPECT_FALSE(vec1 == vec3);
        EXPECT_FALSE(vec1 == vec4);
        EXPECT_FALSE(vec1 == vec5);
        EXPECT_FALSE(vec1 == vec6);

        EXPECT_FALSE(vec1 != vec2);
        EXPECT_TRUE(vec1 != vec3);
        EXPECT_TRUE(vec1 != vec4);
        EXPECT_TRUE(vec1 != vec5);
        EXPECT_TRUE(vec1 != vec6);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(1, 2, 3, 4);
        const Vec4d vec3(1, 2, 3, 5);
        const Vec4d vec4(1, 2, 4, 4);
        const Vec4d vec5(1, 3, 3, 4);
        const Vec4d vec6(2, 2, 3, 4);

        EXPECT_TRUE(vec1 == vec2);
        EXPECT_FALSE(vec1 == vec3);
        EXPECT_FALSE(vec1 == vec4);
        EXPECT_FALSE(vec1 == vec5);
        EXPECT_FALSE(vec1 == vec6);

        EXPECT_FALSE(vec1 != vec2);
        EXPECT_TRUE(vec1 != vec3);
        EXPECT_TRUE(vec1 != vec4);
        EXPECT_TRUE(vec1 != vec5);
        EXPECT_TRUE(vec1 != vec6);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(1, 2, 3, 4);
        const Vec4i vec3(1, 2, 3, 5);
        const Vec4i vec4(1, 2, 4, 4);
        const Vec4i vec5(1, 3, 3, 4);
        const Vec4i vec6(2, 2, 3, 4);

        EXPECT_TRUE(vec1 == vec2);
        EXPECT_FALSE(vec1 == vec3);
        EXPECT_FALSE(vec1 == vec4);
        EXPECT_FALSE(vec1 == vec5);
        EXPECT_FALSE(vec1 == vec6);

        EXPECT_FALSE(vec1 != vec2);
        EXPECT_TRUE(vec1 != vec3);
        EXPECT_TRUE(vec1 != vec4);
        EXPECT_TRUE(vec1 != vec5);
        EXPECT_TRUE(vec1 != vec6);
    }
}

TEST(Vector4Tests, Addition)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);

        Vec4f vec3 = vec1 + vec2;
        EXPECT_FLOAT_EQ(vec3.x, 4.0f);
        EXPECT_FLOAT_EQ(vec3.y, 6.0f);
        EXPECT_FLOAT_EQ(vec3.z, 8.0f);
        EXPECT_FLOAT_EQ(vec3.w, 10.0f);

        vec3 += vec1;
        EXPECT_FLOAT_EQ(vec3.x, 5.0f);
        EXPECT_FLOAT_EQ(vec3.y, 8.0f);
        EXPECT_FLOAT_EQ(vec3.z, 11.0f);
        EXPECT_FLOAT_EQ(vec3.w, 14.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);

        Vec4d vec3 = vec1 + vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 4.0);
        EXPECT_DOUBLE_EQ(vec3.y, 6.0);
        EXPECT_DOUBLE_EQ(vec3.z, 8.0);
        EXPECT_DOUBLE_EQ(vec3.w, 10.0);

        vec3 += vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 5.0);
        EXPECT_DOUBLE_EQ(vec3.y, 8.0);
        EXPECT_DOUBLE_EQ(vec3.z, 11.0);
        EXPECT_DOUBLE_EQ(vec3.w, 14.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);

        Vec4i vec3 = vec1 + vec2;
        EXPECT_EQ(vec3.x, 4);
        EXPECT_EQ(vec3.y, 6);
        EXPECT_EQ(vec3.z, 8);
        EXPECT_EQ(vec3.w, 10);

        vec3 += vec1;
        EXPECT_EQ(vec3.x, 5);
        EXPECT_EQ(vec3.y, 8);
        EXPECT_EQ(vec3.z, 11);
        EXPECT_EQ(vec3.w, 14);
    }
}

TEST(Vector4Tests, Subtraction)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);

        Vec4f vec3 = vec2 - vec1;
        EXPECT_FLOAT_EQ(vec3.x, 2.0f);
        EXPECT_FLOAT_EQ(vec3.y, 2.0f);
        EXPECT_FLOAT_EQ(vec3.z, 2.0f);
        EXPECT_FLOAT_EQ(vec3.w, 2.0f);

        vec3 -= vec1;
        EXPECT_FLOAT_EQ(vec3.x, 1.0f);
        EXPECT_FLOAT_EQ(vec3.y, 0.0f);
        EXPECT_FLOAT_EQ(vec3.z, -1.0f);
        EXPECT_FLOAT_EQ(vec3.w, -2.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);

        Vec4d vec3 = vec2 - vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 2.0);
        EXPECT_DOUBLE_EQ(vec3.y, 2.0);
        EXPECT_DOUBLE_EQ(vec3.z, 2.0);
        EXPECT_DOUBLE_EQ(vec3.w, 2.0);

        vec3 -= vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 1.0);
        EXPECT_DOUBLE_EQ(vec3.y, 0.0);
        EXPECT_DOUBLE_EQ(vec3.z, -1.0);
        EXPECT_DOUBLE_EQ(vec3.w, -2.0);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);

        Vec4i vec3 = vec2 - vec1;
        EXPECT_EQ(vec3.x, 2);
        EXPECT_EQ(vec3.y, 2);
        EXPECT_EQ(vec3.z, 2);
        EXPECT_EQ(vec3.w, 2);

        vec3 -= vec1;
        EXPECT_EQ(vec3.x, 1);
        EXPECT_EQ(vec3.y, 0);
        EXPECT_EQ(vec3.z, -1);
        EXPECT_EQ(vec3.w, -2);
    }
}

TEST(Vector4Tests, MultiplicationScalar)
{
    {
        const Vec4f vec1(1, 2, 3, 4);

        Vec4f vec2 = vec1 * 5.0f;

        EXPECT_FLOAT_EQ(vec2.x, 5.0f);
        EXPECT_FLOAT_EQ(vec2.y, 10.0f);
        EXPECT_FLOAT_EQ(vec2.z, 15.0f);
        EXPECT_FLOAT_EQ(vec2.w, 20.0f);

        vec2 *= 2.0f;
        EXPECT_FLOAT_EQ(vec2.x, 10.0f);
        EXPECT_FLOAT_EQ(vec2.y, 20.0f);
        EXPECT_FLOAT_EQ(vec2.z, 30.0f);
        EXPECT_FLOAT_EQ(vec2.w, 40.0f);

        const Vec4f vec3 = 2.0f * vec2;
        EXPECT_FLOAT_EQ(vec3.x, 20.0f);
        EXPECT_FLOAT_EQ(vec3.y, 40.0f);
        EXPECT_FLOAT_EQ(vec3.z, 60.0f);
        EXPECT_FLOAT_EQ(vec3.w, 80.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);

        Vec4d vec2 = vec1 * 5.0f;

        EXPECT_DOUBLE_EQ(vec2.x, 5.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 15.0f);
        EXPECT_DOUBLE_EQ(vec2.w, 20.0f);

        vec2 *= 2.0f;
        EXPECT_DOUBLE_EQ(vec2.x, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 20.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 30.0f);
        EXPECT_DOUBLE_EQ(vec2.w, 40.0f);

        const Vec4d vec3 = 2.0f * vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 20.0f);
        EXPECT_DOUBLE_EQ(vec3.y, 40.0f);
        EXPECT_DOUBLE_EQ(vec3.z, 60.0f);
        EXPECT_DOUBLE_EQ(vec3.w, 80.0f);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);

        Vec4i vec2 = vec1 * 5.0f;

        EXPECT_EQ(vec2.x, 5.0f);
        EXPECT_EQ(vec2.y, 10.0f);
        EXPECT_EQ(vec2.z, 15.0f);
        EXPECT_EQ(vec2.w, 20.0f);

        vec2 *= 2.0f;
        EXPECT_EQ(vec2.x, 10.0f);
        EXPECT_EQ(vec2.y, 20.0f);
        EXPECT_EQ(vec2.z, 30.0f);
        EXPECT_EQ(vec2.w, 40.0f);

        const Vec4i vec3 = 2.0f * vec2;
        EXPECT_EQ(vec3.x, 20.0f);
        EXPECT_EQ(vec3.y, 40.0f);
        EXPECT_EQ(vec3.z, 60.0f);
        EXPECT_EQ(vec3.w, 80.0f);
    }
}

TEST(Vector4Tests, Multiplication)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        Vec4f vec2(2, 3, 4, 5);

        const Vec4f vec3 = vec1 * vec2;
        EXPECT_FLOAT_EQ(vec3.x, 2.0f);
        EXPECT_FLOAT_EQ(vec3.y, 6.0f);
        EXPECT_FLOAT_EQ(vec3.z, 12.0f);
        EXPECT_FLOAT_EQ(vec3.w, 20.0f);

        vec2 *= vec1;
        EXPECT_FLOAT_EQ(vec2.x, 2.0f);
        EXPECT_FLOAT_EQ(vec2.y, 6.0f);
        EXPECT_FLOAT_EQ(vec2.z, 12.0f);
        EXPECT_FLOAT_EQ(vec2.w, 20.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        Vec4d vec2(2, 3, 4, 5);

        const Vec4d vec3 = vec1 * vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 2.0f);
        EXPECT_DOUBLE_EQ(vec3.y, 6.0f);
        EXPECT_DOUBLE_EQ(vec3.z, 12.0f);
        EXPECT_DOUBLE_EQ(vec3.w, 20.0f);

        vec2 *= vec1;
        EXPECT_DOUBLE_EQ(vec2.x, 2.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 6.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 12.0f);
        EXPECT_DOUBLE_EQ(vec2.w, 20.0f);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(2, 3, 4, 5);

        const Vec4i vec3 = vec1 * vec2;
        EXPECT_EQ(vec3.x, 2.0f);
        EXPECT_EQ(vec3.y, 6.0f);
        EXPECT_EQ(vec3.z, 12.0f);
        EXPECT_EQ(vec3.w, 20.0f);

        vec2 *= vec1;
        EXPECT_EQ(vec2.x, 2.0f);
        EXPECT_EQ(vec2.y, 6.0f);
        EXPECT_EQ(vec2.z, 12.0f);
        EXPECT_EQ(vec2.w, 20.0f);
    }
}

TEST(Vector4Tests, DivisionScalar)
{
    {
        const Vec4f vec1(20.0f, 40.0f, 60.0f, 80);

        Vec4f vec2 = vec1 / 2.0f;

        EXPECT_FLOAT_EQ(vec2.x, 10.0f);
        EXPECT_FLOAT_EQ(vec2.y, 20.0f);
        EXPECT_FLOAT_EQ(vec2.z, 30.0f);
        EXPECT_FLOAT_EQ(vec2.w, 40.0f);

        vec2 /= 2.0f;
        EXPECT_FLOAT_EQ(vec2.x, 5.0f);
        EXPECT_FLOAT_EQ(vec2.y, 10.0f);
        EXPECT_FLOAT_EQ(vec2.z, 15.0f);
        EXPECT_FLOAT_EQ(vec2.w, 20.0f);
    }
    {
        const Vec4d vec1(20.0f, 40.0f, 60.0f, 80);

        Vec4d vec2 = vec1 / 2.0f;

        EXPECT_DOUBLE_EQ(vec2.x, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 20.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 30.0f);
        EXPECT_DOUBLE_EQ(vec2.w, 40.0f);

        vec2 /= 2.0f;
        EXPECT_DOUBLE_EQ(vec2.x, 5.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 15.0f);
        EXPECT_DOUBLE_EQ(vec2.w, 20.0f);
    }
    {
        const Vec4i vec1(20, 40, 60, 80);

        Vec4i vec2 = vec1 / 2.0f;

        EXPECT_EQ(vec2.x, 10.0f);
        EXPECT_EQ(vec2.y, 20.0f);
        EXPECT_EQ(vec2.z, 30.0f);
        EXPECT_EQ(vec2.w, 40.0f);

        vec2 /= 2.0f;
        EXPECT_EQ(vec2.x, 5.0f);
        EXPECT_EQ(vec2.y, 10.0f);
        EXPECT_EQ(vec2.z, 15.0f);
        EXPECT_EQ(vec2.w, 20.0f);
    }
}

TEST(Vector4Tests, Negation)
{
    {
        const Vec4f vec(5, -10, -15, 20);
        const Vec4f neg = -vec;

        EXPECT_FLOAT_EQ(neg.x, -5.0f);
        EXPECT_FLOAT_EQ(neg.y, 10.0f);
        EXPECT_FLOAT_EQ(neg.z, 15.0f);
        EXPECT_FLOAT_EQ(neg.w, -20.0f);
    }
    {
        const Vec4d vec(5, -10, -15, 20);
        const Vec4d neg = -vec;

        EXPECT_DOUBLE_EQ(neg.x, -5.0f);
        EXPECT_DOUBLE_EQ(neg.y, 10.0f);
        EXPECT_DOUBLE_EQ(neg.z, 15.0f);
        EXPECT_DOUBLE_EQ(neg.w, -20.0f);
    }
    {
        const Vec4i vec(5, -10, -15, 20);
        const Vec4i neg = -vec;

        EXPECT_EQ(neg.x, -5.0f);
        EXPECT_EQ(neg.y, 10.0f);
        EXPECT_EQ(neg.z, 15.0f);
        EXPECT_EQ(neg.w, -20.0f);
    }
}

TEST(Vector4Tests, Abs)
{
    {
        const Vec4f vec(5, -10, -15, 20);
        const Vec4f a = Math::Abs(vec);

        EXPECT_FLOAT_EQ(a.x, 5.0f);
        EXPECT_FLOAT_EQ(a.y, 10.0f);
        EXPECT_FLOAT_EQ(a.z, 15.0f);
        EXPECT_FLOAT_EQ(a.w, 20.0f);
    }
    {
        const Vec4d vec(5, -10, -15, 20);
        const Vec4d a = Math::Abs(vec);

        EXPECT_DOUBLE_EQ(a.x, 5.0f);
        EXPECT_DOUBLE_EQ(a.y, 10.0f);
        EXPECT_DOUBLE_EQ(a.z, 15.0f);
        EXPECT_DOUBLE_EQ(a.w, 20.0f);
    }
    {
        const Vec4i vec(5, -10, -15, 20);
        const Vec4i a = Math::Abs(vec);

        EXPECT_EQ(a.x, 5.0f);
        EXPECT_EQ(a.y, 10.0f);
        EXPECT_EQ(a.z, 15.0f);
        EXPECT_EQ(a.w, 20.0f);
    }
}

TEST(Vector4Tests, Length)
{
    {
        const Vec4f v1(3, 4, 5, 6);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(86.0));
        EXPECT_FLOAT_EQ(Math::LengthSquared(v1), 86);
    }
    {
        const Vec4d v1(3, 4, 5, 6);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(86.0));
        EXPECT_DOUBLE_EQ(Math::LengthSquared(v1), 86);
    }
    {
        const Vec4i v1(3, 4, 5, 6);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(86.0));
        EXPECT_EQ(Math::LengthSquared(v1), 86);
    }
}

TEST(Vector4Tests, Lerp)
{
    {
        const Vec4f v1(1, 2, 3, 4);
        const Vec4f v2(3, 4, 5, 6);

        const Vec4f v3 = Math::Lerp(0.5f, v1, v2);
        EXPECT_FLOAT_EQ(v3.x, 2.0f);
        EXPECT_FLOAT_EQ(v3.y, 3.0f);
        EXPECT_FLOAT_EQ(v3.z, 4.0f);
        EXPECT_FLOAT_EQ(v3.w, 5.0f);
    }
    {
        const Vec4d v1(1, 2, 3, 4);
        const Vec4d v2(3, 4, 5, 6);

        const Vec4d v3 = Math::Lerp(0.5, v1, v2);
        EXPECT_DOUBLE_EQ(v3.x, 2.0f);
        EXPECT_DOUBLE_EQ(v3.y, 3.0f);
        EXPECT_DOUBLE_EQ(v3.z, 4.0f);
        EXPECT_DOUBLE_EQ(v3.w, 5.0f);
    }
}

TEST(Vector4Tests, Dot)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);
        const Vec4f vec3(-3, -4, -5, -6);

        EXPECT_FLOAT_EQ(Dot(vec1, vec2), 50.0f);
        EXPECT_FLOAT_EQ(AbsDot(vec1, vec3), 50.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);
        const Vec4d vec3(-3, -4, -5, -6);

        EXPECT_DOUBLE_EQ(Dot(vec1, vec2), 50.0f);
        EXPECT_DOUBLE_EQ(AbsDot(vec1, vec3), 50.0f);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);
        const Vec4i vec3(-3, -4, -5, -6);

        EXPECT_EQ(Dot(vec1, vec2), 50.0f);
        EXPECT_EQ(AbsDot(vec1, vec3), 50.0f);
    }
}

TEST(Vector4Tests, Normalize)
{
    {
        const Vec4f vec(2, 3, 5, 6);
        const Vec4f norm = Normalize(vec);
        EXPECT_TRUE(Math::IsEqual(Math::Length(norm), 1.0, 0.00001));
    }
    {
        const Vec4d vec(2, 3, 5, 6);
        const Vec4d norm = Normalize(vec);
        EXPECT_TRUE(Math::IsEqual(Math::Length(norm), 1.0, 0.00001));
    }
}

TEST(Vector4Tests, Misc)
{
    {
        const Vec4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, -2, 5, -3);
        const Vec4f min = Min(vec1, vec2);
        const Vec4f max = Max(vec1, vec2);

        EXPECT_FLOAT_EQ(min.x, 1.0f);
        EXPECT_FLOAT_EQ(min.y, -2.0f);
        EXPECT_FLOAT_EQ(min.z, 3.0f);
        EXPECT_FLOAT_EQ(min.w, -3.0f);
        EXPECT_FLOAT_EQ(max.x, 3.0f);
        EXPECT_FLOAT_EQ(max.y, 2.0f);
        EXPECT_FLOAT_EQ(max.z, 5.0f);
        EXPECT_FLOAT_EQ(max.w, 4.0f);

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Math::Permute(vec, 1, 2, 3, 0);
        EXPECT_FLOAT_EQ(perm.x, 2.0f);
        EXPECT_FLOAT_EQ(perm.y, 3.0f);
        EXPECT_FLOAT_EQ(perm.z, 4.0f);
        EXPECT_FLOAT_EQ(perm.w, 1.0f);
    }
    {
        const Vec4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, -2, 5, -3);
        const Vec4d min = Min(vec1, vec2);
        const Vec4d max = Max(vec1, vec2);

        EXPECT_DOUBLE_EQ(min.x, 1.0f);
        EXPECT_DOUBLE_EQ(min.y, -2.0f);
        EXPECT_DOUBLE_EQ(min.z, 3.0f);
        EXPECT_DOUBLE_EQ(min.w, -3.0f);
        EXPECT_DOUBLE_EQ(max.x, 3.0f);
        EXPECT_DOUBLE_EQ(max.y, 2.0f);
        EXPECT_DOUBLE_EQ(max.z, 5.0f);
        EXPECT_DOUBLE_EQ(max.w, 4.0f);

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Math::Permute(vec, 1, 2, 3, 0);
        EXPECT_DOUBLE_EQ(perm.x, 2.0f);
        EXPECT_DOUBLE_EQ(perm.y, 3.0f);
        EXPECT_DOUBLE_EQ(perm.z, 4.0f);
        EXPECT_DOUBLE_EQ(perm.w, 1.0f);
    }
    {
        const Vec4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, -2, 5, -3);
        const Vec4i min = Min(vec1, vec2);
        const Vec4i max = Max(vec1, vec2);

        EXPECT_EQ(min.x, 1.0f);
        EXPECT_EQ(min.y, -2.0f);
        EXPECT_EQ(min.z, 3.0f);
        EXPECT_EQ(min.w, -3.0f);
        EXPECT_EQ(max.x, 3.0f);
        EXPECT_EQ(max.y, 2.0f);
        EXPECT_EQ(max.z, 5.0f);
        EXPECT_EQ(max.w, 4.0f);

        const Vec4f vec(1, 2, 3, 4);
        const Vec4f perm = Math::Permute(vec, 1, 2, 3, 0);
        EXPECT_EQ(perm.x, 2.0f);
        EXPECT_EQ(perm.y, 3.0f);
        EXPECT_EQ(perm.z, 4.0f);
        EXPECT_EQ(perm.w, 1.0f);
    }
}

TEST(Vector4Tests, Clamp)
{
    {
        const Vec4f v1(2, -5, 10, -4);
        const Vec4f v2 = Math::Clamp(v1, 0.0f, 5.0f);
        EXPECT_FLOAT_EQ(v2.x, 2);
        EXPECT_FLOAT_EQ(v2.y, 0);
        EXPECT_FLOAT_EQ(v2.z, 5);
        EXPECT_FLOAT_EQ(v2.w, 0);
    }
    {
        const Vec4d v1(2, -5, 10, -4);
        const Vec4d v2 = Math::Clamp(v1, 0.0, 5.0);
        EXPECT_DOUBLE_EQ(v2.x, 2);
        EXPECT_DOUBLE_EQ(v2.y, 0);
        EXPECT_DOUBLE_EQ(v2.z, 5);
        EXPECT_DOUBLE_EQ(v2.w, 0);
    }
    {
        const Vec4i v1(2, -5, 10, -4);
        const Vec4i v2 = Math::Clamp(v1, 0, 5);
        EXPECT_EQ(v2.x, 2);
        EXPECT_EQ(v2.y, 0);
        EXPECT_EQ(v2.z, 5);
        EXPECT_EQ(v2.w, 0);
    }
}

TEST(Vector4Tests, IsEqual)
{
    {
        const Vec4f v1(1, 2, 3, 4);
        const Vec4f v2(2, 3, 4, 5);
        const Vec4f v3(0, 1, 2, 3);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1.0f));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2.0f));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0.5f));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1.0f));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2.0f));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0.5f));
    }
    {
        const Vec4d v1(1, 2, 3, 4);
        const Vec4d v2(2, 3, 4, 5);
        const Vec4d v3(0, 1, 2, 3);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1.0));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2.0));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0.5));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1.0));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2.0));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0.5));
    }
    {
        const Vec4i v1(1, 2, 3, 4);
        const Vec4i v2(2, 3, 4, 5);
        const Vec4i v3(0, 1, 2, 3);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0));
    }
}

TEST(Vector4Tests, MinAndMaxComponent)
{
    {
        const Vec4f v(1, 2, 3, 4);
        EXPECT_FLOAT_EQ(Math::MinComponent(v), 1);
        EXPECT_FLOAT_EQ(Math::MaxComponent(v), 4);
    }
    {
        const Vec4d v(1, 2, 3, 4);
        EXPECT_DOUBLE_EQ(Math::MinComponent(v), 1);
        EXPECT_DOUBLE_EQ(Math::MaxComponent(v), 4);
    }
    {
        const Vec4i v(1, 2, 3, 4);
        EXPECT_EQ(Math::MinComponent(v), 1);
        EXPECT_EQ(Math::MaxComponent(v), 4);
    }
}

TEST(Vector4Tests, MinAndMaxDimension)
{
    {
        const Vec4f v(1, 2, 3, 4);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 3);
    }
    {
        const Vec4d v(1, 2, 3, 4);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 3);
    }
    {
        const Vec4i v(1, 2, 3, 4);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 3);
    }
}

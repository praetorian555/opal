#include <gtest/gtest.h>

#include "math/vector3.h"

using Vec3f = Math::Vector3<float>;
using Vec3d = Math::Vector3<double>;
using Vec3i = Math::Vector3<int>;

TEST(Vector3Tests, Construction)
{
    {
        const Vec3f vec1(1, 2, 3);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);

        const Vec3f vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
    }
    {
        const Vec3d vec1(1, 2, 3);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);

        const Vec3d vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
    }
    {
        const Vec3i vec1(1, 2, 3);
        EXPECT_EQ(vec1.x, 1);
        EXPECT_EQ(vec1.y, 2);
        EXPECT_EQ(vec1.z, 3);

        const Vec3i vec2(1);
        EXPECT_EQ(vec2.x, 1);
        EXPECT_EQ(vec2.y, 1);
        EXPECT_EQ(vec2.z, 1);
    }
}

TEST(Vector3Tests, ContainsNonFinite)
{
    {
        const Vec3f vec1(1, 2, 3);
        EXPECT_FALSE(ContainsNonFinite(vec1));

        const Vec3f vec2(1, 2, std::numeric_limits<float>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec2));

        const Vec3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec3));

        const Vec3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec4));

        const Vec3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        EXPECT_FALSE(ContainsNonFinite(vec1));

        const Vec3d vec2(1, 2, std::numeric_limits<double>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec2));

        const Vec3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        EXPECT_TRUE(ContainsNonFinite(vec3));

        const Vec3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec4));

        const Vec3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNonFinite(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        EXPECT_FALSE(ContainsNonFinite(vec1));
    }
}

TEST(Vector3Tests, ContainsNaN)
{
    {
        const Vec3f vec1(1, 2, 3);
        EXPECT_FALSE(ContainsNaN(vec1));

        const Vec3f vec2(1, 2, std::numeric_limits<float>::infinity());
        EXPECT_FALSE(ContainsNaN(vec2));

        const Vec3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        EXPECT_FALSE(ContainsNaN(vec3));

        const Vec3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec4));

        const Vec3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec5));
    }
    {
        const Vec3d vec1(1, 2, 3);
        EXPECT_FALSE(ContainsNaN(vec1));

        const Vec3d vec2(1, 2, std::numeric_limits<double>::infinity());
        EXPECT_FALSE(ContainsNaN(vec2));

        const Vec3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        EXPECT_FALSE(ContainsNaN(vec3));

        const Vec3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec4));

        const Vec3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        EXPECT_TRUE(ContainsNaN(vec5));
    }
    {
        const Vec3d vec1(1, 2, 4);
        EXPECT_FALSE(ContainsNaN(vec1));
    }
}

TEST(Vector3Tests, Comparisons)
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(1, 2, 3);
        const Vec3f vec3(1, 2, 4);
        const Vec3f vec4(1, 2, 4);
        const Vec3f vec5(1, 3, 3);
        const Vec3f vec6(2, 2, 3);

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
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(1, 2, 3);
        const Vec3d vec3(1, 2, 4);
        const Vec3d vec4(1, 2, 4);
        const Vec3d vec5(1, 3, 3);
        const Vec3d vec6(2, 2, 3);

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
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(1, 2, 3);
        const Vec3i vec3(1, 2, 4);
        const Vec3i vec4(1, 2, 4);
        const Vec3i vec5(1, 3, 3);
        const Vec3i vec6(2, 2, 3);

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

TEST(Vector3Tests, Addition)
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);

        Vec3f vec3 = vec1 + vec2;
        EXPECT_FLOAT_EQ(vec3.x, 4.0f);
        EXPECT_FLOAT_EQ(vec3.y, 6.0f);
        EXPECT_FLOAT_EQ(vec3.z, 8.0f);

        vec3 += vec1;
        EXPECT_FLOAT_EQ(vec3.x, 5.0f);
        EXPECT_FLOAT_EQ(vec3.y, 8.0f);
        EXPECT_FLOAT_EQ(vec3.z, 11.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);

        Vec3d vec3 = vec1 + vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 4.0);
        EXPECT_DOUBLE_EQ(vec3.y, 6.0);
        EXPECT_DOUBLE_EQ(vec3.z, 8.0);

        vec3 += vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 5.0);
        EXPECT_DOUBLE_EQ(vec3.y, 8.0);
        EXPECT_DOUBLE_EQ(vec3.z, 11.0);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);

        Vec3i vec3 = vec1 + vec2;
        EXPECT_EQ(vec3.x, 4);
        EXPECT_EQ(vec3.y, 6);
        EXPECT_EQ(vec3.z, 8);

        vec3 += vec1;
        EXPECT_EQ(vec3.x, 5);
        EXPECT_EQ(vec3.y, 8);
        EXPECT_EQ(vec3.z, 11);
    }
}

TEST(Vector3Tests, Subtraction)
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);

        Vec3f vec3 = vec2 - vec1;
        EXPECT_FLOAT_EQ(vec3.x, 2.0f);
        EXPECT_FLOAT_EQ(vec3.y, 2.0f);
        EXPECT_FLOAT_EQ(vec3.z, 2.0f);

        vec3 -= vec1;
        EXPECT_FLOAT_EQ(vec3.x, 1.0f);
        EXPECT_FLOAT_EQ(vec3.y, 0.0f);
        EXPECT_FLOAT_EQ(vec3.z, -1.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);

        Vec3d vec3 = vec2 - vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 2.0);
        EXPECT_DOUBLE_EQ(vec3.y, 2.0);
        EXPECT_DOUBLE_EQ(vec3.z, 2.0);

        vec3 -= vec1;
        EXPECT_DOUBLE_EQ(vec3.x, 1.0);
        EXPECT_DOUBLE_EQ(vec3.y, 0.0);
        EXPECT_DOUBLE_EQ(vec3.z, -1.0);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);

        Vec3i vec3 = vec2 - vec1;
        EXPECT_EQ(vec3.x, 2);
        EXPECT_EQ(vec3.y, 2);
        EXPECT_EQ(vec3.z, 2);

        vec3 -= vec1;
        EXPECT_EQ(vec3.x, 1);
        EXPECT_EQ(vec3.y, 0);
        EXPECT_EQ(vec3.z, -1);
    }
}

TEST(Vector3Tests, MultiplicationScalar)
{
    {
        const Vec3f vec1(1, 2, 3);

        Vec3f vec2 = vec1 * 5.0f;

        EXPECT_FLOAT_EQ(vec2.x, 5.0f);
        EXPECT_FLOAT_EQ(vec2.y, 10.0f);
        EXPECT_FLOAT_EQ(vec2.z, 15.0f);

        vec2 *= 2.0f;
        EXPECT_FLOAT_EQ(vec2.x, 10.0f);
        EXPECT_FLOAT_EQ(vec2.y, 20.0f);
        EXPECT_FLOAT_EQ(vec2.z, 30.0f);

        const Vec3f vec3 = 2.0f * vec2;
        EXPECT_FLOAT_EQ(vec3.x, 20.0f);
        EXPECT_FLOAT_EQ(vec3.y, 40.0f);
        EXPECT_FLOAT_EQ(vec3.z, 60.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);

        Vec3d vec2 = vec1 * 5.0f;

        EXPECT_DOUBLE_EQ(vec2.x, 5.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 15.0f);

        vec2 *= 2.0f;
        EXPECT_DOUBLE_EQ(vec2.x, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 20.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 30.0f);

        const Vec3d vec3 = 2.0f * vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 20.0f);
        EXPECT_DOUBLE_EQ(vec3.y, 40.0f);
        EXPECT_DOUBLE_EQ(vec3.z, 60.0f);
    }
    {
        const Vec3i vec1(1, 2, 3);

        Vec3i vec2 = vec1 * 5.0f;

        EXPECT_EQ(vec2.x, 5.0f);
        EXPECT_EQ(vec2.y, 10.0f);
        EXPECT_EQ(vec2.z, 15.0f);

        vec2 *= 2.0f;
        EXPECT_EQ(vec2.x, 10.0f);
        EXPECT_EQ(vec2.y, 20.0f);
        EXPECT_EQ(vec2.z, 30.0f);

        const Vec3i vec3 = 2.0f * vec2;
        EXPECT_EQ(vec3.x, 20.0f);
        EXPECT_EQ(vec3.y, 40.0f);
        EXPECT_EQ(vec3.z, 60.0f);
    }
}

TEST(Vector3Tests, Multiplication)
{
    {
        const Vec3f vec1(1, 2, 3);
        Vec3f vec2(2, 3, 4);

        const Vec3f vec3 = vec1 * vec2;
        EXPECT_FLOAT_EQ(vec3.x, 2.0f);
        EXPECT_FLOAT_EQ(vec3.y, 6.0f);
        EXPECT_FLOAT_EQ(vec3.z, 12.0f);

        vec2 *= vec1;
        EXPECT_FLOAT_EQ(vec2.x, 2.0f);
        EXPECT_FLOAT_EQ(vec2.y, 6.0f);
        EXPECT_FLOAT_EQ(vec2.z, 12.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        Vec3d vec2(2, 3, 4);

        const Vec3d vec3 = vec1 * vec2;
        EXPECT_DOUBLE_EQ(vec3.x, 2.0f);
        EXPECT_DOUBLE_EQ(vec3.y, 6.0f);
        EXPECT_DOUBLE_EQ(vec3.z, 12.0f);

        vec2 *= vec1;
        EXPECT_DOUBLE_EQ(vec2.x, 2.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 6.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 12.0f);
    }
    {
        const Vec3i vec1(1, 2, 3);
        Vec3i vec2(2, 3, 4);

        const Vec3i vec3 = vec1 * vec2;
        EXPECT_EQ(vec3.x, 2.0f);
        EXPECT_EQ(vec3.y, 6.0f);
        EXPECT_EQ(vec3.z, 12.0f);

        vec2 *= vec1;
        EXPECT_EQ(vec2.x, 2.0f);
        EXPECT_EQ(vec2.y, 6.0f);
        EXPECT_EQ(vec2.z, 12.0f);
    }
}

TEST(Vector3Tests, DivisionScalar)
{
    {
        const Vec3f vec1(20.0f, 40.0f, 60.0f);

        Vec3f vec2 = vec1 / 2.0f;

        EXPECT_FLOAT_EQ(vec2.x, 10.0f);
        EXPECT_FLOAT_EQ(vec2.y, 20.0f);
        EXPECT_FLOAT_EQ(vec2.z, 30.0f);

        vec2 /= 2.0f;
        EXPECT_FLOAT_EQ(vec2.x, 5.0f);
        EXPECT_FLOAT_EQ(vec2.y, 10.0f);
        EXPECT_FLOAT_EQ(vec2.z, 15.0f);
    }
    {
        const Vec3d vec1(20.0f, 40.0f, 60.0f);

        Vec3d vec2 = vec1 / 2.0f;

        EXPECT_DOUBLE_EQ(vec2.x, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 20.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 30.0f);

        vec2 /= 2.0f;
        EXPECT_DOUBLE_EQ(vec2.x, 5.0f);
        EXPECT_DOUBLE_EQ(vec2.y, 10.0f);
        EXPECT_DOUBLE_EQ(vec2.z, 15.0f);
    }
    {
        const Vec3i vec1(20, 40, 60);

        Vec3i vec2 = vec1 / 2.0f;

        EXPECT_EQ(vec2.x, 10.0f);
        EXPECT_EQ(vec2.y, 20.0f);
        EXPECT_EQ(vec2.z, 30.0f);

        vec2 /= 2.0f;
        EXPECT_EQ(vec2.x, 5.0f);
        EXPECT_EQ(vec2.y, 10.0f);
        EXPECT_EQ(vec2.z, 15.0f);
    }
}

TEST(Vector3Tests, Negation)
{
    {
        const Vec3f vec(5, -10, -15);
        const Vec3f neg = -vec;

        EXPECT_FLOAT_EQ(neg.x, -5.0f);
        EXPECT_FLOAT_EQ(neg.y, 10.0f);
        EXPECT_FLOAT_EQ(neg.z, 15.0f);
    }
    {
        const Vec3d vec(5, -10, -15);
        const Vec3d neg = -vec;

        EXPECT_DOUBLE_EQ(neg.x, -5.0f);
        EXPECT_DOUBLE_EQ(neg.y, 10.0f);
        EXPECT_DOUBLE_EQ(neg.z, 15.0f);
    }
    {
        const Vec3i vec(5, -10, -15);
        const Vec3i neg = -vec;

        EXPECT_EQ(neg.x, -5.0f);
        EXPECT_EQ(neg.y, 10.0f);
        EXPECT_EQ(neg.z, 15.0f);
    }
}

TEST(Vector3Tests, Abs)
{
    {
        const Vec3f vec(5, -10, -15);
        const Vec3f a = Math::Abs(vec);

        EXPECT_FLOAT_EQ(a.x, 5.0f);
        EXPECT_FLOAT_EQ(a.y, 10.0f);
        EXPECT_FLOAT_EQ(a.z, 15.0f);
    }
    {
        const Vec3d vec(5, -10, -15);
        const Vec3d a = Math::Abs(vec);

        EXPECT_DOUBLE_EQ(a.x, 5.0f);
        EXPECT_DOUBLE_EQ(a.y, 10.0f);
        EXPECT_DOUBLE_EQ(a.z, 15.0f);
    }
    {
        const Vec3i vec(5, -10, -15);
        const Vec3i a = Math::Abs(vec);

        EXPECT_EQ(a.x, 5.0f);
        EXPECT_EQ(a.y, 10.0f);
        EXPECT_EQ(a.z, 15.0f);
    }
}

TEST(Vector3Tests, Length)
{
    {
        const Vec3f v1(3, 4, 5);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(50.0));
        EXPECT_FLOAT_EQ(Math::LengthSquared(v1), 50);
    }
    {
        const Vec3d v1(3, 4, 5);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(50.0));
        EXPECT_DOUBLE_EQ(Math::LengthSquared(v1), 50);
    }
    {
        const Vec3i v1(3, 4, 5);

        EXPECT_DOUBLE_EQ(Math::Length(v1), std::sqrt(50.0));
        EXPECT_EQ(Math::LengthSquared(v1), 50);
    }
}

TEST(Vector3Tests, Lerp)
{
    {
        const Vec3f v1(1, 2, 3);
        const Vec3f v2(3, 4, 5);

        const Vec3f v3 = Math::Lerp(0.5f, v1, v2);
        EXPECT_FLOAT_EQ(v3.x, 2.0f);
        EXPECT_FLOAT_EQ(v3.y, 3.0f);
        EXPECT_FLOAT_EQ(v3.z, 4.0f);
    }
    {
        const Vec3d v1(1, 2, 3);
        const Vec3d v2(3, 4, 5);

        const Vec3d v3 = Math::Lerp(0.5, v1, v2);
        EXPECT_DOUBLE_EQ(v3.x, 2.0f);
        EXPECT_DOUBLE_EQ(v3.y, 3.0f);
        EXPECT_DOUBLE_EQ(v3.z, 4.0f);
    }
}

TEST(Vector3Tests, Dot)
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);
        const Vec3f vec3(-3, -4, -5);

        EXPECT_FLOAT_EQ(Dot(vec1, vec2), 26.0f);
        EXPECT_FLOAT_EQ(AbsDot(vec1, vec3), 26.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);
        const Vec3d vec3(-3, -4, -5);

        EXPECT_DOUBLE_EQ(Dot(vec1, vec2), 26.0f);
        EXPECT_DOUBLE_EQ(AbsDot(vec1, vec3), 26.0f);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);
        const Vec3i vec3(-3, -4, -5);

        EXPECT_EQ(Dot(vec1, vec2), 26.0f);
        EXPECT_EQ(AbsDot(vec1, vec3), 26.0f);
    }
}

TEST(Vector3Tests, Cross)
{
    {
        const Vec3f x(1, 0, 0);
        const Vec3f y(0, 1, 0);
        const Vec3f z(0, 0, 1);

        const Vec3f c = Cross(x, y);
        EXPECT_TRUE(c == z);
    }
    {
        const Vec3d x(1, 0, 0);
        const Vec3d y(0, 1, 0);
        const Vec3d z(0, 0, 1);

        const Vec3d c = Cross(x, y);
        EXPECT_TRUE(c == z);
    }
    {
        const Vec3i x(1, 0, 0);
        const Vec3i y(0, 1, 0);
        const Vec3i z(0, 0, 1);

        const Vec3i c = Cross(x, y);
        EXPECT_TRUE(c == z);
    }
}

TEST(Vector2Tests, Cross2D)
{
    {
        const Vec3f vec1(0, 1, 1);
        const Vec3f vec2(1, 0, 2);

        EXPECT_EQ(-1, Cross2D(vec1, vec2));
    }
    {
        const Vec3d vec1(0, 1, 1);
        const Vec3d vec2(1, 0, 2);

        EXPECT_EQ(-1, Cross2D(vec1, vec2));
    }
    {
        const Vec3i vec1(0, 1, 1);
        const Vec3i vec2(1, 0, 2);

        EXPECT_EQ(-1, Cross2D(vec1, vec2));
    }
}

TEST(Vector3Tests, Normalize)
{
    {
        const Vec3f vec(2, 3, 5);
        const Vec3f norm = Normalize(vec);
        EXPECT_TRUE(Math::IsEqual(Math::Length(norm), 1.0, 0.00001));
    }
    {
        const Vec3d vec(2, 3, 5);
        const Vec3d norm = Normalize(vec);
        EXPECT_TRUE(Math::IsEqual(Math::Length(norm), 1.0, 0.00001));
    }
}

TEST(Vector3Tests, Misc)
{
    {
        const Vec3f vec1(1, 2, 3);
        const Vec3f vec2(3, -2, 5);
        const Vec3f min = Min(vec1, vec2);
        const Vec3f max = Max(vec1, vec2);

        EXPECT_FLOAT_EQ(min.x, 1.0f);
        EXPECT_FLOAT_EQ(min.y, -2.0f);
        EXPECT_FLOAT_EQ(min.z, 3.0f);
        EXPECT_FLOAT_EQ(max.x, 3.0f);
        EXPECT_FLOAT_EQ(max.y, 2.0f);
        EXPECT_FLOAT_EQ(max.z, 5.0f);

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Math::Permute(vec, 1, 2, 0);
        EXPECT_FLOAT_EQ(perm.x, 2.0f);
        EXPECT_FLOAT_EQ(perm.y, 3.0f);
        EXPECT_FLOAT_EQ(perm.z, 1.0f);
    }
    {
        const Vec3d vec1(1, 2, 3);
        const Vec3d vec2(3, -2, 5);
        const Vec3d min = Min(vec1, vec2);
        const Vec3d max = Max(vec1, vec2);

        EXPECT_DOUBLE_EQ(min.x, 1.0f);
        EXPECT_DOUBLE_EQ(min.y, -2.0f);
        EXPECT_DOUBLE_EQ(min.z, 3.0f);
        EXPECT_DOUBLE_EQ(max.x, 3.0f);
        EXPECT_DOUBLE_EQ(max.y, 2.0f);
        EXPECT_DOUBLE_EQ(max.z, 5.0f);

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Math::Permute(vec, 1, 2, 0);
        EXPECT_DOUBLE_EQ(perm.x, 2.0f);
        EXPECT_DOUBLE_EQ(perm.y, 3.0f);
        EXPECT_DOUBLE_EQ(perm.z, 1.0f);
    }
    {
        const Vec3i vec1(1, 2, 3);
        const Vec3i vec2(3, -2, 5);
        const Vec3i min = Min(vec1, vec2);
        const Vec3i max = Max(vec1, vec2);

        EXPECT_EQ(min.x, 1.0f);
        EXPECT_EQ(min.y, -2.0f);
        EXPECT_EQ(min.z, 3.0f);
        EXPECT_EQ(max.x, 3.0f);
        EXPECT_EQ(max.y, 2.0f);
        EXPECT_EQ(max.z, 5.0f);

        const Vec3f vec(1, 2, 3);
        const Vec3f perm = Math::Permute(vec, 1, 2, 0);
        EXPECT_EQ(perm.x, 2.0f);
        EXPECT_EQ(perm.y, 3.0f);
        EXPECT_EQ(perm.z, 1.0f);
    }
}

TEST(Vector3Tests, Clamp)
{
    {
        const Vec3f v1(2, -5, 10);
        const Vec3f v2 = Math::Clamp(v1, 0.0f, 5.0f);
        EXPECT_FLOAT_EQ(v2.x, 2);
        EXPECT_FLOAT_EQ(v2.y, 0);
        EXPECT_FLOAT_EQ(v2.z, 5);
    }
    {
        const Vec3d v1(2, -5, 10);
        const Vec3d v2 = Math::Clamp(v1, 0.0, 5.0);
        EXPECT_DOUBLE_EQ(v2.x, 2);
        EXPECT_DOUBLE_EQ(v2.y, 0);
        EXPECT_DOUBLE_EQ(v2.z, 5);
    }
    {
        const Vec3i v1(2, -5, 10);
        const Vec3i v2 = Math::Clamp(v1, 0, 5);
        EXPECT_EQ(v2.x, 2);
        EXPECT_EQ(v2.y, 0);
        EXPECT_EQ(v2.z, 5);
    }
}

TEST(Vector3Tests, Reflection)
{
    {
        const Vec3f incidence(1, 0, 0);
        const Vec3f normal(0, 1, 0);
        const Vec3f reflection = Math::Reflect(incidence, normal);
        EXPECT_FLOAT_EQ(reflection.x, -1.0f);
        EXPECT_FLOAT_EQ(reflection.y, 0.0f);
        EXPECT_FLOAT_EQ(reflection.z, 0.0f);
    }
    {
        const Vec3d incidence(1, 0, 0);
        const Vec3d normal(0, 1, 0);
        const Vec3d reflection = Math::Reflect(incidence, normal);
        EXPECT_DOUBLE_EQ(reflection.x, -1.0f);
        EXPECT_DOUBLE_EQ(reflection.y, 0.0f);
        EXPECT_DOUBLE_EQ(reflection.z, 0.0f);
    }
}

TEST(Vector3Tests, IsEqual)
{
    {
        const Vec3f v1(1, 2, 3);
        const Vec3f v2(2, 3, 4);
        const Vec3f v3(0, 1, 2);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1.0f));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2.0f));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0.5f));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1.0f));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2.0f));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0.5f));
    }
    {
        const Vec3d v1(1, 2, 3);
        const Vec3d v2(2, 3, 4);
        const Vec3d v3(0, 1, 2);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1.0));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2.0));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0.5));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1.0));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2.0));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0.5));
    }
    {
        const Vec3i v1(1, 2, 3);
        const Vec3i v2(2, 3, 4);
        const Vec3i v3(0, 1, 2);

        EXPECT_TRUE(Math::IsEqual(v1, v2, 1));
        EXPECT_TRUE(Math::IsEqual(v1, v2, 2));
        EXPECT_FALSE(Math::IsEqual(v1, v2, 0));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 1));
        EXPECT_TRUE(Math::IsEqual(v1, v3, 2));
        EXPECT_FALSE(Math::IsEqual(v1, v3, 0));
    }
}

TEST(Vector3Tests, MinAndMaxComponent)
{
    {
        const Vec3f v(1, 2, 3);
        EXPECT_FLOAT_EQ(Math::MinComponent(v), 1);
        EXPECT_FLOAT_EQ(Math::MaxComponent(v), 3);
    }
    {
        const Vec3d v(1, 2, 3);
        EXPECT_DOUBLE_EQ(Math::MinComponent(v), 1);
        EXPECT_DOUBLE_EQ(Math::MaxComponent(v), 3);
    }
    {
        const Vec3i v(1, 2, 3);
        EXPECT_EQ(Math::MinComponent(v), 1);
        EXPECT_EQ(Math::MaxComponent(v), 3);
    }
}

TEST(Vector3Tests, MinAndMaxDimension)
{
    {
        const Vec3f v(1, 2, 3);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 2);
    }
    {
        const Vec3d v(1, 2, 3);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 2);
    }
    {
        const Vec3i v(1, 2, 3);
        EXPECT_EQ(Math::MinDimension(v), 0);
        EXPECT_EQ(Math::MaxDimension(v), 2);
    }
}

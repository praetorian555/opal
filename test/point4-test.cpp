#include <limits>

#include "test-helpers.h"

#include "opal/math/point4.h"

using Point4f = Opal::Point4<float>;
using Point4d = Opal::Point4<double>;
using Point4i = Opal::Point4<int>;
using Vec4f = Opal::Vector4<float>;
using Vec4d = Opal::Vector4<double>;
using Vec4i = Opal::Vector4<int>;

TEST_CASE("Point4 constructor", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1.0f);
        CHECK(vec1.y == 2.0f);
        CHECK(vec1.z == 3.0f);
        CHECK(vec1.w == 4.0f);

        const Point4f vec2(1);
        CHECK(vec2.x == 1.0f);
        CHECK(vec2.y == 1.0f);
        CHECK(vec2.z == 1.0f);
        CHECK(vec2.w == 1.0f);
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);
        CHECK(vec1.w == 4);

        const Point4d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
        CHECK(vec2.w == 1);
    }
    {
        const Point4i vec1(1, 2, 3, 4);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);
        CHECK(vec1.w == 4);

        const Point4i vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
        CHECK(vec2.w == 1);
    }
}

TEST_CASE("Point4 access operator", "[math][point4]")
{
    {
        Point4f p1(1, 2, 3, 4);
        CHECK(p1[0] == 1.0f);
        CHECK(p1[1] == 2.0f);
        CHECK(p1[2] == 3.0f);
        CHECK(p1[3] == 4.0f);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        p1[3] = 8;
        CHECK(p1[0] == 5.0f);
        CHECK(p1[1] == 6.0f);
        CHECK(p1[2] == 7.0f);
        CHECK(p1[3] == 8.0f);
    }
    {
        Point4d p1(1, 2, 3, 4);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);
        CHECK(p1[2] == 3);
        CHECK(p1[3] == 4);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        p1[3] = 8;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
        CHECK(p1[2] == 7);
        CHECK(p1[3] == 8);
    }
    {
        Point4i p1(1, 2, 3, 4);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);
        CHECK(p1[2] == 3);
        CHECK(p1[3] == 4);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        p1[3] = 8;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
        CHECK(p1[2] == 7);
        CHECK(p1[3] == 8);
    }
}

TEST_CASE("Point4 non-finite", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Point4 NaN", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point4f vec2(1, 2, 3, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point4f vec3(1, 2, 3, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point4f vec4(1, 2, 3, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point4f vec5(1, 2, 3, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point4d vec2(1, 2, 3, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point4d vec3(1, 2, 3, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point4d vec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point4d vec5(1, 2, 3, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Point4 comparisons", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        const Point4f vec2(1, 2, 3, 4);
        const Point4f vec3(1, 2, 3, 5);
        const Point4f vec4(1, 2, 4, 4);
        const Point4f vec5(1, 3, 3, 4);
        const Point4f vec6(2, 2, 3, 4);

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
        const Point4d vec1(1, 2, 3, 4);
        const Point4d vec2(1, 2, 3, 4);
        const Point4d vec3(1, 2, 3, 5);
        const Point4d vec4(1, 2, 4, 4);
        const Point4d vec5(1, 3, 3, 4);
        const Point4d vec6(2, 2, 3, 4);

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
        const Point4i vec1(1, 2, 3, 4);
        const Point4i vec2(1, 2, 3, 4);
        const Point4i vec3(1, 2, 3, 5);
        const Point4i vec4(1, 2, 4, 4);
        const Point4i vec5(1, 3, 3, 4);
        const Point4i vec6(2, 2, 3, 4);

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

TEST_CASE("Point4 addition", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        const Vec4f vec2(3, 4, 5, 6);

        Point4f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);
        CHECK(vec3.y == 6.0f);
        CHECK(vec3.z == 8.0f);
        CHECK(vec3.w == 10.0f);

        vec3 += vec2;
        CHECK(vec3.x == 7.0f);
        CHECK(vec3.y == 10.0f);
        CHECK(vec3.z == 13.0f);
        CHECK(vec3.w == 16.0f);
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        const Vec4d vec2(3, 4, 5, 6);

        Point4d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);
        CHECK(vec3.y == 6.0);
        CHECK(vec3.z == 8.0);
        CHECK(vec3.w == 10.0);

        vec3 += vec2;
        CHECK(vec3.x == 7.0);
        CHECK(vec3.y == 10.0);
        CHECK(vec3.z == 13.0);
        CHECK(vec3.w == 16.0);
    }
    {
        const Point4i vec1(1, 2, 3, 4);
        const Vec4i vec2(3, 4, 5, 6);

        Point4i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);
        CHECK(vec3.y == 6);
        CHECK(vec3.z == 8);
        CHECK(vec3.w == 10);

        vec3 += vec2;
        CHECK(vec3.x == 7);
        CHECK(vec3.y == 10);
        CHECK(vec3.z == 13);
        CHECK(vec3.w == 16);
    }
}

TEST_CASE("Point4 subtraction", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        const Point4f vec2(3, 4, 5, 6);
        const Vec4f vec3(2, 2, 2, 2);

        const Point4f vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0f);
        CHECK(vec4.y == 2.0f);
        CHECK(vec4.z == 3.0f);
        CHECK(vec4.w == 4.0f);

        const Vec4f vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0f);
        CHECK(vec5.y == 2.0f);
        CHECK(vec5.z == 2.0f);
        CHECK(vec5.w == 2.0f);
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        const Point4d vec2(3, 4, 5, 6);
        const Vec4d vec3(2, 2, 2, 2);

        const Point4d vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0);
        CHECK(vec4.y == 2.0);
        CHECK(vec4.z == 3.0);
        CHECK(vec4.w == 4.0);

        const Vec4d vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0);
        CHECK(vec5.y == 2.0);
        CHECK(vec5.z == 2.0);
        CHECK(vec5.w == 2.0);
    }
    {
        const Point4i vec1(1, 2, 3, 4);
        const Point4i vec2(3, 4, 5, 6);
        const Vec4i vec3(2, 2, 2, 2);

        const Point4i vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0);
        CHECK(vec4.y == 2.0);
        CHECK(vec4.z == 3.0);
        CHECK(vec4.w == 4.0);

        const Vec4i vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0);
        CHECK(vec5.y == 2.0);
        CHECK(vec5.z == 2.0);
        CHECK(vec5.w == 2.0);
    }
}

TEST_CASE("Point4 negation", "[math][point4]")
{
    {
        const Point4f vec(5, -10, -15, 20);
        const Point4f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
        CHECK(neg.w == -20.0f);
    }
    {
        const Point4d vec(5, -10, -15, 20);
        const Point4d neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
        CHECK(neg.z == 15.0);
        CHECK(neg.w == -20.0);
    }
    {
        const Point4i vec(5, -10, -15, 20);
        const Point4i neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
        CHECK(neg.z == 15.0);
        CHECK(neg.w == -20.0);
    }
}

TEST_CASE("Point4 multiplication by scalar", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);

        Point4f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
        CHECK(vec2.w == 20.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);
        CHECK(vec2.w == 40.0f);

        const Point4f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
        CHECK(vec3.w == 80.0f);
    }
    {
        const Point4d vec1(1, 2, 3, 4);

        Point4d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        const Point4d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
        CHECK(vec3.z == 60.0);
        CHECK(vec3.w == 80.0);
    }
    {
        const Point4i vec1(1, 2, 3, 4);

        Point4i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
        CHECK(vec2.z == 15.0);
        CHECK(vec2.w == 20.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);
        CHECK(vec2.z == 30.0);
        CHECK(vec2.w == 40.0);

        const Point4i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
        CHECK(vec3.z == 60.0);
        CHECK(vec3.w == 80.0);
    }
}

TEST_CASE("Point4 division by scalar", "[math][point4]")
{
    {
        const Point4f vec1(20.0f, 40.0f, 60.0f, 80);

        Point4f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);
        CHECK(vec2.w == 40.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
        CHECK(vec2.w == 20.0f);
    }
    {
        const Point4d vec1(20.0, 40.0, 60.0, 80.0);

        Point4d vec2 = vec1 / 2.0f;

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
        const Point4i vec1(20, 40, 60, 80);

        Point4i vec2 = vec1 / 2.0f;

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

TEST_CASE("Point4 abs", "[math][point4]")
{
    {
        const Point4f vec(5, -10, -15, 20);
        const Point4f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
        CHECK(a.w == 20.0f);
    }
    {
        const Point4d vec(5, -10, -15, 20);
        const Point4d a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
        CHECK(a.z == 15.0);
        CHECK(a.w == 20.0);
    }
    {
        const Point4i vec(5, -10, -15, 20);
        const Point4i a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
        CHECK(a.z == 15.0);
        CHECK(a.w == 20.0);
    }
}

TEST_CASE("Point4 to euclidian space", "[math][point4]")
{
{
        const Point4f vec(10, 20, 30, 2);
        const Point4f a = Opal::ToEuclidean(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
        CHECK(a.w == 1.0f);
    }
    {
        const Point4d vec(10, 20, 30, 2);
        const Point4d a = Opal::ToEuclidean(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
        CHECK(a.z == 15.0);
        CHECK(a.w == 1.0);
    }
    {
        const Point4i vec(10, 20, 30, 2);
        const Point4i a = Opal::ToEuclidean(vec);

        CHECK(a.x == 5);
        CHECK(a.y == 10);
        CHECK(a.z == 15);
        CHECK(a.w == 1);
    }
}

TEST_CASE("Point4 comparison with IsEqual", "[math][point4]")
{
    {
        const Point4f v1(1, 2, 3, 4);
        const Point4f v2(2, 3, 4, 5);
        const Point4f v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Point4d v1(1, 2, 3, 4);
        const Point4d v2(2, 3, 4, 5);
        const Point4d v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Point4i v1(1, 2, 3, 4);
        const Point4i v2(2, 3, 4, 5);
        const Point4i v3(0, 1, 2, 3);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Point4 distance", "[math][point4]")
{
    {
        const Point4f p1(3, 4, 5, 6);
        const Point4f p2(4, 5, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 2.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 4.0f);
    }
    {
        const Point4d p1(3, 4, 5, 6);
        const Point4d p2(4, 5, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 2.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 4.0);
    }
    {
        const Point4i p1(3, 4, 5, 6);
        const Point4i p2(4, 5, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 2.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 4);
    }
}

TEST_CASE("Point4 lerp", "[math][point4]")
{
    {
        const Point4f p1(1, 2, 3, 4);
        const Point4f p2(3, 4, 5, 6);

        const Point4f lerp = Opal::Lerp(0.5f, p1, p2);

        CHECK(lerp.x == 2.0f);
        CHECK(lerp.y == 3.0f);
        CHECK(lerp.z == 4.0f);
        CHECK(lerp.w == 5.0f);
    }
    {
        const Point4d p1(1, 2, 3, 4);
        const Point4d p2(3, 4, 5, 6);

        const Point4d lerp = Opal::Lerp(0.5, p1, p2);

        CHECK(lerp.x == 2.0);
        CHECK(lerp.y == 3.0);
        CHECK(lerp.z == 4.0);
        CHECK(lerp.w == 5.0);
    }
    {
        const Point4i p1(1, 2, 3, 4);
        const Point4i p2(3, 4, 5, 6);

        const Point4i lerp = Opal::Lerp(2, p1, p2);

        CHECK(lerp.x == 5);
        CHECK(lerp.y == 6);
        CHECK(lerp.z == 7);
        CHECK(lerp.w == 8);
    }
}

TEST_CASE("Point4 min, max and compute", "[math][point4]")
{
    {
        const Point4f vec1(1, 2, 3, 4);
        const Point4f vec2(3, -2, 5, -3);
        const Point4f min = Min(vec1, vec2);
        const Point4f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(min.w == -3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);
        CHECK(max.w == 4.0f);

        const Point4f vec(1, 2, 3, 4);
        const Point4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 4.0f);
        CHECK(perm.w == 1.0f);
    }
    {
        const Point4d vec1(1, 2, 3, 4);
        const Point4d vec2(3, -2, 5, -3);
        const Point4d min = Min(vec1, vec2);
        const Point4d max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(min.z == 3.0);
        CHECK(min.w == -3.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);
        CHECK(max.z == 5.0);
        CHECK(max.w == 4.0);

        const Point4f vec(1, 2, 3, 4);
        const Point4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 4.0f);
        CHECK(perm.w == 1.0f);
    }
    {
        const Point4i vec1(1, 2, 3, 4);
        const Point4i vec2(3, -2, 5, -3);
        const Point4i min = Min(vec1, vec2);
        const Point4i max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(min.z == 3.0);
        CHECK(min.w == -3.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);
        CHECK(max.z == 5.0);
        CHECK(max.w == 4.0);

        const Point4f vec(1, 2, 3, 4);
        const Point4f perm = Opal::Permute(vec, 1, 2, 3, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 4.0f);
        CHECK(perm.w == 1.0f);
    }
}

TEST_CASE("Point4 floor", "[math][point4]")
{
    {
        const Point4f vec(1.1f, 2.2f, 3.3f, 4.4f);
        const Point4f floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0f);
        CHECK(floor.y == 2.0f);
        CHECK(floor.z == 3.0f);
        CHECK(floor.w == 4.0f);
    }
    {
        const Point4d vec(1.1, 2.2, 3.3, 4.4);
        const Point4d floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0);
        CHECK(floor.y == 2.0);
        CHECK(floor.z == 3.0);
        CHECK(floor.w == 4.0);
    }
    {
        const Point4i vec(1, 2, 3, 4);
        const Point4i floor = Opal::Floor(vec);
        CHECK(floor.x == 1);
        CHECK(floor.y == 2);
        CHECK(floor.z == 3);
        CHECK(floor.w == 4);
    }
}

TEST_CASE("Point4 ceil", "[math][point4]")
{
    {
        const Point4f vec(1.1f, 2.2f, 3.3f, 4.4f);
        const Point4f ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0f);
        CHECK(ceil.y == 3.0f);
        CHECK(ceil.z == 4.0f);
        CHECK(ceil.w == 5.0f);
    }
    {
        const Point4d vec(1.1, 2.2, 3.3, 4.4);
        const Point4d ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0);
        CHECK(ceil.y == 3.0);
        CHECK(ceil.z == 4.0);
        CHECK(ceil.w == 5.0);
    }
    {
        const Point4i vec(1, 2, 3, 4);
        const Point4i ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 1);
        CHECK(ceil.y == 2);
        CHECK(ceil.z == 3);
        CHECK(ceil.w == 4);
    }
}

TEST_CASE("Point4 round", "[math][point4]")
{
    {
        const Point4f vec(1.1f, 2.2f, 3.5f, 4.7f);
        const Point4f round = Opal::Round(vec);
        CHECK(round.x == 1.0f);
        CHECK(round.y == 2.0f);
        CHECK(round.z == 4.0f);
        CHECK(round.w == 5.0f);
    }
    {
        const Point4d vec(1.1, 2.2, 3.5, 4.7);
        const Point4d round = Opal::Round(vec);
        CHECK(round.x == 1.0);
        CHECK(round.y == 2.0);
        CHECK(round.z == 4.0);
        CHECK(round.w == 5.0);
    }
    {
        const Point4i vec(1, 2, 3, 4);
        const Point4i round = Opal::Round(vec);
        CHECK(round.x == 1);
        CHECK(round.y == 2);
        CHECK(round.z == 3);
        CHECK(round.w == 4);
    }
}

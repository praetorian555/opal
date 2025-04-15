#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/point3.h"

using Point3f = Opal::Point3<float>;
using Point3d = Opal::Point3<double>;
using Point3i = Opal::Point3<int>;
using Vec3f = Opal::Vector3<float>;
using Vec3d = Opal::Vector3<double>;
using Vec3i = Opal::Vector3<int>;

TEST_CASE("Point3 construction", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);

        const Point3f vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
    }
    {
        const Point3d vec1(1, 2, 3);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);

        const Point3d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
    }
    {
        const Point3i vec1(1, 2, 3);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);
        CHECK(vec1.z == 3);

        const Point3i vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
        CHECK(vec2.z == 1);
    }
}

TEST_CASE("Point3 access operator", "[math][point3]")
{
    {
        Point3f p1(1, 2, 3);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);
        CHECK(p1[2] == 3);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
        CHECK(p1[2] == 7);
    }
    {
        Point3d p1(1, 2, 3);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);
        CHECK(p1[2] == 3);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
        CHECK(p1[2] == 7);
    }
    {
        Point3i p1(1, 2, 3);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);
        CHECK(p1[2] == 3);

        p1[0] = 5;
        p1[1] = 6;
        p1[2] = 7;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
        CHECK(p1[2] == 7);
    }
}

TEST_CASE("Point3 non-finite", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Point3 NaN", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point3f vec2(1, 2, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point3f vec3(1, 2, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point3f vec4(1, 2, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point3f vec5(1, 2, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point3d vec2(1, 2, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point3d vec3(1, 2, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point3d vec4(1, 2, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point3d vec5(1, 2, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point3d vec1(1, 2, 3);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Point3 comparisons", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 4);
        const Point3f vec2(1, 2, 4);
        const Point3f vec3(1, 2, 5);
        const Point3f vec4(1, 2, 4);
        const Point3f vec5(1, 3, 4);
        const Point3f vec6(2, 2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Point3d vec1(1, 2, 4);
        const Point3d vec2(1, 2, 4);
        const Point3d vec3(1, 2, 5);
        const Point3d vec4(1, 2, 4);
        const Point3d vec5(1, 3, 4);
        const Point3d vec6(2, 2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Point3i vec1(1, 2, 4);
        const Point3i vec2(1, 2, 4);
        const Point3i vec3(1, 2, 5);
        const Point3i vec4(1, 2, 4);
        const Point3i vec5(1, 3, 4);
        const Point3i vec6(2, 2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 == vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 != vec5);
        CHECK(vec1 != vec6);
    }
}

TEST_CASE("Point3 addition", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        const Vec3f vec2(3, 4, 5);

        Point3f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);
        CHECK(vec3.y == 6.0f);
        CHECK(vec3.z == 8.0f);

        vec3 += vec2;
        CHECK(vec3.x == 7.0f);
        CHECK(vec3.y == 10.0f);
        CHECK(vec3.z == 13.0f);
    }
    {
        const Point3d vec1(1, 2, 3);
        const Vec3d vec2(3, 4, 5);

        Point3d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);
        CHECK(vec3.y == 6.0);
        CHECK(vec3.z == 8.0);

        vec3 += vec2;
        CHECK(vec3.x == 7.0);
        CHECK(vec3.y == 10.0);
        CHECK(vec3.z == 13.0);
    }
    {
        const Point3i vec1(1, 2, 3);
        const Vec3i vec2(3, 4, 5);

        Point3i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);
        CHECK(vec3.y == 6);
        CHECK(vec3.z == 8);

        vec3 += vec2;
        CHECK(vec3.x == 7);
        CHECK(vec3.y == 10);
        CHECK(vec3.z == 13);
    }
}

TEST_CASE("Point3 subtraction", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        const Point3f vec2(3, 4, 5);
        const Vec3f vec3(2, 2, 2);

        const Point3f vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0f);
        CHECK(vec4.y == 2.0f);
        CHECK(vec4.z == 3.0f);

        const Vec3f vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0f);
        CHECK(vec5.y == 2.0f);
        CHECK(vec5.z == 2.0f);
    }
    {
        const Point3d vec1(1, 2, 3);
        const Point3d vec2(3, 4, 5);
        const Vec3d vec3(2, 2, 2);

        const Point3d vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0f);
        CHECK(vec4.y == 2.0f);
        CHECK(vec4.z == 3.0f);

        const Vec3d vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0f);
        CHECK(vec5.y == 2.0f);
        CHECK(vec5.z == 2.0f);
    }
    {
        const Point3i vec1(1, 2, 3);
        const Point3i vec2(3, 4, 5);
        const Vec3i vec3(2, 2, 2);

        const Point3i vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0f);
        CHECK(vec4.y == 2.0f);
        CHECK(vec4.z == 3.0f);

        const Vec3i vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0f);
        CHECK(vec5.y == 2.0f);
        CHECK(vec5.z == 2.0f);
    }
}

TEST_CASE("Point3 negation", "[math][point3]")
{
    {
        const Point3f vec(5, -10, -15);
        const Point3f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
    }
    {
        const Point3d vec(5, -10, -15);
        const Point3d neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
    }
    {
        const Point3i vec(5, -10, -15);
        const Point3i neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
        CHECK(neg.z == 15.0f);
    }
}

TEST_CASE("Point3 multiplication by scalar", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);

        Point3f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        const Point3f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
    }
    {
        const Point3d vec1(1, 2, 3);

        Point3d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        const Point3d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
    }
    {
        const Point3i vec1(1, 2, 3);

        Point3i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        const Point3i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
        CHECK(vec3.z == 60.0f);
    }
}

TEST_CASE("Point3 division by scalar", "[math][point3]")
{
    {
        const Point3f vec1(20.0f, 40.0f, 60.0f);

        Point3f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
    }
    {
        const Point3d vec1(20.0f, 40.0f, 60.0f);

        Point3d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
    }
    {
        const Point3i vec1(20, 40, 60);

        Point3i vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);
        CHECK(vec2.z == 30.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
        CHECK(vec2.z == 15.0f);
    }
}

TEST_CASE("Point3 abs", "[math][point3]")
{
    {
        const Point3f vec(5, -10, -15);
        const Point3f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
    }
    {
        const Point3d vec(5, -10, -15);
        const Point3d a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
    }
    {
        const Point3i vec(5, -10, -15);
        const Point3i a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
        CHECK(a.z == 15.0f);
    }
}

TEST_CASE("Point3 comparison with IsEqual", "[math][point3]")
{
    {
        const Point3f v1(1, 2, 3);
        const Point3f v2(2, 3, 4);
        const Point3f v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Point3d v1(1, 2, 3);
        const Point3d v2(2, 3, 4);
        const Point3d v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Point3i v1(1, 2, 3);
        const Point3i v2(2, 3, 4);
        const Point3i v3(0, 1, 2);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Point3 distance", "[math][point3]")
{
    {
        const Point3f p1(3, 4, 5);
        const Point3f p2(4, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 3.0f);
        CHECK(Opal::DistanceSquared(p1, p2) == 9.0f);
    }
    {
        const Point3d p1(3, 4, 5);
        const Point3d p2(4, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 3.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 9.0);
    }
    {
        const Point3i p1(3, 4, 5);
        const Point3i p2(4, 6, 7);

        CHECK(Opal::Distance(p1, p2) == 3.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 9);
    }
}

TEST_CASE("Point3 lerp", "[math][point3]")
{
    {
        const Point3f p1(1, 2, 3);
        const Point3f p2(3, 4, 5);

        const Point3f lerp = Opal::Lerp(0.5f, p1, p2);

        CHECK(lerp.x == 2.0f);
        CHECK(lerp.y == 3.0f);
        CHECK(lerp.z == 4.0f);
    }
    {
        const Point3d p1(1, 2, 3);
        const Point3d p2(3, 4, 5);

        const Point3d lerp = Opal::Lerp(0.5, p1, p2);

        CHECK(lerp.x == 2.0);
        CHECK(lerp.y == 3.0);
        CHECK(lerp.z == 4.0);
    }
    {
        const Point3i p1(1, 2, 3);
        const Point3i p2(3, 4, 5);

        const Point3i lerp = Opal::Lerp(2, p1, p2);

        CHECK(lerp.x == 5);
        CHECK(lerp.y == 6);
        CHECK(lerp.z == 7);
    }
}

TEST_CASE("Point3 min, max and permute", "[math][point3]")
{
    {
        const Point3f vec1(1, 2, 3);
        const Point3f vec2(3, -2, 5);
        const Point3f min = Min(vec1, vec2);
        const Point3f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);

        const Point3f vec(1, 2, 3);
        const Point3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 1.0f);
    }
    {
        const Point3d vec1(1, 2, 3);
        const Point3d vec2(3, -2, 5);
        const Point3d min = Min(vec1, vec2);
        const Point3d max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);

        const Point3f vec(1, 2, 3);
        const Point3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 1.0f);
    }
    {
        const Point3i vec1(1, 2, 3);
        const Point3i vec2(3, -2, 5);
        const Point3i min = Min(vec1, vec2);
        const Point3i max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(min.z == 3.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);
        CHECK(max.z == 5.0f);

        const Point3f vec(1, 2, 3);
        const Point3f perm = Opal::Permute(vec, 1, 2, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 3.0f);
        CHECK(perm.z == 1.0f);
    }
}

TEST_CASE("Point3 floor", "[math][point3]")
{
    {
        const Point3f vec(1.1f, 2.2f, 3.3f);
        const Point3f floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0f);
        CHECK(floor.y == 2.0f);
        CHECK(floor.z == 3.0f);
    }
    {
        const Point3d vec(1.1, 2.2, 3.3);
        const Point3d floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0);
        CHECK(floor.y == 2.0);
        CHECK(floor.z == 3.0);
    }
    {
        const Point3i vec(1, 2, 3);
        const Point3i floor = Opal::Floor(vec);
        CHECK(floor.x == 1);
        CHECK(floor.y == 2);
        CHECK(floor.z == 3);
    }
}

TEST_CASE("Point3 ceil", "[math][point3]")
{
    {
        const Point3f vec(1.1f, 2.2f, 3.3f);
        const Point3f ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0f);
        CHECK(ceil.y == 3.0f);
        CHECK(ceil.z == 4.0f);
    }
    {
        const Point3d vec(1.1, 2.2, 3.3);
        const Point3d ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0);
        CHECK(ceil.y == 3.0);
        CHECK(ceil.z == 4.0);
    }
    {
        const Point3i vec(1, 2, 3);
        const Point3i ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 1);
        CHECK(ceil.y == 2);
        CHECK(ceil.z == 3);
    }
}

TEST_CASE("Point3 round", "[math][point3]")
{
    {
        const Point3f vec(1.1f, 2.2f, 3.5f);
        const Point3f round = Opal::Round(vec);
        CHECK(round.x == 1.0f);
        CHECK(round.y == 2.0f);
        CHECK(round.z == 4.0f);
    }
    {
        const Point3d vec(1.1, 2.2, 3.5);
        const Point3d round = Opal::Round(vec);
        CHECK(round.x == 1.0);
        CHECK(round.y == 2.0);
        CHECK(round.z == 4.0);
    }
    {
        const Point3i vec(1, 2, 3);
        const Point3i round = Opal::Round(vec);
        CHECK(round.x == 1);
        CHECK(round.y == 2);
        CHECK(round.z == 3);
    }
}

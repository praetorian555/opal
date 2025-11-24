#include <limits>

#include "test-helpers.h"

#include "opal/math/point2.h"

using Point2f = Opal::Point2<float>;
using Point2d = Opal::Point2<double>;
using Point2i = Opal::Point2<int>;
using Vec2f = Opal::Vector2<float>;
using Vec2d = Opal::Vector2<double>;
using Vec2i = Opal::Vector2<int>;

TEST_CASE("Point2 constructor", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        CHECK(vec1.x == 1.0f);
        CHECK(vec1.y == 2.0f);

        const Point2f vec2(1);
        CHECK(vec2.x == 1.0f);
        CHECK(vec2.y == 1.0f);
    }
    {
        const Point2d vec1(1, 2);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);

        const Point2d vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
    }
    {
        const Point2i vec1(1, 2);
        CHECK(vec1.x == 1);
        CHECK(vec1.y == 2);

        const Point2i vec2(1);
        CHECK(vec2.x == 1);
        CHECK(vec2.y == 1);
    }
}

TEST_CASE("Point2 access operators", "[math][point2]")
{
    {
        Point2f p1(1, 2);
        CHECK(p1[0] == 1.0f);
        CHECK(p1[1] == 2.0f);

        p1[0] = 5;
        p1[1] = 6;
        CHECK(p1[0] == 5.0f);
        CHECK(p1[1] == 6.0f);
    }
    {
        Point2d p1(1, 2);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);

        p1[0] = 5;
        p1[1] = 6;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
    }
    {
        Point2i p1(1, 2);
        CHECK(p1[0] == 1);
        CHECK(p1[1] == 2);

        p1[0] = 5;
        p1[1] = 6;
        CHECK(p1[0] == 5);
        CHECK(p1[1] == 6);
    }
}

TEST_CASE("Point2 non-finite", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point2f vec2(1, std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point2f vec3(1, -std::numeric_limits<float>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point2f vec4(1, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point2f vec5(1, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point2d vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));

        const Point2d vec2(1, std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec2));

        const Point2d vec3(1, -std::numeric_limits<double>::infinity());
        CHECK(ContainsNonFinite(vec3));

        const Point2d vec4(1, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec4));

        const Point2d vec5(1, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNonFinite(vec5));
    }
    {
        const Point2d vec1(1, 2);
        CHECK_FALSE(ContainsNonFinite(vec1));
    }
}

TEST_CASE("Point2 NaN", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point2f vec2(1, std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point2f vec3(1, -std::numeric_limits<float>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point2f vec4(1, std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point2f vec5(1, -std::numeric_limits<float>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point2d vec1(1, 2);
        CHECK_FALSE(ContainsNaN(vec1));

        const Point2d vec2(1, std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec2));

        const Point2d vec3(1, -std::numeric_limits<double>::infinity());
        CHECK_FALSE(ContainsNaN(vec3));

        const Point2d vec4(1, std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec4));

        const Point2d vec5(1, -std::numeric_limits<double>::quiet_NaN());
        CHECK(ContainsNaN(vec5));
    }
    {
        const Point2d vec1(1, 3);
        CHECK_FALSE(ContainsNaN(vec1));
    }
}

TEST_CASE("Point2 comparisons", "[math][point2]")
{
    {
        const Point2f vec1(1, 4);
        const Point2f vec2(1, 4);
        const Point2f vec3(1, 5);
        const Point2f vec4(1, 4);
        const Point2f vec5(1, 4);
        const Point2f vec6(2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 != vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 == vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Point2d vec1(1, 4);
        const Point2d vec2(1, 4);
        const Point2d vec3(1, 5);
        const Point2d vec4(1, 4);
        const Point2d vec5(1, 4);
        const Point2d vec6(2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 != vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 == vec5);
        CHECK(vec1 != vec6);
    }
    {
        const Point2i vec1(1, 4);
        const Point2i vec2(1, 4);
        const Point2i vec3(1, 5);
        const Point2i vec4(1, 4);
        const Point2i vec5(1, 4);
        const Point2i vec6(2, 4);

        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 != vec4);
        CHECK_FALSE(vec1 != vec5);
        CHECK_FALSE(vec1 == vec6);

        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 == vec4);
        CHECK(vec1 == vec5);
        CHECK(vec1 != vec6);
    }
}

TEST_CASE("Point2 addition", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        const Vec2f vec2(3, 4);

        Point2f vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0f);
        CHECK(vec3.y == 6.0f);

        vec3 += vec2;
        CHECK(vec3.x == 7.0f);
        CHECK(vec3.y == 10.0f);
    }
    {
        const Point2d vec1(1, 2);
        const Vec2d vec2(3, 4);

        Point2d vec3 = vec1 + vec2;
        CHECK(vec3.x == 4.0);
        CHECK(vec3.y == 6.0);

        vec3 += vec2;
        CHECK(vec3.x == 7.0);
        CHECK(vec3.y == 10.0);
    }
    {
        const Point2i vec1(1, 2);
        const Vec2i vec2(3, 4);

        Point2i vec3 = vec1 + vec2;
        CHECK(vec3.x == 4);
        CHECK(vec3.y == 6);

        vec3 += vec2;
        CHECK(vec3.x == 7);
        CHECK(vec3.y == 10);
    }
}

TEST_CASE("Point2 subtraction", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        const Point2f vec2(3, 4);
        const Vec2f vec3(2, 2);

        const Point2f vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0f);
        CHECK(vec4.y == 2.0f);

        const Vec2f vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0f);
        CHECK(vec5.y == 2.0f);
    }
    {
        const Point2d vec1(1, 2);
        const Point2d vec2(3, 4);
        const Vec2d vec3(2, 2);

        const Point2d vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0);
        CHECK(vec4.y == 2.0);

        const Vec2d vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0);
        CHECK(vec5.y == 2.0);
    }
    {
        const Point2i vec1(1, 2);
        const Point2i vec2(3, 4);
        const Vec2i vec3(2, 2);

        const Point2i vec4 = vec2 - vec3;
        CHECK(vec4.x == 1.0);
        CHECK(vec4.y == 2.0);

        const Vec2i vec5 = vec2 - vec1;
        CHECK(vec5.x == 2.0);
        CHECK(vec5.y == 2.0);
    }
}

TEST_CASE("Point2 negation", "[math][point2]")
{
    {
        const Point2f vec(5, -10);
        const Point2f neg = -vec;

        CHECK(neg.x == -5.0f);
        CHECK(neg.y == 10.0f);
    }
    {
        const Point2d vec(5, -10);
        const Point2d neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
    }
    {
        const Point2i vec(5, -10);
        const Point2i neg = -vec;

        CHECK(neg.x == -5.0);
        CHECK(neg.y == 10.0);
    }
}

TEST_CASE("Point2 multiplication by scalar", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);

        Point2f vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        const Point2f vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0f);
        CHECK(vec3.y == 40.0f);
    }
    {
        const Point2d vec1(1, 2);

        Point2d vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);

        const Point2d vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
    }
    {
        const Point2i vec1(1, 2);

        Point2i vec2 = vec1 * 5.0f;

        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);

        vec2 *= 2.0f;
        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);

        const Point2i vec3 = 2.0f * vec2;
        CHECK(vec3.x == 20.0);
        CHECK(vec3.y == 40.0);
    }
}

TEST_CASE("Point2 division by scalar", "[math][point2]")
{
    {
        const Point2f vec1(20.0f, 40.0f);

        Point2f vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0f);
        CHECK(vec2.y == 20.0f);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0f);
        CHECK(vec2.y == 10.0f);
    }
    {
        const Point2d vec1(20.0, 40.0);

        Point2d vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
    }
    {
        const Point2i vec1(20, 40);

        Point2i vec2 = vec1 / 2.0f;

        CHECK(vec2.x == 10.0);
        CHECK(vec2.y == 20.0);

        vec2 /= 2.0f;
        CHECK(vec2.x == 5.0);
        CHECK(vec2.y == 10.0);
    }
}

TEST_CASE("Point2 abs", "[math][point2]")
{
    {
        const Point2f vec(5, -10);
        const Point2f a = Opal::Abs(vec);

        CHECK(a.x == 5.0f);
        CHECK(a.y == 10.0f);
    }
    {
        const Point2d vec(5, -10);
        const Point2d a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
    }
    {
        const Point2i vec(5, -10);
        const Point2i a = Opal::Abs(vec);

        CHECK(a.x == 5.0);
        CHECK(a.y == 10.0);
    }
}

TEST_CASE("Point2 comparison with IsEqual", "[math][point2]")
{
    {
        const Point2f v1(1, 2);
        const Point2f v2(2, 3);
        const Point2f v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1.0f));
        CHECK(Opal::IsEqual(v1, v2, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5f));
        CHECK(Opal::IsEqual(v1, v3, 1.0f));
        CHECK(Opal::IsEqual(v1, v3, 2.0f));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5f));
    }
    {
        const Point2d v1(1, 2);
        const Point2d v2(2, 3);
        const Point2d v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1.0));
        CHECK(Opal::IsEqual(v1, v2, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0.5));
        CHECK(Opal::IsEqual(v1, v3, 1.0));
        CHECK(Opal::IsEqual(v1, v3, 2.0));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0.5));
    }
    {
        const Point2i v1(1, 2);
        const Point2i v2(2, 3);
        const Point2i v3(0, 1);

        CHECK(Opal::IsEqual(v1, v2, 1));
        CHECK(Opal::IsEqual(v1, v2, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v2, 0));
        CHECK(Opal::IsEqual(v1, v3, 1));
        CHECK(Opal::IsEqual(v1, v3, 2));
        CHECK_FALSE(Opal::IsEqual(v1, v3, 0));
    }
}

TEST_CASE("Point2 distance", "[math][point2]")
{
    {
        const Point2f p1(3, 4);
        const Point2f p2(6, 8);

        CHECK(Opal::Distance(p1, p2) == 5.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 25.0f);
    }
    {
        const Point2d p1(3, 4);
        const Point2d p2(6, 8);

        CHECK(Opal::Distance(p1, p2) == 5.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 25.0);
    }
    {
        const Point2i p1(3, 4);
        const Point2i p2(6, 8);

        CHECK(Opal::Distance(p1, p2) == 5.0);
        CHECK(Opal::DistanceSquared(p1, p2) == 25);
    }
}

TEST_CASE("Point2 lerp", "[math][point2]")
{
    {
        const Point2f p1(1, 2);
        const Point2f p2(3, 4);

        const Point2f lerp = Opal::Lerp(0.5f, p1, p2);

        CHECK(lerp.x == 2.0f);
        CHECK(lerp.y == 3.0f);
    }
    {
        const Point2d p1(1, 2);
        const Point2d p2(3, 4);

        const Point2d lerp = Opal::Lerp(0.5, p1, p2);

        CHECK(lerp.x == 2.0);
        CHECK(lerp.y == 3.0);
    }
    {
        const Point2i p1(1, 2);
        const Point2i p2(3, 4);

        const Point2i lerp = Opal::Lerp(2, p1, p2);

        CHECK(lerp.x == 5);
        CHECK(lerp.y == 6);
    }
}

TEST_CASE("Point2 min, max and permute", "[math][point2]")
{
    {
        const Point2f vec1(1, 2);
        const Point2f vec2(3, -2);
        const Point2f min = Min(vec1, vec2);
        const Point2f max = Max(vec1, vec2);

        CHECK(min.x == 1.0f);
        CHECK(min.y == -2.0f);
        CHECK(max.x == 3.0f);
        CHECK(max.y == 2.0f);

        const Point2f vec(1, 2);
        const Point2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
    {
        const Point2d vec1(1, 2);
        const Point2d vec2(3, -2);
        const Point2d min = Min(vec1, vec2);
        const Point2d max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);

        const Point2f vec(1, 2);
        const Point2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
    {
        const Point2i vec1(1, 2);
        const Point2i vec2(3, -2);
        const Point2i min = Min(vec1, vec2);
        const Point2i max = Max(vec1, vec2);

        CHECK(min.x == 1.0);
        CHECK(min.y == -2.0);
        CHECK(max.x == 3.0);
        CHECK(max.y == 2.0);

        const Point2f vec(1, 2);
        const Point2f perm = Opal::Permute(vec, 1, 0);
        CHECK(perm.x == 2.0f);
        CHECK(perm.y == 1.0f);
    }
}

TEST_CASE("Point2 floor", "[math][point2]")
{
    {
        const Point2f vec(1.1f, 2.2f);
        const Point2f floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0f);
        CHECK(floor.y == 2.0f);
    }
    {
        const Point2d vec(1.1, 2.2);
        const Point2d floor = Opal::Floor(vec);
        CHECK(floor.x == 1.0);
        CHECK(floor.y == 2.0);
    }
    {
        const Point2i vec(1, 2);
        const Point2i floor = Opal::Floor(vec);
        CHECK(floor.x == 1);
        CHECK(floor.y == 2);
    }
}

TEST_CASE("Point2 ceil" , "[math][point2]")
{
    {
        const Point2f vec(1.1f, 2.2f);
        const Point2f ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0f);
        CHECK(ceil.y == 3.0f);
    }
    {
        const Point2d vec(1.1, 2.2);
        const Point2d ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 2.0);
        CHECK(ceil.y == 3.0);
    }
    {
        const Point2i vec(1, 2);
        const Point2i ceil = Opal::Ceil(vec);
        CHECK(ceil.x == 1);
        CHECK(ceil.y == 2);
    }
}

TEST_CASE("Point2 round", "[math][point2]")
{
    {
        const Point2f vec(1.1f, 3.5f);
        const Point2f round = Opal::Round(vec);
        CHECK(round.x == 1.0f);
        CHECK(round.y == 4.0f);
    }
    {
        const Point2d vec(1.1, 3.5);
        const Point2d round = Opal::Round(vec);
        CHECK(round.x == 1.0);
        CHECK(round.y == 4.0);
    }
    {
        const Point2i vec(1, 2);
        const Point2i round = Opal::Round(vec);
        CHECK(round.x == 1);
        CHECK(round.y == 2);
    }
}

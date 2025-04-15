#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/bounds2.h"

using Point2f = Opal::Point2<float>;
using Bounds2f = Opal::Bounds2<float>;
using Vector2f = Opal::Vector2<float>;
using Point2d = Opal::Point2<double>;
using Bounds2d = Opal::Bounds2<double>;
using Vector2d = Opal::Vector2<double>;
using Point2i = Opal::Point2<int32_t>;
using Bounds2i = Opal::Bounds2<int32_t>;
using Vector2i = Opal::Vector2<int32_t>;

TEST_CASE("Bounds2 creator", "[math][bounds2]")
{
    {
        const Point2f p1(2, 4);
        const Point2f p2(4, -2);

        const Bounds2f b1(p1);
        const Bounds2f b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point2f(2, -2));
        CHECK(b2.max == Point2f(4, 4));
    }
    {
        const Point2d p1(2, 4);
        const Point2d p2(4, -2);

        const Bounds2d b1(p1);
        const Bounds2d b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point2d(2, -2));
        CHECK(b2.max == Point2d(4, 4));
    }
    {
        const Point2i p1(2, 4);
        const Point2i p2(4, -2);

        const Bounds2i b1(p1);
        const Bounds2i b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point2i(2, -2));
        CHECK(b2.max == Point2i(4, 4));
    }
}

TEST_CASE("Bounds2 access operator", "[math][bounds2]")
{
    {
        const Point2f p1(2, 3);
        const Point2f p2(4, 5);
        Bounds2f b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds2f b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point2f p3(5, 6);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
    {
        const Point2d p1(2, 3);
        const Point2d p2(4, 5);
        Bounds2d b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds2d b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point2d p3(5, 6);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
    {
        const Point2i p1(2, 3);
        const Point2i p2(4, 5);
        Bounds2i b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds2i b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point2i p3(5, 6);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
}

TEST_CASE("Bounds2 comparison", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
}

TEST_CASE("Bounds2 corner", "[math][bounds2]")
{
    {
        const Point2f p1(2, 3);
        const Point2f p2(4, 5);
        const Bounds2f b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b00));
        CHECK(p2 == Corner(b1, 0b11));
        CHECK(Point2f(2, 5) == Corner(b1, 0b10));
    }
    {
        const Point2d p1(2, 3);
        const Point2d p2(4, 5);
        const Bounds2d b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b00));
        CHECK(p2 == Corner(b1, 0b11));
        CHECK(Point2d(2, 5) == Corner(b1, 0b10));
    }
    {
        const Point2i p1(2, 3);
        const Point2i p2(4, 5);
        const Bounds2i b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b00));
        CHECK(p2 == Corner(b1, 0b11));
        CHECK(Point2i(2, 5) == Corner(b1, 0b10));
    }
}

TEST_CASE("Bounds2 diagonal", "[math][bounds2]")
{
    {
        const Point2f p1(2, 3);
        const Point2f p2(4, 5);
        const Bounds2f b1(p1, p2);
        const Vector2f v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
    {
        const Point2d p1(2, 3);
        const Point2d p2(4, 5);
        const Bounds2d b1(p1, p2);
        const Vector2d v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
    {
        const Point2i p1(2, 3);
        const Point2i p2(4, 5);
        const Bounds2i b1(p1, p2);
        const Vector2i v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
}

TEST_CASE("Bounds2 surface area", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Bounds2f b1(p1, p2);
        CHECK(SurfaceArea(b1) == 4.0f);
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Bounds2d b1(p1, p2);
        CHECK(SurfaceArea(b1) == 4.0);
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Bounds2i b1(p1, p2);
        CHECK(SurfaceArea(b1) == 4);
    }
}

TEST_CASE("Bounds2 maximum extent", "[math][bounds2]")
{
    {
        const Point2f p1(2, 3);
        const Point2f p2(5, 5);
        const Bounds2f b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
    {
        const Point2d p1(2, 3);
        const Point2d p2(5, 5);
        const Bounds2d b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
    {
        const Point2i p1(2, 3);
        const Point2i p2(5, 5);
        const Bounds2i b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
}

TEST_CASE("Bounds2 lerp", "[math][bounds2]")
{
    {
        const Point2f p1(2, 3);
        const Point2f p2(5, 5);
        const Bounds2f b1(p1, p2);
        const Point2f p = Lerp(b1, Point2f(0.5f, 0.5f));
        CHECK(Point2f(3.5f, 4.0f) == p);
    }
    {
        const Point2d p1(2, 3);
        const Point2d p2(5, 5);
        const Bounds2d b1(p1, p2);
        const Point2d p = Lerp(b1, Point2d(0.5f, 0.5f));
        CHECK(Point2d(3.5f, 4.0f) == p);
    }
}

TEST_CASE("Bounds2 offset", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(3, 3);
        const Bounds2f b(p1, p2);
        CHECK(Offset(b, p3) == Vector2f(0.5f, 0.5f));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(3, 3);
        const Bounds2d b(p1, p2);
        CHECK(Offset(b, p3) == Vector2d(0.5, 0.5));
    }
}

TEST_CASE("Bounds2 bounding sphere", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Bounds2f b(p1, p2);

        Point2f center;
        float radius = 0.0f;
        BoundingSphere(b, center, radius);

        CHECK(center == Point2f(3, 3));
        CHECK(radius == Opal::Sqrt(2.0f));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Bounds2d b(p1, p2);

        Point2d center;
        double radius = 0.0;
        BoundingSphere(b, center, radius);

        CHECK(center == Point2d(3, 3));
        CHECK(radius == Opal::Sqrt(2.0));
    }
}

TEST_CASE("Bounds2 extent", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(5, 5);
        const Bounds2f b(p1, p2);
        const Vector2f e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(5, 5);
        const Bounds2d b(p1, p2);
        const Vector2d e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(5, 5);
        const Bounds2i b(p1, p2);
        const Vector2i e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
    }
}

TEST_CASE("Bounds2 union", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(0, 0);
        const Point2f p4(-2, -2);
        const Bounds2f b1(p1, p2);
        const Bounds2f b2(p3, p4);
        const Bounds2f b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds2f b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(0, 0);
        const Point2d p4(-2, -2);
        const Bounds2d b1(p1, p2);
        const Bounds2d b2(p3, p4);
        const Bounds2d b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds2d b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(0, 0);
        const Point2i p4(-2, -2);
        const Bounds2i b1(p1, p2);
        const Bounds2i b2(p3, p4);
        const Bounds2i b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds2i b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
}

TEST_CASE("Bounds2 expand", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Bounds2f b1(p1, p2);
        const Bounds2f b2 = Expand(b1, 2.0f);
        CHECK(b2.min == Point2f(0, 0));
        CHECK(b2.max == Point2f(6, 6));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Bounds2d b1(p1, p2);
        const Bounds2d b2 = Expand(b1, 2.0);
        CHECK(b2.min == Point2d(0, 0));
        CHECK(b2.max == Point2d(6, 6));
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Bounds2i b1(p1, p2);
        const Bounds2i b2 = Expand(b1, 2);
        CHECK(b2.min == Point2i(0, 0));
        CHECK(b2.max == Point2i(6, 6));
    }
}

TEST_CASE("Bounds2 overlaps", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(3, 3);
        const Point2f p4(5, 5);
        const Bounds2f b1(p1, p2);
        const Bounds2f b2(p3, p4);
        const Bounds2f b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(3, 3);
        const Point2d p4(5, 5);
        const Bounds2d b1(p1, p2);
        const Bounds2d b2(p3, p4);
        const Bounds2d b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(3, 3);
        const Point2i p4(5, 5);
        const Bounds2i b1(p1, p2);
        const Bounds2i b2(p3, p4);
        const Bounds2i b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
}

TEST_CASE("Bounds2 intersect", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(3, 3);
        const Point2f p4(5, 5);
        const Bounds2f b1(p1, p2);
        const Bounds2f b2(p3, p4);
        const Bounds2f b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(3, 3);
        const Point2d p4(5, 5);
        const Bounds2d b1(p1, p2);
        const Bounds2d b2(p3, p4);
        const Bounds2d b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(3, 3);
        const Point2i p4(5, 5);
        const Bounds2i b1(p1, p2);
        const Bounds2i b2(p3, p4);
        const Bounds2i b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
}

TEST_CASE("Bounds2 inside", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(3, 3);
        const Bounds2f b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(3, 3);
        const Bounds2d b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(3, 3);
        const Bounds2i b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
}

TEST_CASE("Bounds2 inside exclusive", "[math][bounds2]")
{
    {
        const Point2f p1(2, 2);
        const Point2f p2(4, 4);
        const Point2f p3(3, 3);
        const Bounds2f b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
    {
        const Point2d p1(2, 2);
        const Point2d p2(4, 4);
        const Point2d p3(3, 3);
        const Bounds2d b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
    {
        const Point2i p1(2, 2);
        const Point2i p2(4, 4);
        const Point2i p3(3, 3);
        const Bounds2i b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
}

#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math/bounds3.h"

using Point3f = Opal::Point3<float>;
using Bounds3f = Opal::Bounds3<float>;
using Vector3f = Opal::Vector3<float>;
using Point3d = Opal::Point3<double>;
using Bounds3d = Opal::Bounds3<double>;
using Vector3d = Opal::Vector3<double>;
using Point3i = Opal::Point3<int32_t>;
using Bounds3i = Opal::Bounds3<int32_t>;
using Vector3i = Opal::Vector3<int32_t>;

TEST_CASE("Bounds3 constructor", "[math][bounds3]")
{
    {
        const Point3f p1(2, 4, 10);
        const Point3f p2(4, -2, 11);

        const Bounds3f b1(p1);
        const Bounds3f b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point3f(2, -2, 10));
        CHECK(b2.max == Point3f(4, 4, 11));
    }
    {
        const Point3d p1(2, 4, 10);
        const Point3d p2(4, -2, 11);

        const Bounds3d b1(p1);
        const Bounds3d b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point3d(2, -2, 10));
        CHECK(b2.max == Point3d(4, 4, 11));
    }
    {
        const Point3i p1(2, 4, 10);
        const Point3i p2(4, -2, 11);

        const Bounds3i b1(p1);
        const Bounds3i b2(p1, p2);

        CHECK(b1.min == p1);
        CHECK(b1.max == p1);

        CHECK(b2.min == Point3i(2, -2, 10));
        CHECK(b2.max == Point3i(4, 4, 11));
    }
}

TEST_CASE("Bounds3 access operator", "[math][bounds3]")
{
    {
        const Point3f p1(2, 3, 10);
        const Point3f p2(4, 5, 11);
        Bounds3f b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds3f b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point3f p3(5, 6, 7);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
    {
        const Point3d p1(2, 3, 10);
        const Point3d p2(4, 5, 11);
        Bounds3d b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds3d b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point3d p3(5, 6, 7);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
    {
        const Point3i p1(2, 3, 10);
        const Point3i p2(4, 5, 11);
        Bounds3i b1(p1, p2);
        CHECK(b1[0] == p1);
        CHECK(b1[1] == p2);
        const Bounds3i b2(p1, p2);
        CHECK(b2[0] == p1);
        CHECK(b2[1] == p2);

        const Point3i p3(5, 6, 7);
        b1[0] = p3;
        CHECK(b1[0] == p3);
        b1[1] = p3;
        CHECK(b1[1] == p3);
    }
}

TEST_CASE("Bounds3 comparison", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(2, 2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(2, 2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(2, 2, 2);

        CHECK(p1 == p3);
        CHECK_FALSE(p1 == p2);
        CHECK_FALSE(p2 == p3);
    }
}

TEST_CASE("Bounds3 corner", "[math][bounds3]")
{
    {
        const Point3f p1(2, 3, 10);
        const Point3f p2(4, 5, 11);
        const Bounds3f b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b000));
        CHECK(p2 == Corner(b1, 0b111));
        CHECK(Point3f(2, 5, 10) == Corner(b1, 0b010));
    }
    {
        const Point3d p1(2, 3, 10);
        const Point3d p2(4, 5, 11);
        const Bounds3d b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b000));
        CHECK(p2 == Corner(b1, 0b111));
        CHECK(Point3d(2, 5, 10) == Corner(b1, 0b010));
    }
    {
        const Point3i p1(2, 3, 10);
        const Point3i p2(4, 5, 11);
        const Bounds3i b1(p1, p2);

        CHECK(p1 == Corner(b1, 0b000));
        CHECK(p2 == Corner(b1, 0b111));
        CHECK(Point3i(2, 5, 10) == Corner(b1, 0b010));
    }
}

TEST_CASE("Bounds3 diagonal", "[math][bounds3]")
{
    {
        const Point3f p1(2, 3, 10);
        const Point3f p2(4, 5, 11);
        const Bounds3f b1(p1, p2);
        const Vector3f v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
    {
        const Point3d p1(2, 3, 10);
        const Point3d p2(4, 5, 11);
        const Bounds3d b1(p1, p2);
        const Vector3d v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
    {
        const Point3i p1(2, 3, 10);
        const Point3i p2(4, 5, 11);
        const Bounds3i b1(p1, p2);
        const Vector3i v = p2 - p1;
        CHECK(Diagonal(b1) == v);
    }
}

TEST_CASE("Bounds3 surface area", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Bounds3f b1(p1, p2);
        CHECK(SurfaceArea(b1) == 24.0f);
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Bounds3d b1(p1, p2);
        CHECK(SurfaceArea(b1) == 24.0);
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Bounds3i b1(p1, p2);
        CHECK(SurfaceArea(b1) == 24);
    }
}

TEST_CASE("Bounds3 volume", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Bounds3f b1(p1, p2);
        CHECK(Volume(b1) == 8.0f);
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Bounds3d b1(p1, p2);
        CHECK(Volume(b1) == 8.0);
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Bounds3i b1(p1, p2);
        CHECK(Volume(b1) == 8);
    }
}

TEST_CASE("Bounds3 maximum extent", "[math][bounds3]")
{
    {
        const Point3f p1(2, 3, 10);
        const Point3f p2(5, 5, 11);
        const Bounds3f b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
    {
        const Point3d p1(2, 3, 10);
        const Point3d p2(5, 5, 11);
        const Bounds3d b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
    {
        const Point3i p1(2, 3, 10);
        const Point3i p2(5, 5, 11);
        const Bounds3i b1(p1, p2);
        CHECK(MaximumExtent(b1) == 0);
    }
}

TEST_CASE("Bounds3 lerp", "[math][bounds3]")
{
    {
        const Point3f p1(2, 3, 10);
        const Point3f p2(5, 5, 11);
        const Bounds3f b1(p1, p2);
        const Point3f p = Lerp(b1, Point3f(0.5f, 0.5f, 0.5f));
        CHECK(Point3f(3.5f, 4.0f, 10.5f) == p);
    }
    {
        const Point3d p1(2, 3, 10);
        const Point3d p2(5, 5, 11);
        const Bounds3d b1(p1, p2);
        const Point3d p = Lerp(b1, Point3d(0.5f, 0.5f, 0.5f));
        CHECK(Point3d(3.5f, 4.0f, 10.5f) == p);
    }
}

TEST_CASE("Bounds3 offset", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(3, 3, 3);
        const Bounds3f b(p1, p2);
        CHECK(Offset(b, p3) == Vector3f(0.5f, 0.5f, 0.5f));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(3, 3, 3);
        const Bounds3d b(p1, p2);
        CHECK(Offset(b, p3) == Vector3d(0.5, 0.5, 0.5));
    }
}

TEST_CASE("Bounds3 bounding sphere", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Bounds3f b(p1, p2);

        Point3f center;
        float radius = 0.0f;
        BoundingSphere(b, center, radius);

        CHECK(center == Point3f(3, 3, 3));
        CHECK(radius == Opal::Sqrt(3.0f));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Bounds3d b(p1, p2);

        Point3d center;
        double radius = 0.0;
        BoundingSphere(b, center, radius);

        CHECK(center == Point3d(3, 3, 3));
        CHECK(radius == Opal::Sqrt(3.0));
    }
}

TEST_CASE("Bounds3 extent", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 3);
        const Point3f p2(5, 5, 4);
        const Bounds3f b(p1, p2);
        const Vector3f e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
        CHECK(e.z == 1);
    }
    {
        const Point3d p1(2, 2, 3);
        const Point3d p2(5, 5, 4);
        const Bounds3d b(p1, p2);
        const Vector3d e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
        CHECK(e.z == 1);
    }
    {
        const Point3i p1(2, 2, 3);
        const Point3i p2(5, 5, 4);
        const Bounds3i b(p1, p2);
        const Vector3i e = Extent(b);
        CHECK(e.x == 3);
        CHECK(e.y == 3);
        CHECK(e.z == 1);
    }
}

TEST_CASE("Bounds3 union", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(0, 0, 0);
        const Point3f p4(-2, -2, -2);
        const Bounds3f b1(p1, p2);
        const Bounds3f b2(p3, p4);
        const Bounds3f b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds3f b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(0, 0, 0);
        const Point3d p4(-2, -2, -2);
        const Bounds3d b1(p1, p2);
        const Bounds3d b2(p3, p4);
        const Bounds3d b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds3d b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(0, 0, 0);
        const Point3i p4(-2, -2, -2);
        const Bounds3i b1(p1, p2);
        const Bounds3i b2(p3, p4);
        const Bounds3i b3 = Union(b1, p3);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
        const Bounds3i b4 = Union(b1, b2);
        CHECK(b4.min == p4);
        CHECK(b4.max == p2);
    }
}

TEST_CASE("Bounds3 expand", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Bounds3f b1(p1, p2);
        const Bounds3f b2 = Expand(b1, 2.0f);
        CHECK(b2.min == Point3f(0, 0, 0));
        CHECK(b2.max == Point3f(6, 6, 6));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Bounds3d b1(p1, p2);
        const Bounds3d b2 = Expand(b1, 2.0);
        CHECK(b2.min == Point3d(0, 0, 0));
        CHECK(b2.max == Point3d(6, 6, 6));
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Bounds3i b1(p1, p2);
        const Bounds3i b2 = Expand(b1, 2);
        CHECK(b2.min == Point3i(0, 0, 0));
        CHECK(b2.max == Point3i(6, 6, 6));
    }
}

TEST_CASE("Bounds3 overlaps", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(3, 3, 3);
        const Point3f p4(5, 5, 5);
        const Bounds3f b1(p1, p2);
        const Bounds3f b2(p3, p4);
        const Bounds3f b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(3, 3, 3);
        const Point3d p4(5, 5, 5);
        const Bounds3d b1(p1, p2);
        const Bounds3d b2(p3, p4);
        const Bounds3d b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(3, 3, 3);
        const Point3i p4(5, 5, 5);
        const Bounds3i b1(p1, p2);
        const Bounds3i b2(p3, p4);
        const Bounds3i b3(p2, p4);
        CHECK(Overlaps(b1, b2));
        CHECK(Overlaps(b1, b3));
    }
}

TEST_CASE("Bounds3 intersects", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(3, 3, 3);
        const Point3f p4(5, 5, 5);
        const Bounds3f b1(p1, p2);
        const Bounds3f b2(p3, p4);
        const Bounds3f b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(3, 3, 3);
        const Point3d p4(5, 5, 5);
        const Bounds3d b1(p1, p2);
        const Bounds3d b2(p3, p4);
        const Bounds3d b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(3, 3, 3);
        const Point3i p4(5, 5, 5);
        const Bounds3i b1(p1, p2);
        const Bounds3i b2(p3, p4);
        const Bounds3i b3 = Intersect(b1, b2);
        CHECK(b3.min == p3);
        CHECK(b3.max == p2);
    }
}

TEST_CASE("Bounds3 inside", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(3, 3, 3);
        const Bounds3f b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(3, 3, 3);
        const Bounds3d b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(3, 3, 3);
        const Bounds3i b1(p1, p2);
        CHECK(Inside(b1, p3));
        CHECK(Inside(b1, p1));
        CHECK_FALSE(Inside(b1, p2));
    }
}

TEST_CASE("Bounds3 inside exclusive", "[math][bounds3]")
{
    {
        const Point3f p1(2, 2, 2);
        const Point3f p2(4, 4, 4);
        const Point3f p3(3, 3, 3);
        const Bounds3f b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
    {
        const Point3d p1(2, 2, 2);
        const Point3d p2(4, 4, 4);
        const Point3d p3(3, 3, 3);
        const Bounds3d b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
    {
        const Point3i p1(2, 2, 2);
        const Point3i p2(4, 4, 4);
        const Point3i p3(3, 3, 3);
        const Bounds3i b1(p1, p2);
        CHECK(InsideInclusive(b1, p3));
        CHECK(InsideInclusive(b1, p1));
        CHECK(InsideInclusive(b1, p2));
    }
}

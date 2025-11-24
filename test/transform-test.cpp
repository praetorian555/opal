#include "test-helpers.h"

#include "opal/math/transform.h"

using namespace Opal;

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;
using Point3i = Point3<int>;
using Point4f = Point4<float>;
using Point4d = Point4<double>;
using Matrix4f = Matrix4x4<float>;
using Matrix4d = Matrix4x4<double>;
using Quatf = Quaternion<float>;
using Quatd = Quaternion<double>;
using Rotf = Rotator<float>;
using Rotd = Rotator<double>;
using Normal3f = Normal3<float>;
using Normal3d = Normal3<double>;

TEST_CASE("Transform identity", "[math][transform]")
{
    SECTION("float")
    {
        const Matrix4f m = Identity<f32>();
        CHECK(m(0, 0) == 1.0f);
        CHECK(m(0, 1) == 0.0f);
        CHECK(m(0, 2) == 0.0f);
        CHECK(m(0, 3) == 0.0f);
        CHECK(m(1, 0) == 0.0f);
        CHECK(m(1, 1) == 1.0f);
        CHECK(m(1, 2) == 0.0f);
        CHECK(m(1, 3) == 0.0f);
        CHECK(m(2, 0) == 0.0f);
        CHECK(m(2, 1) == 0.0f);
        CHECK(m(2, 2) == 1.0f);
        CHECK(m(2, 3) == 0.0f);
        CHECK(m(3, 0) == 0.0f);
        CHECK(m(3, 1) == 0.0f);
        CHECK(m(3, 2) == 0.0f);
        CHECK(m(3, 3) == 1.0f);
    }
    SECTION("double")
    {
        const Matrix4d m = Identity<f64>();
        CHECK(m(0, 0) == 1.0);
        CHECK(m(0, 1) == 0.0);
        CHECK(m(0, 2) == 0.0);
        CHECK(m(0, 3) == 0.0);
        CHECK(m(1, 0) == 0.0);
        CHECK(m(1, 1) == 1.0);
        CHECK(m(1, 2) == 0.0);
        CHECK(m(1, 3) == 0.0);
        CHECK(m(2, 0) == 0.0);
        CHECK(m(2, 1) == 0.0);
        CHECK(m(2, 2) == 1.0);
        CHECK(m(2, 3) == 0.0);
        CHECK(m(3, 0) == 0.0);
        CHECK(m(3, 1) == 0.0);
        CHECK(m(3, 2) == 0.0);
        CHECK(m(3, 3) == 1.0);
    }
}

TEST_CASE("Transform translation", "[math][transform]")
{
    SECTION("float")
    {
        const Matrix4f t1 = Translate(Vector3f(2, 3, 5));
        CHECK(t1(0, 0) == 1.0f);
        CHECK(t1(0, 1) == 0.0f);
        CHECK(t1(0, 2) == 0.0f);
        CHECK(t1(0, 3) == 2.0f);
        CHECK(t1(1, 0) == 0.0f);
        CHECK(t1(1, 1) == 1.0f);
        CHECK(t1(1, 2) == 0.0f);
        CHECK(t1(1, 3) == 3.0f);
        CHECK(t1(2, 0) == 0.0f);
        CHECK(t1(2, 1) == 0.0f);
        CHECK(t1(2, 2) == 1.0f);
        CHECK(t1(2, 3) == 5.0f);
        CHECK(t1(3, 0) == 0.0f);
        CHECK(t1(3, 1) == 0.0f);
        CHECK(t1(3, 2) == 0.0f);
        CHECK(t1(3, 3) == 1.0f);

        const Matrix4f t2 = Translate(Point3f(2, 3, 5));
        CHECK(t2(0, 0) == 1.0f);
        CHECK(t2(0, 1) == 0.0f);
        CHECK(t2(0, 2) == 0.0f);
        CHECK(t2(0, 3) == 2.0f);
        CHECK(t2(1, 0) == 0.0f);
        CHECK(t2(1, 1) == 1.0f);
        CHECK(t2(1, 2) == 0.0f);
        CHECK(t2(1, 3) == 3.0f);
        CHECK(t2(2, 0) == 0.0f);
        CHECK(t2(2, 1) == 0.0f);
        CHECK(t2(2, 2) == 1.0f);
        CHECK(t2(2, 3) == 5.0f);
        CHECK(t2(3, 0) == 0.0f);
        CHECK(t2(3, 1) == 0.0f);
        CHECK(t2(3, 2) == 0.0f);
        CHECK(t2(3, 3) == 1.0f);
    }
    SECTION("double")
    {
        const Matrix4d t1 = Translate(Vector3d(2, 3, 5));
        CHECK(t1(0, 0) == 1.0);
        CHECK(t1(0, 1) == 0.0);
        CHECK(t1(0, 2) == 0.0);
        CHECK(t1(0, 3) == 2.0);
        CHECK(t1(1, 0) == 0.0);
        CHECK(t1(1, 1) == 1.0);
        CHECK(t1(1, 2) == 0.0);
        CHECK(t1(1, 3) == 3.0);
        CHECK(t1(2, 0) == 0.0);
        CHECK(t1(2, 1) == 0.0);
        CHECK(t1(2, 2) == 1.0);
        CHECK(t1(2, 3) == 5.0);
        CHECK(t1(3, 0) == 0.0);
        CHECK(t1(3, 1) == 0.0);
        CHECK(t1(3, 2) == 0.0);
        CHECK(t1(3, 3) == 1.0);

        const Matrix4d t2 = Translate(Point3d(2, 3, 5));
        CHECK(t2(0, 0) == 1.0);
        CHECK(t2(0, 1) == 0.0);
        CHECK(t2(0, 2) == 0.0);
        CHECK(t2(0, 3) == 2.0);
        CHECK(t2(1, 0) == 0.0);
        CHECK(t2(1, 1) == 1.0);
        CHECK(t2(1, 2) == 0.0);
        CHECK(t2(1, 3) == 3.0);
        CHECK(t2(2, 0) == 0.0);
        CHECK(t2(2, 1) == 0.0);
        CHECK(t2(2, 2) == 1.0);
        CHECK(t2(2, 3) == 5.0);
        CHECK(t2(3, 0) == 0.0);
        CHECK(t2(3, 1) == 0.0);
        CHECK(t2(3, 2) == 0.0);
        CHECK(t2(3, 3) == 1.0);
    }
}

TEST_CASE("Transform scaling", "[math][transform]")
{
    SECTION("float")
    {
        const Matrix4f s1 = Scale<float>(2, 3, 5);
        CHECK(s1(0, 0) == 2.0f);
        CHECK(s1(0, 1) == 0.0f);
        CHECK(s1(0, 2) == 0.0f);
        CHECK(s1(0, 3) == 0.0f);
        CHECK(s1(1, 0) == 0.0f);
        CHECK(s1(1, 1) == 3.0f);
        CHECK(s1(1, 2) == 0.0f);
        CHECK(s1(1, 3) == 0.0f);
        CHECK(s1(2, 0) == 0.0f);
        CHECK(s1(2, 1) == 0.0f);
        CHECK(s1(2, 2) == 5.0f);
        CHECK(s1(2, 3) == 0.0f);
        CHECK(s1(3, 0) == 0.0f);
        CHECK(s1(3, 1) == 0.0f);
        CHECK(s1(3, 2) == 0.0f);
        CHECK(s1(3, 3) == 1.0f);

        const Matrix4f s2 = Scale<float>(2);
        CHECK(s2(0, 0) == 2.0f);
        CHECK(s2(0, 1) == 0.0f);
        CHECK(s2(0, 2) == 0.0f);
        CHECK(s2(0, 3) == 0.0f);
        CHECK(s2(1, 0) == 0.0f);
        CHECK(s2(1, 1) == 2.0f);
        CHECK(s2(1, 2) == 0.0f);
        CHECK(s2(1, 3) == 0.0f);
        CHECK(s2(2, 0) == 0.0f);
        CHECK(s2(2, 1) == 0.0f);
        CHECK(s2(2, 2) == 2.0f);
        CHECK(s2(2, 3) == 0.0f);
        CHECK(s2(3, 0) == 0.0f);
        CHECK(s2(3, 1) == 0.0f);
        CHECK(s2(3, 2) == 0.0f);
        CHECK(s2(3, 3) == 1.0f);
    }
    SECTION("double")
    {
        const Matrix4d s1 = Scale<double>(2, 3, 5);
        CHECK(s1(0, 0) == 2.0);
        CHECK(s1(0, 1) == 0.0);
        CHECK(s1(0, 2) == 0.0);
        CHECK(s1(0, 3) == 0.0);
        CHECK(s1(1, 0) == 0.0);
        CHECK(s1(1, 1) == 3.0);
        CHECK(s1(1, 2) == 0.0);
        CHECK(s1(1, 3) == 0.0);
        CHECK(s1(2, 0) == 0.0);
        CHECK(s1(2, 1) == 0.0);
        CHECK(s1(2, 2) == 5.0);
        CHECK(s1(2, 3) == 0.0);
        CHECK(s1(3, 0) == 0.0);
        CHECK(s1(3, 1) == 0.0);
        CHECK(s1(3, 2) == 0.0);
        CHECK(s1(3, 3) == 1.0);

        const Matrix4d s2 = Scale<double>(2);
        CHECK(s2(0, 0) == 2.0);
        CHECK(s2(0, 1) == 0.0);
        CHECK(s2(0, 2) == 0.0);
        CHECK(s2(0, 3) == 0.0);
        CHECK(s2(1, 0) == 0.0);
        CHECK(s2(1, 1) == 2.0);
        CHECK(s2(1, 2) == 0.0);
        CHECK(s2(1, 3) == 0.0);
        CHECK(s2(2, 0) == 0.0);
        CHECK(s2(2, 1) == 0.0);
        CHECK(s2(2, 2) == 2.0);
        CHECK(s2(2, 3) == 0.0);
        CHECK(s2(3, 0) == 0.0);
        CHECK(s2(3, 1) == 0.0);
        CHECK(s2(3, 2) == 0.0);
        CHECK(s2(3, 3) == 1.0);
    }
}

TEST_CASE("Transform rotation", "[math][transform]")
{
    SECTION("float")
    {
        const float degrees = 45.0f;
        const float s = Sin(Radians(degrees));
        const float c = Cos(Radians(degrees));

        const Matrix4f around_x = RotateX(degrees);
        CHECK(around_x(0, 0) == 1.0f);
        CHECK(around_x(0, 1) == 0.0f);
        CHECK(around_x(0, 2) == 0.0f);
        CHECK(around_x(0, 3) == 0.0f);
        CHECK(around_x(1, 0) == 0.0f);
        CHECK(around_x(1, 1) == c);
        CHECK(around_x(1, 2) == -s);
        CHECK(around_x(1, 3) == 0.0f);
        CHECK(around_x(2, 0) == 0.0f);
        CHECK(around_x(2, 1) == s);
        CHECK(around_x(2, 2) == c);
        CHECK(around_x(2, 3) == 0.0f);
        CHECK(around_x(3, 0) == 0.0f);
        CHECK(around_x(3, 1) == 0.0f);
        CHECK(around_x(3, 2) == 0.0f);
        CHECK(around_x(3, 3) == 1.0f);

        const Matrix4f around_y = RotateY(degrees);
        CHECK(around_y(0, 0) == c);
        CHECK(around_y(0, 1) == 0.0f);
        CHECK(around_y(0, 2) == s);
        CHECK(around_y(0, 3) == 0.0f);
        CHECK(around_y(1, 0) == 0.0f);
        CHECK(around_y(1, 1) == 1.0f);
        CHECK(around_y(1, 2) == 0.0f);
        CHECK(around_y(1, 3) == 0.0f);
        CHECK(around_y(2, 0) == -s);
        CHECK(around_y(2, 1) == 0.0f);
        CHECK(around_y(2, 2) == c);
        CHECK(around_y(2, 3) == 0.0f);
        CHECK(around_y(3, 0) == 0.0f);
        CHECK(around_y(3, 1) == 0.0f);
        CHECK(around_y(3, 2) == 0.0f);
        CHECK(around_y(3, 3) == 1.0f);

        const Matrix4f around_z = RotateZ(degrees);
        CHECK(around_z(0, 0) == c);
        CHECK(around_z(0, 1) == -s);
        CHECK(around_z(0, 2) == 0.0f);
        CHECK(around_z(0, 3) == 0.0f);
        CHECK(around_z(1, 0) == s);
        CHECK(around_z(1, 1) == c);
        CHECK(around_z(1, 2) == 0.0f);
        CHECK(around_z(1, 3) == 0.0f);
        CHECK(around_z(2, 0) == 0.0f);
        CHECK(around_z(2, 1) == 0.0f);
        CHECK(around_z(2, 2) == 1.0f);
        CHECK(around_z(2, 3) == 0.0f);
        CHECK(around_z(3, 0) == 0.0f);
        CHECK(around_z(3, 1) == 0.0f);
        CHECK(around_z(3, 2) == 0.0f);
        CHECK(around_z(3, 3) == 1.0f);

        const Matrix4f around_axis = Rotate(degrees, Vector3f(0, 0, 1));
        CHECK(IsEqual(around_axis, around_z, k_machine_epsilon_float));
    }
    SECTION("double")
    {
        const double degrees = 45.0;
        const double s = Sin(Radians(degrees));
        const double c = Cos(Radians(degrees));

        const Matrix4d around_x = RotateX(degrees);
        CHECK(around_x(0, 0) == 1.0);
        CHECK(around_x(0, 1) == 0.0);
        CHECK(around_x(0, 2) == 0.0);
        CHECK(around_x(0, 3) == 0.0);
        CHECK(around_x(1, 0) == 0.0);
        CHECK(around_x(1, 1) == c);
        CHECK(around_x(1, 2) == -s);
        CHECK(around_x(1, 3) == 0.0);
        CHECK(around_x(2, 0) == 0.0);
        CHECK(around_x(2, 1) == s);
        CHECK(around_x(2, 2) == c);
        CHECK(around_x(2, 3) == 0.0);
        CHECK(around_x(3, 0) == 0.0);
        CHECK(around_x(3, 1) == 0.0);
        CHECK(around_x(3, 2) == 0.0);
        CHECK(around_x(3, 3) == 1.0);

        const Matrix4d around_y = RotateY(degrees);
        CHECK(around_y(0, 0) == c);
        CHECK(around_y(0, 1) == 0.0);
        CHECK(around_y(0, 2) == s);
        CHECK(around_y(0, 3) == 0.0);
        CHECK(around_y(1, 0) == 0.0);
        CHECK(around_y(1, 1) == 1.0);
        CHECK(around_y(1, 2) == 0.0);
        CHECK(around_y(1, 3) == 0.0);
        CHECK(around_y(2, 0) == -s);
        CHECK(around_y(2, 1) == 0.0);
        CHECK(around_y(2, 2) == c);
        CHECK(around_y(2, 3) == 0.0);
        CHECK(around_y(3, 0) == 0.0);
        CHECK(around_y(3, 1) == 0.0);
        CHECK(around_y(3, 2) == 0.0);
        CHECK(around_y(3, 3) == 1.0);

        const Matrix4d around_z = RotateZ(degrees);
        CHECK(around_z(0, 0) == c);
        CHECK(around_z(0, 1) == -s);
        CHECK(around_z(0, 2) == 0.0);
        CHECK(around_z(0, 3) == 0.0);
        CHECK(around_z(1, 0) == s);
        CHECK(around_z(1, 1) == c);
        CHECK(around_z(1, 2) == 0.0);
        CHECK(around_z(1, 3) == 0.0);
        CHECK(around_z(2, 0) == 0.0);
        CHECK(around_z(2, 1) == 0.0);
        CHECK(around_z(2, 2) == 1.0);
        CHECK(around_z(2, 3) == 0.0);
        CHECK(around_z(3, 0) == 0.0);
        CHECK(around_z(3, 1) == 0.0);
        CHECK(around_z(3, 2) == 0.0);
        CHECK(around_z(3, 3) == 1.0);

        const Matrix4d around_axis = Rotate(degrees, Vector3d(0, 0, 1));
        CHECK(IsEqual(around_axis, around_z, k_machine_epsilon_double));
    }
}

TEST_CASE("Transform rotation with quaternions", "[math][transform]")
{
    SECTION("float - Rotate around Z by 45 degrees")
    {
        const Quatf q = Quatf::FromAxisAngleDegrees(Vector3f(0, 0, 1), 45.0f);
        const Matrix4f t = Rotate(q);
        const Matrix4f ref = RotateZ(45.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    SECTION("double - Rotate around Z by 45 degrees")
    {
        const Quatd q = Quatd::FromAxisAngleDegrees(Vector3d(0, 0, 1), 45.0);
        const Matrix4d t = Rotate(q);
        const Matrix4d ref = RotateZ(45.0);
        CHECK(IsEqual(t, ref, k_machine_epsilon_double));
    }
    SECTION("float - Rotate around X by 45 degrees")
    {
        const Quatf q = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 45.0f);
        const Matrix4f t = Rotate(q);
        const Matrix4f ref = RotateX(45.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
}

TEST_CASE("Transform rotation with rotator", "[math][transform]")
{
    SECTION("float")
    {
        const Rotf rot1(45.0f, 0.0f, 0.0f);
        const Matrix4f t1 = Rotate(rot1);
        const Matrix4f ref1 = RotateZ(45.0f);
        CHECK(IsEqual(t1, ref1, k_machine_epsilon_float));
        const Rotf rot2(0.0f, 45.0f, 0.0f);
        const Matrix4f t2 = Rotate(rot2);
        const Matrix4f ref2 = RotateY(45.0f);
        CHECK(IsEqual(t2, ref2, k_machine_epsilon_float));
        const Rotf rot3(0.0f, 0.0f, 45.0f);
        const Matrix4f t3 = Rotate(rot3);
        const Matrix4f ref3 = RotateX(45.0f);
        CHECK(IsEqual(t3, ref3, k_machine_epsilon_float));
    }
    SECTION("double")
    {
        const Rotd rot1(45.0, 0.0, 0.0);
        const Matrix4d t1 = Rotate(rot1);
        const Matrix4d ref1 = RotateZ(45.0);
        CHECK(IsEqual(t1, ref1, k_machine_epsilon_double));
        const Rotd rot2(0.0, 45.0, 0.0);
        const Matrix4d t2 = Rotate(rot2);
        const Matrix4d ref2 = RotateY(45.0);
        CHECK(IsEqual(t2, ref2, k_machine_epsilon_double));
        const Rotd rot3(0.0, 0.0, 45.0);
        const Matrix4d t3 = Rotate(rot3);
        const Matrix4d ref3 = RotateX(45.0);
        CHECK(IsEqual(t3, ref3, k_machine_epsilon_double));
    }
}

TEST_CASE("Transform rotation and translation", "[math][transform]")
{
    SECTION("float")
    {
        const float degrees = 45;
        const Rotf rot1(degrees, 0, 0);
        const float s1 = Sin(Radians(degrees));
        const float c1 = Cos(Radians(degrees));
        const Matrix4f t1 = RotateAndTranslate(rot1, Vector3f(2, 3, 5));
        CHECK(t1(0, 0) == c1);
        CHECK(t1(0, 1) == -s1);
        CHECK(t1(0, 2) == 0.0f);
        CHECK(t1(0, 3) == 2.0f);
        CHECK(t1(1, 0) == s1);
        CHECK(t1(1, 1) == c1);
        CHECK(t1(1, 2) == 0.0f);
        CHECK(t1(1, 3) == 3.0f);
        CHECK(t1(2, 0) == 0.0f);
        CHECK(t1(2, 1) == 0.0f);
        CHECK(t1(2, 2) == 1.0f);
        CHECK(t1(2, 3) == 5.0f);
        CHECK(t1(3, 0) == 0.0f);
        CHECK(t1(3, 1) == 0.0f);
        CHECK(t1(3, 2) == 0.0f);
        CHECK(t1(3, 3) == 1.0f);

        const Matrix4f t2 = RotateAndTranslate(rot1, Point3f(2, 3, 5));
        CHECK(t2(0, 0) == c1);
        CHECK(t2(0, 1) == -s1);
        CHECK(t2(0, 2) == 0.0f);
        CHECK(t2(0, 3) == 2.0f);
        CHECK(t2(1, 0) == s1);
        CHECK(t2(1, 1) == c1);
        CHECK(t2(1, 2) == 0.0f);
        CHECK(t2(1, 3) == 3.0f);
        CHECK(t2(2, 0) == 0.0f);
        CHECK(t2(2, 1) == 0.0f);
        CHECK(t2(2, 2) == 1.0f);
        CHECK(t2(2, 3) == 5.0f);
        CHECK(t2(3, 0) == 0.0f);
        CHECK(t2(3, 1) == 0.0f);
        CHECK(t2(3, 2) == 0.0f);
        CHECK(t2(3, 3) == 1.0f);
    }
    SECTION("double")
    {
        const double degrees = 45;
        const Rotd rot1(degrees, 0, 0);
        const double s1 = Sin(Radians(degrees));
        const double c1 = Cos(Radians(degrees));
        const Matrix4d t1 = RotateAndTranslate(rot1, Vector3d(2, 3, 5));
        CHECK(t1(0, 0) == c1);
        CHECK(t1(0, 1) == -s1);
        CHECK(t1(0, 2) == 0);
        CHECK(t1(0, 3) == 2);
        CHECK(t1(1, 0) == s1);
        CHECK(t1(1, 1) == c1);
        CHECK(t1(1, 2) == 0);
        CHECK(t1(1, 3) == 3);
        CHECK(t1(2, 0) == 0);
        CHECK(t1(2, 1) == 0);
        CHECK(t1(2, 2) == 1);
        CHECK(t1(2, 3) == 5);
        CHECK(t1(3, 0) == 0);
        CHECK(t1(3, 1) == 0);
        CHECK(t1(3, 2) == 0);
        CHECK(t1(3, 3) == 1);

        const Matrix4d t2 = RotateAndTranslate(rot1, Point3d(2, 3, 5));
        CHECK(t2(0, 0) == c1);
        CHECK(t2(0, 1) == -s1);
        CHECK(t2(0, 2) == 0);
        CHECK(t2(0, 3) == 2);
        CHECK(t2(1, 0) == s1);
        CHECK(t2(1, 1) == c1);
        CHECK(t2(1, 2) == 0);
        CHECK(t2(1, 3) == 3);
        CHECK(t2(2, 0) == 0);
        CHECK(t2(2, 1) == 0);
        CHECK(t2(2, 2) == 1);
        CHECK(t2(2, 3) == 5);
        CHECK(t2(3, 0) == 0);
        CHECK(t2(3, 1) == 0);
        CHECK(t2(3, 2) == 0);
        CHECK(t2(3, 3) == 1);
    }
}

TEST_CASE("Transform applying", "[math][transform]")
{
    SECTION("float")
    {
        const Matrix4f t1 = Translate(Vector3f(2, 3, 5));
        const Matrix4f t2 = Scale(2.0f);
        const Point3f p1(1, 1, 1);
        const Point3f p2 = t1 * p1;
        CHECK(p2.x == 3.0f);
        CHECK(p2.y == 4.0f);
        CHECK(p2.z == 6.0f);
        const Point4f p3 = t1 * Point4f(1, 1, 1, 1);
        CHECK(p3.x == 3.0f);
        CHECK(p3.y == 4.0f);
        CHECK(p3.z == 6.0f);
        CHECK(p3.w == 1.0f);
        const Vector3f v1(1, 1, 1);
        const Vector3f v2 = t1 * v1;
        CHECK(v2.x == 1.0f);
        CHECK(v2.y == 1.0f);
        CHECK(v2.z == 1.0f);
        const Vector4f v3 = t1 * Vector4f(1, 1, 1, 1);
        CHECK(v3.x == 3.0f);
        CHECK(v3.y == 4.0f);
        CHECK(v3.z == 6.0f);
        CHECK(v3.w == 1.0f);
        const Vector3f v4 = t2 * v1;
        CHECK(v4.x == 2.0f);
        CHECK(v4.y == 2.0f);
        CHECK(v4.z == 2.0f);
        const Vector4f v5 = t2 * Vector4f(1, 1, 1, 1);
        CHECK(v5.x == 2.0f);
        CHECK(v5.y == 2.0f);
        CHECK(v5.z == 2.0f);
        CHECK(v5.w == 1.0f);
        const Normal3f n1(1, 1, 1);
        const Normal3f n2 = t1 * n1;
        CHECK(n2.x == 1.0f);
        CHECK(n2.y == 1.0f);
        CHECK(n2.z == 1.0f);
    }
    SECTION("double")
    {
        const Matrix4d t1 = Translate(Vector3d(2, 3, 5));
        const Matrix4d t2 = Scale(2.0);
        const Point3d p1(1, 1, 1);
        const Point3d p2 = t1 * p1;
        CHECK(p2.x == 3);
        CHECK(p2.y == 4);
        CHECK(p2.z == 6);
        const Point4d p3 = t1 * Point4d(1, 1, 1, 1);
        CHECK(p3.x == 3);
        CHECK(p3.y == 4);
        CHECK(p3.z == 6);
        CHECK(p3.w == 1);
        const Vector3d v1(1, 1, 1);
        const Vector3d v2 = t1 * v1;
        CHECK(v2.x == 1);
        CHECK(v2.y == 1);
        CHECK(v2.z == 1);
        const Vector4d v3 = t1 * Vector4d(1, 1, 1, 1);
        CHECK(v3.x == 3);
        CHECK(v3.y == 4);
        CHECK(v3.z == 6);
        CHECK(v3.w == 1);
        const Vector3d v4 = t2 * v1;
        CHECK(v4.x == 2);
        CHECK(v4.y == 2);
        CHECK(v4.z == 2);
        const Vector4d v5 = t2 * Vector4d(1, 1, 1, 1);
        CHECK(v5.x == 2);
        CHECK(v5.y == 2);
        CHECK(v5.z == 2);
        CHECK(v5.w == 1);
        const Normal3d n1(1, 1, 1);
        const Normal3d n2 = t1 * n1;
        CHECK(n2.x == 1);
        CHECK(n2.y == 1);
        CHECK(n2.z == 1);
    }
}

TEST_CASE("Transform look at right-handed coordinate system", "[math][transform]")
{
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(0, 0, 1);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_RH(eye, target, up);
        const Matrix4f ref = RotateY(-180.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(0, 0, -1);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_RH(eye, target, up);
        const Matrix4f ref = Identity<float>();
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(1, 0, 0);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_RH(eye, target, up);
        const Matrix4f ref = RotateY(90.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 5);
        const Point3f target(-5, 0, 0);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_RH(eye, target, up);
        const Matrix4f ref = RotateY(-45.0f) * Translate(Point3<float>(0, 0, -5));
        CHECK(IsEqual(t, ref, 10 * k_machine_epsilon_float));
    }
}

TEST_CASE("Transform look at with left-handed coordinate system", "[math][transform]")
{
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(0, 0, 1);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_LH(eye, target, up);
        const Matrix4f ref = Identity<float>();
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(0, 0, -1);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_LH(eye, target, up);
        const Matrix4f ref = RotateY(-180.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 0);
        const Point3f target(1, 0, 0);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_LH(eye, target, up);
        const Matrix4f ref = RotateY(-90.0f);
        CHECK(IsEqual(t, ref, k_machine_epsilon_float));
    }
    {
        const Point3f eye(0, 0, 5);
        const Point3f target(-5, 0, 0);
        const Vector3f up(0, 1, 0);
        const Matrix4f t = LookAt_LH(eye, target, up);
        Matrix4f ref = RotateY(135.0f) * Translate(Point3<float>(0, 0, -5));
        CHECK(IsEqual(t, ref, 10 * k_machine_epsilon_float));
    }
}

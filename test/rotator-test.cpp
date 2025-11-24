#include "test-helpers.h"

#include "opal/math/rotator.h"

using namespace Opal;

using Rotf = Rotator<float>;
using Vector3f = Vector3<float>;
using Rotd = Rotator<double>;
using Vector3d = Vector3<double>;

TEST_CASE("Rotator constructors", "[math][rotator]")
{
    {
        const Rotf rot1 = Rotf::Zero();
        CHECK(rot1.pitch == 0.0f);
        CHECK(rot1.yaw == 0.0f);
        CHECK(rot1.roll == 0.0f);
        const Rotf rot2(10, 50, -30);
        CHECK(rot2.pitch == 10.0f);
        CHECK(rot2.yaw == 50.0f);
        CHECK(rot2.roll == -30.0f);
    }
    {
        const Rotd rot1 = Rotd::Zero();
        CHECK(rot1.pitch == 0.0);
        CHECK(rot1.yaw == 0.0);
        CHECK(rot1.roll == 0.0);
        const Rotd rot2(10, 50, -30);
        CHECK(rot2.pitch == 10.0);
        CHECK(rot2.yaw == 50.0);
        CHECK(rot2.roll == -30.0);
    }
}

TEST_CASE("Rotator comparison", "[math][rotator]")
{
    {
        const Rotf rot1(10, 15, 20);
        const Rotf rot2(15, 10, 20);
        const Rotf rot3(10, 15, 20);
        CHECK(rot1 == rot3);
        CHECK(rot1 != rot2);
    }
    {
        const Rotd rot1(10, 15, 20);
        const Rotd rot2(15, 10, 20);
        const Rotd rot3(10, 15, 20);
        CHECK(rot1 == rot3);
        CHECK(rot1 != rot2);
    }
}

TEST_CASE("Rotator addition", "[math][rotator]")
{
    {
        Rotf rot1(10, 15, 34);
        const Rotf rot2(12, 23, 64);

        const Rotf rot3 = rot1 + rot2;
        CHECK(rot3.pitch == 22.0f);
        CHECK(rot3.yaw == 38.0f);
        CHECK(rot3.roll == 98.0f);

        rot1 += rot2;
        CHECK(rot1.pitch == 22.0f);
        CHECK(rot1.yaw == 38.0f);
        CHECK(rot1.roll == 98.0f);
    }
    {
        Rotd rot1(10, 15, 34);
        const Rotd rot2(12, 23, 64);

        const Rotd rot3 = rot1 + rot2;
        CHECK(rot3.pitch == 22);
        CHECK(rot3.yaw == 38);
        CHECK(rot3.roll == 98);

        rot1 += rot2;
        CHECK(rot1.pitch == 22);
        CHECK(rot1.yaw == 38);
        CHECK(rot1.roll == 98);
    }
}

TEST_CASE("Rotator subtraction", "[math][rotator]")
{
    {
        Rotf rot1(10, 15, 34);
        const Rotf rot2(12, 23, 64);

        const Rotf rot3 = rot1 - rot2;
        CHECK(rot3.pitch == -2.0f);
        CHECK(rot3.yaw == -8.0f);
        CHECK(rot3.roll == -30.0f);

        rot1 -= rot2;
        CHECK(rot1.pitch == -2.0f);
        CHECK(rot1.yaw == -8.0f);
        CHECK(rot1.roll == -30.0f);
    }
    {
        Rotd rot1(10, 15, 34);
        const Rotd rot2(12, 23, 64);

        const Rotd rot3 = rot1 - rot2;
        CHECK(rot3.pitch == -2);
        CHECK(rot3.yaw == -8);
        CHECK(rot3.roll == -30);

        rot1 -= rot2;
        CHECK(rot1.pitch == -2);
        CHECK(rot1.yaw == -8);
        CHECK(rot1.roll == -30);
    }
}

TEST_CASE("Rotator multiplication by scalar", "[math][rotator]")
{
    {
        Rotf rot1(10, 15, 34);

        const Rotf rot2 = 2.0f * rot1;
        CHECK(rot2.pitch == 20.0f);
        CHECK(rot2.yaw == 30.0f);
        CHECK(rot2.roll == 68.0f);

        rot1 *= 2;
        CHECK(rot1.pitch == 20.0f);
        CHECK(rot1.yaw == 30.0f);
        CHECK(rot1.roll == 68.0f);
    }
    {
        Rotd rot1(10, 15, 34);

        const Rotd rot2 = 2.0 * rot1;
        CHECK(rot2.pitch == 20);
        CHECK(rot2.yaw == 30);
        CHECK(rot2.roll == 68);

        rot1 *= 2;
        CHECK(rot1.pitch == 20);
        CHECK(rot1.yaw == 30);
        CHECK(rot1.roll == 68);
    }
}

TEST_CASE("Rotator to vector", "[math][rotator]")
{
    {
        const Rotf rot1(90, 0, 0);
        const Vector3f vec1 = RotatorToVector(rot1);
        const Vector3f ref1 = Vector3f{0, 1, 0};
        CHECK(IsEqual(vec1.x, ref1.x, 0.00001f));
        CHECK(IsEqual(vec1.y, ref1.y, 0.00001f));
        CHECK(IsEqual(vec1.z, ref1.z, 0.00001f));

        const Rotf rot2(0, 270, 0);
        const Vector3f vec2 = RotatorToVector(rot2);
        const Vector3f ref2 = Vector3f{0, 0, 1};
        CHECK(IsEqual(vec2.x, ref2.x, 0.00001f));
        CHECK(IsEqual(vec2.y, ref2.y, 0.00001f));
        CHECK(IsEqual(vec2.z, ref2.z, 0.00001f));

        const Rotf rot3(0, 0, 35);
        const Vector3f vec3 = RotatorToVector(rot3);
        const Vector3f ref3 = Vector3f{1, 0, 0};
        CHECK(IsEqual(vec3.x, ref3.x, 0.00001f));
        CHECK(IsEqual(vec3.y, ref3.y, 0.00001f));
        CHECK(IsEqual(vec3.z, ref3.z, 0.00001f));
    }
    {
        const Rotd rot1(90, 0, 0);
        const Vector3d vec1 = RotatorToVector(rot1);
        const Vector3d ref1 = Vector3d{0, 1, 0};
        CHECK(IsEqual(vec1.x, ref1.x, 0.00001));
        CHECK(IsEqual(vec1.y, ref1.y, 0.00001));
        CHECK(IsEqual(vec1.z, ref1.z, 0.00001));

        const Rotd rot2(0, 270, 0);
        const Vector3d vec2 = RotatorToVector(rot2);
        const Vector3d ref2 = Vector3d{0, 0, 1};
        CHECK(IsEqual(vec2.x, ref2.x, 0.00001));
        CHECK(IsEqual(vec2.y, ref2.y, 0.00001));
        CHECK(IsEqual(vec2.z, ref2.z, 0.00001));

        const Rotd rot3(0, 0, 35);
        const Vector3d vec3 = RotatorToVector(rot3);
        const Vector3d ref3 = Vector3d{1, 0, 0};
        CHECK(IsEqual(vec3.x, ref3.x, 0.00001));
        CHECK(IsEqual(vec3.y, ref3.y, 0.00001));
        CHECK(IsEqual(vec3.z, ref3.z, 0.00001));
    }
}

TEST_CASE("Rotator euler angles", "[math][rotator]")
{
    {
        const Rotf rot(10, 15, 20);
        const Vector3f euler = RotatorToEuler(rot);
        CHECK(rot.roll == euler.x);
        CHECK(rot.yaw == euler.y);
        CHECK(rot.pitch == euler.z);
    }
    {
        const Rotd rot(10, 15, 20);
        Vector3d euler = RotatorToEuler(rot);
        CHECK(rot.roll == euler.x);
        CHECK(rot.yaw == euler.y);
        CHECK(rot.pitch == euler.z);
    }
}

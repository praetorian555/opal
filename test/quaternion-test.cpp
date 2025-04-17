#include "test-helpers.h"

using namespace Opal;

using Quatf = Quaternion<float>;
using Quatd = Quaternion<double>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;
using Matrix4x4f = Matrix4x4<float>;
using Matrix4x4d = Matrix4x4<double>;

TEST_CASE("Quaternion constructors", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        CHECK(q1.vec.x == 1);
        CHECK(q1.vec.y == 2);
        CHECK(q1.vec.z == 3);
        CHECK(q1.w == 4);
        const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(0, 0, 2), 60);
        CHECK(q2.vec.x == 0);
        CHECK(q2.vec.y == 0);
        CHECK(q2.vec.z == 0.5f);
        CHECK(q2.w == 0.866025403784438646763723170752f);
        const Quatf q3 = Quatf::FromAxisAngleRadians(Vector3f(0, 0, 2), Radians(60.0f));
        CHECK(q3.vec.x == 0);
        CHECK(q3.vec.y == 0);
        CHECK(q3.vec.z == 0.5f);
        CHECK(q2.w == 0.866025403784438646763723170752f);
        const Quatf q4 = Quatf::Identity();
        CHECK(q4.vec.x == 0);
        CHECK(q4.vec.y == 0);
        CHECK(q4.vec.z == 0);
        CHECK(q4.w == 1);
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        CHECK(q1.vec.x == 1);
        CHECK(q1.vec.y == 2);
        CHECK(q1.vec.z == 3);
        CHECK(q1.w == 4);
        const Quatd q2 = Quatd::FromAxisAngleDegrees(Vector3d(0, 0, 2), 60);
        CHECK(q2.vec.x == 0);
        CHECK(q2.vec.y == 0);
        CHECK(IsEqual(q2.vec.z, 0.5, k_machine_epsilon_double));
        CHECK(IsEqual(q2.w, 0.866025403784438646763723170752, k_machine_epsilon_double));
        const Quatd q3 = Quatd::FromAxisAngleRadians(Vector3d(0, 0, 2), Radians(60.0));
        CHECK(q3.vec.x == 0);
        CHECK(q3.vec.y == 0);
        CHECK(IsEqual(q3.vec.z, 0.5, k_machine_epsilon_double));
        CHECK(IsEqual(q2.w, 0.866025403784438646763723170752, k_machine_epsilon_double));
        const Quatd q4 = Quatd::Identity();
        CHECK(q4.vec.x == 0);
        CHECK(q4.vec.y == 0);
        CHECK(q4.vec.z == 0);
        CHECK(q4.w == 1);
    }

    // TODO: Test this once the transform is refactored
    //    const Transform T = RotateZ(MATH_REALC(60.0));
    //    const Quaternion Q(T);
    //    const Quaternion Ref =
    //        Quaternion::FromAxisAngleRadians(Vector3(0, 0, 1),
    //        Radians(60.0f));
    //    EXPECT_REAL_EQ(q.vec.x, Ref.vec.x);
    //    EXPECT_REAL_EQ(q.vec.y, Ref.vec.y);
    //    EXPECT_REAL_EQ(q.vec.z, Ref.vec.z);
    //    EXPECT_REAL_EQ(q.w, Ref.w);
}

TEST_CASE("Quaternion comparison", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        const Quatf q2(4, 1, 2, 3);
        const Quatf q3(5, 1, 2, 3);
        const Quatf q4(4, 1, 5, 3);
        CHECK(q1 == q2);
        CHECK_FALSE(q1 != q2);
        CHECK_FALSE(q1 == q3);
        CHECK(q1 != q3);
        CHECK_FALSE(q1 == q4);
        CHECK(q1 != q4);
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        const Quatd q2(4, 1, 2, 3);
        const Quatd q3(5, 1, 2, 3);
        const Quatd q4(4, 1, 5, 3);
        CHECK(q1 == q2);
        CHECK_FALSE(q1 != q2);
        CHECK_FALSE(q1 == q3);
        CHECK(q1 != q3);
        CHECK_FALSE(q1 == q4);
        CHECK(q1 != q4);
    }
}

TEST_CASE("Quaternion addition", "[math][quaternion]")
{
    SECTION("float")
    {
        SECTION("Add quaternions")
        {
            const Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            const Quatf q3 = q1 + q2;
            CHECK(q3.vec.x == 2);
            CHECK(q3.vec.y == 4);
            CHECK(q3.vec.z == 6);
            CHECK(q3.w == 8);
        }
        SECTION("Add quaternions and assign")
        {
            Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            q1 += q2;
            CHECK(q1.vec.x == 2);
            CHECK(q1.vec.y == 4);
            CHECK(q1.vec.z == 6);
            CHECK(q1.w == 8);
        }
    }
    SECTION("double")
    {
        SECTION("Add quaternions")
        {
            const Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            const Quatd q3 = q1 + q2;
            CHECK(q3.vec.x == 2);
            CHECK(q3.vec.y == 4);
            CHECK(q3.vec.z == 6);
            CHECK(q3.w == 8);
        }
        SECTION("Add quaternions and assign")
        {
            Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            q1 += q2;
            CHECK(q1.vec.x == 2);
            CHECK(q1.vec.y == 4);
            CHECK(q1.vec.z == 6);
            CHECK(q1.w == 8);
        }
    }
}

TEST_CASE("Quaternion subtraction", "[math][quaternion]")
{
    SECTION("float")
    {
        SECTION("Subtract quaternions")
        {
            const Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            const Quatf q3 = q1 - q2;
            CHECK(q3.vec.x == 0);
            CHECK(q3.vec.y == 0);
            CHECK(q3.vec.z == 0);
            CHECK(q3.w == 0);
        }
        SECTION("Subtract quaternions and assign")
        {
            Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            q1 -= q2;
            CHECK(q1.vec.x == 0);
            CHECK(q1.vec.y == 0);
            CHECK(q1.vec.z == 0);
            CHECK(q1.w == 0);
        }
    }
    SECTION("double")
    {
        SECTION("Subtract quaternions")
        {
            const Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            const Quatd q3 = q1 - q2;
            CHECK(q3.vec.x == 0);
            CHECK(q3.vec.y == 0);
            CHECK(q3.vec.z == 0);
            CHECK(q3.w == 0);
        }
        SECTION("Subtract quaternions and assign")
        {
            Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            q1 -= q2;
            CHECK(q1.vec.x == 0);
            CHECK(q1.vec.y == 0);
            CHECK(q1.vec.z == 0);
            CHECK(q1.w == 0);
        }
    }
}

TEST_CASE("Quaternion multiplication", "[math][quaternion]")
{
    SECTION("float")
    {
        SECTION("Multiply quaternions")
        {
            const Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            const Quatf q3 = q1 * q2;
            CHECK(q3.vec.x == 8);
            CHECK(q3.vec.y == 16);
            CHECK(q3.vec.z == 24);
            CHECK(q3.w == 2);
        }
        SECTION("Multiply quaternions and assign")
        {
            Quatf q1(4, 1, 2, 3);
            const Quatf q2(4, 1, 2, 3);
            q1 *= q2;
            CHECK(q1.vec.x == 8);
            CHECK(q1.vec.y == 16);
            CHECK(q1.vec.z == 24);
            CHECK(q1.w == 2);
        }
        SECTION("Multiply by scalar")
        {
            const Quatf q1(4, 1, 2, 3);
            const Quatf q2 = q1 * 2;
            CHECK(q2.vec.x == 2);
            CHECK(q2.vec.y == 4);
            CHECK(q2.vec.z == 6);
            CHECK(q2.w == 8);
        }
        SECTION("Multiply by scalar from left")
        {
            const Quatf q1(4, 1, 2, 3);
            const Quatf q2 = 2.0f * q1;
            CHECK(q2.vec.x == 2);
            CHECK(q2.vec.y == 4);
            CHECK(q2.vec.z == 6);
            CHECK(q2.w == 8);
        }
        SECTION("Multiply by scalar and assign")
        {
            Quatf q1(4, 1, 2, 3);
            q1 *= 2;
            CHECK(q1.vec.x == 2);
            CHECK(q1.vec.y == 4);
            CHECK(q1.vec.z == 6);
            CHECK(q1.w == 8);
        }
        SECTION("Multiply 3D vector")
        {
            const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
            const Vector3f Res = q1 * Vector3f(0, 1, 0);
            CHECK(Res.x == 0);
            CHECK(IsEqual(Res.y, 0.0f, 0.0001f));
            CHECK(Res.z == 1);
        }
        SECTION("Multiply 3D point")
        {
            const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
            const Point3f Res = q1 * Point3f(0, 0, 1);
            CHECK(Res.x == 0);
            CHECK(Res.y == -1);
            CHECK(Res.z == 0);
        }
    }
    SECTION("double")
    {
        SECTION("Multiply quaternions")
        {
            const Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            const Quatd q3 = q1 * q2;
            CHECK(q3.vec.x == 8);
            CHECK(q3.vec.y == 16);
            CHECK(q3.vec.z == 24);
            CHECK(q3.w == 2);
        }
        SECTION("Multiply quaternions and assign")
        {
            Quatd q1(4, 1, 2, 3);
            const Quatd q2(4, 1, 2, 3);
            q1 *= q2;
            CHECK(q1.vec.x == 8);
            CHECK(q1.vec.y == 16);
            CHECK(q1.vec.z == 24);
            CHECK(q1.w == 2);
        }
        SECTION("Multiply by scalar")
        {
            const Quatd q1(4, 1, 2, 3);
            const Quatd q2 = q1 * 2;
            CHECK(q2.vec.x == 2);
            CHECK(q2.vec.y == 4);
            CHECK(q2.vec.z == 6);
            CHECK(q2.w == 8);
        }
        SECTION("Multiply by scalar from left")
        {
            const Quatd q1(4, 1, 2, 3);
            const Quatd q2 = 2.0 * q1;
            CHECK(q2.vec.x == 2);
            CHECK(q2.vec.y == 4);
            CHECK(q2.vec.z == 6);
            CHECK(q2.w == 8);
        }
        SECTION("Multiply by scalar and assign")
        {
            Quatd q1(4, 1, 2, 3);
            q1 *= 2;
            CHECK(q1.vec.x == 2);
            CHECK(q1.vec.y == 4);
            CHECK(q1.vec.z == 6);
            CHECK(q1.w == 8);
        }
        SECTION("Multiply 3D vector")
        {
            const Quatd q1 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 90);
            const Vector3d Res = q1 * Vector3d(0, 1, 0);
            CHECK(Res.x == 0);
            CHECK(IsEqual(Res.y, 0.0, k_machine_epsilon_double));
            CHECK(Res.z == 1);
        }
        SECTION("Multiply 3D point")
        {
            const Quatd q1 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 90);
            const Point3d Res = q1 * Point3d(0, 0, 1);
            CHECK(Res.x == 0);
            CHECK(Res.y == -1);
            CHECK(IsEqual(Res.z, 0.0, k_machine_epsilon_double));
        }
    }
}

TEST_CASE("Quaternion division", "[math][quaternion]")
{
    SECTION("float")
    {
        SECTION("Divide quaternion by scalar")
        {
            const Quatf Q1(4, 1, 2, 3);
            const Quatf Q2 = Q1 / 2;
            CHECK(Q2.vec.x == 0.5f);
            CHECK(Q2.vec.y == 1);
            CHECK(Q2.vec.z == 1.5f);
            CHECK(Q2.w == 2);
        }
        SECTION("Divide quaternion by scalar and assign")
        {
            Quatf Q1(4, 1, 2, 3);
            Q1 /= 2;
            CHECK(Q1.vec.x == 0.5f);
            CHECK(Q1.vec.y == 1);
            CHECK(Q1.vec.z == 1.5f);
            CHECK(Q1.w == 2);
        }
    }
    SECTION("double")
    {
        SECTION("Divide quaternion by scalar")
        {
            const Quatd Q1(4, 1, 2, 3);
            const Quatd Q2 = Q1 / 2;
            CHECK(Q2.vec.x == 0.5);
            CHECK(Q2.vec.y == 1);
            CHECK(Q2.vec.z == 1.5);
            CHECK(Q2.w == 2);
        }
        SECTION("Divide quaternion by scalar and assign")
        {
            Quatd Q1(4, 1, 2, 3);
            Q1 /= 2;
            CHECK(Q1.vec.x == 0.5);
            CHECK(Q1.vec.y == 1);
            CHECK(Q1.vec.z == 1.5);
            CHECK(Q1.w == 2);
        }
    }
}

TEST_CASE("Quaternion NaN and non-finite", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(1, 1, NAN, 3);
        CHECK(ContainsNaN(q1));
        CHECK(ContainsNonFinite(q1));
        const Quatf q2(NAN, 1, 2, 3);
        CHECK(ContainsNaN(q2));
        CHECK(ContainsNonFinite(q2));
        const Quatf q3(1, 1, INFINITY, 3);
        CHECK(ContainsNonFinite(q3));
        const Quatf q4(INFINITY, 1, 2, 3);
        CHECK(ContainsNonFinite(q4));
    }
    SECTION("double")
    {
        const Quatd q1(1, 1, NAN, 3);
        CHECK(ContainsNaN(q1));
        CHECK(ContainsNonFinite(q1));
        const Quatd q2(NAN, 1, 2, 3);
        CHECK(ContainsNaN(q2));
        CHECK(ContainsNonFinite(q2));
        const Quatd q3(1, 1, INFINITY, 3);
        CHECK(ContainsNonFinite(q3));
        const Quatd q4(INFINITY, 1, 2, 3);
        CHECK(ContainsNonFinite(q4));
    }
}

TEST_CASE("Quaternion length", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q(4, 1, 2, 3);
        CHECK(Length(q) == 5.477225575051661134569124887f);
        CHECK(LengthSquared(q) == 30.0f);
    }
    SECTION("double")
    {
        const Quatd q(4, 1, 2, 3);
        CHECK(Length(q) == 5.477225575051661134569124887);
        CHECK(LengthSquared(q) == 30.0);
    }
}

TEST_CASE("Quaternion dot product", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        const Quatf q2(4, 1, 2, 3);
        CHECK(Dot(q1, q2) == 30);
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        const Quatd q2(4, 1, 2, 3);
        CHECK(Dot(q1, q2) == 30);
    }
}

TEST_CASE("Quaternion normalize", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        const Quatf q2 = Normalize(q1);
        CHECK(Length(q2) == 1.0f);
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        const Quatd q2 = Normalize(q1);
        CHECK(IsEqual(Length(q2), 1.0, k_machine_epsilon_double));
    }
}

TEST_CASE("Quaternion lerp", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 30);
        const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
        const Quatf q3 = Lerp(0.5f, q1, q2);
        const Quatf q4 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 60);
        CHECK(q3.vec.x == q4.vec.x);
        CHECK(q3.vec.y == q4.vec.y);
        CHECK(q3.vec.z == q4.vec.z);
        CHECK(q3.w == q4.w);
    }
    SECTION("float with 0.4 parameter")
    {
        const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 30);
        const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
        const Quatf q3 = Lerp(0.4f, q1, q2);
        const Quatf q4 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 54);
        CHECK(IsEqual(q3.vec.x, q4.vec.x, 10'000 * k_machine_epsilon_float));
        CHECK(q3.vec.y == q4.vec.y);
        CHECK(q3.vec.z == q4.vec.z);
        CHECK(IsEqual(q3.w, q4.w, 10'000 * k_machine_epsilon_float));
    }
    SECTION("double")
    {
        const Quatd q1 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 30);
        const Quatd q2 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 90);
        const Quatd q3 = Lerp(0.5, q1, q2);
        const Quatd q4 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 60);
        CHECK(IsEqual(q3.vec.x, q4.vec.x, k_machine_epsilon_double));
        CHECK(q3.vec.y == q4.vec.y);
        CHECK(q3.vec.z == q4.vec.z);
        CHECK(q3.w == q4.w);
    }
}

TEST_CASE("Quaternion slerp", "[math][quaternion]")
{
    SECTION("float")
    {
        SECTION("parameter 0.5")
        {
            const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 30);
            const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
            const Quatf q3 = Slerp(0.5f, q1, q2);
            const Quatf q4 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 60);
            CHECK(IsEqual(q3.vec.x, q4.vec.x, k_machine_epsilon_float));
            CHECK(q3.vec.y == q4.vec.y);
            CHECK(q3.vec.z == q4.vec.z);
            CHECK(q3.w == q4.w);
        }
        SECTION("parameter 0.4")
        {
            const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 30);
            const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
            const Quatf q3 = Slerp(0.4f, q1, q2);
            const Quatf q4 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 54);
            CHECK(IsEqual(q3.vec.x, q4.vec.x, k_machine_epsilon_float));
            CHECK(q3.vec.y == q4.vec.y);
            CHECK(q3.vec.z == q4.vec.z);
            CHECK(IsEqual(q3.w, q4.w, k_machine_epsilon_float));
        }
    }
    SECTION("double")
    {
        SECTION("parameter 0.5")
        {
            const Quatd q1 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 30);
            const Quatd q2 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 90);
            const Quatd q3 = Slerp(0.5, q1, q2);
            const Quatd q4 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 60);
            CHECK(q3.vec.x == q4.vec.x);
            CHECK(q3.vec.y == q4.vec.y);
            CHECK(q3.vec.z == q4.vec.z);
            CHECK(IsEqual(q3.w, q4.w, k_machine_epsilon_double));
        }
        SECTION("parameter 0.4")
        {
            const Quatd q1 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 30);
            const Quatd q2 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 90);
            const Quatd q3 = Slerp(0.4, q1, q2);
            const Quatd q4 = Quatd::FromAxisAngleDegrees(Vector3d(1, 0, 0), 54);
            CHECK(q3.vec.x == q4.vec.x);
            CHECK(q3.vec.y == q4.vec.y);
            CHECK(q3.vec.z == q4.vec.z);
            CHECK(IsEqual(q3.w, q4.w, k_machine_epsilon_double));
        }
    }
}

TEST_CASE("Quaternion conjugate", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        const Quatf q2 = Conjugate(q1);
        CHECK(q2.vec.x == -1);
        CHECK(q2.vec.y == -2);
        CHECK(q2.vec.z == -3);
        CHECK(q2.w == 4);
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        const Quatd q2 = Conjugate(q1);
        CHECK(q2.vec.x == -1);
        CHECK(q2.vec.y == -2);
        CHECK(q2.vec.z == -3);
        CHECK(q2.w == 4);
    }
}

TEST_CASE("Quaternion inverse", "[math][quaternion]")
{
    SECTION("float")
    {
        const Quatf q1(4, 1, 2, 3);
        const Quatf q2 = Inverse(q1);
        CHECK(q2.w == 2.0f / 15.0f);
        CHECK(q2.vec.x == -1.0f / 30.0f);
        CHECK(q2.vec.y == -1.0f / 15.0f);
        CHECK(IsEqual(q2.vec.z, -1.0f / 10.0f, k_machine_epsilon_float));
    }
    SECTION("double")
    {
        const Quatd q1(4, 1, 2, 3);
        const Quatd q2 = Inverse(q1);
        CHECK(q2.w == 2.0 / 15.0);
        CHECK(q2.vec.x == -1.0 / 30.0);
        CHECK(q2.vec.y == -1.0 / 15.0);
        CHECK(q2.vec.z == -1.0 / 10.0);
    }
}

TEST_CASE("Quaternion rotate a point", "[math][quaternion]")
{
    SECTION("rotate point around an axis and then rotate it using inverse")
    {
        // Rotate point around an axis
        const Quatf q = Quatf::FromAxisAngleDegrees(Vector3f(0, 0, 1), 90);
        const Point3f p = q * Point3f(1, 0, 0);
        CHECK(p.x == 0);
        CHECK(p.y == 1);
        CHECK(p.z == 0);
        // Rotate using an inverse, should be in the opposite direction by the same amount
        const Quatf q_inv = Inverse(q);
        const Point3f p_inv = q_inv * Point3f(1, 0, 0);
        CHECK(p_inv.x == 0);
        CHECK(p_inv.y == -1);
        CHECK(p_inv.z == 0);
    }
    SECTION("rotate point using a composition of two rotations")
    {
        // Rotate point using a composition of two rotations
        const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(0, 0, 1), 90);
        const Quatf q2 = Quatf::FromAxisAngleDegrees(Vector3f(1, 0, 0), 90);
        const Quatf q_comp = q2 * q1;
        const Point3f p = q_comp * Point3f(1, 0, 0);
        CHECK(IsEqual(p.x, 0.0f, k_machine_epsilon_float));
        CHECK(IsEqual(p.y, 0.0f, k_machine_epsilon_float));
        CHECK(IsEqual(p.z, 1.0f, k_machine_epsilon_float));
    }
    SECTION("rotate a point using a quaternion and a quaternion multiplied by -1")
    {
        const Quatf q1 = Quatf::FromAxisAngleDegrees(Vector3f(0, 0, 1), 45);
        const Quatf q2 = -1.0f * q1;
        const Point3f p1 = q1 * Point3f(1, 0, 0);
        const Point3f p2 = q2 * Point3f(1, 0, 0);
        CHECK(IsEqual(p1.x, 0.7071068f, k_machine_epsilon_float));
        CHECK(IsEqual(p1.y, 0.7071068f, k_machine_epsilon_float));
        CHECK(IsEqual(p1.z, 0.0f, k_machine_epsilon_float));
        CHECK(IsEqual(p2.x, 0.7071068f, k_machine_epsilon_float));
        CHECK(IsEqual(p2.y, 0.7071068f, k_machine_epsilon_float));
        CHECK(IsEqual(p2.z, 0.0f, k_machine_epsilon_float));
    }
}

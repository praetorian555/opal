#include "test-helpers.h"

#include "opal/math/dual-quaternion.h"
#include "opal/math/transform.h"

using namespace Opal;

using DualQuatf = DualQuaternion<float>;
using DualQuatd = DualQuaternion<double>;
using Quatf = Quaternion<float>;
using Quatd = Quaternion<double>;
using Vec3f = Vector3<float>;
using Point3f = Point3<float>;
using Vec3d = Vector3<double>;
using Point3d = Point3<double>;

TEST_CASE("Dual quaternion construct identity", "[math][dual-quaternion]")
{
    const DualQuatf dq = DualQuatf::Identity();
    CHECK(dq.real == Quatf(1, 0, 0, 0));
    CHECK(dq.dual == Quatf(0, 0, 0, 0));
}

TEST_CASE("Dual quaternion construct from two quaternions", "[math][dual-quaternion]")
{
    const Quatf real(1, 2, 3, 4);
    const Quatf dual(5, 6, 7, 8);
    const DualQuatf dq(real, dual);
    CHECK(dq.real == real);
    CHECK(dq.dual == dual);
}

TEST_CASE("Dual quaternion construct from orientation", "[math][dual-quaternion]")
{
    const Quatf orientation(1, 2, 3, 4);
    const DualQuatf dq(orientation);
    CHECK(dq.real == orientation);
    CHECK(dq.dual == Quatf(0, 0, 0, 0));
}

TEST_CASE("Dual quaternion construct from translation", "[math][dual-quaternion]")
{
    const Vec3f translation(1, 2, 3);
    const DualQuatf dq(translation);
    CHECK(dq.real == Quatf(1, 0, 0, 0));
    CHECK(dq.dual == Quatf(0, 0.5f, 1, 1.5f));
}

TEST_CASE("Dual quaternion construct from orientation and translation", "[math][dual-quaternion]")
{
    const Quatf orientation(1, 2, 3, 4);
    const Vec3f translation(5, 6, 7);
    const DualQuatf dq(orientation, translation);
    // TODO: We need better test for this
    const Quatf dual = 0.5f * Quatf(0, translation.x, translation.y, translation.z) * orientation;
    CHECK(dq.real == orientation);
    CHECK(dq.dual == dual);
}

TEST_CASE("Dual quaternion copy constructor", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2(dq1);
    CHECK(dq2.real == dq1.real);
    CHECK(dq2.dual == dq1.dual);
}

TEST_CASE("Dual quaternion copy constructor, different value type", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatd dq2(dq1);
    CHECK(dq2.real == Quatd(1, 2, 3, 4));
    CHECK(dq2.dual == Quatd(5, 6, 7, 8));
}

TEST_CASE("Dual quaternion assignment operator", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    DualQuatf dq2;
    dq2 = dq1;
    CHECK(dq2.real == dq1.real);
    CHECK(dq2.dual == dq1.dual);
}

TEST_CASE("Dual quaternion assignment operator, different value type", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    DualQuatd dq2;
    dq2 = dq1;
    CHECK(dq2.real == Quatd(1, 2, 3, 4));
    CHECK(dq2.dual == Quatd(5, 6, 7, 8));
}

TEST_CASE("Dual quaternion compare equality", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq3(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 9));
    CHECK(dq1 == dq2);
    CHECK_FALSE(dq1 == dq3);
}

TEST_CASE("Dual quaternion compare inequality", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq3(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 9));
    CHECK_FALSE(dq1 != dq2);
    CHECK(dq1 != dq3);
}

TEST_CASE("Dual quaternion comparison with IsEqual", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2(Quatf(1.1f, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq3(Quatf(1, 2, 3, 4), Quatf(5, 6, 7.2f, 8));
    CHECK(IsEqual(dq1, dq2, 0.11f));
    CHECK_FALSE(IsEqual(dq1, dq3, 0.1f));
}

TEST_CASE("Dual quaternion multiply by scalar with assign", "[math][dual-quaternion]")
{
    DualQuatf dq(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    dq *= 2;
    CHECK(dq.real == Quatf(2, 4, 6, 8));
    CHECK(dq.dual == Quatf(10, 12, 14, 16));
}

TEST_CASE("Dual quaternion divide by scalar with assign", "[math][dual-quaternion]")
{
    DualQuatf dq(Quatf(2, 4, 6, 8), Quatf(10, 12, 14, 16));
    dq /= 2;
    CHECK(dq.real == Quatf(1, 2, 3, 4));
    CHECK(dq.dual == Quatf(5, 6, 7, 8));
}

TEST_CASE("Dual quaternion get orientation", "[math][dual-quaternion]")
{
    const DualQuatf dq(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const Quatf orientation = dq.GetOrientation();
    CHECK(orientation == Quatf(1, 2, 3, 4));
}

TEST_CASE("Dual quaternion get translation", "[math][dual-quaternion]")
{
    const DualQuatf dq(Quaternion<float>::FromAxisAngleDegrees(Vec3f(1, 1, 1), 30.0f), Vec3f(5, 6, 7));
    const Vec3f translation = dq.GetTranslation();
    CHECK(IsEqual(translation, Vec3f(5, 6, 7), 10 * k_machine_epsilon_float));
}

TEST_CASE("Dual quaternion unary plus", "[math][dual-quaternion]")
{
    const DualQuatf dq(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2 = +dq;
    CHECK(dq2.real == Quatf(1, 2, 3, 4));
    CHECK(dq2.dual == Quatf(5, 6, 7, 8));
}

TEST_CASE("Dual quaternion negate", "[math][dual-quaternion]")
{
    const DualQuatf dq(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2 = -dq;
    CHECK(dq2.real == Quatf(-1, -2, -3, -4));
    CHECK(dq2.dual == Quatf(-5, -6, -7, -8));
}

TEST_CASE("Dual quaternion addition", "[math][dual-quaternion]")
{
    const DualQuatf dq1(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
    const DualQuatf dq2(Quatf(9, 10, 11, 12), Quatf(13, 14, 15, 16));
    const DualQuatf dq3 = dq1 + dq2;
    CHECK(dq3.real == Quatf(10, 12, 14, 16));
    CHECK(dq3.dual == Quatf(18, 20, 22, 24));
}

TEST_CASE("Dual quaternion transform point", "[math][dual-quaternion]")
{
    const Vec3f translation(0, 2, 0);
    const Quatf orientation = Quaternion<float>::FromAxisAngleDegrees(Vec3f(1, 0, 0), 90.0f);
    const DualQuatf dq(orientation, translation);
    const Point3f point(0, 0, -1);
    const Point3f transformed_point = dq * point;
    const Point3f expected_point = (Translate(translation) * Rotate(orientation)) * point;
    CHECK(IsEqual(transformed_point, expected_point, 10 * k_machine_epsilon_float));
}

TEST_CASE("DualQuaternion Multiplication (DQ * DQ)", "[math][dual_quaternion]")
{
    SECTION("float")
    {
        const DualQuatf identity = DualQuatf::Identity();
        const Quatf rot = Normalize(Quatf(0.7071f, 0.7071f, 0.0f, 0.0f));  // 90 deg around X
        const Vec3f trans(1.0f, 2.0f, 3.0f);
        const DualQuatf dq(rot, trans);

        // Test multiplication by identity
        const DualQuatf res_ident_left = identity * dq;
        const DualQuatf res_ident_right = dq * identity;
        CHECK_DUAL_QUATERNION(dq, res_ident_left, k_machine_epsilon_float);
        CHECK_DUAL_QUATERNION(dq, res_ident_right, k_machine_epsilon_float);

        // Test multiplication of two transformations
        const Quatf rot2 = Normalize(Quatf(0.7071f, 0.0f, 0.7071f, 0.0f));  // 90 deg around Y
        const Vec3f trans2(-1.0f, 0.0f, 1.0f);
        const DualQuatf dq2(rot2, trans2);

        const DualQuatf result = dq * dq2;

        // Expected result combines rotations and translations
        const Quatf expected_rot = rot * rot2;
        const Vec3f expected_trans = Rotate(rot) * trans2 + trans;
        const DualQuatf expected_dq(expected_rot, expected_trans);

        CHECK_DUAL_QUATERNION(result, expected_dq, k_machine_epsilon_float);
    }
    SECTION("double")
    {
        const DualQuatd identity = DualQuatd::Identity();
        const Quatd rot = Normalize(Quatd(0.70710678118, 0.70710678118, 0.0, 0.0));  // 90 deg around X
        const Vec3d trans(1.0, 2.0, 3.0);
        const DualQuatd dq(rot, trans);

        // Test multiplication by identity
        const DualQuatd res_ident_left = identity * dq;
        const DualQuatd res_ident_right = dq * identity;
        CHECK_DUAL_QUATERNION(res_ident_left, dq, k_machine_epsilon_double);
        CHECK_DUAL_QUATERNION(res_ident_right, dq, k_machine_epsilon_double);

        // Test multiplication of two transformations
        const Quatd rot2 = Normalize(Quatd(0.70710678118, 0.0, 0.70710678118, 0.0));  // 90 deg around Y
        const Vec3d trans2(-1.0, 0.0, 1.0);
        const DualQuatd dq2(rot2, trans2);

        const DualQuatd result = dq * dq2;

        // Expected result combines rotations and translations
        const Quatd expected_rot = rot * rot2;
        const Vec3d expected_trans = Rotate(rot) * trans2 + trans;
        const DualQuatd expected_dq(expected_rot, expected_trans);

        CHECK_DUAL_QUATERNION(result, expected_dq, k_machine_epsilon_double);
    }
}

TEST_CASE("DualQuaternion Point Transformation (DQ * Point)", "[math][dual_quaternion]")
{
    const Point3f p(1.0f, 2.0f, 3.0f);
    const Point3d pd(1.0, 2.0, 3.0);

    SECTION("float")
    {
        // Identity transformation
        const DualQuatf identity = DualQuatf::Identity();
        const Point3f p_ident = identity * p;
        CHECK_POINT3(p_ident, p, k_machine_epsilon_float);

        // Pure translation
        const Vec3f trans(5.0f, -1.0f, 2.0f);
        const DualQuatf dq_trans(trans);
        const Point3f p_trans = dq_trans * p;
        const Point3f expected_p_trans = p + trans;
        CHECK_POINT3(p_trans, expected_p_trans, k_machine_epsilon_float);

        // Pure rotation (90 degrees around X)
        const Quatf rot = Normalize(Quatf::FromAxisAngleDegrees(Vec3f(1, 0, 0), 90.0f));  // 90 deg around X
        const DualQuatf dq_rot(rot);
        const Point3f p_rot = dq_rot * p;
        const Point3f expected_p_rot(1.0f, -3.0f, 2.0f);  // (1, 2, 3) rotated 90 deg around X
        CHECK_POINT3(p_rot, expected_p_rot, 10 * k_machine_epsilon_float);

        // Combined rotation and translation
        const DualQuatf dq_combined(rot, trans);
        const Point3f p_combined = dq_combined * p;
        const Point3f expected_p_combined = expected_p_rot + trans;  // Rotate first, then translate
        CHECK_POINT3(p_combined, expected_p_combined, 10 * k_machine_epsilon_float);
    }
    SECTION("double")
    {
        // Identity transformation
        const DualQuatd identity = DualQuatd::Identity();
        const Point3d p_ident = identity * pd;
        CHECK_POINT3(p_ident, pd, k_machine_epsilon_double);

        // Pure translation
        const Vec3d trans(5.0, -1.0, 2.0);
        const DualQuatd dq_trans(trans);
        const Point3d p_trans = dq_trans * pd;
        const Point3d expected_p_trans = pd + trans;
        CHECK_POINT3(p_trans, expected_p_trans, k_machine_epsilon_double);

        // Pure rotation (90 degrees around X)
        const Quatd rot = Normalize(Quatd(0.70710678118, 0.70710678118, 0.0, 0.0));
        const DualQuatd dq_rot(rot);
        const Point3d p_rot = dq_rot * pd;
        const Point3d expected_p_rot(1.0, -3.0, 2.0);  // (1, 2, 3) rotated 90 deg around X
        CHECK_POINT3(p_rot, expected_p_rot, 10 * k_machine_epsilon_double);

        // Combined rotation and translation
        const DualQuatd dq_combined(rot, trans);
        const Point3d p_combined = dq_combined * pd;
        const Point3d expected_p_combined = expected_p_rot + trans;  // Rotate first, then translate
        CHECK_POINT3(p_combined, expected_p_combined, 10 * k_machine_epsilon_double);
    }
}

TEST_CASE("DualQuaternion Scalar Multiplication", "[math][dual_quaternion]")
{
    SECTION("float")
    {
        const DualQuatf dq(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));
        const float scalar = 2.0f;

        const DualQuatf res1 = dq * scalar;
        const DualQuatf expected1(Quatf(2, 4, 6, 8), Quatf(10, 12, 14, 16));
        CHECK_DUAL_QUATERNION(res1, expected1, k_machine_epsilon_float);

        const DualQuatf res2 = scalar * dq;
        CHECK_DUAL_QUATERNION(res2, expected1, k_machine_epsilon_float);
    }
    SECTION("double")
    {
        const DualQuatd dq(Quatd(1, 2, 3, 4), Quatd(5, 6, 7, 8));
        const double scalar = 2.0;

        const DualQuatd res1 = dq * scalar;
        const DualQuatd expected1(Quatd(2, 4, 6, 8), Quatd(10, 12, 14, 16));
        CHECK_DUAL_QUATERNION(res1, expected1, k_machine_epsilon_double);

        const DualQuatd res2 = scalar * dq;
        CHECK_DUAL_QUATERNION(res2, expected1, k_machine_epsilon_double);
    }
}

TEST_CASE("DualQuaternion Scalar Division", "[math][dual_quaternion]")
{
    SECTION("float")
    {
        const DualQuatf dq(Quatf(2, 4, 6, 8), Quatf(10, 12, 14, 16));
        const float scalar = 2.0f;
        const DualQuatf expected(Quatf(1, 2, 3, 4), Quatf(5, 6, 7, 8));

        const DualQuatf res = dq / scalar;
        CHECK_DUAL_QUATERNION(res, expected, k_machine_epsilon_float);

        // Note: Division by zero is handled by OPAL_ASSERT in the implementation, not typically tested via unit tests unless it throws
        // exceptions.
    }
    SECTION("double")
    {
        const DualQuatd dq(Quatd(2, 4, 6, 8), Quatd(10, 12, 14, 16));
        const double scalar = 2.0;
        const DualQuatd expected(Quatd(1, 2, 3, 4), Quatd(5, 6, 7, 8));

        const DualQuatd res = dq / scalar;
        CHECK_DUAL_QUATERNION(res, expected, k_machine_epsilon_double);
    }
}

TEST_CASE("DualQuaternion Inverse", "[math][dual_quaternion]")
{
    const float angle_rad = k_pi_float / 4.0f;  // 45 degrees
    const Vec3f axis = Normalize(Vec3f(1.0f, 1.0f, 1.0f));
    const Quatf rot = Quatf::FromAxisAngleRadians(axis, angle_rad);
    const Vec3f trans(1.0f, -2.0f, 3.0f);

    const double angle_rad_d = k_pi_double / 4.0;  // 45 degrees
    const Vec3d axis_d = Normalize(Vec3d(1.0, 1.0, 1.0));
    const Quatd rot_d = Quatd::FromAxisAngleRadians(axis_d, angle_rad_d);
    const Vec3d trans_d(1.0, -2.0, 3.0);

    SECTION("float")
    {
        // Inverse of identity
        const DualQuatf identity = DualQuatf::Identity();
        const DualQuatf inv_identity = Inverse(identity);
        CHECK_DUAL_QUATERNION(inv_identity, identity, k_machine_epsilon_float);

        // Inverse of pure rotation
        const DualQuatf dq_rot_only(rot);
        const DualQuatf inv_dq_rot_only = Inverse(dq_rot_only);
        const DualQuatf expected_inv_rot(Conjugate(rot));  // Inverse of rotation quat is its conjugate
        CHECK_DUAL_QUATERNION(inv_dq_rot_only, expected_inv_rot, k_machine_epsilon_float);
        CHECK_DUAL_QUATERNION(dq_rot_only * inv_dq_rot_only, identity, k_machine_epsilon_float);

        // Inverse of pure translation
        const DualQuatf dq_trans_only(trans);
        const DualQuatf inv_dq_trans_only = Inverse(dq_trans_only);
        const DualQuatf expected_inv_trans(-trans);  // Inverse of translation is negative translation
        CHECK_DUAL_QUATERNION(inv_dq_trans_only, expected_inv_trans, k_machine_epsilon_float);
        CHECK_DUAL_QUATERNION(dq_trans_only * inv_dq_trans_only, identity, k_machine_epsilon_float);

        // Inverse of combined transformation
        const DualQuatf dq_combined(rot, trans);
        const DualQuatf inv_dq_combined = Inverse(dq_combined);
        // Check dq * inverse(dq) == identity
        CHECK_DUAL_QUATERNION(dq_combined * inv_dq_combined, identity, k_machine_epsilon_float);

        // Check inverse(dq) * dq == identity
        CHECK_DUAL_QUATERNION(inv_dq_combined * dq_combined, identity, k_machine_epsilon_float);

        // Verify inverse calculation manually for a known case
        // inv(r + eps * d) = conj(r) + eps * (-conj(r) * d * conj(r))
        // For pure translation (r=1), inv(1 + eps*t/2) = 1 + eps*(-t/2) = 1 - eps*t/2
        const DualQuatf inv_trans_manual(Quatf::Identity(), Quatf(0.0f, -trans.x / 2.0f, -trans.y / 2.0f, -trans.z / 2.0f));
        CHECK_DUAL_QUATERNION(inv_dq_trans_only, inv_trans_manual, k_machine_epsilon_float);
    }
    SECTION("double")
    {
        // Inverse of identity
        const DualQuatd identity = DualQuatd::Identity();
        const DualQuatd inv_identity = Inverse(identity);
        CHECK_DUAL_QUATERNION(inv_identity, identity, k_machine_epsilon_double);

        // Inverse of pure rotation
        const DualQuatd dq_rot_only(rot_d);
        const DualQuatd inv_dq_rot_only = Inverse(dq_rot_only);
        const DualQuatd expected_inv_rot(Conjugate(rot_d));  // Inverse of rotation quat is its conjugate
        CHECK_DUAL_QUATERNION(inv_dq_rot_only, expected_inv_rot, k_machine_epsilon_double);
        CHECK_DUAL_QUATERNION(dq_rot_only * inv_dq_rot_only, identity, k_machine_epsilon_double);

        // Inverse of pure translation
        const DualQuatd dq_trans_only(trans_d);
        const DualQuatd inv_dq_trans_only = Inverse(dq_trans_only);
        const DualQuatd expected_inv_trans(-trans_d);  // Inverse of translation is negative translation
        CHECK_DUAL_QUATERNION(inv_dq_trans_only, expected_inv_trans, k_machine_epsilon_double);
        CHECK_DUAL_QUATERNION(dq_trans_only * inv_dq_trans_only, identity, k_machine_epsilon_double);

        // Inverse of combined transformation
        const DualQuatd dq_combined(rot_d, trans_d);
        const DualQuatd inv_dq_combined = Inverse(dq_combined);
        // Check dq * inverse(dq) == identity
        CHECK_DUAL_QUATERNION(dq_combined * inv_dq_combined, identity, k_machine_epsilon_double);

        // Check inverse(dq) * dq == identity
        CHECK_DUAL_QUATERNION(inv_dq_combined * dq_combined, identity, k_machine_epsilon_double);
    }
}

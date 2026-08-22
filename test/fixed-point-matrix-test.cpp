#include <cmath>

#include "test-helpers.h"

#include "opal/math/fixed-point-vector.h"
#include "opal/math/fixed-point.h"
#include "opal/math/matrix.h"
#include "opal/math/quaternion.h"

using namespace Opal;

using Q16 = Fixed32;
using Q32 = Fixed64;

namespace
{
template <typename FixedType>
bool IsCloseTo(FixedType value, double expected, double tolerance)
{
    return std::abs(static_cast<double>(value) - expected) <= tolerance;
}

// A rotation runs every component through a dozen products on top of a sine and a cosine, so both widths need room
// for more than their own resolution. What is left is still an order of magnitude inside what the width can express.
constexpr double k_q16_tolerance = 1e-3;
constexpr double k_q32_tolerance = 1e-6;

/** The same rotation in double precision, which is what the fixed point result is held against. */
Vector3<f64> RotateReference(const Vector3<f64>& axis, f64 angle_degrees, const Vector3<f64>& vec)
{
    return Quaternion<f64>::FromAxisAngleDegrees(axis, angle_degrees) * vec;
}
}  // namespace

TEST_CASE("Matrix accepts a fixed point element type", "[math][fixed-point][Matrix]")
{
    static_assert(FloatingPointOrFixedPoint<Q16>);
    static_assert(FloatingPointOrFixedPoint<Q32>);
    static_assert(FloatingPointOrFixedPoint<f32>);
    static_assert(!FloatingPointOrFixedPoint<i32>);

    static_assert(IsPOD<Matrix4x4<Q16>>);
    static_assert(SameAs<Matrix4x4<Q16>::value_type, Q16>);
    static_assert(sizeof(Matrix4x4<Q16>) == 16 * sizeof(i32));

    SECTION("Diagonal construction")
    {
        const Matrix4x4<Q16> identity(Q16(1));
        CHECK(identity(0, 0) == Q16(1));
        CHECK(identity(0, 1) == Q16(0));
        CHECK(identity(3, 3) == Q16(1));
        CHECK(Matrix4x4<Q16>::Zero()(0, 0) == Q16(0));
    }
    SECTION("Equality")
    {
        const Matrix4x4<Q16> a(Q16(2));
        const Matrix4x4<Q16> b(Q16(2));
        const Matrix4x4<Q16> c(Q16(3));
        CHECK(a == b);
        CHECK(a != c);
        CHECK(IsEqual(a, c, Q16(1.5)));
        CHECK_FALSE(IsEqual(a, c, Q16(0.5)));
    }
    SECTION("Construction from rows")
    {
        const Matrix4x4<Q16> mat = Matrix4x4<Q16>::FromRows({Q16(1), Q16(2), Q16(3), Q16(4)}, {Q16(5), Q16(6), Q16(7), Q16(8)},
                                                            {Q16(9), Q16(10), Q16(11), Q16(12)}, {Q16(13), Q16(14), Q16(15), Q16(16)});
        CHECK(mat(0, 0) == Q16(1));
        CHECK(mat(1, 2) == Q16(7));
        CHECK(mat(3, 3) == Q16(16));
    }
}

TEST_CASE("Fixed point matrix arithmetic", "[math][fixed-point][Matrix]")
{
    const Matrix4x4<Q16> a(Q16(2));
    const Matrix4x4<Q16> b(Q16(3));

    SECTION("Addition and subtraction")
    {
        CHECK(a + b == Matrix4x4<Q16>(Q16(5)));
        CHECK(b - a == Matrix4x4<Q16>(Q16(1)));
        Matrix4x4<Q16> accumulator = a;
        accumulator += b;
        CHECK(accumulator == Matrix4x4<Q16>(Q16(5)));
        accumulator -= b;
        CHECK(accumulator == a);
    }
    SECTION("Multiplication")
    {
        CHECK(a * b == Matrix4x4<Q16>(Q16(6)));
        Matrix4x4<Q16> accumulator = a;
        accumulator *= b;
        CHECK(accumulator == Matrix4x4<Q16>(Q16(6)));
    }
    SECTION("Scalar multiplication from either side")
    {
        CHECK(a * Q16(0.5) == Matrix4x4<Q16>(Q16(1)));
        CHECK(Q16(0.5) * a == Matrix4x4<Q16>(Q16(1)));
        CHECK(2 * a == Matrix4x4<Q16>(Q16(4)));
        CHECK(a / Q16(2) == Matrix4x4<Q16>(Q16(1)));
    }
    SECTION("Transpose")
    {
        const Matrix4x4<Q16> mat = Matrix4x4<Q16>::FromRows({Q16(1), Q16(2), Q16(3), Q16(4)}, {Q16(5), Q16(6), Q16(7), Q16(8)},
                                                            {Q16(9), Q16(10), Q16(11), Q16(12)}, {Q16(13), Q16(14), Q16(15), Q16(16)});
        const Matrix4x4<Q16> transposed = Transpose(mat);
        CHECK(transposed(0, 1) == Q16(5));
        CHECK(transposed(1, 0) == Q16(2));
        CHECK(Transpose(transposed) == mat);
    }
    SECTION("Products are exact when the operands are")
    {
        const Matrix4x4<Q16> scale(Q16(0.25));
        CHECK(scale * Matrix4x4<Q16>(Q16(4)) == Matrix4x4<Q16>(Q16(1)));
    }
}

TEST_CASE("Fixed point matrix transforms points and vectors", "[math][fixed-point][Matrix]")
{
    Matrix4x4<Q16> transform(Q16(1));
    transform(0, 3) = Q16(2);
    transform(1, 3) = Q16(3);
    transform(2, 3) = Q16(4);

    SECTION("A point picks up the translation")
    {
        const Point3<Q16> transformed = transform * Point3<Q16>(Q16(1), Q16(1), Q16(1));
        CHECK(transformed == Point3<Q16>(Q16(3), Q16(4), Q16(5)));
    }
    SECTION("A vector does not")
    {
        const Vector3<Q16> transformed = transform * Vector3<Q16>(Q16(1), Q16(1), Q16(1));
        CHECK(transformed == Vector3<Q16>(Q16(1), Q16(1), Q16(1)));
    }
    SECTION("Four component types")
    {
        const Point4<Q16> point = transform * Point4<Q16>(Q16(1), Q16(1), Q16(1), Q16(1));
        CHECK(point == Point4<Q16>(Q16(3), Q16(4), Q16(5), Q16(1)));
        const Vector4<Q16> vec = transform * Vector4<Q16>(Q16(1), Q16(1), Q16(1), Q16(0));
        CHECK(vec == Vector4<Q16>(Q16(1), Q16(1), Q16(1), Q16(0)));
    }
    SECTION("Normals go through the transpose of the upper left block")
    {
        Matrix4x4<Q16> scale(Q16(1));
        scale(0, 0) = Q16(2);
        const Normal3<Q16> normal = scale * Normal3<Q16>(Q16(1), Q16(0), Q16(0));
        CHECK(normal == Normal3<Q16>(Q16(2), Q16(0), Q16(0)));
    }
    SECTION("Block conversions")
    {
        const Matrix3x3<Q16> block = transform.ToMatrix3x3();
        CHECK(block(0, 0) == Q16(1));
        CHECK(block(0, 1) == Q16(0));
        const Matrix4x4<Q16> widened = block.ToMatrix4x4();
        CHECK(widened(0, 0) == Q16(1));
        CHECK(widened(3, 3) == Q16(1));
        CHECK(widened(0, 3) == Q16(0));
    }
}

TEST_CASE("Fixed point matrix inverse", "[math][fixed-point][Matrix]")
{
    SECTION("The identity inverts to itself")
    {
        const Matrix4x4<Q16> identity(Q16(1));
        const auto inverted = Inverse(identity);
        REQUIRE(inverted.HasValue());
        CHECK(inverted.GetValue() == identity);
    }
    SECTION("A translation inverts, and the width decides how well")
    {
        // The same translation a float inverts exactly comes back a thousandth off at Q16.16, because a translation of
        // ten leaves the routine holding a value the narrow type cannot represent. Q31.32 has the bits for it.
        Matrix4x4<Q16> narrow(Q16(1));
        narrow(0, 3) = Q16(5);
        narrow(1, 3) = Q16(10);
        narrow(2, 3) = Q16(-2);
        const auto narrow_inverted = Inverse(narrow);
        REQUIRE(narrow_inverted.HasValue());
        CHECK(IsCloseTo(narrow_inverted.GetValue()(0, 3), -5.0, 2e-3));
        CHECK(IsCloseTo(narrow_inverted.GetValue()(1, 3), -10.0, 2e-3));
        CHECK(IsCloseTo(narrow_inverted.GetValue()(2, 3), 2.0, 2e-3));

        Matrix4x4<Q32> wide(Q32(1));
        wide(0, 3) = Q32(5);
        wide(1, 3) = Q32(10);
        wide(2, 3) = Q32(-2);
        const auto wide_inverted = Inverse(wide);
        REQUIRE(wide_inverted.HasValue());
        const Matrix4x4<Q32> result = wide_inverted.GetValue();
        CHECK(IsCloseTo(result(0, 3), -5.0, 1e-7));
        CHECK(IsCloseTo(result(1, 3), -10.0, 1e-7));
        CHECK(IsCloseTo(result(2, 3), 2.0, 1e-7));
        CHECK(result(0, 0) == Q32(1));
        CHECK(result(3, 3) == Q32(1));
    }
    SECTION("A singular matrix reports InvalidArgument")
    {
        const Matrix4x4<Q16> zero_row = Matrix4x4<Q16>::FromRows({Q16(1), Q16(2), Q16(3), Q16(4)}, {Q16(0), Q16(0), Q16(0), Q16(0)},
                                                                 {Q16(9), Q16(10), Q16(11), Q16(12)}, {Q16(13), Q16(14), Q16(21), Q16(27)});
        const auto inverted = Inverse(zero_row);
        REQUIRE_FALSE(inverted.HasValue());
        CHECK(inverted.GetError() == ErrorCode::InvalidArgument);
    }
    SECTION("A transform round trips at Q31.32")
    {
        // The pivot is inverted before it is used, so the width of the type is what decides whether the result is worth
        // anything. Q31.32 has the room; Q16.16 does not once a pivot strays far from one.
        Matrix4x4<Q32> transform = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(1), Q32(2), Q32(3)), Q32(37)).ToMatrix4x4();
        transform(0, 3) = Q32(2);
        transform(1, 3) = Q32(3);
        transform(2, 3) = Q32(-4);

        const auto inverted = Inverse(transform);
        REQUIRE(inverted.HasValue());
        const Matrix4x4<Q32> product = transform * inverted.GetValue();
        CHECK(IsEqual(product, Matrix4x4<Q32>(Q32(1)), Q32(1e-6)));

        const Point3<Q32> point(Q32(1.5), Q32(-2.25), Q32(0.75));
        const Point3<Q32> round_tripped = inverted.GetValue() * (transform * point);
        CHECK(IsCloseTo(round_tripped.x, 1.5, 1e-6));
        CHECK(IsCloseTo(round_tripped.y, -2.25, 1e-6));
        CHECK(IsCloseTo(round_tripped.z, 0.75, 1e-6));
    }
    SECTION("Cofactor")
    {
        Matrix4x4<Q16> mat(Q16(1));
        mat(0, 0) = Q16(2);
        CHECK(Cofactor(mat, 0, 0) == Q16(1));
        CHECK(Cofactor(mat, 1, 1) == Q16(2));
        CHECK(Cofactor(mat, 0, 1) == Q16(0));
    }
}

TEST_CASE("Quaternion accepts a fixed point element type", "[math][fixed-point][Quaternion]")
{
    static_assert(IsPOD<Quaternion<Q16>>);
    static_assert(SameAs<decltype(Length(Quaternion<Q16>::Identity())), Q16>);

    SECTION("Well known values")
    {
        const Quaternion<Q16> identity = Quaternion<Q16>::Identity();
        CHECK(identity.w == Q16(1));
        CHECK(identity.vec == Vector3<Q16>(Q16(0), Q16(0), Q16(0)));
        CHECK(Length(identity) == Q16(1));
        CHECK(LengthSquared(identity) == Q16(1));
        CHECK(Quaternion<Q16>::Zero() == Quaternion<Q16>(Q16(0), Q16(0), Q16(0), Q16(0)));
        CHECK(identity != Quaternion<Q16>::Zero());
    }
    SECTION("Arithmetic")
    {
        const Quaternion<Q16> a(Q16(1), Q16(2), Q16(3), Q16(4));
        const Quaternion<Q16> b(Q16(0.5), Q16(1), Q16(1.5), Q16(2));
        CHECK(a + b == Quaternion<Q16>(Q16(1.5), Q16(3), Q16(4.5), Q16(6)));
        CHECK(a - b == b);
        CHECK(a * Q16(0.5) == b);
        CHECK(Q16(0.5) * a == b);
        CHECK(a / Q16(2) == b);
        CHECK(-a == Quaternion<Q16>(Q16(-1), Q16(-2), Q16(-3), Q16(-4)));
        CHECK(Dot(a, b) == LengthSquared(a) * Q16(0.5));
    }
    SECTION("Length and normalization")
    {
        const Quaternion<Q16> q(Q16(0), Q16(3), Q16(4), Q16(0));
        CHECK(LengthSquared(q) == Q16(25));
        CHECK(Length(q) == Q16(5));
        const Quaternion<Q16> normalized = Normalize(q);
        CHECK(IsCloseTo(normalized.vec.x, 0.6, k_q16_tolerance));
        CHECK(IsCloseTo(normalized.vec.y, 0.8, k_q16_tolerance));
        CHECK(IsCloseTo(Length(normalized), 1.0, k_q16_tolerance));
    }
    SECTION("Conjugate and inverse")
    {
        const Quaternion<Q16> q = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(90));
        CHECK(Conjugate(q).w == q.w);
        CHECK(Conjugate(q).vec == -q.vec);
        CHECK_QUATERNION(q * Inverse(q), Quaternion<Q16>::Identity(), Q16(0.001));
    }
    SECTION("A type with no infinities is always finite")
    {
        const Quaternion<Q16> q(Q16(1), Q16(2), Q16(3), Q16(4));
        CHECK_FALSE(ContainsNonFinite(q));
        CHECK_FALSE(ContainsNaN(q));
    }
}

TEST_CASE("Fixed point quaternion rotation", "[math][fixed-point][Quaternion]")
{
    SECTION("A quarter turn about z is exact")
    {
        const Quaternion<Q16> q = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(90));
        const Vector3<Q16> rotated = q * Vector3<Q16>(Q16(1), Q16(0), Q16(0));
        CHECK(rotated == Vector3<Q16>(Q16(0), Q16(1), Q16(0)));
        const Point3<Q16> point = q * Point3<Q16>(Q16(1), Q16(0), Q16(0));
        CHECK(point == Point3<Q16>(Q16(0), Q16(1), Q16(0)));
    }
    SECTION("Against the double precision reference")
    {
        const Vector3<f64> axis(1.0, 2.0, 3.0);
        const Vector3<f64> vec(1.0, 0.0, 0.0);
        for (int i = 0; i <= 72; ++i)
        {
            const f64 degrees = static_cast<f64>(i) * 5.0;
            const Vector3<f64> expected = RotateReference(axis, degrees, vec);

            const Quaternion<Q16> q16 = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(1), Q16(2), Q16(3)), Q16(degrees));
            const Vector3<Q16> rotated16 = q16 * Vector3<Q16>(Q16(1), Q16(0), Q16(0));
            CHECK(IsCloseTo(rotated16.x, expected.x, k_q16_tolerance));
            CHECK(IsCloseTo(rotated16.y, expected.y, k_q16_tolerance));
            CHECK(IsCloseTo(rotated16.z, expected.z, k_q16_tolerance));

            const Quaternion<Q32> q32 = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(1), Q32(2), Q32(3)), Q32(degrees));
            const Vector3<Q32> rotated32 = q32 * Vector3<Q32>(Q32(1), Q32(0), Q32(0));
            CHECK(IsCloseTo(rotated32.x, expected.x, k_q32_tolerance));
            CHECK(IsCloseTo(rotated32.y, expected.y, k_q32_tolerance));
            CHECK(IsCloseTo(rotated32.z, expected.z, k_q32_tolerance));
        }
    }
    SECTION("The axis does not have to be normalized")
    {
        const Quaternion<Q16> unit = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(45));
        const Quaternion<Q16> scaled = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(4)), Q16(45));
        CHECK_QUATERNION(scaled, unit, Q16(0.001));
    }
    SECTION("Radians and degrees agree")
    {
        const Quaternion<Q32> from_degrees = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(1), Q32(0)), Q32(60));
        const Quaternion<Q32> from_radians = Quaternion<Q32>::FromAxisAngleRadians(Vector3<Q32>(Q32(0), Q32(1), Q32(0)), Radians(Q32(60)));
        CHECK(from_degrees == from_radians);
    }
}

TEST_CASE("Fixed point quaternion and matrix round trip", "[math][fixed-point][Quaternion]")
{
    SECTION("Through the trace")
    {
        const Quaternion<Q32> q = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(1), Q32(2), Q32(3)), Q32(37));
        const Quaternion<Q32> round_tripped(q.ToMatrix4x4());
        CHECK_QUATERNION(round_tripped, q, Q32(1e-6));
    }
    SECTION("Through the largest diagonal element")
    {
        // A real part of exactly zero is what sends the constructor down its other branch, and a half turn written out
        // by hand is the way to get one: the same rotation built from an axis and an angle lands a step off it.
        const Quaternion<Q32> q(Q32(0), Q32(1), Q32(0), Q32(0));
        const Quaternion<Q32> round_tripped(q.ToMatrix4x4());
        CHECK_QUATERNION(round_tripped, q, Q32(1e-6));
    }
    SECTION("The matrix agrees with rotating the vector")
    {
        const Quaternion<Q32> q = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(1), Q32(2), Q32(3)), Q32(37));
        const Vector3<Q32> vec(Q32(1), Q32(-2), Q32(0.5));
        const Vector3<Q32> by_quaternion = q * vec;
        const Vector3<Q32> by_matrix = q.ToMatrix3x3() * vec;
        CHECK_VECTOR3(by_matrix, by_quaternion, Q32(1e-6));
    }
    SECTION("A quarter turn about z is exact through the matrix")
    {
        const Quaternion<Q16> q = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(90));
        const Matrix3x3<Q16> mat = q.ToMatrix3x3();
        CHECK(mat(0, 0) == Q16(0));
        CHECK(mat(0, 1) == Q16(-1));
        CHECK(mat(1, 0) == Q16(1));
        CHECK(mat(2, 2) == Q16(1));
    }
}

TEST_CASE("Fixed point quaternion interpolation", "[math][fixed-point][Quaternion]")
{
    const Quaternion<Q32> start = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(0), Q32(1)), Q32(0));
    const Quaternion<Q32> end = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(0), Q32(1)), Q32(90));

    SECTION("The ends come back unchanged")
    {
        CHECK_QUATERNION(Lerp(Q32(0), start, end), start, Q32(1e-6));
        CHECK_QUATERNION(Lerp(Q32(1), start, end), end, Q32(1e-6));
        CHECK_QUATERNION(Slerp(Q32(0), start, end), start, Q32(1e-6));
        CHECK_QUATERNION(Slerp(Q32(1), start, end), end, Q32(1e-6));
    }
    SECTION("Half way is the half angle")
    {
        const Quaternion<Q32> middle = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(0), Q32(1)), Q32(45));
        CHECK_QUATERNION(Slerp(Q32(0.5), start, end), middle, Q32(1e-5));
    }
    SECTION("Constant angular velocity across the sweep")
    {
        for (int i = 0; i <= 20; ++i)
        {
            const f64 param = static_cast<f64>(i) / 20.0;
            const Quaternion<Q32> expected = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(0), Q32(1)), Q32(90.0 * param));
            CHECK_QUATERNION(Slerp(Q32(param), start, end), expected, Q32(1e-5));
        }
    }
    SECTION("Near parallel inputs take the linear shortcut")
    {
        const Quaternion<Q32> almost = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(0), Q32(0), Q32(1)), Q32(0.05));
        const Quaternion<Q32> interpolated = Slerp(Q32(0.5), start, almost);
        CHECK(IsCloseTo(Length(interpolated), 1.0, 1e-6));
        CHECK(IsCloseTo(interpolated.vec.z, std::sin(0.25 * 0.05 * 3.14159265358979324 / 180.0), 1e-6));
    }
    SECTION("Slerp holds at Q16.16 for a quarter turn")
    {
        // The unit length assert Slerp opens with allows a tenth of a milli, and an axis angle quaternion built at this
        // width lands within about half of that, so the margin is real but thin.
        const Quaternion<Q16> from = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(0));
        const Quaternion<Q16> to = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(90));
        const Quaternion<Q16> middle = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(0), Q16(0), Q16(1)), Q16(45));
        CHECK_QUATERNION(Slerp(Q16(0.5), from, to), middle, Q16(0.002));
    }
}

TEST_CASE("Fixed point matrix and quaternion are bit exact", "[math][fixed-point][Matrix][Quaternion]")
{
    // The expected values come from an independent model of the arithmetic, not from a previous run. They exist so that
    // the __int128 path and the _mul128 path cannot drift apart without saying so.
    SECTION("Q16.16")
    {
        const Quaternion<Q16> q = Quaternion<Q16>::FromAxisAngleDegrees(Vector3<Q16>(Q16(1), Q16(2), Q16(3)), Q16(37));
        CHECK(q.w.raw == 62149);
        CHECK(q.vec.x.raw == 5558);
        CHECK(q.vec.y.raw == 11116);
        CHECK(q.vec.z.raw == 16674);

        const Vector3<Q16> rotated = q * Vector3<Q16>(Q16(1), Q16(0), Q16(0));
        CHECK(rotated.x.raw == 53281);
        CHECK(rotated.y.raw == 33510);
        CHECK(rotated.z.raw == -18256);

        const Matrix3x3<Q16> mat = q.ToMatrix3x3();
        CHECK(mat(0, 0).raw == 53282);
        CHECK(mat(0, 1).raw == -29738);
        CHECK(mat(0, 2).raw == 23912);
        CHECK(mat(1, 0).raw == 33510);
        CHECK(mat(1, 1).raw == 56110);
        CHECK(mat(1, 2).raw == -4886);
        CHECK(mat(2, 0).raw == -18256);
        CHECK(mat(2, 1).raw == 16198);
        CHECK(mat(2, 2).raw == 60824);
    }
    SECTION("Q31.32")
    {
        const Quaternion<Q32> q = Quaternion<Q32>::FromAxisAngleDegrees(Vector3<Q32>(Q32(1), Q32(2), Q32(3)), Q32(37));
        CHECK(q.w.raw == 4073018385);
        CHECK(q.vec.x.raw == 364227127);
        CHECK(q.vec.y.raw == 728454254);
        CHECK(q.vec.z.raw == 1092681381);

        const Vector3<Q32> rotated = q * Vector3<Q32>(Q32(1), Q32(0), Q32(0));
        CHECK(rotated.x.raw == 3491888430);
        CHECK(rotated.y.raw == 2195982082);
        CHECK(rotated.z.raw == -1196295099);

        const Matrix3x3<Q32> mat = q.ToMatrix3x3();
        CHECK(mat(0, 0).raw == 3491888678);
        CHECK(mat(0, 1).raw == -1948880290);
        CHECK(mat(0, 2).raw == 1566946398);
        CHECK(mat(1, 0).raw == 2195981402);
        CHECK(mat(1, 1).raw == 3677214514);
        CHECK(mat(1, 2).raw == -320158612);
        CHECK(mat(2, 0).raw == -1196294730);
        CHECK(mat(2, 1).raw == 1061461952);
        CHECK(mat(2, 2).raw == 3986090904);
    }
}

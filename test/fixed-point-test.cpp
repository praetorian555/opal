#include <cmath>

#include "test-helpers.h"

#include "opal/container/hash-map.h"
#include "opal/math/fixed-point-vector.h"
#include "opal/math/fixed-point.h"

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

// A Q16.16 value carries about 1.5e-5 per step. The trigonometric functions quantise the angle on top of that, so give
// them a few steps of room. Q31.32 is limited by the approximating polynomial rather than by its own resolution.
constexpr double k_q16_tolerance = 3e-4;
constexpr double k_q32_tolerance = 1e-5;

constexpr double k_pi = 3.14159265358979324;
constexpr double k_quarter_pi = 0.25 * k_pi;
}  // namespace

TEST_CASE("FixedPoint traits", "[math][fixed-point]")
{
    static_assert(IsFixedPoint<Q16>);
    static_assert(IsFixedPoint<const Q32>);
    static_assert(!IsFixedPoint<f32>);
    static_assert(!IsFixedPoint<i32>);

    static_assert(IsPOD<Q16>);
    static_assert(IsPOD<Q32>);
    static_assert(HasUniqueObjectRepresentations<Q16>);

    static_assert(sizeof(Q16) == sizeof(i32));
    static_assert(sizeof(Q32) == sizeof(i64));
    static_assert(Q16::k_frac_bits_value == 16);
    static_assert(Q32::k_frac_bits_value == 32);
}

TEST_CASE("FixedPoint construction", "[math][fixed-point]")
{
    SECTION("From integers")
    {
        CHECK(Q16(0).raw == 0);
        CHECK(Q16(1).raw == 65536);
        CHECK(Q16(3).raw == 3 * 65536);
        CHECK(Q16(-3).raw == -3 * 65536);
        CHECK(Q32(1).raw == static_cast<i64>(1) << 32);
    }
    SECTION("From floating point")
    {
        CHECK(Q16(0.5).raw == 32768);
        CHECK(Q16(-0.5).raw == -32768);
        CHECK(Q16(0.25f).raw == 16384);
        CHECK(Q32(0.5).raw == static_cast<i64>(1) << 31);
    }
    SECTION("Rounds to the nearest representable value")
    {
        // 0.1 sits between two steps, closer to the one above.
        CHECK(Q16(0.1).raw == 6554);
        CHECK(Q16(-0.1).raw == -6554);
    }
    SECTION("Named values")
    {
        CHECK(Q16::Zero().raw == 0);
        CHECK(Q16::One().raw == 65536);
        CHECK(Q16::Epsilon().raw == 1);
        CHECK(Q16::Min().raw == -2147483647 - 1);
        CHECK(Q16::Max().raw == 2147483647);
        CHECK(IsCloseTo(Q16::Pi(), 3.14159265, k_q16_tolerance));
        CHECK(IsCloseTo(Q32::Pi(), 3.14159265, k_q32_tolerance));
    }
    SECTION("Round trips through raw")
    {
        CHECK(Q16::FromRaw(12345).raw == 12345);
        CHECK(Q16::FromRaw(Q16(2.5).raw) == Q16(2.5));
    }
    SECTION("Is usable in a constant expression")
    {
        constexpr Q16 k_two(2);
        constexpr Q16 k_half(0.5);
        static_assert(k_two.raw == 131072);
        static_assert(k_half.raw == 32768);
        static_assert(k_two + k_half == Q16(2.5));
        static_assert(-k_two < k_half);
    }
}

TEST_CASE("FixedPoint conversion", "[math][fixed-point]")
{
    CHECK(static_cast<double>(Q16(2.5)) == 2.5);
    CHECK(static_cast<float>(Q16(-0.75)) == -0.75f);
    CHECK(static_cast<double>(Q32(-2.5)) == -2.5);

    SECTION("To integer truncates towards zero")
    {
        CHECK(static_cast<i32>(Q16(2.75)) == 2);
        CHECK(static_cast<i32>(Q16(-2.75)) == -2);
        CHECK(static_cast<i32>(Q16(-0.5)) == 0);
        CHECK(static_cast<i64>(Q32(7.9)) == 7);
    }
}

TEST_CASE("FixedPoint arithmetic", "[math][fixed-point]")
{
    SECTION("Addition and subtraction are exact")
    {
        CHECK(Q16(0.5) + Q16(0.25) == Q16(0.75));
        CHECK(Q16(2) - Q16(5) == Q16(-3));
        CHECK(Q32(0.5) + Q32(0.25) == Q32(0.75));

        Q16 value(1);
        value += Q16(0.5);
        CHECK(value == Q16(1.5));
        value -= Q16(2);
        CHECK(value == Q16(-0.5));
    }
    SECTION("Multiplication")
    {
        CHECK(Q16(1.5) * Q16(2) == Q16(3));
        CHECK(Q16(-1.5) * Q16(2) == Q16(-3));
        CHECK(Q16(0.5) * Q16(0.5) == Q16(0.25));
        CHECK(Q32(0.5) * Q32(0.5) == Q32(0.25));

        Q16 value(3);
        value *= Q16(0.5);
        CHECK(value == Q16(1.5));
    }
    SECTION("Multiplication rounds halves up")
    {
        // Half a step exactly: the exact product is 0.5 raw units.
        CHECK((Q16::Epsilon() * Q16(0.5)).raw == 1);
        CHECK((Q32::Epsilon() * Q32(0.5)).raw == 1);
    }
    SECTION("Division")
    {
        CHECK(Q16(1) / Q16(4) == Q16(0.25));
        CHECK(Q16(-6) / Q16(3) == Q16(-2));
        CHECK(Q32(1) / Q32(4) == Q32(0.25));
        CHECK(Q32::Min() / Q32(1) == Q32::Min());

        Q16 value(3);
        value /= Q16(2);
        CHECK(value == Q16(1.5));
    }
    SECTION("Division truncates towards zero")
    {
        // One third is 21845.33 raw units.
        CHECK((Q16(1) / Q16(3)).raw == 21845);
        CHECK((Q16(-1) / Q16(3)).raw == -21845);
    }
    SECTION("Negation")
    {
        CHECK(-Q16(2.5) == Q16(-2.5));
        CHECK(+Q16(2.5) == Q16(2.5));
        CHECK(-Q16(0) == Q16(0));
    }
    SECTION("Integers convert on either side")
    {
        CHECK(1 - Q16(0.25) == Q16(0.75));
        CHECK(Q16(0.25) + 1 == Q16(1.25));
        CHECK(2 * Q16(1.5) == Q16(3));
    }
}

TEST_CASE("FixedPoint comparison", "[math][fixed-point]")
{
    CHECK(Q16(1) < Q16(2));
    CHECK(Q16(2) > Q16(1));
    CHECK(Q16(-1) < Q16(0));
    CHECK(Q16(1) <= Q16(1));
    CHECK(Q16(1) >= Q16(1));
    CHECK(Q16(1) != Q16(2));
    CHECK(Q16(1) == Q16(1));
    CHECK(Q16(0) < 1);
    CHECK(1 > Q16(0));
}

TEST_CASE("FixedPoint generic math functions", "[math][fixed-point]")
{
    SECTION("Abs")
    {
        CHECK(Abs(Q16(-2.5)) == Q16(2.5));
        CHECK(Abs(Q16(2.5)) == Q16(2.5));
    }
    SECTION("Min and Max")
    {
        CHECK(Opal::Min(Q16(1), Q16(2)) == Q16(1));
        CHECK(Opal::Max(Q16(1), Q16(2)) == Q16(2));
    }
    SECTION("Clamp")
    {
        CHECK(Clamp(Q16(5), Q16(0), Q16(1)) == Q16(1));
        CHECK(Clamp(Q16(-5), Q16(0), Q16(1)) == Q16(0));
        CHECK(Clamp(Q16(0.5), Q16(0), Q16(1)) == Q16(0.5));
    }
    SECTION("IsEqual")
    {
        CHECK(IsEqual(Q16(1), Q16(1) + Q16::Epsilon(), Q16::Epsilon()));
        CHECK_FALSE(IsEqual(Q16(1), Q16(1.5), Q16::Epsilon()));
    }
    SECTION("There are no infinities and no NaN")
    {
        CHECK(IsFinite(Q16::Max()));
        CHECK(IsFinite(Q16::Min()));
        CHECK_FALSE(IsNaN(Q16(1)));
    }
}

TEST_CASE("FixedPoint rounding", "[math][fixed-point]")
{
    SECTION("Floor")
    {
        CHECK(Floor(Q16(2.5)) == Q16(2));
        CHECK(Floor(Q16(-2.5)) == Q16(-3));
        CHECK(Floor(Q16(2)) == Q16(2));
        CHECK(Floor(Q16(-2)) == Q16(-2));
    }
    SECTION("Ceil")
    {
        CHECK(Ceil(Q16(2.5)) == Q16(3));
        CHECK(Ceil(Q16(-2.5)) == Q16(-2));
        CHECK(Ceil(Q16(2)) == Q16(2));
        CHECK(Ceil(Q16(-2)) == Q16(-2));
    }
    SECTION("Round goes away from zero on a half")
    {
        CHECK(Round(Q16(2.5)) == Q16(3));
        CHECK(Round(Q16(-2.5)) == Q16(-3));
        CHECK(Round(Q16(2.4)) == Q16(2));
        CHECK(Round(Q16(-2.4)) == Q16(-2));
        CHECK(Round(Q16(2.6)) == Q16(3));
        CHECK(Round(Q16(-2.6)) == Q16(-3));
        CHECK(Round(Q16(2)) == Q16(2));
        CHECK(Round(Q16::Min()) == Q16::Min());
    }
    SECTION("Mod keeps the sign of the dividend")
    {
        CHECK(Mod(Q16(5.5), Q16(2)) == Q16(1.5));
        CHECK(Mod(Q16(-5.5), Q16(2)) == Q16(-1.5));
        CHECK(Mod(Q16(4), Q16(2)) == Q16(0));
        CHECK(Mod(Q16(5.5), Q16::Epsilon()) == Q16(0));
        CHECK(Mod(Q16::Min(), -Q16::Epsilon()) == Q16(0));
    }
}

TEST_CASE("FixedPoint square root", "[math][fixed-point]")
{
    SECTION("Exact squares")
    {
        CHECK(Sqrt(Q16(4)) == Q16(2));
        CHECK(Sqrt(Q16(25)) == Q16(5));
        CHECK(Sqrt(Q16(1)) == Q16(1));
        CHECK(Sqrt(Q16(0)) == Q16(0));
        CHECK(Sqrt(Q32(25)) == Q32(5));
    }
    SECTION("Matches the reference within a step")
    {
        for (int i = 1; i <= 200; ++i)
        {
            const double expected = std::sqrt(static_cast<double>(i) * 0.25);
            CHECK(IsCloseTo(Sqrt(Q16(static_cast<double>(i) * 0.25)), expected, 2.0 / 65536.0));
            CHECK(IsCloseTo(Sqrt(Q32(static_cast<double>(i) * 0.25)), expected, 1e-8));
        }
    }
    SECTION("Small values")
    {
        CHECK(IsCloseTo(Sqrt(Q16(0.25)), 0.5, 2.0 / 65536.0));
        CHECK(IsCloseTo(Sqrt(Q16::Epsilon()), std::sqrt(1.0 / 65536.0), 2.0 / 65536.0));
    }
}

TEST_CASE("FixedPoint narrow and single integer bit formats", "[math][fixed-point]")
{
    SECTION("16-bit storage")
    {
        using Q8 = FixedPoint<i16, 8>;
        CHECK((Q8(1.5) * Q8(2)).raw == 768);
        CHECK((Q8(1) / Q8(4)).raw == 64);
        CHECK(Sqrt(Q8(4)) == Q8(2));
        CHECK(IsCloseTo(Sin(Q8(1)), std::sin(1.0), 0.02));
    }
    SECTION("A single integer bit")
    {
        // The square root of anything at or above one needs a starting guess the storage cannot hold as a power of two.
        using Q30 = FixedPoint<i32, 30>;
        CHECK(Sqrt(Q30(1)) == Q30(1));
        CHECK(IsCloseTo(Sqrt(Q30(1.5)), std::sqrt(1.5), 4.0 / 1073741824.0));
        CHECK(IsCloseTo(Sqrt(Q30::Max()), std::sqrt(2.0), 4.0 / 1073741824.0));
        using Q62 = FixedPoint<i64, 62>;
        CHECK(IsCloseTo(Sqrt(Q62::Max()), std::sqrt(2.0), 1e-9));
    }
}

TEST_CASE("FixedPoint interpolation", "[math][fixed-point]")
{
    CHECK(Lerp(Q16(0), Q16(2), Q16(6)) == Q16(2));
    CHECK(Lerp(Q16(1), Q16(2), Q16(6)) == Q16(6));
    CHECK(Lerp(Q16(0.5), Q16(2), Q16(6)) == Q16(4));
    CHECK(Lerp(Q32(0.25), Q32(0), Q32(8)) == Q32(2));
}

TEST_CASE("FixedPoint angle conversion", "[math][fixed-point]")
{
    CHECK(IsCloseTo(Radians(Q16(180)), 3.14159265, k_q16_tolerance));
    CHECK(IsCloseTo(Radians(Q16(90)), 1.57079633, k_q16_tolerance));
    CHECK(IsCloseTo(Degrees(Q16::Pi()), 180.0, 0.02));
    CHECK(IsCloseTo(Radians(Q32(180)), 3.14159265, k_q32_tolerance));
    CHECK(IsCloseTo(Degrees(Q32::Pi()), 180.0, k_q32_tolerance));
}

TEST_CASE("FixedPoint trigonometry", "[math][fixed-point]")
{
    SECTION("Anchor points")
    {
        CHECK(Sin(Q16(0)) == Q16(0));
        CHECK(IsCloseTo(Cos(Q16(0)), 1.0, k_q16_tolerance));
        CHECK(IsCloseTo(Sin(Q16::HalfPi()), 1.0, k_q16_tolerance));
        CHECK(IsCloseTo(Cos(Q16::HalfPi()), 0.0, k_q16_tolerance));
        CHECK(IsCloseTo(Sin(Q16::Pi()), 0.0, k_q16_tolerance));
        CHECK(IsCloseTo(Cos(Q16::Pi()), -1.0, k_q16_tolerance));
    }
    SECTION("Sweeps a full turn")
    {
        for (int i = -360; i <= 360; ++i)
        {
            const double radians = static_cast<double>(i) * 3.14159265358979324 / 180.0;
            CHECK(IsCloseTo(Sin(Q16(radians)), std::sin(radians), k_q16_tolerance));
            CHECK(IsCloseTo(Cos(Q16(radians)), std::cos(radians), k_q16_tolerance));
            CHECK(IsCloseTo(Sin(Q32(radians)), std::sin(radians), k_q32_tolerance));
            CHECK(IsCloseTo(Cos(Q32(radians)), std::cos(radians), k_q32_tolerance));
        }
    }
    SECTION("Reduces angles far outside a single turn")
    {
        const double radians = 100.0;
        CHECK(IsCloseTo(Sin(Q16(radians)), std::sin(radians), 1e-2));
        CHECK(IsCloseTo(Sin(Q32(radians)), std::sin(radians), k_q32_tolerance));
    }
    SECTION("Never leaves the range of minus one to one")
    {
        for (int i = -2000; i <= 2000; ++i)
        {
            const Q16 angle(static_cast<double>(i) * 0.01);
            CHECK(Sin(angle) <= Q16(1));
            CHECK(Sin(angle) >= Q16(-1));
            CHECK(Cos(angle) <= Q16(1));
            CHECK(Cos(angle) >= Q16(-1));
        }
    }
    SECTION("Tangent")
    {
        CHECK(Tan(Q16(0)) == Q16(0));
        CHECK(IsCloseTo(Tan(Q16(0.7853981634)), 1.0, 1e-3));
        CHECK(IsCloseTo(Tan(Q16(-0.7853981634)), -1.0, 1e-3));
        CHECK(IsCloseTo(Tan(Q32(0.7853981634)), 1.0, k_q32_tolerance));
    }
    SECTION("Tangent saturates at a pole instead of overflowing")
    {
        const Q16 tangent = Tan(Q16::HalfPi());
        CHECK((tangent == Q16::Max() || tangent == Q16::Min() || Abs(tangent) > Q16(1000)));
    }
}

TEST_CASE("FixedPoint inverse trigonometry", "[math][fixed-point]")
{
    SECTION("Anchor points")
    {
        CHECK(ArcTan(Q16(0)) == Q16(0));
        CHECK(IsCloseTo(ArcTan(Q16(1)), k_quarter_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan(Q16(-1)), -k_quarter_pi, k_q16_tolerance));
        CHECK(ArcSin(Q16(0)) == Q16(0));
        CHECK(IsCloseTo(ArcSin(Q16(1)), 0.5 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcSin(Q16(-1)), -0.5 * k_pi, k_q16_tolerance));
        CHECK(ArcCos(Q16(1)) == Q16(0));
        CHECK(IsCloseTo(ArcCos(Q16(0)), 0.5 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcCos(Q16(-1)), k_pi, k_q16_tolerance));
    }
    SECTION("Arc tangent of two arguments covers every quadrant")
    {
        CHECK(IsCloseTo(ArcTan2(Q16(1), Q16(1)), 0.25 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan2(Q16(1), Q16(-1)), 0.75 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan2(Q16(-1), Q16(-1)), -0.75 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan2(Q16(-1), Q16(1)), -0.25 * k_pi, k_q16_tolerance));
    }
    SECTION("Arc tangent of two arguments on the axes")
    {
        CHECK(ArcTan2(Q16(0), Q16(0)) == Q16(0));
        CHECK(ArcTan2(Q16(0), Q16(2)) == Q16(0));
        CHECK(IsCloseTo(ArcTan2(Q16(0), Q16(-2)), k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan2(Q16(2), Q16(0)), 0.5 * k_pi, k_q16_tolerance));
        CHECK(IsCloseTo(ArcTan2(Q16(-2), Q16(0)), -0.5 * k_pi, k_q16_tolerance));
    }
    SECTION("Sweeps the arc tangent")
    {
        for (int i = -400; i <= 400; ++i)
        {
            const double value = static_cast<double>(i) * 0.05;
            CHECK(IsCloseTo(ArcTan(Q16(value)), std::atan(value), k_q16_tolerance));
            CHECK(IsCloseTo(ArcTan(Q32(value)), std::atan(value), k_q32_tolerance));
        }
    }
    SECTION("Sweeps the arc tangent of two arguments")
    {
        for (int i = -32; i <= 32; ++i)
        {
            for (int j = -32; j <= 32; ++j)
            {
                const double y = static_cast<double>(i) * 0.125;
                const double x = static_cast<double>(j) * 0.125;
                CHECK(IsCloseTo(ArcTan2(Q16(y), Q16(x)), std::atan2(y, x), k_q16_tolerance));
                CHECK(IsCloseTo(ArcTan2(Q32(y), Q32(x)), std::atan2(y, x), k_q32_tolerance));
            }
        }
    }
    SECTION("Sweeps the arc sine and the arc cosine")
    {
        for (int i = -1000; i <= 1000; ++i)
        {
            const double value = static_cast<double>(i) * 0.001;
            CHECK(IsCloseTo(ArcSin(Q16(value)), std::asin(value), k_q16_tolerance));
            CHECK(IsCloseTo(ArcCos(Q16(value)), std::acos(value), k_q16_tolerance));
            CHECK(IsCloseTo(ArcSin(Q32(value)), std::asin(value), k_q32_tolerance));
            CHECK(IsCloseTo(ArcCos(Q32(value)), std::acos(value), k_q32_tolerance));
        }
    }
    SECTION("Round trips against the forward functions")
    {
        for (int i = -150; i <= 150; ++i)
        {
            const Q32 angle(static_cast<double>(i) * 0.01);
            CHECK(IsCloseTo(ArcSin(Sin(angle)), static_cast<double>(angle), 1e-5));
            CHECK(IsCloseTo(ArcTan2(Sin(angle), Cos(angle)), static_cast<double>(angle), 1e-5));
        }
    }
    SECTION("Never leaves the range of the function")
    {
        for (int i = -1000; i <= 1000; ++i)
        {
            const Q16 value(static_cast<double>(i) * 0.001);
            CHECK(ArcSin(value) >= Q16(-2));
            CHECK(ArcSin(value) <= Q16(2));
            CHECK(ArcCos(value) >= Q16(0));
            CHECK(ArcCos(value) <= Q16(4));
        }
    }
    SECTION("Narrow storage")
    {
        using Q8 = FixedPoint<i16, 8>;
        CHECK(ArcTan(Q8(0)) == Q8(0));
        CHECK(IsCloseTo(ArcTan(Q8(1)), k_quarter_pi, 0.02));
        CHECK(IsCloseTo(ArcTan2(Q8(-1), Q8(-1)), -0.75 * k_pi, 0.02));
        CHECK(IsCloseTo(ArcCos(Q8(0.5)), k_pi / 3.0, 0.02));
    }
}

TEST_CASE("FixedPoint is bit exact", "[math][fixed-point]")
{
    // The point of the type is that every platform agrees on the bits, so these pin the exact result rather than a
    // tolerance. The expected values come from an independent model of the arithmetic, not from a previous run.
    SECTION("Q16.16")
    {
        CHECK(Q16(0.1).raw == 6554);
        CHECK((Q16(0.1) * Q16(0.1)).raw == 655);
        CHECK((Q16(1) / Q16(3)).raw == 21845);
        CHECK(Sqrt(Q16(2)).raw == 92681);
        CHECK(Sin(Q16(1)).raw == 55145);
        CHECK(Cos(Q16(1)).raw == 35411);
        CHECK(Sin(Q16(-1)).raw == -55145);
        CHECK(ArcTan(Q16(1)).raw == 51472);
        CHECK(ArcTan(Q16(0.5)).raw == 30387);
        CHECK(ArcTan(Q16(-2)).raw == -72557);
        CHECK(ArcTan2(Q16(1), Q16(-1)).raw == 154415);
        CHECK(ArcSin(Q16(0.5)).raw == 34315);
        CHECK(ArcCos(Q16(0.5)).raw == 68629);
    }
    SECTION("Q31.32")
    {
        CHECK(Q32(0.1).raw == 429496730);
        CHECK((Q32(0.1) * Q32(0.1)).raw == 42949673);
        CHECK((Q32(1) / Q32(3)).raw == 1431655765);
        CHECK(Sqrt(Q32(2)).raw == 6074000999);
        CHECK(Sin(Q32(1)).raw == 3614090207);
        CHECK(Cos(Q32(1)).raw == 2320580730);
        CHECK(Sin(Q32(-1)).raw == -3614090207);
        CHECK(ArcTan(Q32(1)).raw == 3373259425);
        CHECK(ArcTan(Q32(0.5)).raw == 1991351317);
        CHECK(ArcTan(Q32(-2)).raw == -4755167535);
        CHECK(ArcTan2(Q32(1), Q32(-1)).raw == 10119778280);
        CHECK(ArcSin(Q32(0.5)).raw == 2248839617);
        CHECK(ArcCos(Q32(0.5)).raw == 4497679235);
    }
}

TEST_CASE("FixedPoint in vectors", "[math][fixed-point]")
{
    using Vec2 = Vector2<Q16>;
    using Vec3 = Vector3<Q16>;

    SECTION("Arithmetic")
    {
        const Vec2 a(Q16(1), Q16(2));
        const Vec2 b(Q16(0.5), Q16(0.25));
        CHECK(a + b == Vec2(Q16(1.5), Q16(2.25)));
        CHECK(a - b == Vec2(Q16(0.5), Q16(1.75)));
        CHECK(a * Q16(2) == Vec2(Q16(2), Q16(4)));
        CHECK(Vec2::Zero() == Vec2(Q16(0), Q16(0)));
    }
    SECTION("Products")
    {
        const Vec2 a(Q16(1), Q16(2));
        const Vec2 b(Q16(3), Q16(4));
        CHECK(Dot(a, b) == Q16(11));
        CHECK(Cross(a, b) == Q16(-2));
        CHECK(LengthSquared(Vec2(Q16(3), Q16(4))) == Q16(25));
    }
    SECTION("Length keeps the fixed point type")
    {
        const Vec2 vec(Q16(3), Q16(4));
        static_assert(SameAs<decltype(Length(vec)), Q16>);
        CHECK(Length(vec) == Q16(5));
        CHECK(Length(Vec3(Q16(2), Q16(3), Q16(6))) == Q16(7));
    }
    SECTION("Normalize")
    {
        const Vec2 normalized = Normalize(Vec2(Q16(3), Q16(4)));
        CHECK(IsCloseTo(normalized.x, 0.6, k_q16_tolerance));
        CHECK(IsCloseTo(normalized.y, 0.8, k_q16_tolerance));
        CHECK(IsCloseTo(Length(normalized), 1.0, k_q16_tolerance));
    }
    SECTION("Generic helpers still apply")
    {
        const Vec2 a(Q16(0), Q16(0));
        const Vec2 b(Q16(4), Q16(8));
        CHECK(Lerp(Q16(0.5), a, b) == Vec2(Q16(2), Q16(4)));
        CHECK(Opal::Min(a, b) == a);
        CHECK(Opal::Max(a, b) == b);
        CHECK(Abs(Vec2(Q16(-1), Q16(2))) == Vec2(Q16(1), Q16(2)));
        CHECK_FALSE(ContainsNonFinite(b));
        CHECK_FALSE(ContainsNaN(b));
    }
}

TEST_CASE("FixedPoint in points", "[math][fixed-point]")
{
    using P2 = Point2<Q16>;
    using P3 = Point3<Q16>;

    const P2 a(Q16(1), Q16(2));
    const P2 b(Q16(4), Q16(6));
    static_assert(SameAs<decltype(Distance(a, b)), Q16>);
    CHECK(Distance(a, b) == Q16(5));
    CHECK(DistanceSquared(a, b) == Q16(25));
    CHECK(Distance(P3(Q16(0), Q16(0), Q16(0)), P3(Q16(2), Q16(3), Q16(6))) == Q16(7));
}

TEST_CASE("FixedPoint as a hash map key", "[math][fixed-point]")
{
    HashMap<Q16, i32> map;
    CHECK(map.Insert(Q16(1.5), 10) == ErrorCode::Success);
    CHECK(map.Insert(Q16(-2.25), 20) == ErrorCode::Success);

    CHECK(map.Contains(Q16(1.5)));
    CHECK(map.Contains(Q16(-2.25)));
    CHECK_FALSE(map.Contains(Q16(3)));
    CHECK(map.GetValue(Q16(1.5)) == 10);
    CHECK(map.GetValue(Q16(-2.25)) == 20);

    // Equal keys have to hash the same however they were built.
    CHECK(map.GetValue(Q16(3) / Q16(2)) == 10);
}

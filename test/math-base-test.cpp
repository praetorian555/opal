#include "opal/defines.h"

#include <limits>

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/math-base.h"

TEST_CASE("Constants", "[BaseTests]")
{
    CHECK(Opal::k_inf_float == Catch::Approx(std::numeric_limits<float>::infinity()));
    CHECK(Opal::k_neg_inf_float == Catch::Approx(-std::numeric_limits<float>::infinity()));
    CHECK(Opal::k_smallest_float == Catch::Approx(std::numeric_limits<float>::lowest()));
    CHECK(Opal::k_largest_float == Catch::Approx(std::numeric_limits<float>::max()));

    CHECK(Opal::k_inf_double == Catch::Approx(std::numeric_limits<double>::infinity()));
    CHECK(Opal::k_neg_inf_double == Catch::Approx(-std::numeric_limits<double>::infinity()));
    CHECK(Opal::k_smallest_double == Catch::Approx(std::numeric_limits<double>::lowest()));
    CHECK(Opal::k_largest_double == Catch::Approx(std::numeric_limits<double>::max()));
}

TEST_CASE("NaN", "[BaseTests]")
{
    SECTION("float")
    {
        const float a = 5;
        const float b = 10.2f;
        const float c = nanf("");
        CHECK_FALSE(Opal::IsNaN(a));
        CHECK_FALSE(Opal::IsNaN(b));
        CHECK(Opal::IsNaN(c));
    }
    SECTION("double")
    {
        const double a = 5;
        const double b = 10.2;
        const double c = nanf("");  // using nanf as in the original test
        CHECK_FALSE(Opal::IsNaN(a));
        CHECK_FALSE(Opal::IsNaN(b));
        CHECK(Opal::IsNaN(c));
    }
    SECTION("int")
    {
        const int a = 5;
        CHECK_FALSE(Opal::IsNaN(a));
    }
}

TEST_CASE("IsFinite", "[BaseTests]")
{
    SECTION("float")
    {
        const float a = 5;
        const float b = 10.2f;
        const float c = nanf("");
        CHECK(Opal::IsFinite(a));
        CHECK(Opal::IsFinite(b));
        CHECK_FALSE(Opal::IsFinite(c));
    }
    SECTION("double")
    {
        const double a = 5;
        const double b = 10.2;
        const double c = nanf("");
        CHECK(Opal::IsFinite(a));
        CHECK(Opal::IsFinite(b));
        CHECK_FALSE(Opal::IsFinite(c));
    }
}

TEST_CASE("Clamp", "[BaseTests]")
{
    SECTION("int")
    {
        const int V1 = Opal::Clamp(10, -2, 5);
        CHECK(V1 == 5);
        const int V2 = Opal::Clamp(-3, -2, 5);
        CHECK(V2 == -2);
        const int V3 = Opal::Clamp(0, -2, 5);
        CHECK(V3 == 0);
        const int V4 = Opal::Clamp(0, -2, -2);
        CHECK(V4 == -2);
    }
    SECTION("float")
    {
        const float V1 = Opal::Clamp(10.0f, -2.0f, 5.0f);
        CHECK(V1 == Catch::Approx(5.0f));
        const float V2 = Opal::Clamp(-3.0f, -2.0f, 5.0f);
        CHECK(V2 == Catch::Approx(-2.0f));
        const float V3 = Opal::Clamp(0.0f, -2.0f, 5.0f);
        CHECK(V3 == Catch::Approx(0.0f));
        const float V4 = Opal::Clamp(0.0f, -2.0f, -2.0f);
        CHECK(V4 == Catch::Approx(-2.0f));
    }
    SECTION("double")
    {
        const double V1 = Opal::Clamp(10.0, -2.0, 5.0);
        CHECK(V1 == Catch::Approx(5.0));
        const double V2 = Opal::Clamp(-3.0, -2.0, 5.0);
        CHECK(V2 == Catch::Approx(-2.0));
        const double V3 = Opal::Clamp(0.0, -2.0, 5.0);
        CHECK(V3 == Catch::Approx(0.0));
        const double V4 = Opal::Clamp(0.0, -2.0, -2.0);
        CHECK(V4 == Catch::Approx(-2.0));
    }
}

TEST_CASE("Mod", "[BaseTests]")
{
    SECTION("float")
    {
        const float R1 = Opal::Mod(2.5f, 2.0f);
        CHECK(R1 == Catch::Approx(0.5f));
        const float R7 = Opal::Mod(3.9f, 2.0f);
        CHECK(R7 == Catch::Approx(1.9f));
        const float R2 = Opal::Mod(-2.5f, 2.0f);
        CHECK(R2 == Catch::Approx(-0.5f));
        const float R3 = Opal::Mod(2.5f, -2.0f);
        CHECK(R3 == Catch::Approx(0.5f));
        const float R4 = Opal::Mod(NAN, -2.0f);
        CHECK(Opal::IsNaN(R4));
        const float R5 = Opal::Mod(2.5f, NAN);
        CHECK(Opal::IsNaN(R5));
        const float R6 = Opal::Mod(2.5f, 0.0f);
        CHECK(Opal::IsNaN(R6));
    }
    SECTION("double")
    {
        const double R1 = Opal::Mod(2.5, 2.0);
        CHECK(R1 == Catch::Approx(0.5));
        const double R7 = Opal::Mod(3.9, 2.0);
        CHECK(R7 == Catch::Approx(1.9));
        const double R2 = Opal::Mod(-2.5, 2.0);
        CHECK(R2 == Catch::Approx(-0.5));
        const double R3 = Opal::Mod(2.5, -2.0);
        CHECK(R3 == Catch::Approx(0.5));
        const double R4 = Opal::Mod(static_cast<double>(NAN), -2.0);
        CHECK(Opal::IsNaN(R4));
        const double R5 = Opal::Mod(2.5, static_cast<double>(NAN));
        CHECK(Opal::IsNaN(R5));
        const double R6 = Opal::Mod(2.5, 0.0);
        CHECK(Opal::IsNaN(R6));
    }
    SECTION("int")
    {
        const int R1 = Opal::Mod(3, 2);
        CHECK(R1 == 1);
        const int R2 = Opal::Mod(-3, 2);
        CHECK(R2 == -1);
        const int R3 = Opal::Mod(3, -2);
        CHECK(R3 == 1);
        const int R4 = Opal::Mod(5, 3);
        CHECK(R4 == 2);
    }
}

TEST_CASE("Lerp", "[BaseTests]")
{
    SECTION("float")
    {
        const float R1 = Opal::Lerp(0.0f, 1.0f, 5.0f);
        CHECK(R1 == Catch::Approx(1.0f));
        const float R2 = Opal::Lerp(0.0f, 1.0f, 0.0f);
        CHECK(R2 == Catch::Approx(1.0f));
        const float R3 = Opal::Lerp(0.5f, 1.0f, 5.0f);
        CHECK(R3 == Catch::Approx(3.0f));
        const float R4 = Opal::Lerp(1.0f, 1.0f, 0.0f);
        CHECK(R4 == Catch::Approx(0.0f));
    }
    SECTION("double")
    {
        const double R1 = Opal::Lerp(0.0, 1.0, 5.0);
        CHECK(R1 == Catch::Approx(1.0));
        const double R2 = Opal::Lerp(0.5, 1.0, 5.0);
        CHECK(R2 == Catch::Approx(3.0));
        const double R3 = Opal::Lerp(0.0, 1.0, 0.0);
        CHECK(R3 == Catch::Approx(1.0));
        const double R4 = Opal::Lerp(1.0, 1.0, 0.0);
        CHECK(R4 == Catch::Approx(0.0));
    }
}

TEST_CASE("Radians", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Radians(180.0f) == Catch::Approx(Opal::k_pi_float));
        CHECK(Opal::Radians(90.0f) == Catch::Approx(Opal::k_pi_over_2_float));
    }
    SECTION("double")
    {
        CHECK(Opal::Radians(180.0) == Catch::Approx(Opal::k_pi_double));
        CHECK(Opal::Radians(90.0) == Catch::Approx(Opal::k_pi_over_2_double));
    }
}

TEST_CASE("Degrees", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Degrees(Opal::k_pi_float) == Catch::Approx(180.0f));
        CHECK(Opal::Degrees(Opal::k_pi_over_2_float) == Catch::Approx(90.0f));
    }
    SECTION("double")
    {
        CHECK(Opal::Degrees(Opal::k_pi_double) == Catch::Approx(180.0));
        CHECK(Opal::Degrees(Opal::k_pi_over_2_double) == Catch::Approx(90.0));
    }
}

TEST_CASE("Min", "[BaseTests]")
{
    SECTION("int")
    {
        CHECK(Opal::Min(1, 2) == 1);
        CHECK(Opal::Min(2, 1) == 1);
    }
    SECTION("float")
    {
        CHECK(Opal::Min(1.0f, 2.0f) == Catch::Approx(1.0f));
        CHECK(Opal::Min(2.0f, 1.0f) == Catch::Approx(1.0f));
    }
    SECTION("double")
    {
        CHECK(Opal::Min(1.0, 2.0) == Catch::Approx(1.0));
        CHECK(Opal::Min(2.0, 1.0) == Catch::Approx(1.0));
    }
}

TEST_CASE("Max", "[BaseTests]")
{
    SECTION("int")
    {
        CHECK(Opal::Max(1, 2) == 2);
        CHECK(Opal::Max(2, 1) == 2);
    }
    SECTION("float")
    {
        CHECK(Opal::Max(1.0f, 2.0f) == Catch::Approx(2.0f));
        CHECK(Opal::Max(2.0f, 1.0f) == Catch::Approx(2.0f));
    }
    SECTION("double")
    {
        CHECK(Opal::Max(1.0, 2.0) == Catch::Approx(2.0));
        CHECK(Opal::Max(2.0, 1.0) == Catch::Approx(2.0));
    }
}

TEST_CASE("Round", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Round(0.5f) == Catch::Approx(1));
        CHECK(Opal::Round(1.5f) == Catch::Approx(2));
        CHECK(Opal::Round(1.6f) == Catch::Approx(2));
        CHECK(Opal::Round(2.5f) == Catch::Approx(3));
        CHECK(Opal::Round(-2.5f) == Catch::Approx(-3));
        CHECK(Opal::Round(2.6f) == Catch::Approx(3));
    }
    SECTION("double")
    {
        CHECK(Opal::Round(0.5) == Catch::Approx(1));
        CHECK(Opal::Round(1.5) == Catch::Approx(2));
        CHECK(Opal::Round(1.6) == Catch::Approx(2));
        CHECK(Opal::Round(2.5) == Catch::Approx(3));
        CHECK(Opal::Round(-2.5) == Catch::Approx(-3));
        CHECK(Opal::Round(2.6) == Catch::Approx(3));
    }
}

TEST_CASE("Floor", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Floor(1.5f) == Catch::Approx(1));
        CHECK(Opal::Floor(-1.5f) == Catch::Approx(-2));
        CHECK(Opal::Floor(2.5f) == Catch::Approx(2));
    }
    SECTION("double")
    {
        CHECK(Opal::Floor(1.5) == Catch::Approx(1));
        CHECK(Opal::Floor(-1.5) == Catch::Approx(-2));
        CHECK(Opal::Floor(2.5) == Catch::Approx(2));
    }
}

TEST_CASE("Ceil", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Ceil(1.5f) == Catch::Approx(2));
        CHECK(Opal::Ceil(2.5f) == Catch::Approx(3));
    }
    SECTION("double")
    {
        CHECK(Opal::Ceil(1.5) == Catch::Approx(2));
        CHECK(Opal::Ceil(2.5) == Catch::Approx(3));
    }
}

TEST_CASE("Abs", "[BaseTests]")
{
    SECTION("int")
    {
        CHECK(Opal::Abs(-1) == 1);
        CHECK(Opal::Abs(1) == 1);
    }
    SECTION("float")
    {
        CHECK(Opal::Abs(-1.0f) == Catch::Approx(1.0f));
        CHECK(Opal::Abs(1.0f) == Catch::Approx(1.0f));
    }
    SECTION("double")
    {
        CHECK(Opal::Abs(-1.0) == Catch::Approx(1.0));
        CHECK(Opal::Abs(1.0) == Catch::Approx(1.0));
    }
}

TEST_CASE("Sqrt", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Sqrt(1.0f) == Catch::Approx(1.0f));
        CHECK(Opal::Sqrt(4.0f) == Catch::Approx(2.0f));
    }
    SECTION("double")
    {
        CHECK(Opal::Sqrt(1.0) == Catch::Approx(1.0));
        CHECK(Opal::Sqrt(4.0) == Catch::Approx(2.0));
    }
}

TEST_CASE("Trigonometry", "[BaseTests]")
{
    SECTION("float")
    {
        constexpr float k_epsilon = 1e-6f;
        CHECK(Opal::Sin(0.0f) == Catch::Approx(0.0f));
        CHECK(Opal::Sin(Opal::k_pi_over_2_float) == Catch::Approx(1.0f));
        CHECK(Opal::Sin(Opal::k_pi_float + Opal::k_pi_over_2_float) == Catch::Approx(-1.0f));
        CHECK(Opal::IsEqual(0.0f, Opal::Sin(Opal::k_pi_float), k_epsilon));
        CHECK(Opal::IsEqual(0.0f, Opal::Sin(Opal::k_pi_float * 2), k_epsilon));

        CHECK(Opal::Cos(0.0f) == Catch::Approx(1.0f));
        CHECK(Opal::Cos(Opal::k_pi_float * 2) == Catch::Approx(1.0f));
        CHECK(Opal::Cos(Opal::k_pi_float) == Catch::Approx(-1.0f));
        CHECK(Opal::IsEqual(0.0f, Opal::Cos(Opal::k_pi_float + Opal::k_pi_over_2_float), k_epsilon));
        CHECK(Opal::IsEqual(0.0f, Opal::Cos(Opal::k_pi_over_2_float), k_epsilon));

        CHECK(Opal::Tan(0.0f) == Catch::Approx(0.0f));
        CHECK(Opal::Tan(Opal::k_pi_over_4_float) == Catch::Approx(1.0f));
        CHECK(Opal::IsEqual(1.0f, Opal::Tan(Opal::k_pi_float + Opal::k_pi_over_4_float), k_epsilon));
        CHECK(Opal::IsEqual(0.0f, Opal::Tan(Opal::k_pi_float * 2), k_epsilon));
    }
    SECTION("double")
    {
        constexpr double k_epsilon = 1e-15;
        CHECK(Opal::Sin(0.0) == Catch::Approx(0.0));
        CHECK(Opal::Sin(Opal::k_pi_over_2_double) == Catch::Approx(1.0));
        CHECK(Opal::Sin(Opal::k_pi_double + Opal::k_pi_over_2_double) == Catch::Approx(-1.0));
        CHECK(Opal::IsEqual(0.0, Opal::Sin(Opal::k_pi_double), k_epsilon));
        CHECK(Opal::IsEqual(0.0, Opal::Sin(Opal::k_pi_double * 2), k_epsilon));

        CHECK(Opal::Cos(0.0) == Catch::Approx(1.0));
        CHECK(Opal::Cos(Opal::k_pi_double * 2) == Catch::Approx(1.0));
        CHECK(Opal::Cos(Opal::k_pi_double) == Catch::Approx(-1.0));
        CHECK(Opal::IsEqual(0.0, Opal::Cos(Opal::k_pi_double + Opal::k_pi_over_2_double), k_epsilon));
        CHECK(Opal::IsEqual(0.0, Opal::Cos(Opal::k_pi_over_2_double), k_epsilon));

        CHECK(Opal::Tan(0.0) == Catch::Approx(0.0));
        CHECK(Opal::Tan(Opal::k_pi_over_4_double) == Catch::Approx(1.0));
        CHECK(Opal::IsEqual(1.0, Opal::Tan(Opal::k_pi_double + Opal::k_pi_over_4_double), k_epsilon));
        CHECK(Opal::IsEqual(0.0, Opal::Tan(Opal::k_pi_double * 2), k_epsilon));
    }
}

TEST_CASE("LogNatural", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::LogNatural(128.0f) == Catch::Approx(4.852030263919617f));
        CHECK(Opal::LogNatural(1024.0f) == Catch::Approx(6.931471805599453f));
        CHECK(Opal::LogNatural(1.0f) == Catch::Approx(0.0f));
        CHECK(Opal::LogNatural(0.0f) == Catch::Approx(-Opal::k_inf_float));
        CHECK(Opal::IsNaN(Opal::LogNatural(-1.0f)));
        CHECK(Opal::LogNatural(Opal::k_inf_float) == Catch::Approx(Opal::k_inf_float));
        CHECK(Opal::IsNaN(Opal::LogNatural(Opal::k_neg_inf_float)));
        CHECK(Opal::IsNaN(Opal::LogNatural(NAN)));
    }
    SECTION("double")
    {
        CHECK(Opal::LogNatural(128.0) == Catch::Approx(4.852030263919617));
        CHECK(Opal::LogNatural(1024.0) == Catch::Approx(6.931471805599453));
        CHECK(Opal::LogNatural(1.0) == Catch::Approx(0.0));
        CHECK(Opal::LogNatural(0.0) == Catch::Approx(-Opal::k_inf_double));
        CHECK(Opal::IsNaN(Opal::LogNatural(-1.0)));
        CHECK(Opal::LogNatural(Opal::k_inf_double) == Catch::Approx(Opal::k_inf_double));
        CHECK(Opal::IsNaN(Opal::LogNatural(Opal::k_neg_inf_double)));
        CHECK(Opal::IsNaN(Opal::LogNatural(NAN)));
    }
}

TEST_CASE("Log2", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Log2(128.0f) == Catch::Approx(7.0f));
        CHECK(Opal::Log2(1024.0f) == Catch::Approx(10.0f));
        CHECK(Opal::Log2(1.0f) == Catch::Approx(0.0f));
        CHECK(Opal::Log2(0.0f) == Catch::Approx(-Opal::k_inf_float));
        CHECK(Opal::IsNaN(Opal::Log2(-1.0f)));
        CHECK(Opal::Log2(Opal::k_inf_float) == Catch::Approx(Opal::k_inf_float));
        CHECK(Opal::IsNaN(Opal::Log2(Opal::k_neg_inf_float)));
        CHECK(Opal::IsNaN(Opal::Log2(NAN)));
    }
    SECTION("double")
    {
        CHECK(Opal::Log2(128.0) == Catch::Approx(7.0));
        CHECK(Opal::Log2(1024.0) == Catch::Approx(10.0));
        CHECK(Opal::Log2(1.0) == Catch::Approx(0.0));
        CHECK(Opal::Log2(0.0) == Catch::Approx(-Opal::k_inf_double));
        CHECK(Opal::IsNaN(Opal::Log2(-1.0)));
        CHECK(Opal::Log2(Opal::k_inf_double) == Catch::Approx(Opal::k_inf_double));
        CHECK(Opal::IsNaN(Opal::Log2(Opal::k_neg_inf_double)));
        CHECK(Opal::IsNaN(Opal::Log2(NAN)));
    }
}

TEST_CASE("Power", "[BaseTests]")
{
    SECTION("float")
    {
        CHECK(Opal::Power(2.0f, 2.0f) == Catch::Approx(4));
        CHECK(Opal::Power(2.0f, 3.0f) == Catch::Approx(8));
    }
    SECTION("double")
    {
        CHECK(Opal::Power(2.0, 2.0) == Catch::Approx(4));
        CHECK(Opal::Power(2.0, 3.0) == Catch::Approx(8));
    }
}

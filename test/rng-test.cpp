#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon - virtual - dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/rng.h"

TEST_CASE("UniformLimit", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    for (int i = 0; i < k_case_count; i++)
    {
        const uint32_t num = gen.UniformUInt32(100);
        CHECK(num < 100);
    }
}

TEST_CASE("UniformFloat", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    for (int i = 0; i < k_case_count; i++)
    {
        const float num = gen.UniformFloat();
        CHECK(num < 1);
        CHECK(num >= 0);
    }
}

TEST_CASE("UniformFloatInRange", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    const float low = 10;
    const float high = 50;

    for (int i = 0; i < k_case_count; i++)
    {
        const float num = gen.UniformFloatInRange(low, high);
        CHECK(num < high);
        CHECK(num >= low);
    }
}

TEST_CASE("SameStartIndexSameSequence", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen1(5);
    Opal::RNG gen2(5);

    for (int i = 0; i < k_case_count; i++)
    {
        const float num1 = gen1.UniformFloat();
        const float num2 = gen2.UniformFloat();
        CHECK(num1 == Catch::Approx(num2));
    }
}

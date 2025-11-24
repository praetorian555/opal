#include "test-helpers.h"

#include "opal/rng.h"

TEST_CASE("UniformRange", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    for (int i = 0; i < k_case_count; i++)
    {
        const uint32_t num = gen.RandomU32(100, 200);
        CHECK(num >= 100);
        CHECK(num < 200);
    }
}

TEST_CASE("UniformRangeInt", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    for (int i = 0; i < k_case_count; i++)
    {
        const Opal::i32 num = gen.RandomI32(-100, 200);
        CHECK(num >= -100);
        CHECK(num < 200);
    }
}

TEST_CASE("UniformFloat", "[rng]")
{
    constexpr int k_case_count = 1'000'000;
    Opal::RNG gen(5);
    for (int i = 0; i < k_case_count; i++)
    {
        const float num = gen.RandomF32();
        CHECK(num < 1.0f);
        CHECK(num >= 0.0f);
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
        const float num = gen.RandomF32(low, high);
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
        const float num1 = gen1.RandomF32();
        const float num2 = gen2.RandomF32();
        CHECK(num1 == Catch::Approx(num2));
    }
}

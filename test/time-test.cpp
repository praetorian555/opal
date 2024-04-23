#include "catch2/catch2.hpp"

#include "opal/time.h"

using namespace Opal;

TEST_CASE("Get time stamp", "[Time]")
{
    const f64 seconds = Opal::GetSeconds();
    REQUIRE(seconds >= 0);
    const f64 milliseconds = Opal::GetMilliSeconds();
    REQUIRE(milliseconds >= 0);
    const f64 microseconds = Opal::GetMicroSeconds();
    REQUIRE(microseconds >= 0);
}

TEST_CASE("Measuring short time", "[Time]")
{
    const f64 start = Opal::GetMicroSeconds();
    for (int i = 0; i < 100'000; i++)
    {
        int j = i * i;
        (void)j;
    }
    const f64 end = Opal::GetMicroSeconds();
    REQUIRE(end > start);
    const f64 duration = end - start;
    REQUIRE(duration > 0);
}

TEST_CASE("Measuring long time", "[Time]")
{
    const f64 start = Opal::GetMilliSeconds();
    for (int i = 0; i < 1'000'000'000; i++)
    {
        int j = i * i;
        (void)j;
    }
    const f64 end = Opal::GetMilliSeconds();
    REQUIRE(end > start);
    const f64 duration = end - start;
    REQUIRE(duration > 0);
}

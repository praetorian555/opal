#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/defines.h"
#include "opal/time.h"

using namespace Opal;

OPAL_DISABLE_OPTIMIZATION

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
    for (i64 i = 0; i < 100'000; i++)
    {
        i64 j = i * i;
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
    for (i64 i = 0; i < 1'000'000'000; i++)
    {
        i64 j = i + i;
        (void)j;
    }
    const f64 end = Opal::GetMilliSeconds();
    REQUIRE(end > start);
    const f64 duration = end - start;
    REQUIRE(duration > 0);
}

OPAL_ENABLE_OPTIMIZATION

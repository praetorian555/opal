#include "test-helpers.h"

#include "opal/common.h"
#include "opal/container/dynamic-array.h"

using namespace Opal;

TEST_CASE("Swap", "[common]")
{
    SECTION("Swap base types")
    {
        u32 a = 1;
        u32 b = 2;
        Opal::Swap(a, b);
        REQUIRE(a == 2);
        REQUIRE(b == 1);
    }
    SECTION("Swap arrays")
    {
        u32 a[2] = {1, 2};
        u32 b[2] = {3, 4};
        Opal::Swap(a, b);
        REQUIRE(a[0] == 3);
        REQUIRE(a[1] == 4);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
    }
    SECTION("Swap pointers")
    {
        u32 a = 1;
        u32 b = 2;
        u32* pa = &a;
        u32* pb = &b;
        Opal::Swap(pa, pb);
        REQUIRE(*pa == 2);
        REQUIRE(*pb == 1);
        REQUIRE(pa == &b);
        REQUIRE(pb == &a);
    }
    SECTION("Swap dynamic array")
    {
        DynamicArray<u32> a = {1, 2};
        DynamicArray<u32> b = {3, 4};
        Opal::Swap(a, b);
        REQUIRE(a[0] == 3);
        REQUIRE(a[1] == 4);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
    }
}

OPAL_END_DISABLE_WARNINGS

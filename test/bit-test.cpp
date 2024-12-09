#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/bit.h"

using namespace Opal;

TEST_CASE("Count leading zeros", "[bit]")
{
    const u32 count1 = Opal::CountLeadingZeros(0x00800000u);
    REQUIRE(count1 == 8);

    const u64 value2 = 0x0000008000000000;
    const u64 count2 = Opal::CountLeadingZeros(value2);
    REQUIRE(count2 == 24);
}

TEST_CASE("Count trailing zeros", "[bit]")
{
    const u32 count1 = CountTrailingZeros(0x00008000u);
    REQUIRE(count1 == 15);

    const u64 value2 = 0x0000008000000000;
    const u64 count2 = CountTrailingZeros(value2);
    REQUIRE(count2 == 39);
}

TEST_CASE("BitMask", "[bit-mask]")
{
    SECTION("32 bit")
    {
        const BitMask<u32> bit_mask(5);
        for (const u32 index : bit_mask)
        {
            REQUIRE((index == 0 || index == 2));
        }
        REQUIRE(0 == bit_mask.GetLowestSetBitIndex());
        REQUIRE(2 == bit_mask.GetHighestSetBitIndex());
    }
    SECTION("64 bit")
    {
        const BitMask<u64> bit_mask(5);
        for (const u32 index : bit_mask)
        {
            REQUIRE((index == 0 || index == 2));
        }
        REQUIRE(0 == bit_mask.GetLowestSetBitIndex());
        REQUIRE(2 == bit_mask.GetHighestSetBitIndex());
    }
    SECTION("Compare masks")
    {
        {
            BitMask<u32> mask1(5);
            BitMask<u32> mask2(5);
            BitMask<u32> mask3(8);
            REQUIRE(mask1 == mask2);
            REQUIRE(mask1 != mask3);
        }
        {
            BitMask<u64> mask1(5);
            BitMask<u64> mask2(5);
            BitMask<u64> mask3(8);
            REQUIRE(mask1 == mask2);
            REQUIRE(mask1 != mask3);
        }
    }
}
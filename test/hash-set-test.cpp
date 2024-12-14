#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/hash.h"
#include "opal/container/string.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-set.h"

using namespace Opal;

TEST_CASE("Hash", "[hash]")
{
    u8 data[] = {1, 2, 3};
    u64 hash = CalculateHashFromPointerArray(data, 3);
    REQUIRE(hash != 0);

    StringUtf8 str = "Hello there";
    hash = CalculateHashFromContainer(str);
    REQUIRE(hash != 0);

    DynamicArray<i32> arr = {1, 2 , 3};
    hash = CalculateHashFromContainer(arr);
    REQUIRE(hash != 0);

    struct Pod
    {
        int a = 5;
        double c = 10.0;
    };

    Pod pod;
    hash = CalculateHashFromObject(pod);
    REQUIRE(hash != 0);
}

TEST_CASE("Hash Set", "[hash-set]")
{
    HashSet<i32> set(100);
    REQUIRE(set.GetSize() == 0);
    REQUIRE(set.GetCapacity() == 127);
    for (int i = 0; i < 100; i++)
    {
        set.Insert(i);
    }
    for (int i = 0; i < 100; i++)
    {
        REQUIRE(i == *set.Find(i));
    }
    REQUIRE(set.GetSize() == 100);
    REQUIRE(set.GetCapacity() == 127);

    i32 sum = 0;
    for (i32 i : set)
    {
        sum += i;
    }
    REQUIRE(sum == 4950);

    set.Reserve(1000);
    sum = 0;
    for (i32 i : set)
    {
        sum += i;
    }
    REQUIRE(sum == 4950);
}

TEST_CASE("Hash set automatic growth", "[hash-set]")
{
    HashSet<i32> set(120);
    for (int i = 0; i < 120; i++)
    {
        set.Insert(i);
    }
    REQUIRE(set.GetCapacity() == 255);
    REQUIRE(set.GetSize() == 120);
    for (int i = 0; i < 120; i++)
    {
        REQUIRE(i == *set.Find(i));
    }
}

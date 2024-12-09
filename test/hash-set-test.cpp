#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/hash.h"
#include "opal/container/string.h"
#include "opal/container/dynamic-array.h"

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

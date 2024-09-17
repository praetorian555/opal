#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/container/dynamic-array.h"
#include "opal/sort/key-indexed-counting.h"

TEST_CASE("KeyIndexedCounting", "[sort]")
{
    using namespace Opal;

    SECTION("Non-empty array")
    {
        DynamicArray<u64> keys = {2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};
        DynamicArray<u64> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(values == DynamicArray<u64>({2, 5, 8, 0, 3, 6, 9, 1, 4, 7, 10}));
    }
    SECTION("Empty array")
    {
        DynamicArray<u64> keys;
        DynamicArray<u64> values;
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(values == DynamicArray<u64>());
    }
    SECTION("Bad input")
    {
        DynamicArray<u64> keys = {2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};
        DynamicArray<u64> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::BadInput);
    }
}

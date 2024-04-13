#include "catch2/catch2.hpp"

#include "opal/container/dynamic-array.h"

TEST_CASE("Construction of DynamicArray", "[DynamicArray]")
{
    SECTION("POD type, initial count smaller then default capacity")
    {
        Opal::DynamicArray<int32_t> int_arr(3);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
    }
    SECTION("Pointer type, initial count smaller then default capacity")
    {
        Opal::DynamicArray<int32_t*> ptr_arr(3);
        REQUIRE(ptr_arr.GetCapacity() == 4);
        REQUIRE(ptr_arr.GetSize() == 3);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == nullptr);
        REQUIRE(ptr_arr.GetData()[1] == nullptr);
        REQUIRE(ptr_arr.GetData()[2] == nullptr);
    }
    SECTION("POD type, initial count larger then default capacity")
    {
        Opal::DynamicArray<int32_t> int_arr(5);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
        REQUIRE(int_arr.GetData()[3] == 0);
        REQUIRE(int_arr.GetData()[4] == 0);
    }
    SECTION("Pointer type, initial count larger then default capacity")
    {
        Opal::DynamicArray<int32_t*> ptr_arr(5);
        REQUIRE(ptr_arr.GetCapacity() == 5);
        REQUIRE(ptr_arr.GetSize() == 5);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == nullptr);
        REQUIRE(ptr_arr.GetData()[1] == nullptr);
        REQUIRE(ptr_arr.GetData()[2] == nullptr);
        REQUIRE(ptr_arr.GetData()[3] == nullptr);
        REQUIRE(ptr_arr.GetData()[4] == nullptr);
    }
}

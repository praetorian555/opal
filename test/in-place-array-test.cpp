#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include <catch2/catch2.hpp>

#include "opal/container/in-place-array.h"

#include <array>

using namespace Opal;

TEST_CASE("Construction", "[InPlaceArray][Constructor]")
{
    SECTION("default construction")
    {
        InPlaceArray<int32_t, 3> array;
        REQUIRE(array.GetSize() == 3);
    }
    SECTION("initializer list construction")
    {
        InPlaceArray<int32_t, 5> array({1, 2, 3});
        REQUIRE(array.GetSize() == 5);
        REQUIRE(array[0] == 1);
        REQUIRE(array[1] == 2);
        REQUIRE(array[2] == 3);
        REQUIRE(array[3] == 0);
        REQUIRE(array[4] == 0);

        InPlaceArray<int32_t, 5> array2{1, 2, 3, 4, 5};
        REQUIRE(array2.GetSize() == 5);
        REQUIRE(array2[0] == 1);
        REQUIRE(array2[1] == 2);
        REQUIRE(array2[2] == 3);
        REQUIRE(array2[3] == 4);
        REQUIRE(array2[4] == 5);

        InPlaceArray<int32_t, 5> array3{};
        REQUIRE(array3.GetSize() == 5);
        REQUIRE(array3[0] == 0);
        REQUIRE(array3[1] == 0);
        REQUIRE(array3[2] == 0);
        REQUIRE(array3[3] == 0);
        REQUIRE(array3[4] == 0);
    }
}

TEST_CASE("Get data", "[InPlaceArray][Getter]")
{
    InPlaceArray<int32_t, 3> array({1, 2, 3});
    REQUIRE(array.GetData() == &array[0]);

    const InPlaceArray<int32_t, 3> const_array({1, 2, 3});
    REQUIRE(const_array.GetData() == &const_array[0]);
}

TEST_CASE("Swap", "[InPlaceArray][Swap]")
{
    InPlaceArray<int32_t, 2> a = {1, 2};
    InPlaceArray<int32_t, 2> b = {3, 4};
    Opal::Swap(a, b);
    REQUIRE(a[0] == 3);
    REQUIRE(a[1] == 4);
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
}

TEST_CASE("Comparison", "[InPlaceArray][Comparison]")
{
    InPlaceArray<int32_t, 3> a = {1, 2, 3};
    InPlaceArray<int32_t, 3> b = {1, 2, 3};
    InPlaceArray<int32_t, 3> c = {1, 2, 4};

    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("At", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.At(0).GetValue() == 1);
    REQUIRE(array.At(1).GetValue() == 2);
    REQUIRE(array.At(2).GetValue() == 3);
    REQUIRE(array.At(3).GetError() == ErrorCode::OutOfBounds);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.At(0).GetValue() == 1);
    REQUIRE(const_array.At(1).GetValue() == 2);
    REQUIRE(const_array.At(2).GetValue() == 3);
    REQUIRE(const_array.At(3).GetError() == ErrorCode::OutOfBounds);
}

TEST_CASE("Front", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.Front() == 1);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.Front() == 1);
}

TEST_CASE("Back", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.Back() == 3);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.Back() == 3);
}

TEST_CASE("Fill", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    array.Fill(42);
    REQUIRE(array[0] == 42);
    REQUIRE(array[1] == 42);
    REQUIRE(array[2] == 42);
}

OPAL_END_DISABLE_WARNINGS

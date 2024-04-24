#include "catch2/catch2.hpp"

#include <vector>

#include "opal/container/span.h"
#include "opal/container/array.h"

using namespace Opal;

TEST_CASE("Construction", "[Span]")
{
    SECTION("Default constructor")
    {
        Span<int> span;
        REQUIRE(span.GetData() == nullptr);
        REQUIRE(span.GetSize() == 0);
    }
    SECTION("From C array")
    {
        SECTION("as C array")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            Span<int> span(array);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and count")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            Span<int> span(array, 5);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and iterator")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            Span<int> span(array, array + 5);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
    }
    SECTION("From Array")
    {
        SECTION("iterator and count")
        {
            Array<i32> array(5);
            Span<int> span(array.begin(), 5);
            REQUIRE(span.GetData() == array.GetData());
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and iterator")
        {
            Array<i32> array(5);
            Span<int> span(array.begin(), array.end());
            REQUIRE(span.GetData() == array.GetData());
            REQUIRE(span.GetSize() == 5);
        }
    }
}

TEST_CASE("As bytes", "[Span]")
{
    i32 array[] = {1, 2, 3, 4, 5};
    Span<i32> span(array);
    Span<const u8> bytes = AsBytes(span);
    REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
    REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array));
}

TEST_CASE("As writable bytes", "[Span]")
{
    i32 array[] = {1, 2, 3, 4, 5};
    Span<i32> span(array);
    Span<u8> bytes = AsWritableBytes(span);
    REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
    REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array));
}

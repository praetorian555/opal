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

TEST_CASE("Equality", "[Span]")
{
    i32 array[] = {1, 2, 3, 4, 5};
    Span<i32> span1(array);
    Span<i32> span2(array);
    Span<i32> span3(array + 2, 3);
    REQUIRE(span1 == span2);
    REQUIRE(span1 != span3);
}

TEST_CASE("At access", "[Span]")
{
    SECTION("Const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const Span<i32> span(array);
        auto value = span.At(2);
        REQUIRE(value.HasValue());
        REQUIRE(value.GetValue() == 3);
    }
    SECTION("Const bad access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const Span<i32> span(array);
        auto value = span.At(5);
        REQUIRE(!value.HasValue());
        REQUIRE(value.GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Non-const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        auto value = span.At(2);
        REQUIRE(value.HasValue());
        REQUIRE(value.GetValue() == 3);
    }
    SECTION("Non-const bad access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        auto value = span.At(5);
        REQUIRE(!value.HasValue());
        REQUIRE(value.GetError() == ErrorCode::OutOfBounds);
    }
}

TEST_CASE("Operator [] access", "[Span]")
{
    SECTION("Const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const Span<i32> span(array);
        REQUIRE(span[2] == 3);
    }
    SECTION("Non-const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        REQUIRE(span[2] == 3);
    }
}

TEST_CASE("Front access", "[Span]")
{
    SECTION("Const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const Span<int> span(array);
        auto front = span.Front();
        REQUIRE(front.HasValue());
        REQUIRE(front.GetValue() == 1);
    }
    SECTION("Non-const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<int> span(array);
        auto front = span.Front();
        REQUIRE(front.HasValue());
        REQUIRE(front.GetValue() == 1);
    }
}

TEST_CASE("Back access", "[Span]")
{
    SECTION("Const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const Span<int> span(array);
        auto back = span.Back();
        REQUIRE(back.HasValue());
        REQUIRE(back.GetValue() == 5);
    }
    SECTION("Non-const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<int> span(array);
        auto back = span.Back();
        REQUIRE(back.HasValue());
        REQUIRE(back.GetValue() == 5);
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

TEST_CASE("Sub span", "[Span]")
{
    SECTION("Out of bounds")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        auto subSpan = span.SubSpan(2, 4);
        REQUIRE(!subSpan.HasValue());
        REQUIRE(subSpan.GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Good")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        auto subSpan = span.SubSpan(2, 3);
        REQUIRE(subSpan.HasValue());
        REQUIRE(subSpan.GetValue().GetData() == array + 2);
        REQUIRE(subSpan.GetValue().GetSize() == 3);
    }
}

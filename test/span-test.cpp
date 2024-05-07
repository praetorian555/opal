#include "catch2/catch2.hpp"

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
    SECTION("From Span")
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
    SECTION("From Array as range")
    {
        Array<i32> array(5);
        Span<int> span(array);
        REQUIRE(span.GetData() == array.GetData());
        REQUIRE(span.GetSize() == 5);
    }
    SECTION("From C-style array as range")
    {
        i32 arr[] = {42, 43, 44};
        Span<i32> span(arr);
        REQUIRE(span.GetData() == arr);
        REQUIRE(span.GetSize() == 3);
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

TEST_CASE("Iterator", "[Span]")
{
    SECTION("Difference")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        Span<i32> span(array);
        Span<i32>::IteratorType it1 = span.Begin();
        Span<i32>::IteratorType it2 = span.End();
        REQUIRE(it2 - it1 == 5);
    }
    SECTION("Increment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);
        ++it;
        REQUIRE(it == span.End());
    }
    SECTION("Post increment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(*it == 1);
        it++;
        REQUIRE(*it == 2);
        it++;
        REQUIRE(*it == 3);
        Span<i32>::IteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == span.End());
    }
    SECTION("Decrement")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.End();
        --it;
        REQUIRE(*it == 3);
        --it;
        REQUIRE(*it == 2);
        --it;
        REQUIRE(*it == 1);
        REQUIRE(it == span.Begin());
    }
    SECTION("Post decrement")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.End();
        it--;
        REQUIRE(*it == 3);
        it--;
        REQUIRE(*it == 2);
        Span<i32>::IteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == span.Begin());
    }
    SECTION("Add")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(*(it + 0) == 1);
        REQUIRE(*(it + 1) == 2);
        REQUIRE(*(it + 2) == 3);
        REQUIRE((it + 3) == span.End());

        Span<i32>::IteratorType it2 = span.Begin();
        REQUIRE((3 + it2) == span.End());
    }
    SECTION("Add assignment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(*(it += 0) == 1);
        REQUIRE(*(it += 1) == 2);
        REQUIRE(*(it += 1) == 3);
        REQUIRE((it += 1) == span.End());
    }
    SECTION("Subtract")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.End();
        REQUIRE((it - 0) == span.End());
        REQUIRE(*(it - 1) == 3);
        REQUIRE(*(it - 2) == 2);
        REQUIRE(*(it - 3) == 1);
        REQUIRE((it - 3) == span.Begin());
    }
    SECTION("Subtract assignment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.End();
        REQUIRE((it -= 0) == span.End());
        REQUIRE(*(it -= 1) == 3);
        REQUIRE(*(it -= 1) == 2);
        REQUIRE(*(it -= 1) == 1);
        REQUIRE(it == span.Begin());
    }
    SECTION("Access")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(it[0] == 1);
        REQUIRE(it[1] == 2);
        REQUIRE(it[2] == 3);
    }
    SECTION("Dereference")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it = span.Begin();
        REQUIRE(*it == 1);
    }
    SECTION("Compare")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::IteratorType it1 = span.Begin();
        Span<i32>::IteratorType it2 = span.Begin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        i32 sum = 0;
        for (Span<i32>::IteratorType it = span.Begin(); it != span.End(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 6);
    }
    SECTION("Modern for loop")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        i32 sum = 0;
        for (i32 val : span)
        {
            sum += val;
        }
        REQUIRE(sum == 6);
    }
}

TEST_CASE("Const iterator", "[Span]")
{
    SECTION("Difference")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it1 = span.ConstBegin();
        Span<i32>::ConstIteratorType it2 = span.ConstEnd();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);
        ++it;
        REQUIRE(it == span.ConstEnd());
    }
    SECTION("Post increment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(*it == 1);
        it++;
        REQUIRE(*it == 2);
        it++;
        REQUIRE(*it == 3);
        Span<i32>::ConstIteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == span.ConstEnd());
    }
    SECTION("Decrement")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstEnd();
        --it;
        REQUIRE(*it == 3);
        --it;
        REQUIRE(*it == 2);
        --it;
        REQUIRE(*it == 1);
        REQUIRE(it == span.ConstBegin());
    }
    SECTION("Post decrement")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstEnd();
        it--;
        REQUIRE(*it == 3);
        it--;
        REQUIRE(*it == 2);
        Span<i32>::ConstIteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == span.ConstBegin());
    }
    SECTION("Add")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(*(it + 0) == 1);
        REQUIRE(*(it + 1) == 2);
        REQUIRE(*(it + 2) == 3);
        REQUIRE((it + 3) == span.ConstEnd());

        Span<i32>::ConstIteratorType it2 = span.ConstBegin();
        REQUIRE((3 + it2) == span.ConstEnd());
    }
    SECTION("Add assignment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(*(it += 0) == 1);
        REQUIRE(*(it += 1) == 2);
        REQUIRE(*(it += 1) == 3);
        REQUIRE((it += 1) == span.ConstEnd());
    }
    SECTION("Subtract")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstEnd();
        REQUIRE((it - 0) == span.ConstEnd());
        REQUIRE(*(it - 1) == 3);
        REQUIRE(*(it - 2) == 2);
        REQUIRE(*(it - 3) == 1);
        REQUIRE((it - 3) == span.ConstBegin());
    }
    SECTION("Subtract assignment")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstEnd();
        REQUIRE((it -= 0) == span.ConstEnd());
        REQUIRE(*(it -= 1) == 3);
        REQUIRE(*(it -= 1) == 2);
        REQUIRE(*(it -= 1) == 1);
        REQUIRE(it == span.ConstBegin());
    }
    SECTION("Access")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(it[0] == 1);
        REQUIRE(it[1] == 2);
        REQUIRE(it[2] == 3);
    }
    SECTION("Dereference")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it = span.ConstBegin();
        REQUIRE(*it == 1);
    }
    SECTION("Compare")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        Span<i32>::ConstIteratorType it1 = span.ConstBegin();
        Span<i32>::ConstIteratorType it2 = span.ConstBegin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        i32 sum = 0;
        for (Span<i32>::ConstIteratorType it = span.ConstBegin(); it != span.ConstEnd(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 6);
    }
    SECTION("Modern for loop")
    {
        i32 array[] = {1, 2, 3};
        Span<i32> span(array);
        i32 sum = 0;
        for (const i32& val : span)
        {
            sum += val;
        }
        REQUIRE(sum == 6);
    }
}

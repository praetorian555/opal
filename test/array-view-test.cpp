#include "test-helpers.h"

#include <array>

#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"

using namespace Opal;

TEST_CASE("Construction", "[Span]")
{
    SECTION("Default constructor")
    {
        ArrayView<int> span;
        REQUIRE(span.GetData() == nullptr);
        REQUIRE(span.GetSize() == 0);
    }
    SECTION("From C array")
    {
        SECTION("as C array")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            ArrayView<int> span(array);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and count")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            ArrayView<int> span(array, 5);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and iterator")
        {
            i32 array[] = {1, 2, 3, 4, 5};
            ArrayView<int> span(array, array + 5);
            REQUIRE(span.GetData() == array);
            REQUIRE(span.GetSize() == 5);
        }
    }
    SECTION("From Span")
    {
        SECTION("iterator and count")
        {
            DynamicArray<i32> array(5);
            ArrayView<int> span(array.begin(), 5);
            REQUIRE(span.GetData() == array.GetData());
            REQUIRE(span.GetSize() == 5);
        }
        SECTION("iterator and iterator")
        {
            DynamicArray<i32> array(5);
            ArrayView<int> span(array.begin(), array.end());
            REQUIRE(span.GetData() == array.GetData());
            REQUIRE(span.GetSize() == 5);
        }
    }
    SECTION("From Array as range")
    {
        DynamicArray<i32> array(5);
        ArrayView<int> span(array);
        REQUIRE(span.GetData() == array.GetData());
        REQUIRE(span.GetSize() == 5);
    }
    SECTION("From C-style array as range")
    {
        i32 arr[] = {42, 43, 44};
        ArrayView<i32> span(arr);
        REQUIRE(span.GetData() == arr);
        REQUIRE(span.GetSize() == 3);
    }
}

TEST_CASE("Equality", "[Span]")
{
    i32 array[] = {1, 2, 3, 4, 5};
    ArrayView<i32> span1(array);
    ArrayView<i32> span2(array);
    ArrayView<i32> span3(array + 2, 3);
    REQUIRE(span1 == span2);
    REQUIRE(span1 != span3);
}

TEST_CASE("At access", "[Span]")
{
    SECTION("Const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const ArrayView<i32> span(array);
        auto value = span.At(2);
        REQUIRE(value.HasValue());
        REQUIRE(value.GetValue() == 3);
    }
    SECTION("Const bad access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const ArrayView<i32> span(array);
        auto value = span.At(5);
        REQUIRE(!value.HasValue());
        REQUIRE(value.GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Non-const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
        auto value = span.At(2);
        REQUIRE(value.HasValue());
        REQUIRE(value.GetValue() == 3);
    }
    SECTION("Non-const bad access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
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
        const ArrayView<i32> span(array);
        REQUIRE(span[2] == 3);
    }
    SECTION("Non-const good access")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
        REQUIRE(span[2] == 3);
    }
}

TEST_CASE("Front access", "[Span]")
{
    SECTION("Const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        const ArrayView<int> span(array);
        auto front = span.Front();
        REQUIRE(front.HasValue());
        REQUIRE(front.GetValue() == 1);
    }
    SECTION("Non-const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<int> span(array);
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
        const ArrayView<int> span(array);
        auto back = span.Back();
        REQUIRE(back.HasValue());
        REQUIRE(back.GetValue() == 5);
    }
    SECTION("Non-const")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<int> span(array);
        auto back = span.Back();
        REQUIRE(back.HasValue());
        REQUIRE(back.GetValue() == 5);
    }
}

TEST_CASE("As bytes", "[Span]")
{
    SECTION("From object")
    {
        i32 value = 42;
        ArrayView<const u8> bytes = AsBytes(value);
        REQUIRE(bytes.GetSize() == sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<const u8*>(&value));
    }
    SECTION("From C-style array")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<const u8> bytes = AsBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<const u8*>(array));
    }
    SECTION("From Array")
    {
        DynamicArray<i32> array(5);
        ArrayView<const u8> bytes = AsBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<const u8*>(array.GetData()));
    }
    SECTION("From Span")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
        ArrayView<const u8> bytes = AsBytes(span);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<const u8*>(array));
    }
    SECTION("From STD container")
    {
        std::vector<i32> array = {1, 2, 3, 4, 5};
        ArrayView<const u8> bytes = AsBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<const u8*>(array.data()));
    }
    SECTION("From empty container")
    {
        DynamicArray<i32> arr;
        ArrayView<const u8> bytes = AsBytes(arr);
        REQUIRE(bytes.GetSize() == 0);
        REQUIRE(bytes.GetData() == nullptr);
    }
}

TEST_CASE("As writable bytes", "[Span]")
{
    SECTION("From object")
    {
        i32 value = 42;
        ArrayView<u8> bytes = AsWritableBytes(value);
        REQUIRE(bytes.GetSize() == sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(&value));
    }
    SECTION("From C-style array")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<u8> bytes = AsWritableBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array));
    }
    SECTION("From Array")
    {
        DynamicArray<i32> array(5);
        ArrayView<u8> bytes = AsWritableBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array.GetData()));
    }
    SECTION("From Span")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
        ArrayView<u8> bytes = AsWritableBytes(span);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array));
    }
    SECTION("From STD container")
    {
        std::vector<i32> array = {1, 2, 3, 4, 5};
        ArrayView<u8> bytes = AsWritableBytes(array);
        REQUIRE(bytes.GetSize() == 5 * sizeof(i32));
        REQUIRE(bytes.GetData() == reinterpret_cast<u8*>(array.data()));
    }
    SECTION("From empty container")
    {
        DynamicArray<i32> arr;
        ArrayView<u8> bytes = AsWritableBytes(arr);
        REQUIRE(bytes.GetSize() == 0);
        REQUIRE(bytes.GetData() == nullptr);
    }
}

TEST_CASE("Sub span", "[Span]")
{
    SECTION("Out of bounds")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
        auto subSpan = span.SubSpan(2, 4);
        REQUIRE(!subSpan.HasValue());
        REQUIRE(subSpan.GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Good")
    {
        i32 array[] = {1, 2, 3, 4, 5};
        ArrayView<i32> span(array);
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
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it1 = span.begin();
        ArrayView<i32>::iterator it2 = span.end();
        REQUIRE(it2 - it1 == 5);
    }
    SECTION("Increment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);
        ++it;
        REQUIRE(it == span.end());
    }
    SECTION("Post increment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(*it == 1);
        it++;
        REQUIRE(*it == 2);
        it++;
        REQUIRE(*it == 3);
        ArrayView<i32>::iterator prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == span.end());
    }
    SECTION("Decrement")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.end();
        --it;
        REQUIRE(*it == 3);
        --it;
        REQUIRE(*it == 2);
        --it;
        REQUIRE(*it == 1);
        REQUIRE(it == span.begin());
    }
    SECTION("Post decrement")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.end();
        it--;
        REQUIRE(*it == 3);
        it--;
        REQUIRE(*it == 2);
        ArrayView<i32>::iterator prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == span.begin());
    }
    SECTION("Add")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(*(it + 0) == 1);
        REQUIRE(*(it + 1) == 2);
        REQUIRE(*(it + 2) == 3);
        REQUIRE((it + 3) == span.end());

        ArrayView<i32>::iterator it2 = span.begin();
        REQUIRE((3 + it2) == span.end());
    }
    SECTION("Add assignment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(*(it += 0) == 1);
        REQUIRE(*(it += 1) == 2);
        REQUIRE(*(it += 1) == 3);
        REQUIRE((it += 1) == span.end());
    }
    SECTION("Subtract")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.end();
        REQUIRE((it - 0) == span.end());
        REQUIRE(*(it - 1) == 3);
        REQUIRE(*(it - 2) == 2);
        REQUIRE(*(it - 3) == 1);
        REQUIRE((it - 3) == span.begin());
    }
    SECTION("Subtract assignment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.end();
        REQUIRE((it -= 0) == span.end());
        REQUIRE(*(it -= 1) == 3);
        REQUIRE(*(it -= 1) == 2);
        REQUIRE(*(it -= 1) == 1);
        REQUIRE(it == span.begin());
    }
    SECTION("Access")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(it[0] == 1);
        REQUIRE(it[1] == 2);
        REQUIRE(it[2] == 3);
    }
    SECTION("Dereference")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it = span.begin();
        REQUIRE(*it == 1);
    }
    SECTION("Compare")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::iterator it1 = span.begin();
        ArrayView<i32>::iterator it2 = span.begin();
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
        ArrayView<i32> span(array);
        i32 sum = 0;
        for (ArrayView<i32>::iterator it = span.begin(); it != span.end(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 6);
    }
    SECTION("Modern for loop")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
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
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it1 = span.cbegin();
        ArrayView<i32>::const_iterator it2 = span.cend();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);
        ++it;
        REQUIRE(it == span.cend());
    }
    SECTION("Post increment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(*it == 1);
        it++;
        REQUIRE(*it == 2);
        it++;
        REQUIRE(*it == 3);
        ArrayView<i32>::const_iterator prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == span.cend());
    }
    SECTION("Decrement")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cend();
        --it;
        REQUIRE(*it == 3);
        --it;
        REQUIRE(*it == 2);
        --it;
        REQUIRE(*it == 1);
        REQUIRE(it == span.cbegin());
    }
    SECTION("Post decrement")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cend();
        it--;
        REQUIRE(*it == 3);
        it--;
        REQUIRE(*it == 2);
        ArrayView<i32>::const_iterator prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == span.cbegin());
    }
    SECTION("Add")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(*(it + 0) == 1);
        REQUIRE(*(it + 1) == 2);
        REQUIRE(*(it + 2) == 3);
        REQUIRE((it + 3) == span.cend());

        ArrayView<i32>::const_iterator it2 = span.cbegin();
        REQUIRE((3 + it2) == span.cend());
    }
    SECTION("Add assignment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(*(it += 0) == 1);
        REQUIRE(*(it += 1) == 2);
        REQUIRE(*(it += 1) == 3);
        REQUIRE((it += 1) == span.cend());
    }
    SECTION("Subtract")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cend();
        REQUIRE((it - 0) == span.cend());
        REQUIRE(*(it - 1) == 3);
        REQUIRE(*(it - 2) == 2);
        REQUIRE(*(it - 3) == 1);
        REQUIRE((it - 3) == span.cbegin());
    }
    SECTION("Subtract assignment")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cend();
        REQUIRE((it -= 0) == span.cend());
        REQUIRE(*(it -= 1) == 3);
        REQUIRE(*(it -= 1) == 2);
        REQUIRE(*(it -= 1) == 1);
        REQUIRE(it == span.cbegin());
    }
    SECTION("Access")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(it[0] == 1);
        REQUIRE(it[1] == 2);
        REQUIRE(it[2] == 3);
    }
    SECTION("Dereference")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it = span.cbegin();
        REQUIRE(*it == 1);
    }
    SECTION("Compare")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        ArrayView<i32>::const_iterator it1 = span.cbegin();
        ArrayView<i32>::const_iterator it2 = span.cbegin();
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
        ArrayView<i32> span(array);
        i32 sum = 0;
        for (ArrayView<i32>::const_iterator it = span.cbegin(); it != span.cend(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 6);
    }
    SECTION("Modern for loop")
    {
        i32 array[] = {1, 2, 3};
        ArrayView<i32> span(array);
        i32 sum = 0;
        for (const i32& val : span)
        {
            sum += val;
        }
        REQUIRE(sum == 6);
    }
}

#include "test-helpers.h"

#include "opal/container/string-view.h"

using namespace Opal;

TEST_CASE("Construction", "[StringView]")
{
    SECTION("Default constructor")
    {
        StringViewUtf8 view;
        REQUIRE(view.GetData() == nullptr);
        REQUIRE(view.GetSize() == 0);
        REQUIRE(view.IsEmpty());
    }

    SECTION("From pointer and size")
    {
        const char8* str = "Hello";
        StringViewUtf8 view(str, 5);
        REQUIRE(view.GetData() == str);
        REQUIRE(view.GetSize() == 5);
        REQUIRE_FALSE(view.IsEmpty());
    }

    SECTION("From null-terminated string")
    {
        const char8* str = "Hello";
        StringViewUtf8 view(str);
        REQUIRE(view.GetData() == str);
        REQUIRE(view.GetSize() == 5);
    }

    SECTION("From null pointer")
    {
        const char8* str = nullptr;
        StringViewUtf8 view(str);
        REQUIRE(view.GetData() == nullptr);
        REQUIRE(view.GetSize() == 0);
    }

    SECTION("From String")
    {
        StringUtf8 str("Hello");
        StringViewUtf8 view(str);
        REQUIRE(view.GetData() == str.GetData());
        REQUIRE(view.GetSize() == str.GetSize());
    }

    SECTION("From empty String")
    {
        StringUtf8 str;
        StringViewUtf8 view(str);
        REQUIRE(view.GetData() == nullptr);
        REQUIRE(view.GetSize() == 0);
    }

    SECTION("From ArrayView")
    {
        const char8 data[] = {'H', 'e', 'l', 'l', 'o'};
        ArrayView<const char8> arr(data, 5);
        StringViewUtf8 view(arr);
        REQUIRE(view.GetData() == data);
        REQUIRE(view.GetSize() == 5);
    }

    SECTION("Copy constructor")
    {
        const char8* str = "Hello";
        StringViewUtf8 view1(str, 5);
        StringViewUtf8 view2(view1);
        REQUIRE(view2.GetData() == str);
        REQUIRE(view2.GetSize() == 5);
    }

    SECTION("Copy assignment")
    {
        const char8* str = "Hello";
        StringViewUtf8 view1(str, 5);
        StringViewUtf8 view2;
        view2 = view1;
        REQUIRE(view2.GetData() == str);
        REQUIRE(view2.GetSize() == 5);
    }

    SECTION("Implicit conversion from String")
    {
        StringUtf8 str("Hello");
        auto check = [](StringViewUtf8 view) { return view.GetSize(); };
        REQUIRE(check(str) == 5);
    }
}

TEST_CASE("Access", "[StringView]")
{
    const char8* str = "Hello";
    StringViewUtf8 view(str, 5);

    SECTION("GetData")
    {
        REQUIRE(view.GetData() == str);
    }

    SECTION("operator*")
    {
        REQUIRE(*view == str);
    }

    SECTION("GetSize")
    {
        REQUIRE(view.GetSize() == 5);
    }

    SECTION("IsEmpty")
    {
        REQUIRE_FALSE(view.IsEmpty());
        StringViewUtf8 empty_view;
        REQUIRE(empty_view.IsEmpty());
    }

    SECTION("operator[] valid index")
    {
        REQUIRE(view[0] == 'H');
        REQUIRE(view[1] == 'e');
        REQUIRE(view[4] == 'o');
    }

    SECTION("operator[] out of bounds throws")
    {
        REQUIRE_THROWS_AS(view[5], OutOfBoundsException);
    }

    SECTION("At unchecked access")
    {
        REQUIRE(view.At(0) == 'H');
        REQUIRE(view.At(4) == 'o');
    }

    SECTION("Front")
    {
        auto result = view.Front();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 'H');
    }

    SECTION("Front on empty view")
    {
        StringViewUtf8 empty_view;
        auto result = empty_view.Front();
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
    }

    SECTION("Back")
    {
        auto result = view.Back();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 'o');
    }

    SECTION("Back on empty view")
    {
        StringViewUtf8 empty_view;
        auto result = empty_view.Back();
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
    }
}

TEST_CASE("View operations", "[StringView]")
{
    SECTION("RemovePrefix")
    {
        StringViewUtf8 view("Hello World");
        view.RemovePrefix(6);
        REQUIRE(view.GetSize() == 5);
        REQUIRE(view[0] == 'W');
    }

    SECTION("RemoveSuffix")
    {
        StringViewUtf8 view("Hello World");
        view.RemoveSuffix(6);
        REQUIRE(view.GetSize() == 5);
        REQUIRE(view[4] == 'o');
    }

    SECTION("SubView")
    {
        StringViewUtf8 view("Hello World");
        auto result = view.SubView(6, 5);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue().GetSize() == 5);
        REQUIRE(result.GetValue()[0] == 'W');
    }

    SECTION("SubView with k_npos count")
    {
        StringViewUtf8 view("Hello World");
        auto result = view.SubView(6);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue().GetSize() == 5);
        REQUIRE(result.GetValue()[0] == 'W');
    }

    SECTION("SubView out of bounds")
    {
        StringViewUtf8 view("Hello");
        auto result = view.SubView(10);
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
    }

    SECTION("SubView at the end")
    {
        StringViewUtf8 view("Hello");
        auto result = view.SubView(5);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue().GetSize() == 0);
    }

    SECTION("SubView count exceeds remaining")
    {
        StringViewUtf8 view("Hello");
        auto result = view.SubView(3, 100);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue().GetSize() == 2);
    }
}

TEST_CASE("Comparison", "[StringView]")
{
    SECTION("Equal views")
    {
        StringViewUtf8 view1("Hello");
        StringViewUtf8 view2("Hello");
        REQUIRE(view1 == view2);
    }

    SECTION("Same pointer")
    {
        const char8* str = "Hello";
        StringViewUtf8 view1(str, 5);
        StringViewUtf8 view2(str, 5);
        REQUIRE(view1 == view2);
    }

    SECTION("Different content")
    {
        StringViewUtf8 view1("Hello");
        StringViewUtf8 view2("World");
        REQUIRE_FALSE(view1 == view2);
    }

    SECTION("Different sizes")
    {
        StringViewUtf8 view1("Hello");
        StringViewUtf8 view2("Hell");
        REQUIRE_FALSE(view1 == view2);
    }

    SECTION("Both empty")
    {
        StringViewUtf8 view1;
        StringViewUtf8 view2;
        REQUIRE(view1 == view2);
    }
}

TEST_CASE("Iterators", "[StringView]")
{
    const char8* str = "Hello";
    StringViewUtf8 view(str, 5);

    SECTION("Begin and End")
    {
        auto it = view.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
    }

    SECTION("Range-based for loop")
    {
        u64 count = 0;
        for (auto ch : view)
        {
            REQUIRE(ch == str[count]);
            ++count;
        }
        REQUIRE(count == 5);
    }

    SECTION("ConstBegin and ConstEnd")
    {
        auto it = view.ConstBegin();
        auto end = view.ConstEnd();
        REQUIRE(end - it == 5);
    }

    SECTION("Iterator arithmetic")
    {
        auto it = view.begin();
        it += 2;
        REQUIRE(*it == 'l');
        it -= 1;
        REQUIRE(*it == 'e');
    }

    SECTION("Empty view iterators")
    {
        StringViewUtf8 empty_view;
        REQUIRE(empty_view.begin() == empty_view.end());
    }
}

TEST_CASE("Free function compatibility", "[StringView]")
{
    SECTION("Compare equal views")
    {
        StringViewUtf8 view1("Hello");
        StringViewUtf8 view2("Hello");
        auto result = Compare(view1, view2);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 0);
    }

    SECTION("Compare different views")
    {
        StringViewUtf8 view1("Hello");
        StringViewUtf8 view2("World");
        auto result = Compare(view1, view2);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() < 0);
    }

    SECTION("Find string in view")
    {
        StringViewUtf8 haystack("Hello World");
        StringViewUtf8 needle("World");
        auto pos = Find(haystack, needle);
        REQUIRE(pos == 6);
    }

    SECTION("Find string not in view")
    {
        StringViewUtf8 haystack("Hello World");
        StringViewUtf8 needle("Foo");
        auto pos = Find(haystack, needle);
        REQUIRE(pos == StringViewUtf8::k_npos);
    }

    SECTION("ReverseFind")
    {
        StringViewUtf8 haystack("Hello World");
        StringViewUtf8 needle("World");
        auto pos = ReverseFind(haystack, needle);
        REQUIRE(pos == 6);
    }

    SECTION("StartsWith")
    {
        StringViewUtf8 view("Hello World");
        StringViewUtf8 prefix("Hello");
        REQUIRE(StartsWith(view, prefix));
    }

    SECTION("StartsWith false")
    {
        StringViewUtf8 view("Hello World");
        StringViewUtf8 prefix("World");
        REQUIRE_FALSE(StartsWith(view, prefix));
    }

    SECTION("EndsWith")
    {
        StringViewUtf8 view("Hello World");
        StringViewUtf8 suffix("World");
        REQUIRE(EndsWith(view, suffix));
    }

    SECTION("EndsWith false")
    {
        StringViewUtf8 view("Hello World");
        StringViewUtf8 suffix("Hello");
        REQUIRE_FALSE(EndsWith(view, suffix));
    }
}
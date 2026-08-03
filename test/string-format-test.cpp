#include "test-helpers.h"

#include "opal/allocator.h"
#include "opal/container/string-format.h"

using namespace Opal;

TEST_CASE("Format basic string", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("hello {}", "world");
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("hello world"));
}

TEST_CASE("Format integer", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("{}", 42);
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("42"));
}

TEST_CASE("Format float", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("{:.2f}", 3.14);
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("3.14"));
}

TEST_CASE("Format multiple args", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("{} + {} = {}", 1, 2, 3);
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("1 + 2 = 3"));
}

TEST_CASE("Format no args", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("literal");
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("literal"));
}

TEST_CASE("Format empty string", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue().IsEmpty());
}

TEST_CASE("AppendFormat to existing string", "[StringFormat]")
{
    StringUtf8 str("prefix: ");
    REQUIRE(AppendFormat(str, "value={}", 99) == ErrorCode::Success);
    REQUIRE(StringViewUtf8(str) == StringViewUtf8("prefix: value=99"));
}

TEST_CASE("Format with padding", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("{:04d}", 42);
    REQUIRE(result.HasValue());
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("0042"));
}

TEST_CASE("Format with hex", "[StringFormat]")
{
    Expected<StringUtf8, ErrorCode> result = Format("{:#x}", 255);
    REQUIRE(StringViewUtf8(result.GetValue()) == StringViewUtf8("0xff"));
}

TEST_CASE("Format reports a failed allocation", "[StringFormat]")
{
    NullAllocator allocator;

    SECTION("With arguments")
    {
        PushDefault pd(&allocator);
        // Long enough that the result cannot live in the string's inline storage.
        Expected<StringUtf8, ErrorCode> result = Format("{} and {}", "aaaaaaaaaaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbbbbbb");
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Without arguments")
    {
        PushDefault pd(&allocator);
        Expected<StringUtf8, ErrorCode> result = Format("a literal long enough to need the heap");
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("AppendFormat stops at the first failure")
    {
        StringUtf8 str(&allocator);
        REQUIRE(AppendFormat(str, "{} {} {}", "aaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbb", "cccccccccccccccc") ==
                ErrorCode::OutOfMemory);
    }
}

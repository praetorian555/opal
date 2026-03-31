#include "test-helpers.h"

#include "opal/container/string-format.h"

using namespace Opal;

TEST_CASE("Format basic string", "[StringFormat]")
{
    StringUtf8 result = Format("hello {}", "world");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("hello world"));
}

TEST_CASE("Format integer", "[StringFormat]")
{
    StringUtf8 result = Format("{}", 42);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("42"));
}

TEST_CASE("Format float", "[StringFormat]")
{
    StringUtf8 result = Format("{:.2f}", 3.14);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("3.14"));
}

TEST_CASE("Format multiple args", "[StringFormat]")
{
    StringUtf8 result = Format("{} + {} = {}", 1, 2, 3);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("1 + 2 = 3"));
}

TEST_CASE("Format no args", "[StringFormat]")
{
    StringUtf8 result = Format("literal");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("literal"));
}

TEST_CASE("Format empty string", "[StringFormat]")
{
    StringUtf8 result = Format("");
    REQUIRE(result.IsEmpty());
}

TEST_CASE("AppendFormat to existing string", "[StringFormat]")
{
    StringUtf8 str("prefix: ");
    AppendFormat(str, "value={}", 99);
    REQUIRE(StringViewUtf8(str) == StringViewUtf8("prefix: value=99"));
}

TEST_CASE("Format with padding", "[StringFormat]")
{
    StringUtf8 result = Format("{:04d}", 42);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("0042"));
}

TEST_CASE("Format with hex", "[StringFormat]")
{
    StringUtf8 result = Format("{:#x}", 255);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("0xff"));
}

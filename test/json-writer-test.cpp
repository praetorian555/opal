#include "test-helpers.h"

#include "opal/container/json-reader.h"
#include "opal/container/json-writer.h"

using namespace Opal;

namespace
{
// Unwrap the success case, so the tests that are about the document stay about the document.
template <typename T>
JsonReader ParseOrFail(T&& input, AllocatorBase* allocator = nullptr)
{
    Expected<JsonReader, JsonParseError> result = JsonReader::Parse(std::forward<T>(input), allocator);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}
}  // namespace

namespace
{
// Unwrap the success case, so the tests that are about the JSON text stay about the JSON text.
template <typename... Args>
StringUtf8 SerializeOrFail(Args&&... args)
{
    Expected<StringUtf8, ErrorCode> result = JsonWriter::Serialize(std::forward<Args>(args)...);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}
}  // namespace

// ------------------------------------------------------------------------------------------------
// Primitives.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize null", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("null");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("null"));
}

TEST_CASE("Serialize true", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("true");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("true"));
}

TEST_CASE("Serialize false", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("false");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("false"));
}

TEST_CASE("Serialize integer", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("42");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("42"));
}

TEST_CASE("Serialize negative integer", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("-7");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("-7"));
}

TEST_CASE("Serialize zero", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("0");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("0"));
}

TEST_CASE("Serialize float", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("3.14");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    // Round-trip: parse the result and compare numbers.
    JsonReader reader2 = ParseOrFail(result);
    REQUIRE(reader2.GetRoot().GetNumber() == Catch::Approx(3.14));
}

TEST_CASE("Serialize empty string", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("")"));
}

TEST_CASE("Serialize simple string", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("hello")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("hello")"));
}

// ------------------------------------------------------------------------------------------------
// String escaping.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize string with newline", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("line1\nline2")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("line1\nline2")"));
}

TEST_CASE("Serialize string with tab", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("a\tb")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("a\tb")"));
}

TEST_CASE("Serialize string with backslash", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("a\\b")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("a\\b")"));
}

TEST_CASE("Serialize string with quote", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("\"a\\\"b\"");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    StringUtf8 expected("\"a\\\"b\"");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Serialize string with carriage return", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("a\rb")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("a\rb")"));
}

TEST_CASE("Serialize string with backspace", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("\"a\\bb\"");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    StringUtf8 expected("\"a\\bb\"");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Serialize string with form feed", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"("a\fb")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("a\fb")"));
}

TEST_CASE("Serialize string with control char", "[JsonWriter]")
{
    // \u0001 is a control character that should be escaped as \u0001.
    JsonReader reader = ParseOrFail(R"("a\u0001b")");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"("a\u0001b")"));
}

// ------------------------------------------------------------------------------------------------
// Arrays.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize empty array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[]");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[]"));
}

TEST_CASE("Serialize single element array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[1]");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[1]"));
}

TEST_CASE("Serialize multiple element array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[1,2,3]");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[1,2,3]"));
}

TEST_CASE("Serialize nested array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[[1,2],[3]]");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[[1,2],[3]]"));
}

// ------------------------------------------------------------------------------------------------
// Objects.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize empty object", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("{}");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("{}"));
}

TEST_CASE("Serialize single key object", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"({"a":1})");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"({"a":1})"));
}

TEST_CASE("Serialize nested object", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"({"a":{"b":2}})");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"({"a":{"b":2}})"));
}

// ------------------------------------------------------------------------------------------------
// Pretty print.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Pretty print array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[1,2,3]");
    JsonWriteOptions options;
    options.pretty = true;
    options.indent_width = 4;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    StringUtf8 expected("[\n    1,\n    2,\n    3\n]");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Pretty print object", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"({"a":1})");
    JsonWriteOptions options;
    options.pretty = true;
    options.indent_width = 4;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    StringUtf8 expected("{\n    \"a\": 1\n}");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Pretty print nested", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"({"a":[1,2]})");
    JsonWriteOptions options;
    options.pretty = true;
    options.indent_width = 2;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    StringUtf8 expected("{\n  \"a\": [\n    1,\n    2\n  ]\n}");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Pretty print with tabs", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[1,2]");
    JsonWriteOptions options;
    options.pretty = true;
    options.use_tabs = true;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    StringUtf8 expected("[\n\t1,\n\t2\n]");
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(expected));
}

TEST_CASE("Pretty print empty array", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("[]");
    JsonWriteOptions options;
    options.pretty = true;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[]"));
}

TEST_CASE("Pretty print empty object", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("{}");
    JsonWriteOptions options;
    options.pretty = true;
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), options);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("{}"));
}

// ------------------------------------------------------------------------------------------------
// Round-trip.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Round-trip simple object", "[JsonWriter]")
{
    StringUtf8 input(R"({"name":"Alice","score":100})");
    JsonReader reader1 = ParseOrFail(input);
    StringUtf8 serialized = SerializeOrFail(reader1.GetRoot());
    JsonReader reader2 = ParseOrFail(serialized);

    REQUIRE(reader2.GetRoot()["name"].GetString() == StringViewUtf8("Alice"));
    REQUIRE(reader2.GetRoot()["score"].GetNumberAs<i32>() == 100);
}

TEST_CASE("Round-trip array of mixed types", "[JsonWriter]")
{
    StringUtf8 input(R"([null,true,false,42,"hello"])");
    JsonReader reader1 = ParseOrFail(input);
    StringUtf8 serialized = SerializeOrFail(reader1.GetRoot());
    JsonReader reader2 = ParseOrFail(serialized);

    const JsonValue& root = reader2.GetRoot();
    REQUIRE(root[0].IsNull());
    REQUIRE(root[1].GetBool() == true);
    REQUIRE(root[2].GetBool() == false);
    REQUIRE(root[3].GetNumberAs<i32>() == 42);
    REQUIRE(root[4].GetString() == StringViewUtf8("hello"));
}

// ------------------------------------------------------------------------------------------------
// Errors.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize NaN reports an invalid argument", "[JsonWriter]")
{
    JsonValue value(static_cast<f64>(NAN));
    REQUIRE(JsonWriter::Serialize(value).GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Serialize Infinity reports an invalid argument", "[JsonWriter]")
{
    JsonValue value(static_cast<f64>(INFINITY));
    REQUIRE(JsonWriter::Serialize(value).GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Serialize negative Infinity reports an invalid argument", "[JsonWriter]")
{
    JsonValue value(static_cast<f64>(-INFINITY));
    REQUIRE(JsonWriter::Serialize(value).GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Serialize reports a failed allocation", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail(R"({"a":[1,2,3],"b":"a string long enough to need the heap"})");

    SECTION("Output string cannot be grown")
    {
        NullAllocator allocator;
        REQUIRE(JsonWriter::Serialize(reader.GetRoot(), &allocator).GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Container stack cannot be grown")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        // The output has its own allocator, so the only thing left to fail is the serializer's own stack.
        MallocAllocator output_allocator;
        REQUIRE(JsonWriter::Serialize(reader.GetRoot(), &output_allocator).GetError() == ErrorCode::OutOfMemory);
    }
}

// ------------------------------------------------------------------------------------------------
// Allocator.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize with explicit allocator", "[JsonWriter]")
{
    MallocAllocator allocator;
    JsonReader reader = ParseOrFail("[1,2,3]");
    StringUtf8 result = SerializeOrFail(reader.GetRoot(), &allocator);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[1,2,3]"));
}

// ------------------------------------------------------------------------------------------------
// Integer number precision.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize large integer preserves precision", "[JsonWriter]")
{
    // 2^53 + 1, not representable as f64.
    JsonReader reader = ParseOrFail("9007199254740993");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("9007199254740993"));
}

TEST_CASE("Serialize i64 max", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("9223372036854775807");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("9223372036854775807"));
}

TEST_CASE("Serialize negative large integer", "[JsonWriter]")
{
    JsonReader reader = ParseOrFail("-9007199254740993");
    StringUtf8 result = SerializeOrFail(reader.GetRoot());
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("-9007199254740993"));
}

TEST_CASE("Serialize programmatically built i64", "[JsonWriter]")
{
    JsonValue value = JsonValue::MakeNumber(static_cast<i64>(9007199254740993LL));
    StringUtf8 result = SerializeOrFail(value);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("9007199254740993"));
}

TEST_CASE("Round-trip large integer", "[JsonWriter]")
{
    StringUtf8 input("9007199254740993");
    JsonReader reader1 = ParseOrFail(input);
    StringUtf8 serialized = SerializeOrFail(reader1.GetRoot());
    JsonReader reader2 = ParseOrFail(serialized);
    REQUIRE(reader2.GetRoot().GetIntegerNumber() == 9007199254740993LL);
}

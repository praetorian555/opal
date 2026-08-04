#include "test-helpers.h"

#include "opal/container/json-reader.h"

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

// ------------------------------------------------------------------------------------------------
// Parse basics.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader parse null", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("null");
    REQUIRE(reader.GetRoot().IsNull());
}

TEST_CASE("JsonReader parse true", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("true");
    REQUIRE(reader.GetRoot().IsBool());
    REQUIRE(reader.GetRoot().GetBool() == true);
}

TEST_CASE("JsonReader parse false", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("false");
    REQUIRE(reader.GetRoot().IsBool());
    REQUIRE(reader.GetRoot().GetBool() == false);
}

TEST_CASE("JsonReader parse integer number", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE(reader.GetRoot().GetNumber() == 42.0);
}

TEST_CASE("JsonReader parse negative number", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("-3.14");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE(reader.GetRoot().GetNumber() == Catch::Approx(-3.14));
}

TEST_CASE("JsonReader parse exponent number", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("1.5e10");
    REQUIRE(reader.GetRoot().GetNumber() == Catch::Approx(1.5e10));
}

TEST_CASE("JsonReader parse string", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"("hello")");
    REQUIRE(reader.GetRoot().IsString());
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("hello"));
}

TEST_CASE("JsonReader parse string with escapes", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"("hello\nworld")");
    REQUIRE(reader.GetRoot().IsString());
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("hello\nworld"));
}

TEST_CASE("JsonReader parse string with unicode escape", "[JsonReader]")
{
    // \u0041 = 'A'
    JsonReader reader = ParseOrFail(R"("\u0041")");
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("A"));
}

TEST_CASE("JsonReader parse empty array", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[]");
    REQUIRE(reader.GetRoot().IsArray());
    REQUIRE(reader.GetRoot().GetSize() == 0);
}

TEST_CASE("JsonReader parse array", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[1, 2, 3]");
    const JsonValue& root = reader.GetRoot();
    REQUIRE(root.IsArray());
    REQUIRE(root.GetSize() == 3);
    REQUIRE(root[0].GetNumber() == 1.0);
    REQUIRE(root[1].GetNumber() == 2.0);
    REQUIRE(root[2].GetNumber() == 3.0);
}

TEST_CASE("JsonReader parse empty object", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("{}");
    REQUIRE(reader.GetRoot().IsObject());
    REQUIRE(reader.GetRoot().GetSize() == 0);
}

TEST_CASE("JsonReader parse object", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({"a": 1, "b": true})");
    const JsonValue& root = reader.GetRoot();
    REQUIRE(root.IsObject());
    REQUIRE(root.GetSize() == 2);
    REQUIRE(root["a"].GetNumber() == 1.0);
    REQUIRE(root["b"].GetBool() == true);
}

TEST_CASE("JsonReader parse nested structure", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({
        "players": [
            {"name": "Alice", "score": 100},
            {"name": "Bob", "score": 200}
        ]
    })");
    const JsonValue& root = reader.GetRoot();
    REQUIRE(root["players"][0]["name"].GetString() == StringViewUtf8("Alice"));
    REQUIRE(root["players"][1]["score"].GetNumber() == 200.0);
}

// ------------------------------------------------------------------------------------------------
// GetNumberAs.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader GetNumberAs", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    REQUIRE(reader.GetRoot().GetNumberAs<i32>() == 42);
    REQUIRE(reader.GetRoot().GetNumberAs<u64>() == 42);
}

// ------------------------------------------------------------------------------------------------
// GetPath.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader GetPath", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({
        "a": {"b": {"c": 42}}
    })");
    REQUIRE(reader.GetRoot().GetPath("a.b.c").GetNumber() == 42.0);
}

TEST_CASE("JsonReader GetPath with array index", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({
        "items": [10, 20, 30]
    })");
    REQUIRE(reader.GetRoot().GetPath("items.1").GetNumber() == 20.0);
}

// ------------------------------------------------------------------------------------------------
// Type queries.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader GetType", "[JsonReader]")
{
    REQUIRE(ParseOrFail("null").GetRoot().GetType() == JsonType::Null);
    REQUIRE(ParseOrFail("true").GetRoot().GetType() == JsonType::Bool);
    REQUIRE(ParseOrFail("1").GetRoot().GetType() == JsonType::Number);
    REQUIRE(ParseOrFail(R"("x")").GetRoot().GetType() == JsonType::String);
    REQUIRE(ParseOrFail("[]").GetRoot().GetType() == JsonType::Array);
    REQUIRE(ParseOrFail("{}").GetRoot().GetType() == JsonType::Object);
}

// ------------------------------------------------------------------------------------------------
// Type mismatch errors.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader type mismatch throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    REQUIRE_THROWS_AS(reader.GetRoot().GetBool(), JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot().GetString(), JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot()[0], JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot()["key"], JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot().GetSize(), JsonTypeMismatchException);
}

TEST_CASE("JsonReader array out of bounds throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[1]");
    REQUIRE_THROWS_AS(reader.GetRoot()[1], OutOfBoundsException);
}

TEST_CASE("JsonReader object missing key throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({"a": 1})");
    REQUIRE_THROWS_AS(reader.GetRoot()["b"], InvalidArgumentException);
}

// ------------------------------------------------------------------------------------------------
// Parse errors.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader parse error on invalid input", "[JsonReader]")
{
    REQUIRE(!JsonReader::Parse("").HasValue());
    REQUIRE(!JsonReader::Parse("{").HasValue());
    REQUIRE(!JsonReader::Parse("[1,]").HasValue());
    REQUIRE(!JsonReader::Parse("nul").HasValue());
    REQUIRE(JsonReader::Parse("nul").GetError().code == ErrorCode::InvalidArgument);
}

TEST_CASE("JsonReader reports a failed allocation while parsing", "[JsonReader]")
{
    NullAllocator allocator;

    SECTION("Array")
    {
        Expected<JsonReader, JsonParseError> result = JsonReader::Parse("[1,2,3]", &allocator);
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError().code == ErrorCode::OutOfMemory);
    }
    SECTION("Object")
    {
        Expected<JsonReader, JsonParseError> result = JsonReader::Parse(R"({"a":1})", &allocator);
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError().code == ErrorCode::OutOfMemory);
    }
    SECTION("Empty array")
    {
        Expected<JsonReader, JsonParseError> result = JsonReader::Parse("[]", &allocator);
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError().code == ErrorCode::OutOfMemory);
    }
}

TEST_CASE("JsonReader survives being moved", "[JsonReader]")
{
    // The document is short enough to live in the string's inline storage, which is where the parsed views point.
    // Moving the reader used to relocate those characters out from under them.
    auto MakeReader = []()
    {
        Expected<JsonReader, JsonParseError> parsed = JsonReader::Parse(StringUtf8(R"({"a": 1, "b": true})"));
        REQUIRE(parsed.HasValue());
        return std::move(parsed).GetValue();
    };

    JsonReader reader = MakeReader();
    JsonReader moved = std::move(reader);
    JsonReader moved_again = std::move(moved);

    const JsonValue& root = moved_again.GetRoot();
    REQUIRE(root.IsObject());
    REQUIRE(root.GetSize() == 2);
    REQUIRE(root["a"].GetNumber() == 1.0);
    REQUIRE(root["b"].GetBool() == true);
}

TEST_CASE("JsonReader parse error has line and column", "[JsonReader]")
{
    Expected<JsonReader, JsonParseError> result = JsonReader::Parse("{\n  \"a\": }");
    REQUIRE(!result.HasValue());
    REQUIRE(result.GetError().line == 2);
    // The message outlives the parser and the input, so it is still readable here.
    REQUIRE(result.GetError().GetMessage() != nullptr);
    REQUIRE(result.GetError().GetMessage()[0] != '\0');
}


// ------------------------------------------------------------------------------------------------
// Array iteration.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader array range-for", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[10, 20, 30]");
    f64 sum = 0;
    for (const JsonValue& elem : reader.GetRoot())
    {
        sum += elem.GetNumber();
    }
    REQUIRE(sum == 60.0);
}

TEST_CASE("JsonReader array iteration on non-array throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    REQUIRE_THROWS_AS(reader.GetRoot().begin(), JsonTypeMismatchException);
}

// ------------------------------------------------------------------------------------------------
// Object iteration.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader object Items iteration", "[JsonReader]")
{
    JsonReader reader = ParseOrFail(R"({"x": 1, "y": 2})");
    u64 count = 0;
    for (auto [key, val] : reader.GetRoot().Items())
    {
        REQUIRE(val->IsNumber());
        ++count;
    }
    REQUIRE(count == 2);
}

TEST_CASE("JsonReader Items on non-object throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[1]");
    REQUIRE_THROWS_AS(reader.GetRoot().Items(), JsonTypeMismatchException);
}

// ------------------------------------------------------------------------------------------------
// Visit.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader Visit", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    bool visited = false;
    reader.GetRoot().Visit(Overloaded{
        [](const JsonNull&) {},
        [](bool) {},
        [](f64) {},
        [&visited](i64) { visited = true; },
        [](const StringViewUtf8&) {},
        [](const JsonArray&) {},
        [](const JsonObject&) {},
    });
    REQUIRE(visited);
}

TEST_CASE("JsonReader VisitPartial", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    bool visited = false;
    reader.GetRoot().VisitPartial(Overloaded{
        [&visited](i64) { visited = true; },
    });
    REQUIRE(visited);
}

TEST_CASE("JsonReader VisitPartial ignores unmatched", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    bool visited = false;
    reader.GetRoot().VisitPartial(Overloaded{
        [&visited](bool) { visited = true; },
    });
    REQUIRE_FALSE(visited);
}

// ------------------------------------------------------------------------------------------------
// Clone.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader Clone value", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("[1, 2, 3]");
    JsonValue cloned = reader.GetRoot().Clone();
    REQUIRE(cloned.IsArray());
    REQUIRE(cloned.GetSize() == 3);
    REQUIRE(cloned[0].GetNumber() == 1.0);
}

// ------------------------------------------------------------------------------------------------
// Owned input.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader Parse with owned input", "[JsonReader]")
{
    StringUtf8 input;
    input.Append('[');
    input.Append('1');
    input.Append(']');
    JsonReader reader = ParseOrFail(Move(input));
    REQUIRE(reader.GetRoot().IsArray());
    REQUIRE(reader.GetRoot()[0].GetNumber() == 1.0);
}

// ------------------------------------------------------------------------------------------------
// Whitespace handling.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader handles whitespace", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("  \t\n { \n \"a\" : 1 \n } \n ");
    REQUIRE(reader.GetRoot().IsObject());
    REQUIRE(reader.GetRoot()["a"].GetNumber() == 1.0);
}

// ------------------------------------------------------------------------------------------------
// Trailing content.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader rejects trailing content", "[JsonReader]")
{
    REQUIRE(!JsonReader::Parse("42 42").HasValue());
}


TEST_CASE("Bad read", "[JsonReader]")
{
    const StringUtf8 file_path = R"(D:\Dev\obsidian\build\debug\Debug\obs.cache)";
}

// ------------------------------------------------------------------------------------------------
// Integer number support.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader parses integer as i64", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("42");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE(reader.GetRoot().IsIntegerNumber());
    REQUIRE(reader.GetRoot().GetIntegerNumber() == 42);
}

TEST_CASE("JsonReader parses negative integer as i64", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("-100");
    REQUIRE(reader.GetRoot().IsIntegerNumber());
    REQUIRE(reader.GetRoot().GetIntegerNumber() == -100);
}

TEST_CASE("JsonReader parses float as f64", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("3.14");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE_FALSE(reader.GetRoot().IsIntegerNumber());
}

TEST_CASE("JsonReader parses exponent number as f64", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("1e10");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE_FALSE(reader.GetRoot().IsIntegerNumber());
}

TEST_CASE("JsonReader large integer preserves precision", "[JsonReader]")
{
    // 2^53 + 1, not representable as f64.
    JsonReader reader = ParseOrFail("9007199254740993");
    REQUIRE(reader.GetRoot().IsIntegerNumber());
    REQUIRE(reader.GetRoot().GetIntegerNumber() == 9007199254740993LL);
    REQUIRE(reader.GetRoot().GetNumberAs<i64>() == 9007199254740993LL);
}

TEST_CASE("JsonReader i64 max", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("9223372036854775807");
    REQUIRE(reader.GetRoot().IsIntegerNumber());
    REQUIRE(reader.GetRoot().GetIntegerNumber() == 9223372036854775807LL);
}

TEST_CASE("JsonReader i64 min", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("-9223372036854775808");
    REQUIRE(reader.GetRoot().IsIntegerNumber());
    // Use explicit comparison to avoid literal issues.
    REQUIRE(reader.GetRoot().GetIntegerNumber() == static_cast<i64>(-9223372036854775807LL - 1));
}

TEST_CASE("JsonReader integer overflow falls back to f64", "[JsonReader]")
{
    // Larger than i64 max.
    JsonReader reader = ParseOrFail("9223372036854775808");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE_FALSE(reader.GetRoot().IsIntegerNumber());
}

TEST_CASE("JsonReader GetIntegerNumber on f64 throws", "[JsonReader]")
{
    JsonReader reader = ParseOrFail("3.14");
    REQUIRE_THROWS_AS(reader.GetRoot().GetIntegerNumber(), JsonTypeMismatchException);
}
TEST_CASE("JsonValue Try accessors", "[JsonReader]")
{
    auto parsed = JsonReader::Parse(R"({"flag": true, "count": 7, "ratio": 1.5, "name": "opal", "items": [10, 20]})");
    REQUIRE(parsed.HasValue());
    const JsonValue& root = parsed.GetValue().GetRoot();

    SECTION("Report the value when the type matches")
    {
        REQUIRE(root["flag"].TryGetBool().GetValue() == true);
        REQUIRE(root["count"].TryGetIntegerNumber().GetValue() == 7);
        REQUIRE(root["ratio"].TryGetNumber().GetValue() == 1.5);
        REQUIRE(root["name"].TryGetString().GetValue() == "opal");
    }
    SECTION("Report TypeMismatch instead of throwing")
    {
        REQUIRE(root["name"].TryGetBool().GetError() == ErrorCode::TypeMismatch);
        REQUIRE(root["flag"].TryGetNumber().GetError() == ErrorCode::TypeMismatch);
        REQUIRE(root["ratio"].TryGetIntegerNumber().GetError() == ErrorCode::TypeMismatch);
        REQUIRE(root["count"].TryGetString().GetError() == ErrorCode::TypeMismatch);
    }
    SECTION("The throwing accessors still throw for the same input")
    {
        REQUIRE_THROWS_AS(root["name"].GetBool(), JsonTypeMismatchException);
        REQUIRE_THROWS_AS(root["flag"].GetNumber(), JsonTypeMismatchException);
    }
    SECTION("TryAt")
    {
        REQUIRE(root["items"].TryAt(1).GetValue().GetIntegerNumber() == 20);
        REQUIRE(root["items"].TryAt(9).GetError() == ErrorCode::OutOfBounds);
        REQUIRE(root["name"].TryAt(0).GetError() == ErrorCode::TypeMismatch);
    }
    SECTION("TryFind")
    {
        REQUIRE(root.TryFind("count").GetValue().GetIntegerNumber() == 7);
        REQUIRE(root.TryFind("missing").GetError() == ErrorCode::KeyNotFound);
        REQUIRE(root["items"].TryFind("count").GetError() == ErrorCode::TypeMismatch);
    }
    SECTION("TryGetPath")
    {
        REQUIRE(root.TryGetPath("items.0").GetValue().GetIntegerNumber() == 10);
        REQUIRE(root.TryGetPath("missing.0").GetError() == ErrorCode::KeyNotFound);
        REQUIRE(root.TryGetPath("items.9").GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("GetPath still throws for the same input")
    {
        REQUIRE_THROWS_AS(root.GetPath("missing.0"), InvalidArgumentException);
        REQUIRE_THROWS_AS(root.GetPath("items.9"), OutOfBoundsException);
    }
}

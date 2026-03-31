#include "test-helpers.h"

#include "opal/container/json-reader.h"

using namespace Opal;

// ------------------------------------------------------------------------------------------------
// Parse basics.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader parse null", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("null");
    REQUIRE(reader.GetRoot().IsNull());
}

TEST_CASE("JsonReader parse true", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("true");
    REQUIRE(reader.GetRoot().IsBool());
    REQUIRE(reader.GetRoot().GetBool() == true);
}

TEST_CASE("JsonReader parse false", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("false");
    REQUIRE(reader.GetRoot().IsBool());
    REQUIRE(reader.GetRoot().GetBool() == false);
}

TEST_CASE("JsonReader parse integer number", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE(reader.GetRoot().GetNumber() == 42.0);
}

TEST_CASE("JsonReader parse negative number", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("-3.14");
    REQUIRE(reader.GetRoot().IsNumber());
    REQUIRE(reader.GetRoot().GetNumber() == Catch::Approx(-3.14));
}

TEST_CASE("JsonReader parse exponent number", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("1.5e10");
    REQUIRE(reader.GetRoot().GetNumber() == Catch::Approx(1.5e10));
}

TEST_CASE("JsonReader parse string", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"("hello")");
    REQUIRE(reader.GetRoot().IsString());
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("hello"));
}

TEST_CASE("JsonReader parse string with escapes", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"("hello\nworld")");
    REQUIRE(reader.GetRoot().IsString());
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("hello\nworld"));
}

TEST_CASE("JsonReader parse string with unicode escape", "[JsonReader]")
{
    // \u0041 = 'A'
    JsonReader reader = JsonReader::Parse(R"("\u0041")");
    REQUIRE(reader.GetRoot().GetString() == StringViewUtf8("A"));
}

TEST_CASE("JsonReader parse empty array", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("[]");
    REQUIRE(reader.GetRoot().IsArray());
    REQUIRE(reader.GetRoot().GetSize() == 0);
}

TEST_CASE("JsonReader parse array", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("[1, 2, 3]");
    const JsonValue& root = reader.GetRoot();
    REQUIRE(root.IsArray());
    REQUIRE(root.GetSize() == 3);
    REQUIRE(root[0].GetNumber() == 1.0);
    REQUIRE(root[1].GetNumber() == 2.0);
    REQUIRE(root[2].GetNumber() == 3.0);
}

TEST_CASE("JsonReader parse empty object", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("{}");
    REQUIRE(reader.GetRoot().IsObject());
    REQUIRE(reader.GetRoot().GetSize() == 0);
}

TEST_CASE("JsonReader parse object", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({"a": 1, "b": true})");
    const JsonValue& root = reader.GetRoot();
    REQUIRE(root.IsObject());
    REQUIRE(root.GetSize() == 2);
    REQUIRE(root["a"].GetNumber() == 1.0);
    REQUIRE(root["b"].GetBool() == true);
}

TEST_CASE("JsonReader parse nested structure", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({
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
    JsonReader reader = JsonReader::Parse("42");
    REQUIRE(reader.GetRoot().GetNumberAs<i32>() == 42);
    REQUIRE(reader.GetRoot().GetNumberAs<u64>() == 42);
}

// ------------------------------------------------------------------------------------------------
// GetPath.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader GetPath", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({
        "a": {"b": {"c": 42}}
    })");
    REQUIRE(reader.GetRoot().GetPath("a.b.c").GetNumber() == 42.0);
}

TEST_CASE("JsonReader GetPath with array index", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({
        "items": [10, 20, 30]
    })");
    REQUIRE(reader.GetRoot().GetPath("items.1").GetNumber() == 20.0);
}

// ------------------------------------------------------------------------------------------------
// Type queries.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader GetType", "[JsonReader]")
{
    REQUIRE(JsonReader::Parse("null").GetRoot().GetType() == JsonType::Null);
    REQUIRE(JsonReader::Parse("true").GetRoot().GetType() == JsonType::Bool);
    REQUIRE(JsonReader::Parse("1").GetRoot().GetType() == JsonType::Number);
    REQUIRE(JsonReader::Parse(R"("x")").GetRoot().GetType() == JsonType::String);
    REQUIRE(JsonReader::Parse("[]").GetRoot().GetType() == JsonType::Array);
    REQUIRE(JsonReader::Parse("{}").GetRoot().GetType() == JsonType::Object);
}

// ------------------------------------------------------------------------------------------------
// Type mismatch errors.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader type mismatch throws", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
    REQUIRE_THROWS_AS(reader.GetRoot().GetBool(), JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot().GetString(), JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot()[0], JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot()["key"], JsonTypeMismatchException);
    REQUIRE_THROWS_AS(reader.GetRoot().GetSize(), JsonTypeMismatchException);
}

TEST_CASE("JsonReader array out of bounds throws", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("[1]");
    REQUIRE_THROWS_AS(reader.GetRoot()[1], OutOfBoundsException);
}

TEST_CASE("JsonReader object missing key throws", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({"a": 1})");
    REQUIRE_THROWS_AS(reader.GetRoot()["b"], InvalidArgumentException);
}

// ------------------------------------------------------------------------------------------------
// Parse errors.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader parse error on invalid input", "[JsonReader]")
{
    REQUIRE_THROWS_AS(JsonReader::Parse(""), JsonParseException);
    REQUIRE_THROWS_AS(JsonReader::Parse("{"), JsonParseException);
    REQUIRE_THROWS_AS(JsonReader::Parse("[1,]"), JsonParseException);
    REQUIRE_THROWS_AS(JsonReader::Parse("nul"), JsonParseException);
}

TEST_CASE("JsonReader parse error has line and column", "[JsonReader]")
{
    try
    {
        JsonReader::Parse("{\n  \"a\": }");
        REQUIRE(false);
    }
    catch (const JsonParseException& e)
    {
        REQUIRE(e.line == 2);
    }
}

// ------------------------------------------------------------------------------------------------
// Array iteration.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader array range-for", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("[10, 20, 30]");
    f64 sum = 0;
    for (const JsonValue& elem : reader.GetRoot())
    {
        sum += elem.GetNumber();
    }
    REQUIRE(sum == 60.0);
}

TEST_CASE("JsonReader array iteration on non-array throws", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
    REQUIRE_THROWS_AS(reader.GetRoot().begin(), JsonTypeMismatchException);
}

// ------------------------------------------------------------------------------------------------
// Object iteration.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader object Items iteration", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse(R"({"x": 1, "y": 2})");
    u64 count = 0;
    for (auto [key, val] : reader.GetRoot().Items())
    {
        REQUIRE(val.IsNumber());
        ++count;
    }
    REQUIRE(count == 2);
}

TEST_CASE("JsonReader Items on non-object throws", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("[1]");
    REQUIRE_THROWS_AS(reader.GetRoot().Items(), JsonTypeMismatchException);
}

// ------------------------------------------------------------------------------------------------
// Visit.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader Visit", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
    bool visited = false;
    reader.GetRoot().Visit(Overloaded{
        [](const JsonNull&) {},
        [](bool) {},
        [&visited](f64) { visited = true; },
        [](const StringViewUtf8&) {},
        [](const JsonArray&) {},
        [](const JsonObject&) {},
    });
    REQUIRE(visited);
}

TEST_CASE("JsonReader VisitPartial", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
    bool visited = false;
    reader.GetRoot().VisitPartial(Overloaded{
        [&visited](f64) { visited = true; },
    });
    REQUIRE(visited);
}

TEST_CASE("JsonReader VisitPartial ignores unmatched", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("42");
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
    JsonReader reader = JsonReader::Parse("[1, 2, 3]");
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
    JsonReader reader = JsonReader::Parse(Move(input));
    REQUIRE(reader.GetRoot().IsArray());
    REQUIRE(reader.GetRoot()[0].GetNumber() == 1.0);
}

// ------------------------------------------------------------------------------------------------
// Whitespace handling.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader handles whitespace", "[JsonReader]")
{
    JsonReader reader = JsonReader::Parse("  \t\n { \n \"a\" : 1 \n } \n ");
    REQUIRE(reader.GetRoot().IsObject());
    REQUIRE(reader.GetRoot()["a"].GetNumber() == 1.0);
}

// ------------------------------------------------------------------------------------------------
// Trailing content.
// ------------------------------------------------------------------------------------------------

TEST_CASE("JsonReader rejects trailing content", "[JsonReader]")
{
    REQUIRE_THROWS_AS(JsonReader::Parse("42 42"), JsonParseException);
}

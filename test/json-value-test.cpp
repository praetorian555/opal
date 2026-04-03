#include "test-helpers.h"

#include "opal/container/json-reader.h"
#include "opal/container/json-writer.h"

using namespace Opal;

// ------------------------------------------------------------------------------------------------
// Factory methods.
// ------------------------------------------------------------------------------------------------

TEST_CASE("MakeNull creates null value", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNull();
    REQUIRE(value.IsNull());
    REQUIRE(value.GetType() == JsonType::Null);
}

TEST_CASE("MakeBool creates bool value", "[JsonValue]")
{
    JsonValue value_true = JsonValue::MakeBool(true);
    REQUIRE(value_true.IsBool());
    REQUIRE(value_true.GetBool() == true);

    JsonValue value_false = JsonValue::MakeBool(false);
    REQUIRE(value_false.IsBool());
    REQUIRE(value_false.GetBool() == false);
}

TEST_CASE("MakeNumber creates number value from f64", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(3.14);
    REQUIRE(value.IsNumber());
    REQUIRE(value.GetNumber() == Catch::Approx(3.14));
}

TEST_CASE("MakeNumber creates number value from integer", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(42);
    REQUIRE(value.IsNumber());
    REQUIRE(value.GetNumberAs<i32>() == 42);
}

TEST_CASE("MakeNumber creates number value from negative integer", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(-7);
    REQUIRE(value.IsNumber());
    REQUIRE(value.GetNumberAs<i32>() == -7);
}

TEST_CASE("MakeString creates string value", "[JsonValue]")
{
    StringUtf8 str("hello");
    JsonValue value = JsonValue::MakeString(str);
    REQUIRE(value.IsString());
    REQUIRE(value.GetString() == StringViewUtf8("hello"));
}

TEST_CASE("MakeArray creates empty array", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeArray();
    REQUIRE(value.IsArray());
    REQUIRE(value.GetSize() == 0);
}

TEST_CASE("MakeObject creates empty object", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeObject();
    REQUIRE(value.IsObject());
    REQUIRE(value.GetSize() == 0);
}

TEST_CASE("MakeArray with explicit allocator", "[JsonValue]")
{
    MallocAllocator allocator;
    JsonValue value = JsonValue::MakeArray(&allocator);
    REQUIRE(value.IsArray());
    REQUIRE(value.GetSize() == 0);
}

TEST_CASE("MakeObject with explicit allocator", "[JsonValue]")
{
    MallocAllocator allocator;
    JsonValue value = JsonValue::MakeObject(&allocator);
    REQUIRE(value.IsObject());
    REQUIRE(value.GetSize() == 0);
}

// ------------------------------------------------------------------------------------------------
// Mutation.
// ------------------------------------------------------------------------------------------------

TEST_CASE("PushBack adds elements to array", "[JsonValue]")
{
    JsonValue arr = JsonValue::MakeArray();
    arr.PushBack(JsonValue::MakeNumber(1));
    arr.PushBack(JsonValue::MakeNumber(2));
    arr.PushBack(JsonValue::MakeNumber(3));

    REQUIRE(arr.GetSize() == 3);
    REQUIRE(arr[0].GetNumberAs<i32>() == 1);
    REQUIRE(arr[1].GetNumberAs<i32>() == 2);
    REQUIRE(arr[2].GetNumberAs<i32>() == 3);
}

TEST_CASE("PushBack on non-array throws", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(42);
    REQUIRE_THROWS_AS(value.PushBack(JsonValue::MakeNull()), JsonTypeMismatchException);
}

TEST_CASE("Insert adds key-value pairs to object", "[JsonValue]")
{
    StringUtf8 key_name("name");
    StringUtf8 key_score("score");
    StringUtf8 name_value("Alice");

    JsonValue obj = JsonValue::MakeObject();
    obj.Insert(key_name, JsonValue::MakeString(name_value));
    obj.Insert(key_score, JsonValue::MakeNumber(100));

    REQUIRE(obj.GetSize() == 2);
    REQUIRE(obj[StringViewUtf8("name")].GetString() == StringViewUtf8("Alice"));
    REQUIRE(obj[StringViewUtf8("score")].GetNumberAs<i32>() == 100);
}

TEST_CASE("Insert on non-object throws", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeArray();
    REQUIRE_THROWS_AS(value.Insert(StringViewUtf8("key"), JsonValue::MakeNull()), JsonTypeMismatchException);
}

// ------------------------------------------------------------------------------------------------
// Building nested structures.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Build nested array", "[JsonValue]")
{
    JsonValue inner = JsonValue::MakeArray();
    inner.PushBack(JsonValue::MakeNumber(1));
    inner.PushBack(JsonValue::MakeNumber(2));

    JsonValue outer = JsonValue::MakeArray();
    outer.PushBack(std::move(inner));
    outer.PushBack(JsonValue::MakeNumber(3));

    REQUIRE(outer.GetSize() == 2);
    REQUIRE(outer[0].IsArray());
    REQUIRE(outer[0].GetSize() == 2);
    REQUIRE(outer[0][0].GetNumberAs<i32>() == 1);
    REQUIRE(outer[0][1].GetNumberAs<i32>() == 2);
    REQUIRE(outer[1].GetNumberAs<i32>() == 3);
}

TEST_CASE("Build object with nested array", "[JsonValue]")
{
    StringUtf8 key_name("name");
    StringUtf8 key_scores("scores");
    StringUtf8 name_value("Bob");

    JsonValue scores = JsonValue::MakeArray();
    scores.PushBack(JsonValue::MakeNumber(90));
    scores.PushBack(JsonValue::MakeNumber(85));

    JsonValue obj = JsonValue::MakeObject();
    obj.Insert(key_name, JsonValue::MakeString(name_value));
    obj.Insert(key_scores, std::move(scores));

    REQUIRE(obj[StringViewUtf8("name")].GetString() == StringViewUtf8("Bob"));
    REQUIRE(obj[StringViewUtf8("scores")].GetSize() == 2);
    REQUIRE(obj[StringViewUtf8("scores")][0].GetNumberAs<i32>() == 90);
    REQUIRE(obj[StringViewUtf8("scores")][1].GetNumberAs<i32>() == 85);
}

// ------------------------------------------------------------------------------------------------
// Serialization round-trip.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Serialize programmatically built array", "[JsonValue]")
{
    JsonValue arr = JsonValue::MakeArray();
    arr.PushBack(JsonValue::MakeNumber(1));
    arr.PushBack(JsonValue::MakeNumber(2));
    arr.PushBack(JsonValue::MakeNumber(3));

    StringUtf8 result = JsonWriter::Serialize(arr);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8("[1,2,3]"));
}

TEST_CASE("Serialize programmatically built object", "[JsonValue]")
{
    StringUtf8 key_a("a");

    JsonValue obj = JsonValue::MakeObject();
    obj.Insert(key_a, JsonValue::MakeNumber(1));

    StringUtf8 result = JsonWriter::Serialize(obj);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"({"a":1})"));
}

TEST_CASE("Build and serialize mixed types", "[JsonValue]")
{
    StringUtf8 str_hello("hello");

    JsonValue arr = JsonValue::MakeArray();
    arr.PushBack(JsonValue::MakeNull());
    arr.PushBack(JsonValue::MakeBool(true));
    arr.PushBack(JsonValue::MakeBool(false));
    arr.PushBack(JsonValue::MakeNumber(42));
    arr.PushBack(JsonValue::MakeString(str_hello));

    StringUtf8 result = JsonWriter::Serialize(arr);
    REQUIRE(StringViewUtf8(result) == StringViewUtf8(R"([null,true,false,42,"hello"])"));
}

// ------------------------------------------------------------------------------------------------
// Integer number (i64) support.
// ------------------------------------------------------------------------------------------------

TEST_CASE("MakeNumber from integer stores as i64", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(42);
    REQUIRE(value.IsNumber());
    REQUIRE(value.IsIntegerNumber());
    REQUIRE(value.GetIntegerNumber() == 42);
    REQUIRE(value.GetType() == JsonType::Number);
}

TEST_CASE("MakeNumber from i64 stores as i64", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(static_cast<i64>(9007199254740993LL));
    REQUIRE(value.IsIntegerNumber());
    REQUIRE(value.GetIntegerNumber() == 9007199254740993LL);
}

TEST_CASE("MakeNumber from f64 stores as f64", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(3.14);
    REQUIRE(value.IsNumber());
    REQUIRE_FALSE(value.IsIntegerNumber());
}

TEST_CASE("GetNumber on i64 converts to f64", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(42);
    REQUIRE(value.GetNumber() == 42.0);
}

TEST_CASE("GetNumberAs from i64 narrows directly", "[JsonValue]")
{
    JsonValue value = JsonValue::MakeNumber(static_cast<i64>(9007199254740993LL));
    REQUIRE(value.GetNumberAs<i64>() == 9007199254740993LL);
}
#include "test-helpers.h"

#include "opal/logging.h"
#include "opal/program-arguments.h"

using namespace Opal;

TEST_CASE("Program arguments", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    i32 test_i32 = 0;
    u32 test_u32 = 0;
    StringUtf8 test_str;
    bool test_bool = false;
    DynamicArray<i32> test_arr_i32;
    DynamicArray<u32> test_arr_u32;
    DynamicArray<StringUtf8> test_arr_str;
    i32 test_i32_opt = 0;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(test_i32, {.name = "test_i32", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_u32, {.name = "test_u32", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_str, {.name = "test_str", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_bool, {.name = "test_bool", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_arr_i32, {.name = "test_arr_i32", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_arr_u32, {.name = "test_arr_u32", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_arr_str, {.name = "test_arr_str", .desc = "Test argument", .is_optional = false});
    builder.AddArgumentDefinition(test_i32_opt, {.name = "test_i32_opt", .desc = "Test argument", .is_optional = true});

    const char* arguments[] = {
        "program-name",
        "test_i32=-5",
        "test_u32=6",
        "test_str=\"Hello there\"",
        "test_bool",
        "test_arr_i32=-2,5,10",
        "test_arr_u32=6,8,12",
        "test_arr_str=\"Hello there,partner\"",

    };
    builder.Build(arguments, 8);
    REQUIRE(test_i32 == -5);
    REQUIRE(test_u32 == 6);
    REQUIRE(test_str == "Hello there");
    REQUIRE(test_bool == true);
    REQUIRE(test_arr_i32.GetSize() == 3);
    REQUIRE(test_arr_i32[0] == -2);
    REQUIRE(test_arr_i32[1] == 5);
    REQUIRE(test_arr_i32[2] == 10);
    REQUIRE(test_arr_u32.GetSize() == 3);
    REQUIRE(test_arr_u32[0] == 6);
    REQUIRE(test_arr_u32[1] == 8);
    REQUIRE(test_arr_u32[2] == 12);
    REQUIRE(test_arr_str.GetSize() == 2);
    REQUIRE(test_arr_str[0] == "Hello there");
    REQUIRE(test_arr_str[1] == "partner");

    SetLogger(nullptr);
}

TEST_CASE("String argument with valid possible value", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    StringUtf8 test_str;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(test_str, {.name = "test_str", .desc = "Test argument", .is_optional = false, .possible_values = {"foo", "bar"}});

    const char* arguments[] = {"program-name", "test_str=foo"};
    builder.Build(arguments, 2);
    REQUIRE(test_str == "foo");

    SetLogger(nullptr);
}

TEST_CASE("String argument with invalid possible value", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    StringUtf8 test_str;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(test_str, {.name = "test_str", .desc = "Test argument", .is_optional = false, .possible_values = {"foo", "bar"}});

    const char* arguments[] = {"program-name", "test_str=baz"};
    REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);

    SetLogger(nullptr);
}

TEST_CASE("Array string argument with valid possible values", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<StringUtf8> test_arr_str;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(
        test_arr_str, {.name = "test_arr_str", .desc = "Test argument", .is_optional = false, .possible_values = {"foo", "bar", "baz"}});

    const char* arguments[] = {"program-name", "test_arr_str=foo,bar"};
    builder.Build(arguments, 2);
    REQUIRE(test_arr_str.GetSize() == 2);
    REQUIRE(test_arr_str[0] == "foo");
    REQUIRE(test_arr_str[1] == "bar");

    SetLogger(nullptr);
}

TEST_CASE("Array string argument with invalid possible value", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<StringUtf8> test_arr_str;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(
        test_arr_str, {.name = "test_arr_str", .desc = "Test argument", .is_optional = false, .possible_values = {"foo", "bar"}});

    const char* arguments[] = {"program-name", "test_arr_str=foo,invalid"};
    REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);

    SetLogger(nullptr);
}

TEST_CASE("Required program argument is missing", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    i32 test_i32 = 0;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(test_i32, {.name = "test_i32", .desc = "Test argument", .is_optional = false});

    const char* arguments[] = {
        "program-name"
    };
    REQUIRE_THROWS_AS(builder.Build(arguments, 1), InvalidArgumentException);

    SetLogger(nullptr);
}

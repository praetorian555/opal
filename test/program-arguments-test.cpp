#include "test-helpers.h"

#include "opal/program-arguments.h"

using namespace Opal;

TEST_CASE("Program arguments", "[ProgramArguments]")
{
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
    REQUIRE(builder.Build(arguments, 8) == true);
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
}

TEST_CASE("Required program argument is missing", "[ProgramArguments]")
{
    i32 test_i32 = 0;

    ProgramArgumentsBuilder builder;
    builder.AddArgumentDefinition(test_i32, {.name = "test_i32", .desc = "Test argument", .is_optional = false});

    const char* arguments[] = {
        "program-name"
    };
    REQUIRE(builder.Build(arguments, 1) == false);
}

TEST_CASE("Print help", "[ProgramArguments]")
{
    i32 test_i32 = 0;
    i32 test_i32_opt = 0;

    ProgramArgumentsBuilder builder;
    builder.AddProgramDescription("Test program to see how the help looks.");
    builder.AddUsageExample("prog-name test_i32=<some-value>");
    builder.AddUsageExample("prog-name test_i32=<some-value> test_i32_opt=<some-opt-value>");
    builder.AddArgumentDefinition(test_i32, {.name = "test_i32", .desc = "Value to be processed.", .is_optional = false});
    builder.AddArgumentDefinition(test_i32_opt, {.name = "test_i32_opt", .desc = "Optional value to modify the calculation.", .is_optional = true});

    const char* arguments[] = {
        "prog-name"
    };
    builder.Build(arguments, 1);
}

#include "test-helpers.h"

#include "opal/logging.h"
#include "opal/program-arguments.h"

using namespace Opal;

TEST_CASE("Request help and version", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Help requested")
    {
        const char* arguments[] = {"program-name", "help"};
        ProgramArgumentsBuilder builder;
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), HelpRequestedException);
    }
    SECTION("Help requested with -- prefix")
    {
        const char* arguments[] = {"program-name", "--help"};
        ProgramArgumentsBuilder builder;
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), HelpRequestedException);
    }
    SECTION("Version requested")
    {
        const char* arguments[] = {"program-name", "version"};
        ProgramArgumentsBuilder builder;
        builder.SetVersion(1, 2, 3);
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), VersionRequestedException);
    }
    SECTION("Version requested with -- prefix")
    {
        const char* arguments[] = {"program-name", "--version"};
        ProgramArgumentsBuilder builder;
        builder.SetVersion(1, 2, 3);
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), VersionRequestedException);
    }
}

TEST_CASE("Program arguments", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Loading of valid arguments")
    {
        i32 test_i32 = 0;
        u32 test_u32 = 0;
        StringUtf8 test_str;
        bool test_bool = false;

        ProgramArgumentsBuilder builder;
        builder.AddArgument("test_i32", "Test argument", Ref{test_i32}, false);
        builder.AddArgument("test_u32", "Test argument", Ref{test_u32}, false);
        builder.AddArgument("test_str", "Test argument", Ref{test_str}, false);
        builder.AddArgument("test_bool", "Test argument", Ref{test_bool}, false);

        // clang-format off
        const char* arguments[] = {
            "program-name",
            "test_i32=-5",
            "test_u32=6",
            "test_str=\"Hello there\"",
            "test_bool",
        };
        // clang-format on

        REQUIRE_NOTHROW(builder.Build(arguments, 5));
        REQUIRE(test_i32 == -5);
        REQUIRE(test_u32 == 6);
        REQUIRE(test_str == "Hello there");
        REQUIRE(test_bool == true);
    }
    SECTION("Loading of enum argument")
    {
        enum class CppStandard
        {
            Cpp11,
            Cpp14,
            Cpp17
        };

        CppStandard standard = CppStandard::Cpp11;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("standard", "Standard argument", Ref{standard}, false,
                            {{"c++11", CppStandard::Cpp11}, {"c++14", CppStandard::Cpp14}, {"c++17", CppStandard::Cpp17}});
        const char* arguments[] = {"program-name", "standard=c++14"};
        builder.Build(arguments, 2);
        REQUIRE(standard == CppStandard::Cpp14);
    }
    SECTION("Optional argument missing")
    {
        bool test_bool = false;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("test_bool", "Test argument", Ref{test_bool}, true);

        const char* arguments[] = {
            "program-name",
        };

        REQUIRE_NOTHROW(builder.Build(arguments, 1));
        REQUIRE(test_bool == false);
    }
    SECTION("Required argument missing")
    {
        bool test_bool = false;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("test_bool", "Test argument", Ref{test_bool}, false);

        const char* arguments[] = {
            "program-name",
        };

        REQUIRE_THROWS_AS(builder.Build(arguments, 1), InvalidArgumentException);
        REQUIRE(test_bool == false);
    }

    SetLogger(nullptr);
}

TEST_CASE("AddArgument validation", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Empty name throws")
    {
        bool value = false;
        ProgramArgumentsBuilder builder;
        REQUIRE_THROWS_AS(builder.AddArgument("", "Valid description", Ref{value}, false), InvalidArgumentException);
    }
    SECTION("Empty description throws")
    {
        bool value = false;
        ProgramArgumentsBuilder builder;
        REQUIRE_THROWS_AS(builder.AddArgument("name", "", Ref{value}, false), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("String argument with possible values", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Valid value accepted")
    {
        StringUtf8 mode;
        ProgramArgumentsBuilder builder;
        DynamicArray<StringUtf8> possible_values;
        possible_values.PushBack("debug");
        possible_values.PushBack("release");
        builder.AddArgument("mode", "Build mode", Ref{mode}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "mode=release"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(mode == "release");
    }
    SECTION("Invalid value throws")
    {
        StringUtf8 mode;
        ProgramArgumentsBuilder builder;
        DynamicArray<StringUtf8> possible_values;
        possible_values.PushBack("debug");
        possible_values.PushBack("release");
        builder.AddArgument("mode", "Build mode", Ref{mode}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "mode=profile"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("String argument with possible value mappings", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Valid mapping accepted")
    {
        StringUtf8 output_path;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, StringUtf8> mappings({{"home", "/home/user"}, {"tmp", "/tmp"}});
        builder.AddArgument("output", "Output path", Ref{output_path}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "output=home"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(output_path == "/home/user");
    }
    SECTION("Invalid mapping key throws")
    {
        StringUtf8 output_path;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, StringUtf8> mappings({{"home", "/home/user"}, {"tmp", "/tmp"}});
        builder.AddArgument("output", "Output path", Ref{output_path}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "output=invalid"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("String argument with both possible values and mappings throws", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    StringUtf8 value;
    ProgramArgumentsBuilder builder;
    DynamicArray<StringUtf8> possible_values;
    possible_values.PushBack("a");
    HashMap<StringUtf8, StringUtf8> mappings({{"b", "c"}});
    REQUIRE_THROWS_AS(
        (MakeScoped<Impl::TypedProgramArgumentDefinition<StringUtf8>>(GetDefaultAllocator(), Ref{value}, StringUtf8("name"),
                                                                      StringUtf8("desc"), false, std::move(possible_values),
                                                                      std::move(mappings))),
        InvalidArgumentException);

    SetLogger(nullptr);
}

TEST_CASE("Integer argument types", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("i64 argument")
    {
        i64 value = 0;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("val", "A 64-bit integer", Ref{value}, false);

        const char* arguments[] = {"program-name", "val=9999999999"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(value == 9999999999LL);
    }
    SECTION("u64 argument")
    {
        u64 value = 0;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("val", "A 64-bit unsigned integer", Ref{value}, false);

        const char* arguments[] = {"program-name", "val=18446744073709551615"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(value == 18446744073709551615ULL);
    }
    SECTION("Integer with possible values")
    {
        i32 value = 0;
        ProgramArgumentsBuilder builder;
        DynamicArray<i32> possible_values;
        possible_values.PushBack(1);
        possible_values.PushBack(2);
        possible_values.PushBack(3);
        builder.AddArgument("val", "An integer with constraints", Ref{value}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "val=2"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(value == 2);
    }

    SetLogger(nullptr);
}

TEST_CASE("Integer argument with both possible values and mappings throws", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    i32 value = 0;
    DynamicArray<i32> possible_values;
    possible_values.PushBack(1);
    HashMap<StringUtf8, i32> mappings({{"one", 1}});
    REQUIRE_THROWS_AS(
        (MakeScoped<Impl::TypedProgramArgumentDefinition<i32>>(GetDefaultAllocator(), Ref{value}, StringUtf8("val"), StringUtf8("desc"),
                                                               false, std::move(possible_values), std::move(mappings))),
        InvalidArgumentException);

    SetLogger(nullptr);
}

TEST_CASE("Enum argument validation", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    enum class Color
    {
        Red,
        Green,
        Blue
    };

    SECTION("Enum with invalid mapping key throws")
    {
        Color color = Color::Red;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("color", "A color", Ref{color}, false,
                            HashMap<StringUtf8, Color>({{"red", Color::Red}, {"green", Color::Green}, {"blue", Color::Blue}}));

        const char* arguments[] = {"program-name", "color=yellow"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }
    SECTION("Enum without mappings throws")
    {
        Color color = Color::Red;
        REQUIRE_THROWS_AS(
            (MakeScoped<Impl::TypedProgramArgumentDefinition<Color>>(GetDefaultAllocator(), Ref{color}, StringUtf8("color"),
                                                                     StringUtf8("A color"), false, DynamicArray<Color>{},
                                                                     HashMap<StringUtf8, Color>({}))),
            InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("String argument without quotes", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    StringUtf8 value;
    ProgramArgumentsBuilder builder;
    builder.AddArgument("name", "A name", Ref{value}, false);

    const char* arguments[] = {"program-name", "name=hello"};
    REQUIRE_NOTHROW(builder.Build(arguments, 2));
    REQUIRE(value == "hello");

    SetLogger(nullptr);
}

TEST_CASE("Multiple optional arguments partially provided", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    bool flag_a = false;
    bool flag_b = false;
    StringUtf8 opt_str;
    i32 opt_int = 0;

    ProgramArgumentsBuilder builder;
    builder.AddArgument("flag_a", "Flag A", Ref{flag_a}, true);
    builder.AddArgument("flag_b", "Flag B", Ref{flag_b}, true);
    builder.AddArgument("opt_str", "Optional string", Ref{opt_str}, true);
    builder.AddArgument("opt_int", "Optional integer", Ref{opt_int}, true);

    const char* arguments[] = {"program-name", "flag_a", "opt_int=42"};
    REQUIRE_NOTHROW(builder.Build(arguments, 3));
    REQUIRE(flag_a == true);
    REQUIRE(flag_b == false);
    REQUIRE(opt_str.IsEmpty());
    REQUIRE(opt_int == 42);

    SetLogger(nullptr);
}

TEST_CASE("DynamicArray<i32> argument", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Basic comma-separated values")
    {
        DynamicArray<i32> values;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("vals", "Integer values", Ref{values}, false);

        const char* arguments[] = {"program-name", "vals=1,2,3"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 3);
        REQUIRE(values[0] == 1);
        REQUIRE(values[1] == 2);
        REQUIRE(values[2] == 3);
    }
    SECTION("With possible values - valid")
    {
        DynamicArray<i32> values;
        ProgramArgumentsBuilder builder;
        DynamicArray<i32> possible_values;
        possible_values.PushBack(1);
        possible_values.PushBack(2);
        possible_values.PushBack(3);
        builder.AddArgument("vals", "Integer values", Ref{values}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "vals=1,3"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == 1);
        REQUIRE(values[1] == 3);
    }
    SECTION("With possible values - invalid")
    {
        DynamicArray<i32> values;
        ProgramArgumentsBuilder builder;
        DynamicArray<i32> possible_values;
        possible_values.PushBack(1);
        possible_values.PushBack(2);
        builder.AddArgument("vals", "Integer values", Ref{values}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "vals=1,5"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("DynamicArray<u64> argument", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<u64> values;
    ProgramArgumentsBuilder builder;
    builder.AddArgument("vals", "Unsigned 64-bit values", Ref{values}, false);

    const char* arguments[] = {"program-name", "vals=100,200,300"};
    REQUIRE_NOTHROW(builder.Build(arguments, 2));
    REQUIRE(values.GetSize() == 3);
    REQUIRE(values[0] == 100);
    REQUIRE(values[1] == 200);
    REQUIRE(values[2] == 300);

    SetLogger(nullptr);
}

TEST_CASE("DynamicArray<StringUtf8> argument", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    SECTION("Basic comma-separated values")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("names", "Name values", Ref{values}, false);

        const char* arguments[] = {"program-name", "names=hello,world"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == "hello");
        REQUIRE(values[1] == "world");
    }
    SECTION("With quotes")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        builder.AddArgument("names", "Name values", Ref{values}, false);

        const char* arguments[] = {"program-name", "names=\"hello,world\""};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == "hello");
        REQUIRE(values[1] == "world");
    }
    SECTION("With possible values - valid")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        DynamicArray<StringUtf8> possible_values;
        possible_values.PushBack("foo");
        possible_values.PushBack("bar");
        builder.AddArgument("names", "Name values", Ref{values}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "names=foo,bar"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == "foo");
        REQUIRE(values[1] == "bar");
    }
    SECTION("With possible values - invalid")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        DynamicArray<StringUtf8> possible_values;
        possible_values.PushBack("foo");
        possible_values.PushBack("bar");
        builder.AddArgument("names", "Name values", Ref{values}, false, std::move(possible_values));

        const char* arguments[] = {"program-name", "names=foo,baz"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }
    SECTION("With mappings - valid")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, StringUtf8> mappings({{"home", "/home/user"}, {"tmp", "/tmp"}});
        builder.AddArgument("paths", "Path values", Ref{values}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "paths=home,tmp"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == "/home/user");
        REQUIRE(values[1] == "/tmp");
    }
    SECTION("With mappings - invalid")
    {
        DynamicArray<StringUtf8> values;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, StringUtf8> mappings({{"home", "/home/user"}, {"tmp", "/tmp"}});
        builder.AddArgument("paths", "Path values", Ref{values}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "paths=home,invalid"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("DynamicArray<Enum> argument", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    enum class Color
    {
        Red,
        Green,
        Blue
    };

    SECTION("With mappings - valid")
    {
        DynamicArray<Color> values;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, Color> mappings({{"red", Color::Red}, {"green", Color::Green}, {"blue", Color::Blue}});
        builder.AddArgument("colors", "Color values", Ref{values}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "colors=red,blue"};
        REQUIRE_NOTHROW(builder.Build(arguments, 2));
        REQUIRE(values.GetSize() == 2);
        REQUIRE(values[0] == Color::Red);
        REQUIRE(values[1] == Color::Blue);
    }
    SECTION("With mappings - invalid")
    {
        DynamicArray<Color> values;
        ProgramArgumentsBuilder builder;
        HashMap<StringUtf8, Color> mappings({{"red", Color::Red}, {"green", Color::Green}, {"blue", Color::Blue}});
        builder.AddArgument("colors", "Color values", Ref{values}, false, std::move(mappings));

        const char* arguments[] = {"program-name", "colors=red,yellow"};
        REQUIRE_THROWS_AS(builder.Build(arguments, 2), InvalidArgumentException);
    }

    SetLogger(nullptr);
}

TEST_CASE("DynamicArray argument with both possible values and mappings throws", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<StringUtf8> values;
    DynamicArray<StringUtf8> possible_values;
    possible_values.PushBack("a");
    HashMap<StringUtf8, StringUtf8> mappings({{"b", "c"}});
    REQUIRE_THROWS_AS(
        (MakeScoped<Impl::TypedProgramArgumentDefinition<DynamicArray<StringUtf8>>>(
            GetDefaultAllocator(), Ref{values}, StringUtf8("name"), StringUtf8("desc"), false, std::move(possible_values),
            std::move(mappings))),
        InvalidArgumentException);

    SetLogger(nullptr);
}

TEST_CASE("Optional DynamicArray argument not provided", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<i32> values;
    ProgramArgumentsBuilder builder;
    builder.AddArgument("vals", "Integer values", Ref{values}, true);

    const char* arguments[] = {"program-name"};
    REQUIRE_NOTHROW(builder.Build(arguments, 1));
    REQUIRE(values.IsEmpty());

    SetLogger(nullptr);
}

TEST_CASE("Required DynamicArray argument not provided", "[ProgramArguments]")
{
    Logger logger;
    SetLogger(&logger);

    DynamicArray<i32> values;
    ProgramArgumentsBuilder builder;
    builder.AddArgument("vals", "Integer values", Ref{values}, false);

    const char* arguments[] = {"program-name"};
    REQUIRE_THROWS_AS(builder.Build(arguments, 1), InvalidArgumentException);

    SetLogger(nullptr);
}

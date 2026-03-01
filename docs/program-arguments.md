# Program Arguments

Header: `opal/program-arguments.h`

A typed command-line argument parser that binds arguments directly to C++ variables. Arguments use `name=value` syntax, supports required and optional arguments, value validation, value mappings, enum types, and auto-generated help output.

## Quick Start

```cpp
#include "opal/program-arguments.h"

int main(int argc, const char** argv)
{
    Opal::StringUtf8 config_path;
    Opal::i32 port = 8080;
    bool verbose = false;

    Opal::ProgramArgumentsBuilder builder;
    builder.AddProgramDescription("My application server")
           .AddUsageExample("my_app config_path=server.json port=3000 verbose")
           .SetVersion(1, 0, 0)
           .AddArgument("config_path", "Path to config file", Opal::Ref{config_path}, false)
           .AddArgument("port", "Server port", Opal::Ref{port}, true)
           .AddArgument("verbose", "Enable verbose output", Opal::Ref{verbose}, true);

    try
    {
        builder.Build(argv, argc);
    }
    catch (const Opal::HelpRequestedException&)
    {
        return 0;
    }
    catch (const Opal::VersionRequestedException&)
    {
        return 0;
    }
    catch (const Opal::InvalidArgumentException&)
    {
        return 1;
    }

    // config_path, port, and verbose are now populated
}
```

Command line:

```
my_app config_path=server.json port=3000 verbose
```

## Argument Syntax

Arguments are passed as positional tokens in the form `name=value`. Boolean flags use just the name with no value.

```
my_app name=value flag "quoted_arg=\"hello world\""
```

- `name=value` - assigns `value` to the argument named `name`
- `flag` - sets a `bool` argument to `true`
- Values containing spaces must be wrapped in quotes: `arg="hello world"`
- Array values use comma separation: `ids=1,2,3`

## Supported Types

| Type | Syntax | Example |
|------|--------|---------|
| `bool` | `name` | `verbose` |
| `i32`, `i64` | `name=value` | `count=-5` |
| `u32`, `u64` | `name=value` | `port=8080` |
| `StringUtf8` | `name=value` | `mode=release` |
| Enum types | `name=key` | `standard=c++17` |
| `DynamicArray<i32>` | `name=a,b,c` | `offsets=-2,5,10` |
| `DynamicArray<u32>` | `name=a,b,c` | `ids=6,8,12` |
| `DynamicArray<StringUtf8>` | `name=a,b,c` | `tags=fast,safe,new` |
| `DynamicArray<Enum>` | `name=a,b,c` | `colors=red,blue` |

String values can be quoted to include spaces:

```
my_app message="Hello there"
my_app names="Alice,Bob,Charlie"
```

## Defining Arguments

Use `ProgramArgumentsBuilder` to define arguments and parse the command line. Each argument is bound to a variable via `Ref`.

```cpp
Opal::i32 count = 0;
Opal::StringUtf8 name;
bool debug = false;

Opal::ProgramArgumentsBuilder builder;
builder.AddArgument("count", "Number of iterations", Opal::Ref{count}, false)
       .AddArgument("name", "User name", Opal::Ref{name}, true)
       .AddArgument("debug", "Enable debug mode", Opal::Ref{debug}, true);
```

### Required vs Optional

The last parameter of `AddArgument` controls whether the argument is required (`false`) or optional (`true`).

Required arguments must be present on the command line. If a required argument is missing, `Build` prints help output and throws `InvalidArgumentException`.

Optional arguments keep their initial value if not provided.

### Argument Name and Description

Both `name` and `desc` must be non-empty. Passing an empty name or description throws `InvalidArgumentException`.

```cpp
// Throws InvalidArgumentException
builder.AddArgument("", "Some desc", Opal::Ref{val}, false);
```

## Possible Values

Arguments can be restricted to a predefined set of allowed values. If a value not in the set is provided, `Build` throws `InvalidArgumentException`.

```cpp
Opal::StringUtf8 mode;
Opal::DynamicArray<Opal::StringUtf8> possible_values;
possible_values.PushBack("debug");
possible_values.PushBack("release");
possible_values.PushBack("test");

builder.AddArgument("mode", "Build mode", Opal::Ref{mode}, false, std::move(possible_values));
```

Valid:

```
my_app mode=release
```

Invalid (throws `InvalidArgumentException`):

```
my_app mode=profile
```

### Possible Values with Numeric Types

For integer types, possible values are specified as their native type:

```cpp
Opal::i32 log_level = 0;
Opal::DynamicArray<Opal::i32> possible_values;
possible_values.PushBack(0);
possible_values.PushBack(1);
possible_values.PushBack(2);
possible_values.PushBack(3);

builder.AddArgument("log_level", "Log level", Opal::Ref{log_level}, false, std::move(possible_values));
```

When `possible_values` is empty (the default), any value is accepted.

### Possible Values with Arrays

When used with array types, each element is validated individually.

```cpp
Opal::DynamicArray<Opal::StringUtf8> targets;
Opal::DynamicArray<Opal::StringUtf8> possible_values;
possible_values.PushBack("windows");
possible_values.PushBack("linux");
possible_values.PushBack("macos");

builder.AddArgument("targets", "Build targets", Opal::Ref{targets}, false, std::move(possible_values));
```

Valid:

```
my_app targets=windows,linux
```

Invalid (throws `InvalidArgumentException`):

```
my_app targets=windows,android
```

## Value Mappings

Instead of restricting to a set of raw values, you can provide a mapping from string keys to arbitrary values. The user provides a key on the command line, and the corresponding mapped value is stored. This is the only way to use enum types.

### String Mappings

```cpp
Opal::StringUtf8 output_path;
Opal::HashMap<Opal::StringUtf8, Opal::StringUtf8> mappings({
    {"home", "/home/user"},
    {"tmp", "/tmp"}
});

builder.AddArgument("output", "Output path", Opal::Ref{output_path}, false, std::move(mappings));
```

```
my_app output=home    # output_path == "/home/user"
```

### Enum Arguments

Enum types require a `HashMap<StringUtf8, EnumType>` mapping. Attempting to register an enum argument without mappings throws `InvalidArgumentException`.

```cpp
enum class CppStandard { Cpp11, Cpp14, Cpp17 };

CppStandard standard = CppStandard::Cpp11;
builder.AddArgument("standard", "C++ standard", Opal::Ref{standard}, false,
                    Opal::HashMap<Opal::StringUtf8, CppStandard>({
                        {"c++11", CppStandard::Cpp11},
                        {"c++14", CppStandard::Cpp14},
                        {"c++17", CppStandard::Cpp17}
                    }));
```

```
my_app standard=c++14
```

### Enum Array Arguments

Enum arrays also use mappings:

```cpp
enum class Color { Red, Green, Blue };

Opal::DynamicArray<Color> colors;
builder.AddArgument("colors", "Color values", Opal::Ref{colors}, false,
                    Opal::HashMap<Opal::StringUtf8, Color>({
                        {"red", Color::Red},
                        {"green", Color::Green},
                        {"blue", Color::Blue}
                    }));
```

```
my_app colors=red,blue
```

### Constraints

An argument cannot have both `possible_values` and `possible_value_mappings`. Attempting to provide both throws `InvalidArgumentException`.

## Version

Set a version number with `SetVersion`. Passing `version` or `--version` as any argument prints version info and throws `VersionRequestedException`.

```cpp
builder.SetVersion(1, 2, 3);
```

```
my_app version
my_app --version
```

## Help Output

Passing `help` or `--help` as any argument prints the help text and throws `HelpRequestedException`.

```
my_app help
my_app --help
```

Help output includes the program description, usage examples, and all registered arguments grouped by required/optional. Arguments with possible values or value mappings show the allowed values after the description.

Example output:

```
My application server

Usage examples:
    my_app config_path=server.json port=3000 verbose

Required arguments:
    config_path                   Path to config file
    mode                          Build mode (values: debug, release, test)
    standard                      C++ standard (values: c++11, c++14, c++17)

Optional arguments:
    port                          Server port
    verbose                       Enable verbose output
```

You can set the description and usage examples with:

```cpp
builder.AddProgramDescription("My application server")
       .AddUsageExample("my_app config_path=server.json port=3000 verbose")
       .AddUsageExample("my_app config_path=dev.json");
```

## Error Handling

All error conditions throw exceptions. `Build` returns `void`.

| Scenario | Exception |
|----------|-----------|
| Empty argument name or description | `InvalidArgumentException` at registration |
| Required argument missing | `InvalidArgumentException` (help is printed first) |
| Value not in `possible_values` | `InvalidArgumentException` at parse time |
| Mapping key not found | `InvalidArgumentException` at parse time |
| Enum argument without mappings | `InvalidArgumentException` at registration |
| Both possible values and mappings provided | `InvalidArgumentException` at registration |
| `help` or `--help` passed | `HelpRequestedException` |
| `version` or `--version` passed | `VersionRequestedException` |

## API Reference

### ProgramArgumentsBuilder

| Method | Description |
|--------|-------------|
| `AddProgramDescription(description)` | Set the program description shown in help output |
| `AddUsageExample(example)` | Add a usage example shown in help output |
| `SetVersion(major, minor, patch)` | Set the program version shown for `--version` |
| `AddArgument(name, desc, Ref{dest}, is_optional)` | Register a typed argument bound to `dest` |
| `AddArgument(name, desc, Ref{dest}, is_optional, possible_values)` | Register a typed argument with allowed values |
| `AddArgument(name, desc, Ref{dest}, is_optional, possible_value_mappings)` | Register a typed argument with string-to-value mappings |
| `Build(arguments, count)` | Parse command-line arguments and populate bound variables |

All `AddArgument` overloads return a reference to the builder for method chaining.

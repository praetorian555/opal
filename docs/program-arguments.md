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

    switch (builder.Build(argv, argc))
    {
        case Opal::ProgramArgumentsResult::Success:
            break;
        case Opal::ProgramArgumentsResult::HelpRequested:
        case Opal::ProgramArgumentsResult::VersionRequested:
            return 0;
        default:
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
- `flag` - sets a `bool` argument to `true`; `flag=true` and `flag=false` set it explicitly, and any other value is invalid
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

An integer value must be one whole number that fits its type, in decimal, `0x` hexadecimal or `0` octal; an unsigned
type takes no sign. `port=lots`, `port=80x` and `port=-1` are invalid values, not 0, 80 and a wrapped-around number.

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

Required arguments must be present on the command line. If a required argument is missing, `Build` prints help output and returns `ProgramArgumentsResult::InvalidArgument`.

Optional arguments keep their initial value if not provided.

### Argument Name and Description

Both `name` and `desc` must be non-empty. Passing an empty name or description throws `InvalidArgumentException`.

```cpp
// Throws InvalidArgumentException
builder.AddArgument("", "Some desc", Opal::Ref{val}, false);
```

## Possible Values

Arguments can be restricted to a predefined set of allowed values. If a value not in the set is provided, `Build` returns `ProgramArgumentsResult::InvalidArgument`.

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

Invalid (`Build` returns `ProgramArgumentsResult::InvalidArgument`):

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

Invalid (`Build` returns `ProgramArgumentsResult::InvalidArgument`):

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

## Unknown Arguments

An argument that matches no definition makes `Build` return `ProgramArgumentsResult::InvalidArgument`, with
`GetError()` naming it, so a misspelt option is never silently dropped:

```
my_app verbsoe          # unknown argument 'verbsoe'
```

`AllowUnknownArguments()` ignores such arguments instead.

## Positional Arguments

`AddPositional` collects, in order, every argument that is neither a named one nor starts with `-`, such as file or
target names. An argument containing `=` is positional too when no definition has the name before it, so a URL with a
query string is taken whole.

```cpp
Opal::DynamicArray<Opal::StringUtf8> files;
builder.AddPositional("file", "Files to process", Opal::Ref{files}, false);
```

```
my_app verbose a.txt b.txt      # files == {"a.txt", "b.txt"}
```

The last parameter makes at least one positional value required (`false`) or not (`true`). With a positional argument
defined, only `--help` and `--version` are reserved, so a file may be called `help`.

## Passthrough Arguments

`AddPassthrough` collects every argument after `--` as it is, unparsed, for a program the caller runs on the user's
behalf. Without it, `--` is an unknown argument.

```cpp
Opal::DynamicArray<Opal::StringUtf8> program_arguments;
builder.AddPassthrough("Arguments for the program", Opal::Ref{program_arguments});
```

```
my_app run=game -- --level 1 --help     # program_arguments == {"--level", "1", "--help"}
```

## Prefix Arguments

`AddPrefixArgument` collects every argument that starts with a prefix, without it, and may be repeated. It suits
settings whose name is part of the argument, as `-D` does for compilers:

```cpp
Opal::DynamicArray<Opal::StringUtf8> settings;
builder.AddPrefixArgument("-D", "Option values", Opal::Ref{settings});
```

```
my_app -Dforge=true -Drndr::profile=fast    # settings == {"forge=true", "rndr::profile=fast"}
```

## Commands

A program that does several things, such as `git commit` and `git log`, adds each with `AddCommand`. The first argument
that is not a named one must then be one of them. `Build` parses the arguments before it, stops at the command and
leaves the ones after it to a parser of the command's own, so each command declares only what it takes and prints its
own help:

```cpp
Opal::ProgramArgumentsBuilder program;
program.AddCommand("build", "Build the targets").AddCommand("clean", "Remove what was built");
if (program.Build(argv, argc) != Opal::ProgramArgumentsResult::Success)
{
    return 2;
}
const Opal::u32 index = program.GetCommandIndex();
if (program.GetCommand() == "build")
{
    Opal::u32 jobs = 1;
    Opal::ProgramArgumentsBuilder build;
    build.AddArgument("--jobs", "Jobs at once", Opal::Ref{jobs}, true);
    // The command's name stands where main's program name would.
    const Opal::ProgramArgumentsResult result = build.Build(argv + index, argc - index);
}
```

```
my_app build --jobs=4       # GetCommand() == "build", GetCommandIndex() == 1
my_app --help               # the program's help, which lists the commands
my_app build --help         # the build command's help
my_app biuld                # unknown command 'biuld'
my_app                      # no command given
```

A program with commands has no positional argument.

## Arguments Not From main

`Build(program_name, arguments)` takes the arguments as an `ArrayView<const StringViewUtf8>` without the program's name,
which goes separately and is what `--version` prints. It suits arguments that come from somewhere other than `main`,
such as the ones after a command:

```cpp
const Opal::ArrayView<const Opal::StringViewUtf8> after(arguments.GetData() + index + 1, arguments.GetSize() - index - 1);
const Opal::ProgramArgumentsResult result = build.Build("my_app build", after);
```

## Reporting Errors Yourself

By default `Build` prints the error and the help text through the logger. `SetSilent(true)` keeps it quiet, for a
program that prints in its own format or to its own stream:

- `GetError()` gives the reason for the last `InvalidArgument`: `unknown argument '...'`,
  `value '...' is not valid for argument '...'` with the allowed values when there are some, or
  `required argument '...' not provided`, and for a program with commands `unknown command '...'` or
  `no command given`.
- `GetHelpText()` gives the help text as a string.
- `IsProvided(name)` tells whether the last `Build` was given an argument, by its name, a positional's name or a
  prefix, so a value given can be told from a default.

## Version

Set a version number with `SetVersion`. Passing `version` or `--version` as any argument prints version info and returns `ProgramArgumentsResult::VersionRequested`. With a positional argument defined, only `--version` is reserved.

```cpp
builder.SetVersion(1, 2, 3);
```

```
my_app version
my_app --version
```

## Help Output

Passing `help` or `--help` as any argument prints the help text and returns `ProgramArgumentsResult::HelpRequested`. With a positional argument defined, only `--help` is reserved.

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

`Build` returns a `ProgramArgumentsResult`. Anything the user can type is reported through it, so no parse outcome is
an exception. Defining an argument badly is a programmer error and still throws, at registration.

| Scenario | Result |
|----------|--------|
| Everything parsed | `ProgramArgumentsResult::Success` |
| `help` or `--help` passed | `ProgramArgumentsResult::HelpRequested` (help already printed) |
| `version` or `--version` passed | `ProgramArgumentsResult::VersionRequested` (version already printed) |
| Required argument missing | `ProgramArgumentsResult::InvalidArgument` (help printed first) |
| Argument that matches no definition | `ProgramArgumentsResult::InvalidArgument`, unless `AllowUnknownArguments()` |
| Flag given a value other than `true` or `false` | `ProgramArgumentsResult::InvalidArgument` |
| Value not in `possible_values` | `ProgramArgumentsResult::InvalidArgument` |
| Mapping key not found | `ProgramArgumentsResult::InvalidArgument` |
| Out of memory while parsing | `ProgramArgumentsResult::OutOfMemory` |

| Scenario | Exception |
|----------|-----------|
| Empty argument name or description | `InvalidArgumentException` at registration |
| Enum argument without mappings | `InvalidArgumentException` at registration |
| Both possible values and mappings provided | `InvalidArgumentException` at registration |

`Build` is `[[nodiscard]]`; ignoring the result would silently accept a command line that never parsed.

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
| `AddPositional(name, desc, Ref{dest}, is_optional)` | Collect arguments that are not named ones, in order |
| `AddPassthrough(desc, Ref{dest})` | Collect every argument after `--`, unparsed |
| `AddPrefixArgument(prefix, desc, Ref{dest})` | Collect arguments that start with `prefix`, without it |
| `AddCommand(name, desc)` | Add a command; the parse stops at it |
| `AllowUnknownArguments()` | Ignore arguments that match no definition instead of failing |
| `SetSilent(silent)` | Keep `Build` from printing help, version and errors |
| `Build(arguments, count)` | Parse command-line arguments and populate bound variables, returns `ProgramArgumentsResult` |
| `Build(program_name, arguments)` | Parse arguments given as string views, without the program's name |
| `GetCommand()` | The command the last `Build` found |
| `GetCommandIndex()` | Where the last `Build` found the command in its arguments |
| `GetError()` | Why the last `Build` returned `InvalidArgument` |
| `GetHelpText()` | The help text, as a string |
| `IsProvided(name)` | Whether the last `Build` was given the argument |

Every `Add` and `Set` method returns a reference to the builder for method chaining.

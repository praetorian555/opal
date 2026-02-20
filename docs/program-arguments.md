# Program Arguments

Header: `opal/program-arguments.h`

A typed command-line argument parser that binds arguments directly to C++ variables. Arguments use `name=value` syntax, supports required and optional arguments, value validation, and auto-generated help output.

## Quick Start

```cpp
#include "opal/program-arguments.h"

int main(int argc, const char** argv)
{
    Opal::StringUtf8 config_path;
    Opal::i32 port = 8080;
    bool verbose = false;

    Opal::ProgramArgumentsBuilder builder;
    builder.AddProgramDescription("My application server");
    builder.AddUsageExample("my_app config_path=server.json port=3000 verbose");
    builder.AddArgumentDefinition(config_path, {.name = "config_path", .desc = "Path to config file", .is_optional = false});
    builder.AddArgumentDefinition(port, {.name = "port", .desc = "Server port", .is_optional = true});
    builder.AddArgumentDefinition(verbose, {.name = "verbose", .desc = "Enable verbose output", .is_optional = true});

    if (!builder.Build(argv, argc))
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
| `StringUtf8` | `name=value` | `mode=release` |
| `i32` | `name=value` | `count=-5` |
| `u32` | `name=value` | `port=8080` |
| `bool` | `name` | `verbose` |
| `DynamicArray<StringUtf8>` | `name=a,b,c` | `tags=fast,safe,new` |
| `DynamicArray<i32>` | `name=a,b,c` | `offsets=-2,5,10` |
| `DynamicArray<u32>` | `name=a,b,c` | `ids=6,8,12` |

String values can be quoted to include spaces:

```
my_app message="Hello there"
my_app names="Alice,Bob,Charlie"
```

## Defining Arguments

Use `ProgramArgumentsBuilder` to define arguments and parse the command line. Each argument is bound to a variable by reference.

```cpp
Opal::i32 count = 0;
Opal::StringUtf8 name;
bool debug = false;

Opal::ProgramArgumentsBuilder builder;
builder.AddArgumentDefinition(count, {.name = "count", .desc = "Number of iterations", .is_optional = false});
builder.AddArgumentDefinition(name, {.name = "name", .desc = "User name", .is_optional = true});
builder.AddArgumentDefinition(debug, {.name = "debug", .desc = "Enable debug mode", .is_optional = true});
```

### Required vs Optional

Required arguments must be present on the command line. If a required argument is missing, `Build` prints a message with the help output and returns `false`.

Optional arguments keep their initial value if not provided.

### Argument Name and Description

Both `name` and `desc` must be non-empty. Passing an empty name or description throws `InvalidArgumentException`.

```cpp
// Throws InvalidArgumentException
builder.AddArgumentDefinition(val, {.name = "", .desc = "Some desc", .is_optional = false});
```

## Possible Values

Arguments can be restricted to a predefined set of allowed values. If a value not in the set is provided, `Build` throws `InvalidArgumentException`.

```cpp
Opal::StringUtf8 mode;

builder.AddArgumentDefinition(mode, {
    .name = "mode",
    .desc = "Build mode",
    .is_optional = false,
    .possible_values = {"debug", "release", "test"}
});
```

Valid:

```
my_app mode=release
```

Invalid (throws `InvalidArgumentException`):

```
my_app mode=profile
```

### Possible Values with Arrays

When used with array types, each element is validated individually.

```cpp
Opal::DynamicArray<Opal::StringUtf8> targets;

builder.AddArgumentDefinition(targets, {
    .name = "targets",
    .desc = "Build targets",
    .is_optional = false,
    .possible_values = {"windows", "linux", "macos"}
});
```

Valid:

```
my_app targets=windows,linux
```

Invalid (throws `InvalidArgumentException`):

```
my_app targets=windows,android
```

### Possible Values with Numeric Types

For `i32` and `u32` types, possible values are specified as strings that match the raw input.

```cpp
Opal::i32 log_level = 0;

builder.AddArgumentDefinition(log_level, {
    .name = "log_level",
    .desc = "Log level",
    .is_optional = false,
    .possible_values = {"0", "1", "2", "3"}
});
```

When `possible_values` is empty (the default), any value is accepted.

## Help Output

Passing `help` or `--help` as any argument prints the help text and causes `Build` to return `false`.

```
my_app help
my_app --help
```

Help output includes the program description, usage examples, and all registered arguments grouped by required/optional. Arguments with `possible_values` show the allowed values after the description.

Example output:

```
My application server

Usage examples:
    my_app config_path=server.json port=3000 verbose

Required arguments:
    config_path                   Path to config file
    mode                          Build mode (values: debug, release, test)

Optional arguments:
    port                          Server port
    verbose                       Enable verbose output
```

You can set the description and usage examples with:

```cpp
builder.AddProgramDescription("My application server");
builder.AddUsageExample("my_app config_path=server.json port=3000 verbose");
builder.AddUsageExample("my_app config_path=dev.json");
```

## Error Handling

| Scenario | Behavior |
|----------|----------|
| Empty argument name or description | Throws `InvalidArgumentException` at registration |
| Required argument missing | Prints help and returns `false` |
| Value not in `possible_values` | Throws `InvalidArgumentException` at parse time |
| Unsupported type `T` | Throws `NotImplementedException` at parse time |
| `help` or `--help` passed | Prints help and returns `false` |

## API Reference

### ProgramArgumentDefinitionDesc

```cpp
struct ProgramArgumentDefinitionDesc
{
    StringUtf8 name;
    StringUtf8 desc;
    bool is_optional;
    DynamicArray<StringUtf8> possible_values;
};
```

### ProgramArgumentsBuilder

| Method | Description |
|--------|-------------|
| `AddProgramDescription(description)` | Set the program description shown in help output |
| `AddUsageExample(example)` | Add a usage example shown in help output |
| `AddArgumentDefinition(field, desc)` | Register a typed argument bound to `field` |
| `Build(arguments, count)` | Parse command-line arguments and populate bound variables |

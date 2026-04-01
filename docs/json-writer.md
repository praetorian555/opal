# JSON Writer

Header: `opal/container/json-writer.h`

Serializes `JsonValue` trees to JSON text. Supports both compact and pretty-printed output with configurable indentation. Symmetric counterpart to `JsonReader`.

## Quick Start

```cpp
#include "opal/container/json-reader.h"
#include "opal/container/json-writer.h"

// Parse JSON, modify, serialize back
Opal::JsonReader reader = Opal::JsonReader::Parse(R"({"name":"Alice","score":100})");
Opal::StringUtf8 json = Opal::JsonWriter::Serialize(reader.GetRoot());
// json == {"name":"Alice","score":100}
```

## Building JSON Programmatically

Use `JsonValue` factory methods to construct a JSON tree in code, then serialize it:

```cpp
#include "opal/container/json-reader.h"
#include "opal/container/json-writer.h"

Opal::StringUtf8 name("Alice");

Opal::JsonValue scores = Opal::JsonValue::MakeArray();
scores.PushBack(Opal::JsonValue::MakeNumber(90));
scores.PushBack(Opal::JsonValue::MakeNumber(85));

Opal::JsonValue root = Opal::JsonValue::MakeObject();
root.Insert(name, Opal::JsonValue::MakeString(name));
root.Insert(Opal::StringUtf8("score"), Opal::JsonValue::MakeNumber(100));
root.Insert(Opal::StringUtf8("active"), Opal::JsonValue::MakeBool(true));
root.Insert(Opal::StringUtf8("scores"), std::move(scores));

Opal::StringUtf8 json = Opal::JsonWriter::Serialize(root);
// {"name":"Alice","score":100,"active":true,"scores":[90,85]}
```

### Factory Methods

| Method | Description |
|--------|-------------|
| `JsonValue::MakeNull()` | Null value |
| `JsonValue::MakeBool(bool)` | Boolean value |
| `JsonValue::MakeNumber(f64)` | Number from double |
| `JsonValue::MakeNumber(T)` | Number from any integral or floating-point type |
| `JsonValue::MakeString(StringViewUtf8)` | String value (caller must keep the string data alive) |
| `JsonValue::MakeArray(allocator)` | Empty array |
| `JsonValue::MakeObject(allocator)` | Empty object |

### Mutation Methods

| Method | Description |
|--------|-------------|
| `PushBack(JsonValue&&)` | Append an element to an array. Throws `JsonTypeMismatchException` if not an array. |
| `Insert(StringViewUtf8, JsonValue&&)` | Add a key-value pair to an object. Throws `JsonTypeMismatchException` if not an object. |

`MakeArray` and `MakeObject` accept an optional `AllocatorBase*` (defaults to the default allocator).

## Compact Output

`Serialize` with no options produces compact JSON with no whitespace:

```cpp
Opal::StringUtf8 json = Opal::JsonWriter::Serialize(value);
// [1,2,3]
// {"key":"value"}
```

Both overloads accept an optional `AllocatorBase*` for the output string. If `nullptr`, the default allocator is used.

```cpp
Opal::StringUtf8 json = Opal::JsonWriter::Serialize(value, &my_allocator);
```

## Pretty Print

Pass `JsonWriteOptions` to control formatting:

```cpp
Opal::JsonWriteOptions options;
options.pretty = true;
options.indent_width = 4;  // Default

Opal::StringUtf8 json = Opal::JsonWriter::Serialize(value, options);
```

Output:

```json
{
    "name": "Alice",
    "items": [
        1,
        2,
        3
    ]
}
```

### Options

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `pretty` | `bool` | `false` | Enable pretty printing with newlines and indentation |
| `indent_width` | `u32` | `4` | Number of spaces per indent level (ignored when `use_tabs` is true) |
| `use_tabs` | `bool` | `false` | Use tabs instead of spaces for indentation |

Empty arrays and objects stay compact even in pretty mode:

```cpp
// [] not [\n]
// {} not {\n}
```

### Tabs

```cpp
Opal::JsonWriteOptions options;
options.pretty = true;
options.use_tabs = true;

Opal::StringUtf8 json = Opal::JsonWriter::Serialize(value, options);
```

## String Escaping

All required JSON escape sequences are produced:

| Character | Output |
|-----------|--------|
| `"` | `\"` |
| `\` | `\\` |
| Backspace | `\b` |
| Form feed | `\f` |
| Newline | `\n` |
| Carriage return | `\r` |
| Tab | `\t` |
| Control chars (0x00-0x1F) | `\u00XX` |

Forward slash `/` is not escaped (optional per spec).

## Number Formatting

Numbers are formatted with 17 significant digits (`{:.17g}`) for exact `f64` round-trip fidelity. Trailing zeros are suppressed (e.g., `42.0` serializes as `42`).

NaN and Infinity are not valid JSON. Attempting to serialize them throws `JsonSerializeException`.

```cpp
Opal::JsonValue nan_value(static_cast<Opal::f64>(NAN));
Opal::JsonWriter::Serialize(nan_value);  // Throws JsonSerializeException
```

## Round-Trip

`JsonWriter` is designed to round-trip with `JsonReader`:

```cpp
Opal::StringUtf8 input(R"({"name":"Alice","score":100})");
Opal::JsonReader reader = Opal::JsonReader::Parse(input);
Opal::StringUtf8 output = Opal::JsonWriter::Serialize(reader.GetRoot());
Opal::JsonReader reader2 = Opal::JsonReader::Parse(output);

// reader2.GetRoot()["name"].GetString() == "Alice"
// reader2.GetRoot()["score"].GetNumberAs<Opal::i32>() == 100
```

Note that object key order may differ between input and output since `HashMap` does not preserve insertion order.

## Exceptions

| Exception | When |
|-----------|------|
| `JsonSerializeException` | Value contains NaN or Infinity |

## API Reference

### JsonWriteOptions

```cpp
struct JsonWriteOptions
{
    bool pretty = false;
    u32 indent_width = 4;
    bool use_tabs = false;
};
```

### JsonWriter

| Method | Description |
|--------|-------------|
| `Serialize(value, allocator)` | Serialize to compact JSON |
| `Serialize(value, options, allocator)` | Serialize with formatting options |

Both return `StringUtf8`. The `allocator` parameter defaults to `nullptr` (uses the default allocator).

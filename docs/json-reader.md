# JSON Reader

Header: `opal/container/json-reader.h`

Read-only JSON parser that produces an immutable tree of `JsonValue` nodes. Strings are stored as views into the original input where possible, avoiding copies. Supports all JSON types, escape sequences (including `\uXXXX` with surrogate pairs), dot-path access, range-for iteration, and pattern matching via `Visit`/`VisitPartial`.

## Quick Start

```cpp
#include "opal/container/json-reader.h"

// Parse a JSON string
Opal::JsonReader reader = Opal::JsonReader::Parse(R"({
    "name": "Alice",
    "score": 100,
    "items": [1, 2, 3]
})");

const Opal::JsonValue& root = reader.GetRoot();
Opal::StringViewUtf8 name = root["name"].GetString();
Opal::i32 score = root["score"].GetNumberAs<Opal::i32>();
```

---

## Parsing

`JsonReader::Parse` is the only way to create a reader. It comes in two overloads:

```cpp
// Borrow input — caller must keep the string alive as long as the reader exists.
Opal::JsonReader reader = Opal::JsonReader::Parse(my_string);

// Take ownership — the reader moves the string into itself.
Opal::JsonReader reader = Opal::JsonReader::Parse(std::move(my_string));
```

Both overloads accept an optional `AllocatorBase*` for all internal allocations. If `nullptr`, the default allocator is used.

```cpp
Opal::JsonReader reader = Opal::JsonReader::Parse(input, &my_allocator);
```

Throws `JsonParseException` on malformed input. The exception carries `line`, `column`, and `byte_offset` fields for diagnostics.

```cpp
try
{
    Opal::JsonReader reader = Opal::JsonReader::Parse("{ invalid }");
}
catch (const Opal::JsonParseException& e)
{
    // e.line, e.column, e.byte_offset
}
```

## Accessing the Root

```cpp
const Opal::JsonValue& root = reader.GetRoot();
```

The root can be any JSON type: null, bool, number, string, array, or object.

## JsonValue

### Type Queries

```cpp
root.GetType();          // Returns JsonType enum
root.IsNull();
root.IsBool();
root.IsNumber();         // True for both f64 and i64
root.IsIntegerNumber();  // True only for i64
root.IsString();
root.IsArray();
root.IsObject();
```

`JsonType` values: `Null`, `Bool`, `Number`, `String`, `Array`, `Object`.

### Value Extraction

All extraction methods throw `JsonTypeMismatchException` on type mismatch.

```cpp
bool b = root.GetBool();
Opal::f64 n = root.GetNumber();          // Returns f64 for both f64 and i64 values
Opal::i64 i = root.GetIntegerNumber();   // Returns i64, throws if not an integer
Opal::StringViewUtf8 s = root.GetString();
```

#### Number Storage

The parser stores JSON numbers in two internal representations:

- **`i64`** for integer literals (no `.` or `e`/`E`) that fit in 64-bit signed range.
- **`f64`** for numbers with fractional parts, exponents, or integers that overflow `i64`.

`GetNumber()` always returns `f64`, converting from `i64` if needed (lossy for values beyond 2^53). `GetIntegerNumber()` returns `i64` directly and throws if the value is stored as `f64`.

#### GetNumberAs

`GetNumberAs<T>()` converts the number via `Opal::Narrow`, supporting any integral or floating-point type. When the value is stored as `i64`, it narrows directly from `i64` without going through `f64`, preserving full precision for large integers:

```cpp
Opal::i32 x = root.GetNumberAs<Opal::i32>();
Opal::u64 y = root.GetNumberAs<Opal::u64>();
Opal::i64 hash = root.GetNumberAs<Opal::i64>();  // Exact for integers up to i64 max
```

### Element Access

Access array elements by index and object values by key:

```cpp
const Opal::JsonValue& first = root[0];            // Array index
const Opal::JsonValue& name = root["name"];         // Object key
```

Throws `JsonTypeMismatchException` if the value is not the expected container type. Throws `OutOfBoundsException` for invalid array indices and `InvalidArgumentException` for missing keys.

### Dot-Path Access

Navigate nested structures with a dot-separated path. Numeric segments are treated as array indices when the current value is an array:

```cpp
// Equivalent to root["players"][0]["name"]
const Opal::JsonValue& name = root.GetPath("players.0.name");
```

### Container Size

```cpp
Opal::u64 size = root.GetSize();    // Works on arrays and objects
```

Throws `JsonTypeMismatchException` if the value is not an array or object.

## Iteration

### Array Iteration

`JsonValue` provides `begin()`/`end()` for direct range-for on arrays:

```cpp
Opal::JsonReader reader = Opal::JsonReader::Parse("[10, 20, 30]");
for (const Opal::JsonValue& elem : reader.GetRoot())
{
    Opal::f64 n = elem.GetNumber();
}
```

Throws `JsonTypeMismatchException` if the value is not an array.

### Object Iteration

Use `Items()` to iterate over key-value pairs of an object:

```cpp
for (auto [key, val] : root.Items())
{
    // key is StringViewUtf8, val is const JsonValue&
}
```

Throws `JsonTypeMismatchException` if the value is not an object.

## Pattern Matching

### Visit

`Visit` requires handlers for all variant types, including both `f64` and `i64`:

```cpp
root.Visit(Opal::Overloaded{
    [](const Opal::JsonNull&)         { /* null */ },
    [](bool b)                        { /* bool */ },
    [](Opal::f64 n)                   { /* floating-point number */ },
    [](Opal::i64 n)                   { /* integer number */ },
    [](const Opal::StringViewUtf8& s) { /* string */ },
    [](const Opal::JsonArray& arr)    { /* array */ },
    [](const Opal::JsonObject& obj)   { /* object */ },
});
```

### VisitPartial

`VisitPartial` handles only the types you care about; unmatched types are silently ignored:

```cpp
// Handle all numbers regardless of internal representation
root.VisitPartial(Opal::Overloaded{
    [](Opal::f64 n) { /* floating-point numbers */ },
    [](Opal::i64 n) { /* integer numbers */ },
});
```

## Clone

`JsonValue` is move-only. Use `Clone()` to deep copy a value tree:

```cpp
Opal::JsonValue copy = root.Clone();
Opal::JsonValue copy2 = root.Clone(&my_allocator);
```

## Type Aliases

| Type | Definition |
|------|-----------|
| `JsonArray` | `SharedPtr<DynamicArray<JsonValue>, ThreadingPolicy::SingleThread>` |
| `JsonObject` | `SharedPtr<HashMap<StringViewUtf8, JsonValue>, ThreadingPolicy::SingleThread>` |

Both use `SingleThread` policy since JSON values are not designed for concurrent access.

## Exceptions

| Exception | When |
|-----------|------|
| `JsonParseException` | Malformed JSON input. Fields: `line`, `column`, `byte_offset`. |
| `JsonTypeMismatchException` | Accessing a value as the wrong type. |
| `OutOfBoundsException` | Array index out of range. |
| `InvalidArgumentException` | Object key not found. |
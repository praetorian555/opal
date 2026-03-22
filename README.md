# Opal

A modern C++20 utility library providing data structures, math primitives, threading utilities, and more. Designed for game engine and real-time systems development with pluggable memory allocation throughout.

## Requirements

- **C++20** compiler (MSVC, Clang, or GCC)
- **CMake 3.28+**
- **Platforms:** Windows, Linux

No external dependencies. Third-party libraries (Catch2, wyhash) are vendored.

## Building

```bash
cmake -B build
cmake --build build
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `OPAL_BUILD_TESTS` | `ON` | Build the test suite |
| `OPAL_HARDENING` | `ON` | Enable sanitizers (address, undefined behavior) |
| `OPAL_SHARED_LIBS` | `OFF` | Build as shared library instead of static |

Example with custom options:

```bash
cmake -B build -DOPAL_BUILD_TESTS=OFF -DOPAL_SHARED_LIBS=ON
cmake --build build
```

### Running Tests

```bash
cd build
ctest
```

## Integration

### As a CMake Subdirectory

```cmake
add_subdirectory(path/to/opal)
target_link_libraries(your_target PRIVATE opal)
```

### After Install

```cmake
find_package(opal REQUIRED)
target_link_libraries(your_target PRIVATE opal)
```

## Features

All public API lives in the `Opal` namespace.

### Clone API

Many Opal types delete their copy constructor and copy assignment operator. This forces the user to be explicit about whether they want to transfer ownership via `std::move` or create a deep copy via `Clone`. This design prevents accidental expensive copies and makes the intent clear at the call site.

All containers and resource-owning types follow this pattern, including `DynamicArray`, `Deque`, `String`, `HashMap`, `HashSet`, `InPlaceArray`, `SharedPtr`, `Variant`, `Ref`, `ScopePtr`, and any type derived from `ClonableBase`.

```cpp
Opal::DynamicArray<int> original;
original.PushBack(1);
original.PushBack(2);

// Transfer ownership (no allocation, original is left in a moved-from state)
Opal::DynamicArray<int> moved = std::move(original);

// Deep copy (allocates new memory and copies elements)
Opal::DynamicArray<int> cloned = moved.Clone();

// Deep copy with a specific allocator
Opal::DynamicArray<int> cloned2 = moved.Clone(&my_allocator);
```

The free function `Opal::Clone` dispatches based on type: POD types are copied directly, non-POD types call `.Clone(allocator)`.

```cpp
#include "opal/common.h"

int x = Opal::Clone(42);                          // POD, direct copy
Opal::DynamicArray<int> y = Opal::Clone(moved);   // Calls moved.Clone()
```

For user-defined types, inherit from `ClonableBase` and use the `OPAL_CLONE_FIELDS` macro to get automatic deep cloning with deleted copy operations.

```cpp
#include "opal/clonable-base.h"

struct Player : Opal::ClonableBase<Player>
{
    Opal::StringUtf8 name;
    Opal::i32 score = 0;

    OPAL_CLONE_FIELDS(name, score);
};

Player p;
p.name = "Alice";
p.score = 100;

// Player p2 = p;               // Compile error: copy constructor is deleted
Player p2 = p.Clone();          // Deep clone with default allocator
Player p3 = p.Clone(&alloc);    // Deep clone with custom allocator
Player p4 = std::move(p);       // Move, p is now in a moved-from state
```

### Memory Management

Pluggable allocator architecture with thread-local allocator stacks and RAII guards.

| Allocator | Description |
|-----------|-------------|
| `MallocAllocator` | General-purpose heap allocator, thread-safe |
| `LinearAllocator` | Bump/arena allocator with mark/reset, not thread-safe |
| `SystemMemoryAllocator` | Virtual memory allocator (VirtualAlloc/mmap), not thread-safe |
| `NullAllocator` | Always throws, used for moved-from state |

```cpp
#include "opal/allocator.h"

// Default allocator is implicitly set to MallocAllocator
Opal::DynamicArray<int> arr;

// Use a linear allocator for temporary work
Opal::LinearAllocator arena("Temp", {.bytes_to_reserve = OPAL_MB(64)});
{
    Opal::PushDefault guard(&arena);
    // All containers created here use the arena
}
```

See [docs/memory.md](docs/memory.md) for the full guide.

### Logging

Lightweight logging system with pluggable sinks, per-category level filtering, and configurable output patterns.

```cpp
#include "opal/logging.h"

Opal::GetLogger().Info("General", "Player {} joined", player_name);
```

See [docs/logging.md](docs/logging.md) for the full guide.

### Program Arguments

Typed command-line argument parser that binds arguments directly to C++ variables.

```cpp
#include "opal/program-arguments.h"

Opal::i32 port = 8080;
Opal::ProgramArgumentsBuilder builder;
builder.AddArgument("port", "Server port", Opal::Ref{port}, true);
builder.Build(argv, argc);
```

See [docs/program-arguments.md](docs/program-arguments.md) for the full guide.

### Containers

| Header | Description |
|--------|-------------|
| `opal/container/dynamic-array.h` | Variable-size array with allocator support |
| `opal/container/deque.h` | Double-ended queue |
| `opal/container/array-view.h` | Non-owning view over contiguous memory |
| `opal/container/in-place-array.h` | Fixed-capacity array with stack storage |
| `opal/container/string.h` | Full-featured UTF-8 string |
| `opal/container/string-view.h` | Non-owning UTF-8 string view |
| `opal/container/string-encoding.h` | UTF-8, UTF-16, and UTF-32 encoding support |
| `opal/container/string-hash.h` | String hashing utilities |
| `opal/container/hash-map.h` | Hash map using Swiss tables algorithm |
| `opal/container/hash-set.h` | Hash set using Swiss tables algorithm |
| `opal/container/priority-queue.h` | Priority queue |
| `opal/container/expected.h` | Result type `Expected<T, E>` for error handling |
| `opal/container/scope-ptr.h` | Unique ownership smart pointer |
| `opal/container/shared-ptr.h` | Reference-counted smart pointer with thread-safe and single-threaded policies |
| `opal/container/ref.h` | Reference wrapper with Clone support |
| `opal/container/iterator.h` | Iterator utilities |

Every container accepts an optional `AllocatorBase*` parameter. When omitted, the current default allocator is used.

```cpp
#include "opal/container/dynamic-array.h"

Opal::DynamicArray<int> arr;           // Uses default allocator
Opal::DynamicArray<int> arr(&alloc);   // Uses explicit allocator
```

See [docs/containers.md](docs/containers.md) for the full guide.

### Math

Linear algebra types for 2D/3D applications.

| Header | Description |
|--------|-------------|
| `opal/math/vector2.h` - `vector4.h` | Vector types (2, 3, 4 components) |
| `opal/math/point2.h` - `point4.h` | Point types (2, 3, 4 components) |
| `opal/math/normal3.h` | Surface normal |
| `opal/math/bounds2.h`, `bounds3.h` | Axis-aligned bounding boxes |
| `opal/math/matrix.h` | 4x4 matrix |
| `opal/math/quaternion.h` | Quaternion rotation |
| `opal/math/dual-quaternion.h` | Dual quaternion for rigid transformations |
| `opal/math/rotator.h` | Euler angle rotation |
| `opal/math/transform.h` | Combined translation, rotation, and scale |
| `opal/math-base.h` | Math functions and vector operations |
| `opal/math-constants.h` | Common mathematical constants |

```cpp
#include "opal/math/vector3.h"
#include "opal/math/quaternion.h"

Opal::Vector3f direction = {1.0f, 0.0f, 0.0f};
Opal::Quaternionf rotation = Opal::Quaternionf::FromAxisAngle({0, 1, 0}, Opal::kPi / 2);
```

### Threading

Cross-platform threading primitives and concurrency utilities.

| Header | Description |
|--------|-------------|
| `opal/threading/thread.h` | Thread wrapper |
| `opal/threading/mutex.h` | Mutex with value-based RAII locking |
| `opal/threading/condition-variable.h` | Condition variable |
| `opal/threading/channel-spsc.h` | Single-producer, single-consumer channel |
| `opal/threading/channel-mpmc.h` | Multi-producer, multi-consumer channel |
| `opal/threading/thread-pool.h` | Task-based thread pool |

```cpp
#include "opal/threading/mutex.h"

Opal::Mutex<int> counter(0);
{
    auto guard = counter.Lock();
    *guard.Deref() += 1;
}
```

See [docs/threading.md](docs/threading.md) for the full guide.

### Sorting

| Header | Description |
|--------|-------------|
| `opal/sort/insertion-sort.h` | Insertion sort |
| `opal/sort/heap-sort.h` | Heap sort |
| `opal/sort/merge-sort.h` | Merge sort |
| `opal/sort/quick-sort.h` | Quicksort |
| `opal/sort/key-indexed-counting.h` | Key-indexed counting sort (stable, linear) |

### Delegates

Single-cast and multi-cast function delegates for event-driven patterns.

`Delegate<ReturnType(Args...)>` binds a single callable. Executing an unbound delegate returns a default-constructed value.

```cpp
#include "opal/delegate.h"

Opal::Delegate<int(int, int)> on_add;
on_add.Bind([](int a, int b) { return a + b; });
int result = on_add.Execute(3, 4);  // 7
on_add.Unbind();
```

`MultiDelegate<void(Args...)>` binds multiple callables, each identified by a handle for later removal. It accepts an optional `AllocatorBase*` for the internal storage.

```cpp
Opal::MultiDelegate<void(float)> on_damage;            // Uses default allocator
Opal::MultiDelegate<void(float)> on_damage(&allocator); // Uses explicit allocator

Opal::DelegateHandle h1 = on_damage.Bind([](float dmg) { /* update health bar */ });
Opal::DelegateHandle h2 = on_damage.Bind([](float dmg) { /* play sound */ });

on_damage.Execute(25.0f);  // Invokes both callbacks
on_damage.Unbind(h1);      // Remove first callback
on_damage.Execute(10.0f);  // Invokes only second callback
```

### Variant

Type-safe discriminated union supporting multiple alternative types with visit and pattern matching.

```cpp
#include "opal/variant.h"

Opal::Variant<int, float, Opal::StringUtf8> value(42);
Opal::Visit(value, [](const auto& v) { /* handle each type */ });
```

### Other Utilities

| Header | Description |
|--------|-------------|
| `opal/casts.h` | Checked narrow cast for integral and floating-point conversions |
| `opal/type-traits.h` | Compile-time type traits and C++20 concepts |
| `opal/common.h` | `Swap`, `Clone`, `GetArraySize` |
| `opal/clonable-base.h` | CRTP base for automatic deep cloning via `OPAL_CLONE_FIELDS` macro |
| `opal/exceptions.h` | Exception hierarchy |
| `opal/error-codes.h` | Error code enumeration |
| `opal/enum-flags.h` | `OPAL_ENUM_CLASS_FLAGS` macro for type-safe enum bitmasks |
| `opal/time.h` | Time measurement utilities |
| `opal/paths.h` | Cross-platform path manipulation |
| `opal/file-system.h` | File and directory operations |
| `opal/bit.h` | Bitwise utility functions |
| `opal/hash.h` | Hash functions (wyhash) |
| `opal/rng.h` | Random number generation |
| `opal/source-location.h` | Source code location information |

### Narrow Cast

`Opal::Narrow` is the preferred way to cast between integral and floating-point types instead of `static_cast`. In debug builds, it validates that the conversion is lossless by round-tripping the value and throws `Exception` if data is lost. In release builds, it compiles down to a plain `static_cast`.

```cpp
#include "opal/casts.h"

Opal::i32 big = 300;
Opal::u8 small = Opal::Narrow<Opal::u8>(big);   // Throws in debug (300 > 255)

Opal::f64 pi = 3.14159265358979;
Opal::f32 pi_f = Opal::Narrow<Opal::f32>(pi);   // OK, no precision loss in this case

Opal::i64 value = 42;
Opal::i32 safe = Opal::Narrow<Opal::i32>(value); // OK, 42 fits in i32
```

## License

[MIT](LICENSE) - Copyright (c) 2023 Marko Kostić
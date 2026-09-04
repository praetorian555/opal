# Opal - Claude Code Guide

## Project Overview

C++20 utility library for game engines and real-time systems. Pluggable memory allocation throughout. No external dependencies in the default build (Catch2 and rapidhash vendored in `third-party/`; Google Benchmark is fetched only when `OPAL_BUILD_BENCHMARKS` is on).

**Namespace:** All public API lives in `Opal`.

**Platforms:** Windows, Linux. Cross-platform via `#if defined(OPAL_PLATFORM_WINDOWS)` / `#elif defined(OPAL_PLATFORM_LINUX)`.

## Build & Test

```bash
cmake -B build
cmake --build build
cd build && ctest
```

CMake options: `OPAL_BUILD_TESTS` (ON), `OPAL_HARDENING` (ON), `OPAL_EXCEPTIONS` (ON), `OPAL_BUILD_BENCHMARKS` (OFF).

Opal is a static library. There is no shared build and no export annotations: symbols are whatever the archive holds.

The test target uses a precompiled header (`test/test-helpers.h`), which is most of what a test file parses. It matters on a
machine with few cores - a `-j4` clean build went from 47s to 25s - and is close to free on a machine with many, where the
build is bound by the slowest translation unit rather than by total work. Disable it with
`-DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON` when bisecting a compile error, since a PCH moves where the error is reported.

**Never start a header with a UTF-8 BOM.** GCC's `#pragma once` does not recognise a second inclusion of a file that begins
with one once the file is also inside a precompiled header, so the header is parsed twice and every definition in it collides.
Four headers had one and had to be stripped before the PCH would build. MSVC does not care, so this only shows up on GCC.

`OPAL_EXCEPTIONS=OFF` adds `-fno-exceptions` (`/EHs-c-` on MSVC) and defines `OPAL_NO_EXCEPTIONS`, both `PUBLIC` so a consumer
cannot disagree with the library it links. `OPAL_RAISE` then ends the program through the contract violation handler instead of
throwing, and the `Create` factories are how a caller on a budgeted allocator reports a failed allocation instead. It also forces
`OPAL_BUILD_TESTS` off, since Catch2 needs exceptions. The whole public API is available either way, `JsonReader::Parse`
included: the parser reports a failed parse by recording it on itself and returning, not by unwinding. Nothing checks this
configuration automatically, so build it by hand after touching a `throw`:

```bash
wsl.exe -d Ubuntu-24.04 -e bash -lc 'cd /mnt/d/Dev/opal &&
  cmake -S . -B build/noexcept-wsl-gcc -DCMAKE_BUILD_TYPE=Debug -DOPAL_HARDENING=OFF -DOPAL_EXCEPTIONS=OFF &&
  cmake --build build/noexcept-wsl-gcc -j "$(nproc)"'
```

### Building with GCC and Clang from Windows

MSVC alone does not exercise the whole library. Two classes of bug are invisible to it:

- `size_t` is `unsigned long long` on Windows, the same type as `u64`, and `unsigned long` everywhere
  else. An overload set taking both `u64` and `i64` resolves on MSVC and is ambiguous on Linux.
- A Debug MSVC build tends to zero padding bytes, hiding code that reads them. Release and other
  compilers do not.

Both GCC and Clang are available through WSL and can be driven from a Windows shell, so check there
before pushing anything that touches templates, overloads or raw bytes:

```bash
# GCC. CLion generates this directory with Makefiles, so do not pass -G Ninja.
wsl.exe -d Ubuntu-24.04 -e bash -lc 'cd /mnt/d/Dev/opal &&
  cmake -S . -B build/debug-wsl-gcc -DCMAKE_BUILD_TYPE=Debug -DOPAL_HARDENING=ON &&
  cmake --build build/debug-wsl-gcc -j "$(nproc)" &&
  cd build/debug-wsl-gcc && ctest --output-on-failure'

# Clang.
wsl.exe -d Ubuntu-24.04 -e bash -lc 'cd /mnt/d/Dev/opal &&
  cmake -S . -B build/debug-wsl-clang -G Ninja -DCMAKE_BUILD_TYPE=Debug -DOPAL_HARDENING=ON \
    -DCMAKE_C_COMPILER=/bin/clang-20 -DCMAKE_CXX_COMPILER=/bin/clang++-20 &&
  cmake --build build/debug-wsl-clang -j "$(nproc)" &&
  cd build/debug-wsl-clang && ctest --output-on-failure'
```

`OPAL_HARDENING=ON` turns on ASan and UBSan, which is what the two builds above run under. CI has a
`sanitizers` job covering the same thing on GCC and Clang, with `UBSAN_OPTIONS=halt_on_error=1` so a
UBSan finding fails the run rather than being printed into a green log. Every other CI job passes
`-DOPAL_HARDENING=OFF`. Still run the local build for anything touching lifetimes or threading -
finding it before the push is cheaper than reading it off a CI log.

The build directories match the CLion profiles of the same name and are gitignored.

Test framework: Catch2 (amalgamated, vendored). Custom `main` in `test/main-test.cpp`. Test helpers in `test/test-helpers.h` (custom Catch matchers for math types). When adding a new test file, it must be added to `OPAL_TEST_FILES` in `CMakeLists.txt`.

When adding a new source/header file, it must be added to `OPAL_FILES` in `CMakeLists.txt`.

Benchmarks: Google Benchmark, fetched with FetchContent when `OPAL_BUILD_BENCHMARKS=ON`. Sources in `benchmark/*-benchmark.cpp`, target
`opal_benchmark`. When adding a new benchmark file, it must be added to `OPAL_BENCHMARK_FILES` in `CMakeLists.txt`. Build them Release
with `OPAL_HARDENING=OFF`; see `docs/benchmarks.md` for running and comparing.

## Directory Layout

```
include/opal/              Public headers
include/opal/container/    Container types (dynamic-array, string, hash-map, etc.)
include/opal/math/         Math types (vector, matrix, quaternion, etc.)
include/opal/threading/    Threading primitives (mutex, channel, thread-pool, etc.)
include/opal/sort/         Sorting algorithms
src/                       Source files (only for non-template code)
test/                      Test files (*-test.cpp)
benchmark/                 Benchmark files (*-benchmark.cpp)
cmake/                     CMake helper modules
third-party/               Vendored libraries (catch2, rapidhash)
docs/                      Feature documentation
```

## Type Aliases

Defined in `include/opal/types.h`:

| Alias | Type |
|-------|------|
| `i8`, `i16`, `i32`, `i64` | Signed integers |
| `u8`, `u16`, `u32`, `u64` | Unsigned integers |
| `f32`, `f64` | Floating point |
| `char8` | `char` |
| `char16` | `wchar_t` (Windows) / `char16_t` (Linux) |

Always use these instead of raw C++ types.

## String Types

```
StringUtf8     = String<char8, EncodingUtf8<char8>>       // Primary string type
StringLocale   = String<char8, EncodingLocale>
StringWide     = String<char16, EncodingUtf16LE<char16>>
StringViewUtf8 = StringView<char8, EncodingUtf8<char8>>   // Primary view type
```

Defined at end of `include/opal/container/string.h` and `include/opal/container/string-view.h`.

## Coding Conventions

### Naming

- **Types/Classes:** `PascalCase` (e.g., `DynamicArray`, `MallocAllocator`)
- **Methods:** `PascalCase` (e.g., `PushBack`, `GetSize`, `IsEmpty`)
- **Exception:** `begin()`, `end()`, `cbegin()`, `cend()`, `empty()` are lowercase for STL compatibility
- **Member variables:** `m_` prefix with `snake_case` (e.g., `m_data`, `m_allocator`, `m_has_value`)
- **Local variables:** `snake_case` (e.g., `new_address`, `commit_size`)
- **Constants:** `k_` prefix with `snake_case` (e.g., `k_resize_factor`, `k_default_capacity`, `k_npos`)
- **Global variables:** `g_` prefix with `snake_case` (e.g., `g_default_stack`)
- **Static variables:** `s_` prefix with `snake_case` (e.g., `s_default_allocator`)
- **Template parameters:** `PascalCase` (e.g., `CodeUnitType`, `EncodingType`)
- **Compile-time bool constants:** `k_` prefix, e.g., `k_is_same_value`, `k_is_pointer_value`
- **Concepts:** `PascalCase` (e.g., `Integral`, `FloatingPoint`, `MoveConstructable`, `IsPOD`)
- **Enums:** `PascalCase` enum class with `PascalCase` values (e.g., `ErrorCode::OutOfBounds`)
- **Macros:** `OPAL_` prefix with `SCREAMING_SNAKE_CASE` (e.g., `OPAL_ASSERT`, `OPAL_PLATFORM_WINDOWS`)
- **Files:** `kebab-case` (e.g., `dynamic-array.h`, `string-encoding.cpp`)
- **Namespaces:** `PascalCase` (e.g., `Opal`, `Opal::Impl`)

### Formatting

Controlled by `.clang-format`:
- **Style:** Chromium-based, Allman braces
- **Indent:** 4 spaces, no tabs
- **Column limit:** 140
- Braces on their own line for functions, classes, control flow

### Comments

Be terse. Comments document the interface, not the implementation.

- Put documentation on function and class **declarations** - what it does, parameters, return value, error conditions.
- Avoid comments inside function bodies. If a body needs explaining, prefer clearer names and structure.
- Do not disclose implementation details in public documentation - describe observable behavior and contract, not the internal algorithm, data layout, or allocation strategy.
- Section banners and platform `#if` markers are exempt.

### Header Guards

Use `#pragma once` (no include guards).

### Include Style

- System includes (`<cstring>`, `<utility>`) first, then project includes (`"opal/..."`)
- Use full paths from include root: `"opal/container/dynamic-array.h"`, `"opal/types.h"`
- Relative includes also used within `include/opal/`: `"allocator.h"`, `"container/ref.h"`

### Template Implementation

Templates are implemented in the same `.h` file, below the class definition, separated by a comment banner:

```cpp
}  // namespace Opal

/** Implementation *******************************************************************************/
// or
/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/
```

Some headers use `TEMPLATE_HEADER` / `CLASS_HEADER` macros for brevity:

```cpp
#define TEMPLATE_HEADER template <typename T>
#define CLASS_HEADER Opal::DynamicArray<T>

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(...) { ... }
```

### Non-Template Code

Non-template implementations go in `src/*.cpp`. Method definitions use fully qualified names:

```cpp
void Opal::ClassName::MethodName(...) { ... }
```

Some `.cpp` files are minimal (just include the header) because all logic is in templates:
```cpp
// src/string.cpp
#include "opal/container/string.h"
```

Anonymous namespaces for file-local helpers:
```cpp
namespace
{
// helper functions
}  // namespace
```

### Section Dividers

Use comment banners to separate sections:

```cpp
// ------------------------------------------------------------------------------------------------
// Section name.
// ------------------------------------------------------------------------------------------------

// or

/*************************************************************************************************/
/** Section Name *********************************************************************************/
/*************************************************************************************************/
```

## Key Design Patterns

### Clone Instead of Copy

Copy constructors and copy assignment are **deleted** on all resource-owning types. Use `std::move()` or `.Clone()`:

```cpp
DynamicArray(const DynamicArray&) = delete;
DynamicArray& operator=(const DynamicArray&) = delete;
DynamicArray(DynamicArray&&) noexcept;
DynamicArray& operator=(DynamicArray&&) noexcept;
DynamicArray Clone(AllocatorBase* allocator = nullptr) const;
```

The free function `Opal::Clone()` in `common.h` dispatches: POD types copy directly, non-POD calls `.Clone(allocator)`.

For user types, inherit `ClonableBase<Derived>` and use `OPAL_CLONE_FIELDS(field1, field2, ...)`.

### Allocator Pattern

Every container takes an optional `AllocatorBase*`. If `nullptr`, uses `GetDefaultAllocator()`:

```cpp
DynamicArray(allocator_type* allocator = nullptr);
// In constructor body:
m_allocator = (allocator == nullptr) ? GetDefaultAllocator() : allocator;
```

Allocator hierarchy: `AllocatorBase` (abstract) -> `MallocAllocator`, `LinearAllocator`, `NullAllocator`, `SystemMemoryAllocator`.

Thread-local allocator stacks with RAII guards: `PushDefault`, `PushScratch`, `ScratchAsDefault`.

`Opal::New<T>(allocator, args...)` and `Opal::Delete<T>(allocator, ptr)` for allocator-aware heap objects.

### Error Handling

A failure the caller can act on is returned. A failure that means the caller broke a contract is thrown.

1. **`ErrorCode`** when the operation has no value to hand back - `Reserve`, `Insert`, `Erase`.
2. **`Expected<T, ErrorCode>`** when it does - `TryAt`, `Front`, `Back`, an `Insert` that yields an iterator.
3. **Exceptions** (inheriting from `Opal::Exception`) for contract violations, and for constructors, which have no way to return anything. `GetValue` on a key that is not in the map throws; so does a constructor that cannot allocate.

Allocation failure is recoverable, not exceptional. Allocators are pluggable and often budgeted (`LinearAllocator`, `NullAllocator`), so exhausting one is an ordinary event to branch on: return `ErrorCode::OutOfMemory` instead of throwing. `AllocatorBase::Alloc` returns `nullptr` for the same reason.

`ErrorCode` enum in `error-codes.h`. Exception types in `exceptions.h`.

`OPAL_ASSERT(condition, message)` - active only in debug builds (`OPAL_DEBUG`), maps to `assert()`.

`Deque`, `DynamicArray`, `String`, `HashSet` and `HashMap` all follow this. What still throws is what has nowhere to put a code: constructors, `Clone`, `operator=` from an initializer list, and `String::operator+=`.

### Move Utility

`Opal::Move()` in `type-traits.h` is the project's own `std::move` equivalent. Used throughout instead of `std::move()` (though `std::move()` also appears in some places).

### Concepts and Type Traits

Custom concepts in `type-traits.h`: `IsPOD`, `Integral`, `FloatingPoint`, `IntegralOrFloatingPoint`, `MoveConstructable`, `RandomAccessIterator`, etc. Used with `requires` clauses.

`if constexpr (IsPOD<T>)` pattern used to optimize for trivial types (memcpy vs placement new).

## Test Patterns

- Test framework: Catch2 amalgamated
- Test file naming: `<feature>-test.cpp`
- Use `using namespace Opal;` at top of test files
- Include `"test-helpers.h"` for Catch2 and custom matchers
- Catch2 include wrapped with warning suppression macros
- Test structure: `TEST_CASE("Description", "[Tag]")` with `SECTION("...")` nesting
- Tags match the feature name: `[String]`, `[DynamicArray]`, `[JsonReader]`, `[JsonWriter]`
- Math tests use custom matchers: `CHECK_QUATERNION`, `CHECK_VECTOR3`, `CHECK_POINT3`

## Platform Abstractions

Platform-specific code uses preprocessor:
```cpp
#if defined(OPAL_PLATFORM_WINDOWS)
    // Windows implementation (VirtualAlloc, _aligned_malloc, etc.)
#elif defined(OPAL_PLATFORM_LINUX)
    // Linux implementation (mmap, aligned_alloc, etc.)
#endif
```

Compiler detection: `OPAL_COMPILER_MSVC`, `OPAL_COMPILER_CLANG`, `OPAL_COMPILER_GCC`.

Warning suppression macros: `OPAL_START_DISABLE_WARNINGS`, `OPAL_DISABLE_WARNING(name)`, `OPAL_END_DISABLE_WARNINGS`.

## Commits

Do not add `Co-Authored-By:` or any other co-author trailer to commit messages. No tool attribution lines either.
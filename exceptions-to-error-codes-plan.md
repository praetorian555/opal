# Exceptions to error codes - audit

Pass over every `throw` outside `test/` and `third-party/`. Roughly 300 textual hits; ~160 are
`@throw` doc comments. What follows is the ~200 real throw statements, sorted by what should happen
to them.

The rule from `CLAUDE.md`: a failure the caller can act on is returned, a failure that means the
caller broke a contract is thrown. `Deque`, `DynamicArray`, `String`, `HashSet`, `HashMap` already
follow it. Nothing else in the library does.

---

## 1. Allocators - breaks a contract the docs already promise

`CLAUDE.md` states "`AllocatorBase::Alloc` returns `nullptr`". Two of the four allocators throw
instead, so every `if (ptr == nullptr)` check in the containers is dead code on those allocators and
the `ErrorCode::OutOfMemory` return path is unreachable. This is the one place where the conversion
is a bug fix, not a design change.

| Site | Now | Should be |
|---|---|---|
| `include/opal/allocator.h:105` | `NullAllocator::Alloc` throws `OutOfMemoryException` | `return nullptr` |
| `src/allocator.cpp:120` | `SystemMemoryAllocator::Commit` throws on reserve exhaustion | `Commit` returns `ErrorCode`; `Alloc` returns `nullptr` |
| `src/allocator.cpp:132` | `Commit` throws on `VirtualAlloc`/`mprotect` failure | same |

`SystemMemoryAllocator::Alloc` (`src/allocator.cpp:92`) and `LinearAllocator::Alloc`
(`src/allocator.cpp:182`) both call `Commit` and therefore both throw today. Fixing `Commit` fixes
all three.

Keep as is:

- `src/allocator.cpp:35,39` - `InvalidArgumentException` for `bytes_to_reserve == 0` and
  `bytes_to_initially_alloc > bytes_to_reserve`. Programmer error, and a constructor.
- `src/allocator.cpp:57,70` - constructor cannot reserve. No return channel. A
  `static Expected<SystemMemoryAllocator, ErrorCode> Create(...)` would fix it if we want it.
- `include/opal/allocator.h:216,234` - `New<T>` with a null allocator. Contract violation, better as
  `OPAL_ASSERT`. Note the `try`/`catch` in both overloads only exists because `T`'s constructor may
  throw; it stays needed as long as constructors throw, and `T` is arbitrary, so removing it would
  need a constraint on `T` rather than finishing §11.

## 2. File system - 81 throws, the whole API needs new signatures

`src/file-system.cpp` throws `PathNotFoundException`, `PathAlreadyExistsException`,
`DirectoryNotEmptyException`, `NotDirectoryException`, `OutOfMemoryException` and bare `Exception`
wrapping `GetLastError`/`errno`. Every one is an ordinary runtime outcome of touching a file system.
The `@throw` blocks in `include/opal/file-system.h` (lines 12-148) are documentation only and change
with the signatures.

| Function | Now | Should be |
|---|---|---|
| `CreateFile` | `void` | `ErrorCode` |
| `DeleteFile` | `void` | `ErrorCode` |
| `CreateDirectory` | `void` | `ErrorCode` |
| `DeleteDirectory` | `void` | `ErrorCode` |
| `WriteStringToFile` / `WriteBytesToFile` | `void` | `ErrorCode` |
| `AppendStringToFile` / `AppendBytesToFile` | `void` | `ErrorCode` |
| `ReadFileAsString` | `StringUtf8` | `Expected<StringUtf8, ErrorCode>` |
| `ReadFileAsBytes` | `DynamicArray<u8>` | `Expected<DynamicArray<u8>, ErrorCode>` |
| `CollectDirectoryContents` | `DynamicArray<DirectoryEntry>` | `Expected<DynamicArray<DirectoryEntry>, ErrorCode>` |

`ErrorCode` already has `PathNotFound`, `AlreadyExists`, `NotEmpty`, `OSFailure`, `OutOfMemory`.
Missing: a code for `NotDirectoryException` - add `ErrorCode::NotDirectory`.

`Exists`, `IsDirectory` and `IsFile` already return `bool` and swallow errors; leave them.

## 3. Paths - half converted already

`GetFileName`, `GetStem`, `GetExtension`, `GetParentPath` return `Expected`. The other four do not,
and two of them carry a `CheckAppend` helper whose only job is to turn an `ErrorCode` back into an
exception.

| Site | Now | Should be |
|---|---|---|
| `src/paths.cpp:33,39,46` | `GetCurrentWorkingDirectory` throws on OS failure / transcode failure | `Expected<StringUtf8, ErrorCode>` |
| `src/paths.cpp:64,69,74` | `SetCurrentWorkingDirectory` throws | `ErrorCode` |
| `src/paths.cpp:20` (`CheckAppend`) | `NormalizePath` throws `OutOfMemoryException` | `Expected<StringUtf8, ErrorCode>`, helper deleted |
| `include/opal/paths.h:105` (`Combine`) | same shape, lambda named `check` | `Expected<StringUtf8, ErrorCode>` |

`NormalizePath` is called by `SetCurrentWorkingDirectory`, so these two move together.

## 4. Program arguments - exceptions used as control flow

The worst offender. `src/program-arguments.cpp:59,64` throw `HelpRequestedException` and
`VersionRequestedException` on `--help` / `--version`, which are the two most ordinary command lines
a program receives. `docs/program-arguments.md:304` documents callers wrapping `Parse` in a
`try`/`catch` to handle them.

Proposal: `Parse` returns an enum, no exceptions.

```cpp
enum class ProgramArgumentsResult : u8
{
    Success, HelpRequested, VersionRequested, InvalidArgument, OutOfMemory
};
```

| Site | Now |
|---|---|
| `src/program-arguments.cpp:59,64` | help/version as exceptions |
| `src/program-arguments.cpp:96` | required argument missing - user input, recoverable |
| `src/program-arguments.cpp:46,50` | `OutOfMemoryException` from `PushBack` failing |
| `include/opal/program-arguments.h:135,392` | no mapping for value - user input |
| `include/opal/program-arguments.h:217,222,250,293,348,353,360` | malformed definition or missing mapping |
| `include/opal/program-arguments.h:309,433,457` | value not in possible values - user input |

`ProgramArgumentDefinition::SetValue` is the virtual that throws in most of those; it needs to return
`ErrorCode` so `Parse` can collect the failure.

Keep as is: `include/opal/program-arguments.h:46,50` - empty argument name or description. That is a
programmer building a bad definition, and it is a constructor.

## 5. JSON

### Reader

`JsonReader::Parse` parses untrusted input and throws `JsonParseException` with line, column and
offset (`src/json-reader.cpp:965`). Malformed JSON is the expected case, not a contract violation.

| Site | Now | Should be |
|---|---|---|
| `src/json-reader.cpp:965` (`ThrowError`) | `JsonParseException` | `Expected<JsonReader, JsonParseError>` - keep the line/column/offset in a `JsonParseError` struct, a bare `ErrorCode` loses too much |
| `src/json-reader.cpp:566,590,895` | `OutOfMemoryException` while building the tree | same error channel |

Value access is the other half and is genuinely a contract question. `GetBool`, `GetNumber`,
`GetIntegerNumber`, `GetString` throw `JsonTypeMismatchException` via `ThrowTypeMismatch`
(`src/json-reader.cpp:40`); `operator[]` throws `OutOfBoundsException` (232) and
`InvalidArgumentException` for a missing key (247). Callers who checked `IsX()` first cannot trigger
these, so throwing is defensible - but the map/array accessors are exactly the case `TryAt` exists
for on the containers. Suggest keeping the throwing accessors and adding `TryGetBool`,
`TryGetNumber`, `TryGetString`, `TryAt(index)`, `TryFind(key)`, `TryGetPath` returning `Expected`.

### Writer

| Site | Now | Should be |
|---|---|---|
| `src/json-writer.cpp:143,147` | `JsonSerializeException` for NaN / Infinity | `ErrorCode::InvalidArgument` - a `JsonValue` holding NaN is reachable from user data |
| `src/json-writer.cpp:81,89,254,278` | `OutOfMemoryException` growing the output | `ErrorCode::OutOfMemory` |

`JsonWriter::Serialize` (both overloads) becomes `Expected<StringUtf8, ErrorCode>`.

## 6. Threading

| Site | Now | Should be |
|---|---|---|
| `src/thread.cpp:62,72` | `Impl::CreateThread` throws `Exception` when `CreateThread`/`pthread_create` fails | `Expected<ThreadHandle, ErrorCode>` with `OSFailure` |
| `src/thread.cpp:263` | `OutOfMemoryException` | `ErrorCode::OutOfMemory` |
| `include/opal/threading/thread.h:78` | `Opal::CreateThread` propagates | `Expected<ThreadHandle, ErrorCode>` |

Note `include/opal/threading/thread.h:86` already checks `handle.native_handle == nullptr` after
`Impl::CreateThread` returns. That branch is unreachable today because the throw happens first - it
is the error-code version of this function, already written, just never reached.

`include/opal/threading/channel-mpmc.h:72,117` - `throw Exception("Data type can't be copied!")` in
the `else` of an `if constexpr`. This is not an error code candidate, it is a compile-time
constraint: same fix as commit 5c5a147 (fail the build for a key type with no hasher). Add a
`requires (CopyAssignable<T> || Clonable<T>)` to the channel, or `static_assert(false)` in the else.

## 7. Logging

- `include/opal/logging.h:162` - `Logger::Log` throws `UnregisteredCategoryException` for an
  unregistered category. A log call throwing is bad: logging is often already on an error path, and
  this makes every log statement an exception source. Suggest logging to a fallback category plus
  `OPAL_ASSERT`, or returning `ErrorCode` and letting the macros drop it.
- `src/logging.cpp:244` - `HandleFatal` throws `FatalLogException` after flushing. This one is
  deliberate: a fatal log is supposed to unwind. Keep, though a configurable abort handler would be
  more honest than an exception a caller can swallow.

## 8. String formatting

`include/opal/container/string-format.h:35,65` - `Format` and `AppendFormat` throw
`OutOfMemoryException`. Suggested: `AppendFormat` returns `ErrorCode`, `Format` returns
`Expected<StringUtf8, ErrorCode>`.

Catch: line 35 is inside `StringFormatIterator::operator=`, which `std::vformat_to` drives and which
has no return channel. Latch the first failure in the iterator as a member and read it after
`vformat_to` returns, instead of throwing out through `std::format` internals.

This one is load-bearing - `Logger` and `program-arguments` both format through it.

## 9. Math

`include/opal/math/matrix.h:557,574` - `throw Exception("Singular matrix")` inside the inverse.
Whether a matrix is singular is a property of runtime data, so this is recoverable:
`Expected<Matrix, ErrorCode>` or a `bool Invert(const Matrix&, Matrix& out)`.

`include/opal/math/matrix.h:648` - `NotImplementedException` for `Cofactor` on an unsupported matrix
size. Size is a template parameter, so this belongs in a `requires` clause, not at runtime.

## 10. Contract violations - leave throwing

Listed so the pass does not touch them by accident.

- Bounds checks with an `Expected` sibling already available (`TryAt`, `Front`, `Back`):
  `dynamic-array.h:1136,1138,1150,1152,1162,1172,1182,1192,1616,1643,1669,1705,1709,1759`;
  `deque.h:635,637,649,651`; `hash-map.h:681,692` (`GetValue` on a missing key);
  `string.h:1320,1746,1757,2777`; `string-view.h:96`; `in-place-array.h:349,359`.
- `variant.h:174,190,204,219,418` - `Get<T>()` on the wrong alternative, and access to a moved-from
  variant. A `TryGet` returning `Expected` would be a nice addition; the throwing form is correct.
- `casts.h:31` - `Narrow` round-trip failure, debug builds only.
- `shared-ptr.h:92,123` - `ThreadSafe` policy with a non-thread-safe allocator. Constructor, and a
  static property of the types involved. Better as an assert than an exception, but not an error code.

## 11. Constructors, `Clone`, `operator=` - the known gap

`CLAUDE.md` already records these as what still throws because there is nowhere to put a code.
Listing them so the size of the remaining hole is visible:

- `dynamic-array.h:745,767,789,828,853,1110`
- `string.h:1305,1327,1342,1347,1364,1381,1666`
- `hash-map.h:357,367,373,384,390`
- `hash-set.h:286`
- `string.h:2797,2807,2818` - `String::operator+=`

Closing this means static factories - `static Expected<DynamicArray<T>, ErrorCode> Create(...)`,
`Expected<String, ErrorCode> TryClone(...)` - alongside the throwing constructors. Larger job than
everything above and worth deciding on separately.

## 12. Dead `#else` branches - delete, do not convert

`NotImplementedException` thrown from `#else` arms of platform or compiler checks. None of these
compile on Windows or Linux. Three of them are missing the trailing semicolon, which proves it:
`src/mutex.cpp:43,104,123` and `src/bit.cpp:61,72`.

Two are inside destructors (`src/mutex.cpp:43`, `src/condition-variable.cpp:40`), which are
implicitly `noexcept` - if they ever did compile they would call `std::terminate`.

Replace all of them with `#error "Platform not supported"`, which is what `src/paths.cpp:89` and
`src/bit.cpp:50` already do.

Sites: `src/mutex.cpp:22,43,85,104,123`; `src/condition-variable.cpp:26,40,79,90,102,127`;
`src/signal.cpp:51,76,90,102`; `src/thread.cpp:82,107,122,137,281,322`; `src/bit.cpp:61,72`;
`src/allocator.cpp:73,88,129`; `src/paths.cpp:51,77`; `src/file-system.cpp:80,116,169,213,268,295,452,531,611,701,712,723,734`.

Separate but similar: `string.h:3857,3884,3911,3938` throw `NotImplementedException` from the
`default:` of a `switch` over `NumberSystemBase`, which is exhaustive. Use `OPAL_ASSERT` instead.

---

## Suggested order

1. Allocators (§1) - unblocks the containers' existing `OutOfMemory` paths, which are dead today.
2. String formatting (§8) - `logging` and `program-arguments` both sit on top of it.
3. File system (§2) and paths (§3) - largest surface, no dependents inside the library.
4. Program arguments (§4) and JSON (§5) - both consume the above.
5. Threading (§6), logging (§7), math (§9).
6. Dead `#else` branches (§12) - independent, can go any time.
7. Decide separately on constructors (§11).

## New `ErrorCode` values needed

- `NotDirectory` (§2)

`OSFailure`, `PathNotFound`, `AlreadyExists`, `NotEmpty`, `InvalidArgument`, `OutOfMemory`,
`NotImplemented` already cover the rest.

---

## Task list

### Allocators (§1)

- [x] `NullAllocator::Alloc` returns `nullptr` instead of throwing - `include/opal/allocator.h:105`
- [x] `SystemMemoryAllocator::Commit` returns `ErrorCode` - `src/allocator.cpp:112-135`
- [x] `SystemMemoryAllocator::Alloc` returns `nullptr` on a failed commit - `src/allocator.cpp:92`
- [x] `LinearAllocator::Alloc` returns `nullptr` on a failed commit - `src/allocator.cpp:182`
- [x] `New<T>` returns `nullptr` when the allocator cannot supply storage, instead of placement-new on a null address
- [x] Confirm the containers' `ErrorCode::OutOfMemory` paths are now reachable; add tests using `NullAllocator`
      (the four `Paths::Get*` tests were asserting a throw that came from the allocator, not the function - they now
      assert `ErrorCode::OutOfMemory` off the `Expected`, which is the path that had never once run)
- [ ] `New<T>` null-allocator check becomes `OPAL_ASSERT` - `include/opal/allocator.h:216,234`

### String formatting (§8)

- [x] `StringFormatIterator` latches the first failure instead of throwing - `include/opal/container/string-format.h:35`
- [x] `AppendFormat` returns `ErrorCode`, reads the latched failure after `vformat_to` - `string-format.h:59`
- [x] `Format` returns `Expected<StringUtf8, ErrorCode>` - `string-format.h:84`
- [x] Update call sites. `Logger` turned out not to use either function - it drives `std::vformat_to` through its own
      `BoundedFormatIterator`, which writes into a fixed buffer and cannot fail. `json-writer` is the only caller, and
      it wraps `AppendFormat` in a helper that throws, matching what the rest of that file already does; §5 removes it.

### File system (§2)

- [x] Add `ErrorCode::NotDirectory` - `include/opal/error-codes.h`
- [x] `CreateFile`, `DeleteFile`, `CreateDirectory`, `DeleteDirectory` return `ErrorCode`
      (`CreateDirectory`'s `throw_if_exists` parameter is now `fail_if_already_exists`, matching `CreateFile`)
- [x] `WriteStringToFile`, `WriteBytesToFile`, `AppendStringToFile`, `AppendBytesToFile` return `ErrorCode`
- [x] `ReadFileAsString` returns `Expected<StringUtf8, ErrorCode>`
- [x] `ReadFileAsBytes` returns `Expected<DynamicArray<u8>, ErrorCode>`
- [x] `CollectDirectoryContents` returns `Expected<DynamicArray<DirectoryEntry>, ErrorCode>`
- [x] Strip the `@throw` blocks from `include/opal/file-system.h:12-148`
- [x] Update `test/file-system-test.cpp`
- [x] **`file-system.cpp` no longer throws from a sized container constructor.** The eight wide-path buffers went
      through a `ToWidePath` helper that resizes and transcodes, the four result buffers default-construct and `Resize`,
      and the two remaining `StringWide` constructions in the Windows directory walk became a view and an `Append`
      chain. A `NullAllocator` test covers each group; its absence is why this was invisible.
- [x] `file-system-test.cpp` leaves files behind when an assertion fails mid-test, and the next run then fails on its
      `REQUIRE(!Exists(path))` preconditions. Not caused by this work, but it bit twice during it. A `ScopedTestPaths`
      guard now records every path a test builds and removes whatever survives when the test case ends. Verified by
      failing a section on purpose between create and delete and confirming nothing was left in the working tree.

### Paths (§3)

- [x] `GetCurrentWorkingDirectory` returns `Expected<StringUtf8, ErrorCode>` - `src/paths.cpp:25`
- [x] `NormalizePath` returns `Expected<StringUtf8, ErrorCode>`, delete the `CheckAppend` helper - `src/paths.cpp:16,81`
- [x] `SetCurrentWorkingDirectory` returns `ErrorCode` - `src/paths.cpp:55`
- [x] `Combine` returns `Expected<StringUtf8, ErrorCode>`, delete the `check` lambda - `include/opal/paths.h:87`
- [ ] **Fix `Combine`'s separator check** - `include/opal/paths.h:110`. It reads
      `result.Back().GetValue() != '/' || result.Back().GetValue() != '\\'`, which is true for every possible character,
      so the branch always runs and a separator is inserted even when the accumulated path already ends in one.
      `Combine("a/", "b")` therefore yields `a//b`. Correct condition is `&&`. Deliberately left out of the error-code
      commit: it changes what `Combine` returns for trailing-separator input, so it wants its own change and its own
      test rather than riding along with a signature change.

### Program arguments (§4)

- [x] Add `ProgramArgumentsResult` enum
- [x] `ProgramArgumentDefinition::SetValue` returns `ErrorCode` - `include/opal/program-arguments.h:60`
- [x] Convert the value-parsing throws - `program-arguments.h:135,309,392,433,457`. `GetValueFromMapping` became
      `FindValueFromMapping` returning a pointer, since a missing key is now a code rather than a throw. The array
      `SetValue` was also dropping every `PushBack` result on the floor; those are propagated now.
- [x] Leave the definition constructors throwing - `program-arguments.h:46,50,217,222,250,293,348,353,360`. An empty
      name or an enum without a mapping is the programmer misconfiguring the parser, not the user mistyping, and they
      are constructors either way.
- [x] `Build` returns `ProgramArgumentsResult`; drop `HelpRequestedException` and `VersionRequestedException` - `src/program-arguments.cpp:59,64,96`
- [x] `Build` reports `PushBack` failure as `OutOfMemory` - `src/program-arguments.cpp:46,50`
- [x] Rewrite the `try`/`catch` example in `docs/program-arguments.md:304`, plus the prose and the README snippet
- [ ] The integral scalar `SetValue` never checks `IsPossibleValue`, so `possible_values` is silently ignored for
      `i32`/`u32`/... arguments while the string and array paths do enforce it. Left as-is here to keep the conversion
      behaviour-neutral; `docs/program-arguments.md` documents the numeric case as validated, so one of the two is wrong.

### JSON (§5)

- [ ] Add a `JsonParseError` struct carrying line, column, offset
- [ ] `JsonReader::Parse` returns `Expected<JsonReader, JsonParseError>` - `include/opal/container/json-reader.h:278,286`
- [ ] Route the parser's `ThrowError` and OOM sites through it - `src/json-reader.cpp:566,590,895,965`
- [ ] Add `TryGetBool`, `TryGetNumber`, `TryGetIntegerNumber`, `TryGetString`, `TryAt`, `TryFind`, `TryGetPath`
- [ ] `JsonWriter::Serialize` (both overloads) returns `Expected<StringUtf8, ErrorCode>` - `include/opal/container/json-writer.h:44,54`
- [ ] NaN / Infinity become `ErrorCode::InvalidArgument` - `src/json-writer.cpp:143,147`
- [ ] Serializer OOM sites use the same channel - `src/json-writer.cpp:81,89,254,278`

### Threading (§6)

- [ ] `Impl::CreateThread` returns `Expected<ThreadHandle, ErrorCode>` - `src/thread.cpp:53`
- [ ] `Opal::CreateThread` returns `Expected<ThreadHandle, ErrorCode>`; the existing null check at `thread.h:86` becomes live
- [ ] `src/thread.cpp:263` returns `ErrorCode::OutOfMemory`
- [ ] Constrain `ChannelMpmc` on `CopyAssignable<T> || Clonable<T>` at compile time - `include/opal/threading/channel-mpmc.h:72,117`
- [ ] Run the Clang WSL build - this touches lifetimes and threading, so ASan is not optional

### Logging (§7)

- [ ] `Logger::Log` stops throwing on an unregistered category - `include/opal/logging.h:162`
- [ ] Decide whether `HandleFatal` keeps `FatalLogException` or gets an abort handler - `src/logging.cpp:244`

### Math (§9)

- [ ] Matrix inverse returns `Expected<Matrix, ErrorCode>` for singular input - `include/opal/math/matrix.h:557,574`
- [ ] `Cofactor` size restriction moves to a `requires` clause - `include/opal/math/matrix.h:648`

### Dead `#else` branches (§12)

- [ ] Replace with `#error "Platform not supported"` - `src/mutex.cpp:22,43,85,104,123`
- [ ] `src/condition-variable.cpp:26,40,79,90,102,127`
- [ ] `src/signal.cpp:51,76,90,102`
- [ ] `src/thread.cpp:82,107,122,137,281,322`
- [ ] `src/bit.cpp:61,72`
- [ ] `src/allocator.cpp:73,88,129`
- [ ] `src/paths.cpp:51,77`
- [ ] `src/file-system.cpp:80,116,169,213,268,295,452,531,611,701,712,723,734`
- [ ] Exhaustive-`switch` defaults become `OPAL_ASSERT` - `include/opal/container/string.h:3857,3884,3911,3938`

### Deferred - needs a decision first (§11)

- [ ] Decide on static factories for constructors, `Clone` and `operator+=`
- [ ] `DynamicArray` - `dynamic-array.h:745,767,789,828,853,1110`
- [ ] `String` - `string.h:1305,1327,1342,1347,1364,1381,1666,2797,2807,2818`
- [ ] `HashMap` - `hash-map.h:357,367,373,384,390`
- [ ] `HashSet` - `hash-set.h:286`

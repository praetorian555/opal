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
  static property of the types involved. Better as an assert than an exception, but not an error code. Note this one
  throws out of `SharedPtr::Create` too, which is deliberate: it is a contract the caller broke, not a shortage.
- `shared-ptr.h` construction - the constructor throws `OutOfMemoryException` when it cannot allocate, matching the
  container constructors, and `SharedPtr::Create` returns `Expected<SharedPtr, ErrorCode>` for callers on a budgeted
  allocator. The alternative, leaving the object silently invalid, put the burden on every caller to remember a check
  that nothing enforced - which is exactly how the JSON parser ended up building invalid nodes.

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
everything above and worth deciding on separately. See §13, which is that decision.

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

## 13. Factories, and whether exceptions can go entirely

The question behind §11: replace allocating constructors with factories so the library can build with
`-fno-exceptions`.

**Worth doing for allocating construction. Not worth framing as removing exceptions entirely** - that goal
fails on something other than constructors, and chasing it costs more than it returns.

### What factories buy

Constructors are the only place left where an unavoidable failure has nowhere to go. Everything converted in
§1-§8 had a return channel available. So factories close the last real gap.

The evidence that hidden allocation failure is expensive is already in this document: `SharedPtr` dereferencing
a null reference count, the JSON parser building invalid nodes, and four `Paths::Get*` tests asserting a throw
the function never made. All three were fixed without factories - `nullptr` returns plus checks did it - but a
factory makes them harder to write wrong to begin with.

### What they cost

- **Two-phase initialization goes viral.** A type with a `DynamicArray<T> m_items` member cannot build it in a
  member-init list from a factory. It default-constructs and assigns, so the enclosing type wants a factory too,
  outward through the library and through user code. `JsonReader` already works this way. It is workable, but
  every member has to be cheaply default-constructible and "constructed" stops meaning "valid".
- **Factories force a move, and moves here are not free.** `JsonReader` held views into its own inline string
  storage; routing it through `Expected` moved the object out from under them and failed seven tests. NRVO does
  not save you through `Expected`. Any address-sensitive type becomes a hazard the moment its constructor
  becomes a factory.
- **Call-site cost.** One declaration becomes four lines with an early return, and `Expected` has no `and_then`
  or `Map`, so there is no compact way to chain.

### Why "entirely" does not land

- **Contract violations are a separate problem.** `operator[]`, `Variant::Get<T>`, `Narrow`,
  `HashMap::GetValue` - those are not "no channel available", they are "the caller broke the rules", and §10
  says they should throw. Removing exceptions means turning them into `OPAL_ASSERT` plus `Try*` variants.
  Defensible for a real-time engine, but a bigger behavioural change than factories and a separate decision.
- **`std::format` throws and has no error-code path.** `string-format.h` and `logging.h` drive
  `std::vformat_to`, which raises `std::format_error` on a malformed format string. This decides whether
  `-fno-exceptions` is reachable at all, so it is the first thing to check.
- `Opal::Exception` now derives from `std::exception`, which couples the library toward std exceptions rather
  than away from them.

---

## Suggested order

1. Allocators (§1) - unblocks the containers' existing `OutOfMemory` paths, which are dead today.
2. String formatting (§8) - `logging` and `program-arguments` both sit on top of it.
3. File system (§2) and paths (§3) - largest surface, no dependents inside the library.
4. Program arguments (§4) and JSON (§5) - both consume the above.
5. Threading (§6), logging (§7), math (§9).
6. Dead `#else` branches (§12) - independent, can go any time.
7. Factories for allocating construction (§11, §13). Step 1 there is worth doing whether or not the later
   steps ever happen; step 0 decides whether they can.

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
- [x] **`PushDefaultAllocator` seeds the system allocator when the stack is empty** - `src/allocator.cpp:252`. Only
      `GetDefaultAllocator` seeded index 0, so a thread whose first stack touch was a push put its own allocator at the
      bottom and the matching pop tripped `PopDefaultAllocator`'s "System provided default allocator can't be popped"
      assert. Not caused by this work, but the `NullAllocator` tests it added are what expose it: they open with
      `PushDefault`, so `opal_test "[Paths]"` aborted on the first section while the full run passed, because some
      earlier test had already seeded the stack. The existing child-thread tests miss it too - `Opal::CreateThread`
      reads the default allocator while starting the thread, so the regression test uses a bare `std::thread`.

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
- [x] **Fix `Combine`'s separator check** - `include/opal/paths.h:110`. It read
      `result.Back().GetValue() != '/' || result.Back().GetValue() != '\\'`, which is true for every possible character,
      so the branch always ran and a separator was inserted even when the accumulated path already ended in one.
      `Combine("a/", "b")` yielded `a//b`. Now `&&`, with tests for a trailing `/`, a trailing `\`, both mixed, and a
      trailing separator followed by an empty component. Both separators are accepted on either platform, so
      `Combine("a/", "b")` keeps the separator the caller wrote rather than normalizing it.

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

- [x] Add a `JsonParseError` struct carrying code, line, column, offset and an owned message
- [x] `JsonReader::Parse` returns `Expected<JsonReader, JsonParseError>` - `include/opal/container/json-reader.h:278,286`
- [x] Route the parser's `ThrowError` and OOM sites through it - `src/json-reader.cpp:566,590,895,965`. The abort stays
      a non-local jump inside the parser, now a file-local `ParseAbort` rather than a public exception type, and is
      caught at the `Parse` boundary. Threading a code back through twenty-odd failure points in a recursive descent
      would have buried the grammar for no gain the caller can see.
- [x] The parser does not check the `SharedPtr` allocations behind `JsonArray` and `JsonObject`, so a document that
      runs out of memory mid-parse builds an invalid node rather than reporting `OutOfMemory`. The parser builds both
      through `SharedPtr::Create` now and aborts with `OutOfMemory`, and the `NullAllocator` parse test is in.
- [ ] Add `TryGetBool`, `TryGetNumber`, `TryGetIntegerNumber`, `TryGetString`, `TryAt`, `TryFind`, `TryGetPath`
- [x] `JsonWriter::Serialize` (both overloads) returns `Expected<StringUtf8, ErrorCode>` - `include/opal/container/json-writer.h:44,54`
- [x] NaN / Infinity become `ErrorCode::InvalidArgument` - `src/json-writer.cpp:143,147`
- [x] Serializer OOM sites use the same channel - `src/json-writer.cpp:81,89,254,278`. The serializer records the first
      failure and skips every later write, the same shape as `StringFormatIterator`, so the walk stops instead of
      piling work onto a string that is already short. That let the `AppendFormatted` shim from §8 go.
- [ ] `-Wuseless-cast` on `static_cast<i64>(9007199254740993LL)` in `test/json-value-test.cpp:237,257` and
      `test/json-writer-test.cpp:386`. Pre-existing, unrelated to this work, only visible once those files recompiled.

### Threading (§6)

- [x] `Impl::CreateThread` returns `Expected<ThreadHandle, ErrorCode>` with `OSFailure` - `src/thread.cpp:53`. It still owns the
      thread data and destroys it when the thread does not start, so the caller has nothing to clean up.
- [x] `Opal::CreateThread` returns `Expected<ThreadHandle, ErrorCode>`. The null check at `thread.h:86` was checking the wrong
      thing: what can actually be null now is `New<T>`'s result, and passing that to `Impl::CreateThread` handed the new thread a
      null pointer to dereference. That is the check that is live, and it reports `OutOfMemory`.
- [x] `GetCpuInfo` returns `Expected<CpuInfo, ErrorCode>` - `src/thread.cpp:141`. The `OutOfMemoryException` at 263 was one of
      four failures in that function; the other three - a null `Alloc`, a failed `GetLogicalProcessorInformationEx`, a failed
      `opendir` - returned an empty `CpuInfo` that the caller could not tell from a machine with no cores. All four now report.
      Two `PushBack` results were also being dropped. `PrintCpuInfo` returns `ErrorCode` and prints nothing on failure.
- [x] `ThreadPool`'s constructor reserves room for every handle before starting a thread, so a worker can never start and then
      fail to be recorded, which would leave it running with no sentinel coming - `src/thread-pool.cpp:28`. A failed start shuts
      down the workers that did start and then throws, since a constructor has nowhere to put a code.
- [x] Constrain `QueueMPMC` on `CopyAssignable<T> || Clonable<T>` at compile time - `include/opal/threading/channel-mpmc.h:37`.
      Both `throw Exception("Data type can't be copied!")` branches are gone.
- [x] Run the Clang WSL build - this touches lifetimes and threading, so ASan is not optional

### Logging (§7)

- [x] `Logger::Log` stops throwing on an unregistered category - `include/opal/logging.h:162`. Neither of the two options in §7
      survived contact: an `ErrorCode` return would be dropped by every caller, and the suggested `OPAL_ASSERT` aborts in debug on
      exactly the path being made safe, which is worse than the exception it replaces. What registering a category actually does is
      give it a level of its own, so a category without one is not a mistake - the message is written, gated by the logger's level
      alone. `UnregisteredCategoryException` is gone, and `LoggerNotInitializedException` went with it, having never been thrown.
- [x] `HandleFatal` gets a handler - `src/logging.cpp:244`. `SetFatalLogHandler` / `GetFatalLogHandler` take a
      `void (*)(StringViewUtf8 category, StringViewUtf8 message)`, and the default one throws `FatalLogException`, so behaviour is
      unchanged until someone installs their own. That is what §13 step 4 needs: without exceptions, a program sets a handler that
      ends the process instead of one that can be swallowed.

### Math (§9)

- [x] Matrix inverse returns `Expected<Matrix, ErrorCode>` for singular input - `include/opal/math/matrix.h:557,574`. Singular is
      `ErrorCode::InvalidArgument`: the argument cannot be inverted, and a code of its own would be used by one function.
- [x] `Cofactor` size restriction moves to a `requires` clause - `include/opal/math/matrix.h:648`
- [x] `Inverse`'s square-matrix `OPAL_ASSERT` moves to a `requires` clause as well - same defect one line above `Cofactor`'s, and
      it also takes the unconstrained `Inverse` template out of the overload set for `Quaternion` and `DualQuaternion`, which have
      `Inverse` overloads of their own and were relying on partial ordering to win.

### Dead `#else` branches (§12)

- [x] Replace with `#error "Platform not supported"` - `src/mutex.cpp:22,43,85,104,123`
- [x] `src/condition-variable.cpp:26,40,79,90,102,127`
- [x] `src/signal.cpp:51,76,90,102`
- [x] `src/thread.cpp:82,107,122,137,281,322` - done alongside §6, since the conversion rewrote the same `#if` blocks
- [x] `src/bit.cpp:61,72`
- [x] `src/allocator.cpp:73,88,129`
- [x] `src/paths.cpp:51,77` - already done by §3
- [x] `src/file-system.cpp:80,116,169,213,268,295,452,531,611,701,712,723,734` - already done by §2
- [x] `default:` of the `NumberSystemBase` switches becomes `OPAL_ASSERT` - `include/opal/container/string.h:3857,3884,3911,3938`.
      §12 calls these switches exhaustive; they are not. `NumberSystemBase` has a fourth value, `Binary`, and it is missing from all
      four. What makes the branch dead is the caller: `NumberToString` routes `Binary` to `ToBinary` and only reaches `GetFormat`
      with the three bases that have a printf specifier. So the assert stands for "the caller already handled Binary", not for an
      enum value that cannot exist. Checked against a Release build, where `OPAL_ASSERT` expands to nothing and a `default:` label
      needs a statement after it regardless.

Note: `NotImplementedException` now has no throw site anywhere in the library. Left declared in `exceptions.h`, since removing it
is a public API change rather than part of this pass.

### Factories and optional exceptions (§11, §13)

Decided: add factories for allocating construction, but do not chase removing exceptions entirely. Do the four
steps in order - each one stands on its own, and stopping after any of them leaves the library in a coherent
state. Step 1 alone closes §11.

**Step 0 - answer the question that gates everything else**

- [x] Can `std::format` be kept under `-fno-exceptions`? **Yes.** Steps 3 and 4 are not blocked. Measured, not reasoned about:
      a program calling `std::vformat_to` with a runtime format string was built three ways and run.

      | Toolchain | Flags | Good format | Malformed format |
      |---|---|---|---|
      | GCC 13 / libstdc++ | `-fno-exceptions` | works | `SIGABRT`, exit 134 |
      | Clang 20 / libstdc++ | `-fno-exceptions` | works | `SIGABRT`, exit 134 |
      | MSVC 2022 | `/EHsc- /D_HAS_EXCEPTIONS=0` | works | `0xC0000409`, fail-fast |

      So the format machinery compiles and links without exceptions everywhere; what changes is that a malformed **runtime**
      format string ends the process instead of throwing something catchable. Since `Format`, `AppendFormat` and `Logger` all
      take a `StringViewUtf8` decided at runtime, none of them get a compile-time check today.

      The mitigation, if that trade is unwanted: a literal format string routed through `std::format_string` is checked at
      compile time. Verified - `std::format("value {", argc)` is a hard compile error on GCC, before any of this matters at
      runtime. Taking a compile-time-checked format type where the caller passes a literal would move most malformed formats to
      build time and leave only genuinely dynamic formats able to abort.

      Caveat worth recording: `_HAS_EXCEPTIONS=0` is not a supported configuration of Microsoft's standard library, whatever
      this one program did. Treat the MSVC row as "it worked here", not as a guarantee.

**Step 1 - factories alongside the constructors (additive, nothing breaks)**

Same shape as `SharedPtr::Create` in `ccc93d8`: one private `Construct` returning `ErrorCode`, a throwing
constructor over it, and a `[[nodiscard]] static Expected<T, ErrorCode> Create(...)` next to it.

- [x] `DynamicArray::Create` and `TryClone` - `dynamic-array.h:745,767,789,828,853,1110`. Four private `Construct` overloads hold
      the allocating half of each constructor; the constructors throw over them and `Create` branches on them, so the two cannot
      drift. `Clone` is now written over `TryClone`.
- [x] `String::Create` and `TryClone` - `string.h:1305,1327,1342,1347,1364,1381,1666`. Same shape, over the `InitStorage` that was
      already there.
- [x] ~~`String::TryAppend`~~ - not needed. `Append` already returns `ErrorCode` for all six shapes and is exactly what
      `operator+=` throws over. A `TryAppend` would have been an alias for it.
- [x] `HashMap::Create` - `hash-map.h:357,367,373,384,390`
- [x] `HashSet::Create` - `hash-set.h:286`
- [x] `Deque::Create` and `TryClone`
- [x] Each one needs a `NullAllocator` test. Every allocation bug this document records was invisible for want
      of exactly that test.

Two things the factories turned up, both of the kind this document keeps finding:

- **`Deque::Initialize` never checked its allocation.** It called `Allocate` and then placement-new'd into the result, so a
  `Deque` built on an exhausted allocator wrote through a null pointer rather than throwing. All three constructors went through
  it, and so did `Clone`, which additionally dropped the `ErrorCode` from its own `Reserve` before writing into the copy. So
  `Deque` was the one container whose constructors did not report a failed allocation at all - the plan listed it as needing only
  a `Create`. `Initialize` returns `ErrorCode` now, the constructors throw over it, and `Clone` is written over `TryClone`.
- **`Reserve(0)` is not free on the hash containers.** The first attempt built the empty object for `Create` with
  `HashMap(0, allocator)`, which still asks for the smallest table and therefore threw on a `NullAllocator` - from inside the
  factory whose whole purpose is not to. Both containers got a private tag constructor that touches no allocator. `Deque` needed
  the same, plus care that `Create(allocator)` uses `k_default_capacity` rather than routing through the count overload, which
  rounds up to a power of two and would have given an empty deque a slot its constructor never allocates.

**Step 2 - make `Expected` worth leaning on**

Doing this before step 1 spreads is cheaper than retrofitting every call site afterwards.

- [x] `Expected()` default-constructs into the *value* state - `include/opal/container/expected.h:77`. Deleted. The two
      specializations already disagreed: `Expected<T&, E>` deletes its default constructor and `Expected<T, E>` did not, so
      consistency alone forced a choice. An Expected that holds neither reports success with a value nobody produced, which is
      the failure mode this whole document is about. `std::expected` does default into the value state, so this is a deliberate
      departure rather than an oversight.
- [x] `GetValue` and `GetError` assert rather than being checkable; there is no `ValueOr` on the error side. Added `GetErrorOr`
      to both specializations, and the asserting accessors now document the checked alternative next to them.
- [x] `AndThen` and `Map`, rvalue-qualified so move-only values pass through without a copy. `Map` wraps a plain result,
      `AndThen` takes a function that already returns an `Expected`.
- [x] **The move constructor was assigning into an unconstructed union member.** `m_value = Move(other.m_value)` runs `T`'s
      move assignment over storage that never held a `T`, so a value that owns memory - `String`, `DynamicArray` - would free a
      garbage pointer. The move *assignment* operator already used placement new for the same transition; only the constructor
      was wrong. Nothing caught it because every `Expected` in the library is built in place by return-value elision, and the
      one existing move-construction test used `int32_t`. Tests now move-construct with a 64 byte `std::string` on both the
      value and the error side.
- [x] The seven `String::Insert` "Memory allocation failed" tests inserted two characters into an empty string, which fits the
      small buffer, so the `NullAllocator` was never asked for anything and `REQUIRE_NOTHROW` passed on a successful insert.
      They insert past the small buffer now and assert `ErrorCode::OutOfMemory`. Found by deleting the default constructor:
      they were the only code that needed it.

**Step 3 - contract violations become asserts plus `Try*`**

This is where the exception count actually drops. Bigger behavioural change than step 1: an out-of-range index
aborts instead of unwinding. Defensible for a real-time engine, but it is a product decision.

- [x] `Try*` accessors first, so callers have somewhere to go - covers §5's `TryGetBool`/`TryAt`/`TryFind` and
      §10's `Variant::TryGet`. Two new codes went with them: `ErrorCode::TypeMismatch` and `ErrorCode::KeyNotFound`.
- [ ] Then convert the throwing forms listed in §10 to ~~`OPAL_ASSERT`~~ **`OPAL_VERIFY`**.

      **`OPAL_ASSERT` was the wrong tool and the reasoning above is wrong with it.** This section says an out-of-range index would
      "abort instead of unwinding". It would not: `OPAL_ASSERT` expands to nothing outside `OPAL_DEBUG`, so release builds would
      have had no bounds check at all and an out-of-range index would read past the end silently. That is not a change from
      throwing to aborting, it is a change from always checked to checked only in debug, and it is the opposite of what the rest
      of this document has been doing.

      Decided instead: `OPAL_VERIFY`, which checks in **every** build and routes a failure through a settable handler that
      aborts by default. Same shape as the fatal log handler from §7. The mechanism is in - `include/opal/assert.h`,
      `src/assert.cpp`, `SetContractViolationHandler` / `GetContractViolationHandler` - along with `OPAL_EXCEPTIONS` in
      `defines.h`, which follows the compiler's own `__cpp_exceptions` / `_CPPUNWIND` so a `-fno-exceptions` build does not have
      to be told twice.

- [x] The conversion itself. 33 sites across `dynamic-array.h`, `deque.h`, `hash-map.h`, `string.h`, `string-view.h`,
      `in-place-array.h`, `variant.h` and `casts.h` now use `OPAL_VERIFY`. Only contract violations moved; the
      `OutOfMemoryException` throws in constructors stay, since those are step 4's problem.

      **§10 is wrong about the `Expected` siblings already existing.** It lists the bounds checks as safe to convert because
      `TryAt`, `Front` and `Back` are available. Only `Deque` and `InPlaceArray` had `TryAt`; `DynamicArray`, `String` and
      `StringView` had none, and `DynamicArray::Front`/`Back` threw rather than returning `Expected`. Converting those first
      would have left callers with a check that ends the program and no way to ask the question safely. Added: `TryAt` on
      `DynamicArray`, `String` and `StringView`, and `TryFront`/`TryBack` on `DynamicArray`.

      Test coverage moved where it could and was dropped where it could not. `operator[]`, `At`, `Front`, `Back` and
      `Variant::Get` are all checked through their `Try*` siblings now. What is no longer testable in-process, because the check
      ends the program and Catch2 has no death tests: `DynamicArray::Insert`/`Emplace` with an out of range position or a
      reversed range, and `Clone`/`Visit`/`VisitPartial` on a moved-from `Variant`. Those five sections are gone rather than
      left to abort the runner. A `Variant` test that the moved-from state is reported through `IsActive` and `TryGet` replaces
      part of it.

      Also gone from the messages: the numbers. `OutOfBoundsException` carried the index and the range it violated, and several
      tests asserted on that text. `OPAL_VERIFY` takes a fixed description, so a failure now names the check rather than the
      values that failed it.

**Step 4 - gate the remainder behind `OPAL_EXCEPTIONS`**

- [ ] Compile the throwing constructors out when it is off, leaving only the factories
- [ ] Decide what `HandleFatal` does without exceptions (§7 has the same question already)
- [ ] `Opal::Exception` derives from `std::exception` as of `3453b1e`, which couples toward std exceptions.
      Fine while they are enabled; revisit if step 4 lands.

- [x] Implement hash map based on swiss table by Google https://abseil.io/about/design/swisstables.
- [ ] Implement memory allocator based on https://github.com/mattconte/tlsf.

## String correctness

Bugs found during a review pass of `include/opal/container/string.h`. The first seven were
reproduced against a debug build.

- [x] `Insert(pos, other, other_start_pos, count)` ignores `other_start_pos` (`string.h:1794`, `string.h:1813`).
      `count = other_size` should subtract `other_start_pos`, the copy loop should read
      `other[other_start_pos + i - start_pos]`, and `count > other_size - other_start_pos` is unchecked.
      Repro: `Insert(1, "0123456789", 5, 3)` on `"AB"` yields `"A012B"`, expected `"A567B"`.
- [x] `ReverseFind(haystack, const char*, ...)` is off by one (`string.h:2803`). Clamps to
      `GetSize() - 1` where the `StringClass` overload clamps to `GetSize()`, so a match ending on the
      last code unit is missed. `ReverseFind("abc", "bc")` returns `k_npos`, the `StringClass`
      overload returns 1.
- [x] `Split` and `SplitToArray` assume a one-code-unit delimiter (`string.h:2928`, `string.h:2960`).
      Both advance by `pos + 1` instead of `pos + delimiter.GetSize()`.
      `Split("a::b", "::")` produces a second part of `":b"`.
- [x] `GetSubString` rejects `start_pos == size` (`string.h:2855`), so `Split`/`SplitToArray` drop a
      trailing empty field and report failure. `SplitToArray("a,b,", ",")` returns false with 2 parts.
      Guard should be `start_pos > str.GetSize()`.
- [x] `NumberToString` leading zeros do nothing for hex and octal (`string.h:2995`). The format is
      built as `"%0" PRIX32` - a zero flag with no width. Needs an explicit width.
      `NumberToString<u32>(0xAB, Hexadecimal, true)` returns `"AB"`, not `"000000AB"`.
- [x] `String` has no geometric growth, making `Append` quadratic (`string.h:1434`). Every append
      calls `Reserve(sz + n + 1)`, which allocates exactly that. 2000 single-character appends cause
      1977 reallocations. Mirror `DynamicArray`'s `k_resize_factor`.
- [x] `Append(const value_type*)` and `Insert(pos, const CodeUnitType*, count)` break on self-aliasing
      (`string.h:1571`). `Reserve` reallocates and the caller's pointer dangles before the `memcpy`.
- [x] `Allocate` never checks the result of `alloc->Alloc` (`string.h:2154`). `MallocAllocator` returns
      `nullptr` on failure, so the documented `OutOfMemoryException` is really a null dereference.
      `DynamicArray` already checks (`dynamic-array.h:1428`).
- [x] The substring constructor has no bounds check on `pos` (`string.h:976`). `other.GetSize() - pos`
      underflows, so `String(src, 10)` on a 3-character string requests a 2^64 allocation.
- [x] Every `Assign` overload can double free (`string.h:1183`, `1211`, `1251`, `1294`, `1331`). They
      deallocate, then allocate, leaving the freed pointer in `m_storage.large.data` with the tag still
      marked large - a throw from `Allocate` frees it again in the destructor. Allocate before freeing.
- [x] `count + 1` overflows when `count == k_npos` in `String(str, count)` and the `Assign` overloads,
      which then take the SSO branch and loop `k_npos` times.
- [x] `StringToNumber(const StringClass&)` reads past the end for `StringView` (`string.h:3162`).
      `strtoll` needs a null terminator that a view has no obligation to provide. The local `end` is
      initialized and then immediately overwritten by `strtoll`.
- [x] `Transcode` requires the caller to pre-size the output and says so nowhere (`string.h:2180`).
      `output_span` spans `output.GetSize()` and is never grown; the tests only pass because they call
      `Resize(200)` first. On `InsufficientSpace` the output is left partially written.
      Fixed by growing the output on demand. The callers in `src/` still pre-size with guesses
      (`path.GetSize() * 2`, `MAX_PATH`), which is now pointless and could be dropped.
- [x] `At()` constructs `OutOfBoundsException(pos, 0, sz - 1)` on an empty string (`string.h:1357`,
      `string.h:1368`), underflowing the upper bound in the message.
- [x] `using const_pointer = CodeUnitType*;` is missing its `const` (`string.h:128`).
- [x] The explicit instantiation at `string.h:603` precedes the member definitions that start at
      `string.h:938`, so the instantiation point sees no definitions. Use `extern template` in the
      header and `template class` in `src/string.cpp`.

## String API consistency

- [x] `StringView` inverts the bounds-checking convention: `operator[]` throws and `At()` is unchecked
      (`string-view.h:87`, `string-view.h:100`), the opposite of `String`. Generic `StringLike` code
      changes behavior with the type.
      Resolved in favour of the `StringView` convention: `operator[]` throws, `At()` is unchecked, so
      the default way to index is the safe one. `String` was changed to match.
- [ ] `DynamicArray`, `InPlaceArray` and `Deque` still use the opposite convention, where `At()` is the
      checked accessor. Either flip them the same way or accept that strings and arrays differ.
- [ ] `GetSubString`, `Split`, `SplitToArray` and the five `operator+` overloads are constrained on
      `StringLike` but hard error for `StringView`, which has no allocator-taking constructor, `Clone`
      or `operator+=`.
- [x] `Reserve` and `GetCapacity` count the null terminator, so `Reserve(n)` holds `n - 1` code units.
      Deviates from both `std::string` and `DynamicArray`, and is undocumented.
- [x] `Reserve(0)` throws `InvalidArgumentException`, which makes `Reserve(computed_size)` unsafe in
      generic code.
- [x] Doc and code disagree on error codes: `Assign(const CodeUnitType*, count)` documents `BadInput`
      and returns `InvalidArgument` (`string.h:260` vs `string.h:1281`), same for
      `Insert(pos, str, count)` (`string.h:422` vs `string.h:1746`).
      `ErrorCode::BadInput` does not exist at all; all six mentions in `string.h` were stale.
- [ ] The same stale `ErrorCode::BadInput` appears in `deque.h` (3 mentions), `string-encoding.h` (5)
      and `sort/key-indexed-counting.h` (1).
- [x] `Insert(pos, const String& other, ...)` shifts data before reading it, so inserting a string into
      itself corrupts the result. No self-check. `Insert(iterator, InputIt, InputIt)` and its
      `const_iterator` overload have the same hole, and unlike `Append(InputIt, InputIt)` and
      `Assign(InputIt, InputIt)` they do not return `SelfNotAllowed` for iterators into the string.
- [ ] Pin the deliberate `std::string` deviations in tests, or fix them: `Assign(other, pos, count)`
      and `Erase(pos, count)` reject `pos == size`; `Erase(first, last)` rejects `first == End()`, so an
      empty range erase at the end fails; `Find` with an empty needle at `start_pos == size` returns
      `k_npos` rather than `size`.

## String missing API

- [ ] `Clear`, `PopBack`, `ShrinkToFit`, `Replace`, `Contains`.
- [ ] Relational operators or `operator<=>`. Only `operator==` exists, so strings cannot be sorted
      without calling `Compare`.
- [ ] `operator==(const CodeUnitType*)`.
- [ ] Reverse iterators, to match `DynamicArray`.
- [ ] `std::initializer_list` constructor and assignment, to match `DynamicArray`.
- [ ] A member `GetSubString`.
- [ ] Explicitly `= delete` the copy constructor and copy assignment. They are already implicitly
      deleted by the move constructor, so this is only about following the convention in `CLAUDE.md`.

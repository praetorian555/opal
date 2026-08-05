# ArrayView and StringView - audit

A pass over `include/opal/container/array-view.h` and `include/opal/container/string-view.h` for correctness and
completeness, and over how consistent the two are with each other and with the containers they view.

Findings marked **reproduced** were compiled and run against the library rather than read off the page. Three things that
looked like findings were checked and are not; they are recorded at the bottom so the next pass does not chase them again.

Test coverage is thin: `test/array-view-test.cpp` has 11 test cases and `test/string-view-test.cpp` has 8. Nothing in either
file passes a size that does not fit.

---

## Correctness

### 1. `ArrayView`'s const iterator hands out a mutable reference

`ArrayViewConstIterator` takes its `reference` and `pointer` from `MySpan::reference` and `MySpan::pointer`, which are `T&`
and `T*`, not `const T&` and `const T*`. Every const accessor returns one: `begin() const`, `cbegin()`, `cend()`.

**Reproduced.** This compiles, and it writes:

```cpp
const ArrayView<int> const_view(data);
*const_view.begin() = 99;       // data[0] is now 99
```

`StringConstIterator` gets this right - it uses `StringClass::const_reference`. `ArrayViewConstIterator` was written from the
same shape with the two aliases left pointing at the mutable ones.

- [x] `ArrayViewConstIterator::reference` and `::pointer` become `MySpan::const_reference` and `MySpan::const_pointer`
- [x] A test that a const view cannot be written through, as a compile-time check

### 2. `SubSpan` and `SubView` accept a count that overflows

Both bounds checks add before they compare, and both operands are unsigned 64-bit:

```cpp
if (offset + count > m_size)                    // array-view.h:460
if (count == k_npos || pos + count > m_size)    // string-view.h:176
```

A large `count` wraps the sum to a small number, the check passes, and the caller gets a view over memory that was never
theirs.

**Reproduced**, on a 5-element view and a 5-code-unit view:

```
SubSpan(3, u64max-1) accepted: 1, returned size 18446744073709551614
SubView(2, npos-1)   accepted: 1, returned size 18446744073709551614
```

`SubView` catches `k_npos` exactly, which is why the second case uses `k_npos - 1`.

- [x] Compare without adding: `offset > m_size || count > m_size - offset`
- [x] Same in `SubView`, keeping the `k_npos` clamp
- [x] Tests for both, since neither file has one that passes a size that does not fit

### 3. `RemovePrefix` and `RemoveSuffix` walk off the view

Neither checks `n` against `m_size`. `m_size -= n` wraps and `m_data += n` moves past the end.

**Reproduced**, on a 5-code-unit view:

```
RemovePrefix(10) on size 5 gives size 18446744073709551611
RemoveSuffix(10) on size 5 gives size 18446744073709551611
```

`std::string_view` calls this undefined. This library does not have that option: `operator[]` one screen above these two
checks its argument with `OPAL_VERIFY` in every build, and says so in its documentation. These two are the same kind of
caller mistake and should fail the same way.

- [x] `OPAL_VERIFY(n <= m_size, ...)` in both

### 4. `ArrayView::operator[]` is the only unchecked subscript left

`DynamicArray::operator[]` checks with `OPAL_VERIFY` (`dynamic-array.h:1287`). `StringView::operator[]` checks with
`OPAL_VERIFY` (`string-view.h:95`). `ArrayView::operator[]` does not check at all, and its documentation advertises that as
the point of it.

Commit 39eb3b2 moved the library to checking contracts in every build. This is the one subscript that did not follow.

- [x] `OPAL_VERIFY` in both overloads, and update the comment that promises no checking

### 5. Constructing from an iterator dereferences it before looking at the count

```cpp
ArrayView(InputIt first, size_type count) : m_data(&(*first)), m_size(count)
```

`&(*first)` runs whatever `operator*` the iterator has, including when `count` is zero and `first` is an end iterator. The
two-iterator overload has the same shape.

Read from the code. Building an empty view out of `array.end()` produced a past-the-end pointer without a sanitizer report,
so this is a latent problem rather than a reproduced crash - it costs whatever the iterator's `operator*` costs, which for a
checked iterator would be a failure.

- [x] Take the address without dereferencing where the iterator allows it, or skip the dereference when `count` is zero

---

## Completeness and consistency

The two types are siblings and are documented as such, but they disagree with each other on three things a caller has to
learn separately:

- [x] **`At` means the opposite thing in each.** `ArrayView::At` is the *checked* one and returns
      `Expected<T&, ErrorCode>`. `StringView::At` is the *unchecked* one and `TryAt` is the checked one. Whichever way it
      goes, the pair should agree, and `TryAt` is the name the rest of the library settled on
- [x] **`operator==` means the opposite thing in each.** `ArrayView` compares the pointer and the size, so two views over
      equal contents are unequal. `StringView` compares contents. **Reproduced**: same contents in different storage give 0
      and 1 respectively. `ArrayView`'s behaviour is documented, so this is a decision to make rather than a bug to fix -
      `std::span` deliberately has no `==` at all, which is the third option
- [x] `StringView::Min` is a public static member function of the view. Made private. The only thing outside the class using
      it was the free `GetSubString`, which now clamps without it. `String::Min` was public for the same no-longer-existing
      reason and moved to the private section as well, below the SSO constants that shared its access section

Missing from `ArrayView`:

- [x] `First(count)` and `Last(count)`, and a one-argument `SubSpan(offset)` for the rest of the view. `StringView::SubView`
      already defaults its count. `SubSpan` still refuses a count that runs past the end rather than clamping to it, which is
      what it always did and what its test expects; only `k_npos` means the rest
- [x] Reverse iterators. Neither view has `rbegin`/`rend`. Both have them now, as `std::reverse_iterator` over the existing
      iterators, which model `std::random_access_iterator`. `DynamicArray`'s hand-written reverse iterator lives in
      `dynamic-array.h` and reusing it would have made a lightweight view header pull in the whole array container
- [x] `GetSizeInBytes()`

Missing from `StringView`:

- [x] ~~`StartsWith`, `EndsWith` and `Contains`. `Contains` exists on `String` only, and nothing in the library has the other
      two.~~ **Wrong as written.** Free `StartsWith` and `EndsWith` on `StringLike` already existed at `string.h:1237,1247`;
      the grep behind this item required leading whitespace and so only saw member declarations. What was really missing was
      a free `Contains`, which a view had no way to reach, and a needle that is not a whole `StringClass`. Added: free
      `Contains` and the `const value_type*` and single code unit needles for all three
- [x] `ToString()` calls the throwing `String` constructor and cannot be given an allocator. Every other type that allocates
      grew a `Create` during the exceptions work; this one did not. It takes an allocator now, and `TryToString` reports
      `ErrorCode::OutOfMemory` instead of throwing

---

## Checked and not findings

Recorded so the next pass does not spend time on them:

- **The iterators do work with standard algorithms** despite defining no `iterator_category`. C++20 synthesizes one, and
  `std::random_access_iterator<ArrayView<int>::iterator>` is satisfied. Verified by compiling it
- **The `// TODO: Check if underlying array is contiguous` at `array-view.h:338` overstates the risk.** `Deque<int>` does not
  convert: the constructor's `Range` and `ValueTypeGetter` constraints reject it. Verified by compiling it and reading the
  error. The TODO is still worth keeping for a container that satisfies both and is not contiguous
- **`operator==(const CodeUnitType*)` and `operator<=>(const CodeUnitType*)` handle a null pointer** the way their
  documentation says. `GetStringLength` returns 0 for null (`string.h:3822`), so the `str ? ... : 0` in the view's
  constructor is redundant rather than load-bearing

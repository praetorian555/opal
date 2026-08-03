# Containers

Headers: `opal/container/dynamic-array.h`, `opal/container/deque.h`, `opal/container/array-view.h`, `opal/container/in-place-array.h`, `opal/container/string.h`, `opal/container/string-view.h`, `opal/container/hash-map.h`, `opal/container/hash-set.h`, `opal/container/priority-queue.h`, `opal/container/scope-ptr.h`, `opal/container/shared-ptr.h`, `opal/container/ref.h`, `opal/container/expected.h`, `opal/container/iterator.h`

Opal provides a complete set of containers designed for game engine and real-time systems. All containers follow these conventions:

- **Pluggable allocators.** Every owning container accepts an optional `AllocatorBase*`. When omitted, `GetDefaultAllocator()` is used. See [memory.md](memory.md) for the allocator system.
- **Move-only semantics.** Copy constructors are deleted on owning containers. Use `std::move` to transfer ownership or `.Clone()` to deep copy. See the [Clone API](../README.md#clone-api) section in the README.
- **Iterators.** All sequence and associative containers provide `begin()`/`end()`/`cbegin()`/`cend()` for range-based for loops.

## Quick Start

```cpp
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/scope-ptr.h"

// Dynamic array with default allocator
Opal::DynamicArray<int> numbers = {1, 2, 3};
numbers.PushBack(4);

// Hash map
Opal::HashMap<Opal::StringUtf8, int> scores;
scores.Insert("Alice", 100);
scores.Insert("Bob", 85);

// Unique ownership pointer
Opal::ScopePtr<Widget> widget = Opal::MakeScoped<Widget>(nullptr, "button", 32);
```

---

## DynamicArray

Header: `opal/container/dynamic-array.h`

Variable-size array backed by a contiguous allocation, similar to `std::vector`. Grows automatically on insertion.

```cpp
Opal::DynamicArray<int> arr;                        // Empty, default allocator
Opal::DynamicArray<int> arr(10);                    // 10 default-initialized elements
Opal::DynamicArray<int> arr(10, 42);                // 10 elements, all 42
Opal::DynamicArray<int> arr = {1, 2, 3};            // Initializer list
Opal::DynamicArray<int> arr(&alloc);                // Explicit allocator
Opal::DynamicArray<int> arr(data_ptr, count);       // From raw pointer + count
```

### Element Access

```cpp
arr[0];              // Unchecked access
arr.At(0);           // Checked access (throws OutOfBoundsException)
arr.Front();         // First element
arr.Back();          // Last element
arr.GetData();       // Raw pointer to underlying storage
```

### Modification

```cpp
arr.PushBack(42);                  // Append, returns ErrorCode
arr.EmplaceBack(arg1, arg2);       // Construct in place, returns Expected<T&, ErrorCode>
arr.PopBack();                     // Remove last
arr.Insert(pos, value);            // Returns Expected<iterator, ErrorCode>
arr.Insert(pos, count, value);     // Returns Expected<iterator, ErrorCode>
arr.Emplace(pos, arg1, arg2);      // Returns Expected<iterator, ErrorCode>
arr.Erase(pos);                    // Remove at position
arr.EraseWithSwap(pos);            // O(1) erase by swapping with last element
arr.Remove(value);                 // Remove first matching value
arr.RemoveWithSwap(value);         // O(1) remove first matching value
arr.Append(other_container);       // Returns ErrorCode
arr.Assign(count, value);          // Returns ErrorCode
arr.Clear();                       // Remove all elements
```

### Size and Capacity

```cpp
arr.GetSize();                     // Current element count
arr.GetCapacity();                 // Allocated capacity
arr.IsEmpty();                     // True if size == 0
arr.Reserve(100);                  // Pre-allocate capacity, returns ErrorCode
arr.Resize(50);                    // Change size, returns ErrorCode
arr.Resize(50, 0);                 // Change size, returns ErrorCode
arr.ShrinkToFit();                 // Give up spare capacity, returns ErrorCode
```

### Allocator

```cpp
arr.GetAllocator();                // Get current allocator
arr.SetAllocator(&new_alloc);      // Change allocator (moves data), returns ErrorCode
```

Mutators that can allocate return `ErrorCode`, or `Expected<..., ErrorCode>` when they also have a
value to hand back. Constructors, `Clone` and `operator=` have nowhere to put a code and still throw
`OutOfMemoryException`. Out-of-range access throws, since that is a caller bug rather than a
recoverable condition.

---

## Deque

Header: `opal/container/deque.h`

Double-ended queue backed by a circular buffer. Capacity is always a power of 2 for efficient index wrapping.

```cpp
Opal::Deque<int> dq;                    // Empty, default allocator
Opal::Deque<int> dq(16);                // 16 default-initialized elements
Opal::Deque<int> dq(16, 0);             // 16 elements, all 0
Opal::Deque<int> dq(&alloc);            // Explicit allocator
```

### Element Access

```cpp
dq[0];                // Unchecked access
dq.At(0);             // Returns Expected<T&, ErrorCode>
dq.Front();           // Returns Expected<T&, ErrorCode>
dq.Back();            // Returns Expected<T&, ErrorCode>
```

### Modification

```cpp
dq.PushBack(42);            // Returns ErrorCode
dq.PushFront(42);           // Returns ErrorCode
dq.PopBack();               // Returns ErrorCode
dq.PopFront();              // Returns ErrorCode
dq.Insert(pos, value);      // Returns Expected<iterator, ErrorCode>
dq.Erase(pos);              // Returns Expected<iterator, ErrorCode>
dq.Clear();
```

### Size and Capacity

```cpp
dq.GetSize();
dq.GetCapacity();
dq.IsEmpty();
dq.Reserve(256);             // Returns ErrorCode
dq.Resize(100);              // Returns ErrorCode
```

---

## ArrayView

Header: `opal/container/array-view.h`

Non-owning, read/write view over contiguous memory. Copyable. Similar to `std::span`.

```cpp
Opal::DynamicArray<int> arr = {1, 2, 3};
Opal::ArrayView<int> view(arr);                     // From container
Opal::ArrayView<int> view(arr.GetData(), 3);        // From pointer + count
Opal::ArrayView<int> view(arr.begin(), arr.end());  // From iterator pair

int raw[4] = {10, 20, 30, 40};
Opal::ArrayView<int> view(raw);                     // From C array
```

### Usage

```cpp
view.GetData();                    // Raw pointer
view.GetSize();                    // Element count
view.IsEmpty();
view[0];                           // Unchecked access
view.At(0);                        // Returns Expected<T&, ErrorCode>
view.Front();                      // Returns Expected<T&, ErrorCode>
view.Back();                       // Returns Expected<T&, ErrorCode>
view.SubSpan(offset, count);       // Returns Expected<ArrayView<T>, ErrorCode>
```

### Byte Views

Free functions to reinterpret objects or containers as byte arrays:

```cpp
Opal::ArrayView<const Opal::u8> bytes = Opal::AsBytes(my_struct);
Opal::ArrayView<Opal::u8> writable = Opal::AsWritableBytes(my_struct);
Opal::ArrayView<const Opal::u8> bytes = Opal::AsBytes(my_array);
```

---

## InPlaceArray

Header: `opal/container/in-place-array.h`

Fixed-capacity array with stack storage. Size is always `N`. Similar to `std::array`.

```cpp
Opal::InPlaceArray<int, 4> arr;                    // Uninitialized
Opal::InPlaceArray<int, 4> arr = {1, 2, 3, 4};    // Initializer list
```

### Usage

```cpp
arr.GetSize();           // Always N
arr[0];                  // Unchecked access
arr.At(0);               // Returns Expected<T&, ErrorCode>
arr.Front();
arr.Back();
arr.GetData();           // Raw pointer
arr.Fill(0);             // Set all elements to a value
```

---

## Optional

Header: `opal/container/optional.h`

Holds either a single value of type `T` or nothing. When empty, no `T` is constructed and no heap
allocation occurs; storage for `T` lives inline. Following the project's "Clone instead of Copy"
pattern, copy is deleted — use `Clone()` or `Move()`.

```cpp
Opal::Optional<int> empty;                       // No value
Opal::Optional<int> a = Opal::k_null_opt;        // No value (explicit)
Opal::Optional<int> b(42);                        // Holds 42
Opal::Optional<std::string> c(Opal::k_in_place, 3, 'a');  // Constructs "aaa" in place
```

### Access

```cpp
b.HasValue();            // true
static_cast<bool>(b);    // true
b.GetValue();            // 42 (asserts if empty)
*b;                      // 42
c->size();               // operator-> forwards to the value
b.GetValueOr(0);         // 42 if present, otherwise 0
b == Opal::k_null_opt;   // false
```

### Modification

```cpp
b.Emplace(7);            // Destroys current value, constructs new one in place, returns T&
b.Reset();               // Destroys value, becomes empty
b = Opal::k_null_opt;    // Same as Reset()
auto clone = b.Clone();  // Deep copy (POD copies, non-POD calls .Clone())
```

### Reference Specialization

`Optional<T&>` holds a non-owning pointer to a `T` (or nothing). Move transfers the reference and
empties the source; `Clone()` shares the same referenced object.

```cpp
int value = 10;
Opal::Optional<int&> ref(value);
ref.GetValue() = 20;     // value is now 20
ref.Reset();             // Empty, value untouched
```

### Hashing

`Optional<T>` is hashable via `Opal::Hasher` when `T` is hashable, so it can be used as a
`HashMap`/`HashSet` key. Empty optionals hash to a fixed sentinel value.

---

## String

Headers: `opal/container/string.h`, `opal/container/string-view.h`, `opal/container/string-encoding.h`, `opal/container/string-hash.h`

Full-featured string with encoding support. The primary specialization is `StringUtf8`.

### Specializations

| Type | Code Unit | Encoding |
|------|-----------|----------|
| `StringUtf8` | `char8` | UTF-8 |
| `StringUtf32` | `uchar32` | UTF-32 LE |
| `StringWide` | `char16` | UTF-16 LE |
| `StringLocale` | `char8` | Platform locale |

```cpp
Opal::StringUtf8 str("Hello");                    // From null-terminated string
Opal::StringUtf8 str(5, 'x');                     // "xxxxx"
Opal::StringUtf8 str(other, 2);                   // Substring from position 2
Opal::StringUtf8 str(ptr, count);                 // From pointer + count
Opal::StringUtf8 str(&alloc);                     // Explicit allocator
```

### Modification

```cpp
str.Append(" World");              // Returns ErrorCode
str.Append(other_str);             // Returns ErrorCode
str.Append(5, '!');                // Returns ErrorCode
str.Assign(other_str);             // Overwrite, returns ErrorCode
str += " suffix";                  // Throws OutOfMemoryException on failure
str.Insert(pos, "text");           // Returns Expected<iterator, ErrorCode>
str.Erase(start_pos, count);       // Returns Expected<iterator, ErrorCode>
str.Replace(pos, count, other);    // Returns Expected<iterator, ErrorCode>
str.ReplaceAll(needle, other);     // Returns Expected<size_type, ErrorCode>
str.ReplaceFirst(needle, other);   // Returns Expected<bool, ErrorCode>
str.Resize(new_size);              // Returns ErrorCode
str.Reserve(capacity);             // Returns ErrorCode
str.ShrinkToFit();                 // Give up spare capacity, returns ErrorCode
str.Trim();                        // Cut the size back to the first null terminator
str.Strip();                       // Remove leading and trailing whitespace
str.Reverse();
str.Clear();
```

Mutators that can allocate return `ErrorCode`, or `Expected<..., ErrorCode>` when they also have a
value to hand back. Constructors, `Clone`, `operator=` from an initializer list and `operator+=` have
nowhere to put a code and still throw `OutOfMemoryException`. Out-of-range access throws, since that
is a caller bug rather than a recoverable condition.

### Access

```cpp
str.GetData();                     // Null-terminated code unit pointer
str.GetSize();                     // Size in code units
str.GetCapacity();
str.IsEmpty();
str[0];                            // Unchecked access by code unit index
str.At(0);                         // Checked access
str.Front();                       // Returns Expected
str.Back();                        // Returns Expected
```

### StringView

Non-owning, read-only view. Copyable. Implicitly constructible from `String`.

```cpp
Opal::StringViewUtf8 view("Hello");               // From null-terminated string
Opal::StringViewUtf8 view(str);                   // From String

view.GetData();
view.GetSize();
view[0];
view.SubView(offset, count);       // Returns Expected<StringView, ErrorCode>
view.RemovePrefix(n);              // Shrink from front
view.RemoveSuffix(n);              // Shrink from back
```

### String Comparison

```cpp
Opal::Compare(str1, str2);        // Returns Expected<i32, ErrorCode>
// Negative: str1 < str2, Zero: equal, Positive: str1 > str2
```

### String Hashing

`StringHash<MyString>` provides a hash functor for use with `HashMap`/`HashSet`.

### String Formatting

Header: `opal/container/string-format.h`

Type-safe string formatting using C++20 `std::format` syntax. Two free functions are provided:

- `Format(fmt, args...)` — returns a new `StringUtf8` with the formatted result.
- `AppendFormat(output, fmt, args...)` — appends the formatted result to an existing `StringUtf8`.

When called with no variadic arguments, the format string is appended directly without parsing.

```cpp
#include "opal/container/string-format.h"

// Create a formatted string
Opal::StringUtf8 msg = Opal::Format("hello {}", "world");         // "hello world"

// Numeric formatting
Opal::StringUtf8 num = Opal::Format("{} + {} = {}", 1, 2, 3);    // "1 + 2 = 3"
Opal::StringUtf8 flt = Opal::Format("{:.2f}", 3.14);              // "3.14"
Opal::StringUtf8 hex = Opal::Format("{:#x}", 255);                // "0xff"
Opal::StringUtf8 pad = Opal::Format("{:04d}", 42);                // "0042"

// Append to an existing string
Opal::StringUtf8 str("prefix: ");
Opal::AppendFormat(str, "value={}", 99);                           // "prefix: value=99"

// No args — literal passthrough
Opal::StringUtf8 lit = Opal::Format("literal");                   // "literal"
```

---

## HashMap

Header: `opal/container/hash-map.h`

Open-addressing hash map using the Swiss tables algorithm with SSE SIMD-optimized control byte probing. Stores `Pair<KeyType, ValueType>` entries.

```cpp
Opal::HashMap<Opal::StringUtf8, int> map;                     // Default capacity
Opal::HashMap<int, int> map(1024);                             // Pre-sized
Opal::HashMap<int, int> map(1024, &alloc);                     // Explicit allocator
Opal::HashMap<int, int> map = {{1, 10}, {2, 20}};             // Initializer list
Opal::HashMap<int, int> map(ArrayView<Pair<int, int>>(pairs)); // From array view
```

### Lookup

```cpp
map.Find(key);                     // Returns iterator (end() if not found)
map.Contains(key);                 // Returns bool
map.GetValue(key);                 // Returns reference (throws if not found)
```

### Modification

```cpp
map.Insert(key, value);            // Insert or update, returns ErrorCode
map.Erase(key);                    // Remove by key, returns ErrorCode
map.Erase(it);                     // Remove by iterator, returns ErrorCode
map.Erase(first, last);            // Remove range, returns ErrorCode
map.Clear();
map.Reserve(capacity);             // Returns ErrorCode
```

### Size and Capacity

```cpp
map.GetSize();
map.GetCapacity();
map.GetGrowthLeft();               // Insertions left before the map has to grow
map.IsEmpty();
```

### Copying

```cpp
auto clone = map.Clone();          // Deep copy, same allocator
auto clone = map.Clone(&alloc);    // Deep copy onto another allocator
```

### Conversion

```cpp
map.ToArray();                     // DynamicArray<Pair<K, V>>
map.ToArrayOfKeys();               // DynamicArray<K>
map.ToArrayOfValues();             // DynamicArray<V>
```

### Iteration

```cpp
for (auto& pair : map)
{
    pair.key;       // Do not assign to this, see below
    pair.value;     // Writable
}

// Or via iterator
auto it = map.Find(key);
it.GetKey();        // const reference
it.GetValue();      // Writable reference
```

A key decides where its pair is found, so a key that is changed in place can no longer be looked up.
`GetKey` hands out a `const` reference to enforce that. The pair from `operator*` is handed out whole
so that values stay writable, which leaves `pair.key` assignable; do not assign to it.

---

## HashSet

Header: `opal/container/hash-set.h`

Open-addressing hash set using the Swiss tables algorithm. Same SIMD-optimized internals as `HashMap`.

```cpp
Opal::HashSet<int> set;                   // Default capacity
Opal::HashSet<int> set(256);              // Pre-sized
Opal::HashSet<int> set(256, &alloc);      // Explicit allocator
```

### Lookup

```cpp
set.Find(key);                     // Returns iterator (end() if not found)
set.Contains(key);                 // Returns bool
```

### Modification

```cpp
set.Insert(value);                 // Returns ErrorCode
set.Erase(value);                  // Remove by value, returns ErrorCode
set.Erase(it);                     // Remove by iterator, returns ErrorCode
set.Erase(first, last);           // Remove range, returns ErrorCode
set.Clear();
set.Reserve(capacity);             // Returns ErrorCode
```

### Size and Capacity

```cpp
set.GetSize();
set.GetCapacity();
set.GetGrowthLeft();               // Insertions left before the set has to grow
set.IsEmpty();
```

### Copying

```cpp
auto clone = set.Clone();          // Deep copy, same allocator
auto clone = set.Clone(&alloc);    // Deep copy onto another allocator
```

### Conversion

```cpp
set.ToArray();                     // DynamicArray<KeyType>
```

### Iteration

```cpp
for (const auto& key : set)        // Keys are always handed out const
{
    // ...
}
```

A key decides where it is found, so a key that is changed in place could no longer be looked up. Both
`HashSet` iterators dereference to a `const` key for that reason.

### Custom Hashing

To use custom types as keys, specialize `Opal::Hasher<T>`:

```cpp
struct MyKey
{
    int id;
    bool operator==(const MyKey& other) const { return id == other.id; }
};

template <>
struct Opal::Hasher<MyKey>
{
    Opal::u64 operator()(const MyKey& key) { return static_cast<Opal::u64>(key.id); }
};

Opal::HashSet<MyKey> set;
Opal::HashMap<MyKey, int> map;
```

A hasher comes for free only for types whose bytes determine their value, plus floating point types and
ranges. Anything else, which includes any type with padding, fails to build with a message asking for a
specialization rather than being given one that would let two equal keys hash differently.

---

## PriorityQueue

Header: `opal/container/priority-queue.h`

Binary heap with configurable comparison function. Two variants: basic and index-based.

### PriorityQueue

```cpp
Opal::PriorityQueue<int> pq;                              // Min-heap (std::less)
Opal::PriorityQueue<int, std::greater<int>> max_pq;       // Max-heap
```

```cpp
pq.Enqueue(42);                    // O(log N) insert
pq.GetTop();                       // O(1) peek
pq.PopTop();                       // O(log N) remove and return top
pq.IsEmpty();
pq.GetSize();
```

### IndexPriorityQueue

Associates an integer index with each value, allowing priority updates by index.

```cpp
Opal::IndexPriorityQueue<float> ipq;

ipq.Enqueue(0, 3.14f);            // Index 0, priority 3.14
ipq.Enqueue(1, 1.0f);             // Index 1, priority 1.0
ipq.ChangeValue(0, 0.5f);         // Update priority for index 0
ipq.Contains(0);                   // true
ipq.GetTopIndex();                 // Index of the top element
ipq.GetValueOf(0);                 // Get priority by index
ipq.DeleteValue(0);                // Remove by index
ipq.PopTop();                      // Remove and return top value
```

---

## ScopePtr

Header: `opal/container/scope-ptr.h`

Unique ownership smart pointer. The managed object is destroyed when the `ScopePtr` goes out of scope. Similar to `std::unique_ptr`.

```cpp
// Preferred: use MakeScoped factory
Opal::ScopePtr<Widget> ptr = Opal::MakeScoped<Widget>(nullptr, arg1, arg2);

// Polymorphic: construct Derived, store as Base
Opal::ScopePtr<Base> ptr = Opal::MakeScoped<Base, Derived>(nullptr, arg1);

// From raw pointer (rarely needed)
Widget* raw = Opal::New<Widget>(allocator, arg1);
Opal::ScopePtr<Widget> ptr(allocator, raw);
```

### Usage

```cpp
ptr->Method();                     // Arrow operator
*ptr;                              // Dereference
ptr.Get();                         // Raw pointer
ptr.GetAllocator();                // Allocator used for destruction
ptr.IsValid();                     // True if non-null
ptr.ToRef();                       // Non-owning Ref<T>
ptr.Reset();                       // Destroy managed object
```

### Ownership Transfer

```cpp
Opal::ScopePtr<Widget> other = std::move(ptr);  // ptr is now invalid
```

---

## SharedPtr

Header: `opal/container/shared-ptr.h`

Reference-counted smart pointer. The managed object is destroyed when the last `SharedPtr` sharing ownership is destroyed or reset. Copy is deleted; use `.Clone()` to share ownership.

### Threading Policies

| Policy | Reference Count | Allocator Requirement |
|--------|----------------|----------------------|
| `ThreadingPolicy::ThreadSafe` (default) | `std::atomic<size_t>` | Must be thread-safe |
| `ThreadingPolicy::SingleThread` | `size_t` | No requirement |

```cpp
// Thread-safe (default)
Opal::SharedPtr<Widget> ptr(nullptr, arg1, arg2);

// Single-threaded (avoids atomic overhead)
Opal::SharedPtr<Widget, Opal::ThreadingPolicy::SingleThread> ptr(nullptr, arg1);

// Preferred: use MakeShared factory
auto ptr = Opal::MakeShared<Widget>(nullptr, arg1, arg2);

// Polymorphic
auto ptr = Opal::MakeShared<Base, Derived>(nullptr, arg1);
```

### Sharing Ownership

```cpp
Opal::SharedPtr<int> a(nullptr, 42);
Opal::SharedPtr<int> b = a.Clone();    // b shares ownership, refcount incremented
// a and b point to the same object
a.Reset();                              // Object still alive (b holds it)
b.Reset();                              // Object destroyed (last owner)
```

### Usage

```cpp
ptr->Method();                     // Arrow operator (non-const and const)
ptr.Get();                         // Raw pointer (non-const and const)
ptr.GetRef();                      // Non-owning Ref<T>
ptr.ToRef();                       // Non-owning Ref<T>
ptr.IsValid();                     // True if non-null
ptr.Reset();                       // Release ownership
```

---

## Ref

Header: `opal/container/ref.h`

Non-owning reference wrapper around a raw pointer. Does no memory management. Move-only. Implicitly convertible to `T&`.

```cpp
int value = 42;
Opal::Ref<int> ref(value);          // From reference
Opal::Ref<int> ref(&value);         // From pointer

ref.Get();                           // Returns T&
ref.GetPtr();                        // Returns T*
ref.IsValid();                       // True if non-null
*ref;                                // Dereference
ref->Member();                       // Arrow access
```

`Ref` is commonly used to pass references to thread functions since arguments are forwarded by value:

```cpp
int result = 0;
Opal::ThreadHandle t = Opal::CreateThread(
    [](int& out) { out = 42; },
    Opal::Ref(result));
Opal::JoinThread(t);
```

---

## Expected

Header: `opal/container/expected.h`

Result type that holds either a value of type `T` or an error of type `E`. Move-only. Similar to `std::expected`.

```cpp
Opal::Expected<int, Opal::ErrorCode> result(42);                        // Value
Opal::Expected<int, Opal::ErrorCode> result(Opal::ErrorCode::OutOfBounds); // Error
```

### Usage

```cpp
if (result.HasValue())
{
    int val = result.GetValue();
}
else
{
    Opal::ErrorCode err = result.GetError();
}

// Or with a default
int val = result.GetValueOr(0);
```

A reference specialization `Expected<T&, E>` stores a pointer internally and provides access as a reference:

```cpp
Opal::Expected<int&, Opal::ErrorCode> ref_result = arr.At(0);
if (ref_result.HasValue())
{
    ref_result.GetValue() = 99;    // Modifies the original element
}
```

---

## Iterator Utilities

Header: `opal/container/iterator.h`

Free-function overloads of `begin()`/`end()` for both Opal containers and C-style arrays, enabling generic algorithm use:

```cpp
int raw[3] = {1, 2, 3};
for (auto it = Opal::begin(raw); it != Opal::end(raw); ++it)
{
    // ...
}
```

---

## Error Handling Styles

Containers use two error handling approaches depending on the severity:

| Style | Used By | Example |
|-------|---------|---------|
| **Exceptions** | `SharedPtr`, and constructors everywhere | `OutOfBoundsException`, `OutOfMemoryException`, `InvalidArgumentException` |
| **ErrorCode returns** | `Deque`, `DynamicArray`, `String`, `HashSet`, `HashMap`, `ArrayView` | `ErrorCode::OutOfBounds`, `ErrorCode::OutOfMemory`, `ErrorCode::InvalidArgument` |

`Expected<T, ErrorCode>` is used for operations that need to return both a value and an error status, such as `Deque::At()` or `ArrayView::SubSpan()`.

The two styles are not alternatives picked per container, they answer different questions. A failure the
caller can act on, such as an allocator running out, is returned. A failure that means the caller broke a
contract, such as `HashMap::GetValue` on a key that is not there, is thrown. Constructors throw whatever
happens, since they have nothing to return. See the error handling section of `CLAUDE.md`.

---

## Pattern Analysis

### What works well

**1. Allocator everywhere.** The uniform `AllocatorBase*` parameter across all owning containers makes it straightforward to route allocations to per-system arenas. Combined with the allocator stack from [memory.md](memory.md), this eliminates the need to plumb allocators through deep call chains.

**2. Move-only with explicit Clone.** Deleting copy constructors and requiring `.Clone()` for deep copies prevents accidental expensive copies. The call site always communicates intent.

**3. Swiss tables for hash containers.** SSE SIMD-optimized probing in `HashMap` and `HashSet` gives excellent cache behavior and lookup performance. The shared control-byte layout between both containers keeps the implementation consistent.

**4. Dual-policy SharedPtr.** The `ThreadingPolicy` template parameter allows choosing between atomic and plain reference counting at compile time, avoiding atomic overhead in single-threaded contexts.

**5. Expected as a result type.** `Expected<T, ErrorCode>` provides a clean way to return errors without exceptions, and the reference specialization `Expected<T&, E>` avoids unnecessary copies for container element access.

**6. EraseWithSwap / RemoveWithSwap.** `DynamicArray` provides O(1) erase for cases where element order doesn't matter, which is common in game engines (entity lists, particle systems).

**7. Ref as a threading bridge.** `Ref<T>` solves the problem of passing references to thread functions cleanly, since `CreateThread` forwards arguments by value.

### Areas for improvement

**1. Inconsistent error handling across containers.**

`DynamicArray` and `String` throw exceptions on out-of-bounds access, while `Deque` and `ArrayView` return `Expected<T&, ErrorCode>`. That part is deliberate: an out-of-range index is a caller bug, not a recoverable condition.

Allocation failure is where they used to disagree, and no longer do. `Deque`, `DynamicArray`, `String`, `HashSet` and `HashMap` all return `ErrorCode::OutOfMemory`. What is left throwing is what has nowhere to put a code: constructors, `Clone`, `operator=` from an initializer list, and `String::operator+=`.

**2. Inconsistent naming conventions.**

`Deque` uses `PascalCase` for type aliases (`ValueType`, `SizeType`, `IteratorType`), while all other containers use `snake_case` (`value_type`, `size_type`, `iterator`). `Deque` also uses `Begin()`/`End()` alongside `begin()`/`end()`, while other containers only provide the lowercase versions.

**3. No initializer list constructor for HashSet.**

`HashMap` supports construction from an `initializer_list<pair_type>` and from `ArrayView`, but `HashSet` only supports default construction with a capacity. An `initializer_list<KeyType>` constructor and/or an `ArrayView<KeyType>` constructor would make `HashSet` more convenient to initialize.

**4. No shrink-to-fit on hash containers.**

`HashMap` and `HashSet` grow automatically but never shrink. After bulk `Erase` operations, the underlying table remains at its high-water-mark capacity. A `ShrinkToFit()` or `Compact()` method would allow reclaiming memory.

**5. Missing ToArray on Deque and InPlaceArray.**

`HashMap` provides `ToArray`/`ToArrayOfKeys`/`ToArrayOfValues`, and `HashSet` provides `ToArray`, but `Deque` and `InPlaceArray` have no conversion to `DynamicArray`. This would be useful for interop between container types.

**6. No emplace on hash containers.**

`DynamicArray` has `EmplaceBack` for in-place construction, but `HashMap` and `HashSet` have no equivalent. An `Emplace` method would avoid constructing temporaries when inserting non-POD types.

**7. PriorityQueue lacks allocator support.**

`PriorityQueue` and `IndexPriorityQueue` use raw `new`/`delete` internally and don't accept an `AllocatorBase*`, making them the only containers outside the allocator system. They also lack `Clone()` and move semantics.

**8. No reverse iterators.**

None of the containers provide reverse iterators. While `Reverse()` exists on `String` as a mutation, there's no way to iterate any container backwards without manual index management.

**9. HashMap and HashSet lack a Merge operation.**

There is no way to merge two hash containers without iterating and inserting element-by-element. A `Merge(HashSet&&)` or `Merge(HashMap&&)` that moves elements from the source would be useful for combining intermediate results.

**10. No find-or-insert on HashMap.**

`HashMap::Insert` always takes both key and value. A `FindOrInsert(key, default_value)` or `operator[]` that returns a reference (inserting a default on first access) would simplify the common "get or create" pattern.

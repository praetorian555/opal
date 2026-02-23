# Memory Management

Header: `opal/allocator.h`

Opal uses a pluggable allocator architecture where every container accepts an optional `AllocatorBase*`. Two thread-local allocator stacks (default and scratch) provide implicit allocator propagation via RAII guards.

## AllocatorBase

Abstract base class for all allocators. Every allocator takes a `debug_name` string in its constructor.

```cpp
struct AllocatorBase
{
    AllocatorBase(const char* debug_name);
    virtual void* Alloc(u64 size, u64 alignment) = 0;
    virtual void Free(void* ptr) = 0;
    virtual bool IsThreadSafe() const = 0;
    virtual const char* GetName() const;  // Returns m_debug_name
};
```

## Allocator Hierarchy

```
AllocatorBase (pure virtual)
 +-- MallocAllocator       Stateless, thread-safe, wraps aligned malloc/free
 +-- SystemMemoryAllocator  Reserve/commit via VirtualAlloc/mmap, not thread-safe
 +-- LinearAllocator        Bump allocator backed by SystemMemoryAllocator, not thread-safe
 +-- NullAllocator          Always throws, not thread-safe
```

### MallocAllocator

General-purpose heap allocator. Thread-safe. All instances are equivalent (`operator==` returns `true`). Uses `_aligned_malloc`/`_aligned_free` on Windows, `aligned_alloc`/`free` on Linux. Typically used as the root of the default allocator stack. The debug name is always `"MallocAllocator"`.

### SystemMemoryAllocator

Reserves a virtual address range upfront (`VirtualAlloc` with `MEM_RESERVE` on Windows, `mmap` with `PROT_NONE` on Linux), then commits pages on demand. Not thread-safe. Used internally by `LinearAllocator` as its backing store. Allocations are page-aligned. `Free` is a no-op.

Configuration via `SystemMemoryAllocatorDesc`:

| Field                      | Default        | Description                                       |
|----------------------------|----------------|---------------------------------------------------|
| `bytes_to_reserve`         | `OPAL_MB(1)`   | Virtual address space to reserve (must be > 0)    |
| `bytes_to_initially_alloc` | `OPAL_KB(100)` | Pages committed at construction                   |
| `commit_step_size`         | `OPAL_KB(100)` | Minimum commit granularity for on-demand commits  |

Additional API:

| Method              | Description                              |
|---------------------|------------------------------------------|
| `Commit(u64 size)`  | Manually commit additional pages         |
| `Reset()`           | Reset the offset to 0 (no decommit)     |
| `GetCommitedSize()` | Total bytes currently committed          |
| `GetPageSize()`     | OS page size                             |
| `GetMemory()`       | Pointer to the reserved memory region    |

### LinearAllocator

Bump/arena allocator. Not thread-safe. Non-copyable, non-movable. Backed by an internal `SystemMemoryAllocator`. Allocations advance a pointer with proper alignment. `Free` is a no-op. Memory is reclaimed in bulk via `Reset()` or `Reset(position)` using `Mark()`/`Reset()` pairs. `operator==` always returns `false` (each instance is unique).

When an allocation exceeds the currently committed region, additional pages are committed automatically via the backing `SystemMemoryAllocator`.

```cpp
LinearAllocator arena("Temp", {.bytes_to_reserve = OPAL_MB(64),
                                .bytes_to_initially_alloc = OPAL_MB(1),
                                .commit_step_size = OPAL_MB(1)});
void* a = arena.Alloc(256, 8);
void* b = arena.Alloc(512, 16);

u64 mark = arena.Mark();
void* c = arena.Alloc(128, 8);
arena.Reset(mark);  // c is now invalid, a and b are still valid

arena.Reset();  // All allocations invalidated
```

### NullAllocator

Throws `OutOfMemoryException` on every `Alloc`. `Free` is a no-op. Not thread-safe. Useful for containers that should never allocate (e.g. moved-from state). The debug name is always `"NullAllocator"`.

## Allocator Stacks

Two thread-local stacks manage the "current" allocator context. Each thread has its own independent stack, so Push/Pop/Get are thread-safe without synchronization:

| Stack     | Getter                    | Type               | For                              |
|-----------|---------------------------|--------------------|----------------------------------|
| Default   | `GetDefaultAllocator()`   | `AllocatorBase*`   | Long-lived data                  |
| Scratch   | `GetScratchAllocator()`   | `LinearAllocator*` | Short-lived / per-frame data     |

Note that the scratch stack is typed to `LinearAllocator*`, not `AllocatorBase*`, since scratch allocators need `Mark()`/`Reset()` support.

### Free Functions

| Function                                      | Description                                                      |
|-----------------------------------------------|------------------------------------------------------------------|
| `PushDefaultAllocator(AllocatorBase*)`         | Push allocator onto default stack. `nullptr` re-pushes root.     |
| `PopDefaultAllocator()`                        | Pop top of default stack. Will not pop the root.                 |
| `GetDefaultAllocator()`                        | Return top of default stack. Throws if empty.                    |
| `PushScratchAllocator(LinearAllocator*)`       | Push allocator onto scratch stack. `nullptr` re-pushes root.     |
| `PopScratchAllocator()`                        | Pop top of scratch stack. Will not pop the root.                 |
| `GetScratchAllocator()`                        | Return top of scratch stack. Throws if empty.                    |
| `ResetScratchAllocator()`                      | Call `Reset()` on the current scratch allocator. Throws if empty.|

### Explicit Initialization

The allocator stacks require explicit initialization. The first `PushDefaultAllocator` / `PushScratchAllocator` call with a non-null allocator initializes the respective stack. Calling `GetDefaultAllocator()` or `GetScratchAllocator()` before initialization throws `AllocatorNotInitializedException`.

```cpp
// Initialize the default allocator stack
Opal::MallocAllocator malloc_allocator;
Opal::PushDefaultAllocator(&malloc_allocator);

// Initialize the scratch allocator stack
Opal::LinearAllocator scratch("Scratch", {.bytes_to_reserve = OPAL_GB(4),
                                           .bytes_to_initially_alloc = OPAL_MB(1),
                                           .commit_step_size = OPAL_MB(2)});
Opal::PushScratchAllocator(&scratch);
```

The bottom (root) element can never be popped. Passing `nullptr` to `PushDefaultAllocator` / `PushScratchAllocator` pushes the root allocator (throws if the stack is empty). The maximum stack depth is 32.

### RAII Guards

**`PushDefault`** - Pushes an allocator onto the default stack on construction, pops on destruction.

**`PushScratch`** - Pushes a `LinearAllocator*` onto the scratch stack on construction, pops on destruction.

**`ScratchAsDefault`** - Pushes the current scratch allocator onto the default stack and saves a `Mark()`. On destruction, resets the scratch allocator to the saved mark (if `should_reset_on_destroy` is `true`, which is the default) and pops the default stack.

```cpp
// Temporarily switch the default allocator
{
    Opal::PushDefault pd(&my_allocator);
    // All containers created here without an explicit allocator use my_allocator
    Opal::DynamicArray<int> arr;  // uses my_allocator
}
// Original default restored

// Use scratch as default (with automatic mark/reset)
{
    Opal::ScratchAsDefault sd;
    // All containers created here use the scratch allocator
    // On scope exit, scratch is reset to the mark saved at construction
}

// Use scratch as default without automatic reset
{
    Opal::ScratchAsDefault sd(false);
    // Scratch allocations persist after scope exit
}

// Override the scratch allocator
{
    Opal::PushScratch ps(&my_linear_allocator);
    // GetScratchAllocator() now returns my_linear_allocator
}
```

## Container Integration

Every container (`DynamicArray`, `String`, `HashMap`, etc.) stores an `AllocatorBase*` member:

```cpp
// Explicit allocator
DynamicArray<int> arr(&my_allocator);

// Implicit - uses GetDefaultAllocator()
DynamicArray<int> arr;

// Change allocator at runtime (copies data to new allocator's memory)
arr.SetAllocator(&other_allocator);
```

**Copy constructors** use the caller's allocator (or default), not the source's:
```cpp
DynamicArray<int> copy(original);             // copy uses GetDefaultAllocator()
DynamicArray<int> copy(original, &my_alloc);  // copy uses my_alloc
```

**Move constructors** transfer the source's allocator (zero-copy ownership transfer).

## Object-Level Allocation

`New<T>` and `Delete<T>` wrap placement new / explicit destructor + free:

```cpp
auto* obj = Opal::New<MyClass>(allocator, arg1, arg2);
Opal::Delete(allocator, obj);

// With explicit alignment
auto* aligned = Opal::New<MyClass, 64>(allocator, arg1);
```

`New` throws `Exception` if the allocator is `nullptr`. `Delete` falls back to `GetDefaultAllocator()` if the allocator is `nullptr`.

---

## Pattern Analysis

### What works well

**1. Implicit propagation via stacks.** The `PushDefault` RAII pattern avoids threading an allocator pointer through every call. A subsystem can set up its allocator once at the top of a scope and all containers created inside inherit it automatically. This is particularly good for game engine / real-time patterns where you want per-frame or per-system arenas.

**2. Non-null guarantee.** Every container's `m_allocator` is guaranteed non-null after construction. The fallback to `GetDefaultAllocator()` eliminates null-checks in the hot path (Alloc/Free).

**3. Clean separation of allocation strategies.** `MallocAllocator` for general use, `LinearAllocator` for scratch, `SystemMemoryAllocator` as a building block. Each has clear ownership semantics.

**4. Mark/Reset on LinearAllocator.** Enables nested scratch scopes:
```cpp
u64 mark = arena.Mark();
// do work with temporary allocations...
arena.Reset(mark);
```

**5. Explicit initialization.** The user creates and owns the allocators, and the first `PushDefaultAllocator` / `PushScratchAllocator` call initializes the system. This avoids hidden global state and makes allocator lifetimes clear.

**6. ScratchAsDefault auto-reset.** The mark/reset built into `ScratchAsDefault` makes scratch-as-temporary-default a one-liner with correct cleanup.

### Areas for improvement

**1. No ownership tracking or leak detection.**

There's no way to know how many live allocations exist from a given allocator, or to detect leaks when an allocator is destroyed. A simple allocation counter in debug builds would catch common mistakes.

**2. Copy/move assignment allocator semantics differ across containers.**

The move assignment in `DynamicArray` unconditionally takes the source allocator. But the copy assignment only copies data if the allocators differ, and takes the source allocator. This is a valid design but it's not documented, and getting it wrong has caused bugs (per commit history: "Fix move assignment to assign allocator as well", "Fix move constructor where receiving string has nullptr allocator").

### Potential additions to consider

| Feature | Purpose |
|---------|---------|
| Debug allocation counters | Leak detection per allocator |
| Pool allocator | Fixed-size block allocation for hot paths |
| Allocator stats/profiling | Track peak usage, allocation count, fragmentation |
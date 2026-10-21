# Design Guide

How Opal reports failure, constructs objects, copies them and uses memory, and how to follow the same rules in a library or
engine built on top of it. Types that follow them compose with Opal's containers without surprises: they can be stored by
value, cloned with a budgeted allocator, and built in code that has to keep running when memory runs out.

## Contents

- [Failure](#failure)
- [Construction](#construction)
- [Copying](#copying)
- [Allocators](#allocators)
- [Exceptions Off](#exceptions-off)
- [Checklist for a New Type](#checklist-for-a-new-type)

## Failure

There are two kinds of failure, and they are reported differently.

**A failure the caller can act on is returned.** Running out of memory is the main one. Allocators are pluggable and often
budgeted - a `LinearAllocator` per frame, a `NullAllocator` around code that must not allocate - so exhausting one is an
ordinary outcome to branch on, not an exceptional one.

- An operation with nothing to hand back returns `ErrorCode`: `Reserve`, `Insert`, `PushBack`, `Erase`.
- An operation that hands back a value returns `Expected<T, ErrorCode>`: `TryAt`, `Front`, `Create`, `TryClone`.
- `AllocatorBase::Alloc` returns `nullptr`. It never throws.

```cpp
if (Opal::ErrorCode error = names.PushBack(Opal::Move(name)); error != Opal::ErrorCode::Success)
{
    return error;
}
```

**A failure that means the caller broke a contract is not reported back.** Looking up a key with `GetValue` when the map does not hold
it, indexing past the end of an array. These go to the contract violation handler, which by default prints the
message and aborts. Some older checks raise an exception (`InvalidArgumentException`, `OutOfBoundsException`) instead. Either
way, the fix is in the calling code, not in a branch that handles it.

```cpp
// Install once at startup, before other threads run. Write a crash report, flush logs, then let it abort.
Opal::SetContractViolationHandler([](const char* message) { WriteCrashReport(message); });
```

Owning pointers have an empty state of their own, so they use it. `MakeScoped`, `MakeShared`, `ScopePtr` and `SharedPtr`
come back invalid when an allocation fails, and the caller checks `IsValid()`.

## Construction

Making an empty object never fails. Only filling it can.

### Default constructors never allocate

The default constructors of `DynamicArray`, `Deque`, `String`, `HashMap` and `HashSet` are `noexcept` and do not touch the
allocator beyond remembering which one to use. Room is
made by the first insertion or by `Reserve`. A struct whose members are Opal containers therefore default constructs for free,
and `= {}` member initializers are always safe:

```cpp
struct Scene
{
    Opal::DynamicArray<Entity> entities;
    Opal::HashMap<Opal::StringUtf8, Opal::u32> entity_by_name;
    Opal::Deque<Event> pending_events;
};

static_assert(std::is_nothrow_default_constructible_v<Scene>);
Scene scene = {};  // Cannot fail
```

A moved-from object is left in this same empty state, so it can be destroyed, assigned to or reused.

### Allocating constructors treat failure as fatal

A constructor that allocates - given a capacity, an initializer list, a copy of some data - has no way to return a code. It
raises `OutOfMemoryException`, which with exceptions off ends the program through the contract violation handler. Calling one
asserts that the allocation cannot fail. That is the right call for setup code using the general-purpose allocator, where
running out means the process is finished anyway.

```cpp
Opal::HashMap<Opal::i32, Opal::i32> lookup(1024);       // Raises if 1024 slots cannot be allocated
Opal::DynamicArray<Opal::i32> primes = {2, 3, 5, 7};    // Same
```

### Every allocating constructor has a fallible twin

Where running out is an outcome to handle, use the static `Create` with the same arguments. It returns
`Expected<T, ErrorCode>`:

```cpp
auto lookup = Opal::HashMap<Opal::i32, Opal::i32>::Create(1024, &frame_allocator);
if (!lookup.HasValue())
{
    return lookup.GetError();  // ErrorCode::OutOfMemory
}
```

Or construct empty, which cannot fail, and fill through calls that return a code:

```cpp
Opal::DynamicArray<Opal::i32> values(&frame_allocator);
if (Opal::ErrorCode error = values.Reserve(1024); error != Opal::ErrorCode::Success)
{
    return error;
}
```

### Types that need a resource are built through a factory

A type whose invariants need a resource to exist at all - a thread pool needs its threads - has no usable empty state. It is
built only through `Create`, which returns the error instead of a half-made object. `ThreadPool` does this with a passkey: the
constructor is public so the allocation helpers can call it, but it takes a key type only `Create` can make.

```cpp
class Device
{
    struct CreateKey
    {
        explicit CreateKey() = default;
    };

public:
    [[nodiscard]] static Opal::Expected<Opal::ScopePtr<Device>, Opal::ErrorCode> Create(const DeviceDesc& desc);

    /** Used by Create; call Create instead. */
    Device(CreateKey key, const DeviceDesc& desc);
};
```

Avoid a separate `Init` method instead. It allows an object that exists but is not ready, and every other method then has to
check for that state.

### Keep allocations visible at call sites

A function that only reads a string should take `StringViewUtf8`, not `const StringUtf8&`. A literal then converts to a view
without allocating, and a string is only built where one is actually stored. Opal's file, path, time, socket and JSON functions
take their strings this way.

A view is not null-terminated. Code that hands one to a C API needing a terminator has to copy it first; Opal's file functions copy
a path into a stack buffer on Linux, so they still do not allocate.

This holds for map lookups too. A `HashMap` or `HashSet` keyed by `StringUtf8` can be searched with a view or a literal, so a
function that only looks a key up can take `StringViewUtf8` as well. To give your own key type the same lookups, specialize
`HashLookupKey` with the cheaper type it can be searched by.

## Copying

### Clone instead of copy

Resource-owning types delete their copy constructor and copy assignment. A copy is spelled out, so an expensive one is never
made by accident:

```cpp
Opal::DynamicArray<Opal::i32> moved = Opal::Move(original);   // Transfers ownership, no allocation
Opal::DynamicArray<Opal::i32> copy = moved.Clone();            // Deep copy, same allocator
Opal::DynamicArray<Opal::i32> copy2 = moved.Clone(&allocator); // Deep copy, given allocator
```

The allocator passed to `Clone` is used for the copy and handed down to every element it clones, so a cloned
`DynamicArray<StringUtf8>` keeps all of its strings in the same allocator.

### Clone or TryClone

`Clone` follows the allocating constructors: a failed allocation is fatal. `TryClone` reports it instead, and stops at the first
element that fails to copy, destroying what it had already copied:

```cpp
auto snapshot = world.entities.TryClone(&snapshot_allocator);
if (!snapshot.HasValue())
{
    return snapshot.GetError();
}
```

`Deque`, `DynamicArray`, `String`, `HashMap`, `HashSet`, `InPlaceArray`, `Optional`, `Variant`, `Pair` and every `ClonableBase`
type have both. `SharedPtr` and `JsonValue` have only `Clone`, which shares what they point to instead of copying it, so it cannot
fail.

### Opal::Clone and Opal::TryClone

The free functions in `opal/common.h` copy any value the way the containers do:

| Type | `Opal::Clone(value, allocator)` | `Opal::TryClone(value, allocator)` |
|------|---------------------------------|------------------------------------|
| POD | Copies it | Copies it |
| Has `TryClone` | Calls `Clone` | Calls `TryClone` |
| Has only `Clone` | Calls `Clone` | Calls `Clone`, which can still raise |

Generic code that copies values of a type it does not know should call these rather than the members.

### Making your own type clonable

Inherit `ClonableBase` and list the fields. It deletes the copy operations and provides `Clone` and `TryClone`, which copy each
field with `Opal::Clone` and `Opal::TryClone`:

```cpp
#include "opal/clonable-base.h"

struct Player : Opal::ClonableBase<Player>
{
    Opal::StringUtf8 name;
    Opal::i32 score = 0;
    Opal::DynamicArray<Opal::u32> inventory;

    OPAL_CLONE_FIELDS(name, score, inventory);
};
```

The type has to be default constructible, which costs nothing when its members follow the construction rules above.

A type that writes its own `Clone` should write `TryClone` next to it and have `Clone` call it:

```cpp
Mesh Mesh::Clone(Opal::AllocatorBase* allocator) const
{
    Opal::AllocatorBase* target = allocator != nullptr ? allocator : m_allocator;
    auto clone = TryClone(target);
    if (!clone.HasValue())
    {
        OPAL_RAISE(Opal::OutOfMemoryException(target->GetName(), GetByteSize()));
    }
    return Opal::Move(clone).GetValue();
}
```

## Allocators

Every container takes an optional `AllocatorBase*`. `nullptr` means the current default allocator, the top of a thread-local
stack. A type in your library should take one the same way and pass it on to the containers it owns.

```cpp
class Inventory
{
public:
    explicit Inventory(Opal::AllocatorBase* allocator = nullptr) noexcept : m_items(allocator), m_counts(allocator) {}

private:
    Opal::DynamicArray<Item> m_items;
    Opal::HashMap<ItemId, Opal::u32> m_counts;
};
```

### Scoping an allocator

`PushDefault` makes an allocator the default for everything created in a scope, without threading a pointer through every call.
`ScratchAsDefault` does the same with the scratch allocator and rewinds it when the scope ends. See [memory.md](memory.md).

```cpp
{
    Opal::ScratchAsDefault scratch;
    Opal::DynamicArray<Opal::u32> visible = CollectVisible(scene);  // Allocates from scratch
    Submit(visible);
}  // Scratch memory reclaimed
```

### Catching allocations that should not happen

To prove a hot path does not allocate, make a `NullAllocator` the default around it. Anything that allocates through the
default allocator then fails: a call that returns a code returns `ErrorCode::OutOfMemory`, and an allocating constructor or
`Clone` raises with the allocator's name in the message.

```cpp
void UpdateParticles(ParticleSystem& system)
{
#if defined(MY_DEBUG)
    Opal::NullAllocator no_allocations;
    Opal::PushDefault guard(&no_allocations);
#endif
    // ...
}
```

This finds allocations the type system cannot show, such as an array growing on `PushBack`. It only covers the default
allocator: a container created earlier keeps the allocator it was given.

## Exceptions Off

Building Opal with `OPAL_EXCEPTIONS=OFF` adds `-fno-exceptions` (`/EHs-c-` on MSVC) and defines `OPAL_NO_EXCEPTIONS`, both
`PUBLIC`, so your library is compiled the same way as the Opal it links. Nothing in the public API is removed:

- Everything that returns `ErrorCode` or `Expected` behaves the same.
- What would have raised an exception - an allocating constructor or `Clone` that ran out, a broken contract - goes to the
  contract violation handler and the program ends.

Code that follows this guide does not need to change between the two builds: anything it means to recover from goes through
`Create`, `TryClone` and calls that return a code, and those never raise.

## Checklist for a New Type

- [ ] Default constructor is `noexcept`, takes an optional `AllocatorBase*` and does not allocate.
- [ ] Move constructor and move assignment are `noexcept` and leave the source in the default-constructed state.
- [ ] Copy constructor and copy assignment are deleted.
- [ ] `Clone(AllocatorBase* = nullptr)` and `TryClone(AllocatorBase* = nullptr)` exist, with `Clone` calling `TryClone`.
  `ClonableBase` with `OPAL_CLONE_FIELDS` gives both.
- [ ] Every constructor that allocates has a static `Create` with the same arguments returning `Expected<T, ErrorCode>`.
- [ ] Operations that can run out of memory return `ErrorCode` or `Expected`. They do not throw.
- [ ] A broken precondition goes through `OPAL_VERIFY` (every build) or `OPAL_ASSERT` (debug only), not an error code.
- [ ] A type that cannot exist without a resource is built only through `Create`, with no `Init` method.
- [ ] Read-only string parameters are `StringViewUtf8`.

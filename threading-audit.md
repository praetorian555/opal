# Threading APIs - audit

A pass over `include/opal/threading/` and its four `src/` files for correctness and completeness.
`Mutex`, `ConditionVariable`, `Signal`, `QueueSPSC`, `QueueMPMC`, `ThreadPool` and the free thread
functions.

Two of the findings below are crashes reproduced under ASan rather than read off the page; those
carry their output. The rest are read from the code and marked as such.

The subsystem has one test file, `test/thread-test.cpp`. Nothing in it moves a `Mutex` or a
`ConditionVariable`, which is why §1 survived.

---

## Correctness

### 1. Moving a `ConditionVariable` segfaults

`~ConditionVariable` does not check the handle, so the moved-from object destroys a null one.

```
runtime error: null pointer passed as argument 1, which is declared to never be null
AddressSanitizer: SEGV on unknown address 0x000000000024
    #0 __GI___pthread_cond_destroy
    #1 Opal::ConditionVariable::~ConditionVariable() src/condition-variable.cpp:37
```

`PureMutex::~PureMutex` checks. The condition variable was written from the same shape and the check
was left out. Windows survives only because `CONDITION_VARIABLE` has no destroy call, so this is a
Linux-only crash on a code path Windows developers never see.

- [x] Null-check the destructor, and cover both move construction and move assignment with a test

### 2. `Mutex` and `ConditionVariable` do not check their allocation

Both constructors call `New<>` and use the result without looking at it. `New<>` returns `nullptr` on
a budgeted allocator, which §1 of the exceptions audit made an ordinary outcome.

```
runtime error: null pointer passed as argument 1
AddressSanitizer: SEGV ... WRITE memory access
    #1 ___pthread_mutex_init
    #2 Opal::Impl::PureMutex::PureMutex() src/mutex.cpp:19
```

This is the bug class the whole exceptions audit was about, still sitting in the two threading
primitives that allocate. Constructors have no return channel, so `OPAL_RAISE(OutOfMemoryException)`
matches what every container does; a `Create` factory alongside is the §11 shape if wanted.

- [x] `PureMutex` reports a failed allocation
- [x] `ConditionVariable` reports a failed allocation
- [x] `NullAllocator` tests for both

### 3. `Mutex`, `ConditionVariable` and `Signal` are movable, and moving them is unsound

The standard library makes all three non-movable, for the reasons that apply here unchanged.

- `MutexGuard` holds `Mutex<T>*` and `T*`. Moving a locked `Mutex` leaves the guard unlocking a
  moved-from mutex and dereferencing moved-from storage.
- A moved `Signal` leaves waiters parked on the old address - the futex and `WaitOnAddress` keys are
  addresses. `NotifyOne` on the new object never reaches them. Zeroing the source's state can also
  spuriously release a waiter whose `expected_state` was non-zero.
- `ConditionVariable` has the same address problem for anyone already inside `Wait`.

The two `Signal` move tests pass only because nothing is waiting during them.

Deleting the move operations is the honest fix and is a breaking API change, so it wants a decision
rather than a patch. `ThreadPool` holds its members by value and would be unaffected; `Mutex<T>`
members inside movable types are the thing to check before committing to it.

- [x] Decide: delete the move operations, or document them as valid only when provably unused.
      Decided: deleted on all four of `Mutex`, `Impl::PureMutex`, `ConditionVariable` and `Signal`.
      `ConsoleSink` is the only type in the library holding one as a member and it was never movable
- [x] `Impl::PureMutex::operator=` leaks the existing handle - it overwrites `m_native_handle`
      without destroying what was there. `ConditionVariable::operator=` does destroy first. Fix
      regardless of what the decision above is, unless moves go away entirely. Moot: the operator
      is gone

### 4. `PureMutex` is recursive on Windows and not on Linux

`CRITICAL_SECTION` lets the owning thread re-enter. A default `pthread_mutex_t` deadlocks. Code that
locks twice on one thread works on Windows and hangs on Linux, which is the worst possible split
given the project develops on Windows and CI runs both.

- [x] Pick one. `PTHREAD_MUTEX_RECURSIVE` to match Windows, or a non-recursive Windows mutex
      (`SRWLOCK`) to match Linux, and say which in the docs. Picked `SRWLOCK`, which also moved the
      condition variable to `SleepConditionVariableSRW`

### 5. SPSC producer is missing an acquire

`QueueSPSC::Push` reads `m_read_idx` with `memory_order_relaxed` to decide there is space, then
writes the slot. The consumer releases `m_read_idx` after reading a slot, but the producer's relaxed
load never pairs with it, so the producer's write to a slot is not ordered after the consumer's read
of that same slot on the previous lap.

Benign on x86's store ordering, a real race on ARM. Read from the code, not reproduced - the project
targets x86_64 today.

- [ ] Acquire on the `m_read_idx` loads in `Push` and `TryPush`

### 6. Windows `PureMutex` destructor deletes through `void*`

`Delete(m_allocator, m_native_handle)` deduces `T = void` and calls `ptr->~T()` on it. MSVC accepts
it; the Linux branch passes the typed pointer and would not compile this way.

- [x] Pass `critical_section` rather than `m_native_handle`. The `SRWLOCK` rewrite passes a typed
      `SRWLOCK*`

---

## Completeness

- [ ] `Mutex::Unlock()` is public, so a caller can unlock behind the guard's back and the guard then
      unlocks a second time on destruction. Make it private with `MutexGuard` as a friend
- [ ] `Mutex`'s variadic constructor is unconstrained. `template <typename... Args> Mutex(Args&&...)`
      is a better match than the deleted copy constructor for a non-const `Mutex&`, the usual
      perfect-forwarding trap. Constrain it away from `Mutex` itself
- [ ] `ConditionVariable` has no predicate `Wait`, and **the loop `docs/threading.md` demonstrates in its place is racy**.
      `while (!*cond.Wait(guard))` waits *before* testing the predicate, so a notification that arrives before the waiter
      reaches `Wait` is lost and the thread blocks forever. Confirmed the hard way: a test written from that snippet hung the
      suite on both Linux and Windows until the predicate was moved ahead of the wait. A predicate overload,
      `Wait(guard, pred)`, is the fix; the docs need correcting either way. The docs are corrected and the test that used to
      hang is in the suite; the overload is still missing
- [ ] `ThreadPool::AddFunctionTask` uses the throwing `SharedPtr` constructor and does not document
      it, though `SharedPtr::Create` exists now
- [ ] `ThreadPool::AddFunctionTask` accepts tasks after `Close()`. They queue behind departed
      workers and `WaitForCompletion` blocks forever
- [ ] No `ThreadPool::WaitForAll()`. Callers can wait per task but not for the pool to drain
- [ ] `QueueSPSC` accepts capacity 0. `m_capacity - 1` underflows and `Push` spins forever
- [ ] `MutexGuard` has no `operator*` or `operator->`, and `Deref()` has no const overload
- [ ] `MutexGuard` reports nothing when used after being moved from - `Deref()` returns `nullptr`
- [ ] `QueueSPSC`'s allocator thread-safety check is `OPAL_ASSERT`, so it is gone in release.
      `OPAL_VERIFY` is what the rest of the library settled on for contracts

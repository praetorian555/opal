# Threading - audit

A pass over `include/opal/threading/` and the matching sources in `src/` for correctness and completeness: what the existing
primitives get wrong, and what a real-time consumer - a UI framework loading assets and shaping text off the main thread, an
engine running a frame graph - would reach for and not find.

The findings below were read off the code, not reproduced, unless marked otherwise. Line numbers are as of 0.7.1.

What exists today: `Atomic<T>` with compile-time memory orders, `Mutex<T>` that hands out its data only through a guard,
`ConditionVariable`, `Signal` (a counter over `WaitOnAddress` and futex), bounded SPSC and MPMC channels, a fixed-size
`ThreadPool`, thread create, join, detach and single-core affinity, and CPU topology through `GetCpuInfo`.

---

## Correctness

### 1. A channel's close flag outlives nothing it is handed to

`ChannelSPSC` and `ChannelMPMC` keep the closed flag as a member of the channel struct (`channel-spsc.h:380`,
`channel-mpmc.h:349`), and the transmitter and receiver each hold a `Ref<Atomic<bool>>` to it, which is a raw pointer. The
queue they share is reference-counted; the flag next to it is not.

The documentation says the two ends can be moved to separate threads, and that is the usual way to use them. Once they are,
the channel struct is free to go out of scope - it typically lives on the frame of the function that set the threads up - and
from then on every `Close`, `Receive` and `TryReceive` reads or writes a dead stack slot. The tests never let the channel die
before the threads are joined, so none of them notice.

The flag belongs in the shared queue, where its lifetime is already handled.

**Fixed.** The flag lives in the queue and both ends reach it through their `SharedPtr`. The `ChannelX` structs no longer have
an `m_is_closed` member, and the end constructors take only the queue.

### 2. `PushWithEmplace` constructs over a live object

The SPSC queue's storage is a `DynamicArray<T>` sized to the capacity, so every slot holds a default-constructed `T` from the
start. `PushWithEmplace` and `TryPushWithEmplace` (`channel-spsc.h:145,166`) placement-new into the slot without destroying what
is there. For a `T` that owns anything - a string, a `SharedPtr` - each push leaks what the slot held. The two emplace tests use
a struct of an `i32` and an `f32`, where nothing is lost.

**Fixed.** A `T` that is nothrow-constructible from the arguments is destroyed and rebuilt in place; any other is built as a
temporary and move-assigned, so a throwing constructor cannot leave a destroyed slot behind. Reproduced first: a counting type
leaked 12 instances over three laps of a four-slot queue.

### 3. `TransmitterMPMC::Send(T&&)` copies

`Send(T&&)` calls `m_queue->Push(std::move(item))` (`channel-mpmc.h:239`), but `QueueMPMC` has only `Push(const T&)`, so the
rvalue binds to it and the item is copied, or cloned when `T` is not copy-assignable. A move-only type cannot be sent at all,
because the queue's own constraint asks for copy-assignment or `Clone`. `ThreadPool` sends every task as a `SharedPtr<Task>`
rvalue and pays an atomic increment and decrement per submission for the clone it did not ask for.

There is no `TryPush(T&&)` either, so `TrySend` always copies.

**Fixed.** `QueueMPMC` has `Push(T&&)` and `TryPush(T&&)`, and the transmitter `TrySend(T&&)`; a rejected `TrySend` leaves its
argument untouched. The queue now asks only for move-assignment, and the lvalue overloads alone ask for copy or `Clone`.

### 4. `Close` does not wake a receiver that is already blocked

Documented rather than hidden, but still a hole: `Receive` checks the flag once and then blocks in `Pop`, which waits for a
slot's turn and never looks at the flag again. A consumer thread parked in `Receive` stays parked after `Close`. Shutting one
down cleanly needs a sentinel value or a `TryReceive` polling loop, which is what `ThreadPool` does with its null tasks.

`Receive` and `TryReceive` also raced a send followed by `Close`: a pop that found the queue empty, then a send and a close from
the producer, then the receiver's check of the flag reported the channel closed with an item still in it.

**Fixed.** `Receive` no longer calls the blocking `Pop`. It loops over `TryPop` and the closed flag, waiting in between on a
push counter that every push and `Close` advance, so `Close` wakes it. A receiver that sees the flag set pops once more before
reporting closed, which `TryReceive` does too. The counter shares a cache line with the write index, which the producer writes
on every push anyway; it costs one more atomic add per push in the signaling variants, and nothing in the spinning ones.
`ThreadPool` still shuts down with sentinels; it could use `Close` now. (It does since finding 5.)

### 5. A task added while the pool closes is never run

`AddFunctionTask` checks `m_is_closed` and then sends (`thread-pool.h:104`). A `Close` that lands between the two has already
sent its sentinels and joined the workers, so the task goes into a queue nobody reads. The caller holds a handle whose
`WaitForCompletion` never returns.

Child tasks had the same problem: a task still running when `Close` sent the sentinels submitted its children behind them.

**Fixed.** Reproduced first: one accepted task in 500 close-while-submitting rounds never ran, and all 8 children of a task
running across `Close` were lost. `AddFunctionTask` now counts itself in before checking the flag and out after sending, and
`Close` sets the flag, waits for the count to reach zero, and then closes the channel instead of sending sentinels. Workers
drain the channel before exiting, so children sent after the close still run: the worker that sent them is alive to pop them.

### 6. The pool can deadlock on its own queue

The task channel is bounded and `Send` blocks when it is full. A task that submits children does so from a worker; once every
worker is blocked in `Send`, no one is left to pop. The same happens to a task that calls `WaitForCompletion` on a task queued
behind it, since a waiting worker does not run anything else.

### 7. `ThreadPool` can only fail by ending the program

The constructor raises when a worker does not start (`thread-pool.cpp:38`). Under `OPAL_NO_EXCEPTIONS` that is termination, and
there is no `Create` factory returning `Expected` the way the containers grew one.

### 8. `ConditionVariable::WaitFor` measures against the wall clock on Linux

The deadline is built from `CLOCK_REALTIME` (`condition-variable.cpp`), so an NTP step or a manual clock change lengthens or
cuts short every timed wait in flight. It wants a condition variable created with `pthread_condattr_setclock(CLOCK_MONOTONIC)`.

### 9. `ConditionVariable::WaitFor` truncates its timeout on Windows

The `u64` milliseconds are cast to `DWORD`. Anything over about 49 days wraps, and exactly `0xFFFFFFFF` becomes `INFINITE`.

### 10. `Signal::WaitFor` spins through its last millisecond

The remaining time is a `f64` converted to whole milliseconds for the OS call. Under a millisecond left, the timeout becomes
zero, the wait returns at once, and the loop goes round again until the deadline passes - a busy spin of up to a millisecond on
every timed wait that runs out.

### 11. Linux `CreateThread` spins without a pause

The launcher waits for the new thread to publish its kernel id in an empty loop (`thread.cpp:79`). It is short, but it is the
one spin in the library without `CpuPause`.

### 12. Every mutex and condition variable is a heap allocation

`PureMutex` and `ConditionVariable` allocate their native object from the default allocator (`mutex.cpp:16`) and raise when the
allocation fails. An `SRWLOCK` and a `CONDITION_VARIABLE` are one pointer each; a `pthread_mutex_t` is 40 bytes. Inline storage
of the right size and alignment would remove both the allocation and the failure path, and a `Mutex<T>` would no longer depend
on whichever default allocator happened to be pushed when it was built.

### 13. `SetThreadAffinity` is single-core and silent

It returns `void` and ignores what the OS reports, takes one logical core rather than a mask, and cannot address cores past 64
on Windows, where those live in other processor groups.

### 14. `QueueMPMC` trusts its caller

The constructor checks neither that the capacity is a power of two - `ChannelMPMC` rounds it, the queue does not - nor that the
allocator is thread-safe, both of which `QueueSPSC` verifies. Each slot also aligns the turn counter and the data to separate
cache lines, so a slot is at least two lines whatever `T` is.

---

## Completeness

Ordered by how soon a consumer is likely to need it.

### Tasks and scheduling

- [ ] A task that produces a value: a future or promise pair, or `Task<T>` with `GetResult`. Today a result has to be written
      through a capture
- [ ] A job system: per-worker deques with stealing, dependencies between jobs, a waiting worker that runs other work instead of
      blocking (which also closes finding 6), and `ParallelFor` over a range
- [ ] A queue drained by one named thread, for posting work back to a main or UI thread
- [ ] Cancellation tokens, so stale work can be dropped before it runs
- [ ] `ThreadPool::Create` returning `Expected`, per-worker names and affinity

### Threads

- [ ] `Sleep` and `Yield`. Neither exists, `time.h` included
- [ ] Thread names (`SetThreadDescription`, `pthread_setname_np`), so debuggers and profilers can tell workers apart
- [ ] Stack size and priority at creation. Priority matters on Android, where it decides big or little cores
- [ ] A cheap logical core count, without the sysfs walk `GetCpuInfo` does
- [ ] `GetCurrentThreadId()` without building a handle

### Synchronization

- [ ] A reader-writer lock. `SRWLOCK` already supports shared mode, and the `Mutex<T>` guard pattern carries over as a read
      guard handing out `const T*`
- [ ] A counting semaphore
- [ ] A latch and a barrier
- [ ] `CallOnce`
- [ ] Manual- and auto-reset events. `Signal` comes close but is a counter, not a state
- [ ] A spin lock for very short critical sections
- [ ] `TryLock` with a timeout
- [ ] Locking several mutexes at once without risking lock-order deadlock

### Atomics

- [ ] `FetchAnd`, `FetchOr`, `FetchXor`
- [ ] `FetchAdd` and `FetchSub` on pointers
- [ ] `AtomicThreadFence` and `AtomicSignalFence`
- [ ] `Wait` with a timeout

### Channels

- [ ] `Receive` with a timeout
- [ ] An unbounded channel
- [ ] Waiting on several channels at once
- [ ] Draining a batch in one call

### Platforms

- [ ] macOS and iOS. Every primitive has a Windows and a Linux branch and an `#error` otherwise. The Apple equivalents are
      `os_unfair_lock` and `__ulock_wait`, or pthreads and `std::atomic::wait`

---

## Suggested order

1. Findings 1, 2 and 3. Memory errors and a silent copy, each local to one header.
2. Findings 4 and 5: `Close` wakes blocked receivers, and the pool stops accepting work atomically with closing.
3. `Sleep`, `Yield`, thread names and a reader-writer lock. Small and independent.
4. Futures and a job system with work stealing, together with posting back to a named thread. The one design-sized item.

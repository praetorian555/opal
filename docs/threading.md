# Threading

Headers: `opal/threading/thread.h`, `opal/threading/mutex.h`, `opal/threading/read-write-mutex.h`, `opal/threading/condition-variable.h`, `opal/threading/signal.h`, `opal/threading/channel-spsc.h`, `opal/threading/channel-mpmc.h`, `opal/threading/thread-pool.h`, `opal/threading/future.h`, `opal/threading/executor.h`, `opal/threading/task-queue.h`

Cross-platform threading primitives for Windows and Linux. Includes threads, mutexes, condition variables, lock-free channels, and a task-based thread pool.

## Quick Start

```cpp
#include "opal/threading/thread.h"
#include "opal/threading/mutex.h"

// Create and join a thread
auto handle = Opal::CreateThread([](int x) { /* runs on new thread */ }, 42);
Opal::JoinThread(handle.GetValue());

// Protect shared data with a mutex
Opal::Mutex<int> counter(0);
{
    auto guard = counter.Lock();
    *guard.Deref() += 1;
}
```

## Threads

### Creating Threads

`CreateThread` accepts any callable and its arguments. Arguments are forwarded by value. Use `Ref` to pass references.

It returns `Expected<ThreadHandle, ErrorCode>`: `ErrorCode::OutOfMemory` when the thread state could not be allocated, and
`ErrorCode::OSFailure` when the OS refused to start the thread.

```cpp
auto thread = Opal::CreateThread([]() { /* work */ });
if (!thread.HasValue())
{
    // thread.GetError() is OutOfMemory or OSFailure
    return;
}
Opal::JoinThread(thread.GetValue());
```

The examples below unwrap with `GetValue()` to stay short. `GetValue()` asserts when the call failed, so check `HasValue()` first.

```cpp
#include "opal/threading/thread.h"

// Simple thread
auto t = Opal::CreateThread([]() { /* work */ });
Opal::JoinThread(t.GetValue());

// Thread with arguments
auto t = Opal::CreateThread([](int a, float b) { /* use a, b */ }, 10, 3.14f);
Opal::JoinThread(t.GetValue());

// Pass by reference using Ref
int result = 0;
auto t = Opal::CreateThread(
    [](int& out) { out = 42; },
    Opal::Ref(result));
Opal::JoinThread(t.GetValue());
// result == 42
```

The default allocator must be thread-safe when creating threads. The library asserts this at thread creation time and pushes the caller's default allocator onto the new thread's allocator stack.

### Detaching Threads

`DetachThread` allows a thread to run independently without requiring a join. The thread cleans up its own resources when it finishes. After detaching, the handle should not be used with `JoinThread`.

```cpp
auto t = Opal::CreateThread([]()
{
    // This thread runs independently
    // Resources are cleaned up automatically when it finishes
});
Opal::DetachThread(t.GetValue());
// No need to call JoinThread
```

### Thread ID

Each thread has a `ThreadId` (`u64`) that maps to the platform's native thread identifier. On Windows this is the value from `GetCurrentThreadId()`. On Linux this is the kernel thread ID from `gettid()`, which is the value shown by tools like `htop` and `strace`.

```cpp
auto handle = Opal::CreateThread([]()
{
    Opal::ThreadHandle self = Opal::GetCurrentThreadHandle();
    Opal::ThreadId id = Opal::GetThreadId(self);
    // id is a u64 that can be printed, compared, logged
});
Opal::JoinThread(handle.GetValue());

// Also accessible directly from the handle
Opal::ThreadId id = handle.GetValue().id;
```

### Sleeping and Yielding

`SleepCurrentThread` suspends the calling thread for at least the given number of milliseconds. The OS may oversleep by up to a
scheduler tick, which on Windows is about 15.6 ms unless the process has raised the timer resolution. A sleep of zero gives up
the rest of the time slice. `YieldCurrentThread` does the same without asking for any sleep.

```cpp
Opal::SleepCurrentThread(10);   // At least 10 ms
Opal::YieldCurrentThread();     // Let another ready thread run
```

### Thread Names

`SetThreadName` names a thread for debuggers, profilers and crash dumps. It returns `ErrorCode::Success`,
`ErrorCode::OutOfMemory` when the name could not be converted for the OS, or `ErrorCode::OSFailure` when the OS refused it.

```cpp
auto t = Opal::CreateThread([]() { /* work */ });
Opal::SetThreadName(t.GetValue(), "asset-loader");
Opal::SetThreadName(Opal::GetCurrentThreadHandle(), "main");
```

Code units that are not valid UTF-8 are replaced. Linux and Android keep only the first 15 bytes of a name, so a longer one is
cut to the last whole code point that fits; Windows keeps it whole.

Platform implementation: `SetThreadDescription` on Windows, `pthread_setname_np` on Linux.

### Thread Affinity

Pin a thread to a specific logical core.

```cpp
auto t = Opal::CreateThread([]() { /* work */ });
Opal::SetThreadAffinity(t.GetValue(), 0);  // Pin to logical core 0
Opal::JoinThread(t.GetValue());
```

### CPU Topology

Query the system's CPU topology to discover physical cores, logical cores, and hyperthreading.

```cpp
auto cpu_info = Opal::GetCpuInfo();
if (cpu_info.HasValue())
{
    const Opal::CpuInfo& info = cpu_info.GetValue();
    for (Opal::u64 i = 0; i < info.physical_processors.GetSize(); ++i)
    {
        const Opal::PhysicalCoreInfo& core = info.physical_processors[i];
        // core.id               - Physical core index
        // core.is_hyperthreaded  - true if SMT is enabled on this core
        // core.logical_cores     - BitMask<u64> of assigned logical core indices
        // core.l3_domain         - Index into info.l3_domains, or k_unknown_l3_domain
    }
    for (Opal::u64 i = 0; i < info.l3_domains.GetSize(); ++i)
    {
        const Opal::CacheDomainInfo& domain = info.l3_domains[i];
        // domain.logical_cores   - Logical cores sharing this L3
        // domain.size_bytes      - Its size, 0 if not reported
    }
}

// Or print everything to the logger
Opal::ErrorCode err = Opal::PrintCpuInfo();
```

`l3_domains` lists each L3 cache and the logical cores that share it. On a CPU built from several core complexes, such as a Zen CPU
with two CCDs, each complex has its own L3, and handing data between cores of different complexes costs several times what it
costs within one, so threads that talk to each other belong in one domain. A virtual machine reports whatever its hypervisor
presents, often a single domain.

`GetCpuInfo` reports `ErrorCode::OSFailure` when the topology could not be read and `ErrorCode::OutOfMemory` when the result could not
be grown. `PrintCpuInfo` forwards whatever it reported and prints nothing on failure.

Limited to 64 logical cores due to `BitMask<u64>`.

## Mutex

`Mutex<T>` wraps a value of type T and enforces exclusive access through RAII guards. The protected data can only be accessed while holding the lock.

```cpp
#include "opal/threading/mutex.h"

// Create a mutex protecting an int
Opal::Mutex<int> counter(0);

// Lock and access the data
{
    Opal::MutexGuard<int> guard = counter.Lock();
    *guard.Deref() += 1;
    // Mutex is unlocked when guard goes out of scope
}
```

```cpp
// Protecting a more complex type
struct GameState
{
    int score = 0;
    bool running = true;
};

Opal::Mutex<GameState> state(GameState{});

auto t = Opal::CreateThread([](Opal::Mutex<GameState>& state)
{
    auto guard = state.Lock();
    guard.Deref()->score += 100;
}, Opal::Ref(state));
Opal::JoinThread(t.GetValue());
```

### TryLock

`TryLock()` attempts to acquire the lock without blocking. Returns `Expected<MutexGuard<T>, bool>` — on success you get a guard with RAII unlock, on failure you get `false`.

```cpp
Opal::Mutex<int> counter(0);

auto result = counter.TryLock();
if (result.HasValue())
{
    *result.GetValue().Deref() += 1;
    // Mutex is unlocked when result goes out of scope
}
else
{
    // Lock is held by another thread
}
```

The mutex is not recursive on either platform. A thread that locks one it already holds deadlocks; hold the guard instead of
locking twice.

`Mutex`, `ConditionVariable` and `Signal` can be neither copied nor moved. A live `MutexGuard` points at its mutex and at the
data behind it, and a thread inside `Wait` is parked on the address of the condition variable or signal, so relocating any of
them would strand a guard or a waiter. Hold them by value in a type that does not move, or behind a pointer.

The lock is held in place, so constructing a `Mutex` never allocates and never fails.

Platform implementation: `SRWLOCK` on Windows, `pthread_mutex_t` on Linux.

### API Reference

| Type | Method | Description |
|------|--------|-------------|
| `Mutex<T>` | `Mutex(T&& object)` | Construct with a moved value |
| `Mutex<T>` | `Mutex(Args&&... args)` | Construct T in place |
| `Mutex<T>` | `Lock()` | Acquire the lock, returns `MutexGuard<T>` |
| `Mutex<T>` | `TryLock()` | Non-blocking lock attempt, returns `Expected<MutexGuard<T>, bool>` |
| `MutexGuard<T>` | `Deref()` | Returns `T*` to the protected data |
| `MutexGuard<T>` | `operator*` | Returns `T&` to the protected data |
| `MutexGuard<T>` | `operator->` | Returns `T*` to the protected data |

## Read-Write Mutex

`ReadWriteMutex<T>` wraps a value that many threads may read at once while only one may write it. `LockRead()` returns a
`ReadGuard<T>` that hands out `const T` access, `LockWrite()` a `WriteGuard<T>` that hands out mutable access. The lock is released
when the guard is destroyed.

```cpp
#include "opal/threading/read-write-mutex.h"

Opal::ReadWriteMutex<Opal::DynamicArray<int>> values;

// Any number of threads at once
{
    Opal::ReadGuard<Opal::DynamicArray<int>> reader = values.LockRead();
    size_t count = reader->GetSize();
}

// One thread, with no readers
{
    Opal::WriteGuard<Opal::DynamicArray<int>> writer = values.LockWrite();
    writer->PushBack(42);
}
```

`TryLockRead()` and `TryLockWrite()` return `Expected<ReadGuard<T>, bool>` and `Expected<WriteGuard<T>, bool>`, with `false` when
the lock is not available.

On Linux and Android a thread waiting to write holds off readers that arrive after it, so a steady stream of readers cannot starve
it. Windows makes no fairness promise either way. Neither lock is recursive: a thread that already holds the lock in either mode
and locks it again can deadlock - reads included, since a writer may have queued in between.

The lock is held in place, so constructing a `ReadWriteMutex` never allocates and never fails. Like `Mutex`, it can be neither
copied nor moved.

Platform implementation: `SRWLOCK` on Windows, `pthread_rwlock_t` on Linux.

### API Reference

| Type | Method | Description |
|------|--------|-------------|
| `ReadWriteMutex<T>` | `ReadWriteMutex(T&& object)` | Construct with a moved value |
| `ReadWriteMutex<T>` | `ReadWriteMutex(Args&&... args)` | Construct T in place |
| `ReadWriteMutex<T>` | `LockRead()` | Wait until no writer holds the lock, returns `ReadGuard<T>` |
| `ReadWriteMutex<T>` | `LockWrite()` | Wait until no one holds the lock, returns `WriteGuard<T>` |
| `ReadWriteMutex<T>` | `TryLockRead()` | Non-blocking, returns `Expected<ReadGuard<T>, bool>` |
| `ReadWriteMutex<T>` | `TryLockWrite()` | Non-blocking, returns `Expected<WriteGuard<T>, bool>` |
| `ReadGuard<T>` | `Deref()` / `operator*` / `operator->` | `const T` access to the protected data |
| `WriteGuard<T>` | `Deref()` / `operator*` / `operator->` | `T` access to the protected data |

## Condition Variable

`ConditionVariable` allows threads to wait until another thread signals an event. Works together with `Mutex` and `MutexGuard`.

Prefer the predicate overload of `Wait`. The plain overload waits first and only then returns, so a notification sent before
the waiter got there is lost; the predicate overload tests before waiting at all. Writing the loop by hand works too, as long
as the predicate is tested first:

```cpp
while (!*guard.Deref())
{
    cond.Wait(guard);
}
```

```cpp
#include "opal/threading/condition-variable.h"

Opal::Mutex<bool> ready(false);
Opal::ConditionVariable cond;

// Worker thread waits for a signal
auto t = Opal::CreateThread([&]()
{
    auto guard = ready.Lock();
    cond.Wait(guard, [](bool& is_ready) { return is_ready; });
    // *guard is now true
});

// Main thread signals the worker
{
    auto guard = ready.Lock();
    *guard.Deref() = true;
    cond.NotifyOne();
}

Opal::JoinThread(t.GetValue());
```

### Timed Wait

`WaitFor` blocks until the condition variable is signaled or the timeout expires. Returns `true` if signaled, `false` if timed out. The mutex is re-acquired in both cases.

```cpp
Opal::Mutex<bool> ready(false);
Opal::ConditionVariable cond;

auto guard = ready.Lock();
while (!*guard.Deref())
{
    bool signaled = cond.WaitFor(guard, 1000);  // Wait up to 1 second
    if (!signaled)
    {
        // Timed out
        break;
    }
}
```

### API Reference

| Method | Description |
|--------|-------------|
| `ConditionVariable()` | Construct. Never allocates and never fails |
| `NotifyOne()` | Wake one waiting thread |
| `NotifyAll()` | Wake all waiting threads |
| `Wait(MutexGuard<T>& guard)` | Atomically release mutex and wait, re-acquire on wake. Returns `T*` |
| `Wait(MutexGuard<T>& guard, Predicate pred)` | Wait until `pred(T&)` holds. Tested before the first wait and on every wake. Returns `T*` |
| `WaitFor(MutexGuard<T>& guard, u64 timeout_ms)` | Timed wait. Returns `true` if signaled, `false` if timed out |

Platform implementation: `CONDITION_VARIABLE` on Windows, `pthread_cond_t` on Linux.

## Signal

`Signal` is a lightweight synchronization primitive that allows threads to wait for state changes without requiring a mutex or an allocator. Uses `WaitOnAddress` on Windows and `futex` on Linux. Internally uses a monotonic `u32` counter to avoid lost notifications.

```cpp
#include "opal/threading/signal.h"

Opal::Signal signal;

// Worker thread waits for state to change from 0
auto t = Opal::CreateThread([&]()
{
    signal.Wait(0);  // Blocks while state == 0
    // State has changed, do work
});

// Main thread notifies (advances state and wakes)
signal.NotifyOne();

Opal::JoinThread(t.GetValue());
```

### Polling Loop

Use `GetState` and `Wait` together for a polling loop that efficiently blocks between changes.

```cpp
Opal::Signal signal;

auto t = Opal::CreateThread([&]()
{
    Opal::u32 state = signal.GetState();
    while (running)
    {
        signal.Wait(state);        // Block until state changes
        state = signal.GetState(); // Read new state
        // React to changes...
    }
});

// Producer notifies whenever there is new work
signal.NotifyAll();
```

### Timed Wait

```cpp
Opal::Signal signal;

// Wait up to 100ms for state to change from 0
bool changed = signal.WaitFor(0, 100);
if (!changed)
{
    // Timed out, state is still 0
}
```

### API Reference

| Method | Description |
|--------|-------------|
| `Signal()` | Construct with initial state 0 |
| `GetState()` | Returns current state with acquire semantics |
| `Wait(u32 expected_state)` | Block while state equals expected_state |
| `WaitFor(u32 expected_state, u64 timeout_ms)` | Timed wait, returns `false` on timeout |
| `NotifyOne()` | Advance state and wake one waiting thread |
| `NotifyAll()` | Advance state and wake all waiting threads |

Platform implementation: `WaitOnAddress` / `WakeByAddressSingle` / `WakeByAddressAll` on Windows, `futex` syscall on Linux.

## Channels

Channels provide thread-safe, one-way communication between threads using a bounded queue. Data is sent through a `Transmitter` and received through a `Receiver`.

### SPSC Channel

Single-producer, single-consumer. The transmitter and receiver are move-only. Lock-free using atomics with cache-line aligned indices to prevent false sharing.

Both SPSC and MPMC channels accept a `bool UseSignaling` template parameter:

- **`true`** — blocking operations use `std::atomic::wait/notify` (WaitOnAddress/futex), freeing the CPU while waiting.
- **`false`** — blocking operations busy-wait using a CPU pause instruction, lowest latency but consumes CPU.

All three channels default to `true` (signaling), so a receiver that has nothing to do does not hold a core. Choose spin
wait where consumers have cores of their own and the wakeup latency of a few microseconds matters more.

With signaling, a send or receive only pays for a wakeup when the other side is asleep. A thread about to sleep issues a
process-wide memory barrier first (`FlushProcessWriteBuffers` on Windows, `membarrier` on Linux), which takes microseconds and
briefly interrupts the process's other running threads. That keeps every send and receive free of a hardware fence, and is
paid only when a thread blocks, which costs more anyway.

A receiver with nothing to take polls for a few microseconds, pausing longer each time, before it goes to sleep. Consumers that
run ahead of their producers, such as idle `ThreadPool` workers waiting for the next task, would otherwise sleep and be woken for
every item, and each sleep's barrier slows the producer that has to wake them. The polling costs an MPMC channel a few percent of
throughput when its consumers keep up anyway, and about a quarter with a single consumer on another CCD, which loses the batching
that sleeping gave it. A channel with one consumer should be a `ChannelMPSC`.

```cpp
#include "opal/threading/channel-spsc.h"

Opal::ChannelSPSC<int> channel(128);  // Capacity rounded up to next power of 2

// Move receiver to consumer thread
auto t = Opal::CreateThread(
    [](Opal::ReceiverSPSC<int> receiver)
    {
        auto result = receiver.Receive();     // Blocks until data available
        if (result.HasValue())
        {
            int value = result.GetValue();
        }

        int out;
        Opal::ErrorCode err = receiver.TryReceive(out);  // Non-blocking
        if (err == Opal::ErrorCode::Success)
        {
            // Got data in out
        }
    },
    std::move(channel.receiver));

// Send from producer
channel.transmitter.Send(42);                 // Blocks if full
channel.transmitter.TrySend(43);              // Non-blocking, returns false if full

Opal::JoinThread(t.GetValue());

// Use spin-waiting for lowest latency
Opal::ChannelSPSC<int, false> fast_channel(128);
```

### SPSC Channel Close

The transmitter can close the channel to signal "no more data". After closing, the receiver drains any remaining items in the queue first, then reports `ErrorCode::ChannelClosed`.

```cpp
Opal::ChannelSPSC<int> channel(128);

channel.transmitter.Send(1);
channel.transmitter.Send(2);
channel.transmitter.Close();

// Items sent before Close are still available
auto result = channel.receiver.Receive();
// result.GetValue() == 1

result = channel.receiver.Receive();
// result.GetValue() == 2

// Queue is now empty, closed status is reported
result = channel.receiver.Receive();
// result.GetError() == ErrorCode::ChannelClosed
```

Items sent before `Close()` are always returned before the closed status. `Close()` also wakes a receiver already blocked in
`Receive()`, which then drains what is left and returns `ErrorCode::ChannelClosed`, so a consumer thread can be shut down by
closing its channel; no sentinel value is needed.

### MPMC Channel

Multi-producer, multi-consumer. Transmitters and receivers can be cloned to share across multiple threads. Uses Dmitry Vyukov's bounded MPMC queue algorithm.

The item type must be default constructable and move assignable. Sending an rvalue moves it in, so a move-only type works. Sending
an lvalue copies it, or clones it when the type is not copy assignable; a type that is neither can only be sent as an rvalue.

```cpp
#include "opal/threading/channel-mpmc.h"

Opal::ChannelMPMC<int> channel(256);

// Clone transmitter for multiple producers
Opal::TransmitterMPMC<int> tx2 = channel.transmitter.Clone();

auto producer1 = Opal::CreateThread(
    [](Opal::TransmitterMPMC<int> tx)
    {
        tx.Send(1);
        tx.Send(2);
    },
    std::move(channel.transmitter));

auto producer2 = Opal::CreateThread(
    [](Opal::TransmitterMPMC<int> tx)
    {
        tx.Send(3);
        tx.Send(4);
    },
    std::move(tx2));

// Receive from consumer
int value = channel.receiver.Receive();

Opal::JoinThread(producer1.GetValue());
Opal::JoinThread(producer2.GetValue());
```

### MPMC Channel Close

The MPMC channel supports the same `Close()` semantics as the SPSC channel. The transmitter can close the channel to signal "no more data". After closing, the receiver drains any remaining items first, then reports `ErrorCode::ChannelClosed`.

```cpp
Opal::ChannelMPMC<int> channel(256);

channel.transmitter.Send(1);
channel.transmitter.Send(2);
channel.transmitter.Close();

// Items sent before Close are still available
auto result = channel.receiver.Receive();
// result.GetValue() == 1

result = channel.receiver.Receive();
// result.GetValue() == 2

// Queue is now empty, closed status is reported
result = channel.receiver.Receive();
// result.GetError() == ErrorCode::ChannelClosed
```

`Receive()` and `TryReceive()` behave identically to the SPSC channel. `Close()` wakes every receiver blocked in `Receive()`.
`TryReceive()` returns `ErrorCode::Success`, `ErrorCode::ChannelEmpty`, or `ErrorCode::ChannelClosed`.

Nothing may be sent once `Close()` has been called. A receiver blocked in `Receive()` holds its place in the queue, and gives it
up when the channel closes with nothing sent for it, so an item sent after that may never be received and a later send may block
for good. Debug builds assert on it. When producers may still be sending, wait for them to finish before closing, as
`ThreadPool::Close` does for tasks submitted by running tasks; or, with a single consumer, use an MPSC channel.

### MPSC Channel

Multi-producer, single-consumer: the fan-in shape of worker threads reporting to one thread. Transmitters are cloned for each
producer as with MPMC; the receiver can only be moved, so a second consumer does not compile. Knowing there is one consumer
makes receiving cheaper than through a `ChannelMPMC`. Defaults to signaling, like the other channels.

```cpp
#include "opal/threading/channel-mpsc.h"

Opal::ChannelMPSC<int> channel(256);
Opal::TransmitterMPSC<int> worker_tx = channel.transmitter.Clone();
// hand worker_tx to a worker thread, keep channel.receiver on the consuming thread

int value;
while (channel.receiver.TryReceive(value) == Opal::ErrorCode::Success)
{
    // handle value
}
```

`Close()` works as for the other channels: items sent before it are received first, then `Receive()` and `TryReceive()` report
`ErrorCode::ChannelClosed`. Unlike an MPMC channel, an item sent after `Close()` is still received by a later call, with
either wait mode.

### Channel API Reference

**SPSC**

| Type | Method | Description |
|------|--------|-------------|
| `TransmitterSPSC` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `TransmitterSPSC` | `TrySend(const T&)` | Non-blocking send, returns `false` if full |
| `TransmitterSPSC` | `Close()` | Mark channel as closed and wake a blocked receiver |
| `TransmitterSPSC` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `ReceiverSPSC` | `Receive()` | Blocking receive, returns `Expected<T, ErrorCode>`. Returns `ChannelClosed` once closed and drained |
| `ReceiverSPSC` | `TryReceive(T&)` | Non-blocking receive, returns `ErrorCode` (`Success`, `ChannelEmpty`, or `ChannelClosed`) |
| `ReceiverSPSC` | `IsValid()` | Returns `true` if receiver holds a queue |

SPSC transmitters and receivers are move-only. Both ends share ownership of the channel state, closed flag included, so they stay
valid after the `ChannelSPSC` they came from is destroyed.

**MPMC**

| Type | Method | Description |
|------|--------|-------------|
| `TransmitterMPMC` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `TransmitterMPMC` | `TrySend(const T&)` / `TrySend(T&&)` | Non-blocking send, returns `false` if full. A rejected rvalue is left untouched |
| `TransmitterMPMC` | `Close()` | Mark channel as closed and wake every blocked receiver |
| `TransmitterMPMC` | `Clone()` | Create a shared copy for another producer |
| `TransmitterMPMC` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `ReceiverMPMC` | `Receive()` | Blocking receive, returns `Expected<T, ErrorCode>`. Returns `ChannelClosed` once closed and drained |
| `ReceiverMPMC` | `TryReceive(T&)` | Non-blocking receive, returns `ErrorCode` (`Success`, `ChannelEmpty`, or `ChannelClosed`) |
| `ReceiverMPMC` | `Clone()` | Create a shared copy for another consumer |
| `ReceiverMPMC` | `IsValid()` | Returns `true` if receiver holds a queue |

MPMC transmitters and receivers support `Clone()`. Like the SPSC ends, they stay valid after the `ChannelMPMC` they came from is
destroyed.

**MPSC**

| Type | Method | Description |
|------|--------|-------------|
| `TransmitterMPSC` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `TransmitterMPSC` | `TrySend(const T&)` / `TrySend(T&&)` | Non-blocking send, returns `false` if full. A rejected rvalue is left untouched |
| `TransmitterMPSC` | `Close()` | Mark channel as closed and wake a blocked receiver |
| `TransmitterMPSC` | `Clone()` | Create a shared copy for another producer |
| `TransmitterMPSC` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `ReceiverMPSC` | `Receive()` | Blocking receive, returns `Expected<T, ErrorCode>`. Returns `ChannelClosed` once closed and drained |
| `ReceiverMPSC` | `TryReceive(T&)` | Non-blocking receive, returns `ErrorCode` (`Success`, `ChannelEmpty`, or `ChannelClosed`) |
| `ReceiverMPSC` | `IsValid()` | Returns `true` if receiver holds a queue |

The MPSC receiver is move-only. Both ends stay valid after the `ChannelMPSC` they came from is destroyed.

## Executors

An `Executor` is something that runs work handed to it. `Submit(function)` hands it a callable that takes no arguments and
returns an `Expected<Future<R>, ErrorCode>` for what the callable returns - see [Futures](#futures). Opal has two:

- `ThreadPool` runs work on its own worker threads.
- `TaskQueue` runs work on whichever thread drains it, such as a UI thread once a frame.

`Future::Then(executor, func)` runs a continuation on a chosen executor, which is how work finishes on the UI thread:

```cpp
auto pool = Opal::ThreadPool::Create(4).GetValue();   // Check HasValue in real code
Opal::TaskQueue ui_queue;

auto image = pool->Submit([path = path.Clone()] { return DecodeImage(path); });
Opal::Future<void> shown = image.GetValue().Then(ui_queue, [](Opal::Expected<Image, Opal::ErrorCode>&& decoded)
{
    ShowImage(decoded);   // Runs on the UI thread
});

// UI thread, once a frame
ui_queue.RunPending();
```

To write another executor, derive from `Executor` and implement `Post`, which takes a `SharedPtr<Runnable>` and runs it later,
and `GetAllocator`, which must return a thread-safe allocator. `Submit` comes with the interface. An executor that drops work
without running it calls `Runnable::Abandon` on it, so that a future waiting on that work reports `ErrorCode::BrokenPromise`
instead of waiting forever.

### Task Queue

`TaskQueue` is unbounded, so posting to it never blocks, however long its owner takes to drain it. Any thread may post or
submit; `RunPending()` runs, on the calling thread, the work that was queued when it was called, and returns how many tasks it
ran. Work posted while it runs waits for the next call, so a task that posts itself again cannot keep `RunPending()` from
returning. Call it from one thread at a time.

```cpp
Opal::TaskQueue io_queue;

// A thread that owns a resource and waits for work
while (running)
{
    if (io_queue.WaitFor(100))
    {
        io_queue.RunPending();
    }
}
```

`WaitFor(timeout_ms)` blocks until work is queued. `Close()` stops the queue accepting work, with `ErrorCode::ChannelClosed`, and
leaves what is queued to the next `RunPending()`. Work still queued when the queue is destroyed is abandoned, and its futures
report `ErrorCode::BrokenPromise`.

| Method | Description |
|--------|-------------|
| `TaskQueue(AllocatorBase* allocator = nullptr)` | Create a queue. Never fails; with an allocator that is not thread safe, every `Post` and `Submit` reports `ErrorCode::InvalidArgument` |
| `Submit(Function)` / `Post(SharedPtr<Runnable>)` | Queue work, from any thread |
| `RunPending()` | Run what was queued when called, on the calling thread. Returns the number run |
| `WaitFor(timeout_ms)` | Block until work is queued. Returns `false` on timeout |
| `Close()` | Stop accepting work |
| `GetPendingCount()` | Number of tasks queued and not yet run |

## Thread Pool

A thread pool that runs submitted callables on a fixed number of worker threads and hands back a `Future` for each. Workers
block when idle and wake on submission, consuming no CPU while waiting. Each task and the state of its future are one allocation.

```cpp
#include "opal/threading/thread-pool.h"

auto created = Opal::ThreadPool::Create(8);           // Expected<ScopePtr<ThreadPool>, ErrorCode>, 8 worker threads
if (!created.HasValue())
{
    return created.GetError();
}
Opal::ThreadPool& pool = *created.GetValue();

auto sum = pool.Submit([] { return 2 + 2; });       // Expected<Future<int>, ErrorCode>
if (sum.HasValue())
{
    int value = sum.GetValue().Get().GetValue();    // Blocks until the task has run
}

auto work = pool.Submit([] { /* work */ });         // Future<void>
```

`ThreadPoolDesc` creates a pool with its workers pinned to chosen logical cores and named for debuggers and profilers. Keeping
workers and the threads that feed them inside one L3 domain (see CPU Topology) saves the cost of moving every task between
caches.

```cpp
const Opal::u32 cores[] = {0, 2, 4, 6};                 // One logical core per worker, here the first of each physical core
Opal::ThreadPoolDesc desc;
desc.thread_count = 4;
desc.worker_cores = Opal::ArrayView<const Opal::u32>(cores);
desc.name = "jobs";                                     // Workers are named "jobs 0" to "jobs 3"
auto created = Opal::ThreadPool::Create(desc);
```

Tasks, futures and `ParallelFor` state are allocated from the pool's allocator and usually freed on a worker, so with many small
tasks the allocator is a large share of what a task costs. A `SmallBlockAllocator` (see `docs/memory.md`) keeps a cache per thread
for exactly this pattern:

```cpp
Opal::SmallBlockAllocator allocator;                // Must outlive the pool and every future it hands out
auto created = Opal::ThreadPool::Create(8, 128, &allocator);
```

`Create` returns `ErrorCode::InvalidArgument` for an allocator that is not thread safe, `ErrorCode::OutOfMemory` when the pool or
its channel could not be allocated, and what `CreateThread` reported when a worker did not start; workers that did start are
shut down first. A pool can be neither copied nor moved, so it lives behind the `ScopePtr`.

`Submit` returns `ErrorCode::ChannelClosed` once the pool has been closed and `ErrorCode::OutOfMemory` when the task could not be
allocated. The callable takes no arguments and may return nothing, a value, or an `Expected<R, ErrorCode>`; see
[Futures](#futures) for what each gives. A future that is dropped does not cancel its task.

`WaitForAll()` waits for the whole pool to drain instead of for one task.

```cpp
for (int i = 0; i < 64; ++i)
{
    pool.Submit([] { /* work */ });
}
pool.WaitForAll();
```

`Close()` rejects new tasks, runs every task already accepted - including one whose `Submit` was racing the close - and then
joins the workers. It is called by the destructor.

### Submitting Child Tasks

A running task may submit more tasks to its own pool with `Submit`. These are accepted even while the pool is closing, and run
before `Close()` returns, so a task can hand off the rest of its work without racing shutdown.

A task may submit more than the channel holds. When a worker finds the channel full it runs queued tasks until there is room,
rather than waiting for another worker to make some, since every other worker could be doing the same.

```cpp
pool.Submit([&pool]
{
    for (int i = 0; i < 8; ++i)
    {
        pool.Submit([i] { /* child work */ });
    }
});
```

`WaitForAll()` covers child tasks too: a task is counted as finished only after it returns, so anything it submitted is already
in the queue by then. A task should not block on the future of a task queued behind it: if every worker does that, nothing is left
to run the queue. Attach the follow-up with `Then` instead.

### Parallel For

`ParallelFor` calls a function for every index in a range, spread over the pool's workers, and returns once every call has
returned.

```cpp
Opal::ParallelFor(pool, 0, particles.GetSize(), 256, [&particles, dt](Opal::u64 i)
{
    particles[i].Integrate(dt);
});
```

The range is cut into chunks of `grain_size` indices, which threads claim one at a time, so a slow chunk does not hold up the rest.
Raise the grain until one chunk is worth handing to another thread.

The calling thread takes chunks too, and waits only for chunks already running on other threads, never for work still queued. A
task running on the pool can therefore call `ParallelFor` on the same pool, nested calls included, and it finishes even on a busy,
closed or out of memory pool - the calling thread then does all of the work. Calls happen in no particular order and on any thread,
so the function must be safe to call concurrently with itself, and it must not throw.

### API Reference

| Method | Description |
|--------|-------------|
| `Create(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr)` | Create a pool with N workers, returns `Expected<ScopePtr<ThreadPool>, ErrorCode>` |
| `Create(const ThreadPoolDesc& desc)` | Same, also pinning workers to `desc.worker_cores` and naming them after `desc.name`. `ErrorCode::InvalidArgument` if `worker_cores` is not one core below 64 per worker |
| `Submit(Function)` | Submit a callable taking no arguments, returns `Expected<Future<R>, ErrorCode>` for what it returns |
| `WaitForAll()` | Block until every submitted task has run, child tasks included. Not safe to call while another thread is closing the pool |
| `Close()` | Reject later tasks, run every accepted one, then join all threads. Safe to call multiple times |
| `GetThreadCount()` | Number of worker threads |
| `GetAllocator()` | Allocator used by the pool |
| `ParallelFor(pool, begin, end, grain_size, func)` | Free function. Call `func(index)` for every index in `[begin, end)` across the workers, and wait |

## Futures

A `Future<T>` is a value that arrives later, possibly from another thread. The matching `Promise<T>` sets it, once, with a
value or an `ErrorCode`. The two share reference-counted state, so either side may be destroyed first.

```cpp
#include "opal/threading/future.h"

auto promise = Opal::Promise<int>::Create();     // Expected<Promise<int>, ErrorCode>
Opal::Future<int> future = promise.GetValue().GetFuture();

// On any thread
promise.GetValue().SetValue(42);

// Blocks until the value arrives
Opal::Expected<int, Opal::ErrorCode> result = future.Get();
```

What a future yields is `FutureResult<T>`: an `Expected<T, ErrorCode>`, or for `Future<void>` a plain `ErrorCode` that is
`ErrorCode::Success` when the work finished. A promise destroyed without a value or an error sets `ErrorCode::BrokenPromise`, so a
future never waits for a promise that is gone.

A future has one consumer. It can be moved but not copied, and `Get()`, a successful `TryGet()` and `Then()` each take the result
and leave the future invalid.

### Polling

`IsReady()` and `TryGet()` never block, which suits a loop that checks once per frame. `TryGet()` returns
`ErrorCode::WouldBlock` while the result has not arrived and leaves the future as it was.

```cpp
auto result = future.TryGet();
if (result.HasValue())
{
    Use(result.GetValue());
}
else if (result.GetError() != Opal::ErrorCode::WouldBlock)
{
    ReportFailure(result.GetError());
}
```

`Wait()` blocks without taking the result, and `WaitFor(timeout_ms)` returns `false` if the timeout runs out first.

### From a Thread Pool

`ThreadPool::Submit` runs a callable that takes no arguments and returns a future for what it returns. A callable that returns
nothing gives a `Future<void>`, one that returns `Expected<R, ErrorCode>` gives a `Future<R>` that carries the error.

```cpp
Opal::ThreadPool& pool = *created.GetValue();
auto image = pool.Submit([path = path.Clone()] { return DecodeImage(path); });   // Future<Image>
auto saved = pool.Submit([] { return SaveSettings(); });                         // Future<void>, or Future<R> from an Expected
```

### Continuations

`Then(func)` runs `func` on the result once it arrives and returns a future for what `func` returns. `func` takes the
`FutureResult<T>` by rvalue, errors included, so it decides what an error turns into. Its return type maps the same way as
`Submit`'s: a plain value, `void`, or an `Expected<U, ErrorCode>` that is flattened into a `Future<U>`.

```cpp
Opal::Future<Opal::u64> size = pool.Submit([] { return LoadText(); })
    .GetValue()
    .Then([](Opal::Expected<Opal::StringUtf8, Opal::ErrorCode>&& text) -> Opal::Expected<Opal::u64, Opal::ErrorCode>
          {
              if (!text.HasValue())
              {
                  return Opal::Expected<Opal::u64, Opal::ErrorCode>(text.GetError());
              }
              return Opal::Expected<Opal::u64, Opal::ErrorCode>(text.GetValue().GetSize());
          });
```

`func` runs on the thread that sets the result - a pool worker, for a submitted task - or on the calling thread if the result has
already arrived. A continuation therefore must not block for long, and must not touch state that belongs to one thread, such as UI
objects. `Then` returns an invalid future, and leaves the original one valid, if the continuation could not be allocated.

`Then(executor, func)` posts `func` to an executor instead, even when the result has already arrived, so it runs wherever that
executor runs work - on the UI thread, for a `TaskQueue` that the UI thread drains. Everything it needs is allocated when `Then`
is called, so a result that arrives later only has to be queued. If the executor refuses the work, the next future reports what
`Post` returned, such as `ErrorCode::ChannelClosed`. The executor must outlive the result's arrival.

### Waiting for Several Futures

`WhenAll` turns an array of futures into one future that is ready once all of them are, holding every result in the same order.
Each result is kept as its future reported it, so one failure does not hide what the others produced.

```cpp
Opal::DynamicArray<Opal::Future<Mesh>> loads;
for (const Opal::StringUtf8& path : paths)
{
    auto load = pool.Submit([&path] { return LoadMesh(path); });
    if (load.HasValue())
    {
        loads.PushBack(Opal::Move(load.GetValue()));
    }
}

// Future<DynamicArray<Expected<Mesh, ErrorCode>>>
auto all = Opal::WhenAll(Opal::Move(loads));
all.Then([](auto&& meshes) { /* every load has finished, successfully or not */ });
```

For `Future<void>` the results are `ErrorCode`s, `ErrorCode::Success` for each one that finished. An invalid future in the array
contributes `ErrorCode::InvalidArgument`. `WhenAll` returns an invalid future, and leaves the array as it was, only when its own
state could not be allocated or the allocator is not thread safe.

### API Reference

| Type | Method | Description |
|------|--------|-------------|
| `Promise<T>` | `Create(AllocatorBase* = nullptr)` | Create with shared state, returns `Expected<Promise<T>, ErrorCode>`. The allocator must be thread safe |
| `Promise<T>` | `GetFuture()` | The future this promise fills. Only once |
| `Promise<T>` | `SetValue(T)` / `SetValue()` | Set the value, or finish a `Promise<void>`. Returns `false` if a result was already set |
| `Promise<T>` | `SetResult(Expected<T, ErrorCode>)` | Set the value or error held by an `Expected` |
| `Promise<T>` | `SetError(ErrorCode)` | Set an error |
| `Future<T>` | `IsValid()` / `IsReady()` | Attached and not consumed / result has arrived |
| `Future<T>` | `Wait()` / `WaitFor(timeout_ms)` | Block until the result arrives |
| `Future<T>` | `Get()` | Block, then take the result |
| `Future<T>` | `TryGet()` | Take the result if it has arrived, else `ErrorCode::WouldBlock` |
| `Future<T>` | `Then(func)` | Run `func` on the result, returns a future for what it returns |
| `Future<T>` | `Then(executor, func)` | The same, with `func` posted to `executor` |
| | `WhenAll(DynamicArray<Future<T>>&&, AllocatorBase* = nullptr)` | Future of every result, in order, ready once all have arrived |

## Thread Safety Summary

| Type | Thread Safe? |
|------|-------------|
| `CreateThread` / `JoinThread` | Yes |
| `Mutex<T>` | Yes (that's its purpose) |
| `ReadWriteMutex<T>` | Yes |
| `SleepCurrentThread` / `YieldCurrentThread` / `SetThreadName` | Yes |
| `ConditionVariable` | Yes (when used with Mutex) |
| `Signal` | Yes |
| `ChannelSPSC` | Yes (one producer, one consumer) |
| `ChannelMPMC` | Yes (multiple producers, multiple consumers) |
| `ChannelMPSC` | Yes (multiple producers, one consumer) |
| `ThreadPool` | `Submit` is thread-safe via internal MPMC channel |
| `TaskQueue` | `Submit`, `Post`, `WaitFor`, `Close` from any thread. `RunPending` from one thread at a time |
| `Promise<T>` / `Future<T>` | A promise and its future may be used from different threads. Each one alone is not for sharing |


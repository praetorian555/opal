# Threading

Headers: `opal/threading/thread.h`, `opal/threading/mutex.h`, `opal/threading/condition-variable.h`, `opal/threading/signal.h`, `opal/threading/channel-spsc.h`, `opal/threading/channel-mpmc.h`, `opal/threading/thread-pool.h`

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
    }
}

// Or print everything to the logger
Opal::ErrorCode err = Opal::PrintCpuInfo();
```

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

Platform implementation: `SRWLOCK` on Windows, `pthread_mutex_t` on Linux.

### API Reference

| Type | Method | Description |
|------|--------|-------------|
| `Mutex<T>` | `Mutex(T&& object)` | Construct with a moved value |
| `Mutex<T>` | `Mutex(Args&&... args)` | Construct T in place |
| `Mutex<T>` | `Lock()` | Acquire the lock, returns `MutexGuard<T>` |
| `Mutex<T>` | `TryLock()` | Non-blocking lock attempt, returns `Expected<MutexGuard<T>, bool>` |
| `Mutex<T>` | `Unlock()` | Release the lock manually |
| `MutexGuard<T>` | `Deref()` | Returns `T*` to the protected data |

## Condition Variable

`ConditionVariable` allows threads to wait until another thread signals an event. Works together with `Mutex` and `MutexGuard`.

```cpp
#include "opal/threading/condition-variable.h"

Opal::Mutex<bool> ready(false);
Opal::ConditionVariable cond;

// Worker thread waits for a signal
auto t = Opal::CreateThread([&]()
{
    auto guard = ready.Lock();
    while (!*guard.Deref())
    {
        // Re-check the predicate on every wake, the notify may have arrived
        // before the wait started or the wake may be spurious
        cond.Wait(guard);
    }
    // *guard.Deref() is now true
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
| `ConditionVariable(AllocatorBase* allocator = nullptr)` | Construct with optional allocator |
| `NotifyOne()` | Wake one waiting thread |
| `NotifyAll()` | Wake all waiting threads |
| `Wait(MutexGuard<T>& guard)` | Atomically release mutex and wait, re-acquire on wake. Returns `T*` |
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

SPSC defaults to `true` (signaling). MPMC defaults to `false` (spin).

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

`Receive()` attempts a non-blocking pop first, then checks the closed flag, and only blocks if neither succeeds. This means items already in the queue are always returned before the closed status, but if `Receive()` is already blocked waiting for data, `Close()` alone will not unblock it — use `TryReceive()` in a polling loop for clean shutdown patterns where the receiver may be waiting.

### MPMC Channel

Multi-producer, multi-consumer. Transmitters and receivers can be cloned to share across multiple threads. Uses Dmitry Vyukov's bounded MPMC queue algorithm.

The item type must be default constructable, and either copy assignable or clonable. A type that is neither fails to compile.

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

`Receive()` and `TryReceive()` behave identically to the SPSC channel: a non-blocking pop is attempted first, then the closed flag is checked. `TryReceive()` returns `ErrorCode::Success`, `ErrorCode::ChannelEmpty`, or `ErrorCode::ChannelClosed`.

### Channel API Reference

**SPSC**

| Type | Method | Description |
|------|--------|-------------|
| `TransmitterSPSC` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `TransmitterSPSC` | `TrySend(const T&)` | Non-blocking send, returns `false` if full |
| `TransmitterSPSC` | `Close()` | Mark channel as closed |
| `TransmitterSPSC` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `ReceiverSPSC` | `Receive()` | Blocking receive, returns `Expected<T, ErrorCode>`. Returns `ChannelClosed` if closed |
| `ReceiverSPSC` | `TryReceive(T&)` | Non-blocking receive, returns `ErrorCode` (`Success`, `ChannelEmpty`, or `ChannelClosed`) |
| `ReceiverSPSC` | `IsValid()` | Returns `true` if receiver holds a queue |

SPSC transmitters and receivers are move-only.

**MPMC**

| Type | Method | Description |
|------|--------|-------------|
| `TransmitterMPMC` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `TransmitterMPMC` | `TrySend(const T&)` | Non-blocking send, returns `false` if full |
| `TransmitterMPMC` | `Close()` | Mark channel as closed |
| `TransmitterMPMC` | `Clone()` | Create a shared copy for another producer |
| `TransmitterMPMC` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `ReceiverMPMC` | `Receive()` | Blocking receive, returns `Expected<T, ErrorCode>`. Returns `ChannelClosed` if closed |
| `ReceiverMPMC` | `TryReceive(T&)` | Non-blocking receive, returns `ErrorCode` (`Success`, `ChannelEmpty`, or `ChannelClosed`) |
| `ReceiverMPMC` | `Clone()` | Create a shared copy for another consumer |
| `ReceiverMPMC` | `IsValid()` | Returns `true` if receiver holds a queue |

MPMC transmitters and receivers support `Clone()`.

## Thread Pool

A task-based thread pool that distributes work across a fixed number of worker threads using a signaling MPMC channel internally. Worker threads block when idle and wake on task submission, consuming no CPU while waiting. Shutdown is handled by sending sentinel values through the channel to unblock and terminate each worker.

```cpp
#include "opal/threading/thread-pool.h"

Opal::ThreadPool pool(8);  // 8 worker threads

// Submit a task
Opal::SharedPtr<Opal::Task> task = pool.AddFunctionTask(
    [](Opal::Task::TransmitterType&)
    {
        // Do work
    });

// Wait for completion
task->WaitForCompletion();
```

### Submitting Child Tasks

The function passed to `AddFunctionTask` receives a `TransmitterType&` that can be used to submit additional tasks from within a running task.

```cpp
Opal::ThreadPool pool(4);

auto parent = pool.AddFunctionTask([&pool](Opal::Task::TransmitterType& tx)
{
    // Submit child task through the transmitter
    auto child = Opal::MakeShared<Opal::FunctionTask<std::function<void(Opal::Task::TransmitterType&)>>>(
        pool.GetAllocator(),
        [](Opal::Task::TransmitterType&) { /* child work */ });
    tx.Send(Opal::SharedPtr<Opal::Task>{child.Clone()});
    child->WaitForCompletion();
});

parent->WaitForCompletion();
```

### API Reference

| Method | Description |
|--------|-------------|
| `ThreadPool(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr)` | Create pool with N workers |
| `AddFunctionTask(Function)` | Submit a callable, returns `SharedPtr<Task>` |
| `Close()` | Send sentinel tasks to unblock workers, then join all threads. Safe to call multiple times |
| `GetThreadCount()` | Number of worker threads |
| `GetAllocator()` | Allocator used by the pool |

| Task Method | Description |
|-------------|-------------|
| `WaitForCompletion()` | Block until the task finishes (uses OS signaling, not busy-waiting) |
| `IsCompleted()` | Check if the task has finished |

## Thread Safety Summary

| Type | Thread Safe? |
|------|-------------|
| `CreateThread` / `JoinThread` | Yes |
| `Mutex<T>` | Yes (that's its purpose) |
| `ConditionVariable` | Yes (when used with Mutex) |
| `Signal` | Yes |
| `ChannelSPSC` | Yes (one producer, one consumer) |
| `ChannelMPMC` | Yes (multiple producers, multiple consumers) |
| `ThreadPool` | `AddFunctionTask` is thread-safe via internal MPMC channel |


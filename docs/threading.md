# Threading

Headers: `opal/threading/thread.h`, `opal/threading/mutex.h`, `opal/threading/condition-variable.h`, `opal/threading/channel-spsc.h`, `opal/threading/channel-mpmc.h`, `opal/threading/thread-pool.h`

Cross-platform threading primitives for Windows and Linux. Includes threads, mutexes, condition variables, lock-free channels, and a task-based thread pool.

## Quick Start

```cpp
#include "opal/threading/thread.h"
#include "opal/threading/mutex.h"

// Create and join a thread
Opal::ThreadHandle handle = Opal::CreateThread([](int x) { /* runs on new thread */ }, 42);
Opal::JoinThread(handle);

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

```cpp
#include "opal/threading/thread.h"

// Simple thread
Opal::ThreadHandle t = Opal::CreateThread([]() { /* work */ });
Opal::JoinThread(t);

// Thread with arguments
Opal::ThreadHandle t = Opal::CreateThread([](int a, float b) { /* use a, b */ }, 10, 3.14f);
Opal::JoinThread(t);

// Pass by reference using Ref
int result = 0;
Opal::ThreadHandle t = Opal::CreateThread(
    [](int& out) { out = 42; },
    Opal::Ref(result));
Opal::JoinThread(t);
// result == 42
```

The default allocator must be thread-safe when creating threads. The library asserts this at thread creation time and pushes the caller's default allocator onto the new thread's allocator stack.

### Detaching Threads

`DetachThread` allows a thread to run independently without requiring a join. The thread cleans up its own resources when it finishes. After detaching, the handle should not be used with `JoinThread`.

```cpp
Opal::ThreadHandle t = Opal::CreateThread([]()
{
    // This thread runs independently
    // Resources are cleaned up automatically when it finishes
});
Opal::DetachThread(t);
// No need to call JoinThread
```

### Thread ID

Each thread has a `ThreadId` (`u64`) that maps to the platform's native thread identifier. On Windows this is the value from `GetCurrentThreadId()`. On Linux this is the kernel thread ID from `gettid()`, which is the value shown by tools like `htop` and `strace`.

```cpp
Opal::ThreadHandle handle = Opal::CreateThread([]()
{
    Opal::ThreadHandle self = Opal::GetCurrentThreadHandle();
    Opal::ThreadId id = Opal::GetThreadId(self);
    // id is a u64 that can be printed, compared, logged
});
Opal::JoinThread(handle);

// Also accessible directly from the handle
Opal::ThreadId id = handle.id;
```

### Thread Affinity

Pin a thread to a specific logical core.

```cpp
Opal::ThreadHandle t = Opal::CreateThread([]() { /* work */ });
Opal::SetThreadAffinity(t, 0);  // Pin to logical core 0
Opal::JoinThread(t);
```

### CPU Topology

Query the system's CPU topology to discover physical cores, logical cores, and hyperthreading.

```cpp
Opal::CpuInfo info = Opal::GetCpuInfo();

for (Opal::u64 i = 0; i < info.physical_processors.GetSize(); ++i)
{
    const Opal::PhysicalCoreInfo& core = info.physical_processors[i];
    // core.id               - Physical core index
    // core.is_hyperthreaded  - true if SMT is enabled on this core
    // core.logical_cores     - BitMask<u64> of assigned logical core indices
}

// Or print everything to the logger
Opal::PrintCpuInfo();
```

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

Opal::ThreadHandle t = Opal::CreateThread([](Opal::Mutex<GameState>& state)
{
    auto guard = state.Lock();
    guard.Deref()->score += 100;
}, Opal::Ref(state));
Opal::JoinThread(t);
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

Platform implementation: `CRITICAL_SECTION` on Windows, `pthread_mutex_t` on Linux.

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
Opal::ThreadHandle t = Opal::CreateThread([&]()
{
    auto guard = ready.Lock();
    while (!*cond.Wait(guard))
    {
        // Spurious wakeup, keep waiting
    }
    // *guard.Deref() is now true
});

// Main thread signals the worker
{
    auto guard = ready.Lock();
    *guard.Deref() = true;
    cond.NotifyOne();
}

Opal::JoinThread(t);
```

### API Reference

| Method | Description |
|--------|-------------|
| `ConditionVariable(AllocatorBase* allocator = nullptr)` | Construct with optional allocator |
| `NotifyOne()` | Wake one waiting thread |
| `NotifyAll()` | Wake all waiting threads |
| `Wait(MutexGuard<T>& guard)` | Atomically release mutex and wait, re-acquire on wake. Returns `T*` |

Platform implementation: `CONDITION_VARIABLE` on Windows, `pthread_cond_t` on Linux.

## Channels

Channels provide thread-safe, one-way communication between threads using a bounded queue. Data is sent through a `Transmitter` and received through a `Receiver`.

### SPSC Channel

Single-producer, single-consumer. The transmitter and receiver are move-only. Lock-free using atomics with cache-line aligned indices to prevent false sharing.

```cpp
#include "opal/threading/channel-spsc.h"

Opal::ChannelSPSC<int> channel(128);  // Capacity rounded up to next power of 2

// Move receiver to consumer thread
Opal::ThreadHandle t = Opal::CreateThread(
    [](Opal::ReceiverSPSC<int> receiver)
    {
        int value = receiver.Receive();       // Blocks until data available

        int result;
        if (receiver.TryReceive(result))      // Non-blocking
        {
            // Got data
        }
    },
    std::move(channel.receiver));

// Send from producer
channel.transmitter.Send(42);                 // Blocks if full
channel.transmitter.TrySend(43);              // Non-blocking, returns false if full

Opal::JoinThread(t);
```

### MPMC Channel

Multi-producer, multi-consumer. Transmitters and receivers can be cloned to share across multiple threads. Uses Dmitry Vyukov's bounded MPMC queue algorithm.

```cpp
#include "opal/threading/channel-mpmc.h"

Opal::ChannelMPMC<int> channel(256);

// Clone transmitter for multiple producers
Opal::TransmitterMPMC<int> tx2 = channel.transmitter.Clone();

Opal::ThreadHandle producer1 = Opal::CreateThread(
    [](Opal::TransmitterMPMC<int> tx)
    {
        tx.Send(1);
        tx.Send(2);
    },
    std::move(channel.transmitter));

Opal::ThreadHandle producer2 = Opal::CreateThread(
    [](Opal::TransmitterMPMC<int> tx)
    {
        tx.Send(3);
        tx.Send(4);
    },
    std::move(tx2));

// Receive from consumer
int value = channel.receiver.Receive();

Opal::JoinThread(producer1);
Opal::JoinThread(producer2);
```

### Channel API Reference

| Type | Method | Description |
|------|--------|-------------|
| `Transmitter*` | `Send(const T&)` / `Send(T&&)` | Blocking send |
| `Transmitter*` | `TrySend(const T&)` | Non-blocking send, returns `false` if full |
| `Transmitter*` | `IsValid()` | Returns `true` if transmitter holds a queue |
| `Receiver*` | `Receive()` | Blocking receive |
| `Receiver*` | `TryReceive(T&)` | Non-blocking receive, returns `false` if empty |
| `Receiver*` | `IsValid()` | Returns `true` if receiver holds a queue |
| `TransmitterMPMC` | `Clone()` | Create a shared copy for another producer |
| `ReceiverMPMC` | `Clone()` | Create a shared copy for another consumer |

SPSC transmitters and receivers are move-only. MPMC transmitters and receivers support `Clone()`.

## Thread Pool

A task-based thread pool that distributes work across a fixed number of worker threads using an MPMC channel internally.

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
| `Close()` | Signal workers to exit and join all threads |
| `GetThreadCount()` | Number of worker threads |
| `GetAllocator()` | Allocator used by the pool |

| Task Method | Description |
|-------------|-------------|
| `WaitForCompletion()` | Busy-wait until the task finishes |
| `IsCompleted()` | Check if the task has finished |

## Thread Safety Summary

| Type | Thread Safe? |
|------|-------------|
| `CreateThread` / `JoinThread` | Yes |
| `Mutex<T>` | Yes (that's its purpose) |
| `ConditionVariable` | Yes (when used with Mutex) |
| `ChannelSPSC` | Yes (one producer, one consumer) |
| `ChannelMPMC` | Yes (multiple producers, multiple consumers) |
| `ThreadPool` | `AddFunctionTask` is thread-safe via internal MPMC channel |

## Planned Improvements

### Timed Wait on Condition Variable

`ConditionVariable::Wait` blocks indefinitely. A `WaitFor(MutexGuard<T>&, duration)` variant would allow timed waits, useful for worker threads that need periodic wake-ups. Supported by `SleepConditionVariableCS` with a timeout on Windows and `pthread_cond_timedwait` on Linux.

### Channel Close

Channels currently have no way to signal "no more data". A `Close()` method on the transmitter side would make `Receive()` return an error or empty value, enabling clean shutdown patterns without external atomics like the thread pool's `m_should_exit`.

### Thread Pool Blocking Wait

Worker threads currently busy-wait with `TryReceive()` in a tight loop, which wastes CPU when idle. Switching to a blocking `Receive()` or using a condition variable internally would reduce CPU usage when the pool has no work.

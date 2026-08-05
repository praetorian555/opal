#include "test-helpers.h"

#include <chrono>
#include <type_traits>

#include "opal/container/scope-ptr.h"
#include "opal/exceptions.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string.h"
#include "opal/rng.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/channel-spsc.h"
#include "opal/threading/condition-variable.h"
#include "opal/threading/signal.h"
#include "opal/threading/mutex.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"

using namespace Opal;

namespace
{
// The tests below are about what the threads do, not about the start failing, so they unwrap here.
template <typename Function, typename... Args>
ThreadHandle CreateThreadOrFail(Function&& function, Args&&... args)
{
    Expected<ThreadHandle, ErrorCode> handle = CreateThread(std::forward<Function>(function), std::forward<Args>(args)...);
    REQUIRE(handle.HasValue());
    return handle.GetValue();
}

// Thread-safe, so it gets past CreateThread's assert, and hands back nothing, so the thread state cannot be allocated.
struct ThreadSafeNullAllocator final : AllocatorBase
{
    ThreadSafeNullAllocator() : AllocatorBase("ThreadSafeNullAllocator") {}

    void* Alloc(u64, u64) override { return nullptr; }
    void Free(void*) override {}
    [[nodiscard]] bool IsThreadSafe() const override { return true; }
};

// Hands out memory until it is switched off, so a type that allocates in its constructor can be built and then be made to fail.
struct FailAfterSwitchAllocator final : AllocatorBase
{
    FailAfterSwitchAllocator() : AllocatorBase("FailAfterSwitchAllocator") {}

    void* Alloc(u64 size, u64 alignment) override { return m_fail.load() ? nullptr : m_inner.Alloc(size, alignment); }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return true; }

    MallocAllocator m_inner;
    std::atomic<bool> m_fail{false};
};
}  // namespace

TEST_CASE("Create a thread", "[Thread]")
{
    int n = 5;
    ThreadHandle handle = CreateThreadOrFail(
        [](int nn)
        {
            REQUIRE(nn == 5);
            nn++;
        },
        n);

    JoinThread(handle);
    REQUIRE(n == 5);

    ThreadHandle out_handle;
    handle = CreateThreadOrFail(
        [](int& nn, ThreadHandle& inner_handle)
        {
            REQUIRE(nn == 5);
            nn++;
            inner_handle = GetCurrentThreadHandle();
        },
        Ref(n), Ref(out_handle));

    JoinThread(handle);
    REQUIRE(n == 6);
    REQUIRE(out_handle == handle);
    REQUIRE(GetCurrentThreadHandle() != out_handle);
}

TEST_CASE("Create a thread out of memory", "[Thread]")
{
    ThreadSafeNullAllocator allocator;
    const PushDefault pd(&allocator);

    const Expected<ThreadHandle, ErrorCode> handle = CreateThread([]() {});
    REQUIRE_FALSE(handle.HasValue());
    CHECK(handle.GetError() == ErrorCode::OutOfMemory);
}

TEST_CASE("SPSC queue basic Push and Pop", "[Thread]")
{
    DynamicArray<i64> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    Impl::QueueSPSC<i64> queue(32);

    const ThreadHandle handle = CreateThreadOrFail(
        [](Impl::QueueSPSC<i64>& in_queue, Opal::DynamicArray<i64>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const i64 value = in_data[count];
                const i64 queued_value = in_queue.Pop();
                REQUIRE(queued_value == value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    for (const i64& value : data)
    {
        queue.Push(value);
    }
    JoinThread(handle);
}

TEST_CASE("SPSC queue TryPush", "[Thread]")
{
    DynamicArray<i64> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    Impl::QueueSPSC<i64> queue(32);

    const ThreadHandle handle = CreateThreadOrFail(
        [](Impl::QueueSPSC<i64>& in_queue, Opal::DynamicArray<i64>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const i64 value = in_data[count];
                i64 queued_value;
                while (!in_queue.TryPop(queued_value))
                {
                }
                REQUIRE(queued_value == value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    for (const i64& value : data)
    {
        while (!queue.TryPush(value))
            ;
    }
    JoinThread(handle);
}

TEST_CASE("SPSC queue Push with move", "[Thread]")
{
    struct Data
    {
        i64 value = 0;
        Data() = default;
        Data(i64 v) : value(v) {}

        Data Clone(AllocatorBase* = nullptr) const
        {
            Data data(value);
            return data;
        }

        Data(Data&& other) : value(other.value) { other.value = 0; }

        Data& operator=(Data&& other)
        {
            value = other.value;
            other.value = 0;
            return *this;
        }
    };

    DynamicArray<Data> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    Impl::QueueSPSC<Data> queue(32);

    const ThreadHandle handle = CreateThreadOrFail(
        [](Impl::QueueSPSC<Data>& in_queue, Opal::DynamicArray<Data>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const Data& value = in_data[count];
                const Data queued_value = in_queue.Pop();
                REQUIRE(queued_value.value == value.value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    DynamicArray<Data> data_for_init = data.Clone();
    for (Data& value : data_for_init)
    {
        queue.Push(Move(value));
        REQUIRE(value.value == 0);
    }
    JoinThread(handle);
}

TEST_CASE("SPSC queue Push with Emplace", "[Thread]")
{
    struct Data
    {
        i32 a = 0;
        f32 b = 0.0f;
        Data() = default;
        Data(i32 aa, f32 bb) : a(aa), b(bb) {}
    };

    Impl::QueueSPSC<Data> queue(32);
    queue.PushWithEmplace(5, 5.0f);
    Data data = queue.Pop();
    REQUIRE(data.a == 5);
    REQUIRE(data.b == 5.0f);
}

TEST_CASE("SPSC queue Try Push with Emplace", "[Thread]")
{
    struct Data
    {
        i32 a = 0;
        f32 b = 0.0f;
        Data() = default;
        Data(i32 aa, f32 bb) : a(aa), b(bb) {}
    };

    Impl::QueueSPSC<Data> queue(32);
    REQUIRE(queue.TryPushWithEmplace(5, 5.0f));
    Data data = queue.Pop();
    REQUIRE(data.a == 5);
    REQUIRE(data.b == 5.0f);
}

TEST_CASE("Shared pointer", "[Thread]")
{
    SECTION("Creation and cloning")
    {
        SharedPtr<i32> ptr(GetDefaultAllocator(), 5);
        REQUIRE(5 == *ptr.Get());
        {
            SharedPtr<i32> ptr2 = ptr.Clone();
            REQUIRE(5 == *ptr2.Get());
            REQUIRE(ptr == ptr2);
        }
        REQUIRE(ptr.IsValid());
        REQUIRE(5 == *ptr.Get());
    }
    SECTION("Moving")
    {
        SharedPtr<i32> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32> ptr2 = Move(ptr);
        REQUIRE(ptr2.IsValid());
        REQUIRE(5 == *ptr2.Get());
        REQUIRE(!ptr.IsValid());
        SharedPtr<i32> ptr3(GetDefaultAllocator(), 10);
        ptr3 = Move(ptr2);
        REQUIRE(!ptr2.IsValid());
        REQUIRE(ptr3.IsValid());
        REQUIRE(5 == *ptr3.Get());
    }
    SECTION("Reset")
    {
        SharedPtr<i32> ptr(GetDefaultAllocator(), 5);
        ptr.Reset();
        REQUIRE(!ptr.IsValid());
    }
}

TEST_CASE("SPSC channel", "[Thread]")
{
    SECTION("Basic usage")
    {
        ChannelSPSC<i32> channel(128);
        channel.transmitter.Send(5);
        REQUIRE(channel.transmitter.TrySend(10));
        auto result = channel.receiver.Receive();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 5);
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::Success);
        REQUIRE(val == 10);
    }
    SECTION("With different threads")
    {
        ChannelSPSC<i32> channel(128);

        const ThreadHandle t = CreateThreadOrFail(
            [](ReceiverSPSC<i32> receiver)
            {
                REQUIRE(receiver.IsValid());
                auto result = receiver.Receive();
                REQUIRE(result.HasValue());
                REQUIRE(result.GetValue() == 5);
            },
            Move(channel.receiver));

        REQUIRE(!channel.receiver.IsValid());
        channel.transmitter.Send(5);

        JoinThread(t);
    }
}

TEST_CASE("SPSC queue basic Push and Pop with spin wait", "[Thread]")
{
    DynamicArray<i64> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    Impl::QueueSPSC<i64, false> queue(32);

    const ThreadHandle handle = CreateThreadOrFail(
        [](Impl::QueueSPSC<i64, false>& in_queue, Opal::DynamicArray<i64>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const i64 value = in_data[count];
                const i64 queued_value = in_queue.Pop();
                REQUIRE(queued_value == value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    for (const i64& value : data)
    {
        queue.Push(value);
    }
    JoinThread(handle);
}

TEST_CASE("SPSC channel with spin wait", "[Thread]")
{
    SECTION("Basic usage")
    {
        ChannelSPSC<i32, false> channel(128);
        channel.transmitter.Send(5);
        REQUIRE(channel.transmitter.TrySend(10));
        auto result = channel.receiver.Receive();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 5);
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::Success);
        REQUIRE(val == 10);
    }
    SECTION("With different threads")
    {
        ChannelSPSC<i32, false> channel(128);

        const ThreadHandle t = CreateThreadOrFail(
            [](ReceiverSPSC<i32, false> receiver)
            {
                REQUIRE(receiver.IsValid());
                auto result = receiver.Receive();
                REQUIRE(result.HasValue());
                REQUIRE(result.GetValue() == 5);
            },
            Move(channel.receiver));

        REQUIRE(!channel.receiver.IsValid());
        channel.transmitter.Send(5);

        JoinThread(t);
    }
}

TEST_CASE("SPSC channel Close", "[Thread]")
{
    SECTION("Receive returns ChannelClosed after Close")
    {
        ChannelSPSC<i32> channel(128);
        channel.transmitter.Close();
        auto result = channel.receiver.Receive();
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::ChannelClosed);
    }
    SECTION("TryReceive returns ChannelClosed after Close")
    {
        ChannelSPSC<i32> channel(128);
        channel.transmitter.Close();
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::ChannelClosed);
    }
    SECTION("TryReceive returns ChannelEmpty on empty channel")
    {
        ChannelSPSC<i32> channel(128);
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::ChannelEmpty);
    }
    SECTION("Close after Send, items are drained first")
    {
        ChannelSPSC<i32> channel(128);
        channel.transmitter.Send(42);
        channel.transmitter.Close();
        auto result = channel.receiver.Receive();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 42);
        // Now that the queue is drained, next receive reports closed
        auto result2 = channel.receiver.Receive();
        REQUIRE(!result2.HasValue());
        REQUIRE(result2.GetError() == ErrorCode::ChannelClosed);
    }
    SECTION("Close with receiver on different thread")
    {
        ChannelSPSC<i32> channel(128);

        std::atomic<bool> thread_started{false};
        const ThreadHandle t = CreateThreadOrFail(
            [](ReceiverSPSC<i32> receiver, std::atomic<bool>& started)
            {
                started.store(true);
                // Spin-check until closed (receiver won't block since we don't call blocking Receive)
                i32 val;
                ErrorCode err;
                do
                {
                    err = receiver.TryReceive(val);
                } while (err == ErrorCode::ChannelEmpty);
                REQUIRE(err == ErrorCode::ChannelClosed);
            },
            Move(channel.receiver), Ref(thread_started));

        // Wait for thread to start
        while (!thread_started.load())
        {
        }
        channel.transmitter.Close();
        JoinThread(t);
    }
}

TEST_CASE("Mutex", "[Thread]")
{
    Mutex<i32> mutex(5);
    {
        auto guard = mutex.Lock();
        REQUIRE(*guard.Deref() == 5);
    }
}

TEST_CASE("Mutex TryLock", "[Thread]")
{
    SECTION("Succeeds when not locked")
    {
        Mutex<i32> mutex(42);
        auto result = mutex.TryLock();
        REQUIRE(result.HasValue());
        REQUIRE(*result.GetValue().Deref() == 42);
    }
    SECTION("Fails when already locked")
    {
        Mutex<i32> mutex(42);
        auto guard = mutex.Lock();
        bool try_lock_failed = false;
        const ThreadHandle t = CreateThreadOrFail(
            [](Mutex<i32>& m, bool& failed)
            {
                auto result = m.TryLock();
                failed = !result.HasValue();
            },
            Ref(mutex), Ref(try_lock_failed));
        JoinThread(t);
        REQUIRE(try_lock_failed);
    }
}

TEST_CASE("Condition Variable", "[Thread]")
{
    Mutex<bool> mutex(false);
    ConditionVariable cond;

    const ThreadHandle t = CreateThreadOrFail(
        [&]()
        {
            auto guard = mutex.Lock();
            while (!*cond.Wait(guard))
            {
            }
            REQUIRE(*guard.Deref() == true);
        });

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);
    {
        auto guard = mutex.Lock();
        *guard.Deref() = true;
        cond.NotifyOne();
    }

    JoinThread(t);
    REQUIRE(*mutex.Lock().Deref() == true);
}

TEST_CASE("Condition Variable WaitFor timeout", "[Thread]")
{
    Mutex<bool> mutex(false);
    ConditionVariable cond;

    auto guard = mutex.Lock();
    const bool signaled = cond.WaitFor(guard, 100);
    REQUIRE(signaled == false);
}

TEST_CASE("Condition Variable WaitFor signaled", "[Thread]")
{
    Mutex<bool> mutex(false);
    ConditionVariable cond;

    const ThreadHandle t = CreateThreadOrFail(
        [&]()
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            auto guard = mutex.Lock();
            *guard.Deref() = true;
            cond.NotifyOne();
        });

    auto guard = mutex.Lock();
    while (!*guard.Deref())
    {
        const bool signaled = cond.WaitFor(guard, 5000);
        if (!signaled)
        {
            break;
        }
    }
    REQUIRE(*guard.Deref() == true);

    JoinThread(t);
}

TEST_CASE("MPMC Channel", "[Thread]")
{
    ChannelMPMC<i32> channel(128);
    channel.transmitter.Send(5);
    auto result = channel.receiver.Receive();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == 5);

    ChannelMPMC<i32> channel2(0);
    i32 val;
    REQUIRE(channel2.transmitter.TrySend(5) == true);
    REQUIRE(channel2.transmitter.TrySend(10) == false);
    REQUIRE(channel2.receiver.TryReceive(val) == ErrorCode::Success);
    REQUIRE(val == 5);
    REQUIRE(channel2.receiver.TryReceive(val) == ErrorCode::ChannelEmpty);
}

TEST_CASE("MPMC channel Close", "[Thread]")
{
    SECTION("Receive returns ChannelClosed after Close")
    {
        ChannelMPMC<i32> channel(128);
        channel.transmitter.Close();
        auto result = channel.receiver.Receive();
        REQUIRE(!result.HasValue());
        REQUIRE(result.GetError() == ErrorCode::ChannelClosed);
    }
    SECTION("TryReceive returns ChannelClosed after Close")
    {
        ChannelMPMC<i32> channel(128);
        channel.transmitter.Close();
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::ChannelClosed);
    }
    SECTION("TryReceive returns ChannelEmpty on empty channel")
    {
        ChannelMPMC<i32> channel(128);
        i32 val;
        REQUIRE(channel.receiver.TryReceive(val) == ErrorCode::ChannelEmpty);
    }
    SECTION("Close after Send, items are drained first")
    {
        ChannelMPMC<i32> channel(128);
        channel.transmitter.Send(42);
        channel.transmitter.Close();
        auto result = channel.receiver.Receive();
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 42);
        // Now that the queue is drained, next receive reports closed
        auto result2 = channel.receiver.Receive();
        REQUIRE(!result2.HasValue());
        REQUIRE(result2.GetError() == ErrorCode::ChannelClosed);
    }
    SECTION("Close with receiver on different thread")
    {
        ChannelMPMC<i32> channel(128);

        std::atomic<bool> thread_started{false};
        const ThreadHandle t = CreateThreadOrFail(
            [](ReceiverMPMC<i32> receiver, Ref<std::atomic<bool>> started)
            {
                started->store(true);
                i32 val;
                ErrorCode err;
                do
                {
                    err = receiver.TryReceive(val);
                } while (err == ErrorCode::ChannelEmpty);
                REQUIRE(err == ErrorCode::ChannelClosed);
            },
            channel.receiver.Clone(), Ref(thread_started));

        // Wait for thread to start
        while (!thread_started.load())
        {
        }
        channel.transmitter.Close();
        JoinThread(t);
    }
}

TEST_CASE("Thread pool", "[Thread]")
{
    ThreadPool pool(8);
    i32 value = 5;
    auto task = pool.AddFunctionTask([&value](Task::TransmitterType&) { value = 10; });
    REQUIRE(task.HasValue());
    task.GetValue()->WaitForCompletion();
    REQUIRE(value == 10);
}

TEST_CASE("Thread pool captures string", "[Thread]")
{
    ThreadPool pool(8);
    StringUtf8 value = "Hello";
    auto task = pool.AddFunctionTask([moved_value = value.Clone()](Task::TransmitterType&) { REQUIRE(moved_value == "Hello"); });
    REQUIRE(task.HasValue());
    task.GetValue()->WaitForCompletion();
    REQUIRE(value == "Hello");
}

TEST_CASE("Thread pool rejects tasks after it is closed", "[Thread]")
{
    ThreadPool pool(2);
    pool.Close();

    auto task = pool.AddFunctionTask([](Task::TransmitterType&) {});
    REQUIRE_FALSE(task.HasValue());
    REQUIRE(task.GetError() == ErrorCode::ChannelClosed);
}

TEST_CASE("Thread pool reports a task it could not allocate", "[Thread]")
{
    FailAfterSwitchAllocator allocator;
    ThreadPool pool(2, 128, &allocator);
    allocator.m_fail.store(true);

    auto task = pool.AddFunctionTask([](Task::TransmitterType&) {});
    REQUIRE_FALSE(task.HasValue());
    REQUIRE(task.GetError() == ErrorCode::OutOfMemory);

    allocator.m_fail.store(false);
}

TEST_CASE("Thread pool WaitForAll drains every task", "[Thread]")
{
    ThreadPool pool(4);
    std::atomic<i32> counter{0};

    for (i32 i = 0; i < 64; ++i)
    {
        auto task = pool.AddFunctionTask([&counter](Task::TransmitterType&) { counter.fetch_add(1); });
        REQUIRE(task.HasValue());
    }

    pool.WaitForAll();
    REQUIRE(counter.load() == 64);
}

TEST_CASE("Thread pool WaitForAll drains tasks submitted by other tasks", "[Thread]")
{
    ThreadPool pool(4);
    std::atomic<i32> counter{0};

    auto parent = pool.AddFunctionTask(
        [&counter](Task::TransmitterType& transmitter)
        {
            counter.fetch_add(1);
            auto child_work = [&counter](Task::TransmitterType&) { counter.fetch_add(1); };
            for (i32 i = 0; i < 8; ++i)
            {
                SharedPtr<FunctionTask<decltype(child_work)>> child(GetDefaultAllocator(), child_work);
                transmitter.Send(SharedPtr<Task>{Move(child)});
            }
        });
    REQUIRE(parent.HasValue());

    pool.WaitForAll();
    REQUIRE(counter.load() == 9);
}

TEST_CASE("Signal initial state", "[Thread]")
{
    Signal signal;
    REQUIRE(signal.GetState() == 0);
}

TEST_CASE("Signal NotifyOne wakes waiting thread", "[Thread]")
{
    Signal signal;
    std::atomic<bool> thread_woke_up{false};

    const ThreadHandle t = CreateThreadOrFail(
        [](Signal& sig, std::atomic<bool>& woke_up)
        {
            sig.Wait(0);
            woke_up.store(true);
        },
        Ref(signal), Ref(thread_woke_up));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    REQUIRE(thread_woke_up.load() == false);

    signal.NotifyOne();

    JoinThread(t);
    REQUIRE(thread_woke_up.load() == true);
    REQUIRE(signal.GetState() == 1);
}

TEST_CASE("Signal NotifyAll wakes all waiting threads", "[Thread]")
{
    Signal signal;
    std::atomic<i32> woken_count{0};

    const ThreadHandle t1 = CreateThreadOrFail(
        [](Signal& sig, std::atomic<i32>& count)
        {
            sig.Wait(0);
            count.fetch_add(1);
        },
        Ref(signal), Ref(woken_count));

    const ThreadHandle t2 = CreateThreadOrFail(
        [](Signal& sig, std::atomic<i32>& count)
        {
            sig.Wait(0);
            count.fetch_add(1);
        },
        Ref(signal), Ref(woken_count));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    REQUIRE(woken_count.load() == 0);

    signal.NotifyAll();

    JoinThread(t1);
    JoinThread(t2);
    REQUIRE(woken_count.load() == 2);
}

TEST_CASE("Signal WaitFor returns false on timeout", "[Thread]")
{
    Signal signal;
    bool result = signal.WaitFor(0, 50);
    REQUIRE(result == false);
}

TEST_CASE("Signal WaitFor returns true when state changes", "[Thread]")
{
    Signal signal;

    const ThreadHandle t = CreateThreadOrFail(
        [](Signal& sig)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(50ms);
            sig.NotifyOne();
        },
        Ref(signal));

    bool result = signal.WaitFor(0, 5000);
    REQUIRE(result == true);
    REQUIRE(signal.GetState() == 1);

    JoinThread(t);
}

TEST_CASE("Mutex reports a failed allocation", "[Thread]")
{
    ThreadSafeNullAllocator null_allocator;
    PushDefault guard(&null_allocator);

    REQUIRE_THROWS_AS(Mutex<bool>(false), OutOfMemoryException);
}

TEST_CASE("Condition variable reports a failed allocation", "[Thread]")
{
    ThreadSafeNullAllocator null_allocator;

    REQUIRE_THROWS_AS(ConditionVariable(&null_allocator), OutOfMemoryException);
}

TEST_CASE("Mutex, condition variable and signal are neither copyable nor movable", "[Thread]")
{
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<Mutex<bool>>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<Mutex<bool>>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<ConditionVariable>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<ConditionVariable>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<Signal>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<Signal>);

    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<Mutex<bool>>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<ConditionVariable>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<Signal>);
}

// This is the loop docs/threading.md used to demonstrate, inverted. `while (!*cond.Wait(guard))` waits before it tests the
// predicate, so the notification below arrives while nobody is waiting yet and the thread never wakes. Written that way this
// test hung the suite on both platforms.
TEST_CASE("A condition variable waiter that tests its predicate first does not lose a notification", "[Thread]")
{
    Mutex<bool> ready(false);
    ConditionVariable cond;

    {
        auto guard = ready.Lock();
        *guard.Deref() = true;
    }
    cond.NotifyOne();

    const ThreadHandle waiter = CreateThreadOrFail(
        [](Mutex<bool>& in_ready, ConditionVariable& in_cond)
        {
            auto guard = in_ready.Lock();
            while (!*guard.Deref())
            {
                in_cond.Wait(guard);
            }
        },
        Ref(ready), Ref(cond));

    JoinThread(waiter);

    auto guard = ready.Lock();
    REQUIRE(*guard.Deref());
}

TEST_CASE("Condition variable predicate wait does not lose a notification", "[Thread]")
{
    Mutex<bool> ready(false);
    ConditionVariable cond;

    {
        auto guard = ready.Lock();
        *guard.Deref() = true;
    }
    cond.NotifyOne();

    const ThreadHandle waiter = CreateThreadOrFail(
        [](Mutex<bool>& in_ready, ConditionVariable& in_cond)
        {
            auto guard = in_ready.Lock();
            in_cond.Wait(guard, [](bool& is_ready) { return is_ready; });
        },
        Ref(ready), Ref(cond));

    JoinThread(waiter);

    auto guard = ready.Lock();
    REQUIRE(*guard.Deref());
}

TEST_CASE("Condition variable predicate wait blocks until the predicate holds", "[Thread]")
{
    Mutex<int> counter(0);
    ConditionVariable cond;

    const ThreadHandle waiter = CreateThreadOrFail(
        [](Mutex<int>& in_counter, ConditionVariable& in_cond)
        {
            auto guard = in_counter.Lock();
            in_cond.Wait(guard, [](int& value) { return value >= 3; });
            REQUIRE(*guard.Deref() >= 3);
        },
        Ref(counter), Ref(cond));

    for (int i = 0; i < 3; ++i)
    {
        {
            auto guard = counter.Lock();
            *guard.Deref() += 1;
        }
        cond.NotifyAll();
    }

    JoinThread(waiter);
}

TEST_CASE("Mutex guard dereference operators", "[Thread]")
{
    struct Counter
    {
        int value = 0;
    };

    Mutex<Counter> counter(Counter{});

    auto guard = counter.Lock();
    guard->value = 7;
    REQUIRE((*guard).value == 7);
    REQUIRE(guard.Deref()->value == 7);

    const MutexGuard<Counter>& const_guard = guard;
    REQUIRE(const_guard->value == 7);
    REQUIRE((*const_guard).value == 7);
}

TEST_CASE("Mutex guard move assignment releases the lock it held", "[Thread]")
{
    Mutex<int> first(1);
    Mutex<int> second(2);

    MutexGuard<int> guard = first.Lock();
    guard = second.Lock();

    // The lock on first has to be gone, or this blocks forever.
    auto reacquired = first.TryLock();
    REQUIRE(reacquired.HasValue());
    REQUIRE(*reacquired.GetValue().Deref() == 1);
}

TEST_CASE("Signal multiple notify calls", "[Thread]")
{
    Signal signal;
    signal.NotifyOne();
    signal.NotifyOne();
    signal.NotifyAll();
    REQUIRE(signal.GetState() == 3);
}

TEST_CASE("Signal wait returns immediately when state already changed", "[Thread]")
{
    Signal signal;
    signal.NotifyOne();
    // State is now 1, waiting on 0 should return immediately
    signal.Wait(0);
    REQUIRE(signal.GetState() == 1);
}

#include "test-helpers.h"

#include <chrono>

#include "opal/container/scope-ptr.h"
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

TEST_CASE("Create a thread", "[Thread]")
{
    int n = 5;
    ThreadHandle handle = CreateThread(
        [](int nn)
        {
            REQUIRE(nn == 5);
            nn++;
        },
        n);

    JoinThread(handle);
    REQUIRE(n == 5);

    ThreadHandle out_handle;
    handle = CreateThread(
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

    const ThreadHandle handle = CreateThread(
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

    const ThreadHandle handle = CreateThread(
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

    const ThreadHandle handle = CreateThread(
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

    const DynamicArray<Data> data_for_init = data.Clone();
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

        const ThreadHandle t = CreateThread(
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

    const ThreadHandle handle = CreateThread(
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

        const ThreadHandle t = CreateThread(
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
        const ThreadHandle t = CreateThread(
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
        const ThreadHandle t = CreateThread(
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

    const ThreadHandle t = CreateThread(
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
        const ThreadHandle t = CreateThread(
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
    task->WaitForCompletion();
    REQUIRE(value == 10);
}

TEST_CASE("Thread pool captures string", "[Thread]")
{
    ThreadPool pool(8);
    StringUtf8 value = "Hello";
    auto task = pool.AddFunctionTask([moved_value = value.Clone()](Task::TransmitterType&) { REQUIRE(moved_value == "Hello"); });
    task->WaitForCompletion();
    REQUIRE(value == "Hello");
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

    const ThreadHandle t = CreateThread(
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

    const ThreadHandle t1 = CreateThread(
        [](Signal& sig, std::atomic<i32>& count)
        {
            sig.Wait(0);
            count.fetch_add(1);
        },
        Ref(signal), Ref(woken_count));

    const ThreadHandle t2 = CreateThread(
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

    const ThreadHandle t = CreateThread(
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

TEST_CASE("Signal move constructor", "[Thread]")
{
    Signal signal;
    signal.NotifyOne();
    REQUIRE(signal.GetState() == 1);

    Signal moved_signal(Move(signal));
    REQUIRE(moved_signal.GetState() == 1);
    REQUIRE(signal.GetState() == 0);
}

TEST_CASE("Signal move assignment", "[Thread]")
{
    Signal signal;
    signal.NotifyOne();
    REQUIRE(signal.GetState() == 1);

    Signal other;
    other = Move(signal);
    REQUIRE(other.GetState() == 1);
    REQUIRE(signal.GetState() == 0);
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

#include "test-helpers.h"

#include <chrono>
#include <type_traits>

#include "opal/container/scope-ptr.h"
#include "opal/exceptions.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string.h"
#include "opal/rng.h"
#include "opal/time.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/channel-spsc.h"
#include "opal/threading/condition-variable.h"
#include "opal/threading/signal.h"
#include "opal/threading/mutex.h"
#include "opal/threading/read-write-mutex.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#else
#include <pthread.h>
#include <sched.h>
#endif

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

ScopePtr<ThreadPool> CreatePoolOrFail(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr)
{
    Expected<ScopePtr<ThreadPool>, ErrorCode> pool = ThreadPool::Create(thread_count, channel_capacity, allocator);
    REQUIRE(pool.HasValue());
    return std::move(pool.GetValue());
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

// Hands out memory for the first allocations and refuses every one after, so each allocation a type makes can be made to fail.
struct FailFromNthAllocator final : AllocatorBase
{
    explicit FailFromNthAllocator(i32 succeed_count) : AllocatorBase("FailFromNthAllocator"), m_remaining(succeed_count) {}

    void* Alloc(u64 size, u64 alignment) override { return m_remaining.fetch_sub(1) > 0 ? m_inner.Alloc(size, alignment) : nullptr; }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return true; }

    MallocAllocator m_inner;
    std::atomic<i32> m_remaining;
};

struct NotThreadSafeAllocator final : AllocatorBase
{
    NotThreadSafeAllocator() : AllocatorBase("NotThreadSafeAllocator") {}

    void* Alloc(u64 size, u64 alignment) override { return m_inner.Alloc(size, alignment); }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return false; }

    MallocAllocator m_inner;
};

// Counts the instances alive, so a slot overwritten without being destroyed shows up as one left over.
struct LiveCounted
{
    static inline i32 s_live = 0;

    i32 value = 0;

    LiveCounted() { ++s_live; }
    explicit LiveCounted(i32 v) noexcept : value(v) { ++s_live; }
    LiveCounted(i32 a, i32 b) : value(a + b) { ++s_live; }
    LiveCounted(const LiveCounted& other) : value(other.value) { ++s_live; }
    LiveCounted(LiveCounted&& other) noexcept : value(other.value) { ++s_live; }
    LiveCounted& operator=(const LiveCounted&) = default;
    LiveCounted& operator=(LiveCounted&&) noexcept = default;
    ~LiveCounted() { --s_live; }
};

struct MoveOnly
{
    i32 value = 0;

    MoveOnly() = default;
    explicit MoveOnly(i32 v) : value(v) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = 0; }
    MoveOnly& operator=(MoveOnly&& other) noexcept
    {
        value = other.value;
        other.value = 0;
        return *this;
    }
};

u32 GetCurrentLogicalCore()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return static_cast<u32>(GetCurrentProcessorNumber());
#else
    return static_cast<u32>(sched_getcpu());
#endif
}
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

TEST_CASE("Pin a thread to a logical core", "[Thread]")
{
    const Expected<CpuInfo, ErrorCode> cpu_info = GetCpuInfo();
    REQUIRE(cpu_info.HasValue());
    // The last core rather than the first, so a thread the call failed to move is unlikely to pass by having started there.
    const u32 core = cpu_info.GetValue().logical_cores_count - 1;
    u32 ran_on = ~0u;
    const ThreadHandle handle = CreateThreadOrFail(
        [](u32 target, u32& out_core)
        {
            SetThreadAffinity(GetCurrentThreadHandle(), target);
            out_core = GetCurrentLogicalCore();
        },
        core, Ref(ran_on));
    JoinThread(handle);
    CHECK(ran_on == core);
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

TEST_CASE("SPSC queue emplace destroys what the slot held", "[Thread]")
{
    LiveCounted::s_live = 0;
    {
        Impl::QueueSPSC<LiveCounted> queue(4);
        for (i32 lap = 0; lap < 3; ++lap)
        {
            queue.PushWithEmplace(1);
            REQUIRE(queue.TryPushWithEmplace(2));
            queue.PushWithEmplace(3, 4);
            REQUIRE(queue.TryPushWithEmplace(5, 6));
            REQUIRE(queue.Pop().value == 1);
            REQUIRE(queue.Pop().value == 2);
            REQUIRE(queue.Pop().value == 7);
            REQUIRE(queue.Pop().value == 11);
        }
    }
    REQUIRE(LiveCounted::s_live == 0);
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

namespace
{
// Sends every item through a four-slot channel, then closes it. Each side pauses now and then, so the other keeps finding the
// channel full or empty and has to wait, or with signaling sleep, for it. The receiver must get every item, in order.
template <bool UseSignaling>
void TransferThroughSPSC()
{
    constexpr i32 k_item_count = 50000;
    constexpr i32 k_pause_every = 5000;
    ChannelSPSC<i32, UseSignaling> channel(4);
    const ThreadHandle producer = CreateThreadOrFail(
        [](TransmitterSPSC<i32, UseSignaling> transmitter)
        {
            for (i32 i = 0; i < k_item_count; ++i)
            {
                if (i % k_pause_every == 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                transmitter.Send(i);
            }
            transmitter.Close();
        },
        Move(channel.transmitter));

    i32 expected = 0;
    i32 out_of_order = 0;
    for (auto result = channel.receiver.Receive(); result.HasValue(); result = channel.receiver.Receive())
    {
        out_of_order += result.GetValue() == expected ? 0 : 1;
        expected++;
        if (expected % k_pause_every == k_pause_every / 2)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    JoinThread(producer);
    REQUIRE(out_of_order == 0);
    REQUIRE(expected == k_item_count);
}
}  // namespace

TEST_CASE("SPSC channel delivers every item in order while both ends wait on each other", "[Thread]")
{
    SECTION("With signaling")
    {
        TransferThroughSPSC<true>();
    }
    SECTION("With spin wait")
    {
        TransferThroughSPSC<false>();
    }
}

TEST_CASE("SPSC channel ends outlive the channel", "[Thread]")
{
    TransmitterSPSC<i32> transmitter;
    ReceiverSPSC<i32> receiver;
    {
        ChannelSPSC<i32> channel(4);
        transmitter = Move(channel.transmitter);
        receiver = Move(channel.receiver);
    }
    transmitter.Send(7);
    transmitter.Close();
    auto result = receiver.Receive();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == 7);
    i32 value = 0;
    REQUIRE(receiver.TryReceive(value) == ErrorCode::ChannelClosed);
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

TEST_CASE("MPMC channel ends outlive the channel", "[Thread]")
{
    TransmitterMPMC<i32> transmitter;
    ReceiverMPMC<i32> receiver;
    {
        ChannelMPMC<i32> channel(4);
        transmitter = channel.transmitter.Clone();
        receiver = Move(channel.receiver);
    }
    transmitter.Send(7);
    transmitter.Close();
    auto result = receiver.Receive();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == 7);
    i32 value = 0;
    REQUIRE(receiver.TryReceive(value) == ErrorCode::ChannelClosed);
}

TEST_CASE("MPMC channel moves what it is handed", "[Thread]")
{
    ChannelMPMC<MoveOnly> channel(2);

    MoveOnly first(1);
    channel.transmitter.Send(Move(first));
    REQUIRE(first.value == 0);

    MoveOnly second(2);
    REQUIRE(channel.transmitter.TrySend(Move(second)));
    REQUIRE(second.value == 0);

    MoveOnly third(3);
    REQUIRE(!channel.transmitter.TrySend(Move(third)));
    REQUIRE(third.value == 3);

    auto received = channel.receiver.Receive();
    REQUIRE(received.HasValue());
    REQUIRE(received.GetValue().value == 1);
    MoveOnly out;
    REQUIRE(channel.receiver.TryReceive(out) == ErrorCode::Success);
    REQUIRE(out.value == 2);
}

namespace
{
constexpr i32 k_received_closed = -1;

// Parks a receiver in Receive on another thread, then either sends to it or closes the channel. Hangs if neither wakes it.
template <typename ChannelType, typename ReceiverType>
i32 ReceiveWhileBlocked(ChannelType& channel, ReceiverType receiver, bool close)
{
    std::atomic<bool> started{false};
    std::atomic<i32> outcome{0};
    const ThreadHandle t = CreateThreadOrFail(
        [](ReceiverType in_receiver, Ref<std::atomic<bool>> in_started, Ref<std::atomic<i32>> in_outcome)
        {
            in_started->store(true);
            auto result = in_receiver.Receive();
            if (result.HasValue())
            {
                in_outcome->store(result.GetValue());
            }
            else
            {
                in_outcome->store(result.GetError() == ErrorCode::ChannelClosed ? k_received_closed : 0);
            }
        },
        Move(receiver), Ref(started), Ref(outcome));

    while (!started.load())
    {
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    if (close)
    {
        channel.transmitter.Close();
    }
    else
    {
        channel.transmitter.Send(9);
    }
    JoinThread(t);
    return outcome.load();
}
}  // namespace

TEST_CASE("Close wakes a receiver blocked in Receive", "[Thread]")
{
    SECTION("SPSC with signaling")
    {
        ChannelSPSC<i32, true> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), true) == k_received_closed);
    }
    SECTION("SPSC with spin wait")
    {
        ChannelSPSC<i32, false> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), true) == k_received_closed);
    }
    SECTION("MPMC with signaling")
    {
        ChannelMPMC<i32, true> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), true) == k_received_closed);
    }
    SECTION("MPMC with spin wait")
    {
        ChannelMPMC<i32, false> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), true) == k_received_closed);
    }
}

TEST_CASE("A receiver blocked in Receive gets an item sent later", "[Thread]")
{
    SECTION("SPSC with signaling")
    {
        ChannelSPSC<i32, true> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), false) == 9);
    }
    SECTION("SPSC with spin wait")
    {
        ChannelSPSC<i32, false> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), false) == 9);
    }
    SECTION("MPMC with signaling")
    {
        ChannelMPMC<i32, true> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), false) == 9);
    }
    SECTION("MPMC with spin wait")
    {
        ChannelMPMC<i32, false> channel(4);
        REQUIRE(ReceiveWhileBlocked(channel, Move(channel.receiver), false) == 9);
    }
}

namespace
{
// Parks receiver_count receivers in Receive, sends item_count items, then closes the channel. Returns how many receivers got an
// item and how many were told the channel closed; the items received are summed into received_sum. Hangs if one is never woken.
template <bool UseSignaling>
void SendThenCloseWhileBlocked(i32 receiver_count, i32 item_count, i32& received_count, i32& closed_count, i32& received_sum)
{
    ChannelMPMC<i32, UseSignaling> channel(4);
    std::atomic<i32> started{0};
    std::atomic<i32> received{0};
    std::atomic<i32> closed{0};
    std::atomic<i32> sum{0};
    DynamicArray<ThreadHandle> threads;
    for (i32 i = 0; i < receiver_count; ++i)
    {
        threads.PushBack(CreateThreadOrFail(
            [](ReceiverMPMC<i32, UseSignaling> receiver, Ref<std::atomic<i32>> in_started, Ref<std::atomic<i32>> in_received,
               Ref<std::atomic<i32>> in_closed, Ref<std::atomic<i32>> in_sum)
            {
                in_started->fetch_add(1);
                auto result = receiver.Receive();
                if (result.HasValue())
                {
                    in_received->fetch_add(1);
                    in_sum->fetch_add(result.GetValue());
                }
                else if (result.GetError() == ErrorCode::ChannelClosed)
                {
                    in_closed->fetch_add(1);
                }
            },
            channel.receiver.Clone(), Ref(started), Ref(received), Ref(closed), Ref(sum)));
    }
    while (started.load() < receiver_count)
    {
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    for (i32 i = 1; i <= item_count; ++i)
    {
        channel.transmitter.Send(i * 10);
    }
    channel.transmitter.Close();
    for (const ThreadHandle& t : threads)
    {
        JoinThread(t);
    }
    received_count = received.load();
    closed_count = closed.load();
    received_sum = sum.load();

    auto late = channel.receiver.Receive();
    REQUIRE(!late.HasValue());
    REQUIRE(late.GetError() == ErrorCode::ChannelClosed);
    i32 value = 0;
    REQUIRE(channel.receiver.TryReceive(value) == ErrorCode::ChannelClosed);
}

// Producers send disjoint ranges and the last to finish closes the channel. Half the receivers drain with Receive and half with
// TryReceive, so both ways of taking an item run against each other. Every item must arrive exactly once.
template <bool UseSignaling>
void TransferThroughMPMC(size_t capacity)
{
    constexpr i32 k_producer_count = 4;
    constexpr i32 k_receiver_count = 4;
    constexpr i32 k_items_per_producer = 5000;
    constexpr i32 k_item_count = k_producer_count * k_items_per_producer;

    ChannelMPMC<i32, UseSignaling> channel(capacity);
    std::atomic<i32> producers_left{k_producer_count};
    DynamicArray<DynamicArray<i32>> received(k_receiver_count);
    DynamicArray<ThreadHandle> threads;
    for (i32 p = 0; p < k_producer_count; ++p)
    {
        threads.PushBack(CreateThreadOrFail(
            [](TransmitterMPMC<i32, UseSignaling> transmitter, i32 first, Ref<std::atomic<i32>> in_producers_left)
            {
                for (i32 i = first; i < first + k_items_per_producer; ++i)
                {
                    transmitter.Send(i);
                }
                if (in_producers_left->fetch_sub(1) == 1)
                {
                    transmitter.Close();
                }
            },
            channel.transmitter.Clone(), p * k_items_per_producer, Ref(producers_left)));
    }
    for (u64 r = 0; r < k_receiver_count; ++r)
    {
        threads.PushBack(CreateThreadOrFail(
            [](ReceiverMPMC<i32, UseSignaling> receiver, bool use_try, Ref<DynamicArray<i32>> out)
            {
                if (use_try)
                {
                    i32 value = 0;
                    ErrorCode err = ErrorCode::Success;
                    while ((err = receiver.TryReceive(value)) != ErrorCode::ChannelClosed)
                    {
                        if (err == ErrorCode::Success)
                        {
                            out->PushBack(value);
                        }
                    }
                    return;
                }
                for (auto result = receiver.Receive(); result.HasValue(); result = receiver.Receive())
                {
                    out->PushBack(result.GetValue());
                }
            },
            channel.receiver.Clone(), r % 2 == 1, Ref(received[r])));
    }
    for (const ThreadHandle& t : threads)
    {
        JoinThread(t);
    }

    DynamicArray<i32> seen(k_item_count, 0);
    i32 out_of_range = 0;
    for (const DynamicArray<i32>& list : received)
    {
        for (const i32 value : list)
        {
            if (value < 0 || value >= k_item_count)
            {
                out_of_range++;
                continue;
            }
            seen[static_cast<u64>(value)]++;
        }
    }
    i32 not_once = 0;
    for (const i32 count : seen)
    {
        not_once += count == 1 ? 0 : 1;
    }
    REQUIRE(out_of_range == 0);
    REQUIRE(not_once == 0);
}
}  // namespace

TEST_CASE("Close wakes every MPMC receiver blocked in Receive", "[Thread]")
{
    i32 received = 0;
    i32 closed = 0;
    i32 sum = 0;
    SECTION("With signaling")
    {
        SendThenCloseWhileBlocked<true>(4, 0, received, closed, sum);
    }
    SECTION("With spin wait")
    {
        SendThenCloseWhileBlocked<false>(4, 0, received, closed, sum);
    }
    REQUIRE(received == 0);
    REQUIRE(closed == 4);
}

TEST_CASE("MPMC receivers blocked in Receive share the items sent before Close", "[Thread]")
{
    i32 received = 0;
    i32 closed = 0;
    i32 sum = 0;
    SECTION("With signaling")
    {
        SendThenCloseWhileBlocked<true>(4, 2, received, closed, sum);
    }
    SECTION("With spin wait")
    {
        SendThenCloseWhileBlocked<false>(4, 2, received, closed, sum);
    }
    REQUIRE(received == 2);
    REQUIRE(closed == 2);
    REQUIRE(sum == 30);
}

TEST_CASE("MPMC channel delivers every item once to many receivers", "[Thread]")
{
    SECTION("With signaling, capacity 4")
    {
        TransferThroughMPMC<true>(4);
    }
    SECTION("With signaling, capacity 64")
    {
        TransferThroughMPMC<true>(64);
    }
    SECTION("With spin wait, capacity 4")
    {
        TransferThroughMPMC<false>(4);
    }
    SECTION("With spin wait, capacity 64")
    {
        TransferThroughMPMC<false>(64);
    }
}

TEST_CASE("Thread pool", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(8);
    ThreadPool& pool = *pool_owner;
    i32 value = 5;
    auto task = pool.Submit([&value] { value = 10; });
    REQUIRE(task.HasValue());
    REQUIRE(task.GetValue().Get() == ErrorCode::Success);
    REQUIRE(value == 10);
}

TEST_CASE("Thread pool captures string", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(8);
    ThreadPool& pool = *pool_owner;
    StringUtf8 value = "Hello";
    auto task = pool.Submit([moved_value = value.Clone()] { return moved_value == "Hello"; });
    REQUIRE(task.HasValue());
    REQUIRE(task.GetValue().Get().GetValue());
    REQUIRE(value == "Hello");
}

TEST_CASE("Thread pool rejects tasks after it is closed", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(2);
    ThreadPool& pool = *pool_owner;
    pool.Close();

    auto task = pool.Submit([] {});
    REQUIRE_FALSE(task.HasValue());
    REQUIRE(task.GetError() == ErrorCode::ChannelClosed);
}

TEST_CASE("Thread pool runs every task it accepted while closing", "[Thread]")
{
    constexpr i32 k_iterations = 500;
    i32 not_completed = 0;
    for (i32 iteration = 0; iteration < k_iterations; ++iteration)
    {
        ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(2);
        ThreadPool& pool = *pool_owner;
        DynamicArray<Future<void>> accepted;
        std::atomic<bool> started{false};
        const ThreadHandle submitter = CreateThreadOrFail(
            [](Ref<ThreadPool> in_pool, Ref<DynamicArray<Future<void>>> in_accepted, Ref<std::atomic<bool>> in_started)
            {
                in_started->store(true);
                while (true)
                {
                    auto task = in_pool->Submit([] {});
                    if (!task.HasValue())
                    {
                        break;
                    }
                    in_accepted->PushBack(Move(task.GetValue()));
                }
            },
            Ref(pool), Ref(accepted), Ref(started));

        while (!started.load())
        {
        }
        pool.Close();
        JoinThread(submitter);
        for (const Future<void>& task : accepted)
        {
            not_completed += task.IsReady() ? 0 : 1;
        }
    }
    REQUIRE(not_completed == 0);
}

TEST_CASE("Thread pool reports a task it could not allocate", "[Thread]")
{
    FailAfterSwitchAllocator allocator;
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(2, 128, &allocator);
    ThreadPool& pool = *pool_owner;
    allocator.m_fail.store(true);

    auto task = pool.Submit([] {});
    REQUIRE_FALSE(task.HasValue());
    REQUIRE(task.GetError() == ErrorCode::OutOfMemory);

    allocator.m_fail.store(false);
}

TEST_CASE("Thread pool Create rejects an allocator that is not thread safe", "[Thread]")
{
    NotThreadSafeAllocator allocator;
    auto pool = ThreadPool::Create(2, 128, &allocator);
    REQUIRE(!pool.HasValue());
    REQUIRE(pool.GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Thread pool Create reports each allocation it could not make", "[Thread]")
{
    bool created = false;
    for (i32 succeed_count = 0; succeed_count < 32 && !created; ++succeed_count)
    {
        FailFromNthAllocator allocator(succeed_count);
        auto pool = ThreadPool::Create(2, 16, &allocator);
        if (pool.HasValue())
        {
            created = true;
            auto task = pool.GetValue()->Submit([] { return 1; });
            const bool submitted = task.HasValue();
            REQUIRE((!submitted || task.GetValue().Get().GetValue() == 1));
        }
        else
        {
            REQUIRE(pool.GetError() == ErrorCode::OutOfMemory);
        }
    }
    REQUIRE(created);
}

TEST_CASE("MPMC channel leaves both ends invalid when out of memory", "[Thread]")
{
    ThreadSafeNullAllocator allocator;
    ChannelMPMC<i32> channel(8, &allocator);
    REQUIRE(!channel.transmitter.IsValid());
    REQUIRE(!channel.receiver.IsValid());

    FailFromNthAllocator queue_but_not_slots(2);
    ChannelMPMC<i32> half(8, &queue_but_not_slots);
    REQUIRE(!half.transmitter.IsValid());
}

TEST_CASE("Thread pool WaitForAll drains every task", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(4);
    ThreadPool& pool = *pool_owner;
    std::atomic<i32> counter{0};

    for (i32 i = 0; i < 64; ++i)
    {
        auto task = pool.Submit([&counter] { counter.fetch_add(1); });
        REQUIRE(task.HasValue());
    }

    pool.WaitForAll();
    REQUIRE(counter.load() == 64);
}

TEST_CASE("Thread pool WaitForAll drains tasks submitted by other tasks", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(4);
    ThreadPool& pool = *pool_owner;
    std::atomic<i32> counter{0};

    auto parent = pool.Submit(
        [&pool, &counter]
        {
            counter.fetch_add(1);
            for (i32 i = 0; i < 8; ++i)
            {
                REQUIRE(pool.Submit([&counter] { counter.fetch_add(1); }).HasValue());
            }
        });
    REQUIRE(parent.HasValue());

    pool.WaitForAll();
    REQUIRE(counter.load() == 9);
}

TEST_CASE("Thread pool worker submits more tasks than the channel holds", "[Thread]")
{
    // One worker and a channel of four: the worker fills the channel itself, and no other worker is left to empty it.
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(1, 4);
    ThreadPool& pool = *pool_owner;
    std::atomic<i32> counter{0};
    std::atomic<i32> rejected{0};

    auto parent = pool.Submit(
        [&pool, &counter, &rejected]
        {
            for (i32 i = 0; i < 64; ++i)
            {
                if (!pool.Submit([&counter] { counter.fetch_add(1); }).HasValue())
                {
                    rejected.fetch_add(1);
                }
            }
        });
    REQUIRE(parent.HasValue());
    if (!parent.GetValue().WaitFor(30000))
    {
        // The pool cannot be closed while its worker is stuck, so end the run rather than hang it.
        fprintf(stderr, "Thread pool deadlocked on a worker submitting into a full channel\n");
        std::abort();
    }

    pool.WaitForAll();
    REQUIRE(rejected.load() == 0);
    REQUIRE(counter.load() == 64);
}

TEST_CASE("Thread pool workers all submitting into a full channel", "[Thread]")
{
    constexpr i32 k_parent_count = 8;
    constexpr i32 k_children_per_parent = 200;
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(4, 4);
    ThreadPool& pool = *pool_owner;
    std::atomic<i32> counter{0};

    DynamicArray<Future<void>> parents;
    for (i32 p = 0; p < k_parent_count; ++p)
    {
        auto parent = pool.Submit(
            [&pool, &counter]
            {
                for (i32 i = 0; i < k_children_per_parent; ++i)
                {
                    while (!pool.Submit([&counter] { counter.fetch_add(1); }).HasValue())
                    {
                    }
                }
            });
        REQUIRE(parent.HasValue());
        REQUIRE(parents.PushBack(Move(parent.GetValue())) == ErrorCode::Success);
    }
    for (Future<void>& parent : parents)
    {
        if (!parent.WaitFor(30000))
        {
            fprintf(stderr, "Thread pool deadlocked on workers submitting into a full channel\n");
            std::abort();
        }
    }

    pool.WaitForAll();
    REQUIRE(counter.load() == k_parent_count * k_children_per_parent);
}

TEST_CASE("Thread pool Close runs tasks submitted by tasks still running", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(2);
    ThreadPool& pool = *pool_owner;
    std::atomic<bool> parent_started{false};
    std::atomic<i32> counter{0};
    std::atomic<i32> rejected{0};

    auto parent = pool.Submit(
        [&pool, &parent_started, &counter, &rejected]
        {
            parent_started.store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            for (i32 i = 0; i < 8; ++i)
            {
                if (!pool.Submit([&counter] { counter.fetch_add(1); }).HasValue())
                {
                    rejected.fetch_add(1);
                }
            }
        });
    REQUIRE(parent.HasValue());

    while (!parent_started.load())
    {
    }
    pool.Close();
    REQUIRE(rejected.load() == 0);
    REQUIRE(counter.load() == 8);
}

TEST_CASE("Thread pool rejects a task from another thread while closing but not from its own workers", "[Thread]")
{
    ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(1);
    ThreadPool& pool = *pool_owner;
    std::atomic<bool> parent_started{false};
    std::atomic<bool> release_parent{false};

    auto parent = pool.Submit(
        [&parent_started, &release_parent]
        {
            parent_started.store(true);
            while (!release_parent.load())
            {
                YieldCurrentThread();
            }
        });
    REQUIRE(parent.HasValue());
    while (!parent_started.load())
    {
    }

    std::atomic<bool> closed{false};
    const ThreadHandle closer = CreateThreadOrFail(
        [](Ref<ThreadPool> in_pool, Ref<std::atomic<bool>> in_closed)
        {
            in_pool->Close();
            in_closed->store(true);
        },
        Ref(pool), Ref(closed));
    SleepCurrentThread(20);
    REQUIRE(!closed.load());

    auto outsider = pool.Submit([] {});
    REQUIRE_FALSE(outsider.HasValue());
    REQUIRE(outsider.GetError() == ErrorCode::ChannelClosed);

    release_parent.store(true);
    JoinThread(closer);
    REQUIRE(parent.GetValue().Get() == ErrorCode::Success);
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

TEST_CASE("Mutex and condition variable construct without allocating", "[Thread]")
{
    ThreadSafeNullAllocator null_allocator;
    PushDefault guard(&null_allocator);

    Mutex<bool> mutex(false);
    ConditionVariable condition_variable;
    MutexGuard<bool> locked = mutex.Lock();
    *locked = true;
    REQUIRE(!condition_variable.WaitFor(locked, 1));
    REQUIRE(*locked);
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

TEST_CASE("ReadWriteMutex reads what was written", "[Thread]")
{
    ReadWriteMutex<DynamicArray<i32>> mutex;
    {
        WriteGuard<DynamicArray<i32>> writer = mutex.LockWrite();
        writer->PushBack(1);
        writer->PushBack(2);
    }
    ReadGuard<DynamicArray<i32>> reader = mutex.LockRead();
    REQUIRE(reader->GetSize() == 2);
    REQUIRE((*reader)[1] == 2);
}

TEST_CASE("ReadWriteMutex lets readers in together and keeps a writer out", "[Thread]")
{
    ReadWriteMutex<i32> mutex(5);

    // Each attempt runs on a thread that holds nothing, since locking twice from one thread is not supported.
    const auto try_from_other_thread = [&mutex](bool write)
    {
        std::atomic<bool> acquired{false};
        const ThreadHandle t = CreateThreadOrFail(
            [](Ref<ReadWriteMutex<i32>> in_mutex, bool in_write, Ref<std::atomic<bool>> in_acquired)
            {
                if (in_write)
                {
                    in_acquired->store(in_mutex->TryLockWrite().HasValue());
                }
                else
                {
                    in_acquired->store(in_mutex->TryLockRead().HasValue());
                }
            },
            Ref(mutex), write, Ref(acquired));
        JoinThread(t);
        return acquired.load();
    };

    {
        ReadGuard<i32> reader = mutex.LockRead();
        REQUIRE(*reader == 5);
        REQUIRE(try_from_other_thread(false));
        REQUIRE(!try_from_other_thread(true));
    }
    {
        WriteGuard<i32> writer = mutex.LockWrite();
        *writer = 6;
        REQUIRE(!try_from_other_thread(false));
        REQUIRE(!try_from_other_thread(true));
    }
    REQUIRE(try_from_other_thread(true));
    REQUIRE(*mutex.LockRead() == 6);
}

TEST_CASE("ReadWriteMutex readers never see a write half done", "[Thread]")
{
    struct Pair
    {
        i64 first = 0;
        i64 second = 0;
    };
    constexpr i32 k_writer_count = 4;
    constexpr i32 k_reader_count = 4;
    constexpr i64 k_writes_per_writer = 5000;

    ReadWriteMutex<Pair> mutex;
    std::atomic<bool> writers_done{false};
    std::atomic<i32> torn_reads{0};
    DynamicArray<ThreadHandle> writers;
    DynamicArray<ThreadHandle> readers;
    for (i32 i = 0; i < k_reader_count; ++i)
    {
        readers.PushBack(CreateThreadOrFail(
            [](Ref<ReadWriteMutex<Pair>> in_mutex, Ref<std::atomic<bool>> in_done, Ref<std::atomic<i32>> in_torn)
            {
                while (!in_done->load())
                {
                    ReadGuard<Pair> reader = in_mutex->LockRead();
                    if (reader->first != reader->second)
                    {
                        in_torn->fetch_add(1);
                    }
                }
            },
            Ref(mutex), Ref(writers_done), Ref(torn_reads)));
    }
    for (i32 i = 0; i < k_writer_count; ++i)
    {
        writers.PushBack(CreateThreadOrFail(
            [](Ref<ReadWriteMutex<Pair>> in_mutex)
            {
                for (i64 n = 0; n < k_writes_per_writer; ++n)
                {
                    WriteGuard<Pair> writer = in_mutex->LockWrite();
                    writer->first++;
                    writer->second++;
                }
            },
            Ref(mutex)));
    }
    for (const ThreadHandle& t : writers)
    {
        JoinThread(t);
    }
    writers_done.store(true);
    for (const ThreadHandle& t : readers)
    {
        JoinThread(t);
    }
    REQUIRE(torn_reads.load() == 0);
    REQUIRE(mutex.LockRead()->first == k_writer_count * k_writes_per_writer);
}

TEST_CASE("ReadWriteMutex guard move assignment releases the lock it held", "[Thread]")
{
    ReadWriteMutex<i32> first(1);
    ReadWriteMutex<i32> second(2);

    {
        WriteGuard<i32> writer = first.LockWrite();
        writer = second.LockWrite();
        REQUIRE(first.TryLockWrite().HasValue());
    }
    {
        ReadGuard<i32> reader = first.LockRead();
        reader = second.LockRead();
        REQUIRE(first.TryLockWrite().HasValue());
        REQUIRE(*reader == 2);
    }
}

TEST_CASE("ReadWriteMutex is neither copyable nor movable", "[Thread]")
{
    STATIC_REQUIRE(!std::is_copy_constructible_v<ReadWriteMutex<i32>>);
    STATIC_REQUIRE(!std::is_move_constructible_v<ReadWriteMutex<i32>>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<ReadWriteMutex<i32>>);
    STATIC_REQUIRE(!std::is_move_assignable_v<ReadWriteMutex<i32>>);
    STATIC_REQUIRE(!std::is_copy_constructible_v<ReadGuard<i32>>);
    STATIC_REQUIRE(!std::is_copy_constructible_v<WriteGuard<i32>>);
}

TEST_CASE("SleepCurrentThread sleeps at least as long as asked", "[Thread]")
{
    const f64 start_ms = GetMilliSeconds();
    SleepCurrentThread(20);
    REQUIRE(GetMilliSeconds() - start_ms >= 19.0);

    SleepCurrentThread(0);
    YieldCurrentThread();
}

namespace
{
// Names a new thread and reads the name back through the OS.
StringUtf8 NameThreadAndReadBack(const char8* name)
{
    StringUtf8 read_back;
    ErrorCode set_result = ErrorCode::OSFailure;
    const ThreadHandle t = CreateThreadOrFail(
        [](const char8* in_name, Ref<StringUtf8> in_read_back, Ref<ErrorCode> in_set_result)
        {
            *in_set_result = SetThreadName(GetCurrentThreadHandle(), StringViewUtf8(in_name));
#if defined(OPAL_PLATFORM_WINDOWS)
            PWSTR description = nullptr;
            if (SUCCEEDED(GetThreadDescription(GetCurrentThread(), &description)))
            {
                StringWide wide(description);
                REQUIRE(Transcode(wide, *in_read_back) == ErrorCode::Success);
                LocalFree(description);
            }
#else
            char buffer[16] = {};
            if (pthread_getname_np(pthread_self(), buffer, sizeof(buffer)) == 0)
            {
                *in_read_back = buffer;
            }
#endif
        },
        name, Ref(read_back), Ref(set_result));
    JoinThread(t);
    REQUIRE(set_result == ErrorCode::Success);
    return read_back;
}
}  // namespace

TEST_CASE("SetThreadName names a thread", "[Thread]")
{
    REQUIRE(NameThreadAndReadBack("opal-worker") == "opal-worker");

    // Eight ASCII bytes and four two-byte code points: sixteen bytes, one over Linux's limit.
    const char8* long_name = "worker1-\xC5\xBD\xC5\xBD\xC5\xBD\xC5\xBD";
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(NameThreadAndReadBack(long_name) == long_name);
#else
    REQUIRE(NameThreadAndReadBack(long_name) == "worker1-\xC5\xBD\xC5\xBD\xC5\xBD");
#endif
}

TEST_CASE("SetThreadName rejects an empty handle", "[Thread]")
{
    REQUIRE(SetThreadName(ThreadHandle{}, "name") == ErrorCode::OSFailure);
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

namespace
{

// Runs ParallelFor over [begin, end) and checks every index was visited exactly once.
void CheckParallelForVisitsEachIndexOnce(ThreadPool& pool, u64 begin, u64 end, u64 grain_size)
{
    const u64 count = end > begin ? end - begin : 0;
    DynamicArray<std::atomic<i32>> visits(count);
    ParallelFor(pool, begin, end, grain_size, [&visits, begin](u64 index) { visits[index - begin].fetch_add(1); });
    for (u64 i = 0; i < count; ++i)
    {
        REQUIRE(visits[i].load() == 1);
    }
}

}  // namespace

TEST_CASE("ParallelFor visits each index once", "[Thread]")
{
    ScopePtr<ThreadPool> pool = CreatePoolOrFail(4);

    SECTION("One index at a time")
    {
        CheckParallelForVisitsEachIndexOnce(*pool, 0, 1000, 1);
    }
    SECTION("A grain that does not divide the range")
    {
        CheckParallelForVisitsEachIndexOnce(*pool, 3, 1000, 7);
    }
    SECTION("A grain larger than the range")
    {
        CheckParallelForVisitsEachIndexOnce(*pool, 0, 10, 100);
    }
    SECTION("A grain of zero")
    {
        CheckParallelForVisitsEachIndexOnce(*pool, 0, 50, 0);
    }
    SECTION("An empty or reversed range calls nothing")
    {
        std::atomic<i32> calls{0};
        ParallelFor(*pool, 5, 5, 1, [&calls](u64) { calls.fetch_add(1); });
        ParallelFor(*pool, 9, 2, 1, [&calls](u64) { calls.fetch_add(1); });
        REQUIRE(calls.load() == 0);
    }
    SECTION("A range ending at the largest index")
    {
        constexpr u64 k_max = ~u64{};
        std::atomic<i32> calls{0};
        ParallelFor(*pool, k_max - 10, k_max, 3, [&calls](u64) { calls.fetch_add(1); });
        REQUIRE(calls.load() == 10);
    }
}

TEST_CASE("ParallelFor spreads work over the workers", "[Thread]")
{
    ScopePtr<ThreadPool> pool = CreatePoolOrFail(4);
    std::atomic<i32> running{0};
    std::atomic<i32> most_running{0};
    ParallelFor(*pool, 0, 64, 1,
                [&running, &most_running](u64)
                {
                    const i32 now = running.fetch_add(1) + 1;
                    i32 most = most_running.load();
                    while (now > most && !most_running.compare_exchange_weak(most, now))
                    {
                    }
                    SleepCurrentThread(1);
                    running.fetch_sub(1);
                });
    REQUIRE(most_running.load() > 1);
}

TEST_CASE("ParallelFor finishes without help from the pool", "[Thread]")
{
    SECTION("From the only worker of its own pool")
    {
        ScopePtr<ThreadPool> pool_owner = CreatePoolOrFail(1, 4);
        ThreadPool& pool = *pool_owner;
        std::atomic<i32> calls{0};
        auto outer = pool.Submit([&pool, &calls] { ParallelFor(pool, 0, 100, 1, [&calls](u64) { calls.fetch_add(1); }); });
        REQUIRE(outer.HasValue());
        if (!outer.GetValue().WaitFor(30000))
        {
            fprintf(stderr, "ParallelFor deadlocked on the only worker of its pool\n");
            std::abort();
        }
        REQUIRE(calls.load() == 100);
    }
    SECTION("Nested in itself")
    {
        ScopePtr<ThreadPool> pool = CreatePoolOrFail(2);
        std::atomic<i32> calls{0};
        ParallelFor(*pool, 0, 8, 1,
                    [&pool, &calls](u64) { ParallelFor(*pool, 0, 8, 1, [&calls](u64) { calls.fetch_add(1); }); });
        REQUIRE(calls.load() == 64);
    }
    SECTION("On a closed pool")
    {
        ScopePtr<ThreadPool> pool = CreatePoolOrFail(2);
        pool->Close();
        CheckParallelForVisitsEachIndexOnce(*pool, 0, 100, 4);
    }
}

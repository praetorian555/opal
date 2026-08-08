#include "test-helpers.h"

#include <chrono>
#include <thread>

#include "opal/container/ref.h"
#include "opal/threading/atomic.h"
#include "opal/threading/thread.h"

using namespace Opal;

namespace
{

template <typename Function, typename... Args>
ThreadHandle CreateThreadOrFail(Function&& function, Args&&... args)
{
    Expected<ThreadHandle, ErrorCode> handle = CreateThread(Forward<Function>(function), Forward<Args>(args)...);
    REQUIRE(handle.HasValue());
    return handle.GetValue();
}

}  // namespace

TEST_CASE("Atomic load and store", "[Atomic]")
{
    SECTION("Constructed from a value")
    {
        Atomic<u32> value{7};
        REQUIRE(value.Load<MemoryOrder::Relaxed>() == 7);
    }
    SECTION("Store replaces the value")
    {
        Atomic<u32> value{0};
        value.Store<MemoryOrder::Release>(42);
        REQUIRE(value.Load<MemoryOrder::Acquire>() == 42);
    }
    SECTION("Sequentially consistent by default")
    {
        Atomic<u64> value{1};
        value.Store(2);
        REQUIRE(value.Load() == 2);
    }
    SECTION("Holds a bool")
    {
        Atomic<bool> flag = false;
        REQUIRE_FALSE(flag.Load<MemoryOrder::Acquire>());
        flag.Store<MemoryOrder::Release>(true);
        REQUIRE(flag.Load<MemoryOrder::Acquire>());
    }
}

TEST_CASE("Atomic exchange", "[Atomic]")
{
    Atomic<u32> value{5};
    REQUIRE(value.Exchange<MemoryOrder::AcqRel>(9) == 5);
    REQUIRE(value.Load<MemoryOrder::Acquire>() == 9);
}

TEST_CASE("Atomic compare exchange", "[Atomic]")
{
    SECTION("Strong succeeds when the value matches")
    {
        Atomic<u32> value{3};
        u32 expected = 3;
        REQUIRE(value.CompareExchangeStrong(expected, 4));
        REQUIRE(value.Load() == 4);
        REQUIRE(expected == 3);
    }
    SECTION("Strong fails and reports what was there")
    {
        Atomic<u32> value{3};
        u32 expected = 99;
        REQUIRE_FALSE(value.CompareExchangeStrong(expected, 4));
        REQUIRE(value.Load() == 3);
        REQUIRE(expected == 3);
    }
    SECTION("Weak succeeds inside a loop")
    {
        Atomic<u32> value{10};
        u32 expected = value.Load<MemoryOrder::Relaxed>();
        while (!value.CompareExchangeWeak<MemoryOrder::AcqRel, MemoryOrder::Acquire>(expected, expected + 1))
        {
        }
        REQUIRE(value.Load() == 11);
    }
}

TEST_CASE("Atomic fetch add and sub", "[Atomic]")
{
    Atomic<u64> value{10};
    REQUIRE(value.FetchAdd<MemoryOrder::Relaxed>(5) == 10);
    REQUIRE(value.Load<MemoryOrder::Relaxed>() == 15);
    REQUIRE(value.FetchSub<MemoryOrder::AcqRel>(3) == 15);
    REQUIRE(value.Load<MemoryOrder::Relaxed>() == 12);
}

TEST_CASE("Atomic reports whether it is lock free", "[Atomic]")
{
    STATIC_REQUIRE(Atomic<bool>::k_is_always_lock_free);
    STATIC_REQUIRE(Atomic<u32>::k_is_always_lock_free);
    STATIC_REQUIRE(Atomic<u64>::k_is_always_lock_free);
    STATIC_REQUIRE(Atomic<size_t>::k_is_always_lock_free);
}

TEST_CASE("Atomic storage address is the value", "[Atomic]")
{
    Atomic<u32> value{0};
    u32* address = value.GetStorageAddress();

    value.Store<MemoryOrder::Release>(1234);
    REQUIRE(*address == 1234);
}

TEST_CASE("Atomic counts correctly under contention", "[Atomic]")
{
    constexpr u64 k_thread_count = 4;
    constexpr u64 k_increments_per_thread = 10000;

    Atomic<u64> counter{0};
    DynamicArray<ThreadHandle> threads;
    for (u64 i = 0; i < k_thread_count; ++i)
    {
        threads.PushBack(CreateThreadOrFail(
            [](Atomic<u64>& value)
            {
                for (u64 n = 0; n < k_increments_per_thread; ++n)
                {
                    value.FetchAdd<MemoryOrder::Relaxed>(1);
                }
            },
            Ref(counter)));
    }
    for (const ThreadHandle& handle : threads)
    {
        JoinThread(handle);
    }

    REQUIRE(counter.Load<MemoryOrder::Acquire>() == k_thread_count * k_increments_per_thread);
}

TEST_CASE("Atomic wait blocks until the value changes", "[Atomic]")
{
    Atomic<u32> value{0};
    Atomic<bool> thread_woke_up{false};

    const ThreadHandle t = CreateThreadOrFail(
        [](Atomic<u32>& observed, Atomic<bool>& woke_up)
        {
            while (observed.Load<MemoryOrder::Acquire>() == 0)
            {
                observed.Wait<MemoryOrder::Acquire>(0);
            }
            woke_up.Store<MemoryOrder::Release>(true);
        },
        Ref(value), Ref(thread_woke_up));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    REQUIRE_FALSE(thread_woke_up.Load<MemoryOrder::Acquire>());

    value.Store<MemoryOrder::Release>(1);
    value.NotifyAll();

    JoinThread(t);
    REQUIRE(thread_woke_up.Load<MemoryOrder::Acquire>());
}

TEST_CASE("Atomic notify one wakes a waiter", "[Atomic]")
{
    Atomic<u32> value{0};
    Atomic<bool> thread_woke_up{false};

    const ThreadHandle t = CreateThreadOrFail(
        [](Atomic<u32>& observed, Atomic<bool>& woke_up)
        {
            while (observed.Load<MemoryOrder::Acquire>() == 0)
            {
                observed.Wait<MemoryOrder::Acquire>(0);
            }
            woke_up.Store<MemoryOrder::Release>(true);
        },
        Ref(value), Ref(thread_woke_up));

    value.Store<MemoryOrder::Release>(1);
    value.NotifyOne();

    JoinThread(t);
    REQUIRE(thread_woke_up.Load<MemoryOrder::Acquire>());
}

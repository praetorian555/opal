#include "test-helpers.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "opal/container/string.h"
#include "opal/threading/future.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"

using namespace Opal;

namespace
{
template <typename T>
Promise<T> CreatePromise()
{
    Expected<Promise<T>, ErrorCode> promise = Promise<T>::Create();
    REQUIRE(promise.HasValue());
    return std::move(promise.GetValue());
}

template <typename Function, typename... Args>
ThreadHandle CreateThreadOrFail(Function&& function, Args&&... args)
{
    Expected<ThreadHandle, ErrorCode> handle = CreateThread(std::forward<Function>(function), std::forward<Args>(args)...);
    REQUIRE(handle.HasValue());
    return handle.GetValue();
}

struct NotThreadSafeAllocator final : AllocatorBase
{
    NotThreadSafeAllocator() : AllocatorBase("NotThreadSafeAllocator") {}

    void* Alloc(u64 size, u64 alignment) override { return m_inner.Alloc(size, alignment); }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return false; }

    MallocAllocator m_inner;
};
}  // namespace

TEST_CASE("Future receives the value its promise sets", "[Future]")
{
    Promise<i32> promise = CreatePromise<i32>();
    Future<i32> future = promise.GetFuture();
    REQUIRE(future.IsValid());
    REQUIRE(!future.IsReady());

    auto early = future.TryGet();
    REQUIRE(!early.HasValue());
    REQUIRE(early.GetError() == ErrorCode::WouldBlock);
    REQUIRE(future.IsValid());

    REQUIRE(promise.SetValue(5));
    REQUIRE(!promise.SetValue(6));
    REQUIRE(!promise.SetError(ErrorCode::TimedOut));
    REQUIRE(future.IsReady());

    auto result = future.Get();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == 5);
    REQUIRE(!future.IsValid());
}

TEST_CASE("Future receives the error its promise sets", "[Future]")
{
    Promise<StringUtf8> promise = CreatePromise<StringUtf8>();
    Future<StringUtf8> future = promise.GetFuture();
    REQUIRE(promise.SetError(ErrorCode::TimedOut));

    auto result = future.TryGet();
    REQUIRE(!result.HasValue());
    REQUIRE(result.GetError() == ErrorCode::TimedOut);
}

TEST_CASE("Future takes an Expected as the result", "[Future]")
{
    Promise<i32> promise = CreatePromise<i32>();
    Future<i32> future = promise.GetFuture();
    REQUIRE(promise.SetResult(Expected<i32, ErrorCode>(7)));
    REQUIRE(future.Get().GetValue() == 7);
}

TEST_CASE("Future reports a broken promise", "[Future]")
{
    Future<StringUtf8> future;
    {
        Promise<StringUtf8> promise = CreatePromise<StringUtf8>();
        future = promise.GetFuture();
    }
    auto result = future.Get();
    REQUIRE(!result.HasValue());
    REQUIRE(result.GetError() == ErrorCode::BrokenPromise);

    Future<i32> replaced;
    Promise<i32> first = CreatePromise<i32>();
    replaced = first.GetFuture();
    first = CreatePromise<i32>();
    REQUIRE(replaced.Get().GetError() == ErrorCode::BrokenPromise);
}

TEST_CASE("Future of void", "[Future]")
{
    Promise<void> promise = CreatePromise<void>();
    Future<void> future = promise.GetFuture();
    REQUIRE(future.TryGet() == ErrorCode::WouldBlock);
    REQUIRE(promise.SetValue());
    REQUIRE(future.Get() == ErrorCode::Success);

    Promise<void> failing = CreatePromise<void>();
    Future<void> failed = failing.GetFuture();
    REQUIRE(failing.SetError(ErrorCode::OSFailure));
    REQUIRE(failed.Get() == ErrorCode::OSFailure);

    Future<void> broken;
    {
        Promise<void> dropped = CreatePromise<void>();
        broken = dropped.GetFuture();
    }
    REQUIRE(broken.Get() == ErrorCode::BrokenPromise);
}

TEST_CASE("Invalid future and promise", "[Future]")
{
    Future<i32> future;
    REQUIRE(!future.IsValid());
    REQUIRE(!future.IsReady());
    REQUIRE(!future.WaitFor(0));
    future.Wait();
    REQUIRE(future.Get().GetError() == ErrorCode::InvalidArgument);
    REQUIRE(!future.Then([](Expected<i32, ErrorCode>&& value) { return value.GetValueOr(0); }).IsValid());

    Promise<i32> promise;
    REQUIRE(!promise.IsValid());
    REQUIRE(!promise.SetValue(1));
    REQUIRE(!promise.GetFuture().IsValid());
}

TEST_CASE("Promise needs a thread-safe allocator", "[Future]")
{
    NotThreadSafeAllocator allocator;
    auto promise = Promise<i32>::Create(&allocator);
    REQUIRE(!promise.HasValue());
    REQUIRE(promise.GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Future Get blocks until another thread sets the value", "[Future]")
{
    Promise<i32> promise = CreatePromise<i32>();
    Future<i32> future = promise.GetFuture();
    const ThreadHandle t = CreateThreadOrFail(
        [](Promise<i32> in_promise)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            in_promise.SetValue(42);
        },
        std::move(promise));
    REQUIRE(future.Get().GetValue() == 42);
    JoinThread(t);
}

TEST_CASE("Future WaitFor times out, then sees the value", "[Future]")
{
    Promise<i32> promise = CreatePromise<i32>();
    Future<i32> future = promise.GetFuture();
    REQUIRE(!future.WaitFor(10));
    promise.SetValue(1);
    REQUIRE(future.WaitFor(10));
    REQUIRE(future.Get().GetValue() == 1);
}

TEST_CASE("Future Then", "[Future]")
{
    SECTION("Attached before the value runs when it is set")
    {
        Promise<i32> promise = CreatePromise<i32>();
        Future<i32> doubled = promise.GetFuture().Then([](Expected<i32, ErrorCode>&& value) { return value.GetValue() * 2; });
        REQUIRE(doubled.IsValid());
        REQUIRE(!doubled.IsReady());
        promise.SetValue(21);
        REQUIRE(doubled.Get().GetValue() == 42);
    }
    SECTION("Attached after the value runs at once")
    {
        Promise<i32> promise = CreatePromise<i32>();
        Future<i32> future = promise.GetFuture();
        promise.SetValue(3);
        Future<i32> next = future.Then([](Expected<i32, ErrorCode>&& value) { return value.GetValue() + 1; });
        REQUIRE(!future.IsValid());
        REQUIRE(next.IsReady());
        REQUIRE(next.Get().GetValue() == 4);
    }
    SECTION("Chains, passes errors through and flattens an Expected")
    {
        Promise<i32> promise = CreatePromise<i32>();
        Future<StringUtf8> text = promise.GetFuture()
                                      .Then(
                                          [](Expected<i32, ErrorCode>&& value) -> Expected<i32, ErrorCode>
                                          {
                                              if (!value.HasValue())
                                              {
                                                  return Expected<i32, ErrorCode>(value.GetError());
                                              }
                                              return Expected<i32, ErrorCode>(value.GetValue() * 10);
                                          })
                                      .Then(
                                          [](Expected<i32, ErrorCode>&& value) -> Expected<StringUtf8, ErrorCode>
                                          {
                                              if (!value.HasValue())
                                              {
                                                  return Expected<StringUtf8, ErrorCode>(value.GetError());
                                              }
                                              return Expected<StringUtf8, ErrorCode>(value.GetValue() == 70 ? "seventy" : "other");
                                          });
        promise.SetValue(7);
        REQUIRE(text.Get().GetValue() == "seventy");

        Promise<i32> failing = CreatePromise<i32>();
        Future<i32> failed = failing.GetFuture().Then(
            [](Expected<i32, ErrorCode>&& value) -> Expected<i32, ErrorCode>
            {
                if (!value.HasValue())
                {
                    return Expected<i32, ErrorCode>(value.GetError());
                }
                return Expected<i32, ErrorCode>(value.GetValue());
            });
        failing.SetError(ErrorCode::TimedOut);
        REQUIRE(failed.Get().GetError() == ErrorCode::TimedOut);
    }
    SECTION("Void on either side")
    {
        Promise<void> promise = CreatePromise<void>();
        i32 seen = 0;
        Future<void> done = promise.GetFuture().Then([&seen](ErrorCode code) { seen = code == ErrorCode::Success ? 1 : 2; });
        promise.SetValue();
        REQUIRE(done.Get() == ErrorCode::Success);
        REQUIRE(seen == 1);
    }
    SECTION("Sees a broken promise")
    {
        Future<bool> broken;
        {
            Promise<i32> promise = CreatePromise<i32>();
            broken = promise.GetFuture().Then([](Expected<i32, ErrorCode>&& value)
                                              { return !value.HasValue() && value.GetError() == ErrorCode::BrokenPromise; });
        }
        REQUIRE(broken.Get().GetValue());
    }
}

TEST_CASE("Future Then races SetValue without losing or doubling the continuation", "[Future]")
{
    constexpr i32 k_rounds = 2000;
    i32 wrong = 0;
    for (i32 round = 0; round < k_rounds; ++round)
    {
        Promise<i32> promise = CreatePromise<i32>();
        Future<i32> future = promise.GetFuture();
        std::atomic<i32> runs{0};
        const ThreadHandle t = CreateThreadOrFail([](Promise<i32> in_promise, i32 value) { in_promise.SetValue(value); },
                                                  std::move(promise), round);
        Future<i32> next = future.Then(
            [&runs](Expected<i32, ErrorCode>&& value)
            {
                runs.fetch_add(1);
                return value.GetValue();
            });
        const i32 result = next.Get().GetValue();
        JoinThread(t);
        wrong += (result != round || runs.load() != 1) ? 1 : 0;
    }
    REQUIRE(wrong == 0);
}

TEST_CASE("ThreadPool Submit returns a future", "[Future]")
{
    ThreadPool pool(4);

    auto value = pool.Submit([] { return 6 * 7; });
    REQUIRE(value.HasValue());
    REQUIRE(value.GetValue().Get().GetValue() == 42);

    std::atomic<i32> ran{0};
    auto nothing = pool.Submit([&ran] { ran.store(1); });
    REQUIRE(nothing.HasValue());
    REQUIRE(nothing.GetValue().Get() == ErrorCode::Success);
    REQUIRE(ran.load() == 1);

    auto failing = pool.Submit([]() -> Expected<StringUtf8, ErrorCode> { return Expected<StringUtf8, ErrorCode>(ErrorCode::PathNotFound); });
    REQUIRE(failing.HasValue());
    REQUIRE(failing.GetValue().Get().GetError() == ErrorCode::PathNotFound);

    auto chained = pool.Submit([] { return StringUtf8("asset"); });
    REQUIRE(chained.HasValue());
    Future<u64> size = chained.GetValue().Then([](Expected<StringUtf8, ErrorCode>&& text) { return text.GetValue().GetSize(); });
    REQUIRE(size.Get().GetValue() == 5);

    constexpr i32 k_count = 200;
    DynamicArray<Future<i32>> futures;
    for (i32 i = 0; i < k_count; ++i)
    {
        auto submitted = pool.Submit([i] { return i * i; });
        REQUIRE(submitted.HasValue());
        futures.PushBack(std::move(submitted.GetValue()));
    }
    i64 sum = 0;
    for (Future<i32>& future : futures)
    {
        sum += future.Get().GetValue();
    }
    REQUIRE(sum == 2646700);
}

TEST_CASE("ThreadPool Submit on a closed pool", "[Future]")
{
    ThreadPool pool(1);
    pool.Close();
    auto future = pool.Submit([] { return 1; });
    REQUIRE(!future.HasValue());
    REQUIRE(future.GetError() == ErrorCode::ChannelClosed);
}

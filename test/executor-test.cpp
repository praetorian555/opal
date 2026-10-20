#include "test-helpers.h"

#include <atomic>

#include "opal/container/string.h"
#include "opal/threading/executor.h"
#include "opal/threading/task-queue.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"

using namespace Opal;

namespace
{
template <typename Function, typename... Args>
ThreadHandle CreateThreadOrFail(Function&& function, Args&&... args)
{
    Expected<ThreadHandle, ErrorCode> handle = CreateThread(std::forward<Function>(function), std::forward<Args>(args)...);
    REQUIRE(handle.HasValue());
    return handle.GetValue();
}

// Runs work at once on the posting thread, to show the interface is enough to build an executor on.
struct InlineExecutor final : Executor
{
    ErrorCode Post(SharedPtr<Runnable>&& work) override
    {
        if (!work.IsValid())
        {
            return ErrorCode::InvalidArgument;
        }
        work->Run();
        return ErrorCode::Success;
    }

    [[nodiscard]] AllocatorBase* GetAllocator() const override { return GetDefaultAllocator(); }
};

struct NotThreadSafeAllocator final : AllocatorBase
{
    NotThreadSafeAllocator() : AllocatorBase("NotThreadSafeAllocator") {}

    void* Alloc(u64 size, u64 alignment) override { return m_inner.Alloc(size, alignment); }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return false; }

    MallocAllocator m_inner;
};
}  // namespace

TEST_CASE("TaskQueue runs work only when drained", "[Executor]")
{
    TaskQueue queue;
    auto future = queue.Submit([] { return 7; });
    REQUIRE(future.HasValue());
    REQUIRE(queue.GetPendingCount() == 1);
    REQUIRE(!future.GetValue().IsReady());

    REQUIRE(queue.RunPending() == 1);
    REQUIRE(future.GetValue().Get().GetValue() == 7);
    REQUIRE(queue.RunPending() == 0);
}

TEST_CASE("TaskQueue leaves work posted while draining for the next drain", "[Executor]")
{
    TaskQueue queue;
    i32 runs = 0;
    auto first = queue.Submit(
        [&queue, &runs]
        {
            ++runs;
            REQUIRE(queue.Submit([&runs] { ++runs; }).HasValue());
        });
    REQUIRE(first.HasValue());

    REQUIRE(queue.RunPending() == 1);
    REQUIRE(runs == 1);
    REQUIRE(queue.RunPending() == 1);
    REQUIRE(runs == 2);
}

TEST_CASE("TaskQueue collects work posted from many threads", "[Executor]")
{
    constexpr i32 k_thread_count = 4;
    constexpr i32 k_posts_per_thread = 250;
    TaskQueue queue;
    std::atomic<i32> sum{0};
    DynamicArray<ThreadHandle> threads;
    for (i32 t = 0; t < k_thread_count; ++t)
    {
        threads.PushBack(CreateThreadOrFail(
            [](Ref<TaskQueue> in_queue, Ref<std::atomic<i32>> in_sum)
            {
                for (i32 i = 0; i < k_posts_per_thread; ++i)
                {
                    auto posted = in_queue->Submit([in_sum = in_sum.GetPtr()] { in_sum->fetch_add(1); });
                    if (!posted.HasValue())
                    {
                        return;
                    }
                }
            },
            Ref(queue), Ref(sum)));
    }

    size_t ran = 0;
    while (ran < static_cast<size_t>(k_thread_count * k_posts_per_thread))
    {
        queue.WaitFor(100);
        ran += queue.RunPending();
    }
    for (const ThreadHandle& t : threads)
    {
        JoinThread(t);
    }
    REQUIRE(sum.load() == k_thread_count * k_posts_per_thread);
}

TEST_CASE("TaskQueue WaitFor", "[Executor]")
{
    TaskQueue queue;
    REQUIRE(!queue.WaitFor(10));

    std::atomic<bool> submitted{false};
    const ThreadHandle t = CreateThreadOrFail(
        [](Ref<TaskQueue> in_queue, Ref<std::atomic<bool>> in_submitted)
        {
            SleepCurrentThread(20);
            in_submitted->store(in_queue->Submit([] {}).HasValue());
        },
        Ref(queue), Ref(submitted));
    REQUIRE(queue.WaitFor(5000));
    JoinThread(t);
    REQUIRE(submitted.load());
    REQUIRE(queue.RunPending() == 1);
}

TEST_CASE("TaskQueue Close rejects work and keeps what was queued", "[Executor]")
{
    TaskQueue queue;
    auto queued = queue.Submit([] { return 1; });
    REQUIRE(queued.HasValue());
    queue.Close();
    queue.Close();

    auto rejected = queue.Submit([] { return 2; });
    REQUIRE(!rejected.HasValue());
    REQUIRE(rejected.GetError() == ErrorCode::ChannelClosed);

    REQUIRE(queue.RunPending() == 1);
    REQUIRE(queued.GetValue().Get().GetValue() == 1);
}

TEST_CASE("TaskQueue drops work still queued when destroyed", "[Executor]")
{
    Future<i32> dropped;
    {
        TaskQueue queue;
        auto future = queue.Submit([] { return 1; });
        REQUIRE(future.HasValue());
        dropped = std::move(future.GetValue());
    }
    REQUIRE(dropped.Get().GetError() == ErrorCode::BrokenPromise);
}

TEST_CASE("TaskQueue destruction survives a continuation that posts back to it", "[Executor]")
{
    Future<i32> reposted;
    {
        TaskQueue queue;
        auto future = queue.Submit([] { return 1; });
        REQUIRE(future.HasValue());
        TaskQueue* queue_pointer = &queue;
        reposted = future.GetValue().Then(
            [queue_pointer](Expected<i32, ErrorCode>&& value) -> Expected<i32, ErrorCode>
            {
                auto again = queue_pointer->Submit([] { return 2; });
                if (!again.HasValue())
                {
                    return Expected<i32, ErrorCode>(again.GetError());
                }
                return Expected<i32, ErrorCode>(value.GetErrorOr(ErrorCode::Success) == ErrorCode::Success ? 0 : -1);
            });
    }
    REQUIRE(reposted.Get().GetError() == ErrorCode::ChannelClosed);
}

TEST_CASE("TaskQueue rejects an invalid pointer and a non-thread-safe allocator", "[Executor]")
{
    TaskQueue queue;
    REQUIRE(queue.Post(SharedPtr<Runnable>{}) == ErrorCode::InvalidArgument);

    NotThreadSafeAllocator allocator;
    TaskQueue unsafe_queue(&allocator);
    auto rejected = unsafe_queue.Submit([] {});
    REQUIRE(!rejected.HasValue());
    REQUIRE(rejected.GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Then on an executor runs on the thread that drains it", "[Executor]")
{
    ThreadPool pool(2);
    TaskQueue main_queue;
    const ThreadId main_thread = GetCurrentThreadHandle().id;

    auto computed = pool.Submit([] { return GetCurrentThreadHandle().id; });
    REQUIRE(computed.HasValue());
    Future<bool> checked = computed.GetValue().Then(main_queue,
                                                    [main_thread](Expected<ThreadId, ErrorCode>&& worker)
                                                    {
                                                        const ThreadId here = GetCurrentThreadHandle().id;
                                                        return worker.HasValue() && worker.GetValue() != main_thread && here == main_thread;
                                                    });
    REQUIRE(checked.IsValid());

    while (!checked.IsReady())
    {
        main_queue.WaitFor(100);
        main_queue.RunPending();
    }
    REQUIRE(checked.Get().GetValue());
}

TEST_CASE("Then on an executor never runs inline when the result is ready", "[Executor]")
{
    TaskQueue queue;
    auto promise = Promise<i32>::Create();
    REQUIRE(promise.HasValue());
    Future<i32> future = promise.GetValue().GetFuture();
    promise.GetValue().SetValue(4);

    Future<i32> next = future.Then(queue, [](Expected<i32, ErrorCode>&& value) { return value.GetValue() + 1; });
    REQUIRE(!next.IsReady());
    REQUIRE(queue.RunPending() == 1);
    REQUIRE(next.Get().GetValue() == 5);
}

TEST_CASE("Then on a closed executor reports it", "[Executor]")
{
    TaskQueue queue;
    queue.Close();
    auto promise = Promise<void>::Create();
    REQUIRE(promise.HasValue());
    Future<void> next = promise.GetValue().GetFuture().Then(queue, [](ErrorCode) {});
    promise.GetValue().SetValue();
    REQUIRE(next.Get() == ErrorCode::ChannelClosed);
}

TEST_CASE("Then on an executor drops the input with the queue", "[Executor]")
{
    Future<StringUtf8> next;
    {
        TaskQueue queue;
        auto promise = Promise<StringUtf8>::Create();
        REQUIRE(promise.HasValue());
        next = promise.GetValue().GetFuture().Then(queue, [](Expected<StringUtf8, ErrorCode>&& text) { return text.GetValue().Clone(); });
        promise.GetValue().SetValue(StringUtf8("not run"));
    }
    REQUIRE(next.Get().GetError() == ErrorCode::BrokenPromise);
}

TEST_CASE("Executors work through the interface", "[Executor]")
{
    ThreadPool pool(2);
    TaskQueue queue;
    InlineExecutor inline_executor;
    Executor* executors[] = {&pool, &queue, &inline_executor};
    for (Executor* executor : executors)
    {
        auto future = executor->Submit([] { return 3; });
        REQUIRE(future.HasValue());
        queue.RunPending();
        REQUIRE(future.GetValue().Get().GetValue() == 3);
    }
}

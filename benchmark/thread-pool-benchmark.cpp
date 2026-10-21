// Compares Opal::ThreadPool against a pool built on std::thread, a std::deque of std::function guarded by std::mutex, and
// std::condition_variable, the pool a codebase without one would write. Its Submit wraps the callable in a std::packaged_task.
//
// Five operations, each swept over the number of workers:
//   Post        - the benchmark thread posts tasks that return nothing, then waits for the pool to drain. The cost of handing a
//                 task to a worker, with no future involved.
//   Submit      - the same, through Submit, then takes every task's result from its future. Adds the future's shared state.
//   RoundTrip   - submit one task and wait for its result before submitting the next. The latency of waking an idle worker and
//                 hearing back from it.
//   Spawn       - one task submits the rest from a worker. The path a task takes to fan out work of its own.
//   ParallelFor - a loop spread over the workers, with the calling thread taking part, swept over the loop length.
//
// Tasks do almost nothing, so the numbers are the pool's own overhead. A task that takes longer hides it in proportion.
//
// Workers are not pinned: neither pool lets a caller choose the core a worker runs on, so the scheduler places them, and on a CPU
// with more than one L3 the spread between runs is wider than for the pinned channel benchmarks.
//
// See docs/benchmarks.md for how to build, run and compare the results.

#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "opal/threading/thread-pool.h"

namespace
{

using Item = Opal::u64;

// Tasks per iteration of Post, Submit and Spawn.
constexpr size_t k_task_count = size_t{1} << 14;

// Indices ParallelFor hands a thread at a time.
constexpr Opal::u64 k_grain_size = 1024;

/**
 * Fixed set of worker threads taking std::function tasks from one mutex-guarded queue. Tasks may post more tasks. WaitForAll
 * returns once every task posted, including the ones tasks posted, has run.
 */
class StdThreadPool
{
public:
    explicit StdThreadPool(size_t thread_count)
    {
        m_threads.reserve(thread_count);
        for (size_t i = 0; i < thread_count; i++)
        {
            m_threads.emplace_back([this] { WorkerLoop(); });
        }
    }

    ~StdThreadPool()
    {
        {
            std::lock_guard lock(m_mutex);
            m_is_closed = true;
        }
        m_work_available.notify_all();
        for (std::thread& thread : m_threads)
        {
            thread.join();
        }
    }

    StdThreadPool(const StdThreadPool&) = delete;
    StdThreadPool& operator=(const StdThreadPool&) = delete;

    [[nodiscard]] size_t GetThreadCount() const { return m_threads.size(); }

    void Post(std::function<void()> task)
    {
        {
            std::lock_guard lock(m_mutex);
            m_tasks.push_back(std::move(task));
            m_pending++;
        }
        m_work_available.notify_one();
    }

    template <typename Function>
    std::future<std::invoke_result_t<Function&>> Submit(Function function)
    {
        // std::function needs a copyable callable, and a packaged_task is move-only.
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<Function&>()>>(std::move(function));
        auto future = task->get_future();
        Post([task = std::move(task)] { (*task)(); });
        return future;
    }

    void WaitForAll()
    {
        std::unique_lock lock(m_mutex);
        m_all_done.wait(lock, [this] { return m_pending == 0; });
    }

private:
    void WorkerLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock lock(m_mutex);
                m_work_available.wait(lock, [this] { return !m_tasks.empty() || m_is_closed; });
                if (m_tasks.empty())
                {
                    return;
                }
                task = std::move(m_tasks.front());
                m_tasks.pop_front();
            }
            task();
            bool is_drained = false;
            {
                std::lock_guard lock(m_mutex);
                is_drained = --m_pending == 0;
            }
            if (is_drained)
            {
                m_all_done.notify_all();
            }
        }
    }

    std::mutex m_mutex;
    std::condition_variable m_work_available;
    std::condition_variable m_all_done;
    std::deque<std::function<void()>> m_tasks;
    size_t m_pending = 0;
    bool m_is_closed = false;
    std::vector<std::thread> m_threads;
};

using Std = StdThreadPool;

/**
 * What the threads of StdParallelFor share. The same scheme as Opal::ParallelFor, func called directly rather than through a
 * std::function, so the two sides differ only in the pool they post to.
 */
template <typename Func>
class StdParallelForState
{
public:
    StdParallelForState(Opal::u64 begin, Opal::u64 end, Opal::u64 grain_size, Func& func)
        : m_func(&func), m_begin(begin), m_end(end), m_grain_size(grain_size), m_chunk_count((end - begin - 1) / grain_size + 1)
    {
    }

    [[nodiscard]] Opal::u64 GetChunkCount() const { return m_chunk_count; }

    void RunChunks()
    {
        while (true)
        {
            const Opal::u64 chunk = m_next_chunk.fetch_add(1, std::memory_order_relaxed);
            if (chunk >= m_chunk_count)
            {
                return;
            }
            const Opal::u64 chunk_begin = m_begin + chunk * m_grain_size;
            const Opal::u64 chunk_end = std::min(chunk_begin + m_grain_size, m_end);
            for (Opal::u64 index = chunk_begin; index < chunk_end; index++)
            {
                (*m_func)(index);
            }
            if (m_finished_chunks.fetch_add(1, std::memory_order_acq_rel) + 1 == m_chunk_count)
            {
                m_finished_chunks.notify_all();
            }
        }
    }

    void WaitForChunks()
    {
        for (Opal::u64 finished = m_finished_chunks.load(std::memory_order_acquire); finished != m_chunk_count;
             finished = m_finished_chunks.load(std::memory_order_acquire))
        {
            m_finished_chunks.wait(finished, std::memory_order_acquire);
        }
    }

private:
    Func* m_func;
    Opal::u64 m_begin;
    Opal::u64 m_end;
    Opal::u64 m_grain_size;
    Opal::u64 m_chunk_count;
    std::atomic<Opal::u64> m_next_chunk{0};
    std::atomic<Opal::u64> m_finished_chunks{0};
};

/** Opal::ParallelFor written against StdThreadPool, so the two sides split the loop the same way. */
template <typename Func>
void StdParallelFor(StdThreadPool& pool, Opal::u64 begin, Opal::u64 end, Opal::u64 grain_size, Func&& func)
{
    if (begin >= end)
    {
        return;
    }
    auto state = std::make_shared<StdParallelForState<std::remove_reference_t<Func>>>(begin, end, grain_size, func);
    const Opal::u64 helper_count = std::min<Opal::u64>(pool.GetThreadCount(), state->GetChunkCount() - 1);
    for (Opal::u64 i = 0; i < helper_count; i++)
    {
        pool.Post([state] { state->RunChunks(); });
    }
    state->RunChunks();
    state->WaitForChunks();
}

/** A task with nothing to do, for Opal::ThreadPool::Post, which takes a Runnable rather than a callable. */
class EmptyTask final : public Opal::Runnable
{
public:
    void Run() override { benchmark::ClobberMemory(); }
};

/** A task that posts k_task_count - 1 empty tasks to the pool it runs on. */
class SpawnTask final : public Opal::Runnable
{
public:
    explicit SpawnTask(Opal::ThreadPool& pool) : m_pool(&pool) {}

    void Run() override
    {
        for (size_t i = 1; i < k_task_count; i++)
        {
            m_pool->Post(Opal::SharedPtr<Opal::Runnable>(Opal::SharedPtr<EmptyTask>(m_pool->GetAllocator())));
        }
    }

private:
    Opal::ThreadPool* m_pool;
};

/**
 * Uniform interface over the two pools so every benchmark body is written once. A failure to post or submit is reported as false or
 * as an empty result, and the benchmark stops with an error.
 */
template <typename Pool>
struct PoolOps;

template <>
struct PoolOps<Opal::ThreadPool>
{
    using Handle = Opal::ScopePtr<Opal::ThreadPool>;
    using Future = Opal::Future<Item>;

    static Handle Make(size_t thread_count)
    {
        auto created = Opal::ThreadPool::Create(thread_count);
        return created.HasValue() ? std::move(created).GetValue() : Handle();
    }

    static bool IsValid(const Handle& pool) { return pool.IsValid(); }

    static bool PostEmpty(Opal::ThreadPool& pool)
    {
        Opal::SharedPtr<EmptyTask> task(pool.GetAllocator());
        return task.IsValid() && pool.Post(Opal::SharedPtr<Opal::Runnable>(std::move(task))) == Opal::ErrorCode::Success;
    }

    static bool PostSpawn(Opal::ThreadPool& pool)
    {
        Opal::SharedPtr<SpawnTask> task(pool.GetAllocator(), pool);
        return task.IsValid() && pool.Post(Opal::SharedPtr<Opal::Runnable>(std::move(task))) == Opal::ErrorCode::Success;
    }

    static Future Submit(Opal::ThreadPool& pool, Item value)
    {
        auto submitted = pool.Submit([value] { return value; });
        return submitted.HasValue() ? std::move(submitted).GetValue() : Future();
    }

    static bool IsValid(const Future& future) { return future.IsValid(); }

    static Item Get(Future& future)
    {
        auto result = future.Get();
        return result.HasValue() ? result.GetValue() : ~Item{0};
    }

    static void WaitForAll(Opal::ThreadPool& pool) { pool.WaitForAll(); }

    template <typename Func>
    static void ParallelFor(Opal::ThreadPool& pool, Opal::u64 end, Func&& func)
    {
        Opal::ParallelFor(pool, 0, end, k_grain_size, std::forward<Func>(func));
    }
};

template <>
struct PoolOps<Std>
{
    using Handle = std::unique_ptr<Std>;
    using Future = std::future<Item>;

    static Handle Make(size_t thread_count) { return std::make_unique<Std>(thread_count); }
    static bool IsValid(const Handle& pool) { return pool != nullptr; }

    static bool PostEmpty(Std& pool)
    {
        pool.Post([] { benchmark::ClobberMemory(); });
        return true;
    }

    static bool PostSpawn(Std& pool)
    {
        pool.Post(
            [&pool]
            {
                for (size_t i = 1; i < k_task_count; i++)
                {
                    pool.Post([] { benchmark::ClobberMemory(); });
                }
            });
        return true;
    }

    static Future Submit(Std& pool, Item value)
    {
        return pool.Submit([value] { return value; });
    }
    static bool IsValid(const Future& future) { return future.valid(); }
    static Item Get(Future& future) { return future.get(); }
    static void WaitForAll(Std& pool) { pool.WaitForAll(); }

    template <typename Func>
    static void ParallelFor(Std& pool, Opal::u64 end, Func&& func)
    {
        StdParallelFor(pool, 0, end, k_grain_size, std::forward<Func>(func));
    }
};

// Post k_task_count empty tasks from the benchmark thread, then wait for the pool to drain.
template <typename Pool>
void BM_Post(benchmark::State& state)
{
    using Ops = PoolOps<Pool>;
    auto pool = Ops::Make(static_cast<size_t>(state.range(0)));
    if (!Ops::IsValid(pool))
    {
        state.SkipWithError("could not create the pool");
        return;
    }

    for (auto _ : state)
    {
        bool is_posted = true;
        for (size_t i = 0; i < k_task_count && is_posted; i++)
        {
            is_posted = Ops::PostEmpty(*pool);
        }
        Ops::WaitForAll(*pool);
        if (!is_posted)
        {
            state.SkipWithError("the pool did not accept a task");
            break;
        }
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(k_task_count));
}

// Submit k_task_count tasks that each return their index, then take every result from its future.
template <typename Pool>
void BM_Submit(benchmark::State& state)
{
    using Ops = PoolOps<Pool>;
    auto pool = Ops::Make(static_cast<size_t>(state.range(0)));
    if (!Ops::IsValid(pool))
    {
        state.SkipWithError("could not create the pool");
        return;
    }
    std::vector<typename Ops::Future> futures;
    futures.reserve(k_task_count);
    constexpr Item k_expected_sum = Item{k_task_count} * (k_task_count - 1) / 2;

    for (auto _ : state)
    {
        for (size_t i = 0; i < k_task_count; i++)
        {
            futures.push_back(Ops::Submit(*pool, i));
        }
        Item sum = 0;
        for (typename Ops::Future& future : futures)
        {
            sum += Ops::IsValid(future) ? Ops::Get(future) : ~Item{0};
        }
        futures.clear();
        if (sum != k_expected_sum)
        {
            state.SkipWithError("a task was not submitted or its result was lost");
            break;
        }
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(k_task_count));
}

// Submit one task and wait for its result. Workers are idle in between, so each iteration wakes one.
template <typename Pool>
void BM_RoundTrip(benchmark::State& state)
{
    using Ops = PoolOps<Pool>;
    auto pool = Ops::Make(static_cast<size_t>(state.range(0)));
    if (!Ops::IsValid(pool))
    {
        state.SkipWithError("could not create the pool");
        return;
    }

    Item value = 0;
    for (auto _ : state)
    {
        typename Ops::Future future = Ops::Submit(*pool, value);
        if (!Ops::IsValid(future) || Ops::Get(future) != value)
        {
            state.SkipWithError("a task was not submitted or its result was lost");
            break;
        }
        value++;
    }

    state.SetItemsProcessed(state.iterations());
}

// Post one task that posts k_task_count - 1 empty tasks from its worker, then wait for the pool to drain.
template <typename Pool>
void BM_Spawn(benchmark::State& state)
{
    using Ops = PoolOps<Pool>;
    auto pool = Ops::Make(static_cast<size_t>(state.range(0)));
    if (!Ops::IsValid(pool))
    {
        state.SkipWithError("could not create the pool");
        return;
    }

    for (auto _ : state)
    {
        if (!Ops::PostSpawn(*pool))
        {
            state.SkipWithError("the pool did not accept a task");
            break;
        }
        Ops::WaitForAll(*pool);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(k_task_count));
}

// Square every element of an array into another, spread over the workers and the benchmark thread.
template <typename Pool>
void BM_ParallelFor(benchmark::State& state)
{
    using Ops = PoolOps<Pool>;
    auto pool = Ops::Make(static_cast<size_t>(state.range(0)));
    if (!Ops::IsValid(pool))
    {
        state.SkipWithError("could not create the pool");
        return;
    }
    const size_t size = static_cast<size_t>(state.range(1));
    std::vector<Item> input(size);
    for (size_t i = 0; i < size; i++)
    {
        input[i] = i;
    }
    std::vector<Item> output(size, 0);

    for (auto _ : state)
    {
        Ops::ParallelFor(*pool, size, [&](Opal::u64 index) { output[index] = input[index] * input[index]; });
        benchmark::DoNotOptimize(output.data());
        benchmark::ClobberMemory();
    }

    const size_t last = size - 1;
    if (output[last] != last * last)
    {
        state.SkipWithError("ParallelFor skipped an index");
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(size));
}

constexpr int64_t k_thread_counts[] = {1, 2, 4, 8};

// The benchmark thread blocks while the workers run, so its CPU time says nothing. Every benchmark reports wall time.
void ThreadSweep(benchmark::Benchmark* b)
{
    b->ArgName("threads");
    for (const int64_t threads : k_thread_counts)
    {
        b->Arg(threads);
    }
    b->UseRealTime();
}

// 4K elements is four chunks, so at most four threads get one; 4M elements is 64 MB across the two arrays, which is past L3.
void ParallelForSweep(benchmark::Benchmark* b)
{
    b->ArgNames({"threads", "size"});
    for (const int64_t threads : k_thread_counts)
    {
        for (const int shift : {12, 16, 20, 22})
        {
            b->Args({threads, int64_t{1} << shift});
        }
    }
    b->UseRealTime();
}

}  // namespace

// Names come out as "ThreadPool/<operation>/<Opal|Std>/<args>", so the compare.py that ships with Google Benchmark pairs the sides:
//   compare.py filters results.json "/Opal/" "/Std/"
#define POOL_BENCHMARK(fn, name, configure)                                                       \
    BENCHMARK_TEMPLATE(fn, Opal::ThreadPool)->Name("ThreadPool/" name "/Opal")->Apply(configure); \
    BENCHMARK_TEMPLATE(fn, Std)->Name("ThreadPool/" name "/Std")->Apply(configure)

POOL_BENCHMARK(BM_Post, "Post", ThreadSweep);
POOL_BENCHMARK(BM_Submit, "Submit", ThreadSweep);
POOL_BENCHMARK(BM_RoundTrip, "RoundTrip", ThreadSweep);
POOL_BENCHMARK(BM_Spawn, "Spawn", ThreadSweep);
POOL_BENCHMARK(BM_ParallelFor, "ParallelFor", ParallelForSweep);

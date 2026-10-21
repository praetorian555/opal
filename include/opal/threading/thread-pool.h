#pragma once

#include <type_traits>

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/expected.h"
#include "opal/container/scope-ptr.h"
#include "opal/defines.h"
#include "opal/container/shared-ptr.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/executor.h"
#include "opal/threading/thread.h"
#include "opal/type-traits.h"

namespace Opal
{

/**
 * Executor that runs submitted callables on a fixed number of worker threads and hands back a Future for each.
 * Workers block when idle using OS signaling (no busy-waiting). Closing the pool runs every task it has accepted, then stops
 * the workers.
 *
 * A running task may submit more tasks to its own pool. Those are accepted even while the pool is closing, and run before it
 * finishes closing. A task this accepts runs even when Close is called concurrently. Continuations attached with Future::Then
 * run on the worker that finishes the task.
 */
class ThreadPool final : public Executor
{
    struct CreateKey
    {
        explicit CreateKey() = default;
    };

public:
    /**
     * Create a thread pool and start its workers.
     * @param thread_count Number of worker threads to start.
     * @param channel_capacity Capacity of the internal task channel, rounded up to a power of two. Defaults to 128.
     * @param allocator Allocator for the pool, its channel and the tasks it is given. Must be thread safe. If null, uses the
     *                  default allocator.
     * @return The pool, ErrorCode::InvalidArgument if the allocator is not thread safe, ErrorCode::OutOfMemory if the pool or its
     *         channel could not be allocated, or what CreateThread reported for a worker that did not start. Workers that did
     *         start are shut down before an error is returned.
     * @note Each worker takes the calling thread's default allocator as its own, which must be thread safe.
     */
    [[nodiscard]] static Expected<ScopePtr<ThreadPool>, ErrorCode> Create(size_t thread_count, size_t channel_capacity = 128,
                                                                          AllocatorBase* allocator = nullptr);

    /** Used by Create; call Create instead. */
    ThreadPool(CreateKey key, size_t channel_capacity, AllocatorBase* allocator);
    ~ThreadPool() override;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * Queue work for the workers.
     * @return ErrorCode::Success, ErrorCode::ChannelClosed if the pool has been closed and the caller is not one of its workers, or
     *         ErrorCode::InvalidArgument for an invalid pointer.
     */
    ErrorCode Post(SharedPtr<Runnable>&& work) override;

    /**
     * Blocks until every task submitted to the pool has run, including the ones submitted by tasks that were already running.
     * Returns immediately on a closed pool.
     * @note Not safe to call concurrently with Close.
     */
    void WaitForAll();

    /**
     * Shuts down the thread pool. Tasks submitted after this are rejected. Every task already accepted runs, including the
     * ones those tasks submit, and then all worker threads are joined. Safe to call multiple times. Called automatically by
     * the destructor.
     */
    void Close();

    [[nodiscard]] size_t GetThreadCount() const { return m_threads.GetSize(); }
    [[nodiscard]] AllocatorBase* GetAllocator() const override { return m_allocator; }

private:
    /** Ends a submission begun in Post, and wakes a Close waiting for it to finish. */
    void EndSubmission();

    /** Blocks until every task the pool has accepted has run, including the ones those tasks submitted. */
    void WaitForAccepted();

    /** Start thread_count workers. On failure the ones that started keep running until Close. */
    ErrorCode StartWorkers(size_t thread_count);

    /** @return True if the calling thread is one of this pool's workers. */
    [[nodiscard]] bool IsCurrentThreadWorker() const;

    AllocatorBase* m_allocator = nullptr;
    DynamicArray<ThreadHandle> m_threads;
    ChannelMPMC<SharedPtr<Runnable>, true> m_communicator;
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    // Every worker writes the count on every task it finishes, and every submission reads the flag. Sharing a line would put
    // the readers behind an invalidation per completed task.
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<u64> m_completed_count = 0;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<bool> m_is_closed = false;
    Atomic<u32> m_submitting = 0;
    OPAL_END_DISABLE_WARNINGS
};

/**
 * Call func(index) for every index in [begin, end), spread over the pool's workers, and return once every call has returned.
 *
 * The calling thread takes part, and waits only for calls already running on other threads, never for work still queued. So a pool
 * worker may call this, and it finishes even when the pool is busy, closed or out of memory: the calling thread then makes every call
 * itself. Calls happen in no particular order and on any thread, so func must be safe to call concurrently with itself. func must not
 * throw.
 * @param pool Pool whose workers help.
 * @param begin First index.
 * @param end One past the last index. Nothing is called when it is not greater than begin.
 * @param grain_size How many consecutive indices a thread takes at a time. Raise it when a single call is too cheap to be worth
 *                   handing to another thread. 0 is taken as 1.
 * @param func Callable taking a u64 index.
 */
template <typename Func>
void ParallelFor(ThreadPool& pool, u64 begin, u64 end, u64 grain_size, Func&& func);

namespace Impl
{

/** What the threads taking part in one ParallelFor share: the range, how much of it is handed out and how much has finished. */
template <typename Func>
class ParallelForState
{
public:
    ParallelForState(u64 begin, u64 end, u64 grain_size, Func& func)
        : m_func(&func), m_begin(begin), m_end(end), m_grain_size(grain_size), m_chunk_count((end - begin - 1) / grain_size + 1)
    {
    }

    [[nodiscard]] u64 GetChunkCount() const { return m_chunk_count; }

    /** Claim chunks and run them until none are left to claim. */
    void RunChunks()
    {
        while (true)
        {
            const u64 chunk = m_next_chunk.FetchAdd<MemoryOrder::Relaxed>(1);
            if (chunk >= m_chunk_count)
            {
                return;
            }
            const u64 chunk_begin = m_begin + chunk * m_grain_size;
            const u64 remaining = m_end - chunk_begin;
            const u64 chunk_end = chunk_begin + (remaining < m_grain_size ? remaining : m_grain_size);
            for (u64 index = chunk_begin; index < chunk_end; ++index)
            {
                (*m_func)(index);
            }
            if (m_finished_chunks.FetchAdd<MemoryOrder::AcqRel>(1) + 1 == m_chunk_count)
            {
                m_finished_chunks.NotifyAll();
            }
        }
    }

    /** Block until every chunk has finished. Only chunks some thread has claimed are left by then. */
    void WaitForChunks()
    {
        for (u64 finished = m_finished_chunks.Load<MemoryOrder::Acquire>(); finished != m_chunk_count;
             finished = m_finished_chunks.Load<MemoryOrder::Acquire>())
        {
            m_finished_chunks.Wait<MemoryOrder::Acquire>(finished);
        }
    }

private:
    // Only called for a claimed chunk, so a helper that starts after ParallelFor returned never reaches it.
    Func* m_func;
    u64 m_begin;
    u64 m_end;
    u64 m_grain_size;
    u64 m_chunk_count;
    Atomic<u64> m_next_chunk = 0;
    Atomic<u64> m_finished_chunks = 0;
};

/** A pool task that claims chunks of a ParallelFor alongside the thread that started it. */
template <typename Func>
class ParallelForHelper final : public Runnable
{
public:
    explicit ParallelForHelper(SharedPtr<ParallelForState<Func>>&& state) : m_state(std::move(state)) {}

    void Run() override { m_state->RunChunks(); }

private:
    SharedPtr<ParallelForState<Func>> m_state;
};

}  // namespace Impl

}  // namespace Opal

template <typename Func>
void Opal::ParallelFor(ThreadPool& pool, u64 begin, u64 end, u64 grain_size, Func&& func)
{
    using FuncType = std::remove_reference_t<Func>;
    using State = Impl::ParallelForState<FuncType>;
    using Helper = Impl::ParallelForHelper<FuncType>;

    if (begin >= end)
    {
        return;
    }
    if (grain_size == 0)
    {
        grain_size = 1;
    }
    AllocatorBase* allocator = pool.GetAllocator();
    SharedPtr<State> state(allocator, begin, end, grain_size, func);
    if (!state.IsValid())
    {
        for (u64 index = begin; index < end; ++index)
        {
            func(index);
        }
        return;
    }
    // The calling thread takes a share, so one helper fewer than chunks is enough to keep every chunk busy.
    const u64 thread_count = static_cast<u64>(pool.GetThreadCount());
    const u64 helper_count = thread_count < state->GetChunkCount() - 1 ? thread_count : state->GetChunkCount() - 1;
    for (u64 i = 0; i < helper_count; ++i)
    {
        SharedPtr<Helper> helper(allocator, state.Clone());
        if (!helper.IsValid() || pool.Post(SharedPtr<Runnable>(std::move(helper))) != ErrorCode::Success)
        {
            break;
        }
    }
    state->RunChunks();
    state->WaitForChunks();
}

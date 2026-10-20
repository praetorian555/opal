#pragma once

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/defines.h"
#include "opal/container/shared-ptr.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/future.h"
#include "opal/threading/thread.h"
#include "opal/type-traits.h"

namespace Opal
{

/**
 * Thread pool that runs submitted callables on a fixed number of worker threads and hands back a Future for each.
 * Workers block when idle using OS signaling (no busy-waiting). Closing the pool runs every task it has accepted, then stops
 * the workers.
 */
class ThreadPool
{
public:
    /**
     * Creates a thread pool with the given number of worker threads.
     * @param thread_count Number of worker threads to spawn.
     * @param channel_capacity Capacity of the internal task channel. Defaults to 128.
     * @param allocator Allocator for internal storage. If null, uses the default allocator.
     * @throws OutOfMemoryException or Exception when a worker thread could not be started. Workers that did start are shut down first.
     */
    explicit ThreadPool(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr);
    ~ThreadPool();

    /**
     * Submits a callable as a task and returns a future for what it returns.
     *
     * A running task may submit more tasks to its own pool. Those are accepted even while the pool is closing, and run before
     * it finishes closing.
     * @param function Callable taking no arguments. It may return nothing, giving a Future<void>; a plain value of type R, giving a
     *                 Future<R>; or an Expected<R, ErrorCode>, giving a Future<R> that carries the error too.
     * @return The future, ErrorCode::ChannelClosed if the pool has been closed, or ErrorCode::OutOfMemory if the task could not be
     *         allocated. A task this accepts runs even when Close is called concurrently. Continuations attached with Future::Then
     *         run on the worker that finishes the task.
     */
    template <typename Function>
    auto Submit(Function function)
    {
        using ValueType = Impl::FutureValueOf<std::invoke_result_t<Function&>>;
        using ReturnType = Expected<Future<ValueType>, ErrorCode>;
        using State = Impl::TaskState<Function, ValueType>;

        const bool is_from_worker = IsCurrentThreadWorker();
        if (!is_from_worker)
        {
            m_submitting.FetchAdd(1);
            if (m_is_closed.Load())
            {
                EndSubmission();
                return ReturnType(ErrorCode::ChannelClosed);
            }
        }
        SharedPtr<State> state(m_allocator, m_allocator, std::move(function));
        if (!state.IsValid())
        {
            if (!is_from_worker)
            {
                EndSubmission();
            }
            return ReturnType(ErrorCode::OutOfMemory);
        }
        Future<ValueType> future = Impl::FutureAccess::MakeFuture(SharedPtr<Impl::FutureState<ValueType>>(state.Clone()));
        m_communicator.transmitter.Send(SharedPtr<Impl::Runnable>(std::move(state)));
        if (!is_from_worker)
        {
            EndSubmission();
        }
        return ReturnType(std::move(future));
    }

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
    [[nodiscard]] AllocatorBase* GetAllocator() const { return m_allocator; }

private:
    /** Ends a submission begun in Submit, and wakes a Close waiting for it to finish. */
    void EndSubmission();

    /** @return True if the calling thread is one of this pool's workers. */
    [[nodiscard]] bool IsCurrentThreadWorker() const;

    AllocatorBase* m_allocator = nullptr;
    DynamicArray<ThreadHandle> m_threads;
    ChannelMPMC<SharedPtr<Impl::Runnable>, true> m_communicator;
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    // Every worker writes the count on every task it finishes, and every submission reads the flag. Sharing a line would put
    // the readers behind an invalidation per completed task.
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<u64> m_completed_count = 0;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<bool> m_is_closed = false;
    Atomic<u32> m_submitting = 0;
    OPAL_END_DISABLE_WARNINGS
};

}  // namespace Opal

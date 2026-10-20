#pragma once

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
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
public:
    /**
     * Creates a thread pool with the given number of worker threads.
     * @param thread_count Number of worker threads to spawn.
     * @param channel_capacity Capacity of the internal task channel. Defaults to 128.
     * @param allocator Allocator for internal storage. If null, uses the default allocator.
     * @throws OutOfMemoryException or Exception when a worker thread could not be started. Workers that did start are shut down first.
     */
    explicit ThreadPool(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr);
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

}  // namespace Opal

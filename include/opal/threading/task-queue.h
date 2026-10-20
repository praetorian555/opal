#pragma once

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/shared-ptr.h"
#include "opal/error-codes.h"
#include "opal/threading/atomic.h"
#include "opal/threading/executor.h"
#include "opal/threading/mutex.h"
#include "opal/threading/signal.h"

namespace Opal
{

/**
 * Executor whose work runs on whichever thread drains it: a UI thread once a frame, or a thread that owns some resource and waits
 * for work. Any thread may post or submit; RunPending runs what was queued, on the calling thread.
 *
 * The queue is unbounded, so posting never blocks, however long the owner takes to drain it. Work still queued when the queue is
 * destroyed is dropped without running, and the futures for it report ErrorCode::BrokenPromise.
 */
OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)
class TaskQueue final : public Executor
{
public:
    /**
     * @param allocator Allocator for queued work and for the futures Submit creates. Must be thread safe. If null, uses the default
     *                  allocator.
     * @throws InvalidArgumentException if the allocator is not thread safe.
     */
    explicit TaskQueue(AllocatorBase* allocator = nullptr);
    ~TaskQueue() override;

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue& operator=(TaskQueue&&) = delete;

    /**
     * Queue work to run on the next RunPending. Safe to call from any thread, including from work this queue is running.
     * @return ErrorCode::Success, ErrorCode::ChannelClosed after Close, ErrorCode::InvalidArgument for an invalid pointer, or
     *         ErrorCode::OutOfMemory if the queue could not grow.
     */
    ErrorCode Post(SharedPtr<Runnable>&& work) override;

    [[nodiscard]] AllocatorBase* GetAllocator() const override { return m_allocator; }

    /**
     * Run, on the calling thread, the work that was queued when this was called. Work posted while it runs - by that work or by
     * another thread - waits for the next call, so a task that posts itself again cannot keep this from returning.
     * @return Number of tasks run.
     * @note Call from one thread at a time.
     */
    size_t RunPending();

    /**
     * Block until work is queued or the timeout runs out. Returns at once if work is already queued.
     * @param timeout_ms Longest time to wait, in milliseconds.
     * @return True if work is queued.
     */
    bool WaitFor(u64 timeout_ms);

    /** Stop accepting work. What is already queued still runs on the next RunPending. Safe to call more than once. */
    void Close();

    /** @return Number of tasks queued and not yet run. */
    [[nodiscard]] size_t GetPendingCount();

private:
    AllocatorBase* m_allocator = nullptr;
    Mutex<DynamicArray<SharedPtr<Runnable>>> m_pending;
    DynamicArray<SharedPtr<Runnable>> m_running;
    Atomic<bool> m_is_closed = false;
    Atomic<bool> m_is_draining = false;
    Signal m_posted;
};
OPAL_END_DISABLE_WARNINGS

}  // namespace Opal

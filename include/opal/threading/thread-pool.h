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
 * Base class for tasks that can be submitted to a ThreadPool.
 * Subclass and override Execute() to define the work. Tasks can submit follow-up tasks
 * via the transmitter reference passed to Execute().
 */
struct Task
{
    using TransmitterType = TransmitterMPMC<SharedPtr<Task>, true>;

    virtual ~Task() {}

    /**
     * Called by a worker thread to perform the task's work.
     * @param transmitter Can be used to submit follow-up tasks from within a task.
     */
    virtual void Execute(TransmitterType& transmitter) = 0;

    /**
     * Marks the task as completed and wakes any threads waiting in WaitForCompletion().
     */
    void SetCompleted()
    {
        m_is_completed.Store<MemoryOrder::Release>(true);
        m_is_completed.NotifyAll();
    }

    /**
     * Returns true if the task has been completed.
     */
    bool IsCompleted() const { return m_is_completed.Load<MemoryOrder::Acquire>(); }

    /**
     * Blocks the calling thread until the task is completed. Uses OS signaling, not busy-waiting.
     */
    void WaitForCompletion()
    {
        while (!m_is_completed.Load<MemoryOrder::Acquire>())
        {
            m_is_completed.Wait<MemoryOrder::Acquire>(false);
        }
    }

protected:
    Atomic<bool> m_is_completed = false;
};

/**
 * Task implementation that wraps a callable (lambda, function pointer, etc.).
 * The callable must accept a Task::TransmitterType& parameter.
 * @tparam Function Callable type.
 */
template <typename Function>
struct FunctionTask : Task
{
    explicit FunctionTask(Function function) : m_function(std::move(function)) {}

    void Execute(TransmitterType& transmitter) override { m_function(transmitter); }

protected:
    Function m_function;
};

/**
 * Thread pool that distributes tasks across a fixed number of worker threads.
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
     * Submits a callable as a task.
     * @param function Callable that accepts a Task::TransmitterType& parameter.
     * @return Handle that can be used to wait for the task, ErrorCode::ChannelClosed if the pool has been closed, or
     *         ErrorCode::OutOfMemory if the task could not be allocated. A task this accepts runs even when Close is called
     *         concurrently.
     */
    template <typename Function>
    Expected<SharedPtr<Task>, ErrorCode> AddFunctionTask(Function function)
    {
        using ReturnType = Expected<SharedPtr<Task>, ErrorCode>;
        m_submitting.FetchAdd(1);
        if (m_is_closed.Load())
        {
            EndSubmission();
            return ReturnType(ErrorCode::ChannelClosed);
        }
        SharedPtr<FunctionTask<Function>> handle(m_allocator, std::move(function));
        if (!handle.IsValid())
        {
            EndSubmission();
            return ReturnType(ErrorCode::OutOfMemory);
        }
        m_communicator.transmitter.Send(SharedPtr<Task>{handle.Clone()});
        EndSubmission();
        return ReturnType(SharedPtr<Task>{std::move(handle)});
    }

    /**
     * Submits a callable as a task and returns a future for what it returns.
     * @param function Callable taking no arguments. It may return nothing, giving a Future<void>; a plain value of type R, giving a
     *                 Future<R>; or an Expected<R, ErrorCode>, giving a Future<R> that carries the error too.
     * @return The future, ErrorCode::ChannelClosed if the pool has been closed, or ErrorCode::OutOfMemory if the task could not be
     *         allocated. Continuations attached with Future::Then run on the worker that finishes the task.
     */
    template <typename Function>
    auto Submit(Function function)
    {
        using ValueType = Impl::FutureValueOf<std::invoke_result_t<Function&>>;
        using ReturnType = Expected<Future<ValueType>, ErrorCode>;
        Expected<Promise<ValueType>, ErrorCode> promise = Promise<ValueType>::Create(m_allocator);
        if (!promise.HasValue())
        {
            return ReturnType(promise.GetError());
        }
        Future<ValueType> future = promise.GetValue().GetFuture();
        auto task = AddFunctionTask(
            [function = std::move(function), promise = std::move(promise.GetValue())](Task::TransmitterType&) mutable
            { Impl::FulfillWith(promise, function); });
        if (!task.HasValue())
        {
            return ReturnType(task.GetError());
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
    /** Ends a submission begun in AddFunctionTask, and wakes a Close waiting for it to finish. */
    void EndSubmission();

    AllocatorBase* m_allocator = nullptr;
    DynamicArray<ThreadHandle> m_threads;
    ChannelMPMC<SharedPtr<Task>, true> m_communicator;
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

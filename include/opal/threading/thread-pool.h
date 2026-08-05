#pragma once

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/defines.h"
#include "opal/container/shared-ptr.h"
#include "opal/threading/channel-mpmc.h"
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
        m_is_completed.store(true, std::memory_order_release);
        m_is_completed.notify_all();
    }

    /**
     * Returns true if the task has been completed.
     */
    bool IsCompleted() const { return m_is_completed.load(std::memory_order_acquire); }

    /**
     * Blocks the calling thread until the task is completed. Uses OS signaling, not busy-waiting.
     */
    void WaitForCompletion()
    {
        while (!m_is_completed.load(std::memory_order_acquire))
        {
            m_is_completed.wait(false, std::memory_order_acquire);
        }
    }

protected:
    std::atomic<bool> m_is_completed = false;
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
 * Workers block when idle using OS signaling (no busy-waiting). Shutdown is handled
 * by sending sentinel values through the channel to unblock and terminate each worker.
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
     *         ErrorCode::OutOfMemory if the task could not be allocated.
     */
    template <typename Function>
    Expected<SharedPtr<Task>, ErrorCode> AddFunctionTask(Function function)
    {
        using ReturnType = Expected<SharedPtr<Task>, ErrorCode>;
        if (m_is_closed.load(std::memory_order_acquire))
        {
            return ReturnType(ErrorCode::ChannelClosed);
        }
        Expected<SharedPtr<FunctionTask<Function>>, ErrorCode> task =
            SharedPtr<FunctionTask<Function>>::Create(m_allocator, std::move(function));
        if (!task.HasValue())
        {
            return ReturnType(task.GetError());
        }
        SharedPtr<FunctionTask<Function>> handle = std::move(task).GetValue();
        m_communicator.transmitter.Send(SharedPtr<Task>{handle.Clone()});
        return ReturnType(SharedPtr<Task>{std::move(handle)});
    }

    /**
     * Blocks until every task submitted to the pool has run, including the ones submitted by tasks that were already running.
     * Returns immediately on a closed pool.
     * @note Not safe to call concurrently with Close.
     */
    void WaitForAll();

    /**
     * Shuts down the thread pool. Sends sentinel tasks to unblock all workers and joins all threads. Tasks submitted after this
     * are rejected. Safe to call multiple times. Called automatically by the destructor.
     */
    void Close();

    [[nodiscard]] size_t GetThreadCount() const { return m_threads.GetSize(); }
    [[nodiscard]] AllocatorBase* GetAllocator() const { return m_allocator; }

private:
    AllocatorBase* m_allocator = nullptr;
    DynamicArray<ThreadHandle> m_threads;
    ChannelMPMC<SharedPtr<Task>, true> m_communicator;
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    // Every worker writes the count on every task it finishes, and every submission reads the flag. Sharing a line would put
    // the readers behind an invalidation per completed task.
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<u64> m_completed_count = 0;
    alignas(OPAL_CACHE_LINE_SIZE) std::atomic<bool> m_is_closed = false;
    OPAL_END_DISABLE_WARNINGS
};

}  // namespace Opal

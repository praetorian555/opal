#pragma once

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/shared-ptr.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/thread.h"
#include "opal/type-traits.h"

namespace Opal
{

struct Task
{
    using TransmitterType = TransmitterMPMC<SharedPtr<Task>>;

    virtual ~Task() {}

    virtual void Execute(TransmitterType& transmitter) = 0;

    void SetCompleted() { m_is_completed.store(true, std::memory_order_release); }

    bool IsCompleted() const { return m_is_completed.load(std::memory_order_acquire); }

    void WaitForCompletion()
    {
        while (!m_is_completed.load(std::memory_order_acquire))
        {
        }
    }

protected:
    std::atomic<bool> m_is_completed = false;
};

template <typename Function>
struct FunctionTask : Task
{
    explicit FunctionTask(Function function) : m_function(function) {}

    void Execute(TransmitterType& transmitter) override
    {
        m_function(transmitter);
    }

protected:
    Function m_function;
};

class ThreadPool
{
public:
    explicit ThreadPool(size_t thread_count, size_t channel_capacity = 128, AllocatorBase* allocator = nullptr);
    ~ThreadPool();

    template <typename Function>
    SharedPtr<Task> AddFunctionTask(Function function)
    {
        SharedPtr<FunctionTask<Function>> task(m_allocator, function);
        m_communicator.transmitter.Send(SharedPtr<Task>{task.Clone()});
        return SharedPtr<Task>{Move(task)};
    }

    void Close();

private:
    AllocatorBase* m_allocator = nullptr;
    DynamicArray<ThreadHandle> m_threads;
    ChannelMPMC<SharedPtr<Task>> m_communicator;
    std::atomic<bool> m_should_exit = false;
};

}  // namespace Opal
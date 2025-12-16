#include "opal/threading/thread-pool.h"

using ReceiverType = Opal::ReceiverMPMC<Opal::SharedPtr<Opal::Task>>;
using TransmitterType = Opal::TransmitterMPMC<Opal::SharedPtr<Opal::Task>>;
static void ThreadFunction(ReceiverType receiver, TransmitterType transmitter, std::atomic<bool>& should_exit)
{
    while (!should_exit)
    {
        Opal::SharedPtr<Opal::Task> task;
        if (!receiver.TryReceive(task))
        {
            continue;
        }
        task->Execute(transmitter);
        task->SetCompleted();
    }
}

Opal::ThreadPool::ThreadPool(size_t thread_count, size_t channel_capacity, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator()), m_communicator(channel_capacity, m_allocator)
{
    for (size_t i = 0; i < thread_count; ++i)
    {
        const ThreadHandle thread_handle =
            CreateThread(ThreadFunction, m_communicator.receiver.Clone(), m_communicator.transmitter.Clone(), Ref{m_should_exit});
        m_threads.PushBack(thread_handle);
    }
}

Opal::ThreadPool::~ThreadPool()
{
    Close();
}

void Opal::ThreadPool::Close()
{
    if (m_should_exit)
    {
        return;
    }
    m_should_exit = true;
    for (const ThreadHandle& thread : m_threads)
    {
        JoinThread(thread);
    }
    m_threads.Clear();
}
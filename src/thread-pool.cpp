#include "opal/threading/thread-pool.h"

using ReceiverType = Opal::ReceiverMPMC<Opal::SharedPtr<Opal::Task>, true>;
using TransmitterType = Opal::TransmitterMPMC<Opal::SharedPtr<Opal::Task>, true>;
static void ThreadFunction(ReceiverType receiver, TransmitterType transmitter, Opal::Ref<Opal::AllocatorBase> default_allocator)
{
    OPAL_ASSERT(default_allocator->IsThreadSafe(), "Allocator must be thread safe");
    Opal::PushDefaultAllocator(default_allocator.GetPtr());
    while (true)
    {
        Opal::SharedPtr<Opal::Task> task = receiver.Receive();
        if (!task.IsValid())
        {
            // Sentinel received, exit the thread
            break;
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
        ThreadHandle thread_handle = CreateThread(ThreadFunction, m_communicator.receiver.Clone(), m_communicator.transmitter.Clone(),
                                                        Opal::GetDefaultAllocator());
        m_threads.PushBack(std::move(thread_handle));
    }
}

Opal::ThreadPool::~ThreadPool()
{
    Close();
}

void Opal::ThreadPool::Close()
{
    if (m_is_closed)
    {
        return;
    }
    m_is_closed = true;
    // Send one sentinel per thread to unblock all workers
    for (size_t i = 0; i < m_threads.GetSize(); ++i)
    {
        m_communicator.transmitter.Send(SharedPtr<Task>{});
    }
    for (const ThreadHandle& thread : m_threads)
    {
        JoinThread(thread);
    }
    m_threads.Clear();
}

#include "opal/threading/thread-pool.h"

#include "opal/exceptions.h"

using ReceiverType = Opal::ReceiverMPMC<Opal::SharedPtr<Opal::Task>, true>;
using TransmitterType = Opal::TransmitterMPMC<Opal::SharedPtr<Opal::Task>, true>;
static void ThreadFunction(ReceiverType receiver, TransmitterType transmitter, Opal::Ref<Opal::AllocatorBase> default_allocator,
                           Opal::Atomic<Opal::u64>* completed_count)
{
    OPAL_ASSERT(default_allocator->IsThreadSafe(), "Allocator must be thread safe");
    Opal::PushDefaultAllocator(default_allocator.GetPtr());
    while (true)
    {
        auto result = receiver.Receive();
        if (!result.HasValue())
        {
            // Channel closed, exit the thread
            break;
        }
        Opal::SharedPtr<Opal::Task> task = std::move(result.GetValue());
        if (!task.IsValid())
        {
            // Sentinel received, exit the thread
            break;
        }
        task->Execute(transmitter);
        task->SetCompleted();
        completed_count->FetchAdd<Opal::MemoryOrder::Release>(1);
        completed_count->NotifyAll();
    }
}

Opal::ThreadPool::ThreadPool(size_t thread_count, size_t channel_capacity, AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator()), m_communicator(channel_capacity, m_allocator)
{
    // Room for every handle before the first thread starts, so that a started thread can never fail to be recorded and then be left
    // running with no sentinel coming its way.
    if (m_threads.Reserve(thread_count) != ErrorCode::Success)
    {
        OPAL_RAISE(OutOfMemoryException(__FUNCTION__));
    }
    for (size_t i = 0; i < thread_count; ++i)
    {
        Expected<ThreadHandle, ErrorCode> thread_handle =
            CreateThread(ThreadFunction, m_communicator.receiver.Clone(), m_communicator.transmitter.Clone(),
                         Opal::GetDefaultAllocator(), &m_completed_count);
        if (!thread_handle.HasValue())
        {
            // A constructor has nowhere to put a code, so the workers that did start are shut down and the failure is thrown.
            Close();
            if (thread_handle.GetError() == ErrorCode::OutOfMemory)
            {
                OPAL_RAISE(OutOfMemoryException(__FUNCTION__));
            }
            OPAL_RAISE(Exception("Failed to create thread!"));
        }
        m_threads.PushBack(std::move(thread_handle).GetValue());
    }
}

Opal::ThreadPool::~ThreadPool()
{
    Close();
}

void Opal::ThreadPool::WaitForAll()
{
    while (!m_is_closed.Load<MemoryOrder::Acquire>())
    {
        // Read the send count first. Anything counted here has either completed already or is still to come, and a task that
        // submits a follow-up does so before it is counted as completed, so the follow-up is in the next reading.
        const size_t sent = m_communicator.transmitter.GetSendCount();
        const u64 completed = m_completed_count.Load<MemoryOrder::Acquire>();
        if (completed >= sent)
        {
            return;
        }
        m_completed_count.Wait<MemoryOrder::Acquire>(completed);
    }
}

void Opal::ThreadPool::Close()
{
    if (m_is_closed.Exchange<MemoryOrder::AcqRel>(true))
    {
        return;
    }
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

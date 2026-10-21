#include "opal/threading/thread-pool.h"

#include "opal/threading/cpu-pause.h"


namespace
{
using ReceiverType = Opal::ReceiverMPMC<Opal::SharedPtr<Opal::Runnable>, true>;

thread_local const Opal::ThreadPool* t_worker_pool = nullptr;

// Runs a task and counts it as completed, which is what WaitForAll waits on.
void RunTask(Opal::SharedPtr<Opal::Runnable>& task, Opal::Atomic<Opal::u64>* completed_count)
{
    task->Run();
    task.Reset();
    completed_count->FetchAdd<Opal::MemoryOrder::Release>(1);
    completed_count->NotifyAll();
}

void ThreadFunction(ReceiverType receiver, const Opal::ThreadPool* pool, Opal::Ref<Opal::AllocatorBase> default_allocator,
                    Opal::Atomic<Opal::u64>* completed_count)
{
    OPAL_ASSERT(default_allocator->IsThreadSafe(), "Allocator must be thread safe");
    Opal::PushDefaultAllocator(default_allocator.GetPtr());
    t_worker_pool = pool;
    while (true)
    {
        auto result = receiver.Receive();
        if (!result.HasValue())
        {
            break;
        }
        RunTask(result.GetValue(), completed_count);
    }
    t_worker_pool = nullptr;
}
}  // namespace

Opal::Expected<Opal::ScopePtr<Opal::ThreadPool>, Opal::ErrorCode> Opal::ThreadPool::Create(size_t thread_count, size_t channel_capacity,
                                                                                         AllocatorBase* allocator)
{
    using Result = Expected<ScopePtr<ThreadPool>, ErrorCode>;
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    if (!allocator->IsThreadSafe())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    ScopePtr<ThreadPool> pool = MakeScoped<ThreadPool>(allocator, CreateKey{}, channel_capacity, allocator);
    if (!pool.IsValid() || !pool->m_communicator.transmitter.IsValid())
    {
        return Result(ErrorCode::OutOfMemory);
    }
    const ErrorCode started = pool->StartWorkers(thread_count);
    if (started != ErrorCode::Success)
    {
        pool->Close();
        return Result(started);
    }
    return Result(std::move(pool));
}

Opal::ThreadPool::ThreadPool(CreateKey, size_t channel_capacity, AllocatorBase* allocator)
    : m_allocator(allocator), m_threads(allocator), m_communicator(channel_capacity, allocator)
{
}

Opal::ErrorCode Opal::ThreadPool::StartWorkers(size_t thread_count)
{
    // Room for every handle before the first thread starts, so that a started thread can never fail to be recorded and then be left
    // running unjoined.
    const ErrorCode reserved = m_threads.Reserve(thread_count);
    if (reserved != ErrorCode::Success)
    {
        return reserved;
    }
    for (size_t i = 0; i < thread_count; ++i)
    {
        Expected<ThreadHandle, ErrorCode> thread_handle =
            CreateThread(ThreadFunction, m_communicator.receiver.Clone(), static_cast<const ThreadPool*>(this), Opal::GetDefaultAllocator(),
                         &m_completed_count);
        if (!thread_handle.HasValue())
        {
            return thread_handle.GetError();
        }
        m_threads.PushBack(std::move(thread_handle).GetValue());
    }
    return ErrorCode::Success;
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
    if (m_is_closed.Exchange(true))
    {
        return;
    }
    // A submission that saw the pool open is let finish sending, so its task is in the channel before the channel closes.
    for (u32 submitting = m_submitting.Load(); submitting != 0; submitting = m_submitting.Load())
    {
        m_submitting.Wait(submitting);
    }
    if (m_communicator.transmitter.IsValid())
    {
        m_communicator.transmitter.Close();
    }
    for (const ThreadHandle& thread : m_threads)
    {
        JoinThread(thread);
    }
    m_threads.Clear();
}

void Opal::ThreadPool::EndSubmission()
{
    if (m_submitting.FetchSub(1) == 1 && m_is_closed.Load())
    {
        m_submitting.NotifyAll();
    }
}

bool Opal::ThreadPool::IsCurrentThreadWorker() const
{
    return t_worker_pool == this;
}

Opal::ErrorCode Opal::ThreadPool::Post(SharedPtr<Runnable>&& work)
{
    if (!work.IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    const bool is_from_worker = IsCurrentThreadWorker();
    if (!is_from_worker)
    {
        m_submitting.FetchAdd(1);
        if (m_is_closed.Load())
        {
            EndSubmission();
            return ErrorCode::ChannelClosed;
        }
    }
    if (!is_from_worker)
    {
        m_communicator.transmitter.Send(std::move(work));
        EndSubmission();
        return ErrorCode::Success;
    }
    // A worker that waited for room could wait forever: when every worker is sending into a full channel, none is left to empty
    // it. Running a queued task instead makes the room.
    while (!m_communicator.transmitter.TrySend(std::move(work)))
    {
        SharedPtr<Runnable> queued;
        if (m_communicator.receiver.TryReceive(queued) == ErrorCode::Success)
        {
            RunTask(queued, &m_completed_count);
        }
        else
        {
            CpuPause();
        }
    }
    return ErrorCode::Success;
}

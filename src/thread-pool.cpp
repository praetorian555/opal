#include "opal/threading/thread-pool.h"

#include <string_view>

#include "opal/container/string-format.h"
#include "opal/threading/cpu-pause.h"


namespace
{
using ReceiverType = Opal::ReceiverMPMC<Opal::SharedPtr<Opal::Runnable>, true>;

thread_local const Opal::ThreadPool* t_worker_pool = nullptr;

// Set while a worker runs a queued task inside Post to make room in a full channel.
thread_local bool t_is_making_room = false;

// Tasks submitted into a full channel by a task that a worker ran to make room. The Post that ran it sends them before returning.
thread_local Opal::DynamicArray<Opal::SharedPtr<Opal::Runnable>> t_deferred;

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
    ThreadPoolDesc desc;
    desc.thread_count = thread_count;
    desc.channel_capacity = channel_capacity;
    desc.allocator = allocator;
    return Create(desc);
}

Opal::Expected<Opal::ScopePtr<Opal::ThreadPool>, Opal::ErrorCode> Opal::ThreadPool::Create(const ThreadPoolDesc& desc)
{
    using Result = Expected<ScopePtr<ThreadPool>, ErrorCode>;
    AllocatorBase* allocator = desc.allocator != nullptr ? desc.allocator : GetDefaultAllocator();
    if (!allocator->IsThreadSafe())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    if (!desc.worker_cores.IsEmpty())
    {
        if (desc.worker_cores.GetSize() != desc.thread_count)
        {
            return Result(ErrorCode::InvalidArgument);
        }
        for (const u32 core : desc.worker_cores)
        {
            if (core >= 64)
            {
                return Result(ErrorCode::InvalidArgument);
            }
        }
    }
    ScopePtr<ThreadPool> pool = MakeScoped<ThreadPool>(allocator, CreateKey{}, desc.channel_capacity, allocator);
    if (!pool.IsValid() || !pool->m_communicator.transmitter.IsValid())
    {
        return Result(ErrorCode::OutOfMemory);
    }
    const ErrorCode started = pool->StartWorkers(desc);
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

Opal::ErrorCode Opal::ThreadPool::StartWorkers(const ThreadPoolDesc& desc)
{
    const size_t thread_count = desc.thread_count;
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
        const ThreadHandle handle = thread_handle.GetValue();
        m_threads.PushBack(std::move(thread_handle).GetValue());
        if (!desc.worker_cores.IsEmpty())
        {
            SetThreadAffinity(handle, desc.worker_cores[i]);
        }
        if (!desc.name.IsEmpty())
        {
            Expected<StringUtf8, ErrorCode> name = Format("{} {}", std::string_view(desc.name.GetData(), desc.name.GetSize()), i);
            if (name.HasValue())
            {
                SetThreadName(handle, name.GetValue());
            }
        }
    }
    return ErrorCode::Success;
}

Opal::ThreadPool::~ThreadPool()
{
    Close();
}

void Opal::ThreadPool::WaitForAll()
{
    if (!m_is_closed.Load<MemoryOrder::Acquire>())
    {
        WaitForAccepted();
    }
}

void Opal::ThreadPool::WaitForAccepted()
{
    while (true)
    {
        // Read the completed count first. A task only completes after it was sent, so if as many tasks have been sent by the later
        // reading as had completed by the earlier one, every task sent by then had finished and none was running to send more.
        // The other order is not enough: children a running task sends after the send count is read can complete in time to
        // make up the difference while their parent is still sending.
        const u64 completed = m_completed_count.Load<MemoryOrder::Acquire>();
        const size_t sent = m_communicator.transmitter.GetSendCount();
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
        // Tasks still running may submit more, and nothing may be sent into the channel once it is closed. With every accepted
        // task finished, none is left to submit, so the channel can close.
        if (!m_threads.IsEmpty())
        {
            WaitForAccepted();
        }
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
    // A task run to make room that finds the channel full leaves its work to the Post that ran it, instead of running yet another
    // task: every task that submits more would otherwise nest one level deeper, without limit.
    if (t_is_making_room)
    {
        if (m_communicator.transmitter.TrySend(std::move(work)))
        {
            return ErrorCode::Success;
        }
        return t_deferred.PushBack(std::move(work));
    }
    SendMakingRoom(std::move(work));
    while (!t_deferred.IsEmpty())
    {
        SharedPtr<Runnable> deferred = std::move(t_deferred[t_deferred.GetSize() - 1]);
        t_deferred.PopBack();
        SendMakingRoom(std::move(deferred));
    }
    return ErrorCode::Success;
}

void Opal::ThreadPool::SendMakingRoom(SharedPtr<Runnable>&& work)
{
    // A worker that waited for room could wait forever: when every worker is sending into a full channel, none is left to empty
    // it. Running a queued task instead makes the room.
    while (!m_communicator.transmitter.TrySend(std::move(work)))
    {
        SharedPtr<Runnable> queued;
        if (m_communicator.receiver.TryReceive(queued) == ErrorCode::Success)
        {
            t_is_making_room = true;
            RunTask(queued, &m_completed_count);
            t_is_making_room = false;
        }
        else
        {
            CpuPause();
        }
    }
}

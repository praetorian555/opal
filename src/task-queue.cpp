#include "opal/threading/task-queue.h"

#include "opal/common.h"
#include "opal/time.h"

Opal::TaskQueue::TaskQueue(AllocatorBase* allocator)
    : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator()), m_pending(m_allocator), m_running(m_allocator)
{
}

Opal::TaskQueue::~TaskQueue()
{
    Close();
    DynamicArray<SharedPtr<Runnable>> dropped(m_allocator);
    {
        MutexGuard<DynamicArray<SharedPtr<Runnable>>> pending = m_pending.Lock();
        Swap(*pending, dropped);
    }
    // Abandoning can run a continuation, which may post here; the lock is released first, and the post is refused.
    for (SharedPtr<Runnable>& work : dropped)
    {
        work->Abandon();
    }
}

Opal::ErrorCode Opal::TaskQueue::Post(SharedPtr<Runnable>&& work)
{
    if (!work.IsValid() || !m_allocator->IsThreadSafe())
    {
        return ErrorCode::InvalidArgument;
    }
    if (m_is_closed.Load<MemoryOrder::Acquire>())
    {
        return ErrorCode::ChannelClosed;
    }
    {
        MutexGuard<DynamicArray<SharedPtr<Runnable>>> pending = m_pending.Lock();
        if (pending->PushBack(std::move(work)) != ErrorCode::Success)
        {
            return ErrorCode::OutOfMemory;
        }
    }
    m_posted.NotifyAll();
    return ErrorCode::Success;
}

size_t Opal::TaskQueue::RunPending()
{
    const bool was_draining = m_is_draining.Exchange<MemoryOrder::Acquire>(true);
    OPAL_ASSERT(!was_draining, "TaskQueue::RunPending called from two threads at once, or from work it is running");
    if (was_draining)
    {
        return 0;
    }
    {
        MutexGuard<DynamicArray<SharedPtr<Runnable>>> pending = m_pending.Lock();
        Swap(*pending, m_running);
    }
    const size_t count = m_running.GetSize();
    for (SharedPtr<Runnable>& work : m_running)
    {
        work->Run();
    }
    m_running.Clear();
    m_is_draining.Store<MemoryOrder::Release>(false);
    return count;
}

bool Opal::TaskQueue::WaitFor(u64 timeout_ms)
{
    const f64 deadline_ms = GetMilliSeconds() + static_cast<f64>(timeout_ms);
    while (true)
    {
        const u32 posted_state = m_posted.GetState();
        if (GetPendingCount() > 0)
        {
            return true;
        }
        const f64 remaining_ms = deadline_ms - GetMilliSeconds();
        if (remaining_ms <= 0)
        {
            return false;
        }
        m_posted.WaitFor(posted_state, static_cast<u64>(remaining_ms) + 1);
    }
}

void Opal::TaskQueue::Close()
{
    m_is_closed.Store<MemoryOrder::Release>(true);
}

size_t Opal::TaskQueue::GetPendingCount()
{
    return m_pending.Lock()->GetSize();
}

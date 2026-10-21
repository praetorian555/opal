#pragma once

#include <new>

#include "opal/threading/atomic.h"

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/expected.h"
#include "opal/container/shared-ptr.h"
#include "opal/error-codes.h"
#include "opal/threading/cpu-pause.h"
#include "opal/type-traits.h"

namespace Opal
{

namespace Impl
{

template <typename T>
struct QueueMPMCSlot
{
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> turn = 0;
    Atomic<u32> waiters = 0;
    Atomic<u32> wake = 0;
    T data;
    OPAL_END_DISABLE_WARNINGS
};

/**
 * Lock-free multiple-producer multiple-consumer bounded queue.
 * @tparam T Type of data stored in the queue. Must be default constructable, and either copy assignable or clonable.
 * @tparam UseSignaling If true, uses Atomic::Wait/Notify to block when waiting for a slot's turn.
 *         If false, busy-waits using a CPU pause instruction. Defaults to true.
 * @tparam SingleConsumer If true, the caller guarantees that only one thread ever pops, which makes popping cheaper and lets
 *         items pushed after Close still be popped. Defaults to false.
 */
template <typename T, bool UseSignaling = true, bool SingleConsumer = false>
    requires Opal::DefaultConstructable<T> && Opal::MoveAssignable<T>
class QueueMPMC
{
public:
    /** Creates a queue for capacity items, a power of two. If the storage cannot be allocated, GetCapacity reports zero. */
    QueueMPMC(size_t capacity, AllocatorBase* allocator = nullptr) : m_allocator(allocator != nullptr ? allocator : GetDefaultAllocator())
    {
        void* memory = m_allocator->Alloc(capacity * sizeof(QueueMPMCSlot<T>), alignof(QueueMPMCSlot<T>));
        if (memory == nullptr)
        {
            return;
        }
        m_data = static_cast<QueueMPMCSlot<T>*>(memory);
        for (size_t i = 0; i < capacity; ++i)
        {
            new (&m_data[i]) QueueMPMCSlot<T>();
        }
        m_capacity = capacity;
    }

    ~QueueMPMC()
    {
        for (size_t i = 0; i < m_capacity; ++i)
        {
            m_data[i].~QueueMPMCSlot<T>();
        }
        if (m_data != nullptr)
        {
            m_allocator->Free(m_data);
        }
    }

    QueueMPMC(const QueueMPMC&) = delete;
    QueueMPMC& operator=(const QueueMPMC&) = delete;
    QueueMPMC(QueueMPMC&&) = delete;
    QueueMPMC& operator=(QueueMPMC&&) = delete;

    /** @return Number of items the queue holds, or zero if its storage could not be allocated. */
    [[nodiscard]] size_t GetCapacity() const { return m_capacity; }

    /** Push a copy of data, or a clone when T cannot be copy-assigned. Blocks while the queue is full. */
    void Push(const T& data)
        requires(Opal::CopyAssignable<T> || Opal::Clonable<T>)
    {
        PushValue(data);
    }

    /** Push data by moving it. Blocks while the queue is full. */
    void Push(T&& data) { PushValue(Move(data)); }

    /**
     * Push a copy of data, or a clone when T cannot be copy-assigned, unless the queue is full.
     * @return True if data was pushed.
     */
    bool TryPush(const T& data)
        requires(Opal::CopyAssignable<T> || Opal::Clonable<T>)
    {
        return TryPushValue(data);
    }

    /**
     * Push data by moving it, unless the queue is full.
     * @return True if data was pushed. When false, data is left as it was.
     */
    bool TryPush(T&& data) { return TryPushValue(Move(data)); }

    T Pop()
    {
        size_t read_idx = ClaimReadIndex();
        size_t slot_idx = read_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        while (2 * turn + 1 != current_turn)
        {
            if constexpr (UseSignaling)
            {
                WaitForTurn(slot, current_turn);
            }
            else
            {
                CpuPause();
            }
            current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        }
        T result = std::move(slot.data);
        AdvanceTurn(slot, 2 * turn + 2);
        return result;
    }

    bool TryPop(T& result)
    {
        if constexpr (SingleConsumer)
        {
            return TryPopSingleConsumer(result);
        }
        else
        {
            return TryPopShared(result);
        }
    }

    /** @return How many items have ever been pushed. Only grows, so a caller can compare two readings of it. */
    [[nodiscard]] size_t GetPushCount() const { return m_write_idx.Load<MemoryOrder::Acquire>(); }

    /**
     * Pop an item, blocking while the queue is empty and open.
     * @param result Receives the item.
     * @return True if an item was popped, false if the queue is empty and closed. Items pushed before Close are always popped
     *         before this reports the queue closed.
     */
    bool PopUnlessClosed(T& result)
    {
        if constexpr (SingleConsumer)
        {
            return PopSingleConsumerUnlessClosed(result);
        }
        else
        {
            return PopTicketUnlessClosed(result);
        }
    }

private:
    // With signaling, how many times the only consumer polls, pausing twice as long each time, before it goes to sleep.
    static constexpr u32 k_single_consumer_spin_rounds = 8;

    // With signaling, how many times a consumer holding a ticket polls its slot, pausing four times as long each time, before it
    // goes to sleep. The first poll comes quickly, for latency, and later ones are sparse, since every poll pulls the slot's line
    // away from the producer about to write it.
    static constexpr u32 k_ticket_spin_rounds = 5;

    /** TryPop when several consumers may race for the same slot. */
    bool TryPopShared(T& result)
    {
        size_t read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
        while (true)
        {
            size_t slot_idx = read_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn + 1 != slot.turn.template Load<MemoryOrder::Acquire>())
            {
                const size_t prev_read_idx = read_idx;
                read_idx = m_read_idx.Load<MemoryOrder::Acquire>();
                if (prev_read_idx == read_idx)
                {
                    return false;
                }
            }
            else
            {
                if (m_read_idx.CompareExchangeStrong(read_idx, read_idx + 1))
                {
                    result = std::move(slot.data);
                    AdvanceTurn(slot, 2 * turn + 2);
                    return true;
                }
            }
        }
    }

    /** Take the next read index. Only the one consumer ever touches it when SingleConsumer is set, so no atomic increment. */
    size_t ClaimReadIndex()
    {
        if constexpr (SingleConsumer)
        {
            const size_t read_idx = m_read_idx.Load<MemoryOrder::Relaxed>();
            m_read_idx.Store<MemoryOrder::Relaxed>(read_idx + 1);
            return read_idx;
        }
        else
        {
            return m_read_idx.FetchAdd<MemoryOrder::Relaxed>(1);
        }
    }

    /** TryPop for the only consumer: the next slot is the one at m_read_idx, and no one else can take it. */
    bool TryPopSingleConsumer(T& result)
    {
        const size_t read_idx = m_read_idx.Load<MemoryOrder::Relaxed>();
        auto& slot = m_data[read_idx & (m_capacity - 1)];
        const size_t filled_turn = 2 * (read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1))) + 1;
        if (slot.turn.template Load<MemoryOrder::Acquire>() != filled_turn)
        {
            return false;
        }
        result = Move(slot.data);
        m_read_idx.Store<MemoryOrder::Relaxed>(read_idx + 1);
        AdvanceTurn(slot, filled_turn + 1);
        return true;
    }

    /**
     * PopUnlessClosed for the only consumer. Waits for the slot at m_read_idx and advances past it only once it is taken, so
     * reporting the queue closed gives nothing up: an item pushed after that is popped by the next call.
     */
    bool PopSingleConsumerUnlessClosed(T& result)
    {
        const size_t read_idx = m_read_idx.Load<MemoryOrder::Relaxed>();
        auto& slot = m_data[read_idx & (m_capacity - 1)];
        const size_t filled_turn = 2 * (read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1))) + 1;
        for (u32 round = 0; slot.turn.template Load<MemoryOrder::Acquire>() != filled_turn; round++)
        {
            // Acquiring m_is_closed makes every push that happened before Close visible in m_write_idx.
            if (IsClosed() && m_write_idx.Load<MemoryOrder::Acquire>() <= read_idx)
            {
                return false;
            }
            if constexpr (UseSignaling)
            {
                if (round < k_single_consumer_spin_rounds)
                {
                    for (u32 i = 0; i < (1u << round); i++)
                    {
                        CpuPause();
                    }
                    continue;
                }
                // Registering first means a producer that publishes after the fence sees this consumer and wakes it, and one
                // that published before it is seen by the check below. Close wakes it through m_push_count too.
                m_waiting_consumers.FetchAdd<MemoryOrder::Relaxed>(1);
                AsymmetricThreadFenceHeavy();
                const u32 push_count = m_push_count.Load<MemoryOrder::Acquire>();
                if (slot.turn.template Load<MemoryOrder::Acquire>() != filled_turn && !IsClosed())
                {
                    m_push_count.Wait<MemoryOrder::Acquire>(push_count);
                }
                m_waiting_consumers.FetchSub<MemoryOrder::Relaxed>(1);
            }
            else
            {
                CpuPause();
            }
        }
        result = Move(slot.data);
        m_read_idx.Store<MemoryOrder::Relaxed>(read_idx + 1);
        AdvanceTurn(slot, filled_turn + 1);
        return true;
    }

public:
    /**
     * Mark the queue closed and wake every consumer blocked in PopUnlessClosed. Items already in it can still be popped. With
     * more than one consumer, nothing may be pushed once Close has been called: such an item may never be popped, and a later
     * push may block for good. A single-consumer queue pops items pushed after Close too.
     */
    void Close()
    {
        m_is_closed.Store<MemoryOrder::Release>(true);
        if constexpr (UseSignaling)
        {
            m_push_count.FetchAdd<MemoryOrder::Release>(1);
            m_push_count.NotifyAll();
            if constexpr (!SingleConsumer)
            {
                // Pairs with the heavy fence a consumer takes before it sleeps on its slot: either it sees the queue closed, or
                // this sees it waiting.
                AtomicThreadFence();
                for (size_t i = 0; i < m_capacity; ++i)
                {
                    WakeSlot(m_data[i]);
                }
            }
        }
    }

    /** @return True once Close has been called. */
    [[nodiscard]] bool IsClosed() const { return m_is_closed.Load<MemoryOrder::Acquire>(); }

private:
    /** Spin for pauses CPU pauses, then double pauses up to a cap, so a waiter polls less the longer it waits. */
    static void Backoff(u32& pauses)
    {
        for (u32 i = 0; i < pauses; i++)
        {
            CpuPause();
        }
        pauses = pauses < 64 ? pauses * 2 : pauses;
    }

    /**
     * PopUnlessClosed with more than one consumer. Claims the next read index outright, so consumers never race for the same
     * slot, then waits for that slot to be filled. Gives up once the queue is closed and no producer has claimed the index, which
     * no push will then do.
     */
    bool PopTicketUnlessClosed(T& result)
    {
        const size_t read_idx = m_read_idx.FetchAdd<MemoryOrder::Relaxed>(1);
        auto& slot = m_data[read_idx & (m_capacity - 1)];
        const size_t filled_turn = 2 * (read_idx >> CountSetBits(static_cast<u64>(m_capacity - 1))) + 1;
        u32 round = 0;
        while (slot.turn.template Load<MemoryOrder::Acquire>() != filled_turn)
        {
            if constexpr (UseSignaling)
            {
                // Polls only the slot while spinning. m_is_closed shares a line with m_read_idx, which every claim writes.
                if (round < k_ticket_spin_rounds)
                {
                    for (u32 i = 0; i < (1u << (2 * round)); i++)
                    {
                        CpuPause();
                    }
                    round++;
                    continue;
                }
            }
            // Acquiring m_is_closed makes every push that happened before Close visible in m_write_idx.
            if (IsClosed() && m_write_idx.Load<MemoryOrder::Acquire>() <= read_idx)
            {
                return false;
            }
            if constexpr (UseSignaling)
            {
                SleepUntilFilled(slot, filled_turn);
            }
            else
            {
                CpuPause();
            }
        }
        result = Move(slot.data);
        AdvanceTurn(slot, filled_turn + 1);
        return true;
    }

    // A thread waiting on a slot registers in its waiters count, takes the heavy half of an asymmetric fence, then re-checks
    // before it sleeps on the slot's wake count. Whoever changes what it waits for, a new turn or Close, does so, fences, and
    // bumps the wake count only when the slot has waiters. The fences make sure one of them sees the other.

    /** Block until slot's turn moves off observed_turn. */
    static void WaitForTurn(QueueMPMCSlot<T>& slot, size_t observed_turn)
    {
        slot.waiters.template FetchAdd<MemoryOrder::Relaxed>(1);
        AsymmetricThreadFenceHeavy();
        const u32 wake = slot.wake.template Load<MemoryOrder::Acquire>();
        if (slot.turn.template Load<MemoryOrder::Relaxed>() == observed_turn)
        {
            slot.wake.template Wait<MemoryOrder::Acquire>(wake);
        }
        slot.waiters.template FetchSub<MemoryOrder::Relaxed>(1);
    }

    /** Block until slot reaches filled_turn or the queue is closed. */
    void SleepUntilFilled(QueueMPMCSlot<T>& slot, size_t filled_turn)
    {
        slot.waiters.template FetchAdd<MemoryOrder::Relaxed>(1);
        AsymmetricThreadFenceHeavy();
        const u32 wake = slot.wake.template Load<MemoryOrder::Acquire>();
        if (slot.turn.template Load<MemoryOrder::Acquire>() != filled_turn && !IsClosed())
        {
            slot.wake.template Wait<MemoryOrder::Acquire>(wake);
        }
        slot.waiters.template FetchSub<MemoryOrder::Relaxed>(1);
    }

    /** Wake every thread waiting on slot, if there is one. */
    static void WakeSlot(QueueMPMCSlot<T>& slot)
    {
        if (slot.waiters.template Load<MemoryOrder::Relaxed>() != 0)
        {
            slot.wake.template FetchAdd<MemoryOrder::Release>(1);
            slot.wake.NotifyAll();
        }
    }

    /**
     * Publish a slot's next turn and, with signaling, wake the threads waiting for it. Ends with the light half of an asymmetric
     * fence when signaling, which SignalPushed relies on; a thread about to sleep pays for the heavy half.
     */
    static void AdvanceTurn(QueueMPMCSlot<T>& slot, size_t next_turn)
    {
        slot.turn.template Store<MemoryOrder::Release>(next_turn);
        if constexpr (UseSignaling)
        {
            AsymmetricThreadFenceLight();
            WakeSlot(slot);
        }
    }

    /** Wake the single consumer if it sleeps in PopUnlessClosed. Called right after AdvanceTurn published the item. */
    void SignalPushed()
    {
        if constexpr (UseSignaling)
        {
            if (m_waiting_consumers.Load<MemoryOrder::Relaxed>() != 0)
            {
                m_push_count.FetchAdd<MemoryOrder::Release>(1);
                m_push_count.NotifyOne();
            }
        }
    }

    /** Copy, clone or move data into a slot, by what the caller passed. */
    template <typename U>
    static void StoreInto(T& slot_data, U&& data)
    {
        if constexpr (!std::is_lvalue_reference_v<U>)
        {
            slot_data = Move(data);
        }
        else if constexpr (Opal::CopyAssignable<T>)
        {
            slot_data = data;
        }
        else
        {
            slot_data = data.Clone();
        }
    }

    template <typename U>
    void PushValue(U&& data)
    {
        OPAL_ASSERT(SingleConsumer || !IsClosed(), "Pushed into a closed QueueMPMC with several consumers");
        size_t write_idx = m_write_idx.FetchAdd<MemoryOrder::Relaxed>(1);
        size_t slot_idx = write_idx & (m_capacity - 1);
        auto& slot = m_data[slot_idx];
        size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
        size_t current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        u32 pauses = 1;
        while (2 * turn != current_turn)
        {
            if constexpr (UseSignaling)
            {
                WaitForTurn(slot, current_turn);
            }
            else
            {
                Backoff(pauses);
            }
            current_turn = slot.turn.template Load<MemoryOrder::Acquire>();
        }
        StoreInto(slot.data, std::forward<U>(data));
        AdvanceTurn(slot, 2 * turn + 1);
        SignalPushed();
    }

    template <typename U>
    bool TryPushValue(U&& data)
    {
        OPAL_ASSERT(SingleConsumer || !IsClosed(), "Pushed into a closed QueueMPMC with several consumers");
        size_t write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
        while (true)
        {
            size_t slot_idx = write_idx & (m_capacity - 1);
            auto& slot = m_data[slot_idx];
            size_t turn = write_idx >> CountSetBits(static_cast<u64>(m_capacity - 1));
            if (2 * turn != slot.turn.template Load<MemoryOrder::Acquire>())
            {
                // Slot is not empty, we are most likely full
                const size_t prev_write_idx = write_idx;
                write_idx = m_write_idx.Load<MemoryOrder::Acquire>();
                if (write_idx == prev_write_idx)
                {
                    // Nothing changed since the time we loaded the write_idx
                    // report that we are full
                    return false;
                }
            }
            else
            {
                if (m_write_idx.CompareExchangeStrong(write_idx, write_idx + 1))
                {
                    // If write_idx didn't change since last check, consume it and increment the
                    // atomic
                    StoreInto(slot.data, std::forward<U>(data));
                    AdvanceTurn(slot, 2 * turn + 1);
                    SignalPushed();
                    return true;
                }
            }
        }
    }

    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_MSVC_WARNING(4324)
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_write_idx = 0;
    Atomic<u32> m_push_count = 0;
    Atomic<u32> m_waiting_consumers = 0;
    alignas(OPAL_CACHE_LINE_SIZE) Atomic<size_t> m_read_idx = 0;
    OPAL_END_DISABLE_WARNINGS
    AllocatorBase* m_allocator = nullptr;
    QueueMPMCSlot<T>* m_data = nullptr;
    size_t m_capacity = 0;
    Atomic<bool> m_is_closed = false;
};

}  // namespace Impl

/**
 * Producer end of a multiple-producer multiple-consumer communication channel.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being sent.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 * @tparam SingleConsumer True for the producer end of a ChannelMPSC, which TransmitterMPSC names. Defaults to false.
 */
template <typename T, bool UseSignaling = true, bool SingleConsumer = false>
struct TransmitterMPMC
{
    TransmitterMPMC() = default;
    explicit TransmitterMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling, SingleConsumer>>&& q) : m_queue(std::move(q)) {}
    ~TransmitterMPMC() = default;

    TransmitterMPMC(const TransmitterMPMC& q) = delete;
    TransmitterMPMC& operator=(const TransmitterMPMC& q) = delete;

    TransmitterMPMC(TransmitterMPMC&& other) noexcept : m_queue(std::move(other.m_queue)) {}
    TransmitterMPMC& operator=(TransmitterMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        return *this;
    }

    TransmitterMPMC Clone() const { return TransmitterMPMC{m_queue.Clone()}; }

    bool operator==(const TransmitterMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

    void Send(const T& item) { m_queue->Push(item); }
    void Send(T&& item) { m_queue->Push(std::move(item)); }
    bool TrySend(const T& item) { return m_queue->TryPush(item); }

    /**
     * Send item by moving it, unless the channel is full.
     * @return True if item was sent. When false, item is left as it was.
     */
    bool TrySend(T&& item) { return m_queue->TryPush(std::move(item)); }

    /** @return How many items have ever been sent through this channel, by any transmitter sharing it. */
    [[nodiscard]] size_t GetSendCount() const { return m_queue->GetPushCount(); }

    /**
     * Marks the channel as closed and wakes any receiver blocked in Receive(). Items sent before this are still received;
     * once they are drained, Receive() and TryReceive() return ErrorCode::ChannelClosed without blocking. Through a
     * ChannelMPMC, nothing may be sent after this; through a ChannelMPSC, a later send is still received.
     */
    void Close() { m_queue->Close(); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling, SingleConsumer>> m_queue;
};

/**
 * Consumer end of a multiple-producer multiple-consumer communication channel.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being received.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ReceiverMPMC
{
    ReceiverMPMC() = default;
    explicit ReceiverMPMC(SharedPtr<Impl::QueueMPMC<T, UseSignaling>>&& q) : m_queue(std::move(q)) {}
    ~ReceiverMPMC() = default;

    ReceiverMPMC(const ReceiverMPMC& q) = delete;
    ReceiverMPMC& operator=(const ReceiverMPMC& q) = delete;

    ReceiverMPMC(ReceiverMPMC&& other) noexcept : m_queue(std::move(other.m_queue)) {}
    ReceiverMPMC& operator=(ReceiverMPMC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        return *this;
    }

    ReceiverMPMC Clone() const { return ReceiverMPMC{m_queue.Clone()}; }

    /**
     * Blocks until an item is available or the channel is closed.
     * Items already in the queue are drained first. Only after the queue is empty
     * and the channel has been closed does this return ErrorCode::ChannelClosed.
     */
    Expected<T, ErrorCode> Receive()
    {
        T result;
        if (m_queue->PopUnlessClosed(result))
        {
            return Expected<T, ErrorCode>(std::move(result));
        }
        return Expected<T, ErrorCode>(ErrorCode::ChannelClosed);
    }

    /**
     * Non-blocking receive. Items already in the queue are drained first.
     * Only after the queue is empty and the channel has been closed does this
     * return ErrorCode::ChannelClosed.
     */
    ErrorCode TryReceive(T& result)
    {
        if (m_queue->TryPop(result))
        {
            return ErrorCode::Success;
        }
        if (m_queue->IsClosed())
        {
            return m_queue->TryPop(result) ? ErrorCode::Success : ErrorCode::ChannelClosed;
        }
        return ErrorCode::ChannelEmpty;
    }

    bool operator==(const ReceiverMPMC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling>> m_queue;
};

/**
 * One-way, thread-safe communication channel with multiple producers and multiple consumers.
 * Use the transmitter field to send data and the receiver field to receive it. If the channel's storage cannot be allocated,
 * both ends are left invalid; check IsValid before using them.
 * @tparam T Type of data to be sent over the channel.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ChannelMPMC
{
    TransmitterMPMC<T, UseSignaling> transmitter;
    ReceiverMPMC<T, UseSignaling> receiver;

    explicit ChannelMPMC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        capacity = GetNextPowerOf2(capacity);
        SharedPtr<Impl::QueueMPMC<T, UseSignaling>> q(allocator, capacity, allocator);
        if (!q.IsValid() || q->GetCapacity() == 0)
        {
            return;
        }
        transmitter = TransmitterMPMC<T, UseSignaling>(q.Clone());
        receiver = ReceiverMPMC<T, UseSignaling>(std::move(q));
    }
};

}  // namespace Opal

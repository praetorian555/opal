#pragma once

#include "opal/threading/channel-mpmc.h"

namespace Opal
{

/**
 * Producer end of a multiple-producer single-consumer communication channel.
 * It can be both copied and moved. Use Clone for copying.
 * @tparam T Type of data that is being sent.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
using TransmitterMPSC = TransmitterMPMC<T, UseSignaling, true>;

/**
 * Consumer end of a multiple-producer single-consumer communication channel.
 * It can only be moved: there is one consumer, which is what makes receiving cheaper than through a ChannelMPMC.
 * @tparam T Type of data that is being received.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ReceiverMPSC
{
    ReceiverMPSC() = default;
    explicit ReceiverMPSC(SharedPtr<Impl::QueueMPMC<T, UseSignaling, true>>&& q) : m_queue(std::move(q)) {}
    ~ReceiverMPSC() = default;

    ReceiverMPSC(const ReceiverMPSC& q) = delete;
    ReceiverMPSC& operator=(const ReceiverMPSC& q) = delete;

    ReceiverMPSC(ReceiverMPSC&& other) noexcept : m_queue(std::move(other.m_queue)) {}
    ReceiverMPSC& operator=(ReceiverMPSC&& other) noexcept
    {
        if (*this == other)
        {
            return *this;
        }
        m_queue = std::move(other.m_queue);
        return *this;
    }

    /**
     * Blocks until an item is available or the channel is closed.
     * Items already in the queue are drained first. Only after the queue is empty and the channel has been closed does this
     * return ErrorCode::ChannelClosed. An item sent after that is still received by a later call.
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
     * @return ErrorCode::Success with an item in result, ErrorCode::ChannelEmpty, or ErrorCode::ChannelClosed once the channel
     *         is closed and empty.
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

    bool operator==(const ReceiverMPSC& other) const { return m_queue == other.m_queue; }

    [[nodiscard]] bool IsValid() const { return m_queue.IsValid(); }

private:
    SharedPtr<Impl::QueueMPMC<T, UseSignaling, true>> m_queue;
};

/**
 * One-way, thread-safe communication channel with multiple producers and a single consumer.
 * Use the transmitter field to send data and the receiver field to receive it. Clone the transmitter for each producer; the
 * receiver can only be moved. If the channel's storage cannot be allocated, both ends are left invalid; check IsValid before
 * using them.
 * @tparam T Type of data to be sent over the channel.
 * @tparam UseSignaling If true, blocking operations use OS signaling. If false, busy-waits. Defaults to true.
 */
template <typename T, bool UseSignaling = true>
struct ChannelMPSC
{
    TransmitterMPSC<T, UseSignaling> transmitter;
    ReceiverMPSC<T, UseSignaling> receiver;

    explicit ChannelMPSC(size_t capacity, AllocatorBase* allocator = nullptr)
    {
        capacity = GetNextPowerOf2(capacity);
        SharedPtr<Impl::QueueMPMC<T, UseSignaling, true>> q(allocator, capacity, allocator);
        if (!q.IsValid() || q->GetCapacity() == 0)
        {
            return;
        }
        transmitter = TransmitterMPSC<T, UseSignaling>(q.Clone());
        receiver = ReceiverMPSC<T, UseSignaling>(std::move(q));
    }
};

}  // namespace Opal

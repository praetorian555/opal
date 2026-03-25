#pragma once

#include "opal/threading/cpu-pause.h"
#include "opal/threading/signal.h"

namespace Opal
{

/**
 * Wait policy that busy-waits using a CPU pause instruction.
 * Lowest latency option but consumes CPU while waiting. Best suited for short, infrequent waits
 * where the producer and consumer are running on dedicated cores.
 */
struct SpinWaitPolicy
{
    void Reset() {}
    void Wait() { CpuPause(); }
    void NotifyOne() {}
};

/**
 * Wait policy that blocks the calling thread using the Signal primitive (WaitOnAddress/futex).
 * Frees up CPU while waiting at the cost of higher wake-up latency due to kernel involvement.
 * Best suited for workloads where the queue may be idle for extended periods.
 */
struct SignalWaitPolicy
{
    Signal m_signal;
    u32 m_expected_state = 0;

    void Reset() { m_expected_state = m_signal.GetState(); }

    void Wait()
    {
        m_signal.Wait(m_expected_state);
        m_expected_state = m_signal.GetState();
    }

    void NotifyOne() { m_signal.NotifyOne(); }
};

}  // namespace Opal

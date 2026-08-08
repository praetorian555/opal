#pragma once

#include "opal/defines.h"
#include "opal/threading/atomic.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Lightweight synchronization primitive for signaling changes between threads.
 * Uses WaitOnAddress on Windows and futex on Linux. Does not require a mutex or an allocator.
 * Internally uses a monotonic u32 counter to avoid lost notifications.
 * Can be neither copied nor moved: waiters are parked on the address of this object, so a relocated signal would never reach
 * them again.
 */
OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)
class alignas(OPAL_CACHE_LINE_SIZE) Signal
{
public:
    Signal();
    ~Signal();

    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    Signal(Signal&&) = delete;
    Signal& operator=(Signal&&) = delete;

    /**
     * Returns the current state counter.
     */
    u32 GetState() const;

    /**
     * Blocks the calling thread until the state differs from expected_state.
     * Handles spurious wakeups internally.
     * @param expected_state Value to compare against.
     */
    void Wait(u32 expected_state);

    /**
     * Blocks the calling thread until the state differs from expected_state, or until timeout expires.
     * Handles spurious wakeups internally.
     * @param expected_state Value to compare against.
     * @param timeout_ms Maximum time to wait in milliseconds.
     * @return true if state changed, false if timed out.
     */
    bool WaitFor(u32 expected_state, u64 timeout_ms);

    /**
     * Advances the state and wakes one waiting thread.
     */
    void NotifyOne();

    /**
     * Advances the state and wakes all waiting threads.
     */
    void NotifyAll();

private:
    Atomic<u32> m_state{0};
};
OPAL_END_DISABLE_WARNINGS

}  // namespace Opal

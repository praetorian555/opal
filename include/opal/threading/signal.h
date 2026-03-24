#pragma once

#include <atomic>

#include "opal/types.h"

namespace Opal
{

/**
 * Lightweight synchronization primitive for signaling changes between threads.
 * Uses WaitOnAddress on Windows and futex on Linux. Does not require a mutex or an allocator.
 * Internally uses a monotonic u32 counter to avoid lost notifications.
 */
class Signal
{
public:
    Signal();
    ~Signal();

    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    Signal(Signal&& other) noexcept;
    Signal& operator=(Signal&& other) noexcept;

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
    std::atomic<u32> m_state{0};
};

}  // namespace Opal

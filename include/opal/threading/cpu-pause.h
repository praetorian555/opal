#pragma once

#include "opal/defines.h"
#include "opal/types.h"

#if defined(OPAL_COMPILER_MSVC)
#include <intrin.h>
#endif

namespace Opal
{

/**
 * Emits a platform-specific pause/yield hint for spin-wait loops.
 * On x86 this is the PAUSE instruction, on ARM64 it is YIELD. Reduces power consumption
 * and improves performance of the other hyperthread on the same core.
 */
inline void CpuPause()
{
#if defined(OPAL_COMPILER_MSVC)
    _mm_pause();
#elif defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG)
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#elif defined(__aarch64__)
    asm volatile("yield");
#endif
#endif
}

/** Give up the rest of the calling thread's time slice to another thread that is ready to run, if there is one. */
void YieldCurrentThread();

/**
 * Backoff for a spin-wait loop, one step per failed poll. Pauses double each step up to a cap, then every further step yields
 * the time slice. Pausing keeps latency low while the awaited thread runs on another core; yielding lets it run when it shares
 * this core, where pausing alone would leave it waiting for the spinner's whole time slice.
 */
class SpinBackoff
{
public:
    void Wait()
    {
        if (m_pauses > k_max_pauses)
        {
            YieldCurrentThread();
            return;
        }
        for (u32 i = 0; i < m_pauses; i++)
        {
            CpuPause();
        }
        m_pauses *= 2;
    }

private:
    static constexpr u32 k_max_pauses = 64;
    u32 m_pauses = 1;
};

}  // namespace Opal
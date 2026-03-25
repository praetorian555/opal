#pragma once

#include "opal/defines.h"

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

}  // namespace Opal
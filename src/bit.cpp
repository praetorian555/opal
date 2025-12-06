#include "opal/bit.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <intrin.h>
#endif

#include "opal/defines.h"

Opal::u32 Opal::CountLeadingZeros(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __lzcnt(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u32>(__builtin_clz(value));
#else
#error "Compiler not supported"
#endif
}

Opal::u64 Opal::CountLeadingZeros(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __lzcnt64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u64>(__builtin_clzll(value));
#else
#error "Compiler not supported"
#endif
}

Opal::u32 Opal::CountTrailingZeros(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return _tzcnt_u32(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u32>(__builtin_ctz(value));
#else
#error "Compiler not supported"
#endif
}

Opal::u64 Opal::CountTrailingZeros(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return _tzcnt_u64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return static_cast<u64>(__builtin_ctzll(value));
#else
#error "Compiler not supported"
#endif
}

Opal::u32 Opal::CountSetBits(u32 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __popcnt(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return __builtin_popcount(value);
#else
    throw NotImplementedException(__FUNCTION__)
#endif
}

Opal::u64 Opal::CountSetBits(u64 value)
{
#if defined(OPAL_COMPILER_MSVC)
    return __popcnt64(value);
#elif (defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG))
    return __builtin_popcountll(value);
#else
    throw NotImplementedException(__FUNCTION__)
#endif
}

template <>
OPAL_EXPORT Opal::u32 Opal::GetBitWidth<Opal::u8>()
{
    return 8;
}

template <>
OPAL_EXPORT Opal::u32 Opal::GetBitWidth<Opal::u16>()
{
    return 16;
}

template <>
OPAL_EXPORT Opal::u32 Opal::GetBitWidth<Opal::u32>()
{
    return 32;
}

template <>
OPAL_EXPORT Opal::u32 Opal::GetBitWidth<Opal::u64>()
{
    return 64;
}

Opal::u64 Opal::GetNextPowerOf2(u64 value)
{
    return value < 1 ? 1ull : 1ull << (64ull - CountLeadingZeros(value - 1));
}

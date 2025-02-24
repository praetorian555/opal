﻿#pragma once

#include "opal/export.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Pseudo-random number generator based on the paper PCG: A Family of Simple Fast
 * Space-Efficient Statistically Good Algorithms for Random Number Generation by O'Neill (2014).
 * Uniformally generates number in a given range.
 */
class OPAL_EXPORT RNG
{
public:
    /**
     * RNG with default seed.
     */
    RNG();

    /**
     * RNG with custom seed.
     */
    explicit RNG(u64 starting_index);

    /**
     * Configure the RNG seed.
     * @param starting_index Seed.
     */
    void SetSequence(u64 starting_index);

    /**
     * @brief Generate a uniform random number in range [0, UINT32_MAX).
     * @return Random number.
     */
    u32 RandomU32();

    /**
     * @brief Generate a unsigned int 32 uniform random number in range [min, max).
     * @param min Lower bound. Inclusive.
     * @param max Upper bound. Exclusive.
     * @return Random number.
     */
    u32 RandomU32(u32 min, u32 max);

    /**
     * @brief Generate a uniform random number in range [INT32_MIN, INT32_MAX).
     * @return Random number.
     */
    i32 RandomI32();

    /**
     * @brief Generate a signed int 32 uniform random number in range [min, max).
     * @param min Lower bound. Inclusive.
     * @param max Upper bound. Exclusive.
     * @return Random number.
     */
    i32 RandomI32(i32 min, i32 max);

    /**
     * @brief Generate a uniform random number in range [0, 1).
     * @return Random number.
     */
    f32 RandomF32();

    /**
     * @brief Generate a uniform random number in range [min, max).
     * @param min Lower bound. Inclusive.
     * @param max Upper bound. Exclusive.
     * @return Random number.
     */
    f32 RandomF32(f32 min, f32 max);

private:
    u64 m_state = 0;
    u64 m_inc = 0;
};

}  // namespace Opal

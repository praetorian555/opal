#pragma once

#include "opal/types.h"
#include "opal/export.h"

namespace Opal
{

/**
 * Pseudo-random number generator based on the paper PCG: A Family of Simple Fast
 * Space-Efficient Statistically Good Algorithms for Random Number Generation by O'Neill (2014).
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
     * @brief Generate a uniform random number in range [0, UINT32_MAX - 1].
     * @return Random number.
     */
    u32 UniformUInt32();

    /**
     * @brief Generate a uniform random number in range [0, limit - 1].
     * @param limit Upper bound.
     * @return Random number.
     */
    u32 UniformUInt32(u32 limit);

    /**
     * @brief Generate a uniform random number in range [0, 1).
     * @return Random number.
     */
    float UniformFloat();

    /**
     * @brief Generate a uniform random number in range [start, end).
     * @param start Lower bound.
     * @param end Upper bound.
     * @return Random number.
     */
    float UniformFloatInRange(float start, float end);

private:
    u64 m_state = 0;
    u64 m_inc = 0;
};

}  // namespace Opal

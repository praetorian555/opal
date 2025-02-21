#include "opal/rng.h"
#include "opal/assert.h"
#include "opal/math-base.h"

static constexpr Opal::u64 k_default_state = 0x853c49e6748fea9bULL;
static constexpr Opal::u64 k_default_stream = 0xda3e39cb94b95bdbULL;
static constexpr Opal::u64 k_mul_float = 0x5851f42d4c957f2dULL;
// Largest floating-point constant less then 1
static constexpr float k_one_minus_epsilon = 0x1.fffffep-1;

Opal::RNG::RNG() : m_state(k_default_state), m_inc(k_default_stream) {}

Opal::RNG::RNG(u64 starting_index)
{
    SetSequence(starting_index);
}

void Opal::RNG::SetSequence(u64 starting_index)
{
    m_state = 0u;
    m_inc = (starting_index << 1u) | 1u;
    UniformUInt32();
    m_state += k_default_state;
    UniformUInt32();
}

Opal::u32 Opal::RNG::UniformUInt32()
{
    const u64 old_state = m_state;
    m_state = old_state * k_mul_float + m_inc;
    const auto xor_shifted = static_cast<u32>(((old_state >> 18u) ^ old_state) >> 27u);
    const auto Rot = static_cast<u32>(old_state >> 59u);
    constexpr u32 k_lowest_five_bits = 31u;
    return (xor_shifted >> Rot) | (xor_shifted << ((~Rot + 1u) & k_lowest_five_bits));
}

Opal::u32 Opal::RNG::UniformUInt32(u32 limit)
{
    const u32 threshold = (~limit + 1u) % limit;
    while (true)
    {
        const u32 random = UniformUInt32();
        if (random >= threshold)
        {
            return random % limit;
        }
    }
}

float Opal::RNG::UniformFloat()
{
    constexpr float k_scalar = 0x1p-32f;
    return Min(k_one_minus_epsilon, static_cast<float>(UniformUInt32()) * k_scalar);
}

float Opal::RNG::UniformFloatInRange(float start, float end)
{
    OPAL_ASSERT(start <= end, "Invalid range");

    const float value = UniformFloat();
    return start + value * (end - start);
}
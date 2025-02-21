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
    RandomU32();
    m_state += k_default_state;
    RandomU32();
}

Opal::u32 Opal::RNG::RandomU32()
{
    const u64 old_state = m_state;
    m_state = old_state * k_mul_float + m_inc;
    const auto xor_shifted = static_cast<u32>(((old_state >> 18u) ^ old_state) >> 27u);
    const auto Rot = static_cast<u32>(old_state >> 59u);
    constexpr u32 k_lowest_five_bits = 31u;
    return (xor_shifted >> Rot) | (xor_shifted << ((~Rot + 1u) & k_lowest_five_bits));
}

Opal::u32 Opal::RNG::RandomU32(u32 min, u32 max)
{
    OPAL_ASSERT(max >= min, "Invalid range!");
    const u32 range = max - min;
    const u32 x = RandomU32();
    return min + (u32)((static_cast<u64>(x) * static_cast<u64>(range)) >> 32);
}

Opal::i32 Opal::RNG::RandomI32()
{
    return static_cast<i32>(RandomU32());
}

Opal::i32 Opal::RNG::RandomI32(i32 min, i32 max)
{
    OPAL_ASSERT(max >= min, "Invalid range!");
    u32 range = static_cast<u32>(max - min);
    u32 x = RandomU32();
    return min + (i32)((static_cast<u64>(x) * static_cast<u64>(range)) >> 32);
}

Opal::f32 Opal::RNG::RandomF32()
{
    constexpr f32 k_scalar = 0x1p-32f;
    return Min(k_one_minus_epsilon, static_cast<f32>(RandomU32()) * k_scalar);
}

Opal::f32 Opal::RNG::RandomF32(f32 min, f32 max)
{
    OPAL_ASSERT(min <= max, "Invalid range");
    const f32 value = RandomF32();
    return min + value * (max - min);
}
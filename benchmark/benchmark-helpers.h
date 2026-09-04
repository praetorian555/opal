#pragma once

#include <benchmark/benchmark.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "opal/rng.h"
#include "opal/types.h"

// Shared by every benchmark file: deterministic key sequences and the size sweep, so every container sees the same input and
// is observed in the same cache regimes.
namespace Bench
{

using Key = Opal::u32;

// Distinct RNG sequences so the "miss" keys do not overlap the inserted keys except by chance.
constexpr Opal::u64 k_keys_sequence = 1;
constexpr Opal::u64 k_miss_sequence = 2;
constexpr Opal::u64 k_shuffle_sequence = 3;

/**
 * Random 32-bit keys from a fixed sequence, so every run and every container sees identical input.
 * Keys are not deduplicated. At 16M keys roughly 0.2% are repeats, which is the same for every side.
 */
inline std::vector<Key> MakeKeys(size_t count, Opal::u64 sequence)
{
    Opal::RNG rng(sequence);
    std::vector<Key> keys;
    keys.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        keys.push_back(rng.RandomU32());
    }
    return keys;
}

/**
 * Fisher-Yates shuffle with a fixed sequence. Lookups use a different order than insertion, otherwise
 * a node-based container gets a memory-order walk through nodes allocated in insertion order.
 */
inline std::vector<Key> Shuffled(std::vector<Key> keys, Opal::u64 sequence)
{
    Opal::RNG rng(sequence);
    for (size_t i = keys.size(); i > 1; i--)
    {
        const size_t j = rng.RandomU32(0, static_cast<Opal::u32>(i));
        std::swap(keys[i - 1], keys[j]);
    }
    return keys;
}

// 1K, 8K, 64K, 512K, 4M, 16M keys: L1, L2, L3, and DRAM regimes for both layouts.
inline void SizeSweep(benchmark::Benchmark* b)
{
    for (const int shift : {10, 13, 16, 19, 22, 24})
    {
        b->Arg(int64_t{1} << shift);
    }
}

inline void SizeSweepManualTime(benchmark::Benchmark* b)
{
    SizeSweep(b);
    b->UseManualTime();
}

}  // namespace Bench

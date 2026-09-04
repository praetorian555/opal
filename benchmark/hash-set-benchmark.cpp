// Compares Opal::HashSet against std::unordered_set on the operations that matter for a set:
// insert (growing and pre-reserved), lookup (hit and miss), erase, copy, and iteration.
//
// Every benchmark body is written once as a template over the set type, so both sides do exactly
// the same work and only the container differs. Sizes sweep from 1K to 16M keys so each benchmark
// is observed while the working set fits in L1, L2, L3, and finally DRAM.
//
// See docs/benchmarks.md for how to build, run and compare the results.

#include <benchmark/benchmark.h>

#include <chrono>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "opal/container/hash-set.h"
#include "opal/rng.h"

namespace
{

using Key = Opal::u32;

// Distinct RNG sequences so the "miss" keys do not overlap the inserted keys except by chance.
constexpr Opal::u64 k_keys_sequence = 1;
constexpr Opal::u64 k_miss_sequence = 2;
constexpr Opal::u64 k_shuffle_sequence = 3;

/**
 * Uniform interface over the two set types so every benchmark body is written once.
 */
template <typename Set>
struct SetOps;

template <typename K>
struct SetOps<Opal::HashSet<K>>
{
    using Set = Opal::HashSet<K>;

    static Set Make() { return Set(); }
    static void Reserve(Set& set, size_t count) { set.Reserve(count); }
    static void Insert(Set& set, const K& key) { set.Insert(key); }
    static void Erase(Set& set, const K& key) { set.Erase(key); }
    static bool Contains(const Set& set, const K& key) { return set.Contains(key); }
    static Set Copy(const Set& set) { return set.Clone(); }
    static size_t Size(const Set& set) { return set.GetSize(); }
};

template <typename K>
struct SetOps<std::unordered_set<K>>
{
    using Set = std::unordered_set<K>;

    static Set Make() { return Set(); }
    static void Reserve(Set& set, size_t count) { set.reserve(count); }
    static void Insert(Set& set, const K& key) { set.insert(key); }
    static void Erase(Set& set, const K& key) { set.erase(key); }
    static bool Contains(const Set& set, const K& key) { return set.find(key) != set.end(); }
    static Set Copy(const Set& set) { return Set(set); }
    static size_t Size(const Set& set) { return set.size(); }
};

/**
 * Random 32-bit keys from a fixed sequence, so every run and both set types see identical input.
 * Keys are not deduplicated. At 16M keys roughly 0.2% are repeats, which is the same for both sides.
 */
std::vector<Key> MakeKeys(size_t count, Opal::u64 sequence)
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
 * a node-based set gets a memory-order walk through nodes allocated in insertion order.
 */
std::vector<Key> Shuffled(std::vector<Key> keys, Opal::u64 sequence)
{
    Opal::RNG rng(sequence);
    for (size_t i = keys.size(); i > 1; i--)
    {
        const size_t j = rng.RandomU32(0, static_cast<Opal::u32>(i));
        std::swap(keys[i - 1], keys[j]);
    }
    return keys;
}

template <typename Set>
Set BuildSet(const std::vector<Key>& keys)
{
    using Ops = SetOps<Set>;
    Set set = Ops::Make();
    Ops::Reserve(set, keys.size());
    for (const Key& key : keys)
    {
        Ops::Insert(set, key);
    }
    return set;
}

// Insert N keys into an empty set, letting it grow on its own. Includes every rehash and the final destruction.
template <typename Set>
void BM_InsertGrow(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);

    for (auto _ : state)
    {
        Set set = Ops::Make();
        for (const Key& key : keys)
        {
            Ops::Insert(set, key);
        }
        benchmark::DoNotOptimize(set);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Insert N keys into a set that has already reserved room for them. Isolates the probe and write cost from growth.
template <typename Set>
void BM_InsertReserved(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);

    for (auto _ : state)
    {
        Set set = Ops::Make();
        Ops::Reserve(set, count);
        for (const Key& key : keys)
        {
            Ops::Insert(set, key);
        }
        benchmark::DoNotOptimize(set);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Look up N keys that are all present, in an order different from insertion.
template <typename Set>
void BM_FindHit(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> lookups = Shuffled(keys, k_shuffle_sequence);
    const Set set = BuildSet<Set>(keys);

    for (auto _ : state)
    {
        size_t hits = 0;
        for (const Key& key : lookups)
        {
            hits += Ops::Contains(set, key) ? 1u : 0u;
        }
        benchmark::DoNotOptimize(hits);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Look up N keys that are (almost certainly) absent. Exercises the full probe sequence to a miss.
template <typename Set>
void BM_FindMiss(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> lookups = MakeKeys(count, k_miss_sequence);
    const Set set = BuildSet<Set>(keys);

    for (auto _ : state)
    {
        size_t hits = 0;
        for (const Key& key : lookups)
        {
            hits += Ops::Contains(set, key) ? 1u : 0u;
        }
        benchmark::DoNotOptimize(hits);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Erase every key from a full set. Each iteration needs a fresh copy, which is excluded from the
// measurement by timing the erase loop manually. Registered with UseManualTime.
template <typename Set>
void BM_Erase(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    using Clock = std::chrono::steady_clock;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> order = Shuffled(keys, k_shuffle_sequence);
    const Set source = BuildSet<Set>(keys);

    for (auto _ : state)
    {
        Set set = Ops::Copy(source);

        const auto start = Clock::now();
        for (const Key& key : order)
        {
            Ops::Erase(set, key);
        }
        const auto end = Clock::now();

        benchmark::DoNotOptimize(set);
        state.SetIterationTime(std::chrono::duration<double>(end - start).count());
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Copy a set of N keys. Includes allocation, the copy itself, and destruction of the copy.
template <typename Set>
void BM_Copy(benchmark::State& state)
{
    using Ops = SetOps<Set>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const Set source = BuildSet<Set>(keys);

    for (auto _ : state)
    {
        Set copy = Ops::Copy(source);
        benchmark::DoNotOptimize(copy);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Walk every key in a set of N keys.
template <typename Set>
void BM_Iterate(benchmark::State& state)
{
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const Set set = BuildSet<Set>(keys);

    for (auto _ : state)
    {
        Opal::u64 sum = 0;
        for (const Key& key : set)
        {
            sum += key;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// 1K, 8K, 64K, 512K, 4M, 16M keys: L1, L2, L3, and DRAM regimes for both layouts.
void SizeSweep(benchmark::Benchmark* b)
{
    for (const int shift : {10, 13, 16, 19, 22, 24})
    {
        b->Arg(int64_t{1} << shift);
    }
}

void SizeSweepManualTime(benchmark::Benchmark* b)
{
    SizeSweep(b);
    b->UseManualTime();
}

}  // namespace

// Names come out as "<operation>/<Opal|Std>/<size>", so Google Benchmark's tools/compare.py can pair them with:
//   compare.py filters results.json "/Opal/" "/Std/"
#define SET_BENCHMARK(fn, name, configure)                                              \
    BENCHMARK_TEMPLATE(fn, Opal::HashSet<Key>)->Name(name "/Opal")->Apply(configure);  \
    BENCHMARK_TEMPLATE(fn, std::unordered_set<Key>)->Name(name "/Std")->Apply(configure)

SET_BENCHMARK(BM_InsertGrow, "InsertGrow", SizeSweep);
SET_BENCHMARK(BM_InsertReserved, "InsertReserved", SizeSweep);
SET_BENCHMARK(BM_FindHit, "FindHit", SizeSweep);
SET_BENCHMARK(BM_FindMiss, "FindMiss", SizeSweep);
SET_BENCHMARK(BM_Erase, "Erase", SizeSweepManualTime);
SET_BENCHMARK(BM_Copy, "Copy", SizeSweep);
SET_BENCHMARK(BM_Iterate, "Iterate", SizeSweep);

BENCHMARK_MAIN();

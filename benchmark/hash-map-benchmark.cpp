// Compares Opal::HashMap against std::unordered_map on the operations that matter for a map:
// insert (growing and pre-reserved), lookup (a hit that reads the value, and a miss), erase, copy,
// and iteration over the values.
//
// Same shape as hash-set-benchmark.cpp: every benchmark body is written once as a template over the
// map type, so both sides do exactly the same work and only the container differs.
//
// See docs/benchmarks.md for how to build, run and compare the results.

#include <benchmark/benchmark.h>

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "opal/container/hash-map.h"

#include "benchmark-helpers.h"

namespace
{

using namespace Bench;

using Value = Opal::u32;

/**
 * Uniform interface over the two map types so every benchmark body is written once.
 */
template <typename Map>
struct MapOps;

template <typename K, typename V>
struct MapOps<Opal::HashMap<K, V>>
{
    using Map = Opal::HashMap<K, V>;

    static Map Make() { return Map(); }
    static void Reserve(Map& map, size_t count) { map.Reserve(count); }
    static void Insert(Map& map, const K& key, const V& value) { map.Insert(key, value); }
    static void Erase(Map& map, const K& key) { map.Erase(key); }
    static bool Contains(const Map& map, const K& key) { return map.Contains(key); }
    static V Lookup(const Map& map, const K& key)
    {
        const auto it = map.Find(key);
        return it != map.end() ? it.GetValue() : V{};
    }
    static Map Copy(const Map& map) { return map.Clone(); }
    static size_t Size(const Map& map) { return map.GetSize(); }
    template <typename Pair>
    static V ValueOf(const Pair& pair)
    {
        return pair.value;
    }
};

template <typename K, typename V>
struct MapOps<std::unordered_map<K, V>>
{
    using Map = std::unordered_map<K, V>;

    static Map Make() { return Map(); }
    static void Reserve(Map& map, size_t count) { map.reserve(count); }
    // Insert on the Opal side overwrites an existing key, which is insert_or_assign rather than insert.
    static void Insert(Map& map, const K& key, const V& value) { map.insert_or_assign(key, value); }
    static void Erase(Map& map, const K& key) { map.erase(key); }
    static bool Contains(const Map& map, const K& key) { return map.find(key) != map.end(); }
    static V Lookup(const Map& map, const K& key)
    {
        const auto it = map.find(key);
        return it != map.end() ? it->second : V{};
    }
    static Map Copy(const Map& map) { return Map(map); }
    static size_t Size(const Map& map) { return map.size(); }
    template <typename Pair>
    static V ValueOf(const Pair& pair)
    {
        return pair.second;
    }
};

Value ValueFor(Key key)
{
    return key ^ 0x5bd1e995u;
}

template <typename Map>
Map BuildMap(const std::vector<Key>& keys)
{
    using Ops = MapOps<Map>;
    Map map = Ops::Make();
    Ops::Reserve(map, keys.size());
    for (const Key& key : keys)
    {
        Ops::Insert(map, key, ValueFor(key));
    }
    return map;
}

// Insert N pairs into an empty map, letting it grow on its own. Includes every rehash and the final destruction.
template <typename Map>
void BM_InsertGrow(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);

    for (auto _ : state)
    {
        Map map = Ops::Make();
        for (const Key& key : keys)
        {
            Ops::Insert(map, key, ValueFor(key));
        }
        benchmark::DoNotOptimize(map);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Insert N pairs into a map that has already reserved room for them. Isolates the probe and write cost from growth.
template <typename Map>
void BM_InsertReserved(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);

    for (auto _ : state)
    {
        Map map = Ops::Make();
        Ops::Reserve(map, count);
        for (const Key& key : keys)
        {
            Ops::Insert(map, key, ValueFor(key));
        }
        benchmark::DoNotOptimize(map);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Look up N keys that are all present, in an order different from insertion, and read each value.
template <typename Map>
void BM_FindHit(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> lookups = Shuffled(keys, k_shuffle_sequence);
    const Map map = BuildMap<Map>(keys);

    for (auto _ : state)
    {
        Opal::u64 sum = 0;
        for (const Key& key : lookups)
        {
            sum += Ops::Lookup(map, key);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Look up N keys that are (almost certainly) absent. Exercises the full probe sequence to a miss.
template <typename Map>
void BM_FindMiss(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> lookups = MakeKeys(count, k_miss_sequence);
    const Map map = BuildMap<Map>(keys);

    for (auto _ : state)
    {
        size_t hits = 0;
        for (const Key& key : lookups)
        {
            hits += Ops::Contains(map, key) ? 1u : 0u;
        }
        benchmark::DoNotOptimize(hits);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Erase every key from a full map. Each iteration needs a fresh copy, which is excluded from the
// measurement by timing the erase loop manually. Registered with UseManualTime.
template <typename Map>
void BM_Erase(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    using Clock = std::chrono::steady_clock;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const std::vector<Key> order = Shuffled(keys, k_shuffle_sequence);
    const Map source = BuildMap<Map>(keys);

    for (auto _ : state)
    {
        Map map = Ops::Copy(source);

        const auto start = Clock::now();
        for (const Key& key : order)
        {
            Ops::Erase(map, key);
        }
        const auto end = Clock::now();

        benchmark::DoNotOptimize(map);
        state.SetIterationTime(std::chrono::duration<double>(end - start).count());
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Copy a map of N pairs. Includes allocation, the copy itself, and destruction of the copy.
template <typename Map>
void BM_Copy(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const Map source = BuildMap<Map>(keys);

    for (auto _ : state)
    {
        Map copy = Ops::Copy(source);
        benchmark::DoNotOptimize(copy);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

// Walk every pair in a map of N pairs and read the values.
template <typename Map>
void BM_Iterate(benchmark::State& state)
{
    using Ops = MapOps<Map>;
    const size_t count = static_cast<size_t>(state.range(0));
    const std::vector<Key> keys = MakeKeys(count, k_keys_sequence);
    const Map map = BuildMap<Map>(keys);

    for (auto _ : state)
    {
        Opal::u64 sum = 0;
        for (const auto& pair : map)
        {
            sum += Ops::ValueOf(pair);
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

}  // namespace

// Names come out as "HashMap/<operation>/<Opal|Std>/<size>", so the compare.py that ships with Google Benchmark can pair them with:
//   compare.py filters results.json "/Opal/" "/Std/"
#define MAP_BENCHMARK(fn, name, configure)                                                                \
    BENCHMARK_TEMPLATE(fn, Opal::HashMap<Key, Value>)->Name("HashMap/" name "/Opal")->Apply(configure);  \
    BENCHMARK_TEMPLATE(fn, std::unordered_map<Key, Value>)->Name("HashMap/" name "/Std")->Apply(configure)

MAP_BENCHMARK(BM_InsertGrow, "InsertGrow", SizeSweep);
MAP_BENCHMARK(BM_InsertReserved, "InsertReserved", SizeSweep);
MAP_BENCHMARK(BM_FindHit, "FindHit", SizeSweep);
MAP_BENCHMARK(BM_FindMiss, "FindMiss", SizeSweep);
MAP_BENCHMARK(BM_Erase, "Erase", SizeSweepManualTime);
MAP_BENCHMARK(BM_Copy, "Copy", SizeSweep);
MAP_BENCHMARK(BM_Iterate, "Iterate", SizeSweep);

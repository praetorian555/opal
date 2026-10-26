#pragma once

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "opal/defines.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <sched.h>
#endif

#include "opal/threading/thread.h"
#include "opal/type-traits.h"
#include "opal/types.h"

// Shared by the threaded benchmarks: the cache topology, and pinning threads to cores so a result does not depend on where the
// scheduler happened to put them. See "Threaded benchmarks" in docs/benchmarks.md.
namespace Bench
{

/**
 * Logical processors grouped by the L3 cache they share, sorted, with one logical processor per physical core so no two pinned
 * threads are SMT siblings. On a Zen CPU each group is one CCD. Empty if the topology could not be read.
 */
using CacheDomains = std::vector<std::vector<Opal::u32>>;

inline CacheDomains ReadCacheDomains()
{
    const Opal::Expected<Opal::CpuInfo, Opal::ErrorCode> cpu_info = Opal::GetCpuInfo();
    if (!cpu_info.HasValue())
    {
        return {};
    }
    const Opal::CpuInfo& info = cpu_info.GetValue();
    // A machine that reports no L3 lands every core in one domain.
    CacheDomains domains(info.l3_domains.IsEmpty() ? 1 : info.l3_domains.GetSize());
    for (const Opal::PhysicalCoreInfo& core : info.physical_processors)
    {
        const Opal::u32 domain = info.l3_domains.IsEmpty() ? 0 : core.l3_domain;
        if (core.logical_cores && domain < domains.size())
        {
            domains[domain].push_back(core.logical_cores.GetLowestSetBitIndex());
        }
    }
    std::erase_if(domains, [](const std::vector<Opal::u32>& domain) { return domain.empty(); });
    return domains;
}

inline const CacheDomains& GetCacheDomains()
{
    static const CacheDomains s_domains = []
    {
        CacheDomains domains = ReadCacheDomains();
        for (std::vector<Opal::u32>& domain : domains)
        {
            std::sort(domain.begin(), domain.end());
        }
        std::sort(domains.begin(), domains.end());
        return domains;
    }();
    return s_domains;
}

// Record the domains in the benchmark context, so every result says which cores its placements used.
inline const bool k_cache_domains_reported = []
{
    std::string text;
    for (const std::vector<Opal::u32>& domain : GetCacheDomains())
    {
        text += text.empty() ? "[" : " [";
        for (size_t i = 0; i < domain.size(); i++)
        {
            text += (i == 0 ? "" : ",") + std::to_string(domain[i]);
        }
        text += "]";
    }
    benchmark::AddCustomContext("l3_domains", text.empty() ? "unknown" : text);
    return true;
}();

/** Where a threaded benchmark pins its threads. Every thread gets a physical core of its own. */
enum class Placement
{
    SameL3,   // All threads share one L3, so a handoff stays inside one CCD.
    CrossL3,  // Producers on one L3 and consumers on another, so every handoff crosses between CCDs.
};

/**
 * Pick a logical processor for each thread.
 * @return Processors for the producers followed by the consumers, or empty when this machine cannot provide the placement.
 */
inline std::vector<Opal::u32> AssignCores(Placement placement, size_t producer_count, size_t consumer_count)
{
    const CacheDomains& domains = GetCacheDomains();
    std::vector<Opal::u32> cores;
    if (placement == Placement::SameL3)
    {
        if (domains.empty() || domains[0].size() < producer_count + consumer_count)
        {
            return {};
        }
        cores.assign(domains[0].begin(), domains[0].begin() + static_cast<std::ptrdiff_t>(producer_count + consumer_count));
    }
    else
    {
        if (domains.size() < 2 || domains[0].size() < producer_count || domains[1].size() < consumer_count)
        {
            return {};
        }
        cores.assign(domains[0].begin(), domains[0].begin() + static_cast<std::ptrdiff_t>(producer_count));
        cores.insert(cores.end(), domains[1].begin(), domains[1].begin() + static_cast<std::ptrdiff_t>(consumer_count));
    }
    return cores;
}

/** Pin the calling thread to one logical processor. */
inline void PinCurrentThread(Opal::u32 core)
{
    Opal::SetThreadAffinity(Opal::GetCurrentThreadHandle(), core);
}

/**
 * Pins the calling thread to one logical processor and gives it back the processors it had when the scope ends. For the benchmark
 * thread, which goes on to run every other benchmark.
 */
class ScopedThreadPin
{
public:
    explicit ScopedThreadPin(Opal::u32 core)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        m_previous = SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR{1} << core);
#elif defined(OPAL_PLATFORM_LINUX)
        m_has_previous = sched_getaffinity(0, sizeof(m_previous), &m_previous) == 0;
        PinCurrentThread(core);
#else
        // macOS does not let a thread be pinned.
        (void)core;
#endif
    }

    ~ScopedThreadPin()
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        if (m_previous != 0)
        {
            SetThreadAffinityMask(GetCurrentThread(), m_previous);
        }
#elif defined(OPAL_PLATFORM_LINUX)
        if (m_has_previous)
        {
            sched_setaffinity(0, sizeof(m_previous), &m_previous);
        }
#endif
    }

    ScopedThreadPin(const ScopedThreadPin&) = delete;
    ScopedThreadPin& operator=(const ScopedThreadPin&) = delete;

private:
#if defined(OPAL_PLATFORM_WINDOWS)
    DWORD_PTR m_previous = 0;
#elif defined(OPAL_PLATFORM_LINUX)
    cpu_set_t m_previous{};
    bool m_has_previous = false;
#endif
};

}  // namespace Bench

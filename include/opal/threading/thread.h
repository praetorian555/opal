#pragma once

#include <thread>
#include <tuple>
#include <type_traits>

#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"

namespace Opal
{

namespace Impl
{

struct ThreadDataBase
{
    AllocatorBase* allocator = nullptr;

    ThreadDataBase(AllocatorBase* a) : allocator(a) {}
    virtual ~ThreadDataBase() = default;
    virtual void Invoke() = 0;
};

template <typename Function, typename... Args>
struct ThreadData : ThreadDataBase
{
    Function function;
    std::tuple<std::decay_t<Args>...> args;

    explicit ThreadData(AllocatorBase* alloc, Function&& f, Args&&... a)
        : ThreadDataBase(alloc), function(std::forward<Function>(f)), args(std::forward<Args>(a)...)
    {
    }

    void Invoke() override { std::apply(function, std::move(args)); }
};

}  // namespace Impl

struct ThreadHandle
{
    void* native_handle = nullptr;
    void* native_id = nullptr;

    bool operator==(const ThreadHandle& other) const;
};

namespace Impl
{
ThreadHandle CreateThread(ThreadDataBase* data);
}  // namespace Impl

/**
 * Creates and starts a new thread using the @p function for a body and @p for function arguments.
 * @tparam Function Type of function used for a thread body.
 * @tparam Args Variable list of types of the function arguments.
 * @param function Function used for the thread body.
 * @param args Arguments passed to the function.
 * @return Returns a thread handle in case of a success.
 * @throw Exception when thread creation fails.
 */
template <typename Function, typename... Args>
ThreadHandle CreateThread(Function&& function, Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    OPAL_ASSERT(allocator->IsThreadSafe(), "Allocator must be thread safe!");

    Impl::ThreadDataBase* data =
        Opal::New<Impl::ThreadData<Function, Args...>>(allocator, allocator, std::forward<Function>(function), std::forward<Args>(args)...);
    ThreadHandle handle = Impl::CreateThread(data);
    if (handle.native_handle == nullptr)
    {
        Delete(allocator, data);
    }
    return handle;
}

/**
 * Wait for a thread to exit.
 * @param handle Handle of a thread to wait for.
 */
void JoinThread(ThreadHandle handle);

/**
 * Get a thread handle of a current thread.
 * @return Thread handle.
 */
ThreadHandle GetCurrentThreadHandle();

struct PhysicalCoreInfo
{
    u32 id = 0;
    BitMask<u64> logical_cores;
    bool is_hyperthreaded = false;

    [[nodiscard]] bool operator==(const PhysicalCoreInfo& other) const { return id == other.id; }
};

struct CpuInfo
{
    u32 logical_cores_count = 0;
    DynamicArray<PhysicalCoreInfo> physical_processors;
};

/**
 * Get info about the physical and logical cores of the processor. It returns the total number of logical
 * cores in the system, as well as a list of physical cores. Each physical core has a bit mask representing
 * logical cores that belong to it. Logical cores are indexed starting from 0.
 *
 * On Windows, this uses GetLogicalProcessorInformationEx to enumerate processor cores and their
 * SMT relationships. On Linux, this reads per-CPU topology files from sysfs
 * (/sys/devices/system/cpu/cpuN/topology/) and groups logical CPUs by their (package_id, core_id) pair.
 *
 * @return CpuInfo containing the logical core count and an array of PhysicalCoreInfo entries.
 * @note Limited to 64 logical cores due to BitMask<u64>.
 */
CpuInfo GetCpuInfo();

/**
 * Calls GetCpuInfo and prints all the gathered data using the logger with the "General" category.
 */
void PrintCpuInfo();

/**
 * Pin the thread to a specific logical core.
 * @param handle Handle to a thread to pin.
 * @param logical_core_id Logical core id. Values always start from 0 and go to number of logical
 * cores minus one.
 */
void SetThreadAffinity(ThreadHandle handle, u32 logical_core_id);

}  // namespace Opal

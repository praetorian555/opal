#pragma once

#include "opal/threading/atomic.h"
#include <thread>
#include <tuple>
#include <type_traits>

#include "opal/allocator.h"
#include "opal/bit.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"

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

using ThreadId = u64;
static constexpr ThreadId k_invalid_thread_id = 0;

struct ThreadHandle
{
    void* native_handle = nullptr;
    ThreadId id = k_invalid_thread_id;

    bool operator==(const ThreadHandle& other) const;
};

/**
 * Get the thread id from a thread handle. This is a numeric value that can be printed and compared.
 * On Windows, this is the value returned by GetThreadId / GetCurrentThreadId.
 * On Linux, this is the value returned by gettid.
 * @param handle Thread handle.
 * @return Thread id.
 */
ThreadId GetThreadId(const ThreadHandle& handle);

namespace Impl
{
/** Takes ownership of @p data. Destroys it when the thread does not start, since nothing else will. */
Expected<ThreadHandle, ErrorCode> CreateThread(ThreadDataBase* data);
}  // namespace Impl

/**
 * Creates and starts a new thread using the @p function for a body and @p for function arguments.
 * @tparam Function Type of function used for a thread body.
 * @tparam Args Variable list of types of the function arguments.
 * @param function Function used for the thread body.
 * @param args Arguments passed to the function.
 * @note The default allocator of the calling thread must be thread-safe, and becomes the default allocator of the new thread.
 * @return Handle of the started thread, ErrorCode::OutOfMemory when the thread state could not be allocated, or ErrorCode::OSFailure
 *         when the OS refused to start the thread.
 */
template <typename Function, typename... Args>
[[nodiscard]] Expected<ThreadHandle, ErrorCode> CreateThread(Function&& function, Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    OPAL_ASSERT(allocator->IsThreadSafe(), "Allocator must be thread safe!");

    Impl::ThreadDataBase* data =
        Opal::New<Impl::ThreadData<Function, Args...>>(allocator, allocator, std::forward<Function>(function), std::forward<Args>(args)...);
    if (data == nullptr) [[unlikely]]
    {
        return Expected<ThreadHandle, ErrorCode>(ErrorCode::OutOfMemory);
    }
    return Impl::CreateThread(data);
}

/**
 * Wait for a thread to exit.
 * @param handle Handle of a thread to wait for.
 */
void JoinThread(ThreadHandle handle);

/**
 * Detach a thread, allowing it to run independently. The thread will clean up its own resources
 * when it finishes. After detaching, the handle should not be used with JoinThread.
 * @param handle Handle of a thread to detach.
 */
void DetachThread(ThreadHandle handle);

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
 * @return CpuInfo containing the logical core count and an array of PhysicalCoreInfo entries, ErrorCode::OutOfMemory when the result
 *         could not be grown, or ErrorCode::OSFailure when the topology could not be read.
 * @note Limited to 64 logical cores due to BitMask<u64>.
 */
[[nodiscard]] Expected<CpuInfo, ErrorCode> GetCpuInfo();

/**
 * Calls GetCpuInfo and prints all the gathered data using the logger with the "General" category.
 * @return Whatever GetCpuInfo reported. Nothing is printed when it failed.
 */
ErrorCode PrintCpuInfo();

/**
 * Pin the thread to a specific logical core.
 * @param handle Handle to a thread to pin.
 * @param logical_core_id Logical core id. Values always start from 0 and go to number of logical
 * cores minus one.
 */
void SetThreadAffinity(ThreadHandle handle, u32 logical_core_id);

}  // namespace Opal

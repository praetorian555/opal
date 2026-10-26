#include "opal/threading/thread.h"

#include "opal/container/scope-ptr.h"
#include "opal/container/string.h"
#include "opal/logging.h"
#include "opal/threading/cpu-pause.h"

#include <stdio.h>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_POSIX)
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#if defined(OPAL_PLATFORM_MACOS)
#include <sys/sysctl.h>
#else
#include <sys/syscall.h>
#endif

#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#if defined(OPAL_PLATFORM_WINDOWS)
DWORD WINAPI WindowsThread(LPVOID param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    Opal::PushDefaultAllocator(data->allocator);
    data->Invoke();
    Delete(data->allocator, data);
    return 0;
}
#elif defined(OPAL_PLATFORM_POSIX)
Opal::u64 GetCurrentOsThreadId()
{
#if defined(OPAL_PLATFORM_MACOS)
    Opal::u64 id = 0;
    pthread_threadid_np(nullptr, &id);
    return id;
#else
    return static_cast<Opal::u64>(syscall(SYS_gettid));
#endif
}

// Handed to a new thread so it can report its kernel id back. Lives on the frame of the launcher rather than in the thread's own data,
// which the thread frees as it finishes and which the launcher would otherwise still be reading.
struct ThreadLaunch
{
    Opal::Impl::ThreadDataBase* data = nullptr;
    Opal::Atomic<Opal::u64> thread_id{0};
};

void* ThreadFunction(void* param)
{
    ThreadLaunch* launch = static_cast<ThreadLaunch*>(param);
    Opal::Impl::ThreadDataBase* data = launch->data;
    // Everything needed from `launch` is read above, because publishing the id releases the launcher, and its frame goes with it.
    launch->thread_id.Store<Opal::MemoryOrder::Release>(GetCurrentOsThreadId());
    launch = nullptr;
    Opal::PushDefaultAllocator(data->allocator);
    data->Invoke();
    Delete(data->allocator, data);
    return nullptr;
}
#endif

Opal::Expected<Opal::ThreadHandle, Opal::ErrorCode> Opal::Impl::CreateThread(ThreadDataBase* data)
{
    using Result = Expected<ThreadHandle, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    DWORD thread_id = 0;
    HANDLE thread_handle = ::CreateThread(nullptr, 0, WindowsThread, data, 0, &thread_id);
    if (thread_handle == nullptr)
    {
        // The thread that would have freed this never started.
        Delete(data->allocator, data);
        return Result(ErrorCode::OSFailure);
    }
    return Result(ThreadHandle{.native_handle = thread_handle, .id = static_cast<u64>(thread_id)});
#elif defined(OPAL_PLATFORM_POSIX)
    ThreadLaunch launch;
    launch.data = data;
    pthread_t native_handle;
    if (pthread_create(&native_handle, nullptr, ThreadFunction, &launch) != 0)
    {
        Delete(data->allocator, data);
        return Result(ErrorCode::OSFailure);
    }
    // Spin until the thread has stored its kernel thread id, and keep what it stored. Reading it a second time afterwards is not an
    // option: the thread may have finished and freed its data by then, and `launch` is only alive for as long as this loop.
    u64 thread_id = 0;
    SpinBackoff backoff;
    while ((thread_id = launch.thread_id.Load<MemoryOrder::Acquire>()) == 0)
    {
        backoff.Wait();
    }
    return Result(ThreadHandle{.native_handle = reinterpret_cast<void*>(native_handle), .id = thread_id});
#else
#error "Platform not supported"
#endif
}

bool Opal::ThreadHandle::operator==(const ThreadHandle& other) const
{
    return id == other.id;
}

Opal::ThreadId Opal::GetThreadId(const ThreadHandle& handle)
{
    return handle.id;
}

void Opal::JoinThread(ThreadHandle handle)
{
    if (handle.native_handle != nullptr)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        ::WaitForSingleObject(handle.native_handle, INFINITE);
        ::CloseHandle(handle.native_handle);
#elif defined(OPAL_PLATFORM_POSIX)
        pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
        pthread_join(native_handle, nullptr);
#else
#error "Platform not supported"
#endif
    }
}

void Opal::DetachThread(ThreadHandle handle)
{
    if (handle.native_handle != nullptr)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        ::CloseHandle(handle.native_handle);
#elif defined(OPAL_PLATFORM_POSIX)
        pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
        pthread_detach(native_handle);
#else
#error "Platform not supported"
#endif
    }
}

Opal::ThreadHandle Opal::GetCurrentThreadHandle()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD thread_id = ::GetCurrentThreadId();
    return {.native_handle = ::GetCurrentThread(), .id = static_cast<u64>(thread_id)};
#elif defined(OPAL_PLATFORM_POSIX)
    pthread_t thread_handle = pthread_self();
    const u64 tid = GetCurrentOsThreadId();
    return {.native_handle = reinterpret_cast<void*>(thread_handle), .id = tid};
#else
#error "Platform not supported"
#endif
}

void Opal::SleepCurrentThread(u64 milliseconds)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    constexpr u64 k_longest_sleep_ms = INFINITE - 1;
    do
    {
        const u64 sleep_ms = milliseconds < k_longest_sleep_ms ? milliseconds : k_longest_sleep_ms;
        ::Sleep(static_cast<DWORD>(sleep_ms));
        milliseconds -= sleep_ms;
    } while (milliseconds > 0);
#elif defined(OPAL_PLATFORM_POSIX)
    if (milliseconds == 0)
    {
        sched_yield();
        return;
    }
    struct timespec remaining;
    remaining.tv_sec = static_cast<time_t>(milliseconds / 1000);
    remaining.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000000);
    while (nanosleep(&remaining, &remaining) != 0 && errno == EINTR)
    {
    }
#else
#error "Platform not supported"
#endif
}

void Opal::YieldCurrentThread()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    ::SwitchToThread();
#elif defined(OPAL_PLATFORM_POSIX)
    sched_yield();
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::SetThreadName(ThreadHandle handle, StringViewUtf8 name)
{
    if (handle.native_handle == nullptr)
    {
        return ErrorCode::OSFailure;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide wide_name;
    const ErrorCode transcode_result = Transcode(name, wide_name, InvalidSequencePolicy::Replace);
    if (transcode_result != ErrorCode::Success)
    {
        return transcode_result;
    }
    return SUCCEEDED(::SetThreadDescription(handle.native_handle, wide_name.GetData())) ? ErrorCode::Success : ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_POSIX)
    StringUtf8 valid_name;
    const ErrorCode transcode_result = Transcode(name, valid_name, InvalidSequencePolicy::Replace);
    if (transcode_result != ErrorCode::Success)
    {
        return transcode_result;
    }
#if defined(OPAL_PLATFORM_MACOS)
    // macOS names only the calling thread.
    pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
    if (pthread_equal(native_handle, pthread_self()) == 0)
    {
        return ErrorCode::OSFailure;
    }
    constexpr size_t k_longest_name = 63;
#else
    constexpr size_t k_longest_name = 15;
#endif
    size_t size = valid_name.GetSize() < k_longest_name ? valid_name.GetSize() : k_longest_name;
    while (size > 0 && size < valid_name.GetSize() && (static_cast<u8>(valid_name[size]) & 0xC0) == 0x80)
    {
        --size;
    }
    char buffer[k_longest_name + 1] = {};
    memcpy(buffer, valid_name.GetData(), size);
#if defined(OPAL_PLATFORM_MACOS)
    return pthread_setname_np(buffer) == 0 ? ErrorCode::Success : ErrorCode::OSFailure;
#else
    pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
    return pthread_setname_np(native_handle, buffer) == 0 ? ErrorCode::Success : ErrorCode::OSFailure;
#endif
#else
#error "Platform not supported"
#endif
}

namespace
{

/** An L3 cache found while reading the topology, before the domains are numbered. */
struct L3Domain
{
    Opal::u64 logical_cores = 0;
    Opal::u64 size_bytes = 0;
};

/** Record an L3 cache unless one covering the same logical cores is already recorded. */
Opal::ErrorCode AddL3Domain(Opal::DynamicArray<L3Domain>& domains, Opal::u64 logical_cores, Opal::u64 size_bytes)
{
    if (logical_cores == 0)
    {
        return Opal::ErrorCode::Success;
    }
    for (const L3Domain& domain : domains)
    {
        if (domain.logical_cores == logical_cores)
        {
            return Opal::ErrorCode::Success;
        }
    }
    return domains.PushBack({logical_cores, size_bytes});
}

/** Number the domains in order of their lowest logical core, and point every physical core at the one it uses. */
Opal::ErrorCode FinishL3Domains(Opal::CpuInfo& info, Opal::DynamicArray<L3Domain>& domains)
{
    while (!domains.IsEmpty())
    {
        Opal::u64 lowest = 0;
        for (Opal::u64 i = 1; i < domains.GetSize(); ++i)
        {
            if (Opal::CountTrailingZeros(domains[i].logical_cores) < Opal::CountTrailingZeros(domains[lowest].logical_cores))
            {
                lowest = i;
            }
        }
        Opal::CacheDomainInfo domain;
        domain.id = static_cast<Opal::u32>(info.l3_domains.GetSize());
        domain.logical_cores = Opal::BitMask<Opal::u64>(domains[lowest].logical_cores);
        domain.size_bytes = domains[lowest].size_bytes;
        for (Opal::PhysicalCoreInfo& core : info.physical_processors)
        {
            if (core.logical_cores && ((domains[lowest].logical_cores >> core.logical_cores.GetLowestSetBitIndex()) & 1) != 0)
            {
                core.l3_domain = domain.id;
            }
        }
        if (info.l3_domains.PushBack(Opal::Move(domain)) != Opal::ErrorCode::Success) [[unlikely]]
        {
            return Opal::ErrorCode::OutOfMemory;
        }
        domains[lowest] = domains[domains.GetSize() - 1];
        domains.PopBack();
    }
    return Opal::ErrorCode::Success;
}

#if defined(OPAL_PLATFORM_MACOS)
/** Read an integer sysctl. @return 0 if it could not be read. */
Opal::u64 ReadSysctl(const char* name)
{
    Opal::u64 value = 0;
    size_t size = sizeof(value);
    // A 32-bit value fills the low bytes, which on a little-endian machine leaves it read correctly.
    if (sysctlbyname(name, &value, &size, nullptr, 0) != 0)
    {
        return 0;
    }
    return value;
}
#elif defined(OPAL_PLATFORM_LINUX)
/** Read the first line of a sysfs file into buffer. @return False if the file could not be read. */
bool ReadLine(const char* path, char* buffer, int buffer_size)
{
    FILE* file = fopen(path, "r");
    if (file == nullptr)
    {
        return false;
    }
    const bool is_read = fgets(buffer, buffer_size, file) != nullptr;
    fclose(file);
    return is_read;
}

/** Parse a CPU list such as "0-7,16-23" into a mask. CPUs from 64 up are left out. */
Opal::u64 ParseCpuList(const char* text)
{
    Opal::u64 mask = 0;
    while (*text >= '0' && *text <= '9')
    {
        char* rest = nullptr;
        const unsigned long first = strtoul(text, &rest, 10);
        unsigned long last = first;
        if (*rest == '-')
        {
            last = strtoul(rest + 1, &rest, 10);
        }
        for (unsigned long cpu = first; cpu <= last && cpu < 64; ++cpu)
        {
            mask |= Opal::u64{1} << cpu;
        }
        text = *rest == ',' ? rest + 1 : rest;
    }
    return mask;
}

/** Parse a cache size such as "32768K" into bytes. */
Opal::u64 ParseCacheSize(const char* text)
{
    char* rest = nullptr;
    const Opal::u64 size = strtoull(text, &rest, 10);
    switch (*rest)
    {
        case 'K':
            return size * 1024;
        case 'M':
            return size * 1024 * 1024;
        case 'G':
            return size * 1024 * 1024 * 1024;
        default:
            return size;
    }
}
#endif

}  // namespace

Opal::Expected<Opal::CpuInfo, Opal::ErrorCode> Opal::GetCpuInfo()
{
    using Result = Expected<CpuInfo, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    CpuInfo info;
    DWORD buffer_size = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = NULL;
    GetLogicalProcessorInformationEx(RelationAll, NULL, &buffer_size);
    buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(GetDefaultAllocator()->Alloc(buffer_size, 1));
    if (buffer == nullptr)
    {
        return Result(ErrorCode::OutOfMemory);
    }
    if (GetLogicalProcessorInformationEx(RelationAll, buffer, &buffer_size) == FALSE)
    {
        GetDefaultAllocator()->Free(buffer);
        return Result(ErrorCode::OSFailure);
    }
    BYTE* ptr = reinterpret_cast<BYTE*>(buffer);
    const BYTE* end = ptr + buffer_size;
    DynamicArray<L3Domain> l3_domains;

    while (ptr < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX lp_info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);
        switch (lp_info->Relationship)
        {
            case RelationProcessorCore:
            {
                PhysicalCoreInfo pp_info;
                pp_info.id = static_cast<u32>(info.physical_processors.GetSize());
                pp_info.logical_cores = BitMask<u64>(lp_info->Processor.GroupMask[0].Mask);
                pp_info.is_hyperthreaded = lp_info->Processor.Flags == LTP_PC_SMT;
                info.logical_cores_count += pp_info.logical_cores.GetSetBitCount();
                if (info.physical_processors.PushBack(std::move(pp_info)) != ErrorCode::Success) [[unlikely]]
                {
                    GetDefaultAllocator()->Free(buffer);
                    return Result(ErrorCode::OutOfMemory);
                }
                break;
            }
            case RelationCache:
            {
                // Only processor group 0, as for the cores.
                const CACHE_RELATIONSHIP& cache = lp_info->Cache;
                if (cache.Level == 3 && cache.GroupMask.Group == 0 &&
                    AddL3Domain(l3_domains, cache.GroupMask.Mask, cache.CacheSize) != ErrorCode::Success) [[unlikely]]
                {
                    GetDefaultAllocator()->Free(buffer);
                    return Result(ErrorCode::OutOfMemory);
                }
                break;
            }
            default:
            {
                // Ignore other info
                break;
            }
        }
        ptr += lp_info->Size;
    }

    GetDefaultAllocator()->Free(buffer);
    if (FinishL3Domains(info, l3_domains) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }
    return Result(Move(info));
#elif defined(OPAL_PLATFORM_MACOS)
    // macOS reports counts, not which logical cores share a physical one, so they are numbered in order: each physical core takes
    // the next logical_count / physical_count logical cores.
    CpuInfo info;
    u64 logical_count = ReadSysctl("hw.logicalcpu");
    const u64 physical_count = ReadSysctl("hw.physicalcpu");
    if (logical_count == 0 || physical_count == 0)
    {
        return Result(ErrorCode::OSFailure);
    }
    logical_count = logical_count < 64 ? logical_count : 64;
    const u64 per_core = logical_count / physical_count > 0 ? logical_count / physical_count : 1;
    u64 all_cores = 0;
    for (u64 first = 0; first + per_core <= logical_count; first += per_core)
    {
        PhysicalCoreInfo pp_info;
        pp_info.id = static_cast<u32>(info.physical_processors.GetSize());
        const u64 mask = (per_core == 64 ? ~u64{0} : ((u64{1} << per_core) - 1)) << first;
        pp_info.logical_cores = BitMask<u64>(mask);
        pp_info.is_hyperthreaded = per_core > 1;
        info.logical_cores_count += static_cast<u32>(per_core);
        all_cores |= mask;
        if (info.physical_processors.PushBack(std::move(pp_info)) != ErrorCode::Success) [[unlikely]]
        {
            return Result(ErrorCode::OutOfMemory);
        }
    }
    // Apple silicon has no L3; an Intel Mac has one, shared by every core.
    DynamicArray<L3Domain> l3_domains;
    const u64 l3_size = ReadSysctl("hw.l3cachesize");
    if (l3_size > 0 && AddL3Domain(l3_domains, all_cores, l3_size) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }
    if (FinishL3Domains(info, l3_domains) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }
    return Result(Move(info));
#elif defined(OPAL_PLATFORM_LINUX)
    CpuInfo info;

    DIR* cpu_dir = opendir("/sys/devices/system/cpu");
    if (cpu_dir == nullptr)
    {
        return Result(ErrorCode::OSFailure);
    }

    struct CoreAccum
    {
        u32 package_id;
        u32 core_id;
        u64 mask;
    };
    DynamicArray<CoreAccum> accums;
    DynamicArray<L3Domain> l3_domains;

    struct dirent* entry;
    while ((entry = readdir(cpu_dir)) != nullptr)
    {
        // Looking for directories which names start with cpu<number>, these are logical cores
        if (strncmp(entry->d_name, "cpu", 3) != 0 || entry->d_name[3] < '0' || entry->d_name[3] > '9')
        {
            continue;
        }

        // Extract logical core id from the directory name
        u32 logical_id = static_cast<u32>(atoi(entry->d_name + 3));
        if (logical_id >= 64)
        {
            continue;
        }

        char path[310];
        char buf[32];

        // Extract physical core id inside the package
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/topology/core_id", entry->d_name);
        FILE* f = fopen(path, "r");
        if (f == nullptr)
        {
            continue;
        }
        if (fgets(buf, sizeof(buf), f) == nullptr)
        {
            fclose(f);
            continue;
        }
        fclose(f);
        u32 core_id = static_cast<u32>(atoi(buf));

        // Extract package id
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/topology/physical_package_id", entry->d_name);
        f = fopen(path, "r");
        if (f == nullptr)
        {
            continue;
        }
        if (fgets(buf, sizeof(buf), f) == nullptr)
        {
            fclose(f);
            continue;
        }
        fclose(f);
        u32 package_id = static_cast<u32>(atoi(buf));

        // The L3 this logical core uses, if sysfs reports one. The index directories are not numbered by level.
        for (u32 index = 0; index < 16; ++index)
        {
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/cache/index%u/level", entry->d_name, index);
            if (!ReadLine(path, buf, sizeof(buf)))
            {
                break;
            }
            if (atoi(buf) != 3)
            {
                continue;
            }
            char list[512];
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/cache/index%u/shared_cpu_list", entry->d_name, index);
            if (!ReadLine(path, list, sizeof(list)))
            {
                break;
            }
            u64 size_bytes = 0;
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/cache/index%u/size", entry->d_name, index);
            if (ReadLine(path, buf, sizeof(buf)))
            {
                size_bytes = ParseCacheSize(buf);
            }
            if (AddL3Domain(l3_domains, ParseCpuList(list), size_bytes) != ErrorCode::Success) [[unlikely]]
            {
                closedir(cpu_dir);
                return Result(ErrorCode::OutOfMemory);
            }
            break;
        }

        bool found = false;
        for (u64 i = 0; i < accums.GetSize(); ++i)
        {
            if (accums[i].package_id == package_id && accums[i].core_id == core_id)
            {
                accums[i].mask |= (1ULL << logical_id);
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (accums.PushBack({package_id, core_id, 1ULL << logical_id}) != ErrorCode::Success) [[unlikely]]
            {
                closedir(cpu_dir);
                return Result(ErrorCode::OutOfMemory);
            }
        }
    }
    closedir(cpu_dir);

    for (u64 i = 0; i < accums.GetSize(); ++i)
    {
        PhysicalCoreInfo pp_info;
        pp_info.id = static_cast<u32>(i);
        pp_info.logical_cores = BitMask<u64>(accums[i].mask);
        pp_info.is_hyperthreaded = pp_info.logical_cores.GetSetBitCount() > 1;
        info.logical_cores_count += pp_info.logical_cores.GetSetBitCount();
        if (info.physical_processors.PushBack(std::move(pp_info)) != ErrorCode::Success) [[unlikely]]
        {
            return Result(ErrorCode::OutOfMemory);
        }
    }

    if (FinishL3Domains(info, l3_domains) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }
    return Result(Move(info));
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::PrintCpuInfo()
{
    Expected<CpuInfo, ErrorCode> cpu_info = GetCpuInfo();
    if (!cpu_info.HasValue())
    {
        return cpu_info.GetError();
    }
    const CpuInfo& info = cpu_info.GetValue();
    Logger& logger = GetLogger();
    logger.Info("General", "CPU Info:");
    logger.Info("General", "  Logical cores count: {}", info.logical_cores_count);
    logger.Info("General", "  Physical cores count: {}", static_cast<u32>(info.physical_processors.GetSize()));
    for (u64 i = 0; i < info.physical_processors.GetSize(); ++i)
    {
        const PhysicalCoreInfo& core = info.physical_processors[i];
        logger.Info("General", "  Physical core {}:", core.id);
        logger.Info("General", "    Hyperthreaded: {}", core.is_hyperthreaded ? "yes" : "no");
        char logical_cores_str[300] = {};
        size_t offset = 0;
        for (const u32 bit : core.logical_cores)
        {
            const size_t max_length = sizeof(logical_cores_str) - offset;
            offset += static_cast<u32>(snprintf(logical_cores_str + offset, max_length, "%u ", bit));
        }
        logger.Info("General", "    Logical cores: {}", logical_cores_str);
    }
    for (const CacheDomainInfo& domain : info.l3_domains)
    {
        char logical_cores_str[300] = {};
        size_t offset = 0;
        for (const u32 bit : domain.logical_cores)
        {
            const size_t max_length = sizeof(logical_cores_str) - offset;
            offset += static_cast<u32>(snprintf(logical_cores_str + offset, max_length, "%u ", bit));
        }
        logger.Info("General", "  L3 cache {}: {} KB, logical cores: {}", domain.id, domain.size_bytes / 1024, logical_cores_str);
    }
    return ErrorCode::Success;
}

void Opal::SetThreadAffinity(ThreadHandle handle, u32 logical_core_id)
{
    if (handle.native_handle != nullptr)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        const DWORD_PTR mask = 1ULL << logical_core_id;
        SetThreadAffinityMask(handle.native_handle, mask);
#elif defined(OPAL_PLATFORM_MACOS)
        // macOS does not let a thread be pinned to a core.
        (void)logical_core_id;
#elif defined(OPAL_PLATFORM_LINUX)
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(logical_core_id, &cpu_set);
        // By the kernel thread id rather than pthread_setaffinity_np, which Bionic does not have. The id is SYS_gettid's.
        sched_setaffinity(static_cast<pid_t>(handle.id), sizeof(cpu_set), &cpu_set);
#else
#error "Platform not supported"
#endif
    }
}

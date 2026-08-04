#include "opal/threading/thread.h"

#include "opal/container/scope-ptr.h"
#include "opal/logging.h"

#include <cstdio>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
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
#elif defined(OPAL_PLATFORM_LINUX)
// Handed to a new thread so it can report its kernel id back. Lives on the frame of the launcher rather than in the thread's own data,
// which the thread frees as it finishes and which the launcher would otherwise still be reading.
struct ThreadLaunch
{
    Opal::Impl::ThreadDataBase* data = nullptr;
    std::atomic<Opal::u64> thread_id{0};
};

void* ThreadFunction(void* param)
{
    ThreadLaunch* launch = static_cast<ThreadLaunch*>(param);
    Opal::Impl::ThreadDataBase* data = launch->data;
    // Everything needed from `launch` is read above, because publishing the id releases the launcher, and its frame goes with it.
    launch->thread_id.store(static_cast<Opal::u64>(syscall(SYS_gettid)), std::memory_order_release);
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
#elif defined(OPAL_PLATFORM_LINUX)
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
    while ((thread_id = launch.thread_id.load(std::memory_order_acquire)) == 0)
    {
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
#elif defined(OPAL_PLATFORM_LINUX)
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
#elif defined(OPAL_PLATFORM_LINUX)
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
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t thread_handle = pthread_self();
    const u64 tid = static_cast<u64>(syscall(SYS_gettid));
    return {.native_handle = reinterpret_cast<void*>(thread_handle), .id = tid};
#else
#error "Platform not supported"
#endif
}

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
            default:
            {
                // Ignore other info
                break;
            }
        }
        ptr += lp_info->Size;
    }

    GetDefaultAllocator()->Free(buffer);
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
    return ErrorCode::Success;
}

void Opal::SetThreadAffinity(ThreadHandle handle, u32 logical_core_id)
{
    if (handle.native_handle != nullptr)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        const DWORD_PTR mask = 1ULL << logical_core_id;
        SetThreadAffinityMask(handle.native_handle, mask);
#elif defined(OPAL_PLATFORM_LINUX)
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(logical_core_id, &cpu_set);
        pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
        pthread_setaffinity_np(native_handle, sizeof(cpu_set), &cpu_set);
#else
#error "Platform not supported"
#endif
    }
}

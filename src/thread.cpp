#include "opal/threading/thread.h"

#include "opal/container/scope-ptr.h"
#include "opal/logging.h"

#include <cstdio>

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#endif

#if defined(OPAL_PLATFORM_WINDOWS)
DWORD WINAPI WindowsThread(LPVOID param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    data->Invoke();
    Delete(data->allocator, data);
    return 0;
}
#elif defined(OPAL_PLATFORM_LINUX)
void* ThreadFunction(void* param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    data->Invoke();
    Delete(data->allocator, data);
    return nullptr;
}
#endif

Opal::ThreadHandle Opal::Impl::CreateThread(ThreadDataBase* data)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    DWORD thread_id = 0;
    HANDLE thread_handle = ::CreateThread(nullptr, 0, WindowsThread, data, 0, &thread_id);
    if (thread_handle == nullptr)
    {
        throw Exception("Failed to create thread!");
    }
    return {.native_handle = thread_handle, .native_id = reinterpret_cast<void*>(static_cast<u64>(thread_id))};
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t native_handle;
    pthread_create(&native_handle, nullptr, ThreadFunction, data);
    ThreadHandle handle = {.native_handle = reinterpret_cast<void*>(native_handle)};
    return handle;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

bool Opal::ThreadHandle::operator==(const ThreadHandle& other) const
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return native_id == other.native_id;
#elif defined(OPAL_PLATFORM_LINUX)
    return native_handle == other.native_handle;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
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
        throw NotImplementedException(__FUNCTION__);
#endif
    }
}

Opal::ThreadHandle Opal::GetCurrentThreadHandle()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD thread_id = ::GetCurrentThreadId();
    return {.native_id = reinterpret_cast<void*>(static_cast<u64>(thread_id))};
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t thread_handle = pthread_self();
    return {.native_handle = reinterpret_cast<void*>(thread_handle)};
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

Opal::CpuInfo Opal::GetCpuInfo()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    CpuInfo info;
    AllocatorBase* scratch_allocator = GetScratchAllocator();
    DWORD buffer_size = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = NULL;
    GetLogicalProcessorInformationEx(RelationAll, NULL, &buffer_size);
    buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(scratch_allocator->Alloc(buffer_size, 1));
    if (GetLogicalProcessorInformationEx(RelationAll, buffer, &buffer_size) == FALSE)
    {
        scratch_allocator->Free(buffer);
        return info;
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
                info.physical_processors.PushBack(pp_info);
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

    scratch_allocator->Free(buffer);
    return info;
#elif defined(OPAL_PLATFORM_LINUX)
    CpuInfo info;

    DIR* cpu_dir = opendir("/sys/devices/system/cpu");
    if (cpu_dir == nullptr)
    {
        return info;
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

        char path[256];
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
            accums.PushBack({package_id, core_id, 1ULL << logical_id});
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
        info.physical_processors.PushBack(pp_info);
    }

    return info;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::PrintCpuInfo()
{
    const CpuInfo info = GetCpuInfo();
    Logger& logger = GetLogger();
    logger.Info("General", "CPU Info:");
    logger.Info("General", "  Logical cores count: {}", info.logical_cores_count);
    logger.Info("General", "  Physical cores count: {}", static_cast<u32>(info.physical_processors.GetSize()));
    for (u64 i = 0; i < info.physical_processors.GetSize(); ++i)
    {
        const PhysicalCoreInfo& core = info.physical_processors[i];
        logger.Info("General", "  Physical core {}:", core.id);
        logger.Info("General", "    Hyperthreaded: {}", core.is_hyperthreaded ? "yes" : "no");
        char logical_cores_str[256] = {};
        i32 offset = 0;
        for (auto bit : core.logical_cores)
        {
            offset += snprintf(logical_cores_str + offset, sizeof(logical_cores_str) - offset, "%u ", bit);
        }
        logger.Info("General", "    Logical cores: {}", logical_cores_str);
    }
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
        throw NotImplementedException(__FUNCTION__);
#endif
    }
}

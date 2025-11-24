#include "opal/allocator.h"

#include <malloc.h>

#include "opal/casts.h"
#include "opal/container/dynamic-array.h"
#include "opal/math-base.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "opal/exceptions.h"

namespace
{
Opal::u64 AlignForward(Opal::u64 address, Opal::u64 alignment)
{
    Opal::u64 modulo = address & (alignment - 1);
    if (modulo != 0)
    {
        return address + (alignment - modulo);
    }
    return address;
}
}  // namespace

Opal::SystemMemoryAllocator::SystemMemoryAllocator(const char* debug_name, const SystemMemoryAllocatorDesc& desc)
    : AllocatorBase(debug_name)
{
    if (desc.bytes_to_reserve == 0)
    {
        throw InvalidArgumentException(__FUNCTION__, "bytes_to_reserve", desc.bytes_to_reserve);
    }
    if (desc.bytes_to_initially_alloc > desc.bytes_to_reserve)
    {
        throw InvalidArgumentException(__FUNCTION__, "bytes_to_initially_alloc", desc.bytes_to_initially_alloc);
    }
    m_reserved_size = desc.bytes_to_reserve;
    m_commit_step_size = desc.commit_step_size;

#if defined(OPAL_PLATFORM_WINDOWS)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    m_allocation_granularity = sys_info.dwAllocationGranularity;
    m_page_size = sys_info.dwPageSize;
    if (m_reserved_size % m_allocation_granularity != 0)
    {
        m_reserved_size += m_allocation_granularity - (m_reserved_size % m_allocation_granularity);
    }
    m_memory = VirtualAlloc(nullptr, m_reserved_size, MEM_RESERVE, PAGE_NOACCESS);
    if (m_memory == nullptr)
    {
        throw OutOfMemoryException("Failed to reserve memory for the page allocator in the OS!");
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const i64 page_size = sysconf(_SC_PAGESIZE);
    m_page_size = Narrow<u64>(page_size);
    m_allocation_granularity = Narrow<u64>(page_size);
    if (m_reserved_size % m_allocation_granularity != 0)
    {
        m_reserved_size += m_allocation_granularity - (m_reserved_size % m_allocation_granularity);
    }
    m_memory = mmap(nullptr, m_reserved_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (m_memory == nullptr)
    {
        throw OutOfMemoryException("Failed to reserve memory for the page allocator in the OS!");
    }
#else
    throw NotImplementedException(__FUNCTION__);
#endif
    if (desc.bytes_to_initially_alloc > 0)
    {
        Commit(desc.bytes_to_initially_alloc);
    }
}

Opal::SystemMemoryAllocator::~SystemMemoryAllocator()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    VirtualFree(m_memory, 0, MEM_RELEASE);
#elif defined(OPAL_PLATFORM_LINUX)
    munmap(m_memory, m_reserved_size);
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void* Opal::SystemMemoryAllocator::Alloc(u64 size, u64)
{
    if (size % m_page_size != 0)
    {
        size += m_page_size - (size % m_page_size);
    }
    if (m_offset + size > m_commited_size)
    {
        u64 step_size = Min(m_commit_step_size, m_reserved_size - m_commited_size);
        u64 commit_size = m_offset + size - m_commited_size;
        commit_size = Opal::Max(commit_size, step_size);
        Commit(commit_size);
    }
    void* allocation = reinterpret_cast<void*>(reinterpret_cast<u64>(m_memory) + m_offset);
    m_offset += size;
    return allocation;
}

void Opal::SystemMemoryAllocator::Free(void*) {}

void Opal::SystemMemoryAllocator::Commit(u64 size)
{
    if (m_commited_size + size > m_reserved_size)
    {
        throw OutOfMemoryException("No more reserved memory!");
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    void* commited_memory = VirtualAlloc(m_memory, size, MEM_COMMIT, PAGE_READWRITE);
    if (commited_memory == nullptr)
#elif defined(OPAL_PLATFORM_LINUX)
    if (mprotect(m_memory, size, PROT_READ | PROT_WRITE) != 0)
#else
    throw NotImplementedException(__FUNCTION__);
#endif
    {
        throw OutOfMemoryException("Failed to commit memory for the page allocator!");
    }
    m_commited_size += size;
}
void Opal::SystemMemoryAllocator::Reset()
{
    m_offset = 0;
}

void* Opal::MallocAllocator::Alloc(size_t size, size_t alignment)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return _aligned_malloc(size, alignment);
#else
    (void)alignment;
    return malloc(size);
#endif
}

void Opal::MallocAllocator::Free(void* ptr)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

Opal::LinearAllocator::LinearAllocator(const char* debug_name, const SystemMemoryAllocatorDesc& desc)
    : AllocatorBase(debug_name), m_system_allocator("LinearAllocator - SystemMemoryAllocator", desc)
{
    m_memory = m_system_allocator.Alloc(desc.bytes_to_initially_alloc, 1);
    m_offset = 0;
    m_size = desc.bytes_to_initially_alloc;
    m_commit_step_size = desc.commit_step_size;
}

Opal::LinearAllocator::~LinearAllocator() {}

bool Opal::LinearAllocator::operator==(const Opal::LinearAllocator& other) const
{
    (void)other;
    return false;
}

void* Opal::LinearAllocator::Alloc(size_t size, size_t alignment)
{
    u64 current_address = reinterpret_cast<u64>(m_memory) + m_offset;
    u64 new_address = AlignForward(current_address, alignment);
    u64 next_address = new_address + size;
    if (next_address > reinterpret_cast<u64>(m_memory) + m_size)
    {
        // Commit more memory
        m_system_allocator.Alloc(size, 1);
        m_size = m_system_allocator.GetCommitedSize();
    }
    m_offset = next_address - reinterpret_cast<u64>(m_memory);
    return reinterpret_cast<void*>(new_address);
}

void Opal::LinearAllocator::Free(void* ptr)
{
    (void)ptr;
}

void Opal::LinearAllocator::Reset()
{
    m_offset = 0;
}

namespace
{
Opal::MallocAllocator g_malloc_allocator;
Opal::DynamicArray<Opal::AllocatorBase*> g_default_allocators(1, &g_malloc_allocator, &g_malloc_allocator);
Opal::LinearAllocator g_scratch_allocator("Default Scratch Allocator", {.bytes_to_reserve = OPAL_GB(4),
                                                                        .bytes_to_initially_alloc = OPAL_MB(1),
                                                                        .commit_step_size = OPAL_MB(2)});
Opal::DynamicArray<Opal::LinearAllocator*> g_scratch_allocators(1, &g_scratch_allocator, &g_malloc_allocator);
}  // namespace

Opal::AllocatorBase* Opal::GetDefaultAllocator()
{
    return g_default_allocators.Back().GetValue();
}

void Opal::PushDefaultAllocator(AllocatorBase* allocator)
{
    if (allocator == nullptr)
    {
        g_default_allocators.PushBack(&g_malloc_allocator);
    }
    else
    {
        g_default_allocators.PushBack(allocator);
    }
}

void Opal::PopDefaultAllocator()
{
    if (g_default_allocators.GetSize() == 1)
    {
        return;
    }
    g_default_allocators.PopBack();
}

Opal::LinearAllocator* Opal::GetScratchAllocator()
{
    return g_scratch_allocators.Back().GetValue();
}

void Opal::PushScratchAllocator(LinearAllocator* allocator)
{
    if (allocator == nullptr)
    {
        g_scratch_allocators.PushBack(&g_scratch_allocator);
    }
    else
    {
        g_scratch_allocators.PushBack(allocator);
    }
}

void Opal::PopScratchAllocator()
{
    if (g_scratch_allocators.GetSize() == 1)
    {
        return;
    }
    g_scratch_allocators.PopBack();
}

void Opal::ResetScratchAllocator()
{
    g_scratch_allocators.Back().GetValue()->Reset();
}

Opal::ScratchAsDefault::ScratchAsDefault()
{
    PushDefaultAllocator(GetScratchAllocator());
}

Opal::ScratchAsDefault::~ScratchAsDefault()
{
    PopDefaultAllocator();
}

Opal::PushDefault::PushDefault(AllocatorBase* allocator)
{
    PushDefaultAllocator(allocator);
}

Opal::PushDefault::~PushDefault()
{
    PopDefaultAllocator();
}

Opal::PushScratch::PushScratch(LinearAllocator* allocator)
{
    PushScratchAllocator(allocator);
}

Opal::PushScratch::~PushScratch()
{
    PopScratchAllocator();
}

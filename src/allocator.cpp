#include "opal/allocator.h"

#include <malloc.h>

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
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

Opal::SystemMemoryAllocator::SystemMemoryAllocator(i64 bytes_to_reserve, i64 bytes_to_initially_alloc, const char* debug_name)
    : AllocatorBase(debug_name)
{
    if (bytes_to_reserve == 0)
    {
        throw InvalidArgumentException(__FUNCTION__, "bytes_to_reserve", bytes_to_reserve);
    }
    if (bytes_to_initially_alloc > bytes_to_reserve)
    {
        throw InvalidArgumentException(__FUNCTION__, "bytes_to_initially_alloc", bytes_to_initially_alloc);
    }
    m_reserved_size = bytes_to_reserve;
    m_commited_size = bytes_to_initially_alloc;

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
    if (m_commited_size > 0)
    {
        void* commited_memory = VirtualAlloc(m_memory, m_commited_size, MEM_COMMIT, PAGE_READWRITE);
        if (commited_memory == nullptr)
        {
            throw OutOfMemoryException("Failed to commit initial memory for the page allocator!");
        }
    }
#endif
}

Opal::SystemMemoryAllocator::~SystemMemoryAllocator()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    VirtualFree(m_memory, 0, MEM_RELEASE);
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
        Commit(m_offset + size - m_commited_size);
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
    void* commited_memory = VirtualAlloc(m_memory, size, MEM_COMMIT, PAGE_READWRITE);
    if (commited_memory == nullptr)
    {
        throw OutOfMemoryException("Failed to commit memory for the page allocator!");
    }
    m_commited_size += size;
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

Opal::LinearAllocator::LinearAllocator(u64 size) : AllocatorBase("LinearAllocator")
{
#if defined(OPAL_PLATFORM_WINDOWS)
    m_memory = _aligned_malloc(size, 16);
#else
    m_memory = malloc(size);
#endif
    m_offset = 0;
    m_size = size;
}

Opal::LinearAllocator::~LinearAllocator()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    _aligned_free(m_memory);
#else
    free(m_memory);
#endif
}

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
    if (next_address <= reinterpret_cast<u64>(m_memory) + m_size)
    {
        m_offset = next_address - reinterpret_cast<u64>(m_memory);
        return reinterpret_cast<void*>(new_address);
    }
    return nullptr;
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
Opal::AllocatorBase* g_default_allocator = &g_malloc_allocator;
}  // namespace

Opal::AllocatorBase* Opal::GetDefaultAllocator()
{
    return g_default_allocator;
}

void Opal::SetDefaultAllocator(AllocatorBase* allocator)
{
    if (allocator == nullptr)
    {
        g_default_allocator = &g_malloc_allocator;
        return;
    }
    g_default_allocator = allocator;
}

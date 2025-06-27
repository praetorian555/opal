#include "opal/allocator.h"

#include <malloc.h>

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

Opal::LinearAllocator::LinearAllocator(u64 size)
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

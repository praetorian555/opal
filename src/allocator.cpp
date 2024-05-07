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
    return alignment;
}
}  // namespace

bool Opal::DefaultAllocator::operator==(const Opal::DefaultAllocator& other) const
{
    (void)other;
    return true;
}

void* Opal::DefaultAllocator::Alloc(size_t size, size_t alignment)
{
    return _aligned_malloc(size, alignment);
}

void Opal::DefaultAllocator::Free(void* ptr)
{
    _aligned_free(ptr);
}

void* Opal::MallocAllocator::Alloc(size_t size, size_t alignment)
{
    return _aligned_malloc(size, alignment);
}

void Opal::MallocAllocator::Free(void* ptr)
{
    _aligned_free(ptr);
}

Opal::LinearAllocator::LinearAllocator(u64 size)
{
    m_memory = _aligned_malloc(size, 16);
    m_offset = 0;
    m_size = size;
}

Opal::LinearAllocator::~LinearAllocator()
{
    _aligned_free(m_memory);
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

#include "opal/allocator.h"

#include <malloc.h>

bool Opal::DefaultAllocator::operator==(const Opal::DefaultAllocator& other) const
{
    (void)other;
    return true;
}

void* Opal::DefaultAllocator::Allocate(size_t size, size_t alignment)
{
    return _aligned_malloc(size, alignment);
}

void Opal::DefaultAllocator::Deallocate(void* ptr)
{
    _aligned_free(ptr);
}

void Opal::DefaultAllocator::Reset() {}

#pragma once

#include <malloc.h>

namespace Opal
{

template <typename T>
struct DefaultAllocator
{
    DefaultAllocator() = default;
    DefaultAllocator(const DefaultAllocator& other) = default;
    DefaultAllocator(DefaultAllocator&& other) = default;

    ~DefaultAllocator() = default;

    DefaultAllocator& operator=(const DefaultAllocator& other) = default;
    DefaultAllocator& operator=(DefaultAllocator&& other) = default;

    bool operator==(const DefaultAllocator&) const { return true; }

    T* Allocate(size_t n) { return static_cast<T*>(malloc(n * sizeof(T))); }

    void Deallocate(T* p, size_t n)
    {
        (void)n;
        free(p);
    }
};

}  // namespace Opal
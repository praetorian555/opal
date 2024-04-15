#pragma once

#include <malloc.h>

#include "opal/types.h"

namespace Opal
{

template <typename T>
struct DefaultAllocator
{
    using ValueType = T;
    using PointerType = T*;
    using DifferenceType = i64;

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
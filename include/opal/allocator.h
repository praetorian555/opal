#pragma once

#include <malloc.h>

#include "opal/types.h"
#include "opal/type-traits.h"

namespace Opal
{

template <typename T>
struct DefaultAllocator
{
    static_assert(sizeof(T) > 0, "Type must have a size greater than 0");
    static_assert(!k_is_reference_value<T>, "Type must not be a reference");
    static_assert(!k_is_const_value<T>, "Type must not be const");

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
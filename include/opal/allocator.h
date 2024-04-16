#pragma once

#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

struct DefaultAllocator
{
    DefaultAllocator() = default;
    DefaultAllocator(const DefaultAllocator& other) = default;
    DefaultAllocator(DefaultAllocator&& other) = default;

    ~DefaultAllocator() = default;

    DefaultAllocator& operator=(const DefaultAllocator& other) = default;
    DefaultAllocator& operator=(DefaultAllocator&& other) = default;

    bool operator==(const DefaultAllocator& other) const;

    void* Allocate(size_t size, size_t alignment = 8);
    void Deallocate(void* ptr);
    void Reset();
};

}  // namespace Opal
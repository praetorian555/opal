#pragma once

#include <memory>

#include "opal/defines.h"
#include "opal/allocator.h"

namespace Opal
{

/**
 * Represents a unique pointer to an object of type T. It can't be copied but can be moved.
 */
template <typename T, typename Deleter = std::default_delete<T>>
using ScopePtr = std::unique_ptr<T>;

template <typename T, typename Allocator = AllocatorBase, typename... Args>
Opal::ScopePtr<T> MakeScoped(Allocator* allocator, Args&&... args)
{
    void* memory = nullptr;
    if (allocator == nullptr)
    {
        MallocAllocator malloc_allocator;
        malloc_allocator.Alloc(sizeof(T), 8);
    }
    else
    {
        memory = allocator->Alloc(sizeof(T), 8);
    }
    return Opal::ScopePtr<T>{new(memory) T(std::forward<Args>(args)...)};
}

}  // namespace Opal

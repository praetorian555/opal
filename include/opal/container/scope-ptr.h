#pragma once

#include <memory>

#include "opal/allocator.h"
#include "opal/defines.h"

namespace Opal
{

/**
 * Represents a unique pointer to an object of type T. It can't be copied but can be moved.
 */
template <typename T, typename Deleter = std::default_delete<T>>
using ScopePtr = std::unique_ptr<T>;

/**
 * Allocates memory using the provided allocator and constructs an object of type T in that memory.
 * @tparam T Type of the object to be constructed.
 * @tparam Allocator Type of the allocator to be used for memory allocation.
 * @tparam Args Types of the arguments to be passed to the constructor of the object.
 * @param allocator Allocator to be used for memory allocation.
 * @param args Arguments to be passed to the constructor of the object.
 * @return A unique pointer to the object of type T.
 */
template <typename T, typename Allocator, typename... Args>
ScopePtr<T> MakeScoped(Allocator* allocator, Args&&... args)
{
    void* memory = nullptr;
    if (allocator == nullptr)
    {
        MallocAllocator malloc_allocator;
        memory = malloc_allocator.Alloc(sizeof(T), 8);
    }
    else
    {
        memory = allocator->Alloc(sizeof(T), 8);
    }
    return Opal::ScopePtr<T>{new (memory) T(std::forward<Args>(args)...)};
}

/**
 * Allocates memory using MallocAllocator and constructs an object of type T in that memory.
 * @tparam T Type of the object to be constructed.
 * @tparam Args Types of the arguments to be passed to the constructor of T.
 * @param args Arguments to be passed to the constructor of T.
 * @return A unique pointer to the object of type T.
 */
template <typename T, typename... Args>
ScopePtr<T> MakeDefaultScoped(Args&&... args)
{
    void* memory = nullptr;
    MallocAllocator malloc_allocator;
    memory = malloc_allocator.Alloc(sizeof(T), 8);

    return Opal::ScopePtr<T>{new (memory) T(std::forward<Args>(args)...)};
}

}  // namespace Opal

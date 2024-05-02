#pragma once

#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

struct Allocator
{
    virtual ~Allocator() = default;
    virtual void* Alloc(u64 size, u64 alignment) = 0;
    virtual void Free(void* ptr) = 0;
};

/**
 * Stateless allocator that uses system's malloc and free.
 */
struct DefaultAllocator final : public Allocator
{
    DefaultAllocator() = default;
    DefaultAllocator(const DefaultAllocator& other) = default;
    DefaultAllocator(DefaultAllocator&& other) = default;

    ~DefaultAllocator() override = default;

    DefaultAllocator& operator=(const DefaultAllocator& other) = default;
    DefaultAllocator& operator=(DefaultAllocator&& other) = default;

    bool operator==(const DefaultAllocator& other) const;

    void* Alloc(size_t size, size_t alignment) override;
    void Free(void* ptr) override;
};

struct LinearAllocator
{
    explicit LinearAllocator(u64 size);
    LinearAllocator(const LinearAllocator& other) = delete;
    LinearAllocator(LinearAllocator&& other) = delete;

    ~LinearAllocator();

    LinearAllocator& operator=(const LinearAllocator& other) = default;
    LinearAllocator& operator=(LinearAllocator&& other) = default;

    bool operator==(const LinearAllocator& other) const;

    void* Allocate(size_t size, size_t alignment = 8);
    void Deallocate(void* ptr);
    void Reset();

private:
    void* m_memory;
    u64 m_offset = 0;
    u64 m_size = 0;
};

}  // namespace Opal
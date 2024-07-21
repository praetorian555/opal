#pragma once

#include "opal/types.h"
#include "opal/export.h"

namespace Opal
{

struct OPAL_EXPORT AllocatorBase
{
    virtual ~AllocatorBase() = default;
    virtual void* Alloc(u64 size, u64 alignment) = 0;
    virtual void Free(void* ptr) = 0;
};

/**
 * Stateless allocator that uses system's malloc and free.
 */
struct OPAL_EXPORT DefaultAllocator final : public AllocatorBase
{
    DefaultAllocator() = default;
    DefaultAllocator(const DefaultAllocator& other) = default;
    DefaultAllocator(DefaultAllocator&& other) = default;

    ~DefaultAllocator() override = default;

    DefaultAllocator& operator=(const DefaultAllocator& other) = default;
    DefaultAllocator& operator=(DefaultAllocator&& other) = default;

    bool operator==(const DefaultAllocator& other) const;

    void* Alloc(u64 size, u64 alignment) override;
    void Free(void* ptr) override;
};

/**
 * Stateless allocator that uses system's malloc and free API.
 */
struct OPAL_EXPORT MallocAllocator final : public AllocatorBase
{
    MallocAllocator() = default;
    MallocAllocator(const MallocAllocator& other) = default;
    MallocAllocator(MallocAllocator&& other) = default;

    ~MallocAllocator() override = default;

    MallocAllocator& operator=(const MallocAllocator& other) = default;
    MallocAllocator& operator=(MallocAllocator&& other) = default;

    bool operator==(const MallocAllocator&) const { return true; }

    void* Alloc(u64 size, u64 alignment) override;
    void Free(void* ptr) override;
};

struct OPAL_EXPORT NullAllocator final : public AllocatorBase
{
    NullAllocator() = default;
    NullAllocator(const NullAllocator& other) = default;
    NullAllocator(NullAllocator&& other) = default;

    ~NullAllocator() override = default;

    NullAllocator& operator=(const NullAllocator& other) = default;
    NullAllocator& operator=(NullAllocator&& other) = default;

    bool operator==(const NullAllocator&) const { return true; }

    void* Alloc(u64, u64) override { return nullptr; }
    void Free(void*) override {}
};

struct OPAL_EXPORT LinearAllocator final : public AllocatorBase
{
    explicit LinearAllocator(u64 size);
    LinearAllocator(const LinearAllocator& other) = delete;
    LinearAllocator(LinearAllocator&& other) = delete;

    ~LinearAllocator() override;

    LinearAllocator& operator=(const LinearAllocator& other) = default;
    LinearAllocator& operator=(LinearAllocator&& other) = default;

    bool operator==(const LinearAllocator& other) const;

    void* Alloc(u64 size, u64 alignment) override;
    void Free(void* ptr) override;
    void Reset();

private:
    void* m_memory;
    u64 m_offset = 0;
    u64 m_size = 0;
};

}  // namespace Opal
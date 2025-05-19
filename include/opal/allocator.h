#pragma once

#include <utility>

#include "opal/export.h"
#include "opal/types.h"

namespace Opal
{

struct OPAL_EXPORT AllocatorBase
{
    virtual ~AllocatorBase() = default;
    virtual void* Alloc(u64 size, u64 alignment) = 0;
    virtual void Free(void* ptr) = 0;
    [[nodiscard]] virtual const char* GetName() const = 0;
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

    [[nodiscard]] const char* GetName() const override { return "MallocAllocator"; }
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

    [[nodiscard]] const char* GetName() const override { return "NullAllocator"; }
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

    [[nodiscard]] const char* GetName() const override { return "LinearAllocator"; }

private:
    void* m_memory;
    u64 m_offset = 0;
    u64 m_size = 0;
};

/**
 * Get the pointer to the default allocator.
 * @return Pointer to the default allocator. It will never be nullptr.
 */
OPAL_EXPORT AllocatorBase* GetDefaultAllocator();

/**
 * Set the default allocator.
 * @param allocator Pointer to the allocator. If its nullptr library's default allocator will be used.
 */
OPAL_EXPORT void SetDefaultAllocator(AllocatorBase* allocator);

template <typename T, class... Args>
T* New(AllocatorBase* allocator, Args&&... args)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    void* memory = allocator->Alloc(sizeof(T), alignof(T));
    if (memory == nullptr) [[unlikely]]
    {
        return nullptr;
    }
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, class... Args>
T* New(u32 alignment, AllocatorBase* allocator, Args&&... args)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    void* memory = allocator->Alloc(sizeof(T), alignment);
    if (memory == nullptr) [[unlikely]]
    {
        return nullptr;
    }
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, class... Args>
void Delete(AllocatorBase* allocator, T* ptr)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    ptr->~T();
    allocator->Free(ptr);
}

}  // namespace Opal
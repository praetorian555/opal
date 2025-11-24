#pragma once

#include <utility>

#include "container/ref.h"
#include "opal/export.h"
#include "opal/types.h"

namespace Opal
{

struct OPAL_EXPORT AllocatorBase
{
    AllocatorBase(const char* debug_name) : m_debug_name(debug_name) {}
    virtual ~AllocatorBase() = default;
    virtual void* Alloc(u64 size, u64 alignment) = 0;
    virtual void Free(void* ptr) = 0;
    [[nodiscard]] virtual const char* GetName() const { return m_debug_name; }

protected:
    const char* m_debug_name = nullptr;
};

struct OPAL_EXPORT SystemMemoryAllocatorDesc
{
    u64 bytes_to_reserve = OPAL_MB(1);
    u64 bytes_to_initially_alloc = OPAL_KB(100);
    // Whenever more memory needs to be commited this amount will be commited unless required
    // size is larger then that size will be commited.
    u64 commit_step_size = OPAL_KB(100);
};

/**
 * Allocator that has access to the system memory and is mostly used as a part of other allocators.
 */
struct OPAL_EXPORT SystemMemoryAllocator : public AllocatorBase
{
    SystemMemoryAllocator(const char* debug_name, const SystemMemoryAllocatorDesc& desc = {});
    ~SystemMemoryAllocator() override;

    void* Alloc(u64 size, u64 alignment) override;
    void Free(void* ptr) override;

    void Commit(u64 size);

    void Reset();

    u64 GetCommitedSize() const { return m_commited_size; }
    u64 GetPageSize() const { return m_page_size; }

protected:
    void* m_memory = nullptr;
    u64 m_reserved_size = 0;
    u64 m_commited_size = 0;
    u64 m_commit_step_size = 0;
    u64 m_offset = 0;
    u64 m_allocation_granularity = 0;
    u64 m_page_size = 0;
};

/**
 * Stateless allocator that uses system's malloc and free API.
 */
struct OPAL_EXPORT MallocAllocator final : public AllocatorBase
{
    MallocAllocator() : AllocatorBase("MallocAllocator") {}
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
    NullAllocator() : AllocatorBase("NullAllocator") {}
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
    explicit LinearAllocator(const char* debug_name, const SystemMemoryAllocatorDesc& desc = {});
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
    SystemMemoryAllocator m_system_allocator;
    void* m_memory;
    u64 m_commit_step_size = 0;
    u64 m_offset = 0;
    u64 m_size = 0;
};

/**
 * Get the pointer to the default allocator. This allocator should be used for long lived data.
 * @return Pointer to the default allocator. It will never be nullptr.
 */
OPAL_EXPORT AllocatorBase* GetDefaultAllocator();

/**
 * Push desired allocator to be new default allocator.
 * @param allocator Pointer to the allocator. It will never be nullptr.
 */
OPAL_EXPORT void PushDefaultAllocator(AllocatorBase* allocator);

/**
 * Remove allocator from the top of the stack. The stack will never be empty and default allocator will be available.
 */
OPAL_EXPORT void PopDefaultAllocator();

/**
 * Get default scratch allocator. This allocator should be used for short-lived allocations, usually inside a function span. Deallocation
 * is not possible, and it can only be reset.
 * @return Pointer to a scratch allocator. It will never be nullptr.
 */
OPAL_EXPORT LinearAllocator* GetScratchAllocator();

/**
 * Push desired allocator to be new scratch allocator.
 * @param allocator New allocator.
 */
OPAL_EXPORT void PushScratchAllocator(LinearAllocator* allocator);

/**
 * Remove allocator from top of the scratch allocator stack.
 */
OPAL_EXPORT void PopScratchAllocator();

/**
 * Reset current scratch allocator.
 */
OPAL_EXPORT void ResetScratchAllocator();

template <typename T, class... Args>
T* New(Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    void* memory = allocator->Alloc(sizeof(T), alignof(T));
    if (memory == nullptr) [[unlikely]]
    {
        return nullptr;
    }
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, u32 Alignment, class... Args>
T* New(Args&&... args)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    void* memory = allocator->Alloc(sizeof(T), Alignment);
    if (memory == nullptr) [[unlikely]]
    {
        return nullptr;
    }
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, class... Args>
void Delete(T* ptr)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ptr->~T();
    allocator->Free(ptr);
}

}  // namespace Opal
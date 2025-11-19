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

/**
 * Allocator that has access to the system memory and is mostly used as a part of other allocators.
 */
struct SystemMemoryAllocator : public AllocatorBase
{
    SystemMemoryAllocator(i64 bytes_to_reserve, i64 bytes_to_initially_alloc, const char* debug_name);
    ~SystemMemoryAllocator() override;

    void* Alloc(u64 size, u64 alignment) override;
    void Free(void* ptr) override;

    void Commit(u64 size);

    void Reset();

    [[nodiscard]] u64 GetCommitedSize() const { return m_commited_size; }
    [[nodiscard]] u64 GetPageSize() const { return m_page_size; }

protected:
    void* m_memory = nullptr;
    u64 m_reserved_size = 0;
    u64 m_commited_size = 0;
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
T* New(const Ref<AllocatorBase>& allocator, Args&&... args)
{
    return New<T>(allocator.GetPtr(), std::forward<Args>(args)...);
}

template <typename T, class... Args>
T* New(u32 alignment, const Ref<AllocatorBase>& allocator, Args&&... args)
{
    return New<T>(alignment, allocator, std::forward<Args>(args)...);
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

template <typename T, class... Args>
void Delete(const Ref<AllocatorBase>& allocator, T* ptr)
{
    return Delete<T>(allocator.GetPtr(), ptr);
}


}  // namespace Opal
#pragma once

#include "opal/allocator.h"
#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

namespace Impl
{
struct SmallBlockHeap;
struct SmallBlockAllocatorAccess;
}  // namespace Impl

/**
 * Thread-safe allocator for small blocks that are often freed on a different thread from the one that allocated them, such as the
 * tasks a ThreadPool runs. Each thread allocates from a cache of its own, so allocating and freeing on the same thread takes no lock
 * and no atomic operation, and threads freeing each other's blocks do not contend on a shared free list.
 *
 * Blocks of up to k_max_block_size bytes aligned to at most k_max_alignment are served this way. Larger or more aligned requests
 * are passed straight to the upstream allocator.
 *
 * Memory taken from the upstream allocator for small blocks is kept for reuse until the allocator is destroyed, rather than handed
 * back as soon as it is free. When a thread exits, its cache passes to the next thread that starts allocating.
 *
 * Every block must be freed before the allocator is destroyed, and no thread may use it while it is being destroyed.
 */
class SmallBlockAllocator final : public AllocatorBase
{
public:
    /** Largest block, in bytes, that is served from the per-thread caches. */
    static constexpr u64 k_max_block_size = 512;

    /** Largest alignment that is served from the per-thread caches. */
    static constexpr u64 k_max_alignment = 64;

    /**
     * Does not allocate. Memory is taken from the upstream allocator as threads start allocating.
     * @param upstream Allocator to take memory from. Must be thread safe. If null, uses the default allocator.
     * @param debug_name Name reported by GetName.
     */
    explicit SmallBlockAllocator(AllocatorBase* upstream = nullptr, const char* debug_name = "SmallBlockAllocator");
    ~SmallBlockAllocator() override;

    SmallBlockAllocator(const SmallBlockAllocator&) = delete;
    SmallBlockAllocator& operator=(const SmallBlockAllocator&) = delete;
    SmallBlockAllocator(SmallBlockAllocator&&) = delete;
    SmallBlockAllocator& operator=(SmallBlockAllocator&&) = delete;

    /**
     * Allocate a block.
     * @param size Number of bytes. Zero is taken as one.
     * @param alignment Required alignment, a power of two.
     * @return Address of the block, or nullptr when the upstream allocator could not provide the memory.
     */
    void* Alloc(u64 size, u64 alignment) override;

    /** Free a block allocated by this allocator, on any thread. Does nothing for nullptr. */
    void Free(void* ptr) override;

    [[nodiscard]] bool IsThreadSafe() const override { return true; }

    /** @return The allocator memory is taken from. */
    [[nodiscard]] AllocatorBase* GetUpstream() const { return m_upstream; }

private:
    friend struct Impl::SmallBlockAllocatorAccess;

    /** @return The calling thread's cache, attaching one if it has none. Null if one could not be allocated. */
    Impl::SmallBlockHeap* GetThreadHeap();

    /** Give the calling thread a cache: one left by a thread that exited, or a new one. */
    Impl::SmallBlockHeap* AttachHeap();

    /** Hand back a cache whose thread no longer uses it, so another thread can take it. */
    void RetireHeap(Impl::SmallBlockHeap* heap);

    void* AllocSlow(Impl::SmallBlockHeap* heap, u32 class_index);
    void* AllocLarge(u64 size, u64 alignment);

    AllocatorBase* m_upstream = nullptr;
    u64 m_id = 0;

    // Neighbours in the list of live allocators that exiting threads consult.
    SmallBlockAllocator* m_registry_prev = nullptr;
    SmallBlockAllocator* m_registry_next = nullptr;

    // Guards the two lists below.
    Impl::PureMutex m_mutex;
    Impl::SmallBlockHeap* m_heaps = nullptr;
    Impl::SmallBlockHeap* m_idle_heaps = nullptr;
};

}  // namespace Opal

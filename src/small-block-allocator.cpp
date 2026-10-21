#include "opal/small-block-allocator.h"

#include <new>

#include "opal/assert.h"
#include "opal/defines.h"
#include "opal/threading/atomic.h"

// Memory for small blocks is taken from upstream in pages, each holding blocks of one size class and owned by one heap. A heap is
// one thread's cache. The owning thread allocates and frees through the page's local free list with plain loads and stores. Any
// other thread hands a block back by pushing it onto the page's thread_free list, which the owner takes in one exchange once its
// local list runs dry. A freeing thread collects up to k_remote_batch blocks of one page before pushing them, so a stream of tasks
// freed on a worker moves the page's line between cores once per batch rather than once per block.
//
// Every block is preceded by an 8-byte header. For a small block it points at the block's page. For a block from upstream it is
// k_large_tag, with the address upstream returned stored in the 8 bytes before it.

namespace
{

constexpr Opal::u64 k_page_size = 64 * 1024;
constexpr Opal::u64 k_page_alignment = 64;
constexpr Opal::u64 k_page_header_size = 128;
constexpr Opal::u64 k_header_size = 8;
constexpr Opal::u64 k_large_tag = 1;
constexpr Opal::u32 k_remote_batch = 32;
constexpr Opal::u32 k_thread_entry_count = 8;

// Size classes. The first family is aligned to 16 bytes, the second to 64 for blocks that ask for more.
constexpr Opal::u64 k_small_alignment = 16;
constexpr Opal::u64 k_class_sizes[] = {16,  32,  48,  64,  80, 96,  112, 128, 160, 192, 224, 256,
                                       320, 384, 448, 512, 64, 128, 192, 256, 320, 384, 448, 512};
constexpr Opal::u32 k_class_count = sizeof(k_class_sizes) / sizeof(k_class_sizes[0]);
constexpr Opal::u32 k_first_aligned_class = 16;

constexpr Opal::u64 GetClassAlignment(Opal::u32 class_index)
{
    return class_index < k_first_aligned_class ? k_small_alignment : Opal::SmallBlockAllocator::k_max_alignment;
}

// Each block leaves room before its payload for the header, so payloads stay aligned to the class alignment.
constexpr Opal::u64 GetClassStride(Opal::u32 class_index)
{
    return k_class_sizes[class_index] + GetClassAlignment(class_index);
}

// Class of the 16-byte family for a size of (index * 16) bytes or less.
constexpr auto k_class_by_16 = []
{
    struct Table
    {
        Opal::u8 classes[Opal::SmallBlockAllocator::k_max_block_size / 16 + 1] = {};
    } table;
    Opal::u32 class_index = 0;
    for (Opal::u64 index = 1; index <= Opal::SmallBlockAllocator::k_max_block_size / 16; index++)
    {
        while (k_class_sizes[class_index] < index * 16)
        {
            class_index++;
        }
        table.classes[index] = static_cast<Opal::u8>(class_index);
    }
    return table;
}();

static_assert(k_class_sizes[k_first_aligned_class - 1] == Opal::SmallBlockAllocator::k_max_block_size);
static_assert(k_class_sizes[k_class_count - 1] == Opal::SmallBlockAllocator::k_max_block_size);

}  // namespace

namespace Opal::Impl
{

/** A free block, linked through its first bytes. */
struct SmallBlock
{
    SmallBlock* next;
};

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)
/** Start of a page. Everything but thread_free is touched only by the thread that owns the page's heap. */
struct alignas(k_page_alignment) SmallBlockPage
{
    SmallBlock* local_free = nullptr;
    // Payload of the next block never handed out, and of the last block that fits in the page.
    u8* bump = nullptr;
    u8* last = nullptr;
    u64 stride = 0;
    SmallBlockPage* next = nullptr;
    // Set when the page is made and never changed, so any thread may read it.
    SmallBlockHeap* heap = nullptr;
    // Blocks freed by other threads. On a line of its own, so their pushes do not invalidate the owner's fields.
    alignas(k_page_alignment) Atomic<SmallBlock*> thread_free = nullptr;
};
OPAL_END_DISABLE_WARNINGS

static_assert(sizeof(SmallBlockPage) <= k_page_header_size);

/** One thread's cache. */
struct SmallBlockHeap
{
    explicit SmallBlockHeap(SmallBlockAllocator* owner) : allocator(owner) {}

    SmallBlockAllocator* allocator;
    // Per size class, the page blocks are taken from and every page this heap owns.
    SmallBlockPage* current[k_class_count] = {};
    SmallBlockPage* pages[k_class_count] = {};
    // Blocks of a page owned by another heap, freed on this heap's thread and not yet pushed back.
    SmallBlockPage* pending_page = nullptr;
    SmallBlock* pending_head = nullptr;
    SmallBlock* pending_tail = nullptr;
    u32 pending_count = 0;
    // Links in the allocator's list of every heap and its list of heaps no thread uses.
    SmallBlockHeap* next = nullptr;
    SmallBlockHeap* next_idle = nullptr;
};

/** What the thread-exit hook needs from an allocator. */
struct SmallBlockAllocatorAccess
{
    static u64 GetId(const SmallBlockAllocator& allocator) { return allocator.m_id; }
    static SmallBlockAllocator*& GetRegistryPrev(SmallBlockAllocator& allocator) { return allocator.m_registry_prev; }
    static SmallBlockAllocator*& GetRegistryNext(SmallBlockAllocator& allocator) { return allocator.m_registry_next; }
    static void RetireHeap(SmallBlockAllocator& allocator, SmallBlockHeap* heap) { allocator.RetireHeap(heap); }
};

}  // namespace Opal::Impl

namespace
{

using Opal::Impl::SmallBlock;
using Opal::Impl::SmallBlockAllocatorAccess;
using Opal::Impl::SmallBlockHeap;
using Opal::Impl::SmallBlockPage;

class ScopedLock
{
public:
    explicit ScopedLock(Opal::Impl::PureMutex& mutex) : m_mutex(mutex) { m_mutex.Lock(); }
    ~ScopedLock() { m_mutex.Unlock(); }

    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;

private:
    Opal::Impl::PureMutex& m_mutex;
};

// Live allocators, so that a thread exiting after an allocator was destroyed does not touch it. Never destroyed: threads may exit
// during static destruction.
Opal::Impl::PureMutex& GetRegistryMutex()
{
    alignas(Opal::Impl::PureMutex) static Opal::u8 s_storage[sizeof(Opal::Impl::PureMutex)];
    static Opal::Impl::PureMutex* s_mutex = new (s_storage) Opal::Impl::PureMutex();
    return *s_mutex;
}
Opal::SmallBlockAllocator* g_registry_head = nullptr;
Opal::Atomic<Opal::u64> g_next_allocator_id = 1;

/** The calling thread's heap for one allocator. Ids are never reused, so an entry for a destroyed allocator never matches again. */
struct ThreadHeapEntry
{
    Opal::u64 allocator_id = 0;
    Opal::SmallBlockAllocator* allocator = nullptr;
    SmallBlockHeap* heap = nullptr;
};

// Plain data, so reading it costs no initialization check. The hook below is what runs at thread exit.
constinit thread_local ThreadHeapEntry t_entries[k_thread_entry_count] = {};

bool IsLive(const Opal::SmallBlockAllocator* allocator, Opal::u64 id)
{
    for (Opal::SmallBlockAllocator* live = g_registry_head; live != nullptr; live = SmallBlockAllocatorAccess::GetRegistryNext(*live))
    {
        if (live == allocator)
        {
            return SmallBlockAllocatorAccess::GetId(*live) == id;
        }
    }
    return false;
}

/** Give an entry's heap back to its allocator if that still exists, and clear the entry. The registry mutex must be held. */
void ReleaseEntry(ThreadHeapEntry& entry)
{
    if (entry.allocator_id != 0 && IsLive(entry.allocator, entry.allocator_id))
    {
        SmallBlockAllocatorAccess::RetireHeap(*entry.allocator, entry.heap);
    }
    entry = ThreadHeapEntry{};
}

struct ThreadExitHook
{
    bool is_armed = false;

    ~ThreadExitHook()
    {
        ScopedLock lock(GetRegistryMutex());
        for (ThreadHeapEntry& entry : t_entries)
        {
            ReleaseEntry(entry);
        }
    }
};

thread_local ThreadExitHook t_exit_hook;

void WriteHeader(Opal::u8* payload, Opal::u64 header)
{
    *reinterpret_cast<Opal::u64*>(payload - k_header_size) = header;
}

Opal::u64 ReadHeader(const Opal::u8* payload)
{
    return *reinterpret_cast<const Opal::u64*>(payload - k_header_size);
}

/** Take a block from the page's local free list, or carve one it has never handed out. Null if it has neither. */
void* TakeBlock(SmallBlockPage* page)
{
    if (SmallBlock* block = page->local_free)
    {
        page->local_free = block->next;
        return block;
    }
    if (page->bump <= page->last)
    {
        Opal::u8* payload = page->bump;
        page->bump += page->stride;
        WriteHeader(payload, reinterpret_cast<Opal::u64>(page));
        return payload;
    }
    return nullptr;
}

/** Move the blocks other threads freed onto the local free list, which must be empty. @return True if there were any. */
bool TakeRemoteFrees(SmallBlockPage* page)
{
    OPAL_ASSERT(page->local_free == nullptr, "Remote frees would replace the local free list");
    page->local_free = page->thread_free.Exchange<Opal::MemoryOrder::Acquire>(nullptr);
    return page->local_free != nullptr;
}

/** Push a chain of blocks, linked from head to tail, onto a page owned by another thread. */
void PushRemote(SmallBlockPage* page, SmallBlock* head, SmallBlock* tail)
{
    SmallBlock* expected = page->thread_free.Load<Opal::MemoryOrder::Relaxed>();
    do
    {
        tail->next = expected;
    } while (!page->thread_free.CompareExchangeWeak<Opal::MemoryOrder::Release, Opal::MemoryOrder::Relaxed>(expected, head));
}

void FlushPending(SmallBlockHeap& heap)
{
    if (heap.pending_page != nullptr)
    {
        PushRemote(heap.pending_page, heap.pending_head, heap.pending_tail);
        heap.pending_page = nullptr;
        heap.pending_head = nullptr;
        heap.pending_tail = nullptr;
        heap.pending_count = 0;
    }
}

}  // namespace

Opal::SmallBlockAllocator::SmallBlockAllocator(AllocatorBase* upstream, const char* debug_name)
    : AllocatorBase(debug_name), m_upstream(upstream != nullptr ? upstream : GetDefaultAllocator())
{
    OPAL_VERIFY(m_upstream->IsThreadSafe(), "SmallBlockAllocator needs a thread-safe upstream allocator");
    m_id = g_next_allocator_id.FetchAdd(1);
    ScopedLock lock(GetRegistryMutex());
    m_registry_next = g_registry_head;
    if (g_registry_head != nullptr)
    {
        g_registry_head->m_registry_prev = this;
    }
    g_registry_head = this;
}

Opal::SmallBlockAllocator::~SmallBlockAllocator()
{
    {
        ScopedLock lock(GetRegistryMutex());
        if (m_registry_prev != nullptr)
        {
            m_registry_prev->m_registry_next = m_registry_next;
        }
        else
        {
            g_registry_head = m_registry_next;
        }
        if (m_registry_next != nullptr)
        {
            m_registry_next->m_registry_prev = m_registry_prev;
        }
    }
    for (ThreadHeapEntry& entry : t_entries)
    {
        if (entry.allocator_id == m_id)
        {
            entry = ThreadHeapEntry{};
        }
    }
    for (SmallBlockHeap* heap = m_heaps; heap != nullptr;)
    {
        for (SmallBlockPage* page : heap->pages)
        {
            while (page != nullptr)
            {
                SmallBlockPage* next = page->next;
                page->~SmallBlockPage();
                m_upstream->Free(page);
                page = next;
            }
        }
        SmallBlockHeap* next = heap->next;
        heap->~SmallBlockHeap();
        m_upstream->Free(heap);
        heap = next;
    }
}

void* Opal::SmallBlockAllocator::Alloc(u64 size, u64 alignment)
{
    if (size == 0)
    {
        size = 1;
    }
    if (size > k_max_block_size || alignment > k_max_alignment)
    {
        return AllocLarge(size, alignment);
    }
    const u32 class_index = alignment <= k_small_alignment ? k_class_by_16.classes[(size + 15) / 16]
                                                           : k_first_aligned_class + static_cast<u32>((size + 63) / 64) - 1;
    Impl::SmallBlockHeap* heap = GetThreadHeap();
    if (heap == nullptr)
    {
        return AllocLarge(size, alignment);
    }
    if (Impl::SmallBlockPage* page = heap->current[class_index])
    {
        if (void* block = TakeBlock(page))
        {
            return block;
        }
    }
    return AllocSlow(heap, class_index);
}

void* Opal::SmallBlockAllocator::AllocSlow(Impl::SmallBlockHeap* heap, u32 class_index)
{
    Impl::SmallBlockPage* current = heap->current[class_index];
    if (current != nullptr && TakeRemoteFrees(current))
    {
        return TakeBlock(current);
    }
    for (Impl::SmallBlockPage* page = heap->pages[class_index]; page != nullptr; page = page->next)
    {
        if (page == current)
        {
            continue;
        }
        if (page->local_free != nullptr || TakeRemoteFrees(page) || page->bump <= page->last)
        {
            heap->current[class_index] = page;
            return TakeBlock(page);
        }
    }

    void* memory = m_upstream->Alloc(k_page_size, k_page_alignment);
    if (memory == nullptr)
    {
        return nullptr;
    }
    Impl::SmallBlockPage* page = new (memory) Impl::SmallBlockPage();
    u8* const begin = static_cast<u8*>(memory);
    const u64 block_size = k_class_sizes[class_index];
    const u64 stride = GetClassStride(class_index);
    const u64 first = k_page_header_size + GetClassAlignment(class_index);
    page->stride = stride;
    page->bump = begin + first;
    page->last = begin + first + (k_page_size - first - block_size) / stride * stride;
    page->heap = heap;
    page->next = heap->pages[class_index];
    heap->pages[class_index] = page;
    heap->current[class_index] = page;
    return TakeBlock(page);
}

void* Opal::SmallBlockAllocator::AllocLarge(u64 size, u64 alignment)
{
    // Room for the header and the upstream address, rounded up so the payload keeps the alignment asked for.
    const u64 prefix = alignment > 2 * k_header_size ? alignment : 2 * k_header_size;
    u8* raw = static_cast<u8*>(m_upstream->Alloc(size + prefix, prefix));
    if (raw == nullptr)
    {
        return nullptr;
    }
    u8* payload = raw + prefix;
    *reinterpret_cast<u8**>(payload - 2 * k_header_size) = raw;
    WriteHeader(payload, k_large_tag);
    return payload;
}

void Opal::SmallBlockAllocator::Free(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }
    u8* payload = static_cast<u8*>(ptr);
    const u64 header = ReadHeader(payload);
    if (header == k_large_tag)
    {
        m_upstream->Free(*reinterpret_cast<u8**>(payload - 2 * k_header_size));
        return;
    }
    Impl::SmallBlockPage* page = reinterpret_cast<Impl::SmallBlockPage*>(header);
    OPAL_ASSERT(page->heap->allocator == this, "Block freed through a SmallBlockAllocator that did not allocate it");
    auto* block = static_cast<Impl::SmallBlock*>(ptr);
    Impl::SmallBlockHeap* heap = GetThreadHeap();
    if (heap == page->heap)
    {
        block->next = page->local_free;
        page->local_free = block;
        return;
    }
    if (heap == nullptr)
    {
        PushRemote(page, block, block);
        return;
    }
    if (heap->pending_page != page)
    {
        FlushPending(*heap);
        heap->pending_page = page;
        heap->pending_tail = block;
        block->next = nullptr;
    }
    else
    {
        block->next = heap->pending_head;
    }
    heap->pending_head = block;
    if (++heap->pending_count == k_remote_batch)
    {
        FlushPending(*heap);
    }
}

Opal::Impl::SmallBlockHeap* Opal::SmallBlockAllocator::GetThreadHeap()
{
    for (const ThreadHeapEntry& entry : t_entries)
    {
        if (entry.allocator_id == m_id)
        {
            return entry.heap;
        }
    }
    return AttachHeap();
}

Opal::Impl::SmallBlockHeap* Opal::SmallBlockAllocator::AttachHeap()
{
    t_exit_hook.is_armed = true;
    ThreadHeapEntry* slot = nullptr;
    for (ThreadHeapEntry& entry : t_entries)
    {
        if (entry.allocator_id == 0)
        {
            slot = &entry;
            break;
        }
    }
    if (slot == nullptr)
    {
        slot = &t_entries[k_thread_entry_count - 1];
        ScopedLock lock(GetRegistryMutex());
        ReleaseEntry(*slot);
    }

    Impl::SmallBlockHeap* heap = nullptr;
    {
        ScopedLock lock(m_mutex);
        if (m_idle_heaps != nullptr)
        {
            heap = m_idle_heaps;
            m_idle_heaps = heap->next_idle;
            heap->next_idle = nullptr;
        }
    }
    if (heap == nullptr)
    {
        void* memory = m_upstream->Alloc(sizeof(Impl::SmallBlockHeap), alignof(Impl::SmallBlockHeap));
        if (memory == nullptr)
        {
            return nullptr;
        }
        heap = new (memory) Impl::SmallBlockHeap(this);
        ScopedLock lock(m_mutex);
        heap->next = m_heaps;
        m_heaps = heap;
    }
    *slot = ThreadHeapEntry{m_id, this, heap};
    return heap;
}

void Opal::SmallBlockAllocator::RetireHeap(Impl::SmallBlockHeap* heap)
{
    FlushPending(*heap);
    ScopedLock lock(m_mutex);
    heap->next_idle = m_idle_heaps;
    m_idle_heaps = heap;
}

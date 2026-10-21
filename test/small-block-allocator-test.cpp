#include "test-helpers.h"

#include <cstring>
#include <optional>
#include <vector>

#include "opal/small-block-allocator.h"
#include "opal/threading/atomic.h"
#include "opal/threading/channel-mpmc.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"

using namespace Opal;

namespace
{

/** Thread-safe upstream that counts what passes through it, and can be told to fail. */
class CountingAllocator final : public AllocatorBase
{
public:
    CountingAllocator() : AllocatorBase("CountingAllocator") {}

    void* Alloc(u64 size, u64 alignment) override
    {
        if (m_should_fail.Load())
        {
            return nullptr;
        }
        m_alloc_count.FetchAdd(1);
        return m_malloc.Alloc(size, alignment);
    }

    void Free(void* ptr) override
    {
        if (ptr != nullptr)
        {
            m_free_count.FetchAdd(1);
        }
        m_malloc.Free(ptr);
    }

    [[nodiscard]] bool IsThreadSafe() const override { return true; }

    [[nodiscard]] u64 GetAllocCount() const { return m_alloc_count.Load(); }
    [[nodiscard]] u64 GetFreeCount() const { return m_free_count.Load(); }
    void SetShouldFail(bool should_fail) { m_should_fail.Store(should_fail); }

private:
    MallocAllocator m_malloc;
    Atomic<u64> m_alloc_count = 0;
    Atomic<u64> m_free_count = 0;
    Atomic<bool> m_should_fail = false;
};

template <typename Function>
ThreadHandle CreateThreadOrFail(Function&& function)
{
    Expected<ThreadHandle, ErrorCode> handle = CreateThread(std::forward<Function>(function));
    REQUIRE(handle.HasValue());
    return handle.GetValue();
}

void WaitFor(const Atomic<u32>& stage, u32 value)
{
    while (stage.Load() != value)
    {
        YieldCurrentThread();
    }
}

}  // namespace

TEST_CASE("Blocks are aligned and do not overlap", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    SmallBlockAllocator allocator(&upstream);

    constexpr u64 k_sizes[] = {0, 1, 8, 16, 17, 48, 100, 256, 500, 512, 513, 4000};
    constexpr u64 k_alignments[] = {1, 8, 16, 32, 64, 128};
    constexpr u64 k_count = 64;
    for (const u64 size : k_sizes)
    {
        for (const u64 alignment : k_alignments)
        {
            std::vector<u8*> blocks;
            for (u64 i = 0; i < k_count; i++)
            {
                auto* block = static_cast<u8*>(allocator.Alloc(size, alignment));
                REQUIRE(block != nullptr);
                REQUIRE(reinterpret_cast<u64>(block) % alignment == 0);
                std::memset(block, static_cast<int>(i), size);
                blocks.push_back(block);
            }
            u64 corrupted = 0;
            for (u64 i = 0; i < k_count; i++)
            {
                for (u64 byte = 0; byte < size; byte++)
                {
                    corrupted += blocks[i][byte] != static_cast<u8>(i) ? u64{1} : u64{0};
                }
            }
            CHECK(corrupted == 0);
            for (u8* block : blocks)
            {
                allocator.Free(block);
            }
        }
    }
}

TEST_CASE("Freeing nullptr does nothing", "[SmallBlockAllocator]")
{
    SmallBlockAllocator allocator;
    allocator.Free(nullptr);
    REQUIRE(allocator.IsThreadSafe());
    REQUIRE(std::strcmp(allocator.GetName(), "SmallBlockAllocator") == 0);
}

TEST_CASE("Small blocks are reused", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    SmallBlockAllocator allocator(&upstream);
    constexpr u64 k_count = 2000;
    std::vector<void*> blocks(k_count);

    SECTION("When freed on the same thread")
    {
        for (void*& block : blocks)
        {
            block = allocator.Alloc(48, 8);
        }
        for (void* block : blocks)
        {
            allocator.Free(block);
        }
        const u64 upstream_allocs = upstream.GetAllocCount();
        for (void*& block : blocks)
        {
            block = allocator.Alloc(48, 8);
        }
        REQUIRE(upstream.GetAllocCount() == upstream_allocs);
        for (void* block : blocks)
        {
            allocator.Free(block);
        }
    }
    SECTION("When freed on another thread")
    {
        for (void*& block : blocks)
        {
            block = allocator.Alloc(48, 8);
        }
        const ThreadHandle thread = CreateThreadOrFail(
            [&]
            {
                for (void* block : blocks)
                {
                    allocator.Free(block);
                }
            });
        JoinThread(thread);
        const u64 upstream_allocs = upstream.GetAllocCount();
        for (void*& block : blocks)
        {
            block = allocator.Alloc(48, 8);
        }
        REQUIRE(upstream.GetAllocCount() == upstream_allocs);
        for (void* block : blocks)
        {
            allocator.Free(block);
        }
    }
}

TEST_CASE("Large and over-aligned blocks come from upstream", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    SmallBlockAllocator allocator(&upstream);

    const u64 before = upstream.GetAllocCount();
    void* large = allocator.Alloc(SmallBlockAllocator::k_max_block_size + 1, 8);
    void* aligned = allocator.Alloc(16, SmallBlockAllocator::k_max_alignment * 2);
    REQUIRE(large != nullptr);
    REQUIRE(aligned != nullptr);
    REQUIRE(reinterpret_cast<u64>(aligned) % (SmallBlockAllocator::k_max_alignment * 2) == 0);
    REQUIRE(upstream.GetAllocCount() == before + 2);

    const u64 freed = upstream.GetFreeCount();
    allocator.Free(large);
    allocator.Free(aligned);
    REQUIRE(upstream.GetFreeCount() == freed + 2);
}

TEST_CASE("A thread's cache passes to the next thread", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    SmallBlockAllocator allocator(&upstream);
    auto use_and_exit = [&]
    {
        const ThreadHandle thread = CreateThreadOrFail(
            [&]
            {
                std::vector<void*> blocks(500);
                for (void*& block : blocks)
                {
                    block = allocator.Alloc(64, 16);
                }
                for (void* block : blocks)
                {
                    allocator.Free(block);
                }
            });
        JoinThread(thread);
    };

    use_and_exit();
    const u64 upstream_allocs = upstream.GetAllocCount();
    use_and_exit();
    REQUIRE(upstream.GetAllocCount() == upstream_allocs);
}

TEST_CASE("A failing upstream is reported as nullptr", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    upstream.SetShouldFail(true);
    SmallBlockAllocator allocator(&upstream);
    REQUIRE(allocator.Alloc(32, 8) == nullptr);
    REQUIRE(allocator.Alloc(4096, 8) == nullptr);

    upstream.SetShouldFail(false);
    void* block = allocator.Alloc(32, 8);
    REQUIRE(block != nullptr);
    allocator.Free(block);
}

TEST_CASE("Everything taken from upstream is returned", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    {
        SmallBlockAllocator allocator(&upstream);
        constexpr u32 k_thread_count = 4;
        std::vector<ThreadHandle> threads;
        for (u32 t = 0; t < k_thread_count; t++)
        {
            threads.push_back(CreateThreadOrFail(
                [&allocator, t]
                {
                    std::vector<void*> blocks;
                    for (u64 i = 0; i < 3000; i++)
                    {
                        blocks.push_back(allocator.Alloc(1 + (i * 7 + t) % 700, u64{1} << (i % 7)));
                    }
                    for (void* block : blocks)
                    {
                        allocator.Free(block);
                    }
                }));
        }
        for (const ThreadHandle& thread : threads)
        {
            JoinThread(thread);
        }
        void* block = allocator.Alloc(24, 8);
        allocator.Free(block);
    }
    REQUIRE(upstream.GetAllocCount() > 0);
    REQUIRE(upstream.GetFreeCount() == upstream.GetAllocCount());
}

TEST_CASE("A thread may outlive an allocator it used", "[SmallBlockAllocator]")
{
    CountingAllocator upstream;
    std::optional<SmallBlockAllocator> first(std::in_place, &upstream);
    std::optional<SmallBlockAllocator> second;
    Atomic<u32> stage = 0;
    bool second_worked = false;

    const ThreadHandle thread = CreateThreadOrFail(
        [&]
        {
            first->Free(first->Alloc(32, 8));
            stage.Store(1);
            WaitFor(stage, 2);
            void* block = second->Alloc(32, 8);
            second_worked = block != nullptr;
            second->Free(block);
        });

    WaitFor(stage, 1);
    first.reset();
    second.emplace(&upstream);
    stage.Store(2);
    JoinThread(thread);
    second.reset();

    REQUIRE(second_worked);
    REQUIRE(upstream.GetFreeCount() == upstream.GetAllocCount());
}

TEST_CASE("Blocks handed between threads keep their contents", "[SmallBlockAllocator]")
{
    SmallBlockAllocator allocator;
    constexpr u32 k_producer_count = 4;
    constexpr u32 k_consumer_count = 4;
    constexpr u64 k_items_per_producer = 20000;
    ChannelMPMC<u64*> channel(256);
    Atomic<u64> mismatches = 0;
    Atomic<u64> received = 0;
    Atomic<u32> producers_left = k_producer_count;

    std::vector<TransmitterMPMC<u64*>> transmitters;
    std::vector<ReceiverMPMC<u64*>> receivers;
    for (u32 p = 0; p < k_producer_count; p++)
    {
        transmitters.push_back(channel.transmitter.Clone());
    }
    for (u32 c = 0; c < k_consumer_count; c++)
    {
        receivers.push_back(channel.receiver.Clone());
    }

    std::vector<ThreadHandle> threads;
    for (u32 p = 0; p < k_producer_count; p++)
    {
        threads.push_back(CreateThreadOrFail(
            [&, p]
            {
                TransmitterMPMC<u64*>& tx = transmitters[p];
                for (u64 i = 0; i < k_items_per_producer; i++)
                {
                    const u64 words = 1 + i % 60;
                    auto* block = static_cast<u64*>(allocator.Alloc(words * sizeof(u64), alignof(u64)));
                    block[0] = words;
                    for (u64 w = 1; w < words; w++)
                    {
                        block[w] = p * k_items_per_producer + i + w;
                    }
                    tx.Send(block);
                }
                if (producers_left.FetchSub(1) == 1)
                {
                    tx.Close();
                }
            }));
    }
    for (u32 c = 0; c < k_consumer_count; c++)
    {
        threads.push_back(CreateThreadOrFail(
            [&, c]
            {
                ReceiverMPMC<u64*>& rx = receivers[c];
                while (true)
                {
                    auto result = rx.Receive();
                    if (!result.HasValue())
                    {
                        break;
                    }
                    u64* block = result.GetValue();
                    const u64 words = block[0];
                    for (u64 w = 2; w < words; w++)
                    {
                        mismatches.FetchAdd<MemoryOrder::Relaxed>(block[w] != block[w - 1] + 1 ? u64{1} : u64{0});
                    }
                    received.FetchAdd(1);
                    allocator.Free(block);
                }
            }));
    }
    for (const ThreadHandle& thread : threads)
    {
        JoinThread(thread);
    }

    REQUIRE(received.Load() == k_producer_count * k_items_per_producer);
    REQUIRE(mismatches.Load() == 0);
}

TEST_CASE("Serves the tasks of a ThreadPool", "[SmallBlockAllocator]")
{
    SmallBlockAllocator allocator;
    {
        Expected<ScopePtr<ThreadPool>, ErrorCode> created = ThreadPool::Create(4, 128, &allocator);
        REQUIRE(created.HasValue());
        ThreadPool& pool = *created.GetValue();

        constexpr u64 k_task_count = 5000;
        std::vector<Future<u64>> futures;
        for (u64 i = 0; i < k_task_count; i++)
        {
            auto submitted = pool.Submit([i] { return i; });
            REQUIRE(submitted.HasValue());
            futures.push_back(std::move(submitted).GetValue());
        }
        u64 sum = 0;
        for (Future<u64>& future : futures)
        {
            auto result = future.Get();
            REQUIRE(result.HasValue());
            sum += result.GetValue();
        }
        REQUIRE(sum == k_task_count * (k_task_count - 1) / 2);
    }
}

#include "test-helpers.h"

#include "opal/exceptions.h"
#include "opal/allocator.h"
#include "opal/threading/thread.h"

TEST_CASE("Malloc allocator", "[Allocator]")
{
    Opal::MallocAllocator allocator;

    SECTION("Basic alloc and free")
    {
        void* memory = allocator.Alloc(16, 16);
        REQUIRE(memory != nullptr);
        allocator.Free(memory);
    }
    SECTION("Alignment is respected")
    {
        constexpr Opal::u64 alignments[] = {8, 16, 32, 64, 128, 256};
        for (Opal::u64 alignment : alignments)
        {
            void* memory = allocator.Alloc(1, alignment);
            REQUIRE(memory != nullptr);
            REQUIRE(reinterpret_cast<Opal::u64>(memory) % alignment == 0);
            allocator.Free(memory);
        }
    }
    SECTION("Alignment with various sizes")
    {
        constexpr Opal::u64 sizes[] = {1, 3, 7, 13, 100, 1000};
        for (Opal::u64 size : sizes)
        {
            void* memory = allocator.Alloc(size, 64);
            REQUIRE(memory != nullptr);
            REQUIRE(reinterpret_cast<Opal::u64>(memory) % 64 == 0);
            allocator.Free(memory);
        }
    }
}

TEST_CASE("System memory allocator", "[Allocator]")
{
    SECTION("Working allocator")
    {
        Opal::SystemMemoryAllocator* allocator = nullptr;
        const Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
        REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc));
        REQUIRE(allocator != nullptr);
        REQUIRE(strcmp(allocator->GetName(), "System Memory Allocator") == 0);
        REQUIRE(allocator->GetCommitedSize() == OPAL_MB(100));

        const Opal::u64 page_size = allocator->GetPageSize();
        REQUIRE(page_size > 0);
        void* first_alloc = nullptr;
        REQUIRE_NOTHROW(first_alloc = allocator->Alloc(16, 16));
        REQUIRE(first_alloc != nullptr);
        REQUIRE(reinterpret_cast<Opal::u64>(first_alloc) % page_size == 0);
        void* second_alloc = nullptr;
        REQUIRE_NOTHROW(second_alloc = allocator->Alloc(16, 16));
        REQUIRE(second_alloc != nullptr);
        REQUIRE(reinterpret_cast<Opal::u64>(second_alloc) % page_size == 0);
        REQUIRE(reinterpret_cast<Opal::u64>(first_alloc) + page_size == reinterpret_cast<Opal::u64>(second_alloc));
        allocator->Reset();

        first_alloc = nullptr;
        REQUIRE_NOTHROW(first_alloc = allocator->Alloc(OPAL_MB(150), 16));
        REQUIRE(first_alloc != nullptr);
        REQUIRE(allocator->GetCommitedSize() == OPAL_MB(200));
        allocator->Commit(OPAL_MB(50));
        REQUIRE(allocator->GetCommitedSize() == OPAL_MB(250));

        delete allocator;
    }
    SECTION("Invalid reserved size")
    {
        Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = 0, .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_THROWS_AS(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc),
                          Opal::InvalidArgumentException);
        delete allocator;
    }
    SECTION("Initial commit size larger than reserved size")
    {
        Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = OPAL_MB(100), .bytes_to_initially_alloc = OPAL_MB(150), .commit_step_size = OPAL_MB(100)};
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_THROWS_AS(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc),
                          Opal::InvalidArgumentException);
        delete allocator;
    }
    SECTION("Not enough reserved memory")
    {
        Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = OPAL_MB(10), .bytes_to_initially_alloc = OPAL_MB(1), .commit_step_size = OPAL_MB(1)};
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc));
        REQUIRE(allocator != nullptr);

        void* first_alloc = nullptr;
        REQUIRE_THROWS_AS(first_alloc = allocator->Alloc(OPAL_MB(15), 16), Opal::OutOfMemoryException);
        REQUIRE(first_alloc == nullptr);

        delete allocator;
    }
}

TEST_CASE("Linear allocator", "[Allocator]")
{
    const Opal::SystemMemoryAllocatorDesc desc{
        .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
    Opal::LinearAllocator* allocator = nullptr;
    REQUIRE_NOTHROW(allocator = new Opal::LinearAllocator("Linear Allocator", desc));
    void* memory = allocator->Alloc(16, 8);
    REQUIRE(memory != nullptr);
    allocator->Free(memory);
    allocator->Reset();
    void* memory2 = allocator->Alloc(1024, 8);
    REQUIRE(memory2 != nullptr);
    REQUIRE(memory == memory2);
    delete allocator;
}

TEST_CASE("Null allocator", "[Allocator]")
{
    Opal::NullAllocator allocator;
    void* memory = nullptr;
    REQUIRE_THROWS_AS(memory = allocator.Alloc(16, 16), Opal::OutOfMemoryException);
    REQUIRE(memory == nullptr);
    allocator.Free(memory);
}

TEST_CASE("Default allocator", "[Allocator]")
{
    // MallocAllocator is pushed in main-test.cpp before tests run.
    Opal::AllocatorBase* allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "MallocAllocator") == 0);
    void* memory = allocator->Alloc(16, 16);
    REQUIRE(memory != nullptr);
    allocator->Free(memory);

    const Opal::SystemMemoryAllocatorDesc desc{
        .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
    Opal::LinearAllocator* linear_allocator = nullptr;
    REQUIRE_NOTHROW(linear_allocator = new Opal::LinearAllocator("Linear Allocator", desc));
    Opal::PushDefault pd(linear_allocator);
    allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "Linear Allocator") == 0);
    void* memory2 = allocator->Alloc(16, 16);
    REQUIRE(memory2 != nullptr);
    REQUIRE(memory != memory2);

    // Pushing nullptr should push the root allocator (MallocAllocator from main).
    Opal::PushDefault pd2(nullptr);
    allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "MallocAllocator") == 0);
    void* memory3 = allocator->Alloc(16, 16);
    REQUIRE(memory3 != nullptr);
    allocator->Free(memory3);
    delete linear_allocator;
}

TEST_CASE("New and delete", "[Allocator]")
{
    Opal::SystemMemoryAllocator* allocator = nullptr;
    const Opal::SystemMemoryAllocatorDesc desc{
        .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
    REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc));
    Opal::PushDefault pd(allocator);
    int* a = Opal::New<int>(Opal::GetDefaultAllocator(), 5);
    REQUIRE(a != nullptr);
    REQUIRE(*a == 5);
    Opal::Delete(Opal::GetDefaultAllocator(), a);

    int* b = Opal::New<int, 32>(Opal::GetDefaultAllocator(),10);
    REQUIRE(b != nullptr);
    REQUIRE(*b == 10);
    REQUIRE(reinterpret_cast<Opal::u64>(b) % 32 == 0);
    Opal::Delete(Opal::GetDefaultAllocator(), b);
    delete allocator;
}

TEST_CASE("Allocator not initialized exception", "[Allocator]")
{
    // Pop until we can't pop anymore (size <= 1 is protected).
    Opal::PopDefaultAllocator();

    // GetDefaultAllocator should still work since the root is protected.
    REQUIRE_NOTHROW(Opal::GetDefaultAllocator());

    // Push a new allocator and verify it works, then pop it.
    Opal::MallocAllocator temp_allocator;
    Opal::PushDefaultAllocator(&temp_allocator);
    REQUIRE(std::strcmp(Opal::GetDefaultAllocator()->GetName(), "MallocAllocator") == 0);
    Opal::PopDefaultAllocator();
}

struct ThreadAllocatorResult
{
    bool threw = false;
    Opal::AllocatorBase* allocator = nullptr;
};

TEST_CASE("Thread-local allocator stacks", "[Allocator]")
{
    SECTION("New thread has empty allocator stack")
    {
        ThreadAllocatorResult result;
        Opal::ThreadHandle handle = Opal::CreateThread(
            [](ThreadAllocatorResult& out)
            {
                try
                {
                    Opal::GetDefaultAllocator();
                }
                catch (const Opal::AllocatorNotInitializedException&)
                {
                    out.threw = true;
                }
            },
            Opal::Ref(result));
        Opal::JoinThread(handle);
        REQUIRE(result.threw);
    }
    SECTION("Push on child thread does not affect main thread")
    {
        Opal::AllocatorBase* main_allocator = Opal::GetDefaultAllocator();

        Opal::MallocAllocator child_allocator;
        ThreadAllocatorResult result;
        Opal::ThreadHandle handle = Opal::CreateThread(
            [](Opal::MallocAllocator& alloc, ThreadAllocatorResult& out)
            {
                Opal::PushDefaultAllocator(&alloc);
                out.allocator = Opal::GetDefaultAllocator();
                Opal::PopDefaultAllocator();
            },
            Opal::Ref(child_allocator), Opal::Ref(result));
        Opal::JoinThread(handle);

        REQUIRE(result.allocator != nullptr);
        REQUIRE(std::strcmp(result.allocator->GetName(), "MallocAllocator") == 0);
        REQUIRE(Opal::GetDefaultAllocator() == main_allocator);
    }
    SECTION("Push on main thread does not affect child thread")
    {
        // Create thread before pushing LinearAllocator, since CreateThread requires a thread-safe default allocator.
        ThreadAllocatorResult result;
        Opal::ThreadHandle handle = Opal::CreateThread(
            [](ThreadAllocatorResult& out)
            {
                try
                {
                    Opal::GetDefaultAllocator();
                }
                catch (const Opal::AllocatorNotInitializedException&)
                {
                    out.threw = true;
                }
            },
            Opal::Ref(result));
        Opal::JoinThread(handle);
        REQUIRE(result.threw);

        // Verify that main thread's stack is unaffected.
        const Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(1), .commit_step_size = OPAL_MB(1)};
        Opal::LinearAllocator linear("Thread Test Allocator", desc);
        Opal::PushDefault pd(&linear);
        REQUIRE(std::strcmp(Opal::GetDefaultAllocator()->GetName(), "Thread Test Allocator") == 0);
    }
    SECTION("Each child thread has independent stack")
    {
        Opal::MallocAllocator alloc1;
        const Opal::SystemMemoryAllocatorDesc desc{
            .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(1), .commit_step_size = OPAL_MB(1)};
        Opal::LinearAllocator alloc2("Child Linear Allocator", desc);

        ThreadAllocatorResult result1;
        ThreadAllocatorResult result2;

        Opal::ThreadHandle handle1 = Opal::CreateThread(
            [](Opal::MallocAllocator& alloc, ThreadAllocatorResult& out)
            {
                Opal::PushDefaultAllocator(&alloc);
                out.allocator = Opal::GetDefaultAllocator();
                Opal::PopDefaultAllocator();
            },
            Opal::Ref(alloc1), Opal::Ref(result1));

        Opal::ThreadHandle handle2 = Opal::CreateThread(
            [](Opal::LinearAllocator& alloc, ThreadAllocatorResult& out)
            {
                Opal::PushDefaultAllocator(&alloc);
                out.allocator = Opal::GetDefaultAllocator();
                Opal::PopDefaultAllocator();
            },
            Opal::Ref(alloc2), Opal::Ref(result2));

        Opal::JoinThread(handle1);
        Opal::JoinThread(handle2);

        REQUIRE(result1.allocator == &alloc1);
        REQUIRE(result2.allocator == &alloc2);
    }
}

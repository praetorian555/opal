#include "test-helpers.h"

#include "opal/exceptions.h"
#include "opal/allocator.h"

TEST_CASE("Malloc allocator", "[Allocator]")
{
    Opal::MallocAllocator allocator;
    void* memory = allocator.Alloc(16, 16);
    REQUIRE(memory != nullptr);
    allocator.Free(memory);
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
}

TEST_CASE("Null allocator", "[Allocator]")
{
    Opal::NullAllocator allocator;
    void* memory = allocator.Alloc(16, 16);
    REQUIRE(memory == nullptr);
    allocator.Free(memory);
}

TEST_CASE("Default allocator", "[Allocator]")
{
    Opal::AllocatorBase* allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "MallocAllocator") == 0);
    void* memory = allocator->Alloc(16, 16);
    REQUIRE(memory != nullptr);
    allocator->Free(memory);

    const Opal::SystemMemoryAllocatorDesc desc{
        .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
    Opal::LinearAllocator* linear_allocator = nullptr;
    REQUIRE_NOTHROW(linear_allocator = new Opal::LinearAllocator("Linear Allocator", desc));
    Opal::SetDefaultAllocator(linear_allocator);
    allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "Linear Allocator") == 0);
    void* memory2 = allocator->Alloc(16, 16);
    REQUIRE(memory2 != nullptr);
    REQUIRE(memory != memory2);

    Opal::SetDefaultAllocator(nullptr);
    allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "MallocAllocator") == 0);
    void* memory3 = allocator->Alloc(16, 16);
    REQUIRE(memory3 != nullptr);
    allocator->Free(memory3);
}

TEST_CASE("New and delete", "[Allocator]")
{
    Opal::SystemMemoryAllocator* allocator = nullptr;
    const Opal::SystemMemoryAllocatorDesc desc{
        .bytes_to_reserve = OPAL_GB(1), .bytes_to_initially_alloc = OPAL_MB(100), .commit_step_size = OPAL_MB(100)};
    REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator("System Memory Allocator", desc));
    int* a = Opal::New<int>(allocator, 5);
    REQUIRE(a != nullptr);
    REQUIRE(*a == 5);
    Opal::Delete(allocator, a);

    int* b = Opal::New<int>(32, allocator, 10);
    REQUIRE(b != nullptr);
    REQUIRE(*b == 10);
    REQUIRE(reinterpret_cast<Opal::u64>(b) % 32 == 0);
    Opal::Delete(allocator, b);
}

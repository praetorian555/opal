#include "opal/defines.h"
#include "opal/exceptions.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

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
        REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator(OPAL_GB(1), OPAL_MB(100), "System Memory Allocator"));
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
        REQUIRE(allocator->GetCommitedSize() == OPAL_MB(150));
        allocator->Commit(OPAL_MB(50));
        REQUIRE(allocator->GetCommitedSize() == OPAL_MB(200));

        delete allocator;
    }
    SECTION("Invalid reserved size")
    {
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_THROWS_AS(allocator = new Opal::SystemMemoryAllocator(0, OPAL_MB(100), "System Memory Allocator"), Opal::InvalidArgumentException);
        delete allocator;
    }
    SECTION("Initial commit size larger than reserved size")
    {
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_THROWS_AS(allocator = new Opal::SystemMemoryAllocator(OPAL_MB(100), OPAL_MB(150), "System Memory Allocator"), Opal::InvalidArgumentException);
        delete allocator;
    }
    SECTION("Not enough reserved memory")
    {
        Opal::SystemMemoryAllocator* allocator = nullptr;
        REQUIRE_NOTHROW(allocator = new Opal::SystemMemoryAllocator(OPAL_MB(10), OPAL_MB(1), "System Memory Allocator"));
        REQUIRE(allocator != nullptr);

        void* first_alloc = nullptr;
        REQUIRE_THROWS_AS(first_alloc = allocator->Alloc(OPAL_MB(15), 16), Opal::OutOfMemoryException);
        REQUIRE(first_alloc == nullptr);

        delete allocator;
    }
}

TEST_CASE("Linear allocator", "[Allocator]")
{
    Opal::LinearAllocator allocator(1024);
    void* memory = allocator.Alloc(16, 8);
    REQUIRE(memory != nullptr);
    allocator.Free(memory);
    allocator.Reset();
    void* memory2 = allocator.Alloc(1024, 8);
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

    Opal::LinearAllocator linear_allocator(1024);
    Opal::SetDefaultAllocator(&linear_allocator);
    allocator = Opal::GetDefaultAllocator();
    REQUIRE(std::strcmp(allocator->GetName(), "LinearAllocator") == 0);
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
    Opal::LinearAllocator allocator(1024);
    Opal::AllocatorBase* allocator_ptr = &allocator;
    int* a = Opal::New<int>(allocator_ptr, 5);
    REQUIRE(a != nullptr);
    REQUIRE(*a == 5);
    Opal::Delete(allocator_ptr, a);

    int* b = Opal::New<int>(32, allocator_ptr, 10);
    REQUIRE(b != nullptr);
    REQUIRE(*b == 10);
    REQUIRE(reinterpret_cast<Opal::u64>(b) % 32 == 0);
    Opal::Delete(allocator_ptr, b);
}

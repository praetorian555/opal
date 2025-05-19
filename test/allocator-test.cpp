#include "opal/defines.h"

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

#include "catch2/catch2.hpp"

#include "opal/allocator.h"

TEST_CASE("Default allocator", "[Allocator]")
{
    Opal::DefaultAllocator allocator;
    void* memory = allocator.Alloc(16, 16);
    REQUIRE(memory != nullptr);
    allocator.Free(memory);
}

TEST_CASE("Linear allocator", "[Allocator]")
{
    Opal::LinearAllocator allocator(1024);
    void* memory = allocator.Allocate(16);
    REQUIRE(memory != nullptr);
    allocator.Deallocate(memory);
    allocator.Reset();
    void* memory2 = allocator.Allocate(1024);
    REQUIRE(memory2 != nullptr);
    REQUIRE(memory == memory2);
}
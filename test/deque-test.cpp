#include "catch2/catch2.hpp"

#include "opal/container/deque.h"

using namespace Opal;

TEST_CASE("Constructor", "[Deque]")
{
    SECTION("Default constructor")
    {
        Deque<i32> deque;
        REQUIRE(deque.GetCapacity() == 4);
        REQUIRE(deque.GetSize() == 0);
    }
    SECTION("Default constructor with allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(allocator);
        REQUIRE(deque.GetCapacity() == 4);
        REQUIRE(deque.GetSize() == 0);
    }
    SECTION("Default constructor with move allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(Move(allocator));
        REQUIRE(deque.GetCapacity() == 4);
        REQUIRE(deque.GetSize() == 0);
    }
    SECTION("Constructor with count")
    {
        Deque<i32> deque(5);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 0);
        REQUIRE(deque[1] == 0);
        REQUIRE(deque[2] == 0);
        REQUIRE(deque[3] == 0);
        REQUIRE(deque[4] == 0);
    }
    SECTION("Constructor with count and allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(5, allocator);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 0);
        REQUIRE(deque[1] == 0);
        REQUIRE(deque[2] == 0);
        REQUIRE(deque[3] == 0);
        REQUIRE(deque[4] == 0);
    }
    SECTION("Constructor with count and move allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(5, Move(allocator));
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 0);
        REQUIRE(deque[1] == 0);
        REQUIRE(deque[2] == 0);
        REQUIRE(deque[3] == 0);
        REQUIRE(deque[4] == 0);
    }
    SECTION("Constructor with count and default value")
    {
        Deque<i32> deque(5, 10);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Constructor with count, default value and allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(5, 10, allocator);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Constructor with count, default value and move allocator")
    {
        DefaultAllocator allocator;
        Deque<i32> deque(5, 10, Move(allocator));
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Copy constructor")
    {
        Deque<i32> deque(5, 10);
        Deque<i32> copy(deque);
        REQUIRE(copy.GetCapacity() == 8);
        REQUIRE(copy.GetSize() == 5);
        REQUIRE(copy[0] == 10);
        REQUIRE(copy[1] == 10);
        REQUIRE(copy[2] == 10);
        REQUIRE(copy[3] == 10);
        REQUIRE(copy[4] == 10);
    }
    SECTION("Move constructor")
    {
        Deque<i32> deque(5, 10);
        Deque<i32> other(Move(deque));
        REQUIRE(other.GetCapacity() == 8);
        REQUIRE(other.GetSize() == 5);
        REQUIRE(other[0] == 10);
        REQUIRE(other[1] == 10);
        REQUIRE(other[2] == 10);
        REQUIRE(other[3] == 10);
        REQUIRE(other[4] == 10);
        REQUIRE(deque.GetCapacity() == 0);
    }
}

TEST_CASE("Assignment", "[Deque]")
{
    SECTION("Copy assignment")
    {
        Deque<i32> deque(5, 10);
        Deque<i32> copy;
        copy = deque;
        REQUIRE(copy.GetCapacity() == 8);
        REQUIRE(copy.GetSize() == 5);
        REQUIRE(copy[0] == 10);
        REQUIRE(copy[1] == 10);
        REQUIRE(copy[2] == 10);
        REQUIRE(copy[3] == 10);
        REQUIRE(copy[4] == 10);
    }
    SECTION("Move assignment")
    {
        Deque<i32> deque(5, 10);
        Deque<i32> other;
        other = Move(deque);
        REQUIRE(other.GetCapacity() == 8);
        REQUIRE(other.GetSize() == 5);
        REQUIRE(other[0] == 10);
        REQUIRE(other[1] == 10);
        REQUIRE(other[2] == 10);
        REQUIRE(other[3] == 10);
        REQUIRE(other[4] == 10);
        REQUIRE(deque.GetCapacity() == 0);
    }
}

TEST_CASE("Is empty", "[Deque]")
{
    SECTION("Empty")
    {
        Deque<i32> deque;
        REQUIRE(deque.IsEmpty());
    }
    SECTION("Not empty")
    {
        Deque<i32> deque(4);
        REQUIRE(!deque.IsEmpty());
    }
}

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

TEST_CASE("Equality", "[Deque]")
{
    SECTION("Equal")
    {
        Deque<i32> deque1(5, 10);
        Deque<i32> deque2(5, 10);
        REQUIRE(deque1 == deque2);
    }
    SECTION("Not equal in size")
    {
        Deque<i32> deque1(5, 10);
        Deque<i32> deque2(4, 10);
        REQUIRE(deque1 != deque2);
    }
    SECTION("Not equal in elements")
    {
        Deque<i32> deque1(5, 10);
        Deque<i32> deque2(5, 20);
        REQUIRE(deque1 != deque2);
    }
}

TEST_CASE("Assign", "[Deque]")
{
    SECTION("with count")
    {
        Deque<i32> deque;
        ErrorCode err = deque.Assign(5);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 0);
        REQUIRE(deque[1] == 0);
        REQUIRE(deque[2] == 0);
        REQUIRE(deque[3] == 0);
        REQUIRE(deque[4] == 0);
    }
    SECTION("with count and value")
    {
        Deque<i32> deque;
        ErrorCode err = deque.Assign(5, 10);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("with iterators")
    {
        Deque<i32> deque;
        const i32 values[] = {1, 2, 3, 4, 5};
        ErrorCode err = deque.AssignIt(values, values + 5);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 1);
        REQUIRE(deque[1] == 2);
        REQUIRE(deque[2] == 3);
        REQUIRE(deque[3] == 4);
        REQUIRE(deque[4] == 5);
    }
}

TEST_CASE("At", "[Deque]")
{
    SECTION("Good access")
    {
        Deque<i32> deque(5, 10);
        REQUIRE(deque.At(0).GetValue() == 10);
        REQUIRE(deque.At(1).GetValue() == 10);
        REQUIRE(deque.At(2).GetValue() == 10);
        REQUIRE(deque.At(3).GetValue() == 10);
        REQUIRE(deque.At(4).GetValue() == 10);
    }
    SECTION("Modify with good access")
    {
        Deque<i32> deque(5, 10);
        deque.At(0).GetValue() = 20;
        deque.At(1).GetValue() = 30;
        deque.At(2).GetValue() = 40;
        deque.At(3).GetValue() = 50;
        deque.At(4).GetValue() = 60;
        REQUIRE(deque.At(0).GetValue() == 20);
        REQUIRE(deque.At(1).GetValue() == 30);
        REQUIRE(deque.At(2).GetValue() == 40);
        REQUIRE(deque.At(3).GetValue() == 50);
        REQUIRE(deque.At(4).GetValue() == 60);
    }
    SECTION("Good access const")
    {
        const Deque<i32> deque(5, 10);
        REQUIRE(deque.At(0).GetValue() == 10);
        REQUIRE(deque.At(1).GetValue() == 10);
        REQUIRE(deque.At(2).GetValue() == 10);
        REQUIRE(deque.At(3).GetValue() == 10);
        REQUIRE(deque.At(4).GetValue() == 10);
    }
    SECTION("Bad access")
    {
        Deque<i32> deque(5, 10);
        REQUIRE(deque.At(5).HasValue() == false);
        REQUIRE(deque.At(5).GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Bad access const")
    {
        const Deque<i32> deque(5, 10);
        REQUIRE(deque.At(5).HasValue() == false);
        REQUIRE(deque.At(5).GetError() == ErrorCode::OutOfBounds);
    }
}

TEST_CASE("Front access", "[Deque]")
{
    SECTION("Empty deque")
    {
        Deque<i32> deque;
        REQUIRE(deque.Front().HasValue() == false);
        REQUIRE(deque.Front().GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Const empty deque")
    {
        const Deque<i32> deque;
        REQUIRE(deque.Front().HasValue() == false);
        REQUIRE(deque.Front().GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Non-empty deque")
    {
        Deque<i32> deque(5, 10);
        deque.At(0).GetValue() = 20;
        REQUIRE(deque.Front().GetValue() == 20);
    }
    SECTION("Non-empty const deque")
    {
        const Deque<i32> deque(5, 10);
        REQUIRE(deque.Front().GetValue() == 10);
    }
}

TEST_CASE("Back access", "[Deque]")
{
    SECTION("Empty deque")
    {
        Deque<i32> deque;
        REQUIRE(deque.Back().HasValue() == false);
        REQUIRE(deque.Back().GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Empty const deque")
    {
        const Deque<i32> deque;
        REQUIRE(deque.Back().HasValue() == false);
        REQUIRE(deque.Back().GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Non-empty deque")
    {
        Deque<i32> deque(5, 10);
        deque.At(4).GetValue() = 20;
        REQUIRE(deque.Back().GetValue() == 20);
    }
    SECTION("Non-empty const deque")
    {
        const Deque<i32> deque(5, 10);
        REQUIRE(deque.Back().GetValue() == 10);
    }
}

TEST_CASE("Clear", "[Deque]")
{
    Deque<i32> deque(5, 10);
    deque.Clear();
    REQUIRE(deque.GetCapacity() == 8);
    REQUIRE(deque.GetSize() == 0);
}

TEST_CASE("Reserve", "[Deque]")
{
    SECTION("Increase capacity")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Reserve(10);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Decrease capacity")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Reserve(2);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
}

TEST_CASE("Resize", "[Deque]")
{
    SECTION("New size smaller then old size")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Resize(2);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 2);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
    }
    SECTION("New size same as the old size")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Resize(5);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("New size larger then old size but smaller then capacity")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Resize(7);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 7);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 0);
        REQUIRE(deque[6] == 0);
    }
    SECTION("New size larger then capacity")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Resize(10);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 10);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 0);
        REQUIRE(deque[6] == 0);
        REQUIRE(deque[7] == 0);
        REQUIRE(deque[8] == 0);
        REQUIRE(deque[9] == 0);
    }
}

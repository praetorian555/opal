#include "test-helpers.h"

#include "opal/container/deque.h"

using namespace Opal;

struct Data
{
    Data(i32 val) { value = new i32(val); }
    Data() { value = new i32(5); }
    ~Data() { delete value; }

    Data(const Data& other) { value = new i32(*other.value); }
    Data& operator=(const Data& other)
    {
        if (this != &other)
        {
            delete value;
            value = new i32(*other.value);
        }
        return *this;
    }

    Data(Data&& other) noexcept : value(other.value) { other.value = nullptr; }
    Data& operator=(Data&& other) noexcept
    {
        if (this != &other)
        {
            delete value;
            value = other.value;
            other.value = nullptr;
        }
        return *this;
    }

    i32* value;
};

TEST_CASE("Constructor", "[Deque]")
{
    SECTION("Default constructor")
    {
        Deque<i32> deque;
        REQUIRE(deque.GetCapacity() == 0);
        REQUIRE(deque.GetSize() == 0);
    }
    SECTION("Default constructor with allocator")
    {
        MallocAllocator allocator;
        Deque<i32> deque(allocator);
        REQUIRE(deque.GetCapacity() == 0);
        REQUIRE(deque.GetSize() == 0);
    }
    SECTION("Default constructor with move allocator")
    {
        MallocAllocator allocator;
        Deque<i32> deque(Move(allocator));
        REQUIRE(deque.GetCapacity() == 0);
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
        MallocAllocator allocator;
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
        MallocAllocator allocator;
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
        MallocAllocator allocator;
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
        MallocAllocator allocator;
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

TEST_CASE("Push front", "[Deque]")
{
    SECTION("Empty deque")
    {
        Deque<i32> deque;
        ErrorCode err = deque.PushFront(10);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 1);
        REQUIRE(deque.GetSize() == 1);
        REQUIRE(deque[0] == 10);
    }
    SECTION("Non-empty deque")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.PushFront(20);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Triggers resize")
    {
        Deque<i32> deque(8, 10);
        ErrorCode err = deque.PushFront(20);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 9);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
        REQUIRE(deque[8] == 10);
    }
    SECTION("Empty deque with a copy")
    {
        Deque<i32> deque;
        i32 value = 10;
        ErrorCode err = deque.PushFront(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 1);
        REQUIRE(deque.GetSize() == 1);
        REQUIRE(deque[0] == 10);
    }
    SECTION("Non-empty deque with a copy")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        ErrorCode err = deque.PushFront(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Triggers resize with a copy")
    {
        Deque<i32> deque(8, 10);
        i32 value = 20;
        ErrorCode err = deque.PushFront(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 9);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
        REQUIRE(deque[8] == 10);
    }
}

TEST_CASE("Push back", "[Deque]")
{
    SECTION("Empty")
    {
        Deque<i32> deque;
        ErrorCode err = deque.PushBack(10);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 1);
        REQUIRE(deque.GetSize() == 1);
        REQUIRE(deque[0] == 10);
    }
    SECTION("Empty copy")
    {
        Deque<i32> deque;
        i32 value = 10;
        ErrorCode err = deque.PushBack(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 1);
        REQUIRE(deque.GetSize() == 1);
        REQUIRE(deque[0] == 10);
    }
    SECTION("Non-empty deque")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.PushBack(20);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
    }
    SECTION("Non-empty deque copy")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        ErrorCode err = deque.PushBack(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
    }
    SECTION("Triggers resize")
    {
        Deque<i32> deque(8, 10);
        ErrorCode err = deque.PushBack(20);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 9);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
        REQUIRE(deque[8] == 20);
    }
    SECTION("Triggers resize copy")
    {
        Deque<i32> deque(8, 10);
        i32 value = 20;
        ErrorCode err = deque.PushBack(value);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 16);
        REQUIRE(deque.GetSize() == 9);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
        REQUIRE(deque[8] == 20);
    }
}

TEST_CASE("Pop front and back", "[Deque]")
{
    SECTION("Pop front")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.PopFront();
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Pop front empty")
    {
        Deque<i32> deque;
        ErrorCode err = deque.PopFront();
        REQUIRE(err == ErrorCode::OutOfBounds);
    }
    SECTION("Pop back")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.PopBack();
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Pop back empty")
    {
        Deque<i32> deque;
        ErrorCode err = deque.PopBack();
        REQUIRE(err == ErrorCode::OutOfBounds);
    }
}

TEST_CASE("Iterator", "[Deque]")
{
    SECTION("Difference")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it1 = deque.Begin();
        Deque<i32>::IteratorType it2 = deque.End();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == deque.End());
    }
    SECTION("Post increment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Deque<i32>::IteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == deque.End());
    }
    SECTION("Decrement")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.End();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == deque.Begin());
    }
    SECTION("Post decrement")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.End();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Deque<i32>::IteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == deque.Begin());
    }
    SECTION("Add")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == deque.End());

        Deque<i32>::IteratorType it2 = deque.Begin();
        REQUIRE((3 + it2) == deque.End());
    }
    SECTION("Add assignment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == deque.End());
    }
    SECTION("Subtract")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.End();
        REQUIRE((it - 0) == deque.End());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == deque.Begin());
    }
    SECTION("Subtract assignment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.End();
        REQUIRE((it -= 0) == deque.End());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == deque.Begin());
    }
    SECTION("Access")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it = deque.Begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Deque<Data> deque(3, Data());
        Deque<Data>::IteratorType it = deque.Begin();
        REQUIRE(*(it->value) == 5);
    }
    SECTION("Compare")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::IteratorType it1 = deque.Begin();
        Deque<i32>::IteratorType it2 = deque.Begin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        Deque<i32> deque(3, 42);
        i32 sum = 0;
        for (Deque<i32>::IteratorType it = deque.Begin(); it != deque.End(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        Deque<i32> deque(3, 42);
        i32 sum = 0;
        for (i32 val : deque)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Const iterator", "[Deque]")
{
    SECTION("Difference")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it1 = deque.ConstBegin();
        Deque<i32>::ConstIteratorType it2 = deque.ConstEnd();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == deque.ConstEnd());
    }
    SECTION("Post increment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Deque<i32>::ConstIteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == deque.ConstEnd());
    }
    SECTION("Decrement")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstEnd();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == deque.ConstBegin());
    }
    SECTION("Post decrement")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstEnd();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Deque<i32>::ConstIteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == deque.ConstBegin());
    }
    SECTION("Add")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == deque.ConstEnd());

        Deque<i32>::ConstIteratorType it2 = deque.ConstBegin();
        REQUIRE((3 + it2) == deque.ConstEnd());
    }
    SECTION("Add assignment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == deque.ConstEnd());
    }
    SECTION("Subtract")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstEnd();
        REQUIRE((it - 0) == deque.ConstEnd());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == deque.ConstBegin());
    }
    SECTION("Subtract assignment")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstEnd();
        REQUIRE((it -= 0) == deque.ConstEnd());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == deque.ConstBegin());
    }
    SECTION("Access")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Deque<Data> deque(3, Data());
        Deque<Data>::ConstIteratorType it = deque.ConstBegin();
        REQUIRE(*(it->value) == 5);
    }
    SECTION("Compare")
    {
        Deque<i32> deque(3, 42);
        Deque<i32>::ConstIteratorType it1 = deque.ConstBegin();
        Deque<i32>::ConstIteratorType it2 = deque.ConstBegin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        Deque<i32> deque(3, 42);
        i32 sum = 0;
        for (Deque<i32>::ConstIteratorType it = deque.ConstBegin(); it != deque.ConstEnd(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        const Deque<i32> deque(3, 42);
        i32 sum = 0;
        for (const i32& val : deque)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Insert single", "[Deque]")
{
    SECTION("Insert good copy front")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin(), value).GetValue();
        REQUIRE(deque.Begin() == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Insert good copy mid")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin() + 2, value).GetValue();
        REQUIRE(deque.Begin() + 2 == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 20);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Insert good copy back")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstEnd(), value).GetValue();
        REQUIRE(deque.End() - 1 == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
    }
    SECTION("Insert good move front")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin(), 20).GetValue();
        REQUIRE(deque.Begin() == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Insert good move mid")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin() + 2, 20).GetValue();
        REQUIRE(deque.Begin() + 2 == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 20);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
    }
    SECTION("Insert good move back")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstEnd(), 20).GetValue();
        REQUIRE(deque.End() - 1 == it);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 6);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
    }
    SECTION("Insert bad copy")
    {
        Deque<i32> deque(5, 10);
        i32 value = 20;
        ErrorCode err = deque.Insert(deque.ConstEnd() + 1, value).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Insert bad move")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Insert(deque.ConstEnd() + 1, 20).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
}

TEST_CASE("Insert multiple", "[Deque]")
{
    SECTION("Bad position")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Insert(deque.ConstEnd() + 1, 3, 20).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Zero count")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstEnd(), 0, 20).GetValue();
        REQUIRE(it == deque.End());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Insert front")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin(), 3, 20).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 20);
        REQUIRE(deque[2] == 20);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
    }
    SECTION("Insert mid")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstBegin() + 2, 3, 20).GetValue();
        REQUIRE(it == deque.Begin() + 2);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 20);
        REQUIRE(deque[3] == 20);
        REQUIRE(deque[4] == 20);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
    }
    SECTION("Insert back")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Insert(deque.ConstEnd(), 3, 20).GetValue();
        REQUIRE(it == deque.End() - 3);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
        REQUIRE(deque[6] == 20);
        REQUIRE(deque[7] == 20);
    }
}

TEST_CASE("Insert multiple with iterators", "[Deque]")
{
    SECTION("Bad position")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.InsertIt(deque.ConstEnd() + 1, deque.ConstBegin(), deque.ConstEnd()).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Bad input")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.InsertIt(deque.ConstEnd(), deque.ConstEnd(), deque.ConstBegin()).GetError();
        REQUIRE(err == ErrorCode::InvalidArgument);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Zero count")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.InsertIt(deque.ConstEnd(), deque.ConstBegin(), deque.ConstBegin()).GetValue();
        REQUIRE(it == deque.End());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Insert front")
    {
        Deque<i32> deque(5, 10);
        i32 arr[3] = {20, 30, 40};
        Deque<i32>::IteratorType it = deque.InsertIt(deque.ConstBegin(), arr, arr + 3).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 20);
        REQUIRE(deque[1] == 30);
        REQUIRE(deque[2] == 40);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
    }
    SECTION("Insert mid")
    {
        Deque<i32> deque(5, 10);
        i32 arr[3] = {20, 30, 40};
        Deque<i32>::IteratorType it = deque.InsertIt(deque.ConstBegin() + 2, arr, arr + 3).GetValue();
        REQUIRE(it == deque.Begin() + 2);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 20);
        REQUIRE(deque[3] == 30);
        REQUIRE(deque[4] == 40);
        REQUIRE(deque[5] == 10);
        REQUIRE(deque[6] == 10);
        REQUIRE(deque[7] == 10);
    }
    SECTION("Insert back")
    {
        Deque<i32> deque(5, 10);
        i32 arr[3] = {20, 30, 40};
        Deque<i32>::IteratorType it = deque.InsertIt(deque.ConstEnd(), arr, arr + 3).GetValue();
        REQUIRE(it == deque.End() - 3);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 8);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
        REQUIRE(deque[5] == 20);
        REQUIRE(deque[6] == 30);
        REQUIRE(deque[7] == 40);
    }
}

TEST_CASE("Erase single", "[Deque]")
{
    SECTION("Out of bounds")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Erase(deque.ConstEnd()).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Erase front")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin()).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Erase mid")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin() + 2).GetValue();
        REQUIRE(it == deque.Begin() + 2);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Erase back")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstEnd() - 1).GetValue();
        REQUIRE(it == deque.End() - 1);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Out of bounds non-const iterator")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Erase(deque.End()).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Erase front non-const iterator")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.Begin()).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Erase mid non-const iterator")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.Begin() + 2).GetValue();
        REQUIRE(it == deque.Begin() + 2);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
    SECTION("Erase back non-const iterator")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.End() - 1).GetValue();
        REQUIRE(it == deque.End() - 1);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 4);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
    }
}

TEST_CASE("Erase multiple", "[Deque]")
{
    SECTION("Out of bounds")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Erase(deque.ConstEnd(), deque.ConstEnd() + 1).GetError();
        REQUIRE(err == ErrorCode::OutOfBounds);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Bad input")
    {
        Deque<i32> deque(5, 10);
        ErrorCode err = deque.Erase(deque.ConstEnd(), deque.ConstBegin()).GetError();
        REQUIRE(err == ErrorCode::InvalidArgument);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Zero count")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin(), deque.ConstBegin()).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 5);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
        REQUIRE(deque[2] == 10);
        REQUIRE(deque[3] == 10);
        REQUIRE(deque[4] == 10);
    }
    SECTION("Front")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin(), deque.ConstBegin() + 3).GetValue();
        REQUIRE(it == deque.Begin());
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 2);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
    }
    SECTION("Back")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin() + 2, deque.ConstEnd()).GetValue();
        REQUIRE(it == deque.Begin() + 2);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 2);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
    }
    SECTION("Mid")
    {
        Deque<i32> deque(5, 10);
        Deque<i32>::IteratorType it = deque.Erase(deque.ConstBegin() + 1, deque.ConstBegin() + 4).GetValue();
        REQUIRE(it == deque.Begin() + 1);
        REQUIRE(deque.GetCapacity() == 8);
        REQUIRE(deque.GetSize() == 2);
        REQUIRE(deque[0] == 10);
        REQUIRE(deque[1] == 10);
    }
}

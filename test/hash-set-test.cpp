#include "test-helpers.h"

#include "opal/container/dynamic-array.h"
#include "opal/container/hash-set.h"
#include "opal/container/string.h"
#include "opal/hash.h"

using namespace Opal;

TEST_CASE("Hash", "[hash]")
{
    u8 data[] = {1, 2, 3};
    u64 hash = Hash::CalcRawArray(data, 3);
    REQUIRE(hash != 0);

    StringUtf8 str = "Hello there";
    hash = Hash::CalcRange(str);
    REQUIRE(hash != 0);

    DynamicArray<i32> arr = {1, 2, 3};
    hash = Hash::CalcRange(arr);
    REQUIRE(hash != 0);

    struct Pod
    {
        int a = 5;
        double c = 10.0;
    };

    Pod pod;
    hash = Hash::CalcPOD(pod);
    REQUIRE(hash != 0);
}

TEST_CASE("Hash Set", "[hash-set]")
{
    HashSet<i32> set(100);
    REQUIRE(set.GetSize() == 0);
    REQUIRE(set.GetCapacity() == 127);
    for (int i = 0; i < 100; i++)
    {
        set.Insert(i);
    }
    for (int i = 0; i < 100; i++)
    {
        REQUIRE(i == *set.Find(i));
    }
    REQUIRE(set.GetSize() == 100);
    REQUIRE(set.GetCapacity() == 127);

    i32 sum = 0;
    for (i32 i : set)
    {
        sum += i;
    }
    REQUIRE(sum == 4950);

    set.Reserve(1000);
    sum = 0;
    for (i32 i : set)
    {
        sum += i;
    }
    REQUIRE(sum == 4950);
}

TEST_CASE("Hash set copy constructor", "[hash-set]")
{
    HashSet<i32> set(4);
    set.Insert(5);
    set.Insert(10);
    REQUIRE(set.GetSize() == 2);

    const HashSet<i32> set_copy(set);
    REQUIRE(set_copy.GetSize() == 2);
    REQUIRE(set_copy.Contains(5));
    REQUIRE(set_copy.Contains(10));

    REQUIRE(set.GetSize() == 2);
    REQUIRE(set.Contains(5));
    REQUIRE(set.Contains(10));
}

TEST_CASE("Hash set move constructor", "[hash-set]")
{
    HashSet<i32> set(4);
    set.Insert(5);
    set.Insert(10);
    REQUIRE(set.GetSize() == 2);

    const HashSet<i32> set_move(Opal::Move(set));
    REQUIRE(set_move.GetSize() == 2);
    REQUIRE(set_move.Contains(5));
    REQUIRE(set_move.Contains(10));

    REQUIRE(set.GetSize() == 0);
    REQUIRE(!set.Contains(5));
    REQUIRE(!set.Contains(10));
}

TEST_CASE("Hash set copy assignment", "[hash-set]")
{
    HashSet<i32> set(4);
    set.Insert(5);
    set.Insert(10);

    HashSet<i32> set_copy(4);
    set_copy.Insert(6);

    set_copy = set;
    REQUIRE(set_copy.GetSize() == 2);
    REQUIRE(!set_copy.Contains(6));
    REQUIRE(set_copy.Contains(5));
    REQUIRE(set_copy.Contains(10));
    REQUIRE(set.GetSize() == 2);
    REQUIRE(set.Contains(5));
    REQUIRE(set.Contains(10));
}

TEST_CASE("Hash set move assignment", "[hash-set]")
{
    HashSet<i32> set(4);
    set.Insert(5);
    set.Insert(10);

    HashSet<i32> set_move(4);
    set_move.Insert(6);

    set_move = Move(set);
    REQUIRE(set_move.GetSize() == 2);
    REQUIRE(!set_move.Contains(6));
    REQUIRE(set_move.Contains(5));
    REQUIRE(set_move.Contains(10));
    REQUIRE(set.GetSize() == 0);
    REQUIRE(!set.Contains(5));
    REQUIRE(!set.Contains(10));
}

TEST_CASE("Hash set automatic growth", "[hash-set]")
{
    HashSet<i32> set(120);
    for (int i = 0; i < 120; i++)
    {
        set.Insert(i);
    }
    REQUIRE(set.GetCapacity() == 255);
    REQUIRE(set.GetSize() == 120);
    for (int i = 0; i < 120; i++)
    {
        REQUIRE(i == *set.Find(i));
    }
}

TEST_CASE("Hash set contains", "[hash-set]")
{
    HashSet<i32> set(120);
    for (int i = 0; i < 120; i++)
    {
        set.Insert(i);
    }
    REQUIRE(set.Contains(5));
    REQUIRE(!set.Contains(250));

    i32 t = 5;
    REQUIRE(set.Contains(t));
    t = 250;
    REQUIRE(!set.Contains(t));
}

TEST_CASE("Insert from variable", "[hash-set]")
{
    HashSet<i32> set(120);
    i32 val = 10;
    set.Insert(val);
    REQUIRE(set.Contains(val));
}

TEST_CASE("Deletion", "[hash-set]")
{
    HashSet<i32> set(100);
    set.Insert(5);
    set.Insert(10);
    ErrorCode err = set.Erase(5);
    REQUIRE(set.GetSize() == 1);
    REQUIRE(err == ErrorCode::Success);
    auto it = set.Find(5);
    REQUIRE(it == set.end());
    err = set.Erase(15);
    REQUIRE(err == ErrorCode::InvalidArgument);
}

TEST_CASE("Delete by iterator", "hash-set")
{
    HashSet<i32> set(100);
    set.Insert(1);
    set.Insert(2);
    set.Insert(3);
    const i32 deleted_value = *set.begin();
    ErrorCode err = set.Erase(set.begin());
    REQUIRE(err == ErrorCode::Success);
    REQUIRE(set.GetSize() == 2);
    REQUIRE(set.Find(deleted_value) == set.end());
    err = set.Erase(set.end());
    REQUIRE(err == ErrorCode::OutOfBounds);
}

TEST_CASE("Delete by const iterator", "hash-set")
{
    HashSet<i32> set(100);
    set.Insert(1);
    set.Insert(2);
    set.Insert(3);
    const i32 deleted_value = *set.begin();
    ErrorCode err = set.Erase(set.cbegin());
    REQUIRE(err == ErrorCode::Success);
    REQUIRE(set.GetSize() == 2);
    err = set.Erase(set.cend());
    REQUIRE(set.Find(deleted_value) == set.end());
    REQUIRE(err == ErrorCode::OutOfBounds);
}

TEST_CASE("Erase range by iterator", "hash-set")
{
    HashSet<i32> set(100);
    set.Insert(1);
    set.Insert(2);
    set.Insert(3);

    // Error cases
    ErrorCode err = set.Erase(set.end(), set.begin());
    REQUIRE(err == ErrorCode::OutOfBounds);

    err = set.Erase(set.begin(), set.end());
    REQUIRE(err == ErrorCode::Success);
    REQUIRE(set.GetSize() == 0);
    REQUIRE(set.Find(1) == set.end());
    REQUIRE(set.Find(2) == set.end());
    REQUIRE(set.Find(3) == set.end());
    err = set.Erase(set.begin(), set.end());
    REQUIRE(err == ErrorCode::Success);
}

TEST_CASE("Erase range by const iterator", "hash-set")
{
    HashSet<i32> set(100);
    set.Insert(1);
    set.Insert(2);
    set.Insert(3);

    // Error cases.
    ErrorCode err = set.Erase(set.cend(), set.cbegin());
    REQUIRE(err == ErrorCode::OutOfBounds);

    err = set.Erase(set.cbegin(), set.cend());
    REQUIRE(err == ErrorCode::Success);
    REQUIRE(set.GetSize() == 0);
    REQUIRE(set.Find(1) == set.end());
    REQUIRE(set.Find(2) == set.end());
    REQUIRE(set.Find(3) == set.end());
    err = set.Erase(set.cbegin(), set.cend());
    REQUIRE(err == ErrorCode::Success);
}

TEST_CASE("Check if growth left changes properly")
{
    HashSet<i32> set(100);
    REQUIRE(set.GetGrowthLeft() == set.GetCapacity());
    set.Insert(5);
    REQUIRE(set.GetGrowthLeft() == set.GetCapacity() - 1);
    set.Erase(5);
    REQUIRE(set.GetGrowthLeft() == set.GetCapacity() - 1);
}

TEST_CASE("Clear")
{
    HashSet<i32> set;
    for (int i = 0; i < 100; i++)
    {
        set.Insert(i);
    }
    REQUIRE(set.GetSize() == 100);
    REQUIRE(set.GetGrowthLeft() == 27);

    set.Clear();
    REQUIRE(set.GetSize() == 0);
    REQUIRE(set.GetGrowthLeft() == 127);
}

struct SomeType
{
    i32* a = nullptr;
    f32 b = 5;

    SomeType() = default;
    SomeType(int aa, f32 bb)
    {
        a = new i32(aa);
        b = bb;
    }
    ~SomeType() { delete a; }

    SomeType(const SomeType& other)
    {
        a = new i32();
        *a = *other.a;
        b = other.b;
    }

    SomeType& operator=(const SomeType& other)
    {
        delete a;
        a = new i32();
        *a = *other.a;
        b = other.b;
        return *this;
    }

    bool operator==(const SomeType& other)
    {
        if (b != other.b)
        {
            return false;
        }
        if (a == other.a)
        {
            return true;
        }
        if (a == nullptr || other.a == nullptr)
        {
            return false;
        }
        return *a == *other.a;
    }
};

template <>
struct Opal::Hasher<SomeType>
{
    u64 operator()(const SomeType& value)
    {
        return static_cast<u64>(*value.a) + static_cast<u64>(12341 * value.b);
    }
};

TEST_CASE("Non-POD type")
{
    HashSet<SomeType> set;
    set.Insert(SomeType(5, 10));
}
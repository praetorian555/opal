#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/hash.h"
#include "opal/container/string.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-set.h"

using namespace Opal;

TEST_CASE("Hash", "[hash]")
{
    u8 data[] = {1, 2, 3};
    u64 hash = CalculateHashFromPointerArray(data, 3);
    REQUIRE(hash != 0);

    StringUtf8 str = "Hello there";
    hash = CalculateHashFromContainer(str);
    REQUIRE(hash != 0);

    DynamicArray<i32> arr = {1, 2 , 3};
    hash = CalculateHashFromContainer(arr);
    REQUIRE(hash != 0);

    struct Pod
    {
        int a = 5;
        double c = 10.0;
    };

    Pod pod;
    hash = CalculateHashFromObject(pod);
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


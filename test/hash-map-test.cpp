#include "test-helpers.h"

#include "opal/container/hash-map.h"

#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"

using namespace Opal;

TEST_CASE("Create a hash map", "[HashMap]")
{
    SECTION("Default constructor")
    {
        HashMap<i32, i32> map(5);
        REQUIRE(map.GetSize() == 0);
        REQUIRE(map.GetCapacity() == 7);

        map.Insert(2, 7);
        REQUIRE(map.GetSize() == 1);
        REQUIRE(map.Contains(2));
        REQUIRE(map.GetValue(2) == 7);
    }
    SECTION("Clone")
    {
        HashMap<i32, i32> map(5);
        map.Insert(2, 7);
        map.Insert(5, 14);

        const HashMap<i32, i32> clone = map.Clone();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(map.Contains(2));
        REQUIRE(map.GetValue(2) == 7);
        REQUIRE(map.Contains(5));
        REQUIRE(map.GetValue(5) == 14);
    }
    SECTION("Move")
    {
        HashMap<i32, i32> map(5);
        map.Insert(2, 7);
        map.Insert(5, 14);

        auto moved_map = Move(map);
        REQUIRE(moved_map.GetSize() == 2);
        REQUIRE(moved_map.Contains(2));
        REQUIRE(moved_map.GetValue(2) == 7);
        REQUIRE(moved_map.Contains(5));
        REQUIRE(moved_map.GetValue(5) == 14);

        REQUIRE(map.GetSize() == 0);
        REQUIRE(!map.Contains(2));
        REQUIRE(!map.Contains(5));
    }
    SECTION("Move Assignment")
    {
        HashMap<i32, i32> map(5);
        map.Insert(2, 7);
        map.Insert(5, 14);

        HashMap<i32, i32> second_map(15);
        second_map.Insert(4, 12);

        second_map = Move(map);
        REQUIRE(second_map.GetSize() == 2);
        REQUIRE(second_map.Contains(2));
        REQUIRE(second_map.GetValue(2) == 7);
        REQUIRE(second_map.Contains(5));
        REQUIRE(second_map.GetValue(5) == 14);

        REQUIRE(map.GetSize() == 0);
        REQUIRE(!map.Contains(2));
        REQUIRE(!map.Contains(5));
    }
}

TEST_CASE("Hash map insert", "[HashMap]")
{
    HashMap<i32, i32> map(5);
    map.Insert(2, 7);
    REQUIRE(map.GetSize() == 1);
    REQUIRE(map.Contains(2));
    REQUIRE(map.GetValue(2) == 7);

    i32 key = 5;
    i32 value = 14;
    map.Insert(key, value);
    REQUIRE(map.GetSize() == 2);
    REQUIRE(map.Contains(key));
    REQUIRE(map.GetValue(key) == value);
}

TEST_CASE("Hash map erase", "[HashMap]")
{
    SECTION("Erase with key and iterator")
    {
        HashMap<i32, i32> map(5);
        map.Insert(2, 7);
        map.Insert(5, 14);

        map.Erase(2);
        REQUIRE(map.GetSize() == 1);
        REQUIRE(!map.Contains(2));

        HashMap<i32, i32>::iterator it = map.Find(5);
        REQUIRE(it != map.end());
        map.Erase(it);
        REQUIRE(map.GetSize() == 0);
        REQUIRE(!map.Contains(5));
    }
    SECTION("Erase with const iterator")
    {
        DynamicArray<Pair<i32, i32>> pairs({{2, 7}, {5, 14}});
        const HashMap<i32, i32> map{ArrayView<Pair<i32, i32>>(pairs)};
        HashMap<i32, i32>::const_iterator it = map.Find(5);
        REQUIRE(it.GetValue() == 14);
    }
    SECTION("Clear")
    {
        DynamicArray<Pair<i32, i32>> pairs({{2, 7}, {5, 14}});
        HashMap<i32, i32> map{ArrayView<Pair<i32, i32>>(pairs)};
        REQUIRE(map.GetSize() == 2);
        map.Clear();
        REQUIRE(map.GetSize() == 0);
    }
}

TEST_CASE("Convert to array", "[HashMap]")
{
    SECTION("To array")
    {
        HashMap<i32, i32> map;
        map.Insert(2, 7);
        map.Insert(5, 14);
        auto arr = map.ToArray();
        REQUIRE(arr.GetSize() == 2);
        for (auto& pair : arr)
        {
            REQUIRE(pair.value == map.Find(pair.key).GetValue());
            map.Erase(pair.key);
        }
        REQUIRE(map.GetSize() == 0);
    }
    SECTION("To array of keys")
    {
        HashMap<i32, i32> map;
        map.Insert(2, 7);
        map.Insert(5, 14);
        auto arr = map.ToArrayOfKeys();
        REQUIRE(arr.GetSize() == 2);
        for (auto& key : arr)
        {
            REQUIRE(map.Contains(key));
            map.Erase(key);
        }
        REQUIRE(map.GetSize() == 0);
    }
    SECTION("To array of values")
    {
        HashMap<i32, i32> map;
        map.Insert(2, 7);
        map.Insert(5, 14);
        auto arr = map.ToArrayOfValues();
        REQUIRE(arr.GetSize() == 2);
        for (auto& value : arr)
        {
            for (auto& pair : map)
            {
                if (pair.value == value)
                {
                    map.Erase(pair.key);
                    break;
                }
            }
        }
        REQUIRE(map.GetSize() == 0);
    }
}

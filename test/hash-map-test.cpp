#include "test-helpers.h"

#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"

#include "opal/clonable-base.h"
#include "opal/container/string.h"

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
    SECTION("Initializer list")
    {
        HashMap<i32, i32> map{{2, 7}, {5, 14}};
        REQUIRE(map.GetSize() == 2);
        REQUIRE(map.Contains(2));
        REQUIRE(map.GetValue(2) == 7);
        REQUIRE(map.Contains(5));
        REQUIRE(map.GetValue(5) == 14);
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
    SECTION("To array of keys - string")
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
        HashMap<StringUtf8, StringUtf8> map;
        map.Insert("Hello", "World");
        map.Insert("name", "Marko");
        auto arr = map.ToArrayOfKeys();
        REQUIRE(arr.GetSize() == 2);
        for (auto& value : arr)
        {
            for (auto& pair : map)
            {
                if (pair.key == value)
                {
                    map.Erase(pair.key);
                    break;
                }
            }
        }
        REQUIRE(map.GetSize() == 0);
    }
    SECTION("To array of values - strings")
    {
        HashMap<StringUtf8, StringUtf8> map;
        map.Insert("Hello", "World");
        map.Insert("name", "Marko");
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

TEST_CASE("Clonable interface for pair", "[HashMap]")
{
    enum class Color : u8
    {
        Red,
        Green,
        Blue
    };

    struct Data : ClonableBase<Data>
    {
        Pair<i32, i32> a;
        i32 b = 0;
        DynamicArray<Pair<Color, i32>> c;

        OPAL_CLONE_FIELDS(a, b, c);
    };

    Data first;
    first.a = {1, 2};
    first.b = 3;
    first.c = {{Color::Red, 5}};
    Data second = first.Clone();
    REQUIRE(second.a.key == 1);
    REQUIRE(second.a.value == 2);
    REQUIRE(second.b == 3);
    REQUIRE(second.c[0].key == Color::Red);
    REQUIRE(second.c[0].value == 5);
}

TEST_CASE("Clonable deep copy for pair", "[HashMap]")
{
    struct Data : ClonableBase<Data>
    {
        Pair<StringUtf8, i32> a;
        DynamicArray<Pair<StringUtf8, i32>> b;

        OPAL_CLONE_FIELDS(a, b);
    };

    Data original;
    original.a = {"Hello", 1};
    original.b = {{"World", 2}, {"Foo", 3}};
    Data cloned = original.Clone();

    // Mutating the original should not affect the clone.
    original.a.key = "Changed";
    original.b[0].key = "Changed";

    REQUIRE(cloned.a.key == "Hello");
    REQUIRE(cloned.a.value == 1);
    REQUIRE(cloned.b[0].key == "World");
    REQUIRE(cloned.b[1].key == "Foo");
}

TEST_CASE("Clonable with user-defined constructor for pair", "[HashMap]")
{
    struct Data : ClonableBase<Data>
    {
        StringUtf8 name;
        Pair<i32, i32> dimensions;

        Data() = default;
        Data(StringUtf8 in_name, i32 width, i32 height) : name(std::move(in_name)), dimensions{width, height} {}

        OPAL_CLONE_FIELDS(name, dimensions);
    };

    Data original("Rectangle", 10, 20);
    Data cloned = original.Clone();
    REQUIRE(cloned.name == "Rectangle");
    REQUIRE(cloned.dimensions.key == 10);
    REQUIRE(cloned.dimensions.value == 20);

    // Verify deep copy.
    original.name = "Changed";
    REQUIRE(cloned.name == "Rectangle");
}

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

TEST_CASE("Hash map self move assignment", "[HashMap]")
{
    HashMap<StringUtf8, StringUtf8> map(4);
    map.Insert("Hello", "World");
    map.Insert("name", "Marko");

    HashMap<StringUtf8, StringUtf8>& alias = map;
    map = Move(alias);

    REQUIRE(map.GetSize() == 2);
    REQUIRE(map.Contains("Hello"));
    REQUIRE(map.GetValue("Hello") == "World");
    REQUIRE(map.Contains("name"));
    REQUIRE(map.GetValue("name") == "Marko");
}

TEST_CASE("Hash map use after move", "[HashMap]")
{
    HashMap<StringUtf8, StringUtf8> map(4);
    map.Insert("Hello", "World");
    const HashMap<StringUtf8, StringUtf8> moved(Move(map));

    SECTION("Queries on a moved from map are well defined")
    {
        REQUIRE(map.GetSize() == 0);
        REQUIRE(map.GetCapacity() == 0);
        REQUIRE(!map.Contains("Hello"));
        REQUIRE(map.Find("Hello") == map.end());
        REQUIRE(map.begin() == map.end());
        map.Clear();
        REQUIRE(map.GetSize() == 0);
    }
    SECTION("A moved from map can be filled again")
    {
        map.Insert("Again", "Value");
        REQUIRE(map.GetSize() == 1);
        REQUIRE(map.GetValue("Again") == "Value");
        // The map must not come back as a degenerate table that cannot hold a pair.
        REQUIRE(map.GetCapacity() >= (HashMap<StringUtf8, StringUtf8>::k_default_capacity));
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

TEST_CASE("Hash map iterator member access", "[HashMap]")
{
    HashMap<StringUtf8, StringUtf8> map;
    map.Insert("Hello", "World");

    SECTION("Through a mutable iterator")
    {
        HashMap<StringUtf8, StringUtf8>::iterator it = map.Find("Hello");
        REQUIRE(it != map.end());
        REQUIRE(it->key == "Hello");
        REQUIRE(it->value == "World");
        REQUIRE((*it).value == "World");
    }
    SECTION("Through a const iterator")
    {
        const HashMap<StringUtf8, StringUtf8>& const_map = map;
        HashMap<StringUtf8, StringUtf8>::const_iterator it = const_map.Find("Hello");
        REQUIRE(it != const_map.end());
        REQUIRE(it->key == "Hello");
        REQUIRE(it->value == "World");
        REQUIRE((*it).value == "World");
    }
}

TEST_CASE("Hash map clone", "[HashMap]")
{
    SECTION("Non-POD key and value")
    {
        HashMap<StringUtf8, StringUtf8> map;
        map.Insert("Hello", "World");
        map.Insert("name", "Marko");

        HashMap<StringUtf8, StringUtf8> clone = map.Clone();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(clone.GetValue("Hello") == "World");
        REQUIRE(clone.GetValue("name") == "Marko");

        // Dropping the original must leave the clone intact, so the copy has to be deep.
        map.Clear();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(clone.GetValue("Hello") == "World");
        REQUIRE(clone.GetValue("name") == "Marko");
    }
    SECTION("Empty map")
    {
        HashMap<StringUtf8, StringUtf8> map;
        HashMap<StringUtf8, StringUtf8> clone = map.Clone();
        REQUIRE(clone.GetSize() == 0);
        REQUIRE(clone.begin() == clone.end());
    }
    SECTION("Into a different allocator")
    {
        MallocAllocator allocator;
        HashMap<StringUtf8, StringUtf8> map;
        map.Insert("Hello", "World");

        HashMap<StringUtf8, StringUtf8> clone = map.Clone(&allocator);
        REQUIRE(clone.GetSize() == 1);
        REQUIRE(clone.GetValue("Hello") == "World");
    }
}

TEST_CASE("Hash map to array with non-POD pairs", "[HashMap]")
{
    HashMap<StringUtf8, StringUtf8> map;
    map.Insert("Hello", "World");
    map.Insert("name", "Marko");

    DynamicArray<Pair<StringUtf8, StringUtf8>> pairs = map.ToArray();
    REQUIRE(pairs.GetSize() == 2);

    map.Clear();
    for (const auto& pair : pairs)
    {
        REQUIRE((pair.key == "Hello" || pair.key == "name"));
        if (pair.key == "Hello")
        {
            REQUIRE(pair.value == "World");
        }
        else
        {
            REQUIRE(pair.value == "Marko");
        }
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

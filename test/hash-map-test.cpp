#include "test-helpers.h"

#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/hash-set.h"

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

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)  // Padding the type is the whole point of this one.
struct alignas(64) CacheLineAlignedMapKey
{
    i32 value = 0;

    bool operator==(const CacheLineAlignedMapKey& other) const { return value == other.value; }
};
OPAL_END_DISABLE_WARNINGS

// Four bytes of value and sixty of padding. The default hasher for plain old data hashes the object's
// bytes, padding included, and padding is indeterminate, so two keys that compare equal would not hash
// equal. Hash the member that actually carries the value.
template <>
struct Opal::Hasher<CacheLineAlignedMapKey>
{
    u64 operator()(const CacheLineAlignedMapKey& key) const { return Hash::CalcPOD(key.value); }
};

TEST_CASE("Hash map reports failures through error codes", "[HashMap]")
{
    HashMap<i32, i32> map;

    SECTION("Insert and reserve succeed")
    {
        REQUIRE(map.Reserve(64) == ErrorCode::Success);
        REQUIRE(map.Insert(1, 10) == ErrorCode::Success);
        // Inserting over an existing key is an overwrite, not a failure.
        REQUIRE(map.Insert(1, 20) == ErrorCode::Success);
        REQUIRE(map.GetSize() == 1);
        REQUIRE(map.GetValue(1) == 20);
    }
    SECTION("Erasing a key that is not there is reported")
    {
        map.Insert(1, 10);
        REQUIRE(map.Erase(1) == ErrorCode::Success);
        REQUIRE(map.Erase(1) == ErrorCode::InvalidArgument);
        REQUIRE(map.Erase(99) == ErrorCode::InvalidArgument);
    }
    SECTION("Erasing outside the map is reported")
    {
        map.Insert(1, 10);
        REQUIRE(map.Erase(map.end()) == ErrorCode::OutOfBounds);
        REQUIRE(map.Erase(map.cend()) == ErrorCode::OutOfBounds);
        REQUIRE(map.Erase(map.end(), map.begin()) == ErrorCode::OutOfBounds);
        REQUIRE(map.Erase(map.cend(), map.cbegin()) == ErrorCode::OutOfBounds);
        REQUIRE(map.GetSize() == 1);

        REQUIRE(map.Erase(map.begin()) == ErrorCode::Success);
        REQUIRE(map.IsEmpty());
    }
    SECTION("Allocation failure after construction is returned, not thrown")
    {
        // Hands out the first block the map asks for and refuses every one after it, so the map is
        // built but cannot grow.
        struct OneShotAllocator : AllocatorBase
        {
            OneShotAllocator() : AllocatorBase("OneShotAllocator") {}

            void* Alloc(u64 size, u64 alignment) override
            {
                if (m_exhausted)
                {
                    return nullptr;
                }
                m_exhausted = true;
                return m_inner.Alloc(size, alignment);
            }
            void Free(void* ptr) override { m_inner.Free(ptr); }
            [[nodiscard]] bool IsThreadSafe() const override { return false; }

            MallocAllocator m_inner;
            bool m_exhausted = false;
        };

        OneShotAllocator allocator;
        HashMap<i32, i32> starved(4, &allocator);
        REQUIRE(starved.GetCapacity() == 7);

        REQUIRE(starved.Reserve(64) == ErrorCode::OutOfMemory);
        // The failed reserve must leave the map exactly as it was.
        REQUIRE(starved.GetCapacity() == 7);

        // Filling it past the growth threshold now fails instead of throwing.
        ErrorCode status = ErrorCode::Success;
        for (i32 i = 0; i < 32 && status == ErrorCode::Success; i++)
        {
            status = starved.Insert(i, i);
        }
        REQUIRE(status == ErrorCode::OutOfMemory);
    }
    SECTION("A constructor that cannot allocate throws")
    {
        NullAllocator allocator;
        using Map = HashMap<i32, i32>;
        REQUIRE_THROWS_AS(Map(4, &allocator), OutOfMemoryException);
    }
}

TEST_CASE("Hash map emptiness", "[HashMap]")
{
    HashMap<i32, i32> map;
    REQUIRE(map.IsEmpty());
    REQUIRE(map.empty());

    map.Insert(5, 10);
    REQUIRE(!map.IsEmpty());
    REQUIRE(!map.empty());

    map.Erase(5);
    REQUIRE(map.IsEmpty());

    map.Insert(5, 10);
    map.Clear();
    REQUIRE(map.IsEmpty());
}

TEST_CASE("Hash map erase by range", "[HashMap]")
{
    SECTION("Erasing the whole range empties the map")
    {
        HashMap<i32, i32> map;
        map.Insert(1, 10);
        map.Insert(2, 20);
        map.Insert(3, 30);

        map.Erase(map.begin(), map.end());
        REQUIRE(map.IsEmpty());
        REQUIRE(!map.Contains(1));
        REQUIRE(!map.Contains(2));
        REQUIRE(!map.Contains(3));

        // Erasing an already empty range is not an error.
        map.Erase(map.begin(), map.end());
        REQUIRE(map.IsEmpty());
    }
    SECTION("An inverted range leaves the map alone")
    {
        HashMap<i32, i32> map;
        map.Insert(1, 10);
        map.Insert(2, 20);

        map.Erase(map.end(), map.begin());
        REQUIRE(map.GetSize() == 2);
    }
    SECTION("Through const iterators")
    {
        HashMap<i32, i32> map;
        map.Insert(1, 10);
        map.Insert(2, 20);

        map.Erase(map.cbegin(), map.cend());
        REQUIRE(map.IsEmpty());
    }
}

TEST_CASE("Hash map with pairs aligned more strictly than the block", "[HashMap]")
{
    static_assert(IsPOD<CacheLineAlignedMapKey>, "alignas must not cost the type its POD hasher");

    HashMap<CacheLineAlignedMapKey, i32> map;
    for (i32 i = 0; i < 100; i++)
    {
        map.Insert(CacheLineAlignedMapKey{i}, i * 2);
    }
    REQUIRE(map.GetSize() == 100);

    for (const auto& pair : map)
    {
        REQUIRE(reinterpret_cast<u64>(&pair) % alignof(Pair<CacheLineAlignedMapKey, i32>) == 0);
        REQUIRE(map.GetValue(pair.key) == pair.key.value * 2);
    }
}

TEST_CASE("Hash map reserve never drops below what the map holds", "[HashMap]")
{
    HashMap<i32, i32> map(100);
    for (i32 i = 0; i < 100; i++)
    {
        map.Insert(i, i * 2);
    }
    REQUIRE(map.GetSize() == 100);

    // Asking for less room than the map already uses must not lose pairs.
    map.Reserve(4);
    REQUIRE(map.GetSize() == 100);
    REQUIRE(map.GetCapacity() > 100);
    for (i32 i = 0; i < 100; i++)
    {
        REQUIRE(map.GetValue(i) == i * 2);
    }

    // The map is still usable afterwards.
    map.Insert(100, 200);
    REQUIRE(map.GetValue(100) == 200);
}

TEST_CASE("Hash map capacity survives insert and erase churn", "[HashMap]")
{
    constexpr i32 k_live_keys = 50;
    HashMap<i32, i32> map;
    for (i32 i = 0; i < k_live_keys; i++)
    {
        map.Insert(i, i * 2);
    }

    for (i32 i = k_live_keys; i < 10000; i++)
    {
        map.Insert(i, i * 2);
        map.Erase(i - k_live_keys);
    }

    REQUIRE(map.GetSize() == k_live_keys);
    // Capacity has to track the live pairs, not the number of insertions ever made.
    REQUIRE(map.GetCapacity() < 1000);
    for (i32 i = 10000 - k_live_keys; i < 10000; i++)
    {
        REQUIRE(map.GetValue(i) == i * 2);
    }
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

TEST_CASE("Create a hash map without throwing", "[HashMap]")
{
    SECTION("Capacity")
    {
        auto map = HashMap<i32, i32>::Create(16);
        REQUIRE(map.HasValue());
        REQUIRE(map.GetValue().GetSize() == 0);
    }
    SECTION("Initializer list")
    {
        auto map = HashMap<i32, i32>::Create({{1, 10}, {2, 20}});
        REQUIRE(map.HasValue());
        REQUIRE(map.GetValue().GetSize() == 2);
        REQUIRE(map.GetValue().GetValue(1) == 10);
    }
    SECTION("Reports a failed allocation instead of throwing")
    {
        NullAllocator null_allocator;
        auto map = HashMap<i32, i32>::Create(16, &null_allocator);
        REQUIRE_FALSE(map.HasValue());
        REQUIRE(map.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Initializer list reports a failed allocation")
    {
        NullAllocator null_allocator;
        auto map = HashMap<i32, i32>::Create({{1, 10}, {2, 20}}, &null_allocator);
        REQUIRE_FALSE(map.HasValue());
        REQUIRE(map.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("The throwing constructor still throws")
    {
        NullAllocator null_allocator;
        using MapType = HashMap<i32, i32>;
        REQUIRE_THROWS_AS(MapType(16, &null_allocator), OutOfMemoryException);
    }
}

TEST_CASE("HashMap iteration across many groups", "[hash-map]")
{
    HashMap<i32, i32> map;
    for (i32 i = 0; i < 1000; i++)
    {
        map.Insert(i * 7, i);
    }
    SECTION("Every pair is visited exactly once")
    {
        HashSet<i32> seen;
        u64 count = 0;
        for (const auto& pair : map)
        {
            REQUIRE(pair.key == pair.value * 7);
            REQUIRE_FALSE(seen.Contains(pair.key));
            seen.Insert(pair.key);
            count++;
        }
        REQUIRE(count == map.GetSize());
    }
    SECTION("Pairs erased ahead of the iterator are skipped")
    {
        HashSet<i32> seen;
        bool erased = false;
        for (const auto& pair : map)
        {
            if (erased)
            {
                REQUIRE(pair.value % 2 == 0);
            }
            seen.Insert(pair.key);
            if (!erased)
            {
                for (i32 i = 1; i < 1000; i += 2)
                {
                    map.Erase(i * 7);
                }
                erased = true;
            }
        }
        for (i32 i = 0; i < 1000; i += 2)
        {
            REQUIRE(seen.Contains(i * 7));
        }
    }
}

TEST_CASE("HashMap Clone after erasures", "[hash-map]")
{
    SECTION("POD pairs")
    {
        HashMap<i32, i32> map;
        for (i32 i = 0; i < 300; i++)
        {
            map.Insert(i * 3, i);
        }
        for (i32 i = 0; i < 300; i += 2)
        {
            map.Erase(i * 3);
        }

        HashMap<i32, i32> clone = map.Clone();
        REQUIRE(clone.GetSize() == 150);
        for (i32 i = 0; i < 300; i++)
        {
            REQUIRE(clone.Contains(i * 3) == (i % 2 == 1));
        }
        REQUIRE(clone.GetValue(9) == 3);

        // The clone stays independent and usable, on both erased and live slots.
        REQUIRE(clone.Insert(6, 2) == ErrorCode::Success);
        REQUIRE(clone.GetValue(6) == 2);
        REQUIRE_FALSE(map.Contains(6));
        REQUIRE(clone.Erase(9) == ErrorCode::Success);
        REQUIRE(map.Contains(9));
        REQUIRE(clone.GetSize() == 150);
    }
    SECTION("String pairs")
    {
        HashMap<StringUtf8, StringUtf8> map;
        for (i32 i = 0; i < 200; i++)
        {
            const char8 buffer[3] = {static_cast<char8>('a' + i / 26), static_cast<char8>('a' + i % 26), 0};
            map.Insert(StringUtf8(buffer), StringUtf8(buffer));
        }
        for (i32 i = 0; i < 200; i += 3)
        {
            const char8 buffer[3] = {static_cast<char8>('a' + i / 26), static_cast<char8>('a' + i % 26), 0};
            map.Erase(StringUtf8(buffer));
        }

        HashMap<StringUtf8, StringUtf8> clone = map.Clone();
        REQUIRE(clone.GetSize() == map.GetSize());
        for (i32 i = 0; i < 200; i++)
        {
            const char8 buffer[3] = {static_cast<char8>('a' + i / 26), static_cast<char8>('a' + i % 26), 0};
            const StringUtf8 key(buffer);
            REQUIRE(clone.Contains(key) == (i % 3 != 0));
            if (i % 3 != 0)
            {
                REQUIRE(clone.GetValue(key) == key);
            }
        }

        // Dropping the original must leave the clone intact, so the copy has to be deep.
        map.Clear();
        REQUIRE(clone.GetValue("ab") == "ab");
        REQUIRE_FALSE(clone.Contains("aa"));
    }
}

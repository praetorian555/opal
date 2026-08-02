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

TEST_CASE("Hash of a range covers every byte of every element", "[hash]")
{
    // Both arrays share their first element, and differ only past the first byte of the second one.
    DynamicArray<i32> first = {1, 2};
    DynamicArray<i32> second = {1, 258};
    REQUIRE(Hash::CalcRange(first) != Hash::CalcRange(second));
}

TEST_CASE("Hash of a range depends only on its elements", "[hash]")
{
    SECTION("Equal empty ranges hash the same")
    {
        const StringUtf8 first;
        const StringUtf8 second;
        REQUIRE(Hash::CalcRange(first) == Hash::CalcRange(second));

        const DynamicArray<i32> first_array;
        const DynamicArray<i32> second_array;
        REQUIRE(Hash::CalcRange(first_array) == Hash::CalcRange(second_array));
    }
    SECTION("Equal non empty ranges hash the same")
    {
        const StringUtf8 first = "Hello there";
        const StringUtf8 second = "Hello there";
        REQUIRE(Hash::CalcRange(first) == Hash::CalcRange(second));
    }
    SECTION("An empty range does not hash like a populated one")
    {
        const StringUtf8 empty;
        const StringUtf8 populated = "Hello there";
        REQUIRE(Hash::CalcRange(empty) != Hash::CalcRange(populated));
    }
}

TEST_CASE("Hash set with empty keys", "[hash-set]")
{
    HashSet<StringUtf8> set;
    set.Insert(StringUtf8(""));
    REQUIRE(set.GetSize() == 1);
    REQUIRE(set.Contains(StringUtf8("")));

    // A second empty key is the same key, so it must not add an entry.
    set.Insert(StringUtf8(""));
    REQUIRE(set.GetSize() == 1);

    set.Insert("Hello");
    REQUIRE(set.GetSize() == 2);
    REQUIRE(set.Contains(StringUtf8("")));
    REQUIRE(set.Contains("Hello"));

    REQUIRE(set.Erase(StringUtf8("")) == ErrorCode::Success);
    REQUIRE(!set.Contains(StringUtf8("")));
    REQUIRE(set.Contains("Hello"));
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

TEST_CASE("Hash set self move assignment", "[hash-set]")
{
    HashSet<StringUtf8> set(4);
    set.Insert("Hello");
    set.Insert("World");

    HashSet<StringUtf8>& alias = set;
    set = Move(alias);

    REQUIRE(set.GetSize() == 2);
    REQUIRE(set.Contains("Hello"));
    REQUIRE(set.Contains("World"));
}

TEST_CASE("Hash set use after move", "[hash-set]")
{
    HashSet<StringUtf8> set(4);
    set.Insert("Hello");
    const HashSet<StringUtf8> moved(Move(set));

    SECTION("Queries on a moved from set are well defined")
    {
        REQUIRE(set.GetSize() == 0);
        REQUIRE(set.GetCapacity() == 0);
        REQUIRE(!set.Contains("Hello"));
        REQUIRE(set.Find("Hello") == set.end());
        REQUIRE(set.begin() == set.end());
        REQUIRE(set.ToArray().GetSize() == 0);
        set.Clear();
        REQUIRE(set.GetSize() == 0);
    }
    SECTION("A moved from set can be filled again")
    {
        REQUIRE(set.Insert("Again") == ErrorCode::Success);
        REQUIRE(set.GetSize() == 1);
        REQUIRE(set.Contains("Again"));
        // The set must not come back as a degenerate table that cannot hold a key.
        REQUIRE(set.GetCapacity() >= HashSet<StringUtf8>::k_default_capacity);
    }
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

    SomeType(SomeType&& other) noexcept : a(other.a), b(other.b)
    {
        other.a = nullptr;
    }

    SomeType& operator=(SomeType&& other) noexcept
    {
        if (this != &other)
        {
            delete a;
            a = other.a;
            b = other.b;
            other.a = nullptr;
        }
        return *this;
    }

    SomeType Clone(AllocatorBase* = nullptr)
    {
        SomeType other;
        other.a = new i32(*a);
        other.b = b;
        return other;
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

TEST_CASE("HashSet Clone", "[hash-set]")
{
    SECTION("POD type")
    {
        HashSet<i32> set(100);
        for (i32 i = 0; i < 20; i++)
        {
            set.Insert(i);
        }

        HashSet<i32> clone = set.Clone();
        REQUIRE(clone.GetSize() == set.GetSize());
        for (i32 i = 0; i < 20; i++)
        {
            REQUIRE(clone.Contains(i));
        }

        // The clone is independent of the set it came from.
        set.Clear();
        REQUIRE(clone.GetSize() == 20);
        REQUIRE(clone.Contains(5));
    }
    SECTION("Non-POD type")
    {
        HashSet<StringUtf8> set;
        set.Insert("Hello");
        set.Insert("World");

        HashSet<StringUtf8> clone = set.Clone();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(clone.Contains("Hello"));
        REQUIRE(clone.Contains("World"));

        // Dropping the original must leave the clone's keys intact, so the copy has to be deep.
        set.Clear();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(clone.Contains("Hello"));
        REQUIRE(clone.Contains("World"));
    }
    SECTION("Empty set")
    {
        HashSet<StringUtf8> set;
        HashSet<StringUtf8> clone = set.Clone();
        REQUIRE(clone.GetSize() == 0);
        REQUIRE(clone.begin() == clone.end());
    }
    SECTION("Into a different allocator")
    {
        MallocAllocator allocator;
        HashSet<StringUtf8> set;
        set.Insert("Hello");

        HashSet<StringUtf8> clone = set.Clone(&allocator);
        REQUIRE(clone.GetSize() == 1);
        REQUIRE(clone.Contains("Hello"));
    }
}

TEST_CASE("HashSet ToArray", "[hash-set]")
{
    SECTION("POD type")
    {
        HashSet<i32> set;
        set.Insert(1);
        set.Insert(2);
        set.Insert(3);
        DynamicArray<i32> arr = set.ToArray();
        REQUIRE(arr.GetSize() == 3);
        for (u64 i = 0; i < arr.GetSize(); i++)
        {
            REQUIRE(set.Contains(arr[i]));
        }
    }
    SECTION("Empty set")
    {
        HashSet<i32> set;
        DynamicArray<i32> arr = set.ToArray();
        REQUIRE(arr.GetSize() == 0);
    }
    SECTION("Non-POD type")
    {
        HashSet<StringUtf8> set;
        set.Insert("Hello");
        set.Insert("World");
        DynamicArray<StringUtf8> arr = set.ToArray();
        REQUIRE(arr.GetSize() == 2);
        for (u64 i = 0; i < arr.GetSize(); i++)
        {
            REQUIRE(set.Contains(arr[i]));
        }
    }
}
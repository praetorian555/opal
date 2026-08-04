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

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)
struct PaddedPod
{
    i8 a = 0;
    i64 b = 0;

    bool operator==(const PaddedPod& other) const { return a == other.a && b == other.b; }
};
OPAL_END_DISABLE_WARNINGS

TEST_CASE("Types whose bytes do not determine their value get no default hasher", "[hash]")
{
    // Trivially copyable and standard layout, so it is plain old data, but the seven bytes between the
    // members are indeterminate. Hashing its bytes would let two equal values hash differently.
    static_assert(IsPOD<PaddedPod>);
    static_assert(!HasUniqueObjectRepresentations<PaddedPod>);

    // Naming Hasher<PaddedPod> is a build error, so what it does cannot be asserted from a binary that
    // links. The two traits above are what decides it, and the specializations below cover the types
    // that do get a hasher; uncommenting either line fails the build with the static_assert message.
    //
    //     Hasher<PaddedPod> hasher;
    //     HashSet<PaddedPod> set;  set.Insert(PaddedPod{1, 2});
    static_assert(HasUniqueObjectRepresentations<i32>);
    static_assert(!HasUniqueObjectRepresentations<f32>);
}

TEST_CASE("Floating point keys hash by value, not by bytes", "[hash]")
{
    // Positive and negative zero compare equal but do not share a representation.
    REQUIRE(0.0F == -0.0F);
    REQUIRE(Hasher<f32>{}(0.0F) == Hasher<f32>{}(-0.0F));
    REQUIRE(Hasher<f64>{}(0.0) == Hasher<f64>{}(-0.0));

    REQUIRE(Hasher<f32>{}(1.5F) != Hasher<f32>{}(2.5F));

    HashSet<f32> set;
    set.Insert(0.0F);
    REQUIRE(set.Contains(-0.0F));
    // Inserting the value that already compares equal must not add a second key.
    set.Insert(-0.0F);
    REQUIRE(set.GetSize() == 1);
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

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)  // Padding the type is the whole point of this one.
struct alignas(64) CacheLineAlignedKey
{
    i32 value = 0;

    bool operator==(const CacheLineAlignedKey& other) const { return value == other.value; }
};
OPAL_END_DISABLE_WARNINGS

// Four bytes of value and sixty of padding. The default hasher for plain old data hashes the object's
// bytes, padding included, and padding is indeterminate, so two keys that compare equal would not hash
// equal. Hash the member that actually carries the value.
template <>
struct Opal::Hasher<CacheLineAlignedKey>
{
    u64 operator()(const CacheLineAlignedKey& key) const { return Hash::CalcPOD(key.value); }
};

TEST_CASE("Hash set emptiness", "[hash-set]")
{
    HashSet<i32> set;
    REQUIRE(set.IsEmpty());
    REQUIRE(set.empty());

    set.Insert(5);
    REQUIRE(!set.IsEmpty());
    REQUIRE(!set.empty());

    set.Erase(5);
    REQUIRE(set.IsEmpty());

    set.Insert(5);
    set.Clear();
    REQUIRE(set.IsEmpty());
}

TEST_CASE("Hash set with keys aligned more strictly than the block", "[hash-set]")
{
    static_assert(IsPOD<CacheLineAlignedKey>, "alignas must not cost the type its POD hasher");

    // The control bytes occupy a power of two sized run, 32 bytes for the smallest tables and growing from there. A key aligned more
    // strictly than that run is long has to be pushed past it, so the smallest tables are where the offset arithmetic matters.
    SECTION("Smallest table")
    {
        HashSet<CacheLineAlignedKey> set;
        REQUIRE(set.Insert(CacheLineAlignedKey{1}) == ErrorCode::Success);
        REQUIRE(set.GetCapacity() == 7);

        for (const CacheLineAlignedKey& key : set)
        {
            REQUIRE(reinterpret_cast<u64>(&key) % alignof(CacheLineAlignedKey) == 0);
        }
    }
    SECTION("After growing")
    {
        HashSet<CacheLineAlignedKey> set;
        for (i32 i = 0; i < 100; i++)
        {
            REQUIRE(set.Insert(CacheLineAlignedKey{i}) == ErrorCode::Success);
        }
        REQUIRE(set.GetSize() == 100);

        for (i32 i = 0; i < 100; i++)
        {
            REQUIRE(set.Contains(CacheLineAlignedKey{i}));
        }
        for (const CacheLineAlignedKey& key : set)
        {
            REQUIRE(reinterpret_cast<u64>(&key) % alignof(CacheLineAlignedKey) == 0);
        }
    }
}

TEST_CASE("Hash set reserve never drops below what the set holds", "[hash-set]")
{
    HashSet<i32> set(100);
    for (i32 i = 0; i < 100; i++)
    {
        set.Insert(i);
    }
    REQUIRE(set.GetSize() == 100);

    // Asking for less room than the set already uses must not lose keys.
    REQUIRE(set.Reserve(4) == ErrorCode::Success);
    REQUIRE(set.GetSize() == 100);
    REQUIRE(set.GetCapacity() > 100);
    for (i32 i = 0; i < 100; i++)
    {
        REQUIRE(set.Contains(i));
    }

    // The set is still usable afterwards.
    REQUIRE(set.Insert(100) == ErrorCode::Success);
    REQUIRE(set.Contains(100));
}

TEST_CASE("Hash set capacity survives insert and erase churn", "[hash-set]")
{
    SECTION("A set that never holds more than one key never grows")
    {
        HashSet<i32> set;
        set.Insert(0);
        REQUIRE(set.Erase(0) == ErrorCode::Success);
        const u64 stable_capacity = set.GetCapacity();

        for (i32 i = 1; i < 10000; i++)
        {
            REQUIRE(set.Insert(i) == ErrorCode::Success);
            REQUIRE(set.Erase(i) == ErrorCode::Success);
        }

        REQUIRE(set.GetSize() == 0);
        REQUIRE(set.GetCapacity() == stable_capacity);
    }
    SECTION("A set holding a steady population stays bounded")
    {
        constexpr i32 k_live_keys = 50;
        HashSet<i32> set;
        for (i32 i = 0; i < k_live_keys; i++)
        {
            set.Insert(i);
        }

        for (i32 i = k_live_keys; i < 10000; i++)
        {
            REQUIRE(set.Insert(i) == ErrorCode::Success);
            REQUIRE(set.Erase(i - k_live_keys) == ErrorCode::Success);
        }

        REQUIRE(set.GetSize() == k_live_keys);
        // Capacity has to track the live keys, not the number of insertions ever made.
        REQUIRE(set.GetCapacity() < 1000);
        for (i32 i = 10000 - k_live_keys; i < 10000; i++)
        {
            REQUIRE(set.Contains(i));
        }
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

TEST_CASE("HashSet iterators do not hand out writable keys", "[hash-set]")
{
    // A key picks the slot it lives in, so letting one be written in place would leave it unreachable.
    HashSet<StringUtf8> set;
    set.Insert("Hello");

    auto it = set.begin();
    auto const_it = set.cbegin();

    static_assert(k_is_same_value<decltype(*it), const StringUtf8&>, "A mutable iterator still dereferences to a const key");
    static_assert(k_is_same_value<decltype(*const_it), const StringUtf8&>);
    static_assert(k_is_same_value<decltype(it.operator->()), const StringUtf8*>);
    static_assert(k_is_same_value<decltype(const_it.operator->()), const StringUtf8*>);

    for (auto& key : set)
    {
        REQUIRE(key == "Hello");
    }
}

TEST_CASE("HashSet iterator member access", "[hash-set]")
{
    HashSet<StringUtf8> set;
    set.Insert("Hello");

    SECTION("Through a mutable iterator")
    {
        HashSet<StringUtf8>::iterator it = set.Find("Hello");
        REQUIRE(it != set.end());
        REQUIRE(it->GetSize() == 5);
        REQUIRE((*it).GetSize() == 5);
    }
    SECTION("Through a const iterator")
    {
        const HashSet<StringUtf8>& const_set = set;
        HashSet<StringUtf8>::const_iterator it = const_set.Find("Hello");
        REQUIRE(it != const_set.end());
        REQUIRE(it->GetSize() == 5);
        REQUIRE((*it).GetSize() == 5);
    }
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
TEST_CASE("Create a hash set without throwing", "[hash-set]")
{
    SECTION("Capacity")
    {
        auto set = HashSet<i32>::Create(16);
        REQUIRE(set.HasValue());
        REQUIRE(set.GetValue().GetSize() == 0);
    }
    SECTION("Reports a failed allocation instead of throwing")
    {
        NullAllocator null_allocator;
        auto set = HashSet<i32>::Create(16, &null_allocator);
        REQUIRE_FALSE(set.HasValue());
        REQUIRE(set.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("The throwing constructor still throws")
    {
        NullAllocator null_allocator;
        REQUIRE_THROWS_AS(HashSet<i32>(16, &null_allocator), OutOfMemoryException);
    }
}

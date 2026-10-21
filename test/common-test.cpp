#include "test-helpers.h"

#include "opal/clonable-base.h"
#include "opal/common.h"
#include "opal/container/deque.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/hash-set.h"
#include "opal/container/in-place-array.h"
#include "opal/container/optional.h"
#include "opal/container/string.h"
#include "opal/variant.h"

using namespace Opal;

TEST_CASE("Swap", "[common]")
{
    SECTION("Swap base types")
    {
        u32 a = 1;
        u32 b = 2;
        Opal::Swap(a, b);
        REQUIRE(a == 2);
        REQUIRE(b == 1);
    }
    SECTION("Swap arrays")
    {
        u32 a[2] = {1, 2};
        u32 b[2] = {3, 4};
        Opal::Swap(a, b);
        REQUIRE(a[0] == 3);
        REQUIRE(a[1] == 4);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
    }
    SECTION("Swap pointers")
    {
        u32 a = 1;
        u32 b = 2;
        u32* pa = &a;
        u32* pb = &b;
        Opal::Swap(pa, pb);
        REQUIRE(*pa == 2);
        REQUIRE(*pb == 1);
        REQUIRE(pa == &b);
        REQUIRE(pb == &a);
    }
    SECTION("Swap dynamic array")
    {
        DynamicArray<u32> a = {1, 2};
        DynamicArray<u32> b = {3, 4};
        Opal::Swap(a, b);
        REQUIRE(a[0] == 3);
        REQUIRE(a[1] == 4);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
    }
}

namespace
{

// Hands out a fixed number of blocks and refuses every one after them.
struct CountedAllocator final : AllocatorBase
{
    explicit CountedAllocator(i32 blocks) : AllocatorBase("CountedAllocator"), m_blocks_left(blocks) {}

    void* Alloc(u64 size, u64 alignment) override
    {
        if (m_blocks_left == 0)
        {
            return nullptr;
        }
        m_blocks_left--;
        return m_inner.Alloc(size, alignment);
    }
    void Free(void* ptr) override { m_inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return false; }

    MallocAllocator m_inner;
    i32 m_blocks_left = 0;
};

// Too long for the small string buffer, so every copy takes a block of its own.
const char* const k_long_strings[] = {
    "the first string, long enough to live on the heap",
    "the second string, long enough to live on the heap",
    "the third string, long enough to live on the heap",
};

struct CloneOnly
{
    CloneOnly() = default;
    CloneOnly(const CloneOnly&) = delete;
    CloneOnly(CloneOnly&&) = default;
    CloneOnly& operator=(CloneOnly&&) = default;
    // Not trivially copyable, so Opal does not treat it as POD.
    ~CloneOnly() {}

    CloneOnly Clone(AllocatorBase* = nullptr) const
    {
        CloneOnly copy;
        copy.clone_count = clone_count + 1;
        return copy;
    }

    i32 clone_count = 0;
};

struct Record : ClonableBase<Record>
{
    StringUtf8 first;
    i32 id = 0;
    StringUtf8 second;

    OPAL_CLONE_FIELDS(first, id, second);
};

}  // namespace

TEST_CASE("Default constructors cannot fail", "[common]")
{
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<DynamicArray<StringUtf8>>);
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<Deque<StringUtf8>>);
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<StringUtf8>);
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<HashMap<StringUtf8, StringUtf8>>);
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<HashSet<StringUtf8>>);
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<Record>);
}

TEST_CASE("TryClone", "[common]")
{
    SECTION("Copies a POD value")
    {
        auto copy = Opal::TryClone(42);
        REQUIRE(copy.HasValue());
        REQUIRE(copy.GetValue() == 42);
    }
    SECTION("Falls back to Clone for a type without TryClone")
    {
        CloneOnly source;
        auto copy = Opal::TryClone(source);
        REQUIRE(copy.HasValue());
        REQUIRE(copy.GetValue().clone_count == 1);
    }
    SECTION("Uses the type's own TryClone")
    {
        StringUtf8 source(k_long_strings[0]);
        CountedAllocator allocator(0);
        auto copy = Opal::TryClone(source, &allocator);
        REQUIRE_FALSE(copy.HasValue());
        REQUIRE(copy.GetError() == ErrorCode::OutOfMemory);
    }
}

TEST_CASE("TryClone reports an element that fails to copy", "[common]")
{
    // One block for the container's own storage and one for the first element. The second element is refused.
    constexpr i32 k_blocks_for_one_element = 2;
    constexpr i32 k_blocks_for_all = 4;

    SECTION("DynamicArray")
    {
        DynamicArray<StringUtf8> source;
        for (const char* text : k_long_strings)
        {
            REQUIRE(source.PushBack(StringUtf8(text)) == ErrorCode::Success);
        }
        CountedAllocator starved(k_blocks_for_one_element);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(k_blocks_for_all);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue()[2] == source[2]);
        REQUIRE(&clone.GetValue()[2].GetAllocator() == &enough);
    }
    SECTION("Deque")
    {
        Deque<StringUtf8> source;
        for (const char* text : k_long_strings)
        {
            REQUIRE(source.PushBack(StringUtf8(text)) == ErrorCode::Success);
        }
        CountedAllocator starved(k_blocks_for_one_element);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(k_blocks_for_all);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().GetSize() == 3);
    }
    SECTION("HashMap")
    {
        HashMap<StringUtf8, i32> source;
        for (i32 i = 0; i < 3; i++)
        {
            REQUIRE(source.Insert(StringUtf8(k_long_strings[i]), i) == ErrorCode::Success);
        }
        CountedAllocator starved(k_blocks_for_one_element);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(k_blocks_for_all);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().GetValue(StringUtf8(k_long_strings[2])) == 2);
    }
    SECTION("HashSet")
    {
        HashSet<StringUtf8> source;
        for (const char* text : k_long_strings)
        {
            REQUIRE(source.Insert(StringUtf8(text)) == ErrorCode::Success);
        }
        CountedAllocator starved(k_blocks_for_one_element);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(k_blocks_for_all);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().Contains(StringUtf8(k_long_strings[2])));
    }
    SECTION("Optional")
    {
        Optional<StringUtf8> source(k_in_place, k_long_strings[0]);

        CountedAllocator starved(0);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(1);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().GetValue() == source.GetValue());

        Optional<StringUtf8> empty;
        auto empty_clone = empty.TryClone(&starved);
        REQUIRE(empty_clone.HasValue());
        REQUIRE_FALSE(empty_clone.GetValue().HasValue());
    }
    SECTION("InPlaceArray")
    {
        InPlaceArray<StringUtf8, 3> source;
        for (u64 i = 0; i < 3; i++)
        {
            source[i] = StringUtf8(k_long_strings[i]);
        }

        // An InPlaceArray holds its elements itself, so only the elements allocate.
        CountedAllocator starved(1);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(3);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue()[2] == source[2]);
    }
    SECTION("Variant")
    {
        StringUtf8 text(k_long_strings[0]);
        Variant<i32, StringUtf8> source(Move(text));

        CountedAllocator starved(0);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(1);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().Get<StringUtf8>() == source.Get<StringUtf8>());

        Variant<i32, StringUtf8> number(7);
        auto number_clone = number.TryClone(&starved);
        REQUIRE(number_clone.HasValue());
        REQUIRE(number_clone.GetValue().Get<i32>() == 7);
    }
    SECTION("ClonableBase")
    {
        Record source;
        source.first = StringUtf8(k_long_strings[0]);
        source.id = 7;
        source.second = StringUtf8(k_long_strings[1]);

        CountedAllocator starved(1);
        auto failed = source.TryClone(&starved);
        REQUIRE_FALSE(failed.HasValue());
        REQUIRE(failed.GetError() == ErrorCode::OutOfMemory);

        CountedAllocator enough(2);
        auto clone = source.TryClone(&enough);
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().id == 7);
        REQUIRE(clone.GetValue().second == source.second);
    }
}

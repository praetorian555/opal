#include <string>

#include "test-helpers.h"

#include "opal/container/dynamic-array.h"
#include "opal/container/optional.h"
#include "opal/hash.h"

using namespace Opal;

TEST_CASE("Optional", "[Optional]")
{
    SECTION("Value type")
    {
        SECTION("Default construction is empty")
        {
            Optional<i32> opt;
            REQUIRE(opt.HasValue() == false);
            REQUIRE(static_cast<bool>(opt) == false);
            REQUIRE(opt == k_null_opt);
        }
        SECTION("Null tag construction is empty")
        {
            Optional<i32> opt = k_null_opt;
            REQUIRE(opt.HasValue() == false);
        }
        SECTION("Construction with value")
        {
            Optional<i32> opt(42);
            REQUIRE(opt.HasValue() == true);
            REQUIRE(static_cast<bool>(opt) == true);
            REQUIRE(opt.GetValue() == 42);
            REQUIRE(*opt == 42);
            REQUIRE(opt != k_null_opt);
        }
        SECTION("In-place construction")
        {
            Optional<std::string> opt(k_in_place, 3, 'a');
            REQUIRE(opt.HasValue() == true);
            REQUIRE(opt.GetValue() == "aaa");
        }
        SECTION("GetValueOr")
        {
            Optional<i32> empty;
            REQUIRE(empty.GetValueOr(7) == 7);
            Optional<i32> full(3);
            REQUIRE(full.GetValueOr(7) == 3);
        }
        SECTION("operator->")
        {
            Optional<std::string> opt(k_in_place, "hello");
            REQUIRE(opt->size() == 5);
        }
    }

    SECTION("Move semantics")
    {
        SECTION("Move construction transfers value")
        {
            Optional<std::string> src(k_in_place, "moved");
            Optional<std::string> dst(Move(src));
            REQUIRE(dst.HasValue() == true);
            REQUIRE(dst.GetValue() == "moved");
        }
        SECTION("Move assignment into empty")
        {
            Optional<std::string> src(k_in_place, "value");
            Optional<std::string> dst;
            dst = Move(src);
            REQUIRE(dst.HasValue() == true);
            REQUIRE(dst.GetValue() == "value");
        }
        SECTION("Move assignment of empty resets target")
        {
            Optional<std::string> src;
            Optional<std::string> dst(k_in_place, "value");
            dst = Move(src);
            REQUIRE(dst.HasValue() == false);
        }
    }

    SECTION("Emplace and Reset")
    {
        SECTION("Emplace constructs and returns value")
        {
            Optional<std::string> opt;
            std::string& ref = opt.Emplace(4, 'z');
            REQUIRE(opt.HasValue() == true);
            REQUIRE(ref == "zzzz");
            REQUIRE(&ref == &opt.GetValue());
        }
        SECTION("Emplace replaces existing value")
        {
            Optional<std::string> opt(k_in_place, "old");
            opt.Emplace("new");
            REQUIRE(opt.GetValue() == "new");
        }
        SECTION("Reset makes empty")
        {
            Optional<i32> opt(5);
            opt.Reset();
            REQUIRE(opt.HasValue() == false);
            opt.Reset();  // idempotent
            REQUIRE(opt.HasValue() == false);
        }
        SECTION("Assigning null tag resets")
        {
            Optional<i32> opt(5);
            opt = k_null_opt;
            REQUIRE(opt.HasValue() == false);
        }
    }

    SECTION("Clone")
    {
        SECTION("Clone of empty stays empty")
        {
            Optional<i32> opt;
            Optional<i32> clone = opt.Clone();
            REQUIRE(clone.HasValue() == false);
        }
        SECTION("Clone of POD copies value")
        {
            Optional<i32> opt(99);
            Optional<i32> clone = opt.Clone();
            REQUIRE(clone.HasValue() == true);
            REQUIRE(clone.GetValue() == 99);
        }
        SECTION("Clone of non-POD deep-copies")
        {
            DynamicArray<i32> arr;
            arr.PushBack(1);
            arr.PushBack(2);
            Optional<DynamicArray<i32>> opt(Move(arr));
            Optional<DynamicArray<i32>> clone = opt.Clone();
            REQUIRE(clone.HasValue() == true);
            REQUIRE(clone.GetValue().GetSize() == 2);
            REQUIRE(clone.GetValue()[0] == 1);
            REQUIRE(clone.GetValue()[1] == 2);
            // Independent storage.
            REQUIRE(clone.GetValue().GetData() != opt.GetValue().GetData());
        }
    }

    SECTION("Comparison")
    {
        Optional<i32> a(1);
        Optional<i32> b(1);
        Optional<i32> c(2);
        Optional<i32> empty1;
        Optional<i32> empty2;
        REQUIRE(a == b);
        REQUIRE(a != c);
        REQUIRE(empty1 == empty2);
        REQUIRE(a != empty1);
    }

    SECTION("Hashing")
    {
        Hasher<Optional<i32>> hasher;
        Optional<i32> a(42);
        Optional<i32> b(42);
        Optional<i32> empty;
        REQUIRE(hasher(a) == hasher(b));
        REQUIRE(hasher(a) == Hasher<i32>{}(42));
        REQUIRE(hasher(empty) != hasher(a));
    }

    SECTION("Reference type")
    {
        SECTION("Default construction is empty")
        {
            Optional<i32&> opt;
            REQUIRE(opt.HasValue() == false);
        }
        SECTION("Construction binds reference")
        {
            i32 value = 10;
            Optional<i32&> opt(value);
            REQUIRE(opt.HasValue() == true);
            REQUIRE(opt.GetValue() == 10);
            opt.GetValue() = 20;
            REQUIRE(value == 20);
            REQUIRE(*opt == 20);
        }
        SECTION("GetValueOr")
        {
            i32 fallback = 5;
            Optional<i32&> empty;
            REQUIRE(empty.GetValueOr(fallback) == 5);
        }
        SECTION("Move transfers and empties source")
        {
            i32 value = 1;
            Optional<i32&> src(value);
            Optional<i32&> dst(Move(src));
            REQUIRE(dst.HasValue() == true);
            REQUIRE(src.HasValue() == false);
        }
        SECTION("Reset")
        {
            i32 value = 1;
            Optional<i32&> opt(value);
            opt.Reset();
            REQUIRE(opt.HasValue() == false);
        }
        SECTION("Clone shares the referenced object")
        {
            i32 value = 7;
            Optional<i32&> opt(value);
            Optional<i32&> clone = opt.Clone();
            REQUIRE(clone.HasValue() == true);
            REQUIRE(&clone.GetValue() == &value);
        }
    }
}
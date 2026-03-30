#include "test-helpers.h"

#include "opal/clonable-base.h"
#include "opal/variant.h"

using namespace Opal;

namespace
{

struct NonTrivialType
{
    i32 value = 0;
    bool* destroyed = nullptr;

    NonTrivialType() = default;
    explicit NonTrivialType(i32 v, bool* d = nullptr) : value(v), destroyed(d) {}

    NonTrivialType(const NonTrivialType&) = delete;
    NonTrivialType& operator=(const NonTrivialType&) = delete;

    NonTrivialType(NonTrivialType&& other) noexcept : value(other.value), destroyed(other.destroyed)
    {
        other.value = 0;
        other.destroyed = nullptr;
    }

    NonTrivialType& operator=(NonTrivialType&& other) noexcept
    {
        value = other.value;
        destroyed = other.destroyed;
        other.value = 0;
        other.destroyed = nullptr;
        return *this;
    }

    NonTrivialType Clone(AllocatorBase* = nullptr) const
    {
        NonTrivialType clone(value, destroyed);
        return clone;
    }

    ~NonTrivialType()
    {
        if (destroyed)
        {
            *destroyed = true;
        }
    }
};

struct ClonableType : ClonableBase<ClonableType>
{
    i32 value = 0;
    OPAL_CLONE_FIELDS(value);

    ClonableType() = default;
    ClonableType(ClonableType&& other) { value = other.value; }
    ClonableType& operator=(ClonableType&& other) { value = other.value; return *this; }
};

}  // namespace

// ------------------------------------------------------------------------------------------------
// Construction.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant default construction", "[Variant]")
{
    Variant<i32, f32> v;
    REQUIRE(v.GetIndex() == 0);
    REQUIRE(v.IsActive<i32>());
    REQUIRE_FALSE(v.IsActive<f32>());
    REQUIRE(v.Get<i32>() == 0);
}

TEST_CASE("Variant value construction with first alternative", "[Variant]")
{
    Variant<i32, f32> v(42);
    REQUIRE(v.GetIndex() == 0);
    REQUIRE(v.IsActive<i32>());
    REQUIRE(v.Get<i32>() == 42);
}

TEST_CASE("Variant value construction with second alternative", "[Variant]")
{
    Variant<i32, f32> v(1.5f);
    REQUIRE(v.GetIndex() == 1);
    REQUIRE(v.IsActive<f32>());
    REQUIRE(v.Get<f32>() == 1.5f);
}

TEST_CASE("Variant value construction from lvalue", "[Variant]")
{
    i32 value = 42;
    Variant<i32, f32> v(value);
    REQUIRE(v.IsActive<i32>());
    REQUIRE(v.Get<i32>() == 42);
}

TEST_CASE("Variant value construction with three alternatives", "[Variant]")
{
    SECTION("First")
    {
        Variant<i32, f32, f64> v(10);
        REQUIRE(v.GetIndex() == 0);
        REQUIRE(v.Get<i32>() == 10);
    }
    SECTION("Second")
    {
        Variant<i32, f32, f64> v(2.5f);
        REQUIRE(v.GetIndex() == 1);
        REQUIRE(v.Get<f32>() == 2.5f);
    }
    SECTION("Third")
    {
        Variant<i32, f32, f64> v(3.5);
        REQUIRE(v.GetIndex() == 2);
        REQUIRE(v.Get<f64>() == 3.5);
    }
}

TEST_CASE("Variant value construction with non-trivial type", "[Variant]")
{
    Variant<i32, NonTrivialType> v(NonTrivialType(99));
    REQUIRE(v.IsActive<NonTrivialType>());
    REQUIRE(v.Get<NonTrivialType>().value == 99);
}

// ------------------------------------------------------------------------------------------------
// Move semantics.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant move construction", "[Variant]")
{
    Variant<i32, f32> v1(42);
    Variant<i32, f32> v2(Move(v1));
    REQUIRE(v2.IsActive<i32>());
    REQUIRE(v2.Get<i32>() == 42);
}

TEST_CASE("Variant move construction with second alternative", "[Variant]")
{
    Variant<i32, f32> v1(1.5f);
    Variant<i32, f32> v2(Move(v1));
    REQUIRE(v2.IsActive<f32>());
    REQUIRE(v2.Get<f32>() == 1.5f);
}

TEST_CASE("Variant move construction of non-trivial type", "[Variant]")
{
    bool destroyed = false;
    Variant<i32, NonTrivialType> v1(NonTrivialType(77, &destroyed));
    Variant<i32, NonTrivialType> v2(Move(v1));
    REQUIRE(v2.IsActive<NonTrivialType>());
    REQUIRE(v2.Get<NonTrivialType>().value == 77);
    REQUIRE_FALSE(destroyed);
}

TEST_CASE("Variant move assignment", "[Variant]")
{
    Variant<i32, f32> v1(42);
    Variant<i32, f32> v2(1.5f);
    v2 = Move(v1);
    REQUIRE(v2.IsActive<i32>());
    REQUIRE(v2.Get<i32>() == 42);
}

TEST_CASE("Variant move assignment to self", "[Variant]")
{
    Variant<i32, f32> v(42);
    v = Move(v);
    REQUIRE(v.IsActive<i32>());
    REQUIRE(v.Get<i32>() == 42);
}

TEST_CASE("Variant move assignment destroys previous value", "[Variant]")
{
    bool destroyed = false;
    Variant<i32, NonTrivialType> v(NonTrivialType(99, &destroyed));
    REQUIRE_FALSE(destroyed);
    v = Variant<i32, NonTrivialType>(42);
    REQUIRE(destroyed);
    REQUIRE(v.IsActive<i32>());
    REQUIRE(v.Get<i32>() == 42);
}

// ------------------------------------------------------------------------------------------------
// Get by type.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant Get by type", "[Variant]")
{
    Variant<i32, f32, f64> v(2.5);
    REQUIRE(v.IsActive<f64>());
    REQUIRE(v.Get<f64>() == 2.5);
}

TEST_CASE("Variant Get wrong type throws", "[Variant]")
{
    Variant<i32, f32> v(42);
    REQUIRE_THROWS_AS(v.Get<f32>(), InvalidArgumentException);
}

TEST_CASE("Variant mutation through Get by type", "[Variant]")
{
    Variant<i32, f32> v(42);
    v.Get<i32>() = 100;
    REQUIRE(v.Get<i32>() == 100);
}

// ------------------------------------------------------------------------------------------------
// Get by index.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant Get by index", "[Variant]")
{
    Variant<i32, f32, f64> v(1.5f);
    REQUIRE(v.GetIndex() == 1);
    REQUIRE(v.Get<1>() == 1.5f);
}

TEST_CASE("Variant Get wrong index throws", "[Variant]")
{
    Variant<i32, f32> v(42);
    REQUIRE_THROWS_AS(v.Get<1>(), InvalidArgumentException);
}

TEST_CASE("Variant mutation through Get by index", "[Variant]")
{
    Variant<i32, f32> v(42);
    v.Get<0>() = 100;
    REQUIRE(v.Get<0>() == 100);
}

// ------------------------------------------------------------------------------------------------
// Const access.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant const Get by type", "[Variant]")
{
    const Variant<i32, f32> v(42);
    REQUIRE(v.Get<i32>() == 42);
}

TEST_CASE("Variant const Get by index", "[Variant]")
{
    const Variant<i32, f32> v(42);
    REQUIRE(v.Get<0>() == 42);
}

TEST_CASE("Variant const IsActive", "[Variant]")
{
    const Variant<i32, f32> v(42);
    REQUIRE(v.IsActive<i32>());
    REQUIRE_FALSE(v.IsActive<f32>());
}

TEST_CASE("Variant const Get wrong type throws", "[Variant]")
{
    const Variant<i32, f32> v(42);
    REQUIRE_THROWS_AS(v.Get<f32>(), InvalidArgumentException);
}

TEST_CASE("Variant const Get wrong index throws", "[Variant]")
{
    const Variant<i32, f32> v(42);
    REQUIRE_THROWS_AS(v.Get<1>(), InvalidArgumentException);
}

// ------------------------------------------------------------------------------------------------
// GetIndex.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant GetIndex", "[Variant]")
{
    SECTION("First alternative")
    {
        Variant<i32, f32, f64> v(42);
        REQUIRE(v.GetIndex() == 0);
    }
    SECTION("Second alternative")
    {
        Variant<i32, f32, f64> v(1.0f);
        REQUIRE(v.GetIndex() == 1);
    }
    SECTION("Third alternative")
    {
        Variant<i32, f32, f64> v(1.0);
        REQUIRE(v.GetIndex() == 2);
    }
}

// ------------------------------------------------------------------------------------------------
// Destruction.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant destroys non-trivial type on destruction", "[Variant]")
{
    bool destroyed = false;
    {
        Variant<i32, NonTrivialType> v(NonTrivialType(99, &destroyed));
        REQUIRE(v.IsActive<NonTrivialType>());
        REQUIRE(v.Get<NonTrivialType>().value == 99);
    }
    REQUIRE(destroyed);
}

TEST_CASE("Variant destroys first alternative on destruction", "[Variant]")
{
    bool destroyed = false;
    {
        Variant<NonTrivialType, i32> v(NonTrivialType(50, &destroyed));
        REQUIRE(v.IsActive<NonTrivialType>());
    }
    REQUIRE(destroyed);
}

// ------------------------------------------------------------------------------------------------
// Single alternative.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant with single alternative", "[Variant]")
{
    Variant<i32> v(42);
    REQUIRE(v.GetIndex() == 0);
    REQUIRE(v.IsActive<i32>());
    REQUIRE(v.Get<i32>() == 42);
    REQUIRE(v.Get<0>() == 42);
}

// ------------------------------------------------------------------------------------------------
// Clone.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant Clone with POD first alternative", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> cloned = v.Clone();
    REQUIRE(cloned.GetIndex() == 0);
    REQUIRE(cloned.IsActive<i32>());
    REQUIRE(cloned.Get<i32>() == 42);
}

TEST_CASE("Variant Clone with POD second alternative", "[Variant]")
{
    Variant<i32, f32> v(1.5f);
    Variant<i32, f32> cloned = v.Clone();
    REQUIRE(cloned.GetIndex() == 1);
    REQUIRE(cloned.IsActive<f32>());
    REQUIRE(cloned.Get<f32>() == 1.5f);
}

TEST_CASE("Variant Clone with clonable type", "[Variant]")
{
    ClonableType ct;
    ct.value = 99;
    Variant<i32, ClonableType> v(Move(ct));
    Variant<i32, ClonableType> cloned = v.Clone();
    REQUIRE(cloned.IsActive<ClonableType>());
    REQUIRE(cloned.Get<ClonableType>().value == 99);
}

TEST_CASE("Variant Clone does not affect original", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> cloned = v.Clone();
    cloned.Get<i32>() = 100;
    REQUIRE(v.Get<i32>() == 42);
    REQUIRE(cloned.Get<i32>() == 100);
}

TEST_CASE("Variant Clone with three alternatives", "[Variant]")
{
    Variant<i32, f32, f64> v(3.5);
    Variant<i32, f32, f64> cloned = v.Clone();
    REQUIRE(cloned.GetIndex() == 2);
    REQUIRE(cloned.Get<f64>() == 3.5);
}

// ------------------------------------------------------------------------------------------------
// Visit.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant Visit with generic lambda", "[Variant]")
{
    Variant<i32, f32> v(42);
    bool visited = false;
    v.Visit([&visited](auto& val)
    {
        (void)val;
        visited = true;
    });
    REQUIRE(visited);
}

TEST_CASE("Variant Visit with Overloaded", "[Variant]")
{
    Variant<i32, f32> v(42);
    i32 result = 0;
    v.Visit(Overloaded{
        [&result](i32& val) { result = val; },
        [](f32&) {},
    });
    REQUIRE(result == 42);
}

TEST_CASE("Variant Visit with Overloaded second alternative", "[Variant]")
{
    Variant<i32, f32> v(1.5f);
    f32 result = 0.0f;
    v.Visit(Overloaded{
        [](i32&) {},
        [&result](f32& val) { result = val; },
    });
    REQUIRE(result == 1.5f);
}

TEST_CASE("Variant Visit with return value", "[Variant]")
{
    Variant<i32, f32> v(42);
    i32 result = v.Visit(Overloaded{
        [](i32& val) -> i32 { return val * 2; },
        [](f32&) -> i32 { return -1; },
    });
    REQUIRE(result == 84);
}

TEST_CASE("Variant Visit mutates value", "[Variant]")
{
    Variant<i32, f32> v(42);
    v.Visit(Overloaded{
        [](i32& val) { val = 100; },
        [](f32&) {},
    });
    REQUIRE(v.Get<i32>() == 100);
}

TEST_CASE("Variant Visit on const variant", "[Variant]")
{
    const Variant<i32, f32> v(42);
    i32 result = v.Visit(Overloaded{
        [](const i32& val) -> i32 { return val; },
        [](const f32&) -> i32 { return -1; },
    });
    REQUIRE(result == 42);
}

TEST_CASE("Variant Visit with three alternatives", "[Variant]")
{
    Variant<i32, f32, f64> v(2.5);
    i32 which = v.Visit(Overloaded{
        [](i32&) -> i32 { return 0; },
        [](f32&) -> i32 { return 1; },
        [](f64&) -> i32 { return 2; },
    });
    REQUIRE(which == 2);
}

TEST_CASE("Variant Visit void return", "[Variant]")
{
    Variant<i32, f32> v(42);
    i32 out = 0;
    v.Visit([&out](auto& val) { out = static_cast<i32>(val); });
    REQUIRE(out == 42);
}

// ------------------------------------------------------------------------------------------------
// Moved-from state.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant Get throws on moved-from variant", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> v2(Move(v));
    REQUIRE_THROWS_AS(v.Get<i32>(), InvalidArgumentException);
    REQUIRE_THROWS_AS(v.Get<0>(), InvalidArgumentException);
}

TEST_CASE("Variant Clone throws on moved-from variant", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> v2(Move(v));
    REQUIRE_THROWS_AS(v.Clone(), InvalidArgumentException);
}

TEST_CASE("Variant Visit throws on moved-from variant", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> v2(Move(v));
    REQUIRE_THROWS_AS(v.Visit([](auto&) {}), InvalidArgumentException);
}

// ------------------------------------------------------------------------------------------------
// VisitPartial.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Variant VisitPartial calls matching handler", "[Variant]")
{
    Variant<i32, f32, f64> v(42);
    i32 result = 0;
    v.VisitPartial(Overloaded{
        [&result](i32& val) { result = val; },
    });
    REQUIRE(result == 42);
}

TEST_CASE("Variant VisitPartial ignores unhandled types", "[Variant]")
{
    Variant<i32, f32, f64> v(1.5f);
    i32 result = 0;
    v.VisitPartial(Overloaded{
        [&result](i32& val) { result = val; },
    });
    REQUIRE(result == 0);
}

TEST_CASE("Variant VisitPartial with multiple handlers", "[Variant]")
{
    Variant<i32, f32, f64> v(2.5);
    i32 which = -1;
    v.VisitPartial(Overloaded{
        [&which](i32&) { which = 0; },
        [&which](f64&) { which = 2; },
    });
    REQUIRE(which == 2);
}

TEST_CASE("Variant VisitPartial with no matching handler", "[Variant]")
{
    Variant<i32, f32, f64> v(1.5f);
    bool called = false;
    v.VisitPartial(Overloaded{
        [&called](i32&) { called = true; },
        [&called](f64&) { called = true; },
    });
    REQUIRE_FALSE(called);
}

TEST_CASE("Variant VisitPartial on const variant", "[Variant]")
{
    const Variant<i32, f32, f64> v(42);
    i32 result = 0;
    v.VisitPartial(Overloaded{
        [&result](const i32& val) { result = val; },
    });
    REQUIRE(result == 42);
}

TEST_CASE("Variant VisitPartial with generic lambda", "[Variant]")
{
    Variant<i32, f32, f64> v(42);
    bool visited = false;
    v.VisitPartial([&visited](auto&) { visited = true; });
    REQUIRE(visited);
}

TEST_CASE("Variant VisitPartial throws on moved-from variant", "[Variant]")
{
    Variant<i32, f32> v(42);
    Variant<i32, f32> v2(Move(v));
    REQUIRE_THROWS_AS(v.VisitPartial([](auto&) {}), InvalidArgumentException);
}
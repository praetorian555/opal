#include "test-helpers.h"

#include "opal/container/scope-ptr.h"

using namespace Opal;

TEST_CASE("ScopePtr creation", "[ScopePtr]")
{
    SECTION("Default construction")
    {
        ScopePtr<int> ptr;
        REQUIRE(ptr.Get() == nullptr);
    }
    SECTION("Construct with object")
    {
        int* int_ptr = New<int>(GetDefaultAllocator(), 5);
        ScopePtr<int> ptr(GetDefaultAllocator(), int_ptr);
        REQUIRE(ptr.Get() != nullptr);
        REQUIRE(*ptr.Get() == 5);
    }
    SECTION("Construction with value")
    {
        ScopePtr<int> ptr(GetDefaultAllocator(), 42);
        REQUIRE(ptr.Get() != nullptr);
        REQUIRE(*ptr.Get() == 42);
    }
    SECTION("Move construction")
    {
        ScopePtr<int> ptr(GetDefaultAllocator(), 42);
        ScopePtr<int> move(Move(ptr));
        REQUIRE(move.Get() != nullptr);
        REQUIRE(*move.Get() == 42);
        REQUIRE(ptr.Get() == nullptr);
    }
    SECTION("Move construction with derived type")
    {
        struct A
        {
        };
        struct B : public A
        {
        };

        ScopePtr<A> ptr = ScopePtr<B>(GetDefaultAllocator());
        ScopePtr<B> move(Move(ptr));
        REQUIRE(move.Get() != nullptr);
        REQUIRE(ptr.Get() == nullptr);
    }
}

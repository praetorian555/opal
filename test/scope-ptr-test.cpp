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
    SECTION("Move construction")
    {
        ScopePtr<int> ptr = MakeScoped<int>(GetDefaultAllocator(), 42);
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

        ScopePtr<A> ptr = MakeScoped<B>(nullptr);
        ScopePtr<B> move(Move(ptr));
        REQUIRE(move.Get() != nullptr);
        REQUIRE(ptr.Get() == nullptr);
    }
}

TEST_CASE("MakeScoped", "[ScopePtr]")
{
    SECTION("Basic construction")
    {
        ScopePtr<int> ptr = MakeScoped<int>(GetDefaultAllocator(), 42);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr == 42);
    }
    SECTION("Default allocator when nullptr")
    {
        ScopePtr<int> ptr = MakeScoped<int>(nullptr, 10);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr == 10);
    }
    SECTION("Derived to base conversion")
    {
        struct Base
        {
            virtual ~Base() = default;
            virtual int GetValue() const { return 0; }
        };
        struct Derived : public Base
        {
            int m_value;
            explicit Derived(int value) : m_value(value) {}
            int GetValue() const override { return m_value; }
        };

        ScopePtr<Base> ptr = MakeScoped<Base, Derived>(GetDefaultAllocator(), 99);
        REQUIRE(ptr.IsValid());
        REQUIRE(ptr->GetValue() == 99);
    }
}

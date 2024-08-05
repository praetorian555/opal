#include "catch2/catch2.hpp"

#include "opal/container/scope-ptr.h"

using namespace Opal;

TEST_CASE("ScopePtr creation", "[ScopePtr]")
{
    SECTION("Default construction")
    {
        ScopePtr<int> ptr;
        REQUIRE(ptr.Get() == nullptr);
    }
    SECTION("Construction with value")
    {
        ScopePtr<int> ptr(new int(42));
        REQUIRE(ptr.Get() != nullptr);
        REQUIRE(*ptr.Get() == 42);
    }
    SECTION("Move construction")
    {
        ScopePtr<int> ptr(new int(42));
        ScopePtr<int> move(std::move(ptr));
        REQUIRE(move.Get() != nullptr);
        REQUIRE(*move.Get() == 42);
        REQUIRE(ptr.Get() == nullptr);
    }
}

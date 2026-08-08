#include "test-helpers.h"

#include "opal/container/dynamic-array.h"
#include "opal/container/function.h"

using namespace Opal;

namespace
{

struct Counter
{
    i32 value = 0;

    void Add(i32 amount) { value += amount; }
    i32 Double() { return value * 2; }
};

i32 FreeAdd(i32 a, i32 b)
{
    return a + b;
}

}  // namespace

TEST_CASE("Function default state is unbound", "[Function]")
{
    Function<i32(i32)> function;
    REQUIRE_FALSE(function.IsBound());
}

TEST_CASE("Function holds a lambda", "[Function]")
{
    Function<i32(i32, i32)> function([](i32 a, i32 b) { return a * b; });
    REQUIRE(function.IsBound());
    REQUIRE(function(3, 4) == 12);
}

TEST_CASE("Function holds a function pointer", "[Function]")
{
    Function<i32(i32, i32)> function(&FreeAdd);
    REQUIRE(function(3, 4) == 7);
}

TEST_CASE("Function with void return type", "[Function]")
{
    i32 counter = 0;
    Function<void()> function([&counter]() { counter++; });
    function();
    REQUIRE(counter == 1);
}

TEST_CASE("Function with reference arguments", "[Function]")
{
    Function<void(i32&)> function([](i32& value) { value = 99; });
    i32 value = 0;
    function(value);
    REQUIRE(value == 99);
}

TEST_CASE("Function reset leaves it unbound", "[Function]")
{
    Function<i32()> function([]() { return 1; });
    REQUIRE(function.IsBound());

    function.Reset();
    REQUIRE_FALSE(function.IsBound());
}

TEST_CASE("Function destroys the callable it holds", "[Function]")
{
    i32 destructor_calls = 0;
    struct Tracker
    {
        i32* calls;
        ~Tracker()
        {
            if (calls != nullptr)
            {
                (*calls)++;
            }
        }
        Tracker(i32* c) : calls(c) {}
        Tracker(Tracker&& other) noexcept : calls(other.calls) { other.calls = nullptr; }
        Tracker(const Tracker& other) = default;
        void operator()() const {}
    };

    {
        Function<void()> function{Tracker(&destructor_calls)};
        function();
    }
    REQUIRE(destructor_calls == 1);
}

TEST_CASE("Function moves", "[Function]")
{
    SECTION("Move construction transfers the callable")
    {
        Function<i32()> source([]() { return 7; });
        Function<i32()> destination(Move(source));

        REQUIRE_FALSE(source.IsBound());
        REQUIRE(destination.IsBound());
        REQUIRE(destination() == 7);
    }
    SECTION("Move assignment replaces the callable")
    {
        Function<i32()> source([]() { return 7; });
        Function<i32()> destination([]() { return 1; });
        destination = Move(source);

        REQUIRE_FALSE(source.IsBound());
        REQUIRE(destination() == 7);
    }
    SECTION("Move assignment from an unbound function unbinds")
    {
        Function<i32()> source;
        Function<i32()> destination([]() { return 1; });
        destination = Move(source);

        REQUIRE_FALSE(destination.IsBound());
    }
    SECTION("Self move assignment keeps the callable")
    {
        Function<i32()> function([]() { return 7; });
        Function<i32()>& alias = function;
        function = Move(alias);

        REQUIRE(function.IsBound());
        REQUIRE(function() == 7);
    }
}

TEST_CASE("Function clones", "[Function]")
{
    SECTION("Clone holds an independent copy")
    {
        i32 counter = 0;
        Function<void()> original([&counter]() { counter++; });
        Function<void()> copy = original.Clone();

        original();
        copy();
        REQUIRE(counter == 2);
        REQUIRE(original.IsBound());
        REQUIRE(copy.IsBound());
    }
    SECTION("Cloning an unbound function yields an unbound function")
    {
        Function<void()> original;
        Function<void()> copy = original.Clone();
        REQUIRE_FALSE(copy.IsBound());
    }
}

TEST_CASE("Function holds a move only callable", "[Function]")
{
    DynamicArray<i32> owned = {1, 2, 3};
    Function<i32()> function([captured = Move(owned)]() { return static_cast<i32>(captured.GetSize()); });

    REQUIRE(function.IsBound());
    REQUIRE(function() == 3);
}

TEST_CASE("Function binds a member function", "[Function]")
{
    SECTION("With arguments")
    {
        Counter counter;
        Function<void(i32)> function = Function<void(i32)>::FromMethod<&Counter::Add>(&counter);

        function(5);
        function(3);
        REQUIRE(counter.value == 8);
    }
    SECTION("With a return value")
    {
        Counter counter;
        counter.value = 21;
        Function<i32()> function = Function<i32()>::FromMethod<&Counter::Double>(&counter);
        REQUIRE(function() == 42);
    }
}

TEST_CASE("Function capacity is configurable", "[Function]")
{
    // Read through a reference so the captures are genuinely stored, not folded into the lambda.
    DynamicArray<i64> source = {1, 2, 3, 4, 5, 6};
    const i64 a = source.At(0), b = source.At(1), c = source.At(2), d = source.At(3), e = source.At(4), f = source.At(5);
    Function<i64(), 64> function([a, b, c, d, e, f]() { return a + b + c + d + e + f; });

    static_assert(sizeof(decltype(function)) >= 64, "Storage must hold the requested capacity");

    REQUIRE(decltype(function)::k_storage_size == 64);
    REQUIRE(function() == 21);
}

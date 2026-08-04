#include <string>

#include "test-helpers.h"

#include "opal/container/expected.h"

using namespace Opal;

TEST_CASE("Expected", "[Expected]")
{
    SECTION("Regular type")
    {
        STATIC_REQUIRE_FALSE(std::is_default_constructible_v<Expected<int32_t, std::string>>);
        STATIC_REQUIRE_FALSE(std::is_default_constructible_v<Expected<int32_t&, std::string>>);

        SECTION("Construction with value")
        {
            Expected<int32_t, std::string> expected(42);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 42);
        }
        SECTION("Construction with error")
        {
            Expected<int32_t, std::string> expected("Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Move construction")
        {
            Expected<int32_t, std::string> expected(42);
            Expected<int32_t, std::string> move(std::move(expected));
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
        }
        SECTION("Move construction of a value that owns memory")
        {
            // A long string does not fit the small buffer, so moving it into unconstructed storage would free a garbage pointer.
            Expected<std::string, int32_t> expected(std::string(64, 'x'));
            Expected<std::string, int32_t> move(std::move(expected));
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == std::string(64, 'x'));
        }
        SECTION("Move construction of an error that owns memory")
        {
            Expected<int32_t, std::string> expected(std::string(64, 'e'));
            Expected<int32_t, std::string> move(std::move(expected));
            REQUIRE(move.HasValue() == false);
            REQUIRE(move.GetError() == std::string(64, 'e'));
        }
    }
    SECTION("Reference type")
    {
        SECTION("Construction with value")
        {
            int32_t value = 42;
            Expected<int32_t&, std::string> expected(value);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 42);
            REQUIRE(&expected.GetValue() == &value);
        }
        SECTION("Construction with error")
        {
            Expected<int32_t&, std::string> expected("Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Move construction")
        {
            int32_t value = 42;
            Expected<int32_t&, std::string> expected(value);
            Expected<int32_t&, std::string> move(std::move(expected));
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
        }
    }
}

TEST_CASE("Assignment of expected", "[Expected]")
{
    SECTION("Regular type")
    {
        SECTION("Move assignment with expected value")
        {
            Expected<int32_t, std::string> expected(42);
            Expected<int32_t, std::string> move(0);
            move = std::move(expected);
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
        }
        SECTION("Move assignment with error")
        {
            Expected<int32_t, std::string> expected("Error");
            Expected<int32_t, std::string> move(0);
            move = std::move(expected);
            REQUIRE(move.HasValue() == false);
            REQUIRE(move.GetError() == "Error");
        }
    }
    SECTION("Reference type")
    {
        SECTION("Move assignment with expected value")
        {
            int32_t value = 42;
            int32_t value2 = 43;
            Expected<int32_t&, std::string> expected(value);
            Expected<int32_t&, std::string> move(value2);
            move = std::move(expected);
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
            REQUIRE(&move.GetValue() == &value);
        }
        SECTION("Move assignment with error")
        {
            Expected<int32_t&, std::string> expected("Error");
            Expected<int32_t&, std::string> move("");
            move = std::move(expected);
            REQUIRE(move.HasValue() == false);
            REQUIRE(move.GetError() == "Error");
        }
    }
}

TEST_CASE("Expected defaulted accessors", "[Expected]")
{
    SECTION("GetValueOr")
    {
        REQUIRE(Expected<int32_t, std::string>(42).GetValueOr(7) == 42);
        REQUIRE(Expected<int32_t, std::string>("Error").GetValueOr(7) == 7);
    }
    SECTION("GetErrorOr")
    {
        REQUIRE(Expected<int32_t, std::string>("Error").GetErrorOr("None") == "Error");
        REQUIRE(Expected<int32_t, std::string>(42).GetErrorOr("None") == "None");
    }
    SECTION("GetErrorOr on the reference specialization")
    {
        int32_t value = 42;
        REQUIRE(Expected<int32_t&, std::string>(value).GetErrorOr("None") == "None");
        REQUIRE(Expected<int32_t&, std::string>("Error").GetErrorOr("None") == "Error");
    }
}

TEST_CASE("Expected chaining", "[Expected]")
{
    const auto double_it = [](int32_t v) { return Expected<int32_t, std::string>(v * 2); };
    const auto fail = [](int32_t) { return Expected<int32_t, std::string>("Failed"); };

    SECTION("AndThen runs on a value")
    {
        auto result = Expected<int32_t, std::string>(21).AndThen(double_it);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 42);
    }
    SECTION("AndThen passes an error through without calling the function")
    {
        bool called = false;
        auto result = Expected<int32_t, std::string>("Error").AndThen(
            [&called](int32_t v)
            {
                called = true;
                return Expected<int32_t, std::string>(v);
            });
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == "Error");
        REQUIRE_FALSE(called);
    }
    SECTION("AndThen reports the failure of the function it ran")
    {
        auto result = Expected<int32_t, std::string>(21).AndThen(fail);
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == "Failed");
    }
    SECTION("AndThen chains")
    {
        auto result = Expected<int32_t, std::string>(5).AndThen(double_it).AndThen(double_it);
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 20);
    }
    SECTION("Map wraps a plain result")
    {
        auto result = Expected<int32_t, std::string>(21).Map([](int32_t v) { return v * 2; });
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 42);
    }
    SECTION("Map can change the value type")
    {
        // The error type stays int32_t here on purpose: Expected<T, E> has an explicit constructor for each, so T and E cannot be
        // the same type, and mapping an int32_t value to a std::string with a std::string error would ask for exactly that.
        auto result = Expected<int32_t, int64_t>(42).Map([](int32_t v) { return std::to_string(v); });
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == "42");
    }
    SECTION("Map passes an error through without calling the function")
    {
        bool called = false;
        auto result = Expected<int32_t, std::string>("Error").Map(
            [&called](int32_t v)
            {
                called = true;
                return v;
            });
        REQUIRE_FALSE(result.HasValue());
        REQUIRE(result.GetError() == "Error");
        REQUIRE_FALSE(called);
    }
    SECTION("Map moves a value that owns memory")
    {
        auto result = Expected<std::string, int32_t>(std::string(64, 'x')).Map([](std::string v) { return v.size(); });
        REQUIRE(result.HasValue());
        REQUIRE(result.GetValue() == 64);
    }
}

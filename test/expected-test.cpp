#include <string>

#include "catch2/catch2.hpp"

#include "opal/container/expected.h"

TEST_CASE("Expected", "[Expected]")
{
    SECTION("Regular type")
    {
        SECTION("Default construction")
        {
            Opal::Expected<int32_t, std::string> expected;
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 0);
        }
        SECTION("Construction with value")
        {
            Opal::Expected<int32_t, std::string> expected(42);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 42);
        }
        SECTION("Construction with error")
        {
            Opal::Expected<int32_t, std::string> expected("Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Copy construction")
        {
            Opal::Expected<int32_t, std::string> expected(42);
            Opal::Expected<int32_t, std::string> copy(expected);
            REQUIRE(copy.HasValue() == true);
            REQUIRE(copy.GetValue() == 42);
        }
        SECTION("Move construction")
        {
            Opal::Expected<int32_t, std::string> expected(42);
            Opal::Expected<int32_t, std::string> move(std::move(expected));
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 0);
        }
    }
    SECTION("Reference type")
    {
        SECTION("Default construction")
        {
            Opal::Expected<int32_t&, std::string> expected;
            REQUIRE(expected.HasValue() == false);
        }
        SECTION("Construction with value")
        {
            int32_t value = 42;
            Opal::Expected<int32_t&, std::string> expected(value);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 42);
            REQUIRE(&expected.GetValue() == &value);
        }
        SECTION("Construction with error")
        {
            Opal::Expected<int32_t&, std::string> expected("Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Copy construction")
        {
            int32_t value = 42;
            Opal::Expected<int32_t&, std::string> expected(value);
            Opal::Expected<int32_t&, std::string> copy(expected);
            REQUIRE(copy.HasValue() == true);
            REQUIRE(copy.GetValue() == 42);
            REQUIRE(&copy.GetValue() == &value);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(&expected.GetValue() == &value);
        }
        SECTION("Move construction")
        {
            int32_t value = 42;
            Opal::Expected<int32_t&, std::string> expected(value);
            Opal::Expected<int32_t&, std::string> move(std::move(expected));
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
            REQUIRE(&move.GetValue() == &value);
            REQUIRE(expected.HasValue() == false);
        }
    }
}

TEST_CASE("Assignment of expected", "[Expected]")
{
    SECTION("Regular type")
    {
        SECTION("Copy assignment with expected value")
        {
            Opal::Expected<int32_t, std::string> expected(42);
            Opal::Expected<int32_t, std::string> copy;
            copy = expected;
            REQUIRE(copy.HasValue() == true);
            REQUIRE(copy.GetValue() == 42);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(expected.GetValue() == 42);
        }
        SECTION("Copy assignment with error")
        {
            Opal::Expected<int32_t, std::string> expected("Error");
            Opal::Expected<int32_t, std::string> copy;
            copy = expected;
            REQUIRE(copy.HasValue() == false);
            REQUIRE(copy.GetError() == "Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Move assignment with expected value")
        {
            Opal::Expected<int32_t, std::string> expected(42);
            Opal::Expected<int32_t, std::string> move;
            move = std::move(expected);
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
        }
        SECTION("Move assignment with error")
        {
            Opal::Expected<int32_t, std::string> expected("Error");
            Opal::Expected<int32_t, std::string> move;
            move = std::move(expected);
            REQUIRE(move.HasValue() == false);
            REQUIRE(move.GetError() == "Error");
        }
    }
    SECTION("Reference type")
    {
        SECTION("Copy assignment with expected value")
        {
            int32_t value = 42;
            Opal::Expected<int32_t&, std::string> expected(value);
            Opal::Expected<int32_t&, std::string> copy;
            copy = expected;
            REQUIRE(copy.HasValue() == true);
            REQUIRE(copy.GetValue() == 42);
            REQUIRE(&copy.GetValue() == &value);
            REQUIRE(expected.HasValue() == true);
            REQUIRE(&expected.GetValue() == &value);
        }
        SECTION("Copy assignment with error")
        {
            Opal::Expected<int32_t&, std::string> expected("Error");
            Opal::Expected<int32_t&, std::string> copy;
            copy = expected;
            REQUIRE(copy.HasValue() == false);
            REQUIRE(copy.GetError() == "Error");
            REQUIRE(expected.HasValue() == false);
            REQUIRE(expected.GetError() == "Error");
        }
        SECTION("Move assignment with expected value")
        {
            int32_t value = 42;
            Opal::Expected<int32_t&, std::string> expected(value);
            Opal::Expected<int32_t&, std::string> move;
            move = std::move(expected);
            REQUIRE(move.HasValue() == true);
            REQUIRE(move.GetValue() == 42);
            REQUIRE(&move.GetValue() == &value);
        }
        SECTION("Move assignment with error")
        {
            Opal::Expected<int32_t&, std::string> expected("Error");
            Opal::Expected<int32_t&, std::string> move;
            move = std::move(expected);
            REQUIRE(move.HasValue() == false);
            REQUIRE(move.GetError() == "Error");
        }
    }
}

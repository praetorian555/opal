#include "test-helpers.h"

#include "opal/enum-flags.h"

using namespace Opal;

enum class TestFlags : u8
{
    None = 0,
    A = 1 << 0,
    B = 1 << 1,
    C = 1 << 2,
    All = A | B | C,
};
OPAL_ENUM_CLASS_FLAGS(TestFlags)

TEST_CASE("EnumFlags bitwise OR", "[EnumFlags]")
{
    TestFlags flags = TestFlags::A | TestFlags::B;
    REQUIRE((flags & TestFlags::A) == TestFlags::A);
    REQUIRE((flags & TestFlags::B) == TestFlags::B);
    REQUIRE((flags & TestFlags::C) != TestFlags::C);
}

TEST_CASE("EnumFlags bitwise AND", "[EnumFlags]")
{
    TestFlags flags = TestFlags::All;
    TestFlags result = flags & TestFlags::B;
    REQUIRE(result == TestFlags::B);
}

TEST_CASE("EnumFlags bitwise XOR", "[EnumFlags]")
{
    TestFlags flags = TestFlags::A | TestFlags::B;
    TestFlags result = flags ^ TestFlags::A;
    REQUIRE(result == TestFlags::B);
}

TEST_CASE("EnumFlags bitwise NOT", "[EnumFlags]")
{
    TestFlags flags = ~TestFlags::None;
    REQUIRE((flags & TestFlags::A) == TestFlags::A);
    REQUIRE((flags & TestFlags::B) == TestFlags::B);
    REQUIRE((flags & TestFlags::C) == TestFlags::C);
}

TEST_CASE("EnumFlags logical NOT", "[EnumFlags]")
{
    REQUIRE(!TestFlags::None);
    REQUIRE_FALSE(!TestFlags::A);
}

TEST_CASE("EnumFlags OR assignment", "[EnumFlags]")
{
    TestFlags flags = TestFlags::None;
    flags |= TestFlags::A;
    flags |= TestFlags::C;
    REQUIRE((flags & TestFlags::A) == TestFlags::A);
    REQUIRE((flags & TestFlags::B) != TestFlags::B);
    REQUIRE((flags & TestFlags::C) == TestFlags::C);
}

TEST_CASE("EnumFlags AND assignment", "[EnumFlags]")
{
    TestFlags flags = TestFlags::All;
    flags &= TestFlags::B;
    REQUIRE(flags == TestFlags::B);
}

TEST_CASE("EnumFlags XOR assignment", "[EnumFlags]")
{
    TestFlags flags = TestFlags::A | TestFlags::B;
    flags ^= TestFlags::B;
    REQUIRE(flags == TestFlags::A);
}

TEST_CASE("EnumFlags toggle flag on and off", "[EnumFlags]")
{
    TestFlags flags = TestFlags::None;
    flags |= TestFlags::A;
    REQUIRE((flags & TestFlags::A) == TestFlags::A);

    flags &= ~TestFlags::A;
    REQUIRE((flags & TestFlags::A) != TestFlags::A);
}
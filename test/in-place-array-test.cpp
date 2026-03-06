#include "test-helpers.h"

#include "opal/clonable-base.h"
#include "opal/container/in-place-array.h"
#include "opal/container/string.h"

using namespace Opal;

namespace
{
struct ClonableData : ClonableBase<ClonableData>
{
    StringUtf8 name;
    i32 value = 0;

    OPAL_CLONE_FIELDS(name, value);

    bool operator==(const ClonableData& other) const { return name == other.name && value == other.value; }
    bool operator!=(const ClonableData& other) const { return !(*this == other); }
};
}  // namespace

TEST_CASE("Construction", "[InPlaceArray][Constructor]")
{
    SECTION("default construction")
    {
        InPlaceArray<int32_t, 3> array;
        REQUIRE(array.GetSize() == 3);
    }
    SECTION("initializer list construction")
    {
        InPlaceArray<int32_t, 5> array({1, 2, 3});
        REQUIRE(array.GetSize() == 5);
        REQUIRE(array[0] == 1);
        REQUIRE(array[1] == 2);
        REQUIRE(array[2] == 3);
        REQUIRE(array[3] == 0);
        REQUIRE(array[4] == 0);

        InPlaceArray<int32_t, 5> array2{1, 2, 3, 4, 5};
        REQUIRE(array2.GetSize() == 5);
        REQUIRE(array2[0] == 1);
        REQUIRE(array2[1] == 2);
        REQUIRE(array2[2] == 3);
        REQUIRE(array2[3] == 4);
        REQUIRE(array2[4] == 5);

        InPlaceArray<int32_t, 5> array3{};
        REQUIRE(array3.GetSize() == 5);
        REQUIRE(array3[0] == 0);
        REQUIRE(array3[1] == 0);
        REQUIRE(array3[2] == 0);
        REQUIRE(array3[3] == 0);
        REQUIRE(array3[4] == 0);
    }
    SECTION("Initializer list with clonable type")
    {
        InPlaceArray<StringUtf8, 3> array{"Hello", "There", "World"};
        REQUIRE(array[0] == "Hello");
        REQUIRE(array[1] == "There");
        REQUIRE(array[2] == "World");
    }
    SECTION("Initializer list with clonable type and not enough members")
    {
        InPlaceArray<StringUtf8, 4> array{"Hello", "There", "World"};
        REQUIRE(array[0] == "Hello");
        REQUIRE(array[1] == "There");
        REQUIRE(array[2] == "World");
        REQUIRE(array[3] == "");
    }
}

TEST_CASE("Get data", "[InPlaceArray][Getter]")
{
    InPlaceArray<int32_t, 3> array({1, 2, 3});
    REQUIRE(array.GetData() == &array[0]);

    const InPlaceArray<int32_t, 3> const_array({1, 2, 3});
    REQUIRE(const_array.GetData() == &const_array[0]);
}

TEST_CASE("Swap", "[InPlaceArray][Swap]")
{
    InPlaceArray<int32_t, 2> a = {1, 2};
    InPlaceArray<int32_t, 2> b = {3, 4};
    Opal::Swap(a, b);
    REQUIRE(a[0] == 3);
    REQUIRE(a[1] == 4);
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
}

TEST_CASE("Comparison", "[InPlaceArray][Comparison]")
{
    InPlaceArray<int32_t, 3> a = {1, 2, 3};
    InPlaceArray<int32_t, 3> b = {1, 2, 3};
    InPlaceArray<int32_t, 3> c = {1, 2, 4};

    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("At", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.At(0).GetValue() == 1);
    REQUIRE(array.At(1).GetValue() == 2);
    REQUIRE(array.At(2).GetValue() == 3);
    REQUIRE(array.At(3).GetError() == ErrorCode::OutOfBounds);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.At(0).GetValue() == 1);
    REQUIRE(const_array.At(1).GetValue() == 2);
    REQUIRE(const_array.At(2).GetValue() == 3);
    REQUIRE(const_array.At(3).GetError() == ErrorCode::OutOfBounds);
}

TEST_CASE("Front", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.Front() == 1);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.Front() == 1);
}

TEST_CASE("Back", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    REQUIRE(array.Back() == 3);

    const InPlaceArray<int32_t, 3> const_array = {1, 2, 3};
    REQUIRE(const_array.Back() == 3);
}

TEST_CASE("Fill", "[InPlaceArray]")
{
    InPlaceArray<int32_t, 3> array = {1, 2, 3};
    array.Fill(42);
    REQUIRE(array[0] == 42);
    REQUIRE(array[1] == 42);
    REQUIRE(array[2] == 42);
}

TEST_CASE("Iterator", "[InPlaceArray]")
{
    SECTION("Increment")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        InPlaceArray<i32, 3>::iterator it = int_arr.begin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 43);
        ++it;
        REQUIRE(*it == 44);
        ++it;
        REQUIRE(it == int_arr.end());
    }
    SECTION("Post increment")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        InPlaceArray<i32, 3>::iterator it = int_arr.begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 43);
        it++;
        REQUIRE(*it == 44);
        auto prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.end());
    }
    SECTION("Decrement")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        --it;
        REQUIRE(*it == 44);
        --it;
        REQUIRE(*it == 43);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Post decrement")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        it--;
        REQUIRE(*it == 44);
        it--;
        REQUIRE(*it == 43);
        auto prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Add")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 43);
        REQUIRE(*(it + 2) == 44);
        REQUIRE((it + 3) == int_arr.end());

        auto it2 = int_arr.begin();
        REQUIRE((3 + it2) == int_arr.end());
    }
    SECTION("Add assignment")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        InPlaceArray<i32, 3>::iterator it = int_arr.begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 43);
        REQUIRE(*(it += 1) == 44);
        REQUIRE((it += 1) == int_arr.end());
    }
    SECTION("Subtract")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        REQUIRE((it - 0) == int_arr.end());
        REQUIRE(*(it - 1) == 44);
        REQUIRE(*(it - 2) == 43);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.begin());
    }
    SECTION("Subtract assignment")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        REQUIRE((it -= 0) == int_arr.end());
        REQUIRE(*(it -= 1) == 44);
        REQUIRE(*(it -= 1) == 43);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Difference")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it1 = int_arr.begin();
        auto it2 = int_arr.end();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Access")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 43);
        REQUIRE(it[2] == 44);
    }
    SECTION("Dereference")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        struct Data
        {
            i32 value = 5;
        };

        InPlaceArray<Data, 1> arr{};
        auto it = arr.begin();
        REQUIRE(it->value == 5);
    }
    SECTION("Compare")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it1 = int_arr.begin();
        auto it2 = int_arr.begin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        i32 sum = 0;
        for (auto it = int_arr.begin(); it != int_arr.end(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 129);
    }
    SECTION("Modern for loop")
    {
        InPlaceArray<i32, 3> int_arr{42, 43, 44};
        i32 sum = 0;
        for (i32 val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 129);
    }
}

TEST_CASE("Const iterator", "[InPlaceArray]")
{
    SECTION("Increment")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 43);
        ++it;
        REQUIRE(*it == 44);
        ++it;
        REQUIRE(it == int_arr.end());
    }
    SECTION("Post increment")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 43);
        it++;
        REQUIRE(*it == 44);
        auto prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.end());
    }
    SECTION("Decrement")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        --it;
        REQUIRE(*it == 44);
        --it;
        REQUIRE(*it == 43);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Post decrement")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        it--;
        REQUIRE(*it == 44);
        it--;
        REQUIRE(*it == 43);
        auto prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Add")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 43);
        REQUIRE(*(it + 2) == 44);
        REQUIRE((it + 3) == int_arr.end());

        auto it2 = int_arr.begin();
        REQUIRE((3 + it2) == int_arr.end());
    }
    SECTION("Add assignment")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 43);
        REQUIRE(*(it += 1) == 44);
        REQUIRE((it += 1) == int_arr.cend());
    }
    SECTION("Subtract")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        REQUIRE((it - 0) == int_arr.cend());
        REQUIRE(*(it - 1) == 44);
        REQUIRE(*(it - 2) == 43);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.begin());
    }
    SECTION("Subtract assignment")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.end();
        REQUIRE((it -= 0) == int_arr.end());
        REQUIRE(*(it -= 1) == 44);
        REQUIRE(*(it -= 1) == 43);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.cbegin());
    }
    SECTION("Difference")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it1 = int_arr.begin();
        auto it2 = int_arr.end();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Access")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 43);
        REQUIRE(it[2] == 44);
    }
    SECTION("Dereference")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it = int_arr.begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        struct Data
        {
            i32 value = 5;
        };

        const InPlaceArray<Data, 1> arr{};
        auto it = arr.begin();
        REQUIRE(it->value == 5);
    }
    SECTION("Compare")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        auto it1 = int_arr.begin();
        auto it2 = int_arr.begin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        i32 sum = 0;
        for (auto it = int_arr.begin(); it != int_arr.end(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 129);
    }
    SECTION("Modern for loop")
    {
        const InPlaceArray<i32, 3> int_arr{42, 43, 44};
        i32 sum = 0;
        for (i32 val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 129);
    }
}

TEST_CASE("Clone with POD elements", "[InPlaceArray][Clone]")
{
    InPlaceArray<i32, 3> original = {10, 20, 30};
    InPlaceArray<i32, 3> cloned = original.Clone();
    REQUIRE(cloned[0] == 10);
    REQUIRE(cloned[1] == 20);
    REQUIRE(cloned[2] == 30);
}

TEST_CASE("Clone is a deep copy with Clonable elements", "[InPlaceArray][Clone]")
{
    InPlaceArray<ClonableData, 2> original;
    original[0].name = "Alice";
    original[0].value = 1;
    original[1].name = "Bob";
    original[1].value = 2;

    InPlaceArray<ClonableData, 2> cloned = original.Clone();
    REQUIRE(cloned[0].name == "Alice");
    REQUIRE(cloned[0].value == 1);
    REQUIRE(cloned[1].name == "Bob");
    REQUIRE(cloned[1].value == 2);

    original[0].name = "Modified";
    original[0].value = 99;
    REQUIRE(cloned[0].name == "Alice");
    REQUIRE(cloned[0].value == 1);
}

TEST_CASE("Default construction with Clonable elements", "[InPlaceArray][Constructor]")
{
    InPlaceArray<ClonableData, 2> array;
    REQUIRE(array.GetSize() == 2);
}

TEST_CASE("Move construction with Clonable elements", "[InPlaceArray][Constructor]")
{
    InPlaceArray<ClonableData, 2> original;
    original[0].name = "Hello";
    original[0].value = 42;

    InPlaceArray<ClonableData, 2> moved(Move(original));
    REQUIRE(moved[0].name == "Hello");
    REQUIRE(moved[0].value == 42);
}

TEST_CASE("Move assignment with Clonable elements", "[InPlaceArray][Constructor]")
{
    InPlaceArray<ClonableData, 2> original;
    original[0].name = "Hello";
    original[0].value = 42;

    InPlaceArray<ClonableData, 2> assigned;
    assigned = Move(original);
    REQUIRE(assigned[0].name == "Hello");
    REQUIRE(assigned[0].value == 42);
}

TEST_CASE("Access with Clonable elements", "[InPlaceArray]")
{
    InPlaceArray<ClonableData, 3> array;
    array[0].name = "A";
    array[0].value = 1;
    array[1].name = "B";
    array[1].value = 2;
    array[2].name = "C";
    array[2].value = 3;

    REQUIRE(array[0].name == "A");
    REQUIRE(array[1].name == "B");
    REQUIRE(array[2].name == "C");

    REQUIRE(array.At(0).GetValue().name == "A");
    REQUIRE(array.At(1).GetValue().name == "B");
    REQUIRE(array.At(2).GetValue().name == "C");
    REQUIRE(array.At(3).HasValue() == false);

    REQUIRE(array.Front().name == "A");
    REQUIRE(array.Back().name == "C");
}

TEST_CASE("Fill with Clonable elements", "[InPlaceArray]")
{
    ClonableData fill_value;
    fill_value.name = "Filled";
    fill_value.value = 7;

    InPlaceArray<ClonableData, 3> array;
    array.Fill(fill_value);
    REQUIRE(array[0].name == "Filled");
    REQUIRE(array[0].value == 7);
    REQUIRE(array[1].name == "Filled");
    REQUIRE(array[1].value == 7);
    REQUIRE(array[2].name == "Filled");
    REQUIRE(array[2].value == 7);
}

TEST_CASE("Swap with Clonable elements", "[InPlaceArray][Swap]")
{
    InPlaceArray<ClonableData, 2> a;
    a[0].name = "A0";
    a[0].value = 1;
    a[1].name = "A1";
    a[1].value = 2;

    InPlaceArray<ClonableData, 2> b;
    b[0].name = "B0";
    b[0].value = 3;
    b[1].name = "B1";
    b[1].value = 4;

    Opal::Swap(a, b);
    REQUIRE(a[0].name == "B0");
    REQUIRE(a[0].value == 3);
    REQUIRE(a[1].name == "B1");
    REQUIRE(a[1].value == 4);
    REQUIRE(b[0].name == "A0");
    REQUIRE(b[0].value == 1);
    REQUIRE(b[1].name == "A1");
    REQUIRE(b[1].value == 2);
}

TEST_CASE("Comparison with Clonable elements", "[InPlaceArray][Comparison]")
{
    InPlaceArray<ClonableData, 2> a;
    a[0].name = "X";
    a[0].value = 1;
    a[1].name = "Y";
    a[1].value = 2;

    InPlaceArray<ClonableData, 2> b = a.Clone();
    REQUIRE(a == b);

    b[0].value = 99;
    REQUIRE(a != b);
}

TEST_CASE("Iterator with Clonable elements", "[InPlaceArray]")
{
    InPlaceArray<ClonableData, 3> array;
    array[0].name = "A";
    array[0].value = 1;
    array[1].name = "B";
    array[1].value = 2;
    array[2].name = "C";
    array[2].value = 3;

    i32 sum = 0;
    for (const auto& item : array)
    {
        sum += item.value;
    }
    REQUIRE(sum == 6);
}

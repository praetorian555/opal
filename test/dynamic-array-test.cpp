#include "catch2/catch2.hpp"

#include "opal/container/dynamic-array.h"

namespace
{
i32 g_default_call_count = 0;
i32 g_value_call_count = 0;
i32 g_copy_call_count = 0;
i32 g_copy_assign_call_count = 0;
i32 g_destroy_call_count = 0;
struct NonPod
{
    i32* ptr = nullptr;
    NonPod()
    {
        ptr = new i32(5);
        g_default_call_count++;
    }
    explicit NonPod(i32 value)
    {
        ptr = new i32(value);
        g_value_call_count++;
    }
    NonPod(const NonPod& other)
    {
        ptr = new i32(*other.ptr);
        g_copy_call_count++;
    }
    NonPod& operator=(const NonPod& other)
    {
        if (this != &other)
        {
            delete ptr;
            ptr = new i32(*other.ptr);
        }
        g_copy_assign_call_count++;
        return *this;
    }
    NonPod(NonPod&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    NonPod& operator=(NonPod&& other) noexcept
    {
        if (this != &other)
        {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    ~NonPod()
    {
        delete ptr;
        g_destroy_call_count++;
    }
};
}  // namespace

TEST_CASE("Construction with POD data", "[DynamicArray]")
{
    SECTION("Size constructor")
    {
        SECTION("Initial size smaller then default capacity")
        {
            Opal::DynamicArray<i32> int_arr(3);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 0);
            REQUIRE(int_arr.GetData()[1] == 0);
            REQUIRE(int_arr.GetData()[2] == 0);
        }
        SECTION("Initial size larger then default capacity")
        {
            Opal::DynamicArray<i32> int_arr(5);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 0);
            REQUIRE(int_arr.GetData()[1] == 0);
            REQUIRE(int_arr.GetData()[2] == 0);
            REQUIRE(int_arr.GetData()[3] == 0);
            REQUIRE(int_arr.GetData()[4] == 0);
        }
    }
    SECTION("Size and default value constructor")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Copy constructor")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32> int_arr_copy(int_arr);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
        REQUIRE(int_arr_copy.GetCapacity() == 4);
        REQUIRE(int_arr_copy.GetSize() == 3);
        REQUIRE(int_arr_copy.GetData() != nullptr);
        REQUIRE(int_arr_copy.GetData()[0] == 42);
        REQUIRE(int_arr_copy.GetData()[1] == 42);
        REQUIRE(int_arr_copy.GetData()[2] == 42);
    }
    SECTION("Move constructor")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32> int_arr_copy(std::move(int_arr));
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr_copy.GetCapacity() == 4);
        REQUIRE(int_arr_copy.GetSize() == 3);
        REQUIRE(int_arr_copy.GetData() != nullptr);
        REQUIRE(int_arr_copy.GetData()[0] == 42);
        REQUIRE(int_arr_copy.GetData()[1] == 42);
        REQUIRE(int_arr_copy.GetData()[2] == 42);
    }
}

TEST_CASE("Construction with non-POD data", "[DynamicArray]")
{
    SECTION("Size constructor")
    {
        SECTION("Initial size smaller then default capacity")
        {
            g_default_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3);
                REQUIRE(non_pod_arr.GetCapacity() == 4);
                REQUIRE(non_pod_arr.GetSize() == 3);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 5);
                REQUIRE(g_default_call_count == 3);
            }
            REQUIRE(g_destroy_call_count == 3);
        }
        SECTION("Initial size larger then default capacity")
        {
            g_default_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(5);
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 5);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[3].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[4].ptr == 5);
                REQUIRE(g_default_call_count == 5);
            }
            REQUIRE(g_destroy_call_count == 5);
        }
    }
    SECTION("Size and default value constructor")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            NonPod default_value(42);
            Opal::DynamicArray<NonPod> non_pod_arr(3, default_value);
            REQUIRE(non_pod_arr.GetCapacity() == 4);
            REQUIRE(non_pod_arr.GetSize() == 3);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
    SECTION("Copy constructor")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            Opal::DynamicArray<NonPod> non_pod_arr_copy(non_pod_arr);
            REQUIRE(non_pod_arr.GetCapacity() == 4);
            REQUIRE(non_pod_arr.GetSize() == 3);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
            REQUIRE(non_pod_arr_copy.GetCapacity() == 4);
            REQUIRE(non_pod_arr_copy.GetSize() == 3);
            REQUIRE(non_pod_arr_copy.GetData() != nullptr);
            REQUIRE(*non_pod_arr_copy.GetData()[0].ptr == 42);
            REQUIRE(*non_pod_arr_copy.GetData()[1].ptr == 42);
            REQUIRE(*non_pod_arr_copy.GetData()[2].ptr == 42);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 6);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 7);
    }
    SECTION("Move constructor")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            Opal::DynamicArray<NonPod> non_pod_arr_copy(std::move(non_pod_arr));
            REQUIRE(non_pod_arr.GetCapacity() == 0);
            REQUIRE(non_pod_arr.GetSize() == 0);
            REQUIRE(non_pod_arr.GetData() == nullptr);
            REQUIRE(non_pod_arr_copy.GetCapacity() == 4);
            REQUIRE(non_pod_arr_copy.GetSize() == 3);
            REQUIRE(non_pod_arr_copy.GetData() != nullptr);
            REQUIRE(*non_pod_arr_copy.GetData()[0].ptr == 42);
            REQUIRE(*non_pod_arr_copy.GetData()[1].ptr == 42);
            REQUIRE(*non_pod_arr_copy.GetData()[2].ptr == 42);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Copy assignment", "[DynamicArray]")
{
    SECTION("POD type")
    {
        SECTION("Copy default array")
        {
            Opal::DynamicArray<i32> int_arr;
            Opal::DynamicArray<i32> int_arr_copy;
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 4);
            REQUIRE(int_arr_copy.GetSize() == 0);
            REQUIRE(int_arr_copy.GetData() != nullptr);
        }
        SECTION("Receiver array has less allocated memory")
        {
            Opal::DynamicArray<i32> int_arr(5, 25);
            Opal::DynamicArray<i32> int_arr_copy(3, 42);
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 25);
            REQUIRE(int_arr.GetData()[1] == 25);
            REQUIRE(int_arr.GetData()[2] == 25);
            REQUIRE(int_arr.GetData()[3] == 25);
            REQUIRE(int_arr.GetData()[4] == 25);
            REQUIRE(int_arr_copy.GetCapacity() == 5);
            REQUIRE(int_arr_copy.GetSize() == 5);
            REQUIRE(int_arr_copy.GetData() != nullptr);
            REQUIRE(int_arr_copy.GetData()[0] == 25);
            REQUIRE(int_arr_copy.GetData()[1] == 25);
            REQUIRE(int_arr_copy.GetData()[2] == 25);
            REQUIRE(int_arr_copy.GetData()[3] == 25);
            REQUIRE(int_arr_copy.GetData()[4] == 25);
        }
        SECTION("Receiver array has more allocated memory")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            Opal::DynamicArray<i32> int_arr_copy(5, 25);
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
            REQUIRE(int_arr_copy.GetCapacity() == 5);
            REQUIRE(int_arr_copy.GetSize() == 3);
            REQUIRE(int_arr_copy.GetData() != nullptr);
            REQUIRE(int_arr_copy.GetData()[0] == 42);
            REQUIRE(int_arr_copy.GetData()[1] == 42);
            REQUIRE(int_arr_copy.GetData()[2] == 42);
        }
    }
    SECTION("Non-POD type")
    {
        SECTION("Copy default array")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr;
                Opal::DynamicArray<NonPod> non_pod_arr_copy;
                non_pod_arr_copy = non_pod_arr;
                REQUIRE(non_pod_arr.GetCapacity() == 4);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 4);
                REQUIRE(non_pod_arr_copy.GetSize() == 0);
                REQUIRE(non_pod_arr_copy.GetData() != nullptr);
                REQUIRE(g_value_call_count == 0);
                REQUIRE(g_copy_call_count == 0);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 0);
        }
        SECTION("Receiver array has less allocated memory")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
                Opal::DynamicArray<NonPod> non_pod_arr_copy(3, NonPod(24));
                non_pod_arr_copy = non_pod_arr;
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 5);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[3].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[4].ptr == 42);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 5);
                REQUIRE(non_pod_arr_copy.GetSize() == 5);
                REQUIRE(non_pod_arr_copy.GetData() != nullptr);
                REQUIRE(*non_pod_arr_copy.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[2].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[3].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[4].ptr == 42);
                REQUIRE(g_value_call_count == 2);
                REQUIRE(g_copy_call_count == 13);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 15);
        }
        SECTION("Receiver has more allocated memory")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                Opal::DynamicArray<NonPod> non_pod_arr_copy(5, NonPod(24));
                non_pod_arr_copy = non_pod_arr;
                REQUIRE(non_pod_arr.GetCapacity() == 4);
                REQUIRE(non_pod_arr.GetSize() == 3);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 5);
                REQUIRE(non_pod_arr_copy.GetSize() == 3);
                REQUIRE(non_pod_arr_copy.GetData() != nullptr);
                REQUIRE(*non_pod_arr_copy.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[2].ptr == 42);
                REQUIRE(g_value_call_count == 2);
                REQUIRE(g_copy_call_count == 11);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 13);
        }
    }
}

TEST_CASE("Move assignment", "[DynamicArray]")
{
    SECTION("Pod type")
    {
        SECTION("Move empty array")
        {
            Opal::DynamicArray<i32> int_arr;
            Opal::DynamicArray<i32> int_arr_copy;
            int_arr_copy = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 4);
            REQUIRE(int_arr_copy.GetSize() == 0);
            REQUIRE(int_arr_copy.GetData() != nullptr);
        }
        SECTION("Move non-empty array")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            Opal::DynamicArray<i32> int_arr_copy(5, 25);
            int_arr_copy = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 4);
            REQUIRE(int_arr_copy.GetSize() == 3);
            REQUIRE(int_arr_copy.GetData() != nullptr);
            REQUIRE(int_arr_copy.GetData()[0] == 42);
            REQUIRE(int_arr_copy.GetData()[1] == 42);
            REQUIRE(int_arr_copy.GetData()[2] == 42);
        }
    }
    SECTION("Non-pod type")
    {
        SECTION("Move empty array")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr;
                Opal::DynamicArray<NonPod> non_pod_arr_copy;
                non_pod_arr_copy = std::move(non_pod_arr);
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 4);
                REQUIRE(non_pod_arr_copy.GetSize() == 0);
                REQUIRE(non_pod_arr_copy.GetData() != nullptr);
                REQUIRE(g_value_call_count == 0);
                REQUIRE(g_copy_call_count == 0);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 0);
        }
        SECTION("Move non-empty array")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                Opal::DynamicArray<NonPod> non_pod_arr_copy(5, NonPod(24));
                non_pod_arr_copy = std::move(non_pod_arr);
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 4);
                REQUIRE(non_pod_arr_copy.GetSize() == 3);
                REQUIRE(non_pod_arr_copy.GetData() != nullptr);
                REQUIRE(*non_pod_arr_copy.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr_copy.GetData()[2].ptr == 42);
                REQUIRE(g_value_call_count == 2);
                REQUIRE(g_copy_call_count == 8);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 10);
        }
    }
}

TEST_CASE("Assign with POD data", "[DynamicArray]")
{
    SECTION("Assign count less then current size")
    {
        Opal::DynamicArray<i32> int_arr(5, 25);
        int_arr.Assign(3, 42);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Assign count larger then current size")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr.Assign(5, 25);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 25);
        REQUIRE(int_arr.GetData()[1] == 25);
        REQUIRE(int_arr.GetData()[2] == 25);
        REQUIRE(int_arr.GetData()[3] == 25);
        REQUIRE(int_arr.GetData()[4] == 25);
    }
}

TEST_CASE("Assign with non-POD data", "[DynamicArray]")
{
    SECTION("Assign count less then current size")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
            non_pod_arr.Assign(3, NonPod(24));
            REQUIRE(non_pod_arr.GetCapacity() == 5);
            REQUIRE(non_pod_arr.GetSize() == 3);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(*non_pod_arr.GetData()[0].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[1].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[2].ptr == 24);
            REQUIRE(g_value_call_count == 2);
            REQUIRE(g_copy_call_count == 8);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 10);
    }
    SECTION("Assign count larger then current size")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            non_pod_arr.Assign(5, NonPod(24));
            REQUIRE(non_pod_arr.GetCapacity() == 5);
            REQUIRE(non_pod_arr.GetSize() == 5);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(*non_pod_arr.GetData()[0].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[1].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[2].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[3].ptr == 24);
            REQUIRE(*non_pod_arr.GetData()[4].ptr == 24);
            REQUIRE(g_value_call_count == 2);
            REQUIRE(g_copy_call_count == 8);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 10);
    }
}

TEST_CASE("Access element with At", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.At(0).GetValue() == 42);
        REQUIRE(int_arr.At(1).GetValue() == 42);
        REQUIRE(int_arr.At(2).GetValue() == 42);
        REQUIRE(int_arr.At(3).HasValue() == false);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            REQUIRE(*non_pod_arr.At(0).GetValue().ptr == 42);
            REQUIRE(*non_pod_arr.At(1).GetValue().ptr == 42);
            REQUIRE(*non_pod_arr.At(2).GetValue().ptr == 42);
            REQUIRE(non_pod_arr.At(3).HasValue() == false);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Change element using At access", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr.At(0).GetValue() = 24;
        int_arr.At(1).GetValue() = 25;
        int_arr.At(2).GetValue() = 26;
        REQUIRE(int_arr.At(0).GetValue() == 24);
        REQUIRE(int_arr.At(1).GetValue() == 25);
        REQUIRE(int_arr.At(2).GetValue() == 26);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr.At(0).GetValue().ptr = 24;
            *non_pod_arr.At(1).GetValue().ptr = 25;
            *non_pod_arr.At(2).GetValue().ptr = 26;
            REQUIRE(*non_pod_arr.At(0).GetValue().ptr == 24);
            REQUIRE(*non_pod_arr.At(1).GetValue().ptr == 25);
            REQUIRE(*non_pod_arr.At(2).GetValue().ptr == 26);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Access element with operator[]", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr[0] == 42);
        REQUIRE(int_arr[1] == 42);
        REQUIRE(int_arr[2] == 42);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            REQUIRE(*non_pod_arr[0].ptr == 42);
            REQUIRE(*non_pod_arr[1].ptr == 42);
            REQUIRE(*non_pod_arr[2].ptr == 42);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Change value using operator[] access", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr[0] = 24;
        int_arr[1] = 25;
        int_arr[2] = 26;
        REQUIRE(int_arr[0] == 24);
        REQUIRE(int_arr[1] == 25);
        REQUIRE(int_arr[2] == 26);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[0].ptr = 24;
            *non_pod_arr[1].ptr = 25;
            *non_pod_arr[2].ptr = 26;
            REQUIRE(*non_pod_arr[0].ptr == 24);
            REQUIRE(*non_pod_arr[1].ptr == 25);
            REQUIRE(*non_pod_arr[2].ptr == 26);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Access element with Front", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr[0] = 25;
        REQUIRE(int_arr.Front().GetValue() == 25);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[0].ptr = 25;
            REQUIRE(*non_pod_arr.Front().GetValue().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Access element with Back", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr[2] = 25;
        REQUIRE(int_arr.Back().GetValue() == 25);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[2].ptr = 25;
            REQUIRE(*non_pod_arr.Back().GetValue().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Is empty", "[DynamicArray]")
{
    SECTION("Empty array")
    {
        Opal::DynamicArray<i32> int_arr;
        REQUIRE(int_arr.IsEmpty() == true);
    }
    SECTION("Non-empty array")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.IsEmpty() == false);
    }
}

TEST_CASE("Reserve", "[DynamicArray]")
{
    SECTION("POD data")
    {
        SECTION("Less then current capacity")
        {
            Opal::DynamicArray<i32> int_arr(5, 25);
            int_arr.Reserve(3);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 25);
            REQUIRE(int_arr.GetData()[1] == 25);
            REQUIRE(int_arr.GetData()[2] == 25);
            REQUIRE(int_arr.GetData()[3] == 25);
            REQUIRE(int_arr.GetData()[4] == 25);
        }
        SECTION("More then current capacity")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            int_arr.Reserve(5);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
    }
    SECTION("Non-POD data")
    {
        SECTION("Less then current capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
                non_pod_arr.Reserve(3);
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 5);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[3].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[4].ptr == 42);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 5);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 6);
        }
        SECTION("More then current capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.Reserve(5);
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 3);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 3);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 4);
        }
    }
}

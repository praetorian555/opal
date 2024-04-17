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

    bool operator==(const NonPod& other) const { return *ptr == *other.ptr; }
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
    SECTION("Allocator default constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(allocator);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Allocator move default constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(Move(allocator));
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Allocator size constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(3, allocator);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
    }
    SECTION("Allocator move size constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(3, Move(allocator));
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
    }
    SECTION("Allocator size and default value constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(3, 42, allocator);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Allocator move size and default value constructor")
    {
        Opal::DefaultAllocator allocator;
        Opal::DynamicArray<i32> int_arr(3, 42, Move(allocator));
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
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
        SECTION("Copy into itself")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            int_arr = int_arr;
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
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
        SECTION("Move into itself")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            int_arr = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
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

TEST_CASE("Compare", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr1(3, 42);
        Opal::DynamicArray<i32> int_arr2(3, 42);
        Opal::DynamicArray<i32> int_arr3(3, 24);
        Opal::DynamicArray<i32> int_arr4(2, 42);
        REQUIRE(int_arr1 == int_arr2);
        REQUIRE(int_arr1 != int_arr3);
        REQUIRE(int_arr1 != int_arr4);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr1(3, NonPod(42));
            Opal::DynamicArray<NonPod> non_pod_arr2(3, NonPod(42));
            Opal::DynamicArray<NonPod> non_pod_arr3(3, NonPod(24));
            Opal::DynamicArray<NonPod> non_pod_arr4(2, NonPod(42));
            REQUIRE(non_pod_arr1 == non_pod_arr2);
            REQUIRE(non_pod_arr1 != non_pod_arr3);
            REQUIRE(non_pod_arr1 != non_pod_arr4);
            REQUIRE(g_value_call_count == 4);
            REQUIRE(g_copy_call_count == 11);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 15);
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
    SECTION("Assign 0 elements")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr.Assign(0, 25);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Assign with iterators")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        Opal::ErrorCode err = int_arr.AssignIt(values.begin(), values.end());
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 25);
        REQUIRE(int_arr.GetData()[1] == 26);
        REQUIRE(int_arr.GetData()[2] == 27);
        REQUIRE(int_arr.GetData()[3] == 28);
        REQUIRE(int_arr.GetData()[4] == 29);
    }
    SECTION("Assign with bad iterators")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        Opal::ErrorCode err = int_arr.AssignIt(values.end(), values.begin());
        REQUIRE(err == Opal::ErrorCode::BadInput);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Assign with equal iterators")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        Opal::ErrorCode err = int_arr.AssignIt(values.begin(), values.begin());
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
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

TEST_CASE("Resize", "[DynamicArray]")
{
    SECTION("POD data")
    {
        SECTION("To new size which is same as old size")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            int_arr.Resize(3);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("To new size which is less then old size")
        {
            Opal::DynamicArray<i32> int_arr(5, 25);
            int_arr.Resize(3);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 25);
            REQUIRE(int_arr.GetData()[1] == 25);
            REQUIRE(int_arr.GetData()[2] == 25);
        }
        SECTION("To new size which is greater then old size and smaller then capacity")
        {
            Opal::DynamicArray<i32> int_arr(3, 5);
            int_arr.Reserve(5);
            int_arr.Resize(4);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 5);
            REQUIRE(int_arr.GetData()[1] == 5);
            REQUIRE(int_arr.GetData()[2] == 5);
            REQUIRE(int_arr.GetData()[3] == 0);
        }
        SECTION("To new size which is greater then capacity")
        {
            Opal::DynamicArray<i32> int_arr(3, 5);
            int_arr.Resize(6);
            REQUIRE(int_arr.GetCapacity() == 6);
            REQUIRE(int_arr.GetSize() == 6);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 5);
            REQUIRE(int_arr.GetData()[1] == 5);
            REQUIRE(int_arr.GetData()[2] == 5);
            REQUIRE(int_arr.GetData()[3] == 0);
            REQUIRE(int_arr.GetData()[4] == 0);
            REQUIRE(int_arr.GetData()[5] == 0);
        }
    }
    SECTION("Non-POD data")
    {
        SECTION("To new size which is same as old size")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.Resize(3);
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
            REQUIRE(g_destroy_call_count == 5);
        }
        SECTION("To new size which is less then old size")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
                non_pod_arr.Resize(3);
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 3);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 5);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 7);
        }
        SECTION("To new size which is greater then old size and smaller then capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.Reserve(5);
                non_pod_arr.Resize(4);
                REQUIRE(non_pod_arr.GetCapacity() == 5);
                REQUIRE(non_pod_arr.GetSize() == 4);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[3].ptr == 5);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 4);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 6);
        }
        SECTION("To new size which is greater then capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.Resize(6);
                REQUIRE(non_pod_arr.GetCapacity() == 6);
                REQUIRE(non_pod_arr.GetSize() == 6);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
                REQUIRE(*non_pod_arr.GetData()[3].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[4].ptr == 5);
                REQUIRE(*non_pod_arr.GetData()[5].ptr == 5);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 6);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 8);
        }
    }
}

TEST_CASE("Clear", "[DynamicArray]")
{
    SECTION("POD data")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        int_arr.Clear();
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            non_pod_arr.Clear();
            REQUIRE(non_pod_arr.GetCapacity() == 4);
            REQUIRE(non_pod_arr.GetSize() == 0);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Push back", "[DynamicArray]")
{
    SECTION("POD data")
    {
        SECTION("with copy")
        {
            SECTION("with enough capacity")
            {
                Opal::DynamicArray<i32> int_arr(3, 42);
                const i32 val = 25;
                int_arr.PushBack(val);
                REQUIRE(int_arr.GetCapacity() == 4);
                REQUIRE(int_arr.GetSize() == 4);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 25);
            }
            SECTION("without enough capacity")
            {
                Opal::DynamicArray<i32> int_arr(4, 42);
                const i32 val = 25;
                int_arr.PushBack(val);
                REQUIRE(int_arr.GetCapacity() == 7);
                REQUIRE(int_arr.GetSize() == 5);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 42);
                REQUIRE(int_arr[4] == 25);
            }
        }
        SECTION("With move")
        {
            SECTION("With enough capacity")
            {
                Opal::DynamicArray<i32> int_arr(3, 42);
                int_arr.PushBack(25);
                REQUIRE(int_arr.GetCapacity() == 4);
                REQUIRE(int_arr.GetSize() == 4);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 25);
            }
            SECTION("Without enough capacity")
            {
                Opal::DynamicArray<i32> int_arr(4, 42);
                int_arr.PushBack(25);
                REQUIRE(int_arr.GetCapacity() == 7);
                REQUIRE(int_arr.GetSize() == 5);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 42);
                REQUIRE(int_arr[4] == 25);
            }
        }
    }
    SECTION("Non-POD data")
    {
        SECTION("With copy")
        {
            SECTION("With enough capacity")
            {
                g_value_call_count = 0;
                g_copy_call_count = 0;
                g_copy_assign_call_count = 0;
                g_destroy_call_count = 0;
                {
                    Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                    const NonPod val(25);
                    non_pod_arr.PushBack(val);
                    REQUIRE(non_pod_arr.GetCapacity() == 4);
                    REQUIRE(non_pod_arr.GetSize() == 4);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_copy_call_count == 4);
                    REQUIRE(g_copy_assign_call_count == 0);
                }
                REQUIRE(g_destroy_call_count == 6);
            }
            SECTION("Without enough capacity")
            {
                g_value_call_count = 0;
                g_copy_call_count = 0;
                g_copy_assign_call_count = 0;
                g_destroy_call_count = 0;
                {
                    Opal::DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
                    const NonPod val(25);
                    non_pod_arr.PushBack(val);
                    REQUIRE(non_pod_arr.GetCapacity() == 7);
                    REQUIRE(non_pod_arr.GetSize() == 5);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 42);
                    REQUIRE(*non_pod_arr[4].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_copy_call_count == 5);
                    REQUIRE(g_copy_assign_call_count == 0);
                }
                REQUIRE(g_destroy_call_count == 7);
            }
        }
        SECTION("With move")
        {
            SECTION("With enough capacity")
            {
                g_value_call_count = 0;
                g_copy_call_count = 0;
                g_copy_assign_call_count = 0;
                g_destroy_call_count = 0;
                {
                    Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                    non_pod_arr.PushBack(NonPod(25));
                    REQUIRE(non_pod_arr.GetCapacity() == 4);
                    REQUIRE(non_pod_arr.GetSize() == 4);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_copy_call_count == 3);
                    REQUIRE(g_copy_assign_call_count == 0);
                }
                REQUIRE(g_destroy_call_count == 6);
            }
            SECTION("Without enough capacity")
            {
                g_value_call_count = 0;
                g_copy_call_count = 0;
                g_copy_assign_call_count = 0;
                g_destroy_call_count = 0;
                {
                    Opal::DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
                    non_pod_arr.PushBack(NonPod(25));
                    REQUIRE(non_pod_arr.GetCapacity() == 7);
                    REQUIRE(non_pod_arr.GetSize() == 5);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 42);
                    REQUIRE(*non_pod_arr[4].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_copy_call_count == 4);
                    REQUIRE(g_copy_assign_call_count == 0);
                }
                REQUIRE(g_destroy_call_count == 7);
            }
        }
    }
}

TEST_CASE("Pop back", "[DynamicArray]")
{
    SECTION("POD data")
    {
        SECTION("Empty array")
        {
            Opal::DynamicArray<i32> int_arr;
            int_arr.PopBack();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() != nullptr);
        }
        SECTION("Non-empty array")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            int_arr.PopBack();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
    }
    SECTION("Non-POD data")
    {
        SECTION("Empty array")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr;
                non_pod_arr.PopBack();
                REQUIRE(non_pod_arr.GetCapacity() == 4);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(g_value_call_count == 0);
                REQUIRE(g_copy_call_count == 0);
                REQUIRE(g_copy_assign_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 0);
        }
        SECTION("Non-empty array")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.PopBack();
                REQUIRE(non_pod_arr.GetCapacity() == 4);
                REQUIRE(non_pod_arr.GetSize() == 2);
                REQUIRE(non_pod_arr.GetData() != nullptr);
                REQUIRE(*non_pod_arr[0].ptr == 42);
                REQUIRE(*non_pod_arr[1].ptr == 42);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 3);
                REQUIRE(g_copy_assign_call_count == 0);
                REQUIRE(g_destroy_call_count == 2);
            }
            REQUIRE(g_destroy_call_count == 4);
        }
    }
}

TEST_CASE("Iterator", "[DynamicArray]")
{
    SECTION("Difference")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it1 = int_arr.Begin();
        Opal::DynamicArray<i32>::IteratorType it2 = int_arr.End();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == int_arr.End());
    }
    SECTION("Post increment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Opal::DynamicArray<i32>::IteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.End());
    }
    SECTION("Decrement")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.End();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.Begin());
    }
    SECTION("Post decrement")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.End();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Opal::DynamicArray<i32>::IteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.Begin());
    }
    SECTION("Add")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.End());

        Opal::DynamicArray<i32>::IteratorType it2 = int_arr.Begin();
        REQUIRE((3 + it2) == int_arr.End());
    }
    SECTION("Add assignment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.End());
    }
    SECTION("Subtract")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.End();
        REQUIRE((it - 0) == int_arr.End());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.Begin());
    }
    SECTION("Subtract assignment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.End();
        REQUIRE((it -= 0) == int_arr.End());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.Begin());
    }
    SECTION("Access")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
        Opal::DynamicArray<NonPod>::IteratorType it = non_pod_arr.Begin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::IteratorType it1 = int_arr.Begin();
        Opal::DynamicArray<i32>::IteratorType it2 = int_arr.Begin();
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
        Opal::DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (Opal::DynamicArray<i32>::IteratorType it = int_arr.Begin(); it != int_arr.End(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (i32 val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Const iterator", "[DynamicArray]")
{
    SECTION("Difference")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it1 = int_arr.ConstBegin();
        Opal::DynamicArray<i32>::ConstIteratorType it2 = int_arr.ConstEnd();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == int_arr.ConstEnd());
    }
    SECTION("Post increment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Opal::DynamicArray<i32>::ConstIteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.ConstEnd());
    }
    SECTION("Decrement")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstEnd();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.ConstBegin());
    }
    SECTION("Post decrement")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstEnd();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Opal::DynamicArray<i32>::ConstIteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.ConstBegin());
    }
    SECTION("Add")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.ConstEnd());

        Opal::DynamicArray<i32>::ConstIteratorType it2 = int_arr.ConstBegin();
        REQUIRE((3 + it2) == int_arr.ConstEnd());
    }
    SECTION("Add assignment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.ConstEnd());
    }
    SECTION("Subtract")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstEnd();
        REQUIRE((it - 0) == int_arr.ConstEnd());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.ConstBegin());
    }
    SECTION("Subtract assignment")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstEnd();
        REQUIRE((it -= 0) == int_arr.ConstEnd());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.ConstBegin());
    }
    SECTION("Access")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Opal::DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
        Opal::DynamicArray<NonPod>::ConstIteratorType it = non_pod_arr.ConstBegin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        Opal::DynamicArray<i32> int_arr(3, 42);
        Opal::DynamicArray<i32>::ConstIteratorType it1 = int_arr.ConstBegin();
        Opal::DynamicArray<i32>::ConstIteratorType it2 = int_arr.ConstBegin();
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
        Opal::DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (Opal::DynamicArray<i32>::ConstIteratorType it = int_arr.ConstBegin(); it != int_arr.ConstEnd(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        const Opal::DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (const i32& val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Insert", "[DynamicArray]")
{
    SECTION("Insert one element")
    {
        SECTION("In mid")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
        }
        SECTION("In mid move")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, 25).GetValue();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
        }
        SECTION("At the end")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
        }
        SECTION("At the end move")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), 25).GetValue();
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
        }
        SECTION("Bad position")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, val).GetError();
            REQUIRE(err == Opal::ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad position move")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            Opal::ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, 25).GetError();
            REQUIRE(err == Opal::ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
    }
    SECTION("Insert multiple same elements")
    {
        SECTION("In mid")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 7);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 25);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("At the end")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 7);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
            REQUIRE(int_arr[4] == 25);
        }
        SECTION("At beginning")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::DynamicArray<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin(), 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 7);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 25);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Bad position")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, 2, val).GetError();
            REQUIRE(err == Opal::ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad count")
        {
            Opal::DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            Opal::ErrorCode err = int_arr.Insert(int_arr.ConstBegin(), 0, val).GetError();
            REQUIRE(err == Opal::ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 4);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
    }
}
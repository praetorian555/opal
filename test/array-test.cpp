#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/container/array.h"

using namespace Opal;

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

TEST_CASE("Construction with POD data", "[Array]")
{
    SECTION("Default constructor")
    {
        Array<i32> int_arr;
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Default constructor with allocator")
    {
        MallocAllocator allocator;
        Array<i32> int_arr(&allocator);
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Size constructor")
    {
        Array<i32> int_arr(5);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
        REQUIRE(int_arr.GetData()[3] == 0);
        REQUIRE(int_arr.GetData()[4] == 0);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Size constructor with allocator")
    {
        MallocAllocator allocator;
        Array<i32> int_arr(5, &allocator);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
        REQUIRE(int_arr.GetData()[3] == 0);
        REQUIRE(int_arr.GetData()[4] == 0);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Size and default value constructor")
    {
        Array<i32> int_arr(3, 42);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Size and default value constructor with allocator")
    {
        MallocAllocator allocator;
        Array<i32> int_arr(3, 42, &allocator);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Pointer and size")
    {
        i32 data[] = {42, 43, 44};
        Array<i32> int_arr(data, 3);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 43);
        REQUIRE(int_arr.GetData()[2] == 44);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Pointer, size and allocator constructor")
    {
        MallocAllocator allocator;
        i32 data[] = {42, 43, 44};
        Array<i32> int_arr(data, 3, &allocator);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 43);
        REQUIRE(int_arr.GetData()[2] == 44);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Copy constructor")
    {
        Array<i32> int_arr(3, 42);
        Array<i32> int_arr_copy(int_arr);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
        REQUIRE(int_arr_copy.GetCapacity() == 3);
        REQUIRE(int_arr_copy.GetSize() == 3);
        REQUIRE(int_arr_copy.GetData() != nullptr);
        REQUIRE(int_arr_copy.GetData()[0] == 42);
        REQUIRE(int_arr_copy.GetData()[1] == 42);
        REQUIRE(int_arr_copy.GetData()[2] == 42);
        REQUIRE(int_arr_copy.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Copy constructor with allocator")
    {
        MallocAllocator allocator;
        Array<i32> int_arr(3, 42);
        Array<i32> int_arr_copy(int_arr, &allocator);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
        REQUIRE(int_arr_copy.GetCapacity() == 3);
        REQUIRE(int_arr_copy.GetSize() == 3);
        REQUIRE(int_arr_copy.GetData() != nullptr);
        REQUIRE(int_arr_copy.GetData()[0] == 42);
        REQUIRE(int_arr_copy.GetData()[1] == 42);
        REQUIRE(int_arr_copy.GetData()[2] == 42);
        REQUIRE(int_arr_copy.GetAllocator() == &allocator);
    }
    SECTION("Move constructor")
    {
        MallocAllocator allocator;
        Array<i32> int_arr(3, 42, &allocator);
        Array<i32> int_arr_copy(Move(int_arr));
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr_copy.GetCapacity() == 3);
        REQUIRE(int_arr_copy.GetSize() == 3);
        REQUIRE(int_arr_copy.GetData() != nullptr);
        REQUIRE(int_arr_copy.GetData()[0] == 42);
        REQUIRE(int_arr_copy.GetData()[1] == 42);
        REQUIRE(int_arr_copy.GetData()[2] == 42);
        REQUIRE(int_arr_copy.GetAllocator() == &allocator);
    }
    SECTION("Initializer list")
    {
        Array<i32> int_arr{42, 43, 44};
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 43);
        REQUIRE(int_arr.GetData()[2] == 44);
    }
    SECTION("Initializer list one element")
    {
        Array<i32> int_arr{42};
        REQUIRE(int_arr.GetCapacity() == 1);
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
    }
    SECTION("Initializer list with allocator")
    {
        MallocAllocator allocator;
        Array<i32> int_arr{{42}, &allocator};
        REQUIRE(int_arr.GetCapacity() == 1);
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
}

TEST_CASE("Construction with non-POD data", "[Array]")
{
    SECTION("Size constructor")
    {
        SECTION("Initial size smaller then default capacity")
        {
            g_default_call_count = 0;
            g_destroy_call_count = 0;
            {
                Array<NonPod> non_pod_arr(3);
                REQUIRE(non_pod_arr.GetCapacity() == 3);
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
                Array<NonPod> non_pod_arr(5);
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
            Array<NonPod> non_pod_arr(3, default_value);
            REQUIRE(non_pod_arr.GetCapacity() == 3);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
            Array<NonPod> non_pod_arr_copy(non_pod_arr);
            REQUIRE(non_pod_arr.GetCapacity() == 3);
            REQUIRE(non_pod_arr.GetSize() == 3);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(*non_pod_arr.GetData()[0].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[1].ptr == 42);
            REQUIRE(*non_pod_arr.GetData()[2].ptr == 42);
            REQUIRE(non_pod_arr_copy.GetCapacity() == 3);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
            Array<NonPod> non_pod_arr_copy(std::move(non_pod_arr));
            REQUIRE(non_pod_arr.GetCapacity() == 0);
            REQUIRE(non_pod_arr.GetSize() == 0);
            REQUIRE(non_pod_arr.GetData() == nullptr);
            REQUIRE(non_pod_arr_copy.GetCapacity() == 3);
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

TEST_CASE("Copy assignment", "[Array]")
{
    SECTION("POD type")
    {
        SECTION("Copy into itself")
        {
            Array<i32> int_arr(3, 42);
            int_arr = int_arr;
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("Copy default array")
        {
            Array<i32> int_arr;
            Array<i32> int_arr_copy;
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 0);
            REQUIRE(int_arr_copy.GetSize() == 0);
            REQUIRE(int_arr_copy.GetData() == nullptr);
        }
        SECTION("Receiver array has less allocated memory")
        {
            Array<i32> int_arr(5, 25);
            Array<i32> int_arr_copy(3, 42);
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
            Array<i32> int_arr(3, 42);
            Array<i32> int_arr_copy(5, 25);
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 3);
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
        SECTION("Different allocators")
        {
            MallocAllocator allocator1;
            LinearAllocator allocator2(1024);
            Array<i32> int_arr(3, 42, &allocator1);
            Array<i32> int_arr_copy(5, 25, &allocator2);
            int_arr_copy = int_arr;
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
            REQUIRE(int_arr.GetAllocator() == &allocator1);
            REQUIRE(int_arr_copy.GetCapacity() == 3);
            REQUIRE(int_arr_copy.GetSize() == 3);
            REQUIRE(int_arr_copy.GetData() != nullptr);
            REQUIRE(int_arr_copy.GetData()[0] == 42);
            REQUIRE(int_arr_copy.GetData()[1] == 42);
            REQUIRE(int_arr_copy.GetData()[2] == 42);
            REQUIRE(int_arr_copy.GetAllocator() == &allocator1);
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
                Array<NonPod> non_pod_arr;
                Array<NonPod> non_pod_arr_copy;
                non_pod_arr_copy = non_pod_arr;
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 0);
                REQUIRE(non_pod_arr_copy.GetSize() == 0);
                REQUIRE(non_pod_arr_copy.GetData() == nullptr);
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
                Array<NonPod> non_pod_arr(5, NonPod(42));
                Array<NonPod> non_pod_arr_copy(3, NonPod(24));
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
                Array<NonPod> non_pod_arr_copy(5, NonPod(24));
                non_pod_arr_copy = non_pod_arr;
                REQUIRE(non_pod_arr.GetCapacity() == 3);
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

TEST_CASE("Move assignment", "[Array]")
{
    SECTION("Pod type")
    {
        SECTION("Move into itself")
        {
            Array<i32> int_arr(3, 42);
            int_arr = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("Move empty array")
        {
            Array<i32> int_arr;
            Array<i32> int_arr_copy;
            int_arr_copy = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 0);
            REQUIRE(int_arr_copy.GetSize() == 0);
            REQUIRE(int_arr_copy.GetData() == nullptr);
        }
        SECTION("Move non-empty array")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> int_arr_copy(5, 25);
            int_arr_copy = std::move(int_arr);
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
            REQUIRE(int_arr_copy.GetCapacity() == 3);
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
                Array<NonPod> non_pod_arr;
                Array<NonPod> non_pod_arr_copy;
                non_pod_arr_copy = std::move(non_pod_arr);
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 0);
                REQUIRE(non_pod_arr_copy.GetSize() == 0);
                REQUIRE(non_pod_arr_copy.GetData() == nullptr);
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
                Array<NonPod> non_pod_arr_copy(5, NonPod(24));
                non_pod_arr_copy = std::move(non_pod_arr);
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(non_pod_arr_copy.GetCapacity() == 3);
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

TEST_CASE("Compare", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr1(3, 42);
        Array<i32> int_arr2(3, 42);
        Array<i32> int_arr3(3, 24);
        Array<i32> int_arr4(2, 42);
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
            Array<NonPod> non_pod_arr1(3, NonPod(42));
            Array<NonPod> non_pod_arr2(3, NonPod(42));
            Array<NonPod> non_pod_arr3(3, NonPod(24));
            Array<NonPod> non_pod_arr4(2, NonPod(42));
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

TEST_CASE("Assign with POD data", "[Array]")
{
    SECTION("Assign count less then current size")
    {
        Array<i32> int_arr(5, 25);
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
        Array<i32> int_arr(3, 42);
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
        Array<i32> int_arr(3, 42);
        int_arr.Assign(0, 25);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Assign with iterators")
    {
        Array<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        ErrorCode err = int_arr.Assign(values.begin(), values.end());
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 25);
        REQUIRE(int_arr.GetData()[1] == 26);
        REQUIRE(int_arr.GetData()[2] == 27);
        REQUIRE(int_arr.GetData()[3] == 28);
        REQUIRE(int_arr.GetData()[4] == 29);
    }
    SECTION("Assign with Array iterators")
    {
        Array<i32> int_arr(3, 42);
        Array<i32> values(5, 25);
        ErrorCode err = int_arr.Assign(values.begin(), values.end());
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 25);
        REQUIRE(int_arr.GetData()[1] == 25);
        REQUIRE(int_arr.GetData()[2] == 25);
        REQUIRE(int_arr.GetData()[3] == 25);
        REQUIRE(int_arr.GetData()[4] == 25);
    }
    SECTION("Assign with bad iterators")
    {
        Array<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        ErrorCode err = int_arr.Assign(values.end(), values.begin());
        REQUIRE(err == ErrorCode::BadInput);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Assign with equal iterators")
    {
        Array<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        ErrorCode err = int_arr.Assign(values.begin(), values.begin());
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
}

TEST_CASE("Assign with non-POD data", "[Array]")
{
    SECTION("Assign count less then current size")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            Array<NonPod> non_pod_arr(5, NonPod(42));
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Access element with At", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
        REQUIRE(int_arr.At(0).GetValue() == 42);
        REQUIRE(int_arr.At(1).GetValue() == 42);
        REQUIRE(int_arr.At(2).GetValue() == 42);
        REQUIRE(int_arr.At(3).HasValue() == false);
    }
    SECTION("Const POD data")
    {
        const Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Change element using At access", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Access element with operator[]", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Change value using operator[] access", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Access element with Front", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[0].ptr = 25;
            REQUIRE(*non_pod_arr.Front().GetValue().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Access element with Back", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[2].ptr = 25;
            REQUIRE(*non_pod_arr.Back().GetValue().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Is empty", "[Array]")
{
    SECTION("Empty array")
    {
        Array<i32> int_arr;
        REQUIRE(int_arr.IsEmpty() == true);
    }
    SECTION("Non-empty array")
    {
        Array<i32> int_arr(3, 42);
        REQUIRE(int_arr.IsEmpty() == false);
    }
}

TEST_CASE("Reserve", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("Less then current capacity")
        {
            Array<i32> int_arr(5, 25);
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
            Array<i32> int_arr(3, 42);
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
                Array<NonPod> non_pod_arr(5, NonPod(42));
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Resize", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("To new size which is same as old size")
        {
            Array<i32> int_arr(3, 42);
            int_arr.Resize(3);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("To new size which is less then old size")
        {
            Array<i32> int_arr(5, 25);
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
            Array<i32> int_arr(3, 5);
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
            Array<i32> int_arr(3, 5);
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.Resize(3);
                REQUIRE(non_pod_arr.GetCapacity() == 3);
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
                Array<NonPod> non_pod_arr(5, NonPod(42));
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
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

TEST_CASE("Clear", "[Array]")
{
    SECTION("POD data")
    {
        Array<i32> int_arr(3, 42);
        int_arr.Clear();
        REQUIRE(int_arr.GetCapacity() == 3);
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
            Array<NonPod> non_pod_arr(3, NonPod(42));
            non_pod_arr.Clear();
            REQUIRE(non_pod_arr.GetCapacity() == 3);
            REQUIRE(non_pod_arr.GetSize() == 0);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_copy_call_count == 3);
            REQUIRE(g_copy_assign_call_count == 0);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

TEST_CASE("Push back", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("with copy")
        {
            SECTION("with enough capacity")
            {
                Array<i32> int_arr(3, 42);
                const i32 val = 25;
                int_arr.PushBack(val);
                REQUIRE(int_arr.GetCapacity() == 5);
                REQUIRE(int_arr.GetSize() == 4);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 25);
            }
            SECTION("without enough capacity")
            {
                Array<i32> int_arr(4, 42);
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
                Array<i32> int_arr(3, 42);
                int_arr.PushBack(25);
                REQUIRE(int_arr.GetCapacity() == 5);
                REQUIRE(int_arr.GetSize() == 4);
                REQUIRE(int_arr.GetData() != nullptr);
                REQUIRE(int_arr[0] == 42);
                REQUIRE(int_arr[1] == 42);
                REQUIRE(int_arr[2] == 42);
                REQUIRE(int_arr[3] == 25);
            }
            SECTION("Without enough capacity")
            {
                Array<i32> int_arr(4, 42);
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
                    Array<NonPod> non_pod_arr(3, NonPod(42));
                    const NonPod val(25);
                    non_pod_arr.PushBack(val);
                    REQUIRE(non_pod_arr.GetCapacity() == 5);
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
                    Array<NonPod> non_pod_arr(4, NonPod(42));
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
                    Array<NonPod> non_pod_arr(3, NonPod(42));
                    non_pod_arr.PushBack(NonPod(25));
                    REQUIRE(non_pod_arr.GetCapacity() == 5);
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
                    Array<NonPod> non_pod_arr(4, NonPod(42));
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

TEST_CASE("Pop back", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("Empty array")
        {
            Array<i32> int_arr;
            int_arr.PopBack();
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
        }
        SECTION("Non-empty array")
        {
            Array<i32> int_arr(3, 42);
            int_arr.PopBack();
            REQUIRE(int_arr.GetCapacity() == 3);
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
                Array<NonPod> non_pod_arr;
                non_pod_arr.PopBack();
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
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
                Array<NonPod> non_pod_arr(3, NonPod(42));
                non_pod_arr.PopBack();
                REQUIRE(non_pod_arr.GetCapacity() == 3);
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

TEST_CASE("Iterator", "[Array]")
{
    SECTION("Difference")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it1 = int_arr.Begin();
        Array<i32>::IteratorType it2 = int_arr.End();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
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
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Array<i32>::IteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.End());
    }
    SECTION("Decrement")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.End();
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
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.End();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Array<i32>::IteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.Begin());
    }
    SECTION("Add")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.End());

        Array<i32>::IteratorType it2 = int_arr.Begin();
        REQUIRE((3 + it2) == int_arr.End());
    }
    SECTION("Add assignment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.End());
    }
    SECTION("Subtract")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.End();
        REQUIRE((it - 0) == int_arr.End());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.Begin());
    }
    SECTION("Subtract assignment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.End();
        REQUIRE((it -= 0) == int_arr.End());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.Begin());
    }
    SECTION("Access")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it = int_arr.Begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Array<NonPod> non_pod_arr(3, NonPod(42));
        Array<NonPod>::IteratorType it = non_pod_arr.Begin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::IteratorType it1 = int_arr.Begin();
        Array<i32>::IteratorType it2 = int_arr.Begin();
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
        Array<i32> int_arr(3, 42);
        i32 sum = 0;
        for (Array<i32>::IteratorType it = int_arr.Begin(); it != int_arr.End(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        Array<i32> int_arr(3, 42);
        i32 sum = 0;
        for (i32 val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Const iterator", "[Array]")
{
    SECTION("Difference")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it1 = int_arr.ConstBegin();
        Array<i32>::ConstIteratorType it2 = int_arr.ConstEnd();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
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
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        Array<i32>::ConstIteratorType prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.ConstEnd());
    }
    SECTION("Decrement")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstEnd();
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
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstEnd();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        Array<i32>::ConstIteratorType prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.ConstBegin());
    }
    SECTION("Add")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.ConstEnd());

        Array<i32>::ConstIteratorType it2 = int_arr.ConstBegin();
        REQUIRE((3 + it2) == int_arr.ConstEnd());
    }
    SECTION("Add assignment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.ConstEnd());
    }
    SECTION("Subtract")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstEnd();
        REQUIRE((it - 0) == int_arr.ConstEnd());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.ConstBegin());
    }
    SECTION("Subtract assignment")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstEnd();
        REQUIRE((it -= 0) == int_arr.ConstEnd());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.ConstBegin());
    }
    SECTION("Access")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it = int_arr.ConstBegin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        Array<NonPod> non_pod_arr(3, NonPod(42));
        Array<NonPod>::ConstIteratorType it = non_pod_arr.ConstBegin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        Array<i32> int_arr(3, 42);
        Array<i32>::ConstIteratorType it1 = int_arr.ConstBegin();
        Array<i32>::ConstIteratorType it2 = int_arr.ConstBegin();
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
        Array<i32> int_arr(3, 42);
        i32 sum = 0;
        for (Array<i32>::ConstIteratorType it = int_arr.ConstBegin(); it != int_arr.ConstEnd(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        const Array<i32> int_arr(3, 42);
        i32 sum = 0;
        for (const i32& val : int_arr)
        {
            sum += val;
        }
        REQUIRE(sum == 126);
    }
}

TEST_CASE("Insert", "[Array]")
{
    SECTION("Insert one element")
    {
        SECTION("In mid")
        {
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
        }
        SECTION("In mid move")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, 25).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 25);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
        }
        SECTION("At the end")
        {
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
        }
        SECTION("At the end move")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), 25).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
        }
        SECTION("Bad position")
        {
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, val).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad position move")
        {
            Array<i32> int_arr(3, 42);
            ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, 25).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
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
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
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
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
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
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin(), 2, val).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
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
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, 2, val).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad count")
        {
            Array<i32> int_arr(3, 42);
            i32 val = 25;
            ErrorCode err = int_arr.Insert(int_arr.ConstBegin(), 0, val).GetError();
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
    }
    SECTION("Insert multiple with other iterator")
    {
        SECTION("In mid")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(2, 5);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, other.ConstBegin(), other.ConstEnd()).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 5);
            REQUIRE(int_arr[2] == 5);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("At end")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(2, 5);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), other.ConstBegin(), other.ConstEnd()).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 5);
            REQUIRE(int_arr[4] == 5);
        }
        SECTION("At end, a lot of elements")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(100, 5);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstEnd(), other.ConstBegin(), other.ConstEnd()).GetValue();
            REQUIRE(int_arr.GetCapacity() == 103);
            REQUIRE(int_arr.GetSize() == 103);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            for (i32 i = 0; i < 100; ++i)
            {
                REQUIRE(int_arr[3 + i] == 5);
            }
        }
        SECTION("At beginning")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(2, 5);
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin(), other.ConstBegin(), other.ConstEnd()).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 5);
            REQUIRE(int_arr[1] == 5);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Bad position")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(2, 5);
            ErrorCode err = int_arr.Insert(int_arr.ConstEnd() + 1, other.ConstBegin(), other.ConstEnd()).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad other iterator")
        {
            Array<i32> int_arr(3, 42);
            Array<i32> other(2, 5);
            ErrorCode err = int_arr.Insert(int_arr.ConstBegin(), other.ConstEnd(), other.ConstBegin()).GetError();
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Insert from C style array")
        {
            Array<i32> int_arr(3, 42);
            i32 other[] = {5, 5};
            Array<i32>::IteratorType it = int_arr.Insert(int_arr.ConstBegin() + 1, other, other + 2).GetValue();
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 5);
            REQUIRE(int_arr[2] == 5);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
    }
}

TEST_CASE("Erase", "[Array]")
{
    SECTION("Single element")
    {
        SECTION("from mid")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstBegin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstEnd() - 1).GetValue();
            REQUIRE(it - int_arr.Begin() == int_arr.ConstEnd() - int_arr.ConstBegin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstBegin()).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds")
        {
            Array<i32> int_arr(3, 42);
            ErrorCode err = int_arr.Erase(int_arr.ConstEnd()).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("from mid non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.Begin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.End() - 1).GetValue();
            REQUIRE(it - int_arr.Begin() == int_arr.End() - int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.Begin()).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds non-const")
        {
            Array<i32> int_arr(3, 42);
            ErrorCode err = int_arr.Erase(int_arr.End()).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
    }
    SECTION("Single element with swap")
    {
        SECTION("from mid")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.ConstBegin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.ConstEnd() - 1).GetValue();
            REQUIRE(it - int_arr.Begin() == int_arr.ConstEnd() - int_arr.ConstBegin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.ConstBegin()).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds")
        {
            Array<i32> int_arr(3, 42);
            ErrorCode err = int_arr.EraseWithSwap(int_arr.ConstEnd()).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("from mid non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.Begin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.End() - 1).GetValue();
            REQUIRE(it - int_arr.Begin() == int_arr.End() - int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning non-const")
        {
            Array<i32> int_arr(3, 42);
            Array<i32>::IteratorType it = int_arr.EraseWithSwap(int_arr.Begin()).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds non-const")
        {
            Array<i32> int_arr(3, 42);
            ErrorCode err = int_arr.EraseWithSwap(int_arr.End()).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
    }
    SECTION("Multiple elements")
    {
        SECTION("From mid")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstBegin() + 1, int_arr.ConstBegin() + 3).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("From end")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstEnd() - 3, int_arr.ConstEnd()).GetValue();
            REQUIRE(it == int_arr.End());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("From beginning")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstBegin(), int_arr.ConstBegin() + 2).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Out of bounds input")
        {
            Array<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.ConstBegin() + 1, int_arr.ConstEnd() + 1).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Bad input")
        {
            Array<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.ConstEnd(), int_arr.ConstBegin()).GetError();
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Empty range")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.ConstBegin() + 1, int_arr.ConstBegin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("From mid non-const")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.Begin() + 1, int_arr.Begin() + 3).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("From end non-const")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.End() - 3, int_arr.End()).GetValue();
            REQUIRE(it == int_arr.End());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("From beginning non-const")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.Begin(), int_arr.Begin() + 2).GetValue();
            REQUIRE(it == int_arr.Begin());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Out of bounds input non-const")
        {
            Array<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.Begin() + 1, int_arr.End() + 1).GetError();
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Bad input non-const")
        {
            Array<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.End(), int_arr.Begin()).GetError();
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
        SECTION("Empty range non-const")
        {
            Array<i32> int_arr(5, 42);
            Array<i32>::IteratorType it = int_arr.Erase(int_arr.Begin() + 1, int_arr.Begin() + 1).GetValue();
            REQUIRE(it - int_arr.Begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 42);
            REQUIRE(int_arr[4] == 42);
        }
    }
}

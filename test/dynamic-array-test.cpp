#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wnon-virtual-dtor")
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/container/dynamic-array.h"

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
        DynamicArray<i32> int_arr;
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr.GetAllocator() == Opal::GetDefaultAllocator());
    }
    SECTION("Default constructor with allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr(&allocator);
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Size constructor")
    {
        DynamicArray<i32> int_arr(5);
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
        DynamicArray<i32> int_arr(5, &allocator);
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
        DynamicArray<i32> int_arr(3, 42);
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
        DynamicArray<i32> int_arr(3, 42, &allocator);
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
        DynamicArray<i32> int_arr(data, 3);
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
        DynamicArray<i32> int_arr(data, 3, &allocator);
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
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32> int_arr_copy(int_arr);
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
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32> int_arr_copy(int_arr, &allocator);
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
        DynamicArray<i32> int_arr(3, 42, &allocator);
        DynamicArray<i32> int_arr_copy(Move(int_arr));
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
        DynamicArray<i32> int_arr{42, 43, 44};
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 43);
        REQUIRE(int_arr.GetData()[2] == 44);
    }
    SECTION("Initializer list one element")
    {
        DynamicArray<i32> int_arr{42};
        REQUIRE(int_arr.GetCapacity() == 1);
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
    }
    SECTION("Initializer list with allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr{{42}, &allocator};
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
                DynamicArray<NonPod> non_pod_arr(3);
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
                DynamicArray<NonPod> non_pod_arr(5);
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
            DynamicArray<NonPod> non_pod_arr(3, default_value);
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            DynamicArray<NonPod> non_pod_arr_copy(non_pod_arr);
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            DynamicArray<NonPod> non_pod_arr_copy(std::move(non_pod_arr));
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
            DynamicArray<i32> int_arr(3, 42);
            OPAL_START_DISABLE_WARNINGS
#if defined(OPAL_COMPILER_CLANG)
            OPAL_DISABLE_WARNING("-Wself-assign-overloaded")
#endif
            int_arr = int_arr;
            OPAL_END_DISABLE_WARNINGS
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("Copy default array")
        {
            DynamicArray<i32> int_arr;
            DynamicArray<i32> int_arr_copy;
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
            DynamicArray<i32> int_arr(5, 25);
            DynamicArray<i32> int_arr_copy(3, 42);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> int_arr_copy(5, 25);
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
            LinearAllocator allocator2("Linear allocator");
            DynamicArray<i32> int_arr(3, 42, &allocator1);
            DynamicArray<i32> int_arr_copy(5, 25, &allocator2);
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
                DynamicArray<NonPod> non_pod_arr;
                DynamicArray<NonPod> non_pod_arr_copy;
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
                DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
                DynamicArray<NonPod> non_pod_arr_copy(3, NonPod(24));
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                DynamicArray<NonPod> non_pod_arr_copy(5, NonPod(24));
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
            DynamicArray<i32> int_arr(3, 42);
            OPAL_START_DISABLE_WARNINGS
#if defined(OPAL_COMPILER_CLANG)
            OPAL_DISABLE_WARNING("-Wself-assign-overloaded")
            OPAL_DISABLE_WARNING("-Wself-move")
#elif defined(OPAL_COMPILER_GCC)
            OPAL_DISABLE_WARNING("-Wself-move")
#endif
            int_arr = std::move(int_arr);
            OPAL_END_DISABLE_WARNINGS
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr.GetData()[0] == 42);
            REQUIRE(int_arr.GetData()[1] == 42);
            REQUIRE(int_arr.GetData()[2] == 42);
        }
        SECTION("Move empty array")
        {
            DynamicArray<i32> int_arr;
            DynamicArray<i32> int_arr_copy;
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> int_arr_copy(5, 25);
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
                DynamicArray<NonPod> non_pod_arr;
                DynamicArray<NonPod> non_pod_arr_copy;
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                DynamicArray<NonPod> non_pod_arr_copy(5, NonPod(24));
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
        DynamicArray<i32> int_arr1(3, 42);
        DynamicArray<i32> int_arr2(3, 42);
        DynamicArray<i32> int_arr3(3, 24);
        DynamicArray<i32> int_arr4(2, 42);
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
            DynamicArray<NonPod> non_pod_arr1(3, NonPod(42));
            DynamicArray<NonPod> non_pod_arr2(3, NonPod(42));
            DynamicArray<NonPod> non_pod_arr3(3, NonPod(24));
            DynamicArray<NonPod> non_pod_arr4(2, NonPod(42));
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
        DynamicArray<i32> int_arr(5, 25);
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
        DynamicArray<i32> int_arr(3, 42);
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
        DynamicArray<i32> int_arr(3, 42);
        int_arr.Assign(0, 25);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() != nullptr);
    }
    SECTION("Assign with iterators")
    {
        DynamicArray<i32> int_arr(3, 42);
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
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32> values(5, 25);
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
        DynamicArray<i32> int_arr(3, 42);
        std::array<i32, 5> values = {25, 26, 27, 28, 29};
        ErrorCode err = int_arr.Assign(values.end(), values.begin());
        REQUIRE(err == ErrorCode::InvalidArgument);
        REQUIRE(int_arr.GetCapacity() == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Assign with equal iterators")
    {
        DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
        DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.At(0) == 42);
        REQUIRE(int_arr.At(1) == 42);
        REQUIRE(int_arr.At(2) == 42);
        REQUIRE_THROWS_AS(int_arr.At(3), OutOfBoundsException);
    }
    SECTION("Const POD data")
    {
        const DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.At(0) == 42);
        REQUIRE(int_arr.At(1) == 42);
        REQUIRE(int_arr.At(2) == 42);
        REQUIRE_THROWS_AS(int_arr.At(3), OutOfBoundsException);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            REQUIRE(*non_pod_arr.At(0).ptr == 42);
            REQUIRE(*non_pod_arr.At(1).ptr == 42);
            REQUIRE(*non_pod_arr.At(2).ptr == 42);
            REQUIRE_THROWS_AS(non_pod_arr.At(3), OutOfBoundsException);
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
        DynamicArray<i32> int_arr(3, 42);
        int_arr.At(0) = 24;
        int_arr.At(1) = 25;
        int_arr.At(2) = 26;
        REQUIRE(int_arr.At(0) == 24);
        REQUIRE(int_arr.At(1) == 25);
        REQUIRE(int_arr.At(2) == 26);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr.At(0).ptr = 24;
            *non_pod_arr.At(1).ptr = 25;
            *non_pod_arr.At(2).ptr = 26;
            REQUIRE(*non_pod_arr.At(0).ptr == 24);
            REQUIRE(*non_pod_arr.At(1).ptr == 25);
            REQUIRE(*non_pod_arr.At(2).ptr == 26);
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
        DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
        DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
        DynamicArray<i32> int_arr(3, 42);
        int_arr[0] = 25;
        REQUIRE(int_arr.Front() == 25);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[0].ptr = 25;
            REQUIRE(*non_pod_arr.Front().ptr == 25);
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
        DynamicArray<i32> int_arr(3, 42);
        int_arr[2] = 25;
        REQUIRE(int_arr.Back() == 25);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_copy_call_count = 0;
        g_copy_assign_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[2].ptr = 25;
            REQUIRE(*non_pod_arr.Back().ptr == 25);
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
        DynamicArray<i32> int_arr;
        REQUIRE(int_arr.IsEmpty() == true);
    }
    SECTION("Non-empty array")
    {
        DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.IsEmpty() == false);
    }
}

TEST_CASE("Reserve", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("Less then current capacity")
        {
            DynamicArray<i32> int_arr(5, 25);
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
            DynamicArray<i32> int_arr(3, 42);
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
                DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
            DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<i32> int_arr(5, 25);
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
            DynamicArray<i32> int_arr(3, 5);
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
            DynamicArray<i32> int_arr(3, 5);
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
                DynamicArray<NonPod> non_pod_arr(5, NonPod(42));
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
        DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
                DynamicArray<i32> int_arr(3, 42);
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
                DynamicArray<i32> int_arr(4, 42);
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
                DynamicArray<i32> int_arr(3, 42);
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
                DynamicArray<i32> int_arr(4, 42);
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
                    DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
                    DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
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
                    DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
                    DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
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

TEST_CASE("Emplace back", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("With enough capacity")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32& ref = int_arr.EmplaceBack(25);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr[3] == 25);
            REQUIRE(ref == 25);
        }
        SECTION("Without enough capacity")
        {
            DynamicArray<i32> int_arr(4, 42);
            i32& ref = int_arr.EmplaceBack(25);
            REQUIRE(int_arr.GetSize() == 5);
            REQUIRE(int_arr[4] == 25);
            REQUIRE(ref == 25);
        }
    }
    SECTION("Non-POD data")
    {
        SECTION("With enough capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                g_value_call_count = 0;
                g_copy_call_count = 0;
                NonPod& ref = non_pod_arr.EmplaceBack(25);
                REQUIRE(non_pod_arr.GetSize() == 4);
                REQUIRE(*non_pod_arr[3].ptr == 25);
                REQUIRE(*ref.ptr == 25);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 0);
            }
        }
        SECTION("Without enough capacity")
        {
            g_value_call_count = 0;
            g_copy_call_count = 0;
            g_copy_assign_call_count = 0;
            g_destroy_call_count = 0;
            {
                DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
                g_value_call_count = 0;
                g_copy_call_count = 0;
                NonPod& ref = non_pod_arr.EmplaceBack(25);
                REQUIRE(non_pod_arr.GetSize() == 5);
                REQUIRE(*non_pod_arr[4].ptr == 25);
                REQUIRE(*ref.ptr == 25);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_copy_call_count == 0);
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
            DynamicArray<i32> int_arr;
            int_arr.PopBack();
            REQUIRE(int_arr.GetCapacity() == 0);
            REQUIRE(int_arr.GetSize() == 0);
            REQUIRE(int_arr.GetData() == nullptr);
        }
        SECTION("Non-empty array")
        {
            DynamicArray<i32> int_arr(3, 42);
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
                DynamicArray<NonPod> non_pod_arr;
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
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
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
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it1 = int_arr.begin();
        DynamicArray<i32>::iterator it2 = int_arr.end();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == int_arr.end());
    }
    SECTION("Post increment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        DynamicArray<i32>::iterator prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.end());
    }
    SECTION("Decrement")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.end();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Post decrement")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.end();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        DynamicArray<i32>::iterator prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Add")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.end());

        DynamicArray<i32>::iterator it2 = int_arr.begin();
        REQUIRE((3 + it2) == int_arr.end());
    }
    SECTION("Add assignment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.end());
    }
    SECTION("Subtract")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.end();
        REQUIRE((it - 0) == int_arr.end());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.begin());
    }
    SECTION("Subtract assignment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.end();
        REQUIRE((it -= 0) == int_arr.end());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.begin());
    }
    SECTION("Access")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it = int_arr.begin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
        DynamicArray<NonPod>::iterator it = non_pod_arr.begin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::iterator it1 = int_arr.begin();
        DynamicArray<i32>::iterator it2 = int_arr.begin();
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
        DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (DynamicArray<i32>::iterator it = int_arr.begin(); it != int_arr.end(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        DynamicArray<i32> int_arr(3, 42);
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
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it1 = int_arr.cbegin();
        DynamicArray<i32>::const_iterator it2 = int_arr.cend();
        REQUIRE(it2 - it1 == 3);
    }
    SECTION("Increment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == int_arr.cend());
    }
    SECTION("Post increment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        it++;
        REQUIRE(*it == 42);
        DynamicArray<i32>::const_iterator prev = it++;
        REQUIRE(it - prev == 1);
        REQUIRE(it == int_arr.cend());
    }
    SECTION("Decrement")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cend();
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        --it;
        REQUIRE(*it == 42);
        REQUIRE(it == int_arr.cbegin());
    }
    SECTION("Post decrement")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cend();
        it--;
        REQUIRE(*it == 42);
        it--;
        REQUIRE(*it == 42);
        DynamicArray<i32>::const_iterator prev = it--;
        REQUIRE(prev - it == 1);
        REQUIRE(it == int_arr.cbegin());
    }
    SECTION("Add")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(*(it + 0) == 42);
        REQUIRE(*(it + 1) == 42);
        REQUIRE(*(it + 2) == 42);
        REQUIRE((it + 3) == int_arr.cend());

        DynamicArray<i32>::const_iterator it2 = int_arr.cbegin();
        REQUIRE((3 + it2) == int_arr.cend());
    }
    SECTION("Add assignment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(*(it += 0) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE(*(it += 1) == 42);
        REQUIRE((it += 1) == int_arr.cend());
    }
    SECTION("Subtract")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cend();
        REQUIRE((it - 0) == int_arr.cend());
        REQUIRE(*(it - 1) == 42);
        REQUIRE(*(it - 2) == 42);
        REQUIRE(*(it - 3) == 42);
        REQUIRE((it - 3) == int_arr.cbegin());
    }
    SECTION("Subtract assignment")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cend();
        REQUIRE((it -= 0) == int_arr.cend());
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(*(it -= 1) == 42);
        REQUIRE(it == int_arr.cbegin());
    }
    SECTION("Access")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(it[0] == 42);
        REQUIRE(it[1] == 42);
        REQUIRE(it[2] == 42);
    }
    SECTION("Dereference")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it = int_arr.cbegin();
        REQUIRE(*it == 42);
    }
    SECTION("Pointer")
    {
        DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
        DynamicArray<NonPod>::const_iterator it = non_pod_arr.cbegin();
        REQUIRE(*(it->ptr) == 42);
    }
    SECTION("Compare")
    {
        DynamicArray<i32> int_arr(3, 42);
        DynamicArray<i32>::const_iterator it1 = int_arr.cbegin();
        DynamicArray<i32>::const_iterator it2 = int_arr.cbegin();
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
        DynamicArray<i32> int_arr(3, 42);
        i32 sum = 0;
        for (DynamicArray<i32>::const_iterator it = int_arr.cbegin(); it != int_arr.cend(); ++it)
        {
            sum += *it;
        }
        REQUIRE(sum == 126);
    }
    SECTION("Modern for loop")
    {
        const DynamicArray<i32> int_arr(3, 42);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, val));
            REQUIRE(*it == val);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, 25));
            REQUIRE(*it == 25);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), val));
            REQUIRE(*it == val);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), 25));
            REQUIRE(*it == 25);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            REQUIRE_THROWS_AS(int_arr.Insert(int_arr.cend() + 1, val), OutOfBoundsException);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad position move")
        {
            DynamicArray<i32> int_arr(3, 42);
            REQUIRE_THROWS_AS(int_arr.Insert(int_arr.cend() + 1, 25), OutOfBoundsException);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, 2, val));
            REQUIRE(*it == 25);
            REQUIRE(*(it + 1) == 25);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), 2, val));
            REQUIRE(*it == 25);
            REQUIRE(*(it + 1) == 25);
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
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), 2, val));
            REQUIRE(*it == 25);
            REQUIRE(*(it + 1) == 25);
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
            DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            REQUIRE_THROWS_AS(int_arr.Insert(int_arr.cend() + 1, 2, val), OutOfBoundsException);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad count")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), 0, val));
            REQUIRE(it == int_arr.begin());
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
            DynamicArray<i32> int_arr(3, 42);
            const DynamicArray<i32> other(2, 5);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, other.cbegin(), other.cend()));
            REQUIRE(*it == 5);
            REQUIRE(*(it + 1) == 5);
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
            DynamicArray<i32> int_arr(3, 42);
            const DynamicArray<i32> other(2, 5);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), other.cbegin(), other.cend()));
            REQUIRE(*it == 5);
            REQUIRE(*(it + 1) == 5);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> other(100, 5);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), other.cbegin(), other.cend()));
            for (int i = 0; i < 100; ++i)
            {
                REQUIRE(*(it + i) == 5);
            }
            REQUIRE(int_arr.GetCapacity() == 103);
            REQUIRE(int_arr.GetSize() == 103);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            for (u32 i = 0; i < 100; ++i)
            {
                REQUIRE(int_arr[3 + i] == 5);
            }
        }
        SECTION("At beginning")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> other(2, 5);
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), other.cbegin(), other.cend()));
            REQUIRE(*it == 5);
            REQUIRE(*(it + 1) == 5);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> other(2, 5);
            REQUIRE_THROWS_AS(int_arr.Insert(int_arr.cend() + 1, other.cbegin(), other.cend()), OutOfBoundsException);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Bad other iterator")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32> other(2, 5);
            REQUIRE_THROWS_AS(int_arr.Insert(int_arr.cbegin(), other.cend(), other.cbegin()), InvalidArgumentException);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Insert from C style array")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32 other[] = {5, 5};
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, other, other + 2));
            REQUIRE(*it == 5);
            REQUIRE(*(it + 1) == 5);
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cbegin() + 1);
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cend() - 1);
            REQUIRE(it - int_arr.begin() == int_arr.cend() - int_arr.cbegin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cbegin());
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds")
        {
            DynamicArray<i32> int_arr(3, 42);
            auto it = int_arr.Erase(int_arr.cend());
            REQUIRE(it == int_arr.end());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("from mid non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.begin() + 1);
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.end() - 1);
            REQUIRE(it - int_arr.begin() == int_arr.end() - int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.begin());
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            auto it = int_arr.Erase(int_arr.end());
            REQUIRE(it == int_arr.end());
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
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.cbegin() + 1);
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.cend() - 1);
            REQUIRE(it - int_arr.begin() == int_arr.cend() - int_arr.cbegin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.cbegin());
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds")
        {
            DynamicArray<i32> int_arr(3, 42);
            int_arr.EraseWithSwap(int_arr.cend());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("from mid non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.begin() + 1);
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from end non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.end() - 1);
            REQUIRE(it - int_arr.begin() == int_arr.end() - int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("from beginning non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            DynamicArray<i32>::iterator it = int_arr.EraseWithSwap(int_arr.begin());
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 3);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("out of bounds non-const")
        {
            DynamicArray<i32> int_arr(3, 42);
            int_arr.EraseWithSwap(int_arr.end());
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
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cbegin() + 1, int_arr.cbegin() + 3).GetValue();
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("From end")
        {
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cend() - 3, int_arr.cend()).GetValue();
            REQUIRE(it == int_arr.end());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("From beginning")
        {
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cbegin(), int_arr.cbegin() + 2).GetValue();
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Out of bounds input")
        {
            DynamicArray<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.cbegin() + 1, int_arr.cend() + 1).GetError();
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
            DynamicArray<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.cend(), int_arr.cbegin()).GetError();
            REQUIRE(err == ErrorCode::InvalidArgument);
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
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.cbegin() + 1, int_arr.cbegin() + 1).GetValue();
            REQUIRE(it - int_arr.begin() == 1);
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
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.begin() + 1, int_arr.begin() + 3).GetValue();
            REQUIRE(it - int_arr.begin() == 1);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("From end non-const")
        {
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.end() - 3, int_arr.end()).GetValue();
            REQUIRE(it == int_arr.end());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 2);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
        }
        SECTION("From beginning non-const")
        {
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.begin(), int_arr.begin() + 2).GetValue();
            REQUIRE(it == int_arr.begin());
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 3);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
        }
        SECTION("Out of bounds input non-const")
        {
            DynamicArray<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.begin() + 1, int_arr.end() + 1).GetError();
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
            DynamicArray<i32> int_arr(5, 42);
            ErrorCode err = int_arr.Erase(int_arr.end(), int_arr.begin()).GetError();
            REQUIRE(err == ErrorCode::InvalidArgument);
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
            DynamicArray<i32> int_arr(5, 42);
            DynamicArray<i32>::iterator it = int_arr.Erase(int_arr.begin() + 1, int_arr.begin() + 1).GetValue();
            REQUIRE(it - int_arr.begin() == 1);
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

TEST_CASE("Remove", "[Array]")
{
    Opal::DynamicArray<i32> arr;
    arr.PushBack(5);
    arr.PushBack(10);
    arr.PushBack(20);
    SECTION("With swap")
    {
        SECTION("From start")
        {
            arr.Remove(5);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 10);
            REQUIRE(arr[1] == 20);
        }
        SECTION("From end")
        {
            arr.Remove(20);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 10);
        }
        SECTION("From middle")
        {
            arr.Remove(10);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 20);
        }
        SECTION("Element not in the array")
        {
            arr.Remove(15);
            REQUIRE(arr.GetSize() == 3);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 10);
            REQUIRE(arr[2] == 20);
        }
    }
    SECTION("Without swap")
    {
        SECTION("From start")
        {
            arr.RemoveWithSwap(5);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 20);
            REQUIRE(arr[1] == 10);
        }
        SECTION("From end")
        {
            arr.RemoveWithSwap(20);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 10);
        }
        SECTION("From middle")
        {
            arr.RemoveWithSwap(10);
            REQUIRE(arr.GetSize() == 2);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 20);
        }
        SECTION("Element not in the array")
        {
            arr.RemoveWithSwap(15);
            REQUIRE(arr.GetSize() == 3);
            REQUIRE(arr[0] == 5);
            REQUIRE(arr[1] == 10);
            REQUIRE(arr[2] == 20);
        }
    }
}

TEST_CASE("Append multiple elements", "[Array]")
{
    SECTION("Append with copy from std container")
    {
        std::array arr = {3, 5, 6};
        DynamicArray<i32> int_arr;
        int_arr.Append(arr);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 3);
        REQUIRE(int_arr[1] == 5);
        REQUIRE(int_arr[2] == 6);
        REQUIRE(!arr.empty());
    }
    SECTION("Append with move from std container")
    {
        std::array arr = {3, 5, 6};
        DynamicArray<i32> int_arr;
        int_arr.Append(std::move(arr));
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 3);
        REQUIRE(int_arr[1] == 5);
        REQUIRE(int_arr[2] == 6);
    }
}

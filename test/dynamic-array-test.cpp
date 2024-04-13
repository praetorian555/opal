#include "catch2/catch2.hpp"

#include "opal/container/dynamic-array.h"

namespace
{
int32_t g_default_call_count = 0;
int32_t g_value_call_count = 0;
int32_t g_copy_call_count = 0;
int32_t g_destroy_call_count = 0;
struct NonPod
{
    int32_t* ptr = nullptr;
    NonPod()
    {
        ptr = new int32_t(5);
        g_default_call_count++;
    }
    explicit NonPod(int32_t value)
    {
        ptr = new int32_t(value);
        g_default_call_count++;
    }
    NonPod(const NonPod& other)
    {
        ptr = new int32_t(*other.ptr);
        g_value_call_count++;
    }
    NonPod& operator=(const NonPod& other)
    {
        if (this != &other)
        {
            delete ptr;
            ptr = new int32_t(*other.ptr);
        }
        g_copy_call_count++;
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

TEST_CASE("Construction of DynamicArray with POD", "[DynamicArray]")
{
    SECTION("POD type, initial count smaller then default capacity")
    {
        Opal::DynamicArray<int32_t> int_arr(3);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
    }
    SECTION("Pointer type, initial count smaller then default capacity")
    {
        Opal::DynamicArray<int32_t*> ptr_arr(3);
        REQUIRE(ptr_arr.GetCapacity() == 4);
        REQUIRE(ptr_arr.GetSize() == 3);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == nullptr);
        REQUIRE(ptr_arr.GetData()[1] == nullptr);
        REQUIRE(ptr_arr.GetData()[2] == nullptr);
    }
    SECTION("POD type, initial count larger then default capacity")
    {
        Opal::DynamicArray<int32_t> int_arr(5);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr.GetSize() == 5);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 0);
        REQUIRE(int_arr.GetData()[1] == 0);
        REQUIRE(int_arr.GetData()[2] == 0);
        REQUIRE(int_arr.GetData()[3] == 0);
        REQUIRE(int_arr.GetData()[4] == 0);
    }
    SECTION("Pointer type, initial count larger then default capacity")
    {
        Opal::DynamicArray<int32_t*> ptr_arr(5);
        REQUIRE(ptr_arr.GetCapacity() == 5);
        REQUIRE(ptr_arr.GetSize() == 5);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == nullptr);
        REQUIRE(ptr_arr.GetData()[1] == nullptr);
        REQUIRE(ptr_arr.GetData()[2] == nullptr);
        REQUIRE(ptr_arr.GetData()[3] == nullptr);
        REQUIRE(ptr_arr.GetData()[4] == nullptr);
    }
    SECTION("POD type, initial count and default value")
    {
        Opal::DynamicArray<int32_t> int_arr(3, 42);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetData() != nullptr);
        REQUIRE(int_arr.GetData()[0] == 42);
        REQUIRE(int_arr.GetData()[1] == 42);
        REQUIRE(int_arr.GetData()[2] == 42);
    }
    SECTION("Pointer type, initial count and default value")
    {
        int32_t default_value = 42;
        Opal::DynamicArray<int32_t*> ptr_arr(3, &default_value);
        REQUIRE(ptr_arr.GetCapacity() == 4);
        REQUIRE(ptr_arr.GetSize() == 3);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == &default_value);
        REQUIRE(ptr_arr.GetData()[1] == &default_value);
        REQUIRE(ptr_arr.GetData()[2] == &default_value);
    }
    SECTION("POD copy")
    {
        Opal::DynamicArray<int32_t> int_arr(3, 42);
        Opal::DynamicArray<int32_t> int_arr_copy(int_arr);
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
    SECTION("Pointer copy")
    {
        int32_t default_value = 42;
        Opal::DynamicArray<int32_t*> ptr_arr(3, &default_value);
        Opal::DynamicArray<int32_t*> ptr_arr_copy(ptr_arr);
        REQUIRE(ptr_arr.GetCapacity() == 4);
        REQUIRE(ptr_arr.GetSize() == 3);
        REQUIRE(ptr_arr.GetData() != nullptr);
        REQUIRE(ptr_arr.GetData()[0] == &default_value);
        REQUIRE(ptr_arr.GetData()[1] == &default_value);
        REQUIRE(ptr_arr.GetData()[2] == &default_value);
        REQUIRE(ptr_arr_copy.GetCapacity() == 4);
        REQUIRE(ptr_arr_copy.GetSize() == 3);
        REQUIRE(ptr_arr_copy.GetData() != nullptr);
        REQUIRE(ptr_arr_copy.GetData()[0] == &default_value);
        REQUIRE(ptr_arr_copy.GetData()[1] == &default_value);
        REQUIRE(ptr_arr_copy.GetData()[2] == &default_value);
    }
    SECTION("POD move")
    {
        Opal::DynamicArray<int32_t> int_arr(3, 42);
        Opal::DynamicArray<int32_t> int_arr_copy(std::move(int_arr));
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
    SECTION("Pointer move")
    {
        int32_t default_value = 42;
        Opal::DynamicArray<int32_t*> ptr_arr(3, &default_value);
        Opal::DynamicArray<int32_t*> ptr_arr_copy(std::move(ptr_arr));
        REQUIRE(ptr_arr.GetCapacity() == 0);
        REQUIRE(ptr_arr.GetSize() == 0);
        REQUIRE(ptr_arr.GetData() == nullptr);
        REQUIRE(ptr_arr_copy.GetCapacity() == 4);
        REQUIRE(ptr_arr_copy.GetSize() == 3);
        REQUIRE(ptr_arr_copy.GetData() != nullptr);
        REQUIRE(ptr_arr_copy.GetData()[0] == &default_value);
        REQUIRE(ptr_arr_copy.GetData()[1] == &default_value);
        REQUIRE(ptr_arr_copy.GetData()[2] == &default_value);
    }
}

TEST_CASE("Construction of DynamicArray with non-POD", "[DynamicArray]")
{
    SECTION("Non-POD type, initial count smaller then default capacity")
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
    SECTION("Non-POD type, initial count larger then default capacity")
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
    SECTION("Non-POD type, initial count and default value")
    {
        g_value_call_count = 0;
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
            REQUIRE(g_value_call_count == 3);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
}

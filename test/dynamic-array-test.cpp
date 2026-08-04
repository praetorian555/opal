#include <algorithm>
#include <iterator>

#include "opal/container/string.h"
#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wnon-virtual-dtor")
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

#include "opal/clonable-base.h"
#include "opal/container/dynamic-array.h"

using namespace Opal;

namespace
{
i32 g_default_call_count = 0;
i32 g_value_call_count = 0;
i32 g_clone_call_count = 0;
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
    NonPod Clone(AllocatorBase* = nullptr) const
    {
        g_clone_call_count++;
        NonPod value;
        *value.ptr = *ptr;
        return value;
    }

    bool operator==(const NonPod& other) const { return *ptr == *other.ptr; }
};

// Owns a heap value like NonPod, but copyable, which is what the copying Insert overloads
// (count-of-a-value, and a range) need from their element type.
struct OwnedCopy
{
    i32* ptr = nullptr;
    explicit OwnedCopy(i32 value) : ptr(new i32(value)) {}
    OwnedCopy(const OwnedCopy& other) : ptr(other.ptr != nullptr ? new i32(*other.ptr) : nullptr) {}
    OwnedCopy& operator=(const OwnedCopy& other)
    {
        if (this != &other)
        {
            // A container may assign onto an element it just moved out of, so a null of our
            // own is a state to recover from rather than one to dereference.
            if (other.ptr == nullptr)
            {
                delete ptr;
                ptr = nullptr;
            }
            else if (ptr == nullptr)
            {
                ptr = new i32(*other.ptr);
            }
            else
            {
                *ptr = *other.ptr;
            }
        }
        return *this;
    }
    OwnedCopy(OwnedCopy&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    OwnedCopy& operator=(OwnedCopy&& other) noexcept
    {
        if (this != &other)
        {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    ~OwnedCopy() { delete ptr; }
};

// Owns a heap value and, like plenty of real move assignments, does not guard against
// self-assignment. Move-assigning one of these onto itself leaves it holding nothing.
struct SelfMoveUnsafe
{
    i32* ptr = nullptr;
    explicit SelfMoveUnsafe(i32 value) : ptr(new i32(value)) {}
    SelfMoveUnsafe(const SelfMoveUnsafe& other) : ptr(other.ptr != nullptr ? new i32(*other.ptr) : nullptr) {}
    SelfMoveUnsafe& operator=(const SelfMoveUnsafe& other)
    {
        if (this != &other)
        {
            delete ptr;
            ptr = other.ptr != nullptr ? new i32(*other.ptr) : nullptr;
        }
        return *this;
    }
    SelfMoveUnsafe(SelfMoveUnsafe&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    SelfMoveUnsafe& operator=(SelfMoveUnsafe&& other) noexcept
    {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }
    ~SelfMoveUnsafe() { delete ptr; }
};

// Every constructor adds to the live count and the destructor takes one off, so the count only
// returns to zero if each object built is also torn down.
// Non-POD, copyable so it can sit in an initializer_list, and clonable through the contract
// ClonableBase documents: a Clone that takes an allocator. The parameter has no default, so this
// only compiles where the array goes through Opal::Clone rather than calling Clone() itself.
struct ClonableCopy
{
    i32 value = 0;
    explicit ClonableCopy(i32 in_value) : value(in_value) {}
    ClonableCopy(const ClonableCopy& other) : value(other.value) {}
    ClonableCopy& operator=(const ClonableCopy& other)
    {
        value = other.value;
        return *this;
    }
    ~ClonableCopy() {}

    ClonableCopy Clone(AllocatorBase*) const { return ClonableCopy(value); }
};
static_assert(!IsPOD<ClonableCopy>);

// Reading through a const iterator must not hand back something writable, whether the array it
// came from is const or not.
static_assert(k_is_same_value<decltype(*DynamicArray<i32>().cbegin()), const i32&>);
static_assert(k_is_same_value<decltype(DynamicArray<i32>().cbegin().operator->()), const i32*>);
static_assert(k_is_same_value<decltype(*DynamicArray<i32>().begin()), i32&>);

// Passes the first `allowed` allocations through and reports failure for every one after that by
// returning null, which is how malloc reports it.
struct BudgetedAllocator final : public AllocatorBase
{
    explicit BudgetedAllocator(i32 allowed_count) : AllocatorBase("BudgetedAllocator"), allowed(allowed_count) {}

    void* Alloc(u64 size, u64 alignment) override
    {
        if (allowed <= 0)
        {
            return nullptr;
        }
        allowed--;
        return inner.Alloc(size, alignment);
    }
    void Free(void* ptr) override { inner.Free(ptr); }
    [[nodiscard]] bool IsThreadSafe() const override { return false; }

    MallocAllocator inner;
    i32 allowed = 0;
};

i32 g_live_count = 0;
struct CountedLive
{
    i32 value = 0;
    explicit CountedLive(i32 in_value) : value(in_value) { g_live_count++; }
    CountedLive(const CountedLive& other) : value(other.value) { g_live_count++; }
    CountedLive(CountedLive&& other) noexcept : value(other.value) { g_live_count++; }
    CountedLive& operator=(const CountedLive& other) = default;
    CountedLive& operator=(CountedLive&& other) noexcept = default;
    ~CountedLive() { g_live_count--; }
};

// POD, but three padding bytes sit between the members, so two objects can hold equal values in
// bytes that are not equal.
struct PaddedPod
{
    u8 tag;
    i32 value;

    bool operator==(const PaddedPod& other) const { return tag == other.tag && value == other.value; }
};
static_assert(IsPOD<PaddedPod>);
static_assert(sizeof(PaddedPod) > sizeof(u8) + sizeof(i32));
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
    SECTION("Empty initializer list")
    {
        DynamicArray<i32> int_arr(std::initializer_list<i32>{});
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
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
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 3);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
    SECTION("Move constructor")
    {
        g_value_call_count = 0;
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 3);
        }
        REQUIRE(g_destroy_call_count == 4);
    }
    SECTION("Initializer list")
    {
        DynamicArray<ClonableCopy> arr{ClonableCopy(1), ClonableCopy(2), ClonableCopy(3)};
        REQUIRE(arr.GetCapacity() == 3);
        REQUIRE(arr.GetSize() == 3);
        REQUIRE(arr[0].value == 1);
        REQUIRE(arr[1].value == 2);
        REQUIRE(arr[2].value == 3);
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
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 0);
        }
        SECTION("Move non-empty array")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 8);
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
    SECTION("POD data differing past the first element")
    {
        DynamicArray<i32> int_arr1{1, 2, 3};
        DynamicArray<i32> int_arr2{1, 2, 3};
        DynamicArray<i32> int_arr3{1, 2, 4};
        DynamicArray<i32> int_arr4{1, 9, 3};
        REQUIRE(int_arr1 == int_arr2);
        REQUIRE(int_arr1 != int_arr3);
        REQUIRE(int_arr1 != int_arr4);
    }
    SECTION("POD data with equal values in unequal padding bytes")
    {
        constexpr u64 k_count = 2;
        DynamicArray<PaddedPod> arr1(k_count, PaddedPod{});
        DynamicArray<PaddedPod> arr2(k_count, PaddedPod{});
        memset(arr1.GetData(), 0x00, k_count * sizeof(PaddedPod));
        memset(arr2.GetData(), 0xFF, k_count * sizeof(PaddedPod));
        for (u64 i = 0; i < k_count; i++)
        {
            arr1[i].tag = 7;
            arr1[i].value = 42;
            arr2[i].tag = 7;
            arr2[i].value = 42;
        }
        REQUIRE(arr1 == arr2);
    }
    SECTION("Floating point data where zero and negative zero are equal")
    {
        DynamicArray<f32> float_arr1{1.0f, 0.0f};
        DynamicArray<f32> float_arr2{1.0f, -0.0f};
        REQUIRE(float_arr1 == float_arr2);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 11);
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
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 8);
        }
        REQUIRE(g_destroy_call_count == 10);
    }
    SECTION("Assign count larger then current size")
    {
        g_value_call_count = 0;
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 8);
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
        REQUIRE(int_arr.TryAt(3).GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Const POD data")
    {
        const DynamicArray<i32> int_arr(3, 42);
        REQUIRE(int_arr.At(0) == 42);
        REQUIRE(int_arr.At(1) == 42);
        REQUIRE(int_arr.At(2) == 42);
        REQUIRE(int_arr.TryAt(3).GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("An empty array reports an out of range index rather than a wrapped upper bound")
    {
        DynamicArray<i32> int_arr;
        REQUIRE(int_arr.TryAt(0).GetError() == ErrorCode::OutOfBounds);
        const DynamicArray<i32> const_int_arr;
        REQUIRE(const_int_arr.TryAt(0).GetError() == ErrorCode::OutOfBounds);
        REQUIRE(int_arr.TryFront().GetError() == ErrorCode::OutOfBounds);
        REQUIRE(int_arr.TryBack().GetError() == ErrorCode::OutOfBounds);
    }
    SECTION("Non-POD data")
    {
        g_value_call_count = 0;
        g_clone_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            REQUIRE(*non_pod_arr.At(0).ptr == 42);
            REQUIRE(*non_pod_arr.At(1).ptr == 42);
            REQUIRE(*non_pod_arr.At(2).ptr == 42);
            REQUIRE(non_pod_arr.TryAt(3).GetError() == ErrorCode::OutOfBounds);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_clone_call_count == 3);
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
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 3);
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
        g_clone_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            REQUIRE(*non_pod_arr[0].ptr == 42);
            REQUIRE(*non_pod_arr[1].ptr == 42);
            REQUIRE(*non_pod_arr[2].ptr == 42);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_clone_call_count == 3);
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
        g_clone_call_count = 0;
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
            REQUIRE(g_clone_call_count == 3);
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
        g_clone_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[0].ptr = 25;
            REQUIRE(*non_pod_arr.Front().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_clone_call_count == 3);
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
        g_clone_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            *non_pod_arr[2].ptr = 25;
            REQUIRE(*non_pod_arr.Back().ptr == 25);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_clone_call_count == 3);
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

// The array clones instead of copying, and says so rather than letting the copy operations be
// deleted as a side effect of declaring the move ones.
static_assert(!CopyConstructable<DynamicArray<i32>>);
static_assert(!CopyAssignable<DynamicArray<i32>>);
static_assert(MoveConstructable<DynamicArray<i32>>);
static_assert(MoveAssignable<DynamicArray<i32>>);

// The iterators carry no iterator_category of their own. C++20 lets std::iterator_traits work that
// out from what they can do, which is why the standard algorithms accept them, but it means the
// guarantee rests on every one of those operations staying put: drop operator--, or make
// difference_type unsigned, and the category quietly degrades or disappears. These pin it.
static_assert(std::random_access_iterator<DynamicArray<i32>::iterator>);
static_assert(std::random_access_iterator<DynamicArray<i32>::const_iterator>);
static_assert(std::random_access_iterator<DynamicArray<i32>::reverse_iterator>);
static_assert(std::random_access_iterator<DynamicArray<i32>::const_reverse_iterator>);
static_assert(std::is_same_v<std::iterator_traits<DynamicArray<i32>::iterator>::iterator_category, std::random_access_iterator_tag>);
static_assert(
    std::is_same_v<std::iterator_traits<DynamicArray<i32>::const_iterator>::iterator_category, std::random_access_iterator_tag>);
static_assert(std::is_same_v<std::iterator_traits<DynamicArray<i32>::iterator>::value_type, i32>);
static_assert(std::is_same_v<std::iterator_traits<DynamicArray<i32>::iterator>::difference_type, i64>);

// A non-POD element type has to work the same way.
static_assert(std::random_access_iterator<DynamicArray<OwnedCopy>::iterator>);
static_assert(std::random_access_iterator<DynamicArray<OwnedCopy>::const_reverse_iterator>);

TEST_CASE("Standard algorithms over the iterators", "[Array]")
{
    SECTION("Sorting through the forward iterators")
    {
        DynamicArray<i32> int_arr{5, 1, 4, 2, 3};
        std::sort(int_arr.begin(), int_arr.end());
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[1] == 2);
        REQUIRE(int_arr[4] == 5);
    }
    SECTION("Sorting through the reverse iterators puts it the other way round")
    {
        DynamicArray<i32> int_arr{5, 1, 4, 2, 3};
        std::sort(int_arr.rbegin(), int_arr.rend());
        REQUIRE(int_arr[0] == 5);
        REQUIRE(int_arr[4] == 1);
    }
    SECTION("Reading algorithms take the const iterators")
    {
        const DynamicArray<i32> int_arr{5, 1, 4, 2, 3};
        REQUIRE(std::find(int_arr.cbegin(), int_arr.cend(), 4) == int_arr.cbegin() + 2);
        REQUIRE(std::count(int_arr.cbegin(), int_arr.cend(), 2) == 1);
        REQUIRE(*std::max_element(int_arr.cbegin(), int_arr.cend()) == 5);
        REQUIRE(*std::min_element(int_arr.crbegin(), int_arr.crend()) == 1);
    }
    SECTION("Random access is what it claims, not a forward walk")
    {
        // binary_search only gives its log n behaviour on a genuine random access iterator, and
        // reverse would not compile at all without operator-- and the ordering operators.
        DynamicArray<i32> int_arr{1, 2, 3, 4, 5, 6, 7, 8};
        REQUIRE(std::binary_search(int_arr.begin(), int_arr.end(), 6));
        REQUIRE(!std::binary_search(int_arr.begin(), int_arr.end(), 9));
        std::reverse(int_arr.begin(), int_arr.end());
        REQUIRE(int_arr[0] == 8);
        REQUIRE(int_arr[7] == 1);
    }
}

TEST_CASE("Reverse iterator", "[Array]")
{
    SECTION("Walks the elements back to front")
    {
        DynamicArray<i32> int_arr{1, 2, 3, 4};
        DynamicArray<i32> seen;
        for (DynamicArray<i32>::reverse_iterator it = int_arr.rbegin(); it != int_arr.rend(); ++it)
        {
            seen.PushBack(*it);
        }
        REQUIRE(seen.GetSize() == 4);
        REQUIRE(seen[0] == 4);
        REQUIRE(seen[1] == 3);
        REQUIRE(seen[2] == 2);
        REQUIRE(seen[3] == 1);
    }
    SECTION("An empty array has nothing to walk")
    {
        DynamicArray<i32> int_arr;
        REQUIRE(int_arr.rbegin() == int_arr.rend());
        REQUIRE(int_arr.crbegin() == int_arr.crend());
    }
    SECTION("Writes through to the array")
    {
        DynamicArray<i32> int_arr{1, 2, 3};
        *int_arr.rbegin() = 9;
        REQUIRE(int_arr[2] == 9);
    }
    SECTION("Reads a const array")
    {
        const DynamicArray<i32> int_arr{1, 2, 3};
        REQUIRE(*int_arr.rbegin() == 3);
        REQUIRE(*int_arr.crbegin() == 3);
        REQUIRE(int_arr.crend() - int_arr.crbegin() == 3);
    }
    SECTION("The base is the position one past what it refers to")
    {
        DynamicArray<i32> int_arr{1, 2, 3};
        REQUIRE(int_arr.rbegin().GetBase() == int_arr.end());
        REQUIRE(int_arr.rend().GetBase() == int_arr.begin());
        REQUIRE(*int_arr.rbegin() == 3);
    }
    SECTION("Steps and jumps")
    {
        DynamicArray<i32> int_arr{1, 2, 3, 4, 5};
        DynamicArray<i32>::reverse_iterator it = int_arr.rbegin();
        REQUIRE(*it == 5);
        REQUIRE(*(++it) == 4);
        REQUIRE(*(it++) == 4);
        REQUIRE(*it == 3);
        REQUIRE(*(--it) == 4);
        REQUIRE(*(it--) == 4);
        REQUIRE(*it == 5);
        REQUIRE(*(it + 2) == 3);
        REQUIRE(*(2 + it) == 3);
        REQUIRE(it[3] == 2);
        it += 4;
        REQUIRE(*it == 1);
        it -= 2;
        REQUIRE(*it == 3);
        REQUIRE(*(it - 1) == 4);
    }
    SECTION("Difference and ordering follow the walking direction")
    {
        DynamicArray<i32> int_arr{1, 2, 3, 4};
        REQUIRE(int_arr.rend() - int_arr.rbegin() == 4);
        REQUIRE(int_arr.rbegin() < int_arr.rend());
        REQUIRE(int_arr.rend() > int_arr.rbegin());
        REQUIRE(int_arr.rbegin() <= int_arr.rbegin());
        REQUIRE(int_arr.rend() >= int_arr.rbegin());
        REQUIRE(int_arr.rbegin() != int_arr.rend());
    }
    SECTION("Arrow reaches the element")
    {
        DynamicArray<OwnedCopy> arr;
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        REQUIRE(*arr.rbegin()->ptr == 2);
    }
}

TEST_CASE("Emplace at a position", "[Array]")
{
    SECTION("At the front, in the middle and at the end")
    {
        DynamicArray<i32> int_arr{1, 2, 3};
        DynamicArray<i32>::iterator it = int_arr.Emplace(int_arr.cbegin(), 9).GetValue();
        REQUIRE(it == int_arr.begin());
        REQUIRE(*it == 9);
        int_arr.Emplace(int_arr.cbegin() + 2, 8);
        int_arr.Emplace(int_arr.cend(), 7);
        REQUIRE(int_arr.GetSize() == 6);
        REQUIRE(int_arr[0] == 9);
        REQUIRE(int_arr[1] == 1);
        REQUIRE(int_arr[2] == 8);
        REQUIRE(int_arr[5] == 7);
    }
    SECTION("Into an empty array")
    {
        DynamicArray<i32> int_arr;
        int_arr.Emplace(int_arr.cbegin(), 9);
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr[0] == 9);
    }
    SECTION("Builds the element from its arguments")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Emplace(arr.cbegin(), 7);
        arr.Emplace(arr.cbegin(), 8);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 8);
        REQUIRE(*arr[1].ptr == 7);
    }
    SECTION("From an element of the array, at the capacity boundary")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(2);
        arr.PushBack(OwnedCopy(7));
        arr.PushBack(OwnedCopy(8));
        REQUIRE(arr.GetCapacity() == 2);
        arr.Emplace(arr.cbegin(), arr[1]);
        REQUIRE(arr.GetSize() == 3);
        REQUIRE(*arr[0].ptr == 8);
        REQUIRE(*arr[1].ptr == 7);
        REQUIRE(*arr[2].ptr == 8);
    }
}

TEST_CASE("Assign from an initializer list", "[Array]")
{
    SECTION("Replaces the contents")
    {
        DynamicArray<i32> int_arr(5, 42);
        int_arr.Assign({1, 2, 3});
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr.GetCapacity() == 5);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[2] == 3);
    }
    SECTION("Grows when the list is longer")
    {
        DynamicArray<i32> int_arr(2, 42);
        int_arr.Assign({1, 2, 3, 4});
        REQUIRE(int_arr.GetSize() == 4);
        REQUIRE(int_arr.GetCapacity() == 4);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[3] == 4);
    }
    SECTION("An empty list empties the array")
    {
        DynamicArray<i32> int_arr(3, 42);
        int_arr.Assign({});
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 3);
    }
    SECTION("Assignment operator does the same and keeps the allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr(&allocator);
        int_arr = {1, 2, 3};
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[2] == 3);
        REQUIRE(int_arr.GetAllocator() == &allocator);
    }
    SECTION("Non-POD elements are destroyed exactly once")
    {
        g_live_count = 0;
        {
            DynamicArray<ClonableCopy> arr{ClonableCopy(1), ClonableCopy(2)};
            arr.Assign({ClonableCopy(3)});
            REQUIRE(arr.GetSize() == 1);
            REQUIRE(arr[0].value == 3);
        }
        REQUIRE(g_live_count == 0);
    }
}

TEST_CASE("Find and Contains", "[Array]")
{
    SECTION("Finds the first match")
    {
        DynamicArray<i32> int_arr{5, 7, 9, 7};
        REQUIRE(int_arr.Find(7) == int_arr.begin() + 1);
        REQUIRE(int_arr.Find(5) == int_arr.begin());
        REQUIRE(int_arr.Find(9) == int_arr.begin() + 2);
    }
    SECTION("Reports no match as the end")
    {
        DynamicArray<i32> int_arr{5, 7, 9};
        REQUIRE(int_arr.Find(4) == int_arr.end());
        DynamicArray<i32> empty_arr;
        REQUIRE(empty_arr.Find(4) == empty_arr.end());
    }
    SECTION("Reads through a const array")
    {
        const DynamicArray<i32> int_arr{5, 7, 9};
        REQUIRE(int_arr.Find(7) == int_arr.cbegin() + 1);
        REQUIRE(int_arr.Find(4) == int_arr.cend());
        REQUIRE(int_arr.Contains(7));
        REQUIRE(!int_arr.Contains(4));
    }
    SECTION("Contains answers for an empty array")
    {
        DynamicArray<i32> int_arr;
        REQUIRE(!int_arr.Contains(0));
    }
    SECTION("FindIf takes a predicate")
    {
        DynamicArray<i32> int_arr{5, 7, 9, 12};
        auto is_even = [](const i32& value) { return value % 2 == 0; };
        REQUIRE(int_arr.FindIf(is_even) == int_arr.begin() + 3);
        auto is_negative = [](const i32& value) { return value < 0; };
        REQUIRE(int_arr.FindIf(is_negative) == int_arr.end());
    }
    SECTION("The iterator found is a live one")
    {
        DynamicArray<i32> int_arr{5, 7, 9};
        *int_arr.Find(7) = 8;
        REQUIRE(int_arr[1] == 8);
    }
}

TEST_CASE("Remove if", "[Array]")
{
    SECTION("Removes every match and keeps the order")
    {
        DynamicArray<i32> int_arr{1, 2, 3, 4, 5, 6};
        const u64 removed = int_arr.RemoveIf([](const i32& value) { return value % 2 == 0; });
        REQUIRE(removed == 3);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[1] == 3);
        REQUIRE(int_arr[2] == 5);
    }
    SECTION("Removing nothing leaves the array alone")
    {
        DynamicArray<i32> int_arr{1, 3, 5};
        REQUIRE(int_arr.RemoveIf([](const i32& value) { return value % 2 == 0; }) == 0);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[2] == 5);
    }
    SECTION("Removing everything empties it without giving up the memory")
    {
        DynamicArray<i32> int_arr{1, 2, 3};
        REQUIRE(int_arr.RemoveIf([](const i32&) { return true; }) == 3);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 3);
    }
    SECTION("Removing from the front and the back")
    {
        DynamicArray<i32> int_arr{1, 2, 3, 4};
        REQUIRE(int_arr.RemoveIf([](const i32& value) { return value == 1 || value == 4; }) == 2);
        REQUIRE(int_arr.GetSize() == 2);
        REQUIRE(int_arr[0] == 2);
        REQUIRE(int_arr[1] == 3);
    }
    SECTION("Owning elements are destroyed exactly once")
    {
        g_live_count = 0;
        {
            DynamicArray<CountedLive> arr;
            arr.Reserve(4);
            arr.PushBack(CountedLive(1));
            arr.PushBack(CountedLive(2));
            arr.PushBack(CountedLive(3));
            arr.PushBack(CountedLive(4));
            REQUIRE(arr.RemoveIf([](const CountedLive& element) { return element.value % 2 == 0; }) == 2);
            REQUIRE(g_live_count == 2);
            REQUIRE(arr[0].value == 1);
            REQUIRE(arr[1].value == 3);
        }
        REQUIRE(g_live_count == 0);
    }
}

TEST_CASE("Shrink to fit", "[Array]")
{
    SECTION("Gives up the spare room")
    {
        DynamicArray<i32> int_arr;
        int_arr.Reserve(16);
        int_arr.PushBack(1);
        int_arr.PushBack(2);
        REQUIRE(int_arr.GetCapacity() == 16);
        int_arr.ShrinkToFit();
        REQUIRE(int_arr.GetCapacity() == 2);
        REQUIRE(int_arr.GetSize() == 2);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[1] == 2);
    }
    SECTION("An array with no spare room is left alone")
    {
        DynamicArray<i32> int_arr(3, 42);
        const i32* data_before = int_arr.GetData();
        REQUIRE(int_arr.GetCapacity() == 3);
        int_arr.ShrinkToFit();
        REQUIRE(int_arr.GetData() == data_before);
        REQUIRE(int_arr.GetCapacity() == 3);
    }
    SECTION("An emptied array releases its storage outright")
    {
        DynamicArray<i32> int_arr(8, 42);
        int_arr.Clear();
        int_arr.ShrinkToFit();
        REQUIRE(int_arr.GetCapacity() == 0);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetData() == nullptr);
        // Still usable afterwards.
        int_arr.PushBack(7);
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr[0] == 7);
    }
    SECTION("Non-POD elements carry over and the originals are destroyed")
    {
        g_live_count = 0;
        {
            DynamicArray<CountedLive> arr;
            arr.Reserve(16);
            arr.PushBack(CountedLive(1));
            arr.PushBack(CountedLive(2));
            arr.ShrinkToFit();
            REQUIRE(g_live_count == 2);
            REQUIRE(arr.GetCapacity() == 2);
            REQUIRE(arr[0].value == 1);
            REQUIRE(arr[1].value == 2);
        }
        REQUIRE(g_live_count == 0);
    }
}

TEST_CASE("Swap two arrays", "[Array]")
{
    // Opal::Swap already picks its move-based overload for DynamicArray, so the array needs no
    // Swap of its own. This pins that, and that it stays a handful of pointer moves.
    SECTION("Exchanges contents, capacity and allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> left(&allocator);
        left.Reserve(8);
        left.PushBack(1);
        DynamicArray<i32> right;
        right.PushBack(2);
        right.PushBack(3);

        Swap(left, right);

        REQUIRE(left.GetSize() == 2);
        REQUIRE(left[0] == 2);
        REQUIRE(left[1] == 3);
        REQUIRE(left.GetAllocator() == GetDefaultAllocator());
        REQUIRE(right.GetSize() == 1);
        REQUIRE(right[0] == 1);
        REQUIRE(right.GetCapacity() == 8);
        REQUIRE(right.GetAllocator() == &allocator);
    }
    SECTION("Does not touch the elements themselves")
    {
        g_live_count = 0;
        {
            DynamicArray<CountedLive> left;
            left.PushBack(CountedLive(1));
            DynamicArray<CountedLive> right;
            right.PushBack(CountedLive(2));
            const i32 live_before = g_live_count;
            Swap(left, right);
            REQUIRE(g_live_count == live_before);
            REQUIRE(left[0].value == 2);
            REQUIRE(right[0].value == 1);
        }
        REQUIRE(g_live_count == 0);
    }
}

TEST_CASE("Set allocator", "[Array]")
{
    SECTION("An array always has an allocator")
    {
        DynamicArray<i32> int_arr;
        REQUIRE(int_arr.GetAllocator() != nullptr);
        REQUIRE(int_arr.GetAllocator() == GetDefaultAllocator());
    }
    SECTION("Moving to another allocator gives up the spare room")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr;
        int_arr.Reserve(16);
        int_arr.PushBack(1);
        int_arr.PushBack(2);
        REQUIRE(int_arr.GetCapacity() == 16);
        int_arr.SetAllocator(&allocator);
        REQUIRE(int_arr.GetAllocator() == &allocator);
        REQUIRE(int_arr.GetCapacity() == 2);
        REQUIRE(int_arr.GetSize() == 2);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[1] == 2);
    }
    SECTION("Passing nullptr moves to the default allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr(&allocator);
        int_arr.PushBack(1);
        int_arr.SetAllocator(nullptr);
        REQUIRE(int_arr.GetAllocator() == GetDefaultAllocator());
        REQUIRE(int_arr.GetSize() == 1);
        REQUIRE(int_arr[0] == 1);
    }
    SECTION("Passing the allocator already in use does nothing")
    {
        MallocAllocator allocator;
        DynamicArray<i32> int_arr(&allocator);
        int_arr.Reserve(16);
        int_arr.PushBack(1);
        const i32* data_before = int_arr.GetData();
        int_arr.SetAllocator(&allocator);
        REQUIRE(int_arr.GetData() == data_before);
        REQUIRE(int_arr.GetCapacity() == 16);
        REQUIRE(int_arr[0] == 1);
    }
    SECTION("An empty array moves without allocating")
    {
        // Nothing to carry over, so the new allocator is not asked for anything.
        BudgetedAllocator allocator(0);
        DynamicArray<i32> int_arr;
        REQUIRE_NOTHROW(int_arr.SetAllocator(&allocator));
        REQUIRE(int_arr.GetAllocator() == &allocator);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 0);
    }
}

TEST_CASE("Allocation failure", "[Array]")
{
    SECTION("Reserve reports an allocator that hands back nothing")
    {
        BudgetedAllocator allocator(0);
        DynamicArray<i32> int_arr(&allocator);
        REQUIRE(int_arr.Reserve(4) == ErrorCode::OutOfMemory);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 0);
    }
    SECTION("PushBack reports an allocator that hands back nothing")
    {
        BudgetedAllocator allocator(0);
        DynamicArray<i32> int_arr(&allocator);
        REQUIRE(int_arr.PushBack(1) == ErrorCode::OutOfMemory);
        REQUIRE(int_arr.GetSize() == 0);
    }
    SECTION("Resize reports an allocator that hands back nothing")
    {
        BudgetedAllocator allocator(0);
        DynamicArray<i32> int_arr(&allocator);
        REQUIRE(int_arr.Resize(4) == ErrorCode::OutOfMemory);
        REQUIRE(int_arr.GetSize() == 0);
    }
    SECTION("Assign holds on to a buffer it owns when the allocation fails")
    {
        // One allocation builds the array, leaving none for the Assign that has to grow it.
        BudgetedAllocator allocator(1);
        DynamicArray<i32> int_arr(2, 42, &allocator);
        REQUIRE(int_arr.GetSize() == 2);
        REQUIRE(int_arr.Assign(8, 7) == ErrorCode::OutOfMemory);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 2);
        // The buffer it kept has to be one it still owns, so that growing again works and the
        // destructor frees live storage rather than storage already handed back.
        allocator.allowed = 1;
        REQUIRE(int_arr.Assign(8, 7) == ErrorCode::Success);
        REQUIRE(int_arr.GetSize() == 8);
        REQUIRE(int_arr.GetCapacity() == 8);
        REQUIRE(int_arr[0] == 7);
        REQUIRE(int_arr[7] == 7);
    }
    SECTION("Assign from a range holds on to a buffer it owns when the allocation fails")
    {
        BudgetedAllocator allocator(1);
        DynamicArray<i32> int_arr(2, 42, &allocator);
        const DynamicArray<i32> source(8, 7);
        REQUIRE(int_arr.Assign(source.cbegin(), source.cend()) == ErrorCode::OutOfMemory);
        REQUIRE(int_arr.GetSize() == 0);
        REQUIRE(int_arr.GetCapacity() == 2);
        allocator.allowed = 1;
        REQUIRE(int_arr.Assign(source.cbegin(), source.cend()) == ErrorCode::Success);
        REQUIRE(int_arr.GetSize() == 8);
        REQUIRE(int_arr[0] == 7);
        REQUIRE(int_arr[7] == 7);
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
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 5);
            }
            REQUIRE(g_destroy_call_count == 6);
        }
        SECTION("More then current capacity")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 3);
            }
            // 1 temporary, 3 originals left behind by the reallocation, 3 live elements.
            REQUIRE(g_destroy_call_count == 7);
        }
    }
}

// Moving the elements into a new buffer leaves the originals behind as live objects. They own
// their storage until they are destroyed, so skipping that leaks whatever they still hold.
TEST_CASE("Moving to a new buffer destroys the elements it moved from", "[Array]")
{
    SECTION("Reserve")
    {
        g_live_count = 0;
        {
            DynamicArray<CountedLive> arr;
            arr.Reserve(2);
            arr.PushBack(CountedLive(1));
            arr.PushBack(CountedLive(2));
            REQUIRE(g_live_count == 2);
            arr.Reserve(8);
            REQUIRE(g_live_count == 2);
            REQUIRE(arr.GetCapacity() == 8);
            REQUIRE(arr[0].value == 1);
            REQUIRE(arr[1].value == 2);
        }
        REQUIRE(g_live_count == 0);
    }
    SECTION("SetAllocator")
    {
        MallocAllocator allocator;
        g_live_count = 0;
        {
            DynamicArray<CountedLive> arr;
            arr.Reserve(4);
            arr.PushBack(CountedLive(1));
            arr.PushBack(CountedLive(2));
            REQUIRE(g_live_count == 2);
            arr.SetAllocator(&allocator);
            REQUIRE(g_live_count == 2);
            REQUIRE(arr.GetAllocator() == &allocator);
            REQUIRE(arr[0].value == 1);
            REQUIRE(arr[1].value == 2);
        }
        REQUIRE(g_live_count == 0);
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
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 3);
            }
            REQUIRE(g_destroy_call_count == 5);
        }
        SECTION("To new size which is less then old size")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 5);
            }
            REQUIRE(g_destroy_call_count == 7);
        }
        SECTION("To new size which is greater then old size and smaller then capacity")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 4);
            }
            // 2 temporaries, 3 originals left behind by the Reserve, 4 live elements.
            REQUIRE(g_destroy_call_count == 9);
        }
        SECTION("To new size which is greater then capacity")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 6);
            }
            // 2 temporaries, 3 originals left behind by the growth, 6 live elements.
            REQUIRE(g_destroy_call_count == 11);
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
        g_clone_call_count = 0;
        g_destroy_call_count = 0;
        {
            DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
            non_pod_arr.Clear();
            REQUIRE(non_pod_arr.GetCapacity() == 3);
            REQUIRE(non_pod_arr.GetSize() == 0);
            REQUIRE(non_pod_arr.GetData() != nullptr);
            REQUIRE(g_value_call_count == 1);
            REQUIRE(g_clone_call_count == 3);
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
                g_clone_call_count = 0;
                g_destroy_call_count = 0;
                {
                    DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                    NonPod val(25);
                    non_pod_arr.PushBack(std::move(val));
                    REQUIRE(non_pod_arr.GetCapacity() == 5);
                    REQUIRE(non_pod_arr.GetSize() == 4);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_clone_call_count == 3);
                }
                // 2 objects outside the array, 3 originals left behind by the growth, 4 live
                // elements. Nothing is staged: the value pushed does not live in this array.
                REQUIRE(g_destroy_call_count == 9);
            }
            SECTION("Without enough capacity")
            {
                g_value_call_count = 0;
                g_clone_call_count = 0;
                g_destroy_call_count = 0;
                {
                    DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
                    NonPod val(25);
                    non_pod_arr.PushBack(std::move(val));
                    REQUIRE(non_pod_arr.GetCapacity() == 7);
                    REQUIRE(non_pod_arr.GetSize() == 5);
                    REQUIRE(non_pod_arr.GetData() != nullptr);
                    REQUIRE(*non_pod_arr[0].ptr == 42);
                    REQUIRE(*non_pod_arr[1].ptr == 42);
                    REQUIRE(*non_pod_arr[2].ptr == 42);
                    REQUIRE(*non_pod_arr[3].ptr == 42);
                    REQUIRE(*non_pod_arr[4].ptr == 25);
                    REQUIRE(g_value_call_count == 2);
                    REQUIRE(g_clone_call_count == 4);
                }
                // 2 objects outside the array, 4 originals left behind by the growth, 5 live
                // elements. Nothing is staged: the value pushed does not live in this array.
                REQUIRE(g_destroy_call_count == 11);
            }
        }
        SECTION("With move")
        {
            SECTION("With enough capacity")
            {
                g_value_call_count = 0;
                g_clone_call_count = 0;
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
                    REQUIRE(g_clone_call_count == 3);
                }
                // 2 objects outside the array, 3 originals left behind by the growth, 4 live
                // elements. Nothing is staged: the value pushed does not live in this array.
                REQUIRE(g_destroy_call_count == 9);
            }
            SECTION("Without enough capacity")
            {
                g_value_call_count = 0;
                g_clone_call_count = 0;
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
                    REQUIRE(g_clone_call_count == 4);
                }
                // 2 objects outside the array, 4 originals left behind by the growth, 5 live
                // elements. Nothing is staged: the value pushed does not live in this array.
                REQUIRE(g_destroy_call_count == 11);
            }
        }
    }
}

// Growing frees the buffer the elements live in. A value handed in by reference is allowed to be
// one of those elements, so it has to survive the growth it triggers.
TEST_CASE("Adding an element of the array to itself", "[Array]")
{
    SECTION("PushBack a copy at the capacity boundary")
    {
        DynamicArray<i32> int_arr;
        int_arr.Reserve(2);
        int_arr.PushBack(7);
        int_arr.PushBack(8);
        REQUIRE(int_arr.GetCapacity() == 2);
        int_arr.PushBack(int_arr[0]);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 7);
        REQUIRE(int_arr[1] == 8);
        REQUIRE(int_arr[2] == 7);
    }
    SECTION("PushBack a move at the capacity boundary")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(2);
        arr.PushBack(OwnedCopy(7));
        arr.PushBack(OwnedCopy(8));
        REQUIRE(arr.GetCapacity() == 2);
        arr.PushBack(Move(arr[0]));
        REQUIRE(arr.GetSize() == 3);
        REQUIRE(arr[2].ptr != nullptr);
        REQUIRE(*arr[1].ptr == 8);
        REQUIRE(*arr[2].ptr == 7);
    }
    SECTION("EmplaceBack from an element at the capacity boundary")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(2);
        arr.PushBack(OwnedCopy(7));
        arr.PushBack(OwnedCopy(8));
        REQUIRE(arr.GetCapacity() == 2);
        arr.EmplaceBack(arr[0]);
        REQUIRE(arr.GetSize() == 3);
        REQUIRE(arr[2].ptr != nullptr);
        REQUIRE(*arr[0].ptr == 7);
        REQUIRE(*arr[2].ptr == 7);
    }
    SECTION("Insert a copy at the capacity boundary")
    {
        DynamicArray<i32> int_arr;
        int_arr.Reserve(2);
        int_arr.PushBack(7);
        int_arr.PushBack(8);
        REQUIRE(int_arr.GetCapacity() == 2);
        int_arr.Insert(int_arr.cbegin(), int_arr[1]);
        REQUIRE(int_arr.GetSize() == 3);
        REQUIRE(int_arr[0] == 8);
        REQUIRE(int_arr[1] == 7);
        REQUIRE(int_arr[2] == 8);
    }
    SECTION("Insert several copies at the capacity boundary")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(2);
        arr.PushBack(OwnedCopy(7));
        arr.PushBack(OwnedCopy(8));
        REQUIRE(arr.GetCapacity() == 2);
        arr.Insert(arr.cbegin() + 1, 3, arr[0]);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 7);
        REQUIRE(*arr[1].ptr == 7);
        REQUIRE(*arr[2].ptr == 7);
        REQUIRE(*arr[3].ptr == 7);
        REQUIRE(*arr[4].ptr == 8);
    }
    SECTION("Insert a value from after the position, with room to spare")
    {
        // No reallocation here, but opening the gap shifts the very element the value names.
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(8);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        arr.Insert(arr.cbegin(), arr[2]);
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(*arr[0].ptr == 3);
        REQUIRE(*arr[1].ptr == 1);
        REQUIRE(*arr[2].ptr == 2);
        REQUIRE(*arr[3].ptr == 3);
    }
    SECTION("Insert several copies of a value from after the position, with room to spare")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(8);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        arr.Insert(arr.cbegin(), 2, arr[2]);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 3);
        REQUIRE(*arr[1].ptr == 3);
        REQUIRE(*arr[2].ptr == 1);
        REQUIRE(*arr[4].ptr == 3);
    }
    SECTION("Assign from an element of the array")
    {
        DynamicArray<NonPod> arr;
        arr.PushBack(NonPod(1));
        arr.PushBack(NonPod(2));
        arr.Assign(4, arr[1]);
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[3].ptr == 2);
    }
    SECTION("Resize from an element of the array")
    {
        DynamicArray<NonPod> arr;
        arr.PushBack(NonPod(1));
        arr.PushBack(NonPod(2));
        REQUIRE(arr.GetCapacity() == 2);
        arr.Resize(5, arr[0]);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
        REQUIRE(*arr[2].ptr == 1);
        REQUIRE(*arr[4].ptr == 1);
    }
}

// A source range that reads out of the array being changed cannot survive its elements being
// shifted or released, so those cases build the result somewhere else first.
TEST_CASE("Inserting and assigning a range of the array into itself", "[Array]")
{
    SECTION("Insert a subrange at the front, at the capacity boundary")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(3);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        REQUIRE(arr.GetCapacity() == 3);
        arr.Insert(arr.cbegin(), arr.cbegin() + 1, arr.cbegin() + 3);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[1].ptr == 3);
        REQUIRE(*arr[2].ptr == 1);
        REQUIRE(*arr[3].ptr == 2);
        REQUIRE(*arr[4].ptr == 3);
    }
    SECTION("Insert a subrange from after the position, with room to spare")
    {
        // No reallocation, so the shift is what would clobber the source.
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(16);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        arr.Insert(arr.cbegin(), arr.cbegin() + 1, arr.cbegin() + 3);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[1].ptr == 3);
        REQUIRE(*arr[2].ptr == 1);
        REQUIRE(*arr[3].ptr == 2);
        REQUIRE(*arr[4].ptr == 3);
    }
    SECTION("Insert a subrange from before the position")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(16);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        arr.Insert(arr.cend(), arr.cbegin(), arr.cbegin() + 2);
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 1);
        REQUIRE(*arr[4].ptr == 2);
    }
    SECTION("Assign from a subrange of the array")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(4);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        arr.PushBack(OwnedCopy(3));
        REQUIRE(arr.Assign(arr.cbegin() + 1, arr.cbegin() + 3) == ErrorCode::Success);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[1].ptr == 3);
    }
    SECTION("Assign from the whole array")
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(4);
        arr.PushBack(OwnedCopy(1));
        arr.PushBack(OwnedCopy(2));
        REQUIRE(arr.Assign(arr.cbegin(), arr.cend()) == ErrorCode::Success);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
    }
}

TEST_CASE("Emplace back", "[Array]")
{
    SECTION("POD data")
    {
        SECTION("With enough capacity")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32& ref = int_arr.EmplaceBack(25).GetValue();
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr[3] == 25);
            REQUIRE(ref == 25);
        }
        SECTION("Without enough capacity")
        {
            DynamicArray<i32> int_arr(4, 42);
            i32& ref = int_arr.EmplaceBack(25).GetValue();
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
            g_clone_call_count = 0;
            g_destroy_call_count = 0;
            {
                DynamicArray<NonPod> non_pod_arr(3, NonPod(42));
                g_value_call_count = 0;
                g_clone_call_count = 0;
                NonPod& ref = non_pod_arr.EmplaceBack(25).GetValue();
                REQUIRE(non_pod_arr.GetSize() == 4);
                REQUIRE(*non_pod_arr[3].ptr == 25);
                REQUIRE(*ref.ptr == 25);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_clone_call_count == 0);
            }
        }
        SECTION("Without enough capacity")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
            g_destroy_call_count = 0;
            {
                DynamicArray<NonPod> non_pod_arr(4, NonPod(42));
                g_value_call_count = 0;
                g_clone_call_count = 0;
                NonPod& ref = non_pod_arr.EmplaceBack(25).GetValue();
                REQUIRE(non_pod_arr.GetSize() == 5);
                REQUIRE(*non_pod_arr[4].ptr == 25);
                REQUIRE(*ref.ptr == 25);
                REQUIRE(g_value_call_count == 1);
                REQUIRE(g_clone_call_count == 0);
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
            g_clone_call_count = 0;
            g_destroy_call_count = 0;
            {
                DynamicArray<NonPod> non_pod_arr;
                non_pod_arr.PopBack();
                REQUIRE(non_pod_arr.GetCapacity() == 0);
                REQUIRE(non_pod_arr.GetSize() == 0);
                REQUIRE(non_pod_arr.GetData() == nullptr);
                REQUIRE(g_value_call_count == 0);
                REQUIRE(g_clone_call_count == 0);
            }
            REQUIRE(g_destroy_call_count == 0);
        }
        SECTION("Non-empty array")
        {
            g_value_call_count = 0;
            g_clone_call_count = 0;
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
                REQUIRE(g_clone_call_count == 3);
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, val).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, 25).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), val).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), 25).GetValue());
            REQUIRE(*it == 25);
            REQUIRE(int_arr.GetCapacity() == 5);
            REQUIRE(int_arr.GetSize() == 4);
            REQUIRE(int_arr.GetData() != nullptr);
            REQUIRE(int_arr[0] == 42);
            REQUIRE(int_arr[1] == 42);
            REQUIRE(int_arr[2] == 42);
            REQUIRE(int_arr[3] == 25);
        }
    }
    SECTION("Insert multiple same elements")
    {
        SECTION("In mid")
        {
            DynamicArray<i32> int_arr(3, 42);
            const i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, 2, val).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), 2, val).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), 2, val).GetValue());
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
        SECTION("Bad count")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32 val = 25;
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), 0, val).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, other.cbegin(), other.cend()).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), other.cbegin(), other.cend()).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cend(), other.cbegin(), other.cend()).GetValue());
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
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin(), other.cbegin(), other.cend()).GetValue());
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
        SECTION("Insert from C style array")
        {
            DynamicArray<i32> int_arr(3, 42);
            i32 other[] = {5, 5};
            DynamicArray<i32>::iterator it;
            REQUIRE_NOTHROW(it = int_arr.Insert(int_arr.cbegin() + 1, other, other + 2).GetValue());
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

// Inserting shifts the elements at and after the position up. The slots they land in past
// the old end hold no object yet, so they must be constructed into rather than assigned to:
// assigning would run the element's cleanup on memory that was never initialized.
TEST_CASE("Insert of an owning element type", "[Array]")
{
    const auto make = [](i32 count)
    {
        DynamicArray<NonPod> arr;
        arr.Reserve(16);  // room for every insert below, so no reallocation moves anything
        for (i32 i = 0; i < count; ++i)
        {
            arr.PushBack(NonPod(i + 1));
        }
        return arr;
    };
    const auto make_copyable = [](i32 count)
    {
        DynamicArray<OwnedCopy> arr;
        arr.Reserve(16);
        for (i32 i = 0; i < count; ++i)
        {
            arr.PushBack(OwnedCopy(i + 1));
        }
        return arr;
    };

    SECTION("Insert one element at the front")
    {
        DynamicArray<NonPod> arr = make(3);
        arr.Insert(arr.cbegin(), NonPod(9));
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(*arr[0].ptr == 9);
        REQUIRE(*arr[1].ptr == 1);
        REQUIRE(*arr[2].ptr == 2);
        REQUIRE(*arr[3].ptr == 3);
    }
    SECTION("Insert one element at the end")
    {
        DynamicArray<NonPod> arr = make(3);
        arr.Insert(arr.cend(), NonPod(9));
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 9);
    }
    SECTION("Insert into an empty array")
    {
        DynamicArray<NonPod> arr = make(0);
        arr.Insert(arr.cbegin(), NonPod(9));
        REQUIRE(arr.GetSize() == 1);
        REQUIRE(*arr[0].ptr == 9);
    }
    SECTION("Insert more elements than the array holds after the position")
    {
        // The gap reaches past the old end, so some survivors are constructed into raw
        // storage and some of the inserted values are too.
        DynamicArray<OwnedCopy> arr = make_copyable(3);
        arr.Insert(arr.cbegin() + 2, 4, OwnedCopy(9));
        REQUIRE(arr.GetSize() == 7);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
        REQUIRE(*arr[2].ptr == 9);
        REQUIRE(*arr[5].ptr == 9);
        REQUIRE(*arr[6].ptr == 3);
    }
    SECTION("Insert fewer elements than the array holds after the position")
    {
        DynamicArray<OwnedCopy> arr = make_copyable(5);
        arr.Insert(arr.cbegin() + 1, 2, OwnedCopy(9));
        REQUIRE(arr.GetSize() == 7);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 9);
        REQUIRE(*arr[2].ptr == 9);
        REQUIRE(*arr[3].ptr == 2);
        REQUIRE(*arr[6].ptr == 5);
    }
    SECTION("Insert a range at the end")
    {
        DynamicArray<OwnedCopy> source = make_copyable(2);  // 1, 2
        DynamicArray<OwnedCopy> arr = make_copyable(3);     // 1, 2, 3
        arr.Insert(arr.cbegin() + 3, source.begin(), source.end());
        REQUIRE(arr.GetSize() == 5);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 1);
        REQUIRE(*arr[4].ptr == 2);
        REQUIRE(source.GetSize() == 2);  // the source is copied, not consumed
        REQUIRE(*source[0].ptr == 1);
    }
    SECTION("Insert a range in the middle")
    {
        DynamicArray<OwnedCopy> source = make_copyable(2);
        DynamicArray<OwnedCopy> arr = make_copyable(4);
        arr.Insert(arr.cbegin() + 1, source.begin(), source.end());
        REQUIRE(arr.GetSize() == 6);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 1);
        REQUIRE(*arr[2].ptr == 2);
        REQUIRE(*arr[3].ptr == 2);
        REQUIRE(*arr[5].ptr == 4);
    }
    SECTION("Insert an empty range")
    {
        DynamicArray<OwnedCopy> source = make_copyable(2);
        DynamicArray<OwnedCopy> arr = make_copyable(4);
        DynamicArray<OwnedCopy>::iterator it;
        REQUIRE_NOTHROW(it = arr.Insert(arr.cbegin() + 1, source.begin(), source.begin()).GetValue());
        REQUIRE(it == arr.begin() + 1);
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 4);
    }
    SECTION("Insert an empty range does not move elements onto themselves")
    {
        // An empty range opens a zero-wide gap, and shifting elements across it assigns each
        // one onto itself. An element type that does not guard its move assignment loses what
        // it owns.
        DynamicArray<SelfMoveUnsafe> source;
        DynamicArray<SelfMoveUnsafe> arr;
        arr.Reserve(4);
        for (i32 i = 0; i < 4; ++i)
        {
            arr.PushBack(SelfMoveUnsafe(i + 1));
        }
        arr.Insert(arr.cbegin() + 1, source.begin(), source.end());
        REQUIRE(arr.GetSize() == 4);
        REQUIRE(arr[0].ptr != nullptr);
        REQUIRE(arr[1].ptr != nullptr);
        REQUIRE(arr[2].ptr != nullptr);
        REQUIRE(arr[3].ptr != nullptr);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 4);
    }
}

// Erasing an element that is not the last shifts the survivors down over it. The shift must
// not assign into a slot whose element was already destroyed: for an owning type that runs
// the assignment's cleanup on a pointer that was already freed, which is a double free.
TEST_CASE("Erase of an owning element type", "[Array]")
{
    const auto make = []()
    {
        DynamicArray<NonPod> arr;
        arr.Reserve(3);  // no reallocation, so the destructor count below counts only erases
        arr.PushBack(NonPod(1));
        arr.PushBack(NonPod(2));
        arr.PushBack(NonPod(3));
        g_destroy_call_count = 0;
        return arr;
    };

    SECTION("Erase the first element through an iterator")
    {
        DynamicArray<NonPod> arr = make();
        arr.Erase(arr.begin());
        REQUIRE(g_destroy_call_count == 1);  // exactly one element left the array
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[1].ptr == 3);
    }
    SECTION("Erase the first element through a const iterator")
    {
        DynamicArray<NonPod> arr = make();
        arr.Erase(arr.cbegin());
        REQUIRE(g_destroy_call_count == 1);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 2);
        REQUIRE(*arr[1].ptr == 3);
    }
    SECTION("Erase the last element, which shifts nothing")
    {
        DynamicArray<NonPod> arr = make();
        arr.Erase(arr.end() - 1);
        REQUIRE(g_destroy_call_count == 1);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
    }
    SECTION("EraseWithSwap moves the last element into the hole")
    {
        DynamicArray<NonPod> arr = make();
        arr.EraseWithSwap(arr.begin());
        REQUIRE(g_destroy_call_count == 1);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 3);
        REQUIRE(*arr[1].ptr == 2);
    }
    SECTION("Erase a range from the front")
    {
        DynamicArray<NonPod> arr = make();
        arr.Erase(arr.begin(), arr.begin() + 2);
        REQUIRE(g_destroy_call_count == 2);
        REQUIRE(arr.GetSize() == 1);
        REQUIRE(*arr[0].ptr == 3);
    }
    SECTION("Erase every element through a range")
    {
        DynamicArray<NonPod> arr = make();
        arr.Erase(arr.begin(), arr.end());
        REQUIRE(g_destroy_call_count == 3);
        REQUIRE(arr.GetSize() == 0);
    }
    SECTION("EraseWithSwap on the last element, which swaps nothing")
    {
        // The other branch of EraseWithSwap: there is no element to move into the hole, so it
        // only destroys. Covered for i32 but never for a type where a missed destructor shows.
        DynamicArray<NonPod> arr = make();
        DynamicArray<NonPod>::iterator it = arr.EraseWithSwap(arr.end() - 1);
        REQUIRE(it == arr.end());
        REQUIRE(g_destroy_call_count == 1);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
    }
    SECTION("EraseWithSwap on the only element")
    {
        DynamicArray<NonPod> arr;
        arr.PushBack(NonPod(1));
        g_destroy_call_count = 0;
        arr.EraseWithSwap(arr.begin());
        REQUIRE(g_destroy_call_count == 1);
        REQUIRE(arr.GetSize() == 0);
    }
    SECTION("Remove takes out the first match and destroys it once")
    {
        DynamicArray<NonPod> arr = make();
        arr.Remove(NonPod(2));
        // One for the element removed, one for the temporary compared against.
        REQUIRE(g_destroy_call_count == 2);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 3);
    }
    SECTION("Remove of a value that is not there leaves the array alone")
    {
        DynamicArray<NonPod> arr = make();
        arr.Remove(NonPod(9));
        REQUIRE(g_destroy_call_count == 1);  // only the temporary
        REQUIRE(arr.GetSize() == 3);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[2].ptr == 3);
    }
    SECTION("RemoveWithSwap takes out the first match and destroys it once")
    {
        DynamicArray<NonPod> arr = make();
        arr.RemoveWithSwap(NonPod(1));
        REQUIRE(g_destroy_call_count == 2);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 3);
        REQUIRE(*arr[1].ptr == 2);
    }
    SECTION("RemoveWithSwap of the last element")
    {
        DynamicArray<NonPod> arr = make();
        arr.RemoveWithSwap(NonPod(3));
        REQUIRE(g_destroy_call_count == 2);
        REQUIRE(arr.GetSize() == 2);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
    }
    SECTION("RemoveWithSwap of a value that is not there leaves the array alone")
    {
        DynamicArray<NonPod> arr = make();
        arr.RemoveWithSwap(NonPod(9));
        REQUIRE(g_destroy_call_count == 1);  // only the temporary
        REQUIRE(arr.GetSize() == 3);
    }
    SECTION("Removing every element one at a time")
    {
        DynamicArray<NonPod> arr = make();
        arr.Remove(NonPod(2));
        arr.Remove(NonPod(1));
        arr.Remove(NonPod(3));
        REQUIRE(arr.GetSize() == 0);
        // Three elements and three temporaries.
        REQUIRE(g_destroy_call_count == 6);
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
    SECTION("Append leaves a non-const lvalue source intact")
    {
        // An owning element type, so a source that was moved out of rather than cloned shows up
        // as a null of its own.
        DynamicArray<NonPod> source;
        source.PushBack(NonPod(1));
        source.PushBack(NonPod(2));
        DynamicArray<NonPod> non_pod_arr;
        non_pod_arr.Append(source);
        REQUIRE(non_pod_arr.GetSize() == 2);
        REQUIRE(*non_pod_arr[0].ptr == 1);
        REQUIRE(*non_pod_arr[1].ptr == 2);
        REQUIRE(source.GetSize() == 2);
        REQUIRE(source[0].ptr != nullptr);
        REQUIRE(source[1].ptr != nullptr);
        REQUIRE(*source[0].ptr == 1);
        REQUIRE(*source[1].ptr == 2);
    }
    SECTION("Append an array to itself")
    {
        // Every element is read after the array has already grown once, so without room taken up
        // front the loop walks a buffer PushBack has freed.
        DynamicArray<NonPod> arr;
        arr.PushBack(NonPod(1));
        arr.PushBack(NonPod(2));
        arr.PushBack(NonPod(3));
        REQUIRE(arr.GetCapacity() == 4);
        arr.Append(arr);
        REQUIRE(arr.GetSize() == 6);
        REQUIRE(*arr[0].ptr == 1);
        REQUIRE(*arr[1].ptr == 2);
        REQUIRE(*arr[2].ptr == 3);
        REQUIRE(*arr[3].ptr == 1);
        REQUIRE(*arr[4].ptr == 2);
        REQUIRE(*arr[5].ptr == 3);
    }
    SECTION("Append takes the room for the whole range in one allocation")
    {
        // Two allocations all told: one for the first element, one for the append. Letting each
        // PushBack grow on its own would take four more, and the budget would run out.
        BudgetedAllocator allocator(2);
        DynamicArray<i32> int_arr(&allocator);
        int_arr.PushBack(1);
        const DynamicArray<i32> source(16, 7);
        REQUIRE_NOTHROW(int_arr.Append(source));
        REQUIRE(allocator.allowed == 0);
        REQUIRE(int_arr.GetSize() == 17);
        REQUIRE(int_arr[0] == 1);
        REQUIRE(int_arr[16] == 7);
    }
    SECTION("Append consumes an rvalue source")
    {
        DynamicArray<NonPod> source;
        source.PushBack(NonPod(1));
        source.PushBack(NonPod(2));
        DynamicArray<NonPod> non_pod_arr;
        non_pod_arr.Append(Move(source));
        REQUIRE(non_pod_arr.GetSize() == 2);
        REQUIRE(*non_pod_arr[0].ptr == 1);
        REQUIRE(*non_pod_arr[1].ptr == 2);
        REQUIRE(source.GetSize() == 2);
        REQUIRE(source[0].ptr == nullptr);
        REQUIRE(source[1].ptr == nullptr);
    }
}

TEST_CASE("Clone", "[Array]")
{
    SECTION("Clone empty array with nullptr allocator")
    {
        DynamicArray<i32> src;
        DynamicArray<i32> clone = src.Clone(nullptr);
        REQUIRE(clone.GetSize() == 0);
        REQUIRE(clone.GetAllocator() == src.GetAllocator());
    }
    SECTION("Clone POD array with nullptr allocator")
    {
        DynamicArray<i32> src = {1, 2, 3};
        DynamicArray<i32> clone = src.Clone(nullptr);
        REQUIRE(clone.GetSize() == 3);
        REQUIRE(clone[0] == 1);
        REQUIRE(clone[1] == 2);
        REQUIRE(clone[2] == 3);
        REQUIRE(clone.GetCapacity() == src.GetCapacity());
        REQUIRE(clone.GetAllocator() == src.GetAllocator());
        REQUIRE(clone.GetData() != src.GetData());
    }
    SECTION("Clone POD array with different allocator")
    {
        MallocAllocator allocator;
        DynamicArray<i32> src = {10, 20, 30};
        DynamicArray<i32> clone = src.Clone(&allocator);
        REQUIRE(clone.GetSize() == 3);
        REQUIRE(clone[0] == 10);
        REQUIRE(clone[1] == 20);
        REQUIRE(clone[2] == 30);
        REQUIRE(clone.GetCapacity() == src.GetCapacity());
        REQUIRE(clone.GetAllocator() == &allocator);
        REQUIRE(clone.GetAllocator() != src.GetAllocator());
    }
    SECTION("Clone non-POD array with nullptr allocator")
    {
        DynamicArray<NonPod> src;
        src.PushBack(NonPod(1));
        src.PushBack(NonPod(2));
        DynamicArray<NonPod> clone = src.Clone(nullptr);
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(*clone[0].ptr == 1);
        REQUIRE(*clone[1].ptr == 2);
        REQUIRE(clone.GetCapacity() == src.GetCapacity());
        REQUIRE(clone.GetAllocator() == src.GetAllocator());
        REQUIRE(clone.GetData() != src.GetData());
    }
    SECTION("Clone non-POD array with different allocator")
    {
        MallocAllocator allocator;
        DynamicArray<NonPod> src;
        src.PushBack(NonPod(3));
        src.PushBack(NonPod(4));
        DynamicArray<NonPod> clone = src.Clone(&allocator);
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(*clone[0].ptr == 3);
        REQUIRE(*clone[1].ptr == 4);
        REQUIRE(clone.GetAllocator() == &allocator);
    }
    SECTION("The clone takes its memory from the allocator it was given")
    {
        // The sections above check the allocator the clone reports, which a Clone that recorded
        // the pointer without ever calling it would also pass. This asks the allocator itself.
        BudgetedAllocator allocator(1);
        DynamicArray<i32> src = {1, 2, 3};
        DynamicArray<i32> clone = src.Clone(&allocator);
        REQUIRE(allocator.allowed == 0);
        REQUIRE(clone.GetSize() == 3);
        REQUIRE(clone[0] == 1);
        REQUIRE(clone[2] == 3);
    }
    SECTION("Cloning an array of arrays copies each one deeply")
    {
        DynamicArray<DynamicArray<i32>> src;
        src.PushBack(DynamicArray<i32>{1, 2});
        src.PushBack(DynamicArray<i32>{3});
        DynamicArray<DynamicArray<i32>> clone = src.Clone();
        REQUIRE(clone.GetSize() == 2);
        REQUIRE(clone[0].GetSize() == 2);
        REQUIRE(clone[0][0] == 1);
        REQUIRE(clone[1][0] == 3);
        REQUIRE(clone[0].GetData() != src[0].GetData());
        // Changing the clone leaves the source alone.
        clone[0][0] = 9;
        REQUIRE(src[0][0] == 1);
    }
    SECTION("Modifying clone does not affect source")
    {
        DynamicArray<i32> src = {1, 2, 3};
        DynamicArray<i32> clone = src.Clone(nullptr);
        clone[0] = 99;
        REQUIRE(src[0] == 1);
        REQUIRE(clone[0] == 99);
    }
    SECTION("Modifying source does not affect clone")
    {
        DynamicArray<i32> src = {1, 2, 3};
        DynamicArray<i32> clone = src.Clone(nullptr);
        src[0] = 99;
        REQUIRE(src[0] == 99);
        REQUIRE(clone[0] == 1);
    }
}

TEST_CASE("Clonable interface for arrays", "[Array]")
{
    struct Data : ClonableBase<Data>
    {
        DynamicArray<StringUtf8> a;
        i32 b = 0;

        OPAL_CLONE_FIELDS(a, b);
    };

    Data first;
    first.a = {"Hello", "There"};
    first.b = 5;
    Data second = first.Clone();
    REQUIRE(second.a[0] == "Hello");
    REQUIRE(second.a[1] == "There");
    REQUIRE(second.b == 5);
}

TEST_CASE("Clonable deep copy for arrays", "[Array]")
{
    struct Data : ClonableBase<Data>
    {
        DynamicArray<StringUtf8> a;
        i32 b = 0;

        OPAL_CLONE_FIELDS(a, b);
    };

    Data original;
    original.a = {"Hello", "There"};
    original.b = 5;
    Data cloned = original.Clone();

    // Mutating the original should not affect the clone.
    original.a[0] = "Changed";
    original.b = 99;

    REQUIRE(cloned.a[0] == "Hello");
    REQUIRE(cloned.a[1] == "There");
    REQUIRE(cloned.b == 5);
    REQUIRE(original.a[0] == "Changed");
    REQUIRE(original.b == 99);
}

TEST_CASE("Clonable with user-defined constructor for arrays", "[Array]")
{
    struct Data : ClonableBase<Data>
    {
        DynamicArray<StringUtf8> items;
        i32 count = 0;

        Data() = default;
        explicit Data(DynamicArray<StringUtf8> in_items)
            : items(std::move(in_items)), count(static_cast<i32>(this->items.GetSize()))
        {
        }

        OPAL_CLONE_FIELDS(items, count);
    };

    Data original(DynamicArray<StringUtf8>{"A", "B", "C"});
    Data cloned = original.Clone();
    REQUIRE(cloned.items[0] == "A");
    REQUIRE(cloned.items[1] == "B");
    REQUIRE(cloned.items[2] == "C");
    REQUIRE(cloned.count == 3);

    // Verify deep copy.
    original.items[0] = "Changed";
    REQUIRE(cloned.items[0] == "A");
}


TEST_CASE("Create a dynamic array without throwing", "[DynamicArray]")
{
    SECTION("Count")
    {
        auto array = DynamicArray<i32>::Create(3);
        REQUIRE(array.HasValue());
        REQUIRE(array.GetValue().GetSize() == 3);
        REQUIRE(array.GetValue()[0] == 0);
    }
    SECTION("Count and value")
    {
        auto array = DynamicArray<i32>::Create(3, 42);
        REQUIRE(array.HasValue());
        REQUIRE(array.GetValue().GetSize() == 3);
        REQUIRE(array.GetValue()[2] == 42);
    }
    SECTION("Data and count")
    {
        const i32 data[] = {1, 2, 3};
        auto array = DynamicArray<i32>::Create(data, 3);
        REQUIRE(array.HasValue());
        REQUIRE(array.GetValue().GetSize() == 3);
        REQUIRE(array.GetValue()[1] == 2);
    }
    SECTION("Initializer list")
    {
        auto array = DynamicArray<i32>::Create({1, 2, 3});
        REQUIRE(array.HasValue());
        REQUIRE(array.GetValue().GetSize() == 3);
        REQUIRE(array.GetValue()[2] == 3);
    }
    SECTION("Zero count allocates nothing, even from a null allocator")
    {
        NullAllocator null_allocator;
        auto array = DynamicArray<i32>::Create(size_t{0}, &null_allocator);
        REQUIRE(array.HasValue());
        REQUIRE(array.GetValue().IsEmpty());
    }
}

TEST_CASE("Create a dynamic array out of memory", "[DynamicArray]")
{
    NullAllocator null_allocator;

    SECTION("Count")
    {
        auto array = DynamicArray<i32>::Create(3, &null_allocator);
        REQUIRE_FALSE(array.HasValue());
        REQUIRE(array.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Count and value")
    {
        auto array = DynamicArray<i32>::Create(3, 42, &null_allocator);
        REQUIRE_FALSE(array.HasValue());
        REQUIRE(array.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Data and count")
    {
        const i32 data[] = {1, 2, 3};
        auto array = DynamicArray<i32>::Create(data, 3, &null_allocator);
        REQUIRE_FALSE(array.HasValue());
        REQUIRE(array.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Initializer list")
    {
        auto array = DynamicArray<i32>::Create({1, 2, 3}, &null_allocator);
        REQUIRE_FALSE(array.HasValue());
        REQUIRE(array.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("The throwing constructor still throws")
    {
        REQUIRE_THROWS_AS(DynamicArray<i32>(3, &null_allocator), OutOfMemoryException);
    }
}

TEST_CASE("TryClone a dynamic array", "[DynamicArray]")
{
    DynamicArray<i32> source = {1, 2, 3};

    SECTION("Copies the elements")
    {
        auto clone = source.TryClone();
        REQUIRE(clone.HasValue());
        REQUIRE(clone.GetValue().GetSize() == 3);
        REQUIRE(clone.GetValue()[1] == 2);

        source[1] = 99;
        REQUIRE(clone.GetValue()[1] == 2);
    }
    SECTION("Reports a failed allocation instead of throwing")
    {
        NullAllocator null_allocator;
        auto clone = source.TryClone(&null_allocator);
        REQUIRE_FALSE(clone.HasValue());
        REQUIRE(clone.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Clone still throws for the same input")
    {
        NullAllocator null_allocator;
        REQUIRE_THROWS_AS(source.Clone(&null_allocator), OutOfMemoryException);
    }
}

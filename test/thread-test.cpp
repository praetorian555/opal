#include "test-helpers.h"

#include "opal/container/shared-ptr.h"
#include "opal/rng.h"
#include "opal/thread.h"

using namespace Opal;

TEST_CASE("Create a thread", "[Thread]")
{
    int n = 5;
    ThreadHandle handle = CreateThread(
        [](int nn)
        {
            REQUIRE(nn == 5);
            nn++;
        },
        n);

    JoinThread(handle);
    REQUIRE(n == 5);

    ThreadHandle out_handle;
    handle = CreateThread(
        [](int& nn, ThreadHandle& inner_handle)
        {
            REQUIRE(nn == 5);
            nn++;
            inner_handle = GetCurrentThreadHandle();
        },
        Ref(n), Ref(out_handle));

    JoinThread(handle);
    REQUIRE(n == 6);
    REQUIRE(out_handle == handle);
    REQUIRE(GetCurrentThreadHandle() != out_handle);
}

TEST_CASE("SPSC queue basic Push and Pop", "[Thread]")
{
    DynamicArray<i64> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    QueueSPSC<i64> queue(32);

    const ThreadHandle handle = CreateThread(
        [](QueueSPSC<i64>& in_queue, Opal::DynamicArray<i64>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const i64 value = in_data[count];
                const i64 queued_value = in_queue.Pop();
                REQUIRE(queued_value == value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    for (const i64& value : data)
    {
        queue.Push(value);
    }
    JoinThread(handle);
}

TEST_CASE("SPSC queue Push with move", "[Thread]")
{
    struct Data
    {
        i64 value = 0;
        Data() = default;
        Data(i64 v) : value(v) {}

        Data(const Data& other) : value(other.value) {}

        Data& operator=(const Data& other)
        {
            value = other.value;
            return *this;
        }

        Data(Data&& other) : value(other.value) { other.value = 0; }

        Data& operator=(Data&& other)
        {
            value = other.value;
            other.value = 0;
            return *this;
        }
    };

    DynamicArray<Data> data;
    constexpr size_t k_capacity = 1024;
    Opal::RNG rng;
    for (size_t i = 0; i < k_capacity; ++i)
    {
        i64 value = static_cast<i64>(rng.RandomI32()) << 32 | rng.RandomI32();
        data.PushBack(value);
    }
    QueueSPSC<Data> queue(32);

    const ThreadHandle handle = CreateThread(
        [](QueueSPSC<Data>& in_queue, Opal::DynamicArray<Data>& in_data)
        {
            size_t count = 0;
            while (count < k_capacity)
            {
                const Data value = in_data[count];
                const Data queued_value = in_queue.Pop();
                REQUIRE(queued_value.value == value.value);
                count++;
            }
        },
        Ref(queue), Ref(data));

    const DynamicArray<Data> data_for_init = data;
    for (Data& value : data_for_init)
    {
        queue.Push(Move(value));
        REQUIRE(value.value == 0);
    }
    JoinThread(handle);
}

TEST_CASE("SPSC queue Push with Emplace", "[Thread]")
{
    struct Data
    {
        i32 a = 0;
        f32 b = 0.0f;
        Data() = default;
        Data(i32 aa, f32 bb) : a(aa), b(bb) {}
    };

    QueueSPSC<Data> queue(32);
    queue.PushWithEmplace(5, 5.0f);
    Data data = queue.Pop();
    REQUIRE(data.a == 5);
    REQUIRE(data.b == 5.0f);
}

TEST_CASE("Shared pointer", "[Thread]")
{
    SECTION("Creation and cloning")
    {
        SharedPtr<i32> ptr(GetDefaultAllocator(), 5);
        REQUIRE(5 == *ptr.Get());
        {
            SharedPtr<i32> ptr2 = ptr.Clone();
            REQUIRE(5 == *ptr2.Get());
            REQUIRE(ptr == ptr2);
        }
        REQUIRE(ptr.IsValid());
        REQUIRE(5 == *ptr.Get());
    }
    SECTION("Moving")
    {
        SharedPtr<i32> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32> ptr2 = Move(ptr);
        REQUIRE(ptr2.IsValid());
        REQUIRE(5 == *ptr2.Get());
        REQUIRE(!ptr.IsValid());
        SharedPtr<i32> ptr3(GetDefaultAllocator(), 10);
        ptr3 = Move(ptr2);
        REQUIRE(!ptr2.IsValid());
        REQUIRE(ptr3.IsValid());
        REQUIRE(5 == *ptr3.Get());
    }
}

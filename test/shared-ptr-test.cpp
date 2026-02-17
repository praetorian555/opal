#include "test-helpers.h"

#include "opal/container/shared-ptr.h"
#include "opal/exceptions.h"

using namespace Opal;

struct Base
{
    virtual ~Base() = default;
    i32 value = 0;
};

struct Derived : public Base
{
    i32 extra = 0;
};

TEMPLATE_TEST_CASE("SharedPtr default construction", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr;
    REQUIRE_FALSE(ptr.IsValid());
    REQUIRE(ptr.Get() == nullptr);
}

TEMPLATE_TEST_CASE("SharedPtr construction with args", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 42);
    REQUIRE(ptr.IsValid());
    REQUIRE(*ptr.Get() == 42);
}

TEMPLATE_TEST_CASE("SharedPtr construction with nullptr allocator", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr(nullptr, 42);
    REQUIRE(ptr.IsValid());
    REQUIRE(*ptr.Get() == 42);
}

TEMPLATE_TEST_CASE("SharedPtr construction with raw pointer", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("Valid pointer")
    {
        i32* raw = New<i32>(GetDefaultAllocator(), 10);
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), raw);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr.Get() == 10);
    }
    SECTION("Null pointer")
    {
        const SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), static_cast<i32*>(nullptr));
        REQUIRE_FALSE(ptr.IsValid());
    }
}

TEMPLATE_TEST_CASE("SharedPtr move construction", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy Policy = TestType::value;

    SharedPtr<i32, Policy> ptr(GetDefaultAllocator(), 5);
    SharedPtr<i32, Policy> moved(std::move(ptr));
    REQUIRE(moved.IsValid());
    REQUIRE(*moved.Get() == 5);
    REQUIRE_FALSE(ptr.IsValid());
}

TEMPLATE_TEST_CASE("SharedPtr converting move construction", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<Derived, k_policy> derived(GetDefaultAllocator());
    derived->value = 7;
    derived->extra = 13;
    SharedPtr<Base, k_policy> base(std::move(derived));
    REQUIRE(base.IsValid());
    REQUIRE(base->value == 7);
    REQUIRE_FALSE(derived.IsValid());
}

TEMPLATE_TEST_CASE("SharedPtr move assignment", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("std::move into default constructed")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32, k_policy> other;
        other = std::move(ptr);
        REQUIRE(other.IsValid());
        REQUIRE(*other.Get() == 5);
        REQUIRE_FALSE(ptr.IsValid());
    }
    SECTION("std::move into valid ptr replaces value")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32, k_policy> other(GetDefaultAllocator(), 10);
        other = std::move(ptr);
        REQUIRE(other.IsValid());
        REQUIRE(*other.Get() == 5);
        REQUIRE_FALSE(ptr.IsValid());
    }
    SECTION("Self move assignment")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32, k_policy>& ref = ptr;
        ptr = std::move(ref);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr.Get() == 5);
    }
}

TEMPLATE_TEST_CASE("SharedPtr arrow operator", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<Base, k_policy> ptr(GetDefaultAllocator());
    ptr->value = 99;
    REQUIRE(ptr->value == 99);

    const SharedPtr<Base, k_policy>& cref = ptr;
    REQUIRE(cref->value == 99);
}

TEMPLATE_TEST_CASE("SharedPtr Clone", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("Clone shares data")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        SharedPtr<i32, k_policy> clone = ptr.Clone();
        REQUIRE(clone.IsValid());
        REQUIRE(*clone.Get() == 5);
        REQUIRE(ptr.Get() == clone.Get());
        REQUIRE(ptr == clone);
    }
    SECTION("Original valid after clone destroyed")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        {
            const SharedPtr<i32, k_policy> clone = ptr.Clone();
            REQUIRE(clone.IsValid());
        }
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr.Get() == 5);
    }
    SECTION("Clone of invalid ptr")
    {
        const SharedPtr<i32, k_policy> ptr;
        const SharedPtr<i32, k_policy> clone = ptr.Clone();
        REQUIRE_FALSE(clone.IsValid());
    }
    SECTION("Multiple clones")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 42);
        SharedPtr<i32, k_policy> clone1 = ptr.Clone();
        SharedPtr<i32, k_policy> clone2 = ptr.Clone();
        SharedPtr<i32, k_policy> clone3 = clone1.Clone();
        REQUIRE(ptr.Get() == clone1.Get());
        REQUIRE(ptr.Get() == clone2.Get());
        REQUIRE(ptr.Get() == clone3.Get());
        ptr.Reset();
        clone1.Reset();
        clone2.Reset();
        REQUIRE(clone3.IsValid());
        REQUIRE(*clone3.Get() == 42);
    }
}

TEMPLATE_TEST_CASE("SharedPtr Reset", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("Reset valid ptr")
    {
        SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
        ptr.Reset();
        REQUIRE_FALSE(ptr.IsValid());
        REQUIRE(ptr.Get() == nullptr);
    }
    SECTION("Reset invalid ptr")
    {
        SharedPtr<i32, k_policy> ptr;
        ptr.Reset();
        REQUIRE_FALSE(ptr.IsValid());
    }
}

TEMPLATE_TEST_CASE("SharedPtr IsValid", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    const SharedPtr<i32, k_policy> invalid;
    REQUIRE_FALSE(invalid.IsValid());

    const SharedPtr<i32, k_policy> valid(GetDefaultAllocator(), 1);
    REQUIRE(valid.IsValid());
}

TEMPLATE_TEST_CASE("SharedPtr equality operator", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 5);
    SharedPtr<i32, k_policy> clone = ptr.Clone();
    SharedPtr<i32, k_policy> other(GetDefaultAllocator(), 5);

    REQUIRE(ptr == clone);
    REQUIRE_FALSE(ptr == other);
}

TEMPLATE_TEST_CASE("SharedPtr Get", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 7);
    REQUIRE(ptr.Get() != nullptr);
    REQUIRE(*ptr.Get() == 7);

    const SharedPtr<i32, k_policy>& cref = ptr;
    REQUIRE(cref.Get() != nullptr);
    REQUIRE(*cref.Get() == 7);
}

TEMPLATE_TEST_CASE("SharedPtr GetRef", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SharedPtr<i32, k_policy> ptr(GetDefaultAllocator(), 11);

    const Ref<i32> ref = ptr.GetRef();
    REQUIRE(ref.IsValid());
    REQUIRE(ref.Get() == 11);
    REQUIRE(ref.GetPtr() == ptr.Get());

    const SharedPtr<i32, k_policy>& cref = ptr;
    const Ref<const i32> const_ref = cref.GetRef();
    REQUIRE(const_ref.IsValid());
    REQUIRE(const_ref.Get() == 11);
    REQUIRE(const_ref.GetPtr() == ptr.Get());
}

TEMPLATE_TEST_CASE("SharedPtr destructor releases resources", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    static i32 s_destructor_count = 0;

    struct Tracked
    {
        ~Tracked() { ++s_destructor_count; }
    };

    {
        const SharedPtr<Tracked, k_policy> ptr(GetDefaultAllocator());
        REQUIRE(s_destructor_count == 0);
    }
    REQUIRE(s_destructor_count == 1);
}

TEMPLATE_TEST_CASE("SharedPtr clone prevents premature destruction", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    static i32 s_destructor_count = 0;

    struct Tracked
    {
        ~Tracked() { ++s_destructor_count; }
    };

    SharedPtr<Tracked, k_policy> ptr(GetDefaultAllocator());
    {
        const SharedPtr<Tracked, k_policy> clone = ptr.Clone();
        REQUIRE(s_destructor_count == 0);
    }
    REQUIRE(s_destructor_count == 0);
    ptr.Reset();
    REQUIRE(s_destructor_count == 1);
}

TEST_CASE("SharedPtr ThreadSafe policy throws when allocator is not thread-safe", "[SharedPtr]")
{
    LinearAllocator allocator("NonThreadSafe");

    SECTION("Construction with args")
    {
        REQUIRE_THROWS_AS((SharedPtr<i32, ThreadingPolicy::ThreadSafe>(&allocator, 42)), InvalidArgumentException);
    }
    SECTION("Construction with raw pointer")
    {
        i32* raw = New<i32>(GetDefaultAllocator(), 10);
        REQUIRE_THROWS_AS((SharedPtr<i32, ThreadingPolicy::ThreadSafe>(&allocator, raw)), InvalidArgumentException);
        Delete(GetDefaultAllocator(), raw);
    }
}

TEST_CASE("SharedPtr SingleThread policy does not throw when allocator is not thread-safe", "[SharedPtr]")
{
    LinearAllocator allocator("NonThreadSafe");

    SECTION("Construction with args")
    {
        REQUIRE_NOTHROW(SharedPtr<i32, ThreadingPolicy::SingleThread>(&allocator, 42));
    }
    SECTION("Construction with raw pointer")
    {
        i32* raw = New<i32>(&allocator, 10);
        REQUIRE_NOTHROW(SharedPtr<i32, ThreadingPolicy::SingleThread>(&allocator, raw));
    }
}

TEMPLATE_TEST_CASE("MakeShared with same type", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("Default construction")
    {
        auto ptr = MakeShared<Base, Base, k_policy>(nullptr);
        REQUIRE(ptr.IsValid());
        REQUIRE(ptr->value == 0);
    }

    SECTION("With nullptr allocator")
    {
        auto ptr = MakeShared<i32, i32, k_policy>(nullptr, 42);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr.Get() == 42);
    }

    SECTION("With explicit allocator")
    {
        auto ptr = MakeShared<i32, i32, k_policy>(GetDefaultAllocator(), 99);
        REQUIRE(ptr.IsValid());
        REQUIRE(*ptr.Get() == 99);
    }
}

TEMPLATE_TEST_CASE("MakeShared with derived type", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    SECTION("Returns SharedPtr<Base> holding a Derived")
    {
        auto ptr = MakeShared<Base, Derived, k_policy>(nullptr);
        REQUIRE(ptr.IsValid());
        ptr->value = 42;
        REQUIRE(ptr->value == 42);
    }

    SECTION("Derived members accessible via downcast")
    {
        auto ptr = MakeShared<Base, Derived, k_policy>(nullptr);
        auto* derived = static_cast<Derived*>(ptr.Get());
        derived->extra = 77;
        REQUIRE(derived->extra == 77);
        REQUIRE(derived->value == 0);
    }

    SECTION("Clone shares the same object")
    {
        auto ptr = MakeShared<Base, Derived, k_policy>(nullptr);
        ptr->value = 10;
        auto clone = ptr.Clone();
        REQUIRE(clone.IsValid());
        REQUIRE(clone->value == 10);
        REQUIRE(ptr.Get() == clone.Get());
    }
}

TEMPLATE_TEST_CASE("MakeShared with constructor arguments", "[SharedPtr]",
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::ThreadSafe>),
                    (std::integral_constant<ThreadingPolicy, ThreadingPolicy::SingleThread>))
{
    constexpr ThreadingPolicy k_policy = TestType::value;

    struct Widget
    {
        i32 a;
        i32 b;
        Widget(i32 x, i32 y) : a(x), b(y) {}
    };

    auto ptr = MakeShared<Widget, Widget, k_policy>(nullptr, 3, 7);
    REQUIRE(ptr.IsValid());
    REQUIRE(ptr->a == 3);
    REQUIRE(ptr->b == 7);
}

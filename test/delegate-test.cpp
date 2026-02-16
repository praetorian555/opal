#include "test-helpers.h"

#include "opal/delegate.h"

using namespace Opal;

TEST_CASE("Delegate default state is unbound", "[Delegate]")
{
    Delegate<i32(i32)> delegate;
    REQUIRE_FALSE(delegate.IsBound());
}

TEST_CASE("Delegate bind and execute", "[Delegate]")
{
    Delegate<i32(i32, i32)> delegate;
    delegate.Bind([](i32 a, i32 b) { return a + b; });
    REQUIRE(delegate.IsBound());
    REQUIRE(delegate.Execute(3, 4) == 7);
}

TEST_CASE("Delegate unbind", "[Delegate]")
{
    Delegate<i32()> delegate;
    delegate.Bind([]() { return 42; });
    REQUIRE(delegate.IsBound());

    delegate.Unbind();
    REQUIRE_FALSE(delegate.IsBound());
}

TEST_CASE("Delegate execute when unbound returns default", "[Delegate]")
{
    Delegate<i32()> delegate;
    REQUIRE(delegate.Execute() == 0);

    Delegate<f32(i32)> float_delegate;
    REQUIRE(float_delegate.Execute(1) == 0.0f);
}

TEST_CASE("Delegate rebind replaces callable", "[Delegate]")
{
    Delegate<i32()> delegate;
    delegate.Bind([]() { return 1; });
    REQUIRE(delegate.Execute() == 1);

    delegate.Bind([]() { return 2; });
    REQUIRE(delegate.Execute() == 2);
}

TEST_CASE("Delegate with void return type", "[Delegate]")
{
    i32 counter = 0;
    Delegate<void()> delegate;
    delegate.Bind([&counter]() { counter++; });
    delegate.Execute();
    REQUIRE(counter == 1);
}

TEST_CASE("Delegate with reference arguments", "[Delegate]")
{
    Delegate<void(i32&)> delegate;
    delegate.Bind([](i32& val) { val = 99; });

    i32 value = 0;
    delegate.Execute(value);
    REQUIRE(value == 99);
}

// MultiDelegate tests

TEST_CASE("MultiDelegate default state has no bindings", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    REQUIRE_FALSE(delegate.IsAnyBound());
}

TEST_CASE("MultiDelegate bind returns valid handle", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    DelegateHandle handle = delegate.Bind([]() {});
    REQUIRE(handle != k_invalid_delegate_handle);
    REQUIRE(delegate.IsBound(handle));
    REQUIRE(delegate.IsAnyBound());
}

TEST_CASE("MultiDelegate bind multiple callables", "[MultiDelegate]")
{
    i32 sum = 0;
    MultiDelegate<void(i32)> delegate;
    DelegateHandle h1 = delegate.Bind([&sum](i32 val) { sum += val; });
    DelegateHandle h2 = delegate.Bind([&sum](i32 val) { sum += val * 2; });

    REQUIRE(h1 != h2);
    delegate.Execute(5);
    REQUIRE(sum == 15);  // 5 + 10
}

TEST_CASE("MultiDelegate unbind by handle", "[MultiDelegate]")
{
    i32 call_count = 0;
    MultiDelegate<void()> delegate;
    DelegateHandle h1 = delegate.Bind([&call_count]() { call_count++; });
    DelegateHandle h2 = delegate.Bind([&call_count]() { call_count++; });

    delegate.Unbind(h1);
    REQUIRE_FALSE(delegate.IsBound(h1));
    REQUIRE(delegate.IsBound(h2));
    REQUIRE(delegate.IsAnyBound());

    delegate.Execute();
    REQUIRE(call_count == 1);
}

TEST_CASE("MultiDelegate unbind all leaves empty", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    DelegateHandle h1 = delegate.Bind([]() {});
    DelegateHandle h2 = delegate.Bind([]() {});

    delegate.Unbind(h1);
    delegate.Unbind(h2);
    REQUIRE_FALSE(delegate.IsAnyBound());
}

TEST_CASE("MultiDelegate unbind with invalid handle is no-op", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    delegate.Bind([]() {});
    delegate.Unbind(k_invalid_delegate_handle);
    REQUIRE(delegate.IsAnyBound());
}

TEST_CASE("MultiDelegate unbind with unknown handle is no-op", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    DelegateHandle handle = delegate.Bind([]() {});
    delegate.Unbind(handle + 100);
    REQUIRE(delegate.IsBound(handle));
}

TEST_CASE("MultiDelegate IsBound returns false for unbound handle", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    DelegateHandle handle = delegate.Bind([]() {});
    delegate.Unbind(handle);
    REQUIRE_FALSE(delegate.IsBound(handle));
}

TEST_CASE("MultiDelegate execute with no bindings is no-op", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    delegate.Execute();  // Should not crash
}

TEST_CASE("MultiDelegate with reference arguments", "[MultiDelegate]")
{
    MultiDelegate<void(i32&)> delegate;
    delegate.Bind([](i32& val) { val += 10; });
    delegate.Bind([](i32& val) { val *= 2; });

    i32 value = 5;
    delegate.Execute(value);
    // Both callbacks are called, order depends on iteration over HashMap
    // After both: value should be modified by both callbacks
    REQUIRE(value != 5);
}
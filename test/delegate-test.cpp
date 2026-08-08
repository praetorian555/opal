#include "test-helpers.h"

#include "opal/delegate.h"

using namespace Opal;

namespace
{

struct Listener
{
    i32 value = 0;

    void OnEvent(i32 amount) { value += amount; }
    i32 Read() const { return value; }
};

}  // namespace

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

TEST_CASE("Delegate binds a member function", "[Delegate]")
{
    Listener listener;
    Delegate<void(i32)> delegate;
    delegate.Bind<&Listener::OnEvent>(&listener);

    REQUIRE(delegate.IsBound());
    delegate.Execute(5);
    delegate.Execute(3);
    REQUIRE(listener.value == 8);
}

TEST_CASE("Delegate binds a const member function", "[Delegate]")
{
    Listener listener;
    listener.value = 11;

    Delegate<i32()> delegate;
    delegate.Bind<&Listener::Read>(&listener);
    REQUIRE(delegate.Execute() == 11);
}

TEST_CASE("Delegate holds a move only callable", "[Delegate]")
{
    DynamicArray<i32> owned = {1, 2, 3, 4};
    Delegate<i32()> delegate;
    delegate.Bind([captured = Move(owned)]() { return static_cast<i32>(captured.GetSize()); });

    REQUIRE(delegate.Execute() == 4);
}

TEST_CASE("Delegate rebind replaces a member function binding", "[Delegate]")
{
    Listener listener;
    Delegate<void(i32)> delegate;
    delegate.Bind<&Listener::OnEvent>(&listener);
    delegate.Execute(5);

    i32 other = 0;
    delegate.Bind([&other](i32 amount) { other += amount; });
    delegate.Execute(5);

    REQUIRE(listener.value == 5);
    REQUIRE(other == 5);
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
    REQUIRE(value == 30);  // (5 + 10) * 2, callbacks run in bind order
}

TEST_CASE("MultiDelegate executes in bind order", "[MultiDelegate]")
{
    DynamicArray<i32> order;
    MultiDelegate<void()> delegate;
    for (i32 i = 0; i < 8; ++i)
    {
        delegate.Bind([&order, i]() { order.PushBack(i); });
    }
    delegate.Execute();

    REQUIRE(order == DynamicArray<i32>({0, 1, 2, 3, 4, 5, 6, 7}));
}

TEST_CASE("MultiDelegate unbind preserves the order of the rest", "[MultiDelegate]")
{
    DynamicArray<i32> order;
    MultiDelegate<void()> delegate;
    DelegateHandle h0 = delegate.Bind([&order]() { order.PushBack(0); });
    DelegateHandle h1 = delegate.Bind([&order]() { order.PushBack(1); });
    DelegateHandle h2 = delegate.Bind([&order]() { order.PushBack(2); });

    delegate.Unbind(h1);
    delegate.Execute();

    REQUIRE(order == DynamicArray<i32>({0, 2}));
    REQUIRE(delegate.IsBound(h0));
    REQUIRE_FALSE(delegate.IsBound(h1));
    REQUIRE(delegate.IsBound(h2));
}

TEST_CASE("MultiDelegate binds a member function", "[MultiDelegate]")
{
    Listener first;
    Listener second;

    MultiDelegate<void(i32)> delegate;
    delegate.Bind<&Listener::OnEvent>(&first);
    delegate.Bind<&Listener::OnEvent>(&second);

    delegate.Execute(4);
    REQUIRE(first.value == 4);
    REQUIRE(second.value == 4);
}

TEST_CASE("MultiDelegate handles are not reused after unbind", "[MultiDelegate]")
{
    MultiDelegate<void()> delegate;
    DelegateHandle first = delegate.Bind([]() {});
    delegate.Unbind(first);
    DelegateHandle second = delegate.Bind([]() {});

    REQUIRE(second != first);
    REQUIRE_FALSE(delegate.IsBound(first));
    REQUIRE(delegate.IsBound(second));
}
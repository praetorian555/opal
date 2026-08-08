#pragma once

#include "opal/container/dynamic-array.h"
#include "opal/container/function.h"
#include "opal/type-traits.h"

namespace Opal
{

using DelegateHandle = i32;
static constexpr DelegateHandle k_invalid_delegate_handle = -1;

template <typename Signature, u64 k_capacity = k_default_function_capacity>
struct Delegate;

/**
 * Single-cast delegate that wraps a callable with signature ReturnType(Args...).
 * Only one callable can be bound at a time. Executing an unbound delegate returns a default-constructed ReturnType.
 * @tparam Signature Signature of the callable, written as ReturnType(Args...).
 * @tparam k_capacity Number of bytes reserved for the bound callable. A callable that does not fit is a compile error.
 */
template <typename ReturnType, typename... Args, u64 k_capacity>
struct Delegate<ReturnType(Args...), k_capacity>
{
    using FunctionType = Function<ReturnType(Args...), k_capacity>;

    /** Bind a callable to this delegate, replacing any previously bound callable. */
    template <typename Callable>
    void Bind(Callable&& callable)
    {
        m_functor = FunctionType(Forward<Callable>(callable));
    }

    /**
     * Bind a member function to this delegate, replacing any previously bound callable. The instance is not owned and
     * must outlive the binding.
     * @tparam k_method Pointer to the member function to call.
     * @param instance The object to call it on.
     */
    template <auto k_method, typename Instance>
    void Bind(Instance* instance)
    {
        m_functor = FunctionType::template FromMethod<k_method>(instance);
    }

    /** Unbind the current callable. */
    void Unbind() { m_functor.Reset(); }

    /** Returns true if a callable is currently bound. */
    [[nodiscard]] bool IsBound() const { return m_functor.IsBound(); }

    /**
     * Execute the bound callable with the given arguments.
     * If no callable is bound, returns a default-constructed ReturnType.
     */
    template <typename... ExecArgs>
    ReturnType Execute(ExecArgs&&... arguments)
    {
        if constexpr (k_is_void_value<ReturnType>)
        {
            if (m_functor.IsBound())
            {
                m_functor(Forward<ExecArgs>(arguments)...);
            }
        }
        else
        {
            if (m_functor.IsBound())
            {
                return m_functor(Forward<ExecArgs>(arguments)...);
            }

            return ReturnType{};
        }
    }

private:
    FunctionType m_functor;
};

template <typename Signature, u64 k_capacity = k_default_function_capacity>
struct MultiDelegate;

/**
 * Multi-cast delegate that supports binding multiple callables with signature void(Args...).
 * Each bound callable is identified by a DelegateHandle, which can be used to unbind it later.
 * Executing the delegate invokes all bound callables, in the order they were bound.
 * @tparam Signature Signature of the callables, written as void(Args...).
 * @tparam k_capacity Number of bytes reserved for each bound callable. A callable that does not fit is a compile error.
 */
template <typename... Args, u64 k_capacity>
struct MultiDelegate<void(Args...), k_capacity>
{
    using FunctionType = Function<void(Args...), k_capacity>;

    explicit MultiDelegate(AllocatorBase* allocator = nullptr) : m_bindings(allocator) {}

    /**
     * Bind a callable and return a handle that can be used to unbind it later.
     * @return The handle, or k_invalid_delegate_handle if the binding could not be stored.
     */
    template <typename Callable>
    DelegateHandle Bind(Callable&& callable)
    {
        return Add(FunctionType(Forward<Callable>(callable)));
    }

    /**
     * Bind a member function and return a handle that can be used to unbind it later. The instance is not owned and
     * must outlive the binding.
     * @tparam k_method Pointer to the member function to call.
     * @param instance The object to call it on.
     * @return The handle, or k_invalid_delegate_handle if the binding could not be stored.
     */
    template <auto k_method, typename Instance>
    DelegateHandle Bind(Instance* instance)
    {
        return Add(FunctionType::template FromMethod<k_method>(instance));
    }

    /** Unbind the callable associated with the given handle. No-op if the handle is invalid or not found. */
    void Unbind(DelegateHandle handle)
    {
        if (handle == k_invalid_delegate_handle)
        {
            return;
        }

        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it)
        {
            if (it->handle == handle)
            {
                m_bindings.Erase(it);
                return;
            }
        }
    }

    /** Returns true if the callable associated with the given handle is still bound. */
    [[nodiscard]] bool IsBound(DelegateHandle handle) const
    {
        if (handle == k_invalid_delegate_handle)
        {
            return false;
        }

        for (const auto& binding : m_bindings)
        {
            if (binding.handle == handle)
            {
                return true;
            }
        }
        return false;
    }

    /** Returns true if any callable is bound. */
    [[nodiscard]] bool IsAnyBound() const { return !m_bindings.IsEmpty(); }

    /** Execute all bound callables with the given arguments, in the order they were bound. */
    template <typename... ExecArgs>
    void Execute(ExecArgs&&... arguments)
    {
        for (auto& binding : m_bindings)
        {
            binding.function(Forward<ExecArgs>(arguments)...);
        }
    }

private:
    struct Binding
    {
        DelegateHandle handle = k_invalid_delegate_handle;
        FunctionType function;
    };

    DelegateHandle Add(FunctionType&& function)
    {
        const DelegateHandle handle = m_handle_generator;
        if (m_bindings.PushBack(Binding{handle, Move(function)}) != ErrorCode::Success) [[unlikely]]
        {
            return k_invalid_delegate_handle;
        }
        m_handle_generator++;
        return handle;
    }

    DynamicArray<Binding> m_bindings;
    DelegateHandle m_handle_generator = 0;
};

}  // namespace Opal

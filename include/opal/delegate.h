#pragma once

#include <functional>

#include "opal/container/hash-map.h"
#include "opal/type-traits.h"

namespace Opal
{

using DelegateHandle = int;
static constexpr DelegateHandle k_invalid_delegate_handle = -1;

template <typename ReturnType, typename... Args>
struct Delegate;

/**
 * Single-cast delegate that wraps a callable with signature ReturnType(Args...).
 * Only one callable can be bound at a time. Executing an unbound delegate returns a default-constructed ReturnType.
 */
template <typename ReturnType, typename... Args>
struct Delegate<ReturnType(Args...)>
{
    using Function = std::function<ReturnType(Args...)>;

    /** Bind a callable to this delegate, replacing any previously bound callable. */
    void Bind(Function functor) { m_functor = functor; }

    /** Unbind the current callable. */
    void Unbind() { m_functor = nullptr; }

    /** Returns true if a callable is currently bound. */
    [[nodiscard]] bool IsBound() const { return m_functor != nullptr; }

    /**
     * Execute the bound callable with the given arguments.
     * If no callable is bound, returns a default-constructed ReturnType.
     */
    template <typename... ExecArgs>
    ReturnType Execute(ExecArgs&&... arguments)
    {
        if constexpr (k_is_void_value<ReturnType>)
        {
            if (m_functor)
            {
                m_functor(std::forward<ExecArgs>(arguments)...);
            }
        }
        else
        {
            if (m_functor)
            {
                return m_functor(std::forward<ExecArgs>(arguments)...);
            }

            return ReturnType{};
        }
    }

private:
    Function m_functor;
};

template <typename... Args>
struct MultiDelegate;

/**
 * Multi-cast delegate that supports binding multiple callables with signature void(Args...).
 * Each bound callable is identified by a DelegateHandle, which can be used to unbind it later.
 * Executing the delegate invokes all bound callables.
 */
template <typename... Args>
struct MultiDelegate<void(Args...)>
{
    using Function = std::function<void(Args...)>;

    /** Bind a callable and return a handle that can be used to unbind it later. */
    DelegateHandle Bind(Function functor)
    {
        const DelegateHandle handle = m_handle_generator++;
        m_functors.Insert(handle, functor);
        return handle;
    }

    /** Unbind the callable associated with the given handle. No-op if the handle is invalid or not found. */
    void Unbind(DelegateHandle handle)
    {
        if (handle == k_invalid_delegate_handle)
        {
            return;
        }

        m_functors.Erase(handle);
    }

    /** Returns true if the callable associated with the given handle is still bound. */
    [[nodiscard]] bool IsBound(DelegateHandle handle) const
    {
        return m_functors.Find(handle) != m_functors.end();
    }

    /** Returns true if any callable is bound. */
    [[nodiscard]] bool IsAnyBound() const
    {
        return m_functors.GetSize() > 0;
    }

    /** Execute all bound callables with the given arguments. */
    template <typename... ExecArgs>
    void Execute(ExecArgs&&... arguments)
    {
        for (auto& pair : m_functors)
        {
            pair.value(std::forward<ExecArgs>(arguments)...);
        }
    }

private:
    HashMap<DelegateHandle, Function> m_functors;
    DelegateHandle m_handle_generator = 0;
};

}  // namespace Opal

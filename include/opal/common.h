#pragma once

#include "allocator.h"
#include "opal/container/expected.h"
#include "opal/error-codes.h"
#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

template <typename T, u64 N>
u64 GetArraySize(T (&)[N])
{
    return N;
}

/**
 * Swaps the values of two variables. By default it uses a copy constructor to swap the values.
 * @tparam T The type of the variables.
 * @param lhs The first variable.
 * @param rhs The second variable.
 */
template <typename T>
void Swap(T& lhs, T& rhs)
{
    T temp = lhs;
    lhs = rhs;
    rhs = temp;
}

/**
 * Swaps the values of two variables. This version of the function is used when the type is move assignable.
 * @tparam T The type of the variables.
 * @param lhs The first variable.
 * @param rhs The second variable.
 */
template <typename T>
    requires MoveConstructable<T> && MoveAssignable<T>
void Swap(T& lhs, T& rhs)
{
    T temp = Move(lhs);
    lhs = Move(rhs);
    rhs = Move(temp);
}

template <typename T>
    requires ArrayLiteral<T>
void Swap(T& lhs, T& rhs)
{
    for (size_t i = 0; i < GetArraySize(lhs); ++i)
    {
        Swap(lhs[i], rhs[i]);
    }
};

template <typename T>
constexpr T Clone(const T& source, AllocatorBase* allocator = nullptr)
{
    if constexpr (IsPOD<T>)
    {
        return source;
    }
    else
    {
        return source.Clone(allocator);
    }
}

/**
 * Copies a value the way Clone does, reporting a failed allocation instead of throwing it.
 * @param source Value to copy.
 * @param allocator Allocator the copy should use, or nullptr for the one the type picks.
 * @return The copy, or the error the type's TryClone reported. A type that has Clone but no TryClone is cloned with Clone, which
 * can still throw.
 */
template <typename T>
Expected<T, ErrorCode> TryClone(const T& source, AllocatorBase* allocator = nullptr)
{
    if constexpr (IsPOD<T>)
    {
        return Expected<T, ErrorCode>(source);
    }
    else if constexpr (requires { source.TryClone(allocator); })
    {
        return source.TryClone(allocator);
    }
    else
    {
        return Expected<T, ErrorCode>(source.Clone(allocator));
    }
}

}  // namespace Opal

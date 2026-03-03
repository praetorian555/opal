#pragma once

#include "allocator.h"
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
T Clone(const T& source, AllocatorBase* allocator = nullptr)
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


}  // namespace Opal

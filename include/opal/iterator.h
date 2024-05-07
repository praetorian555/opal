#pragma once

#include "opal/types.h"

namespace Opal
{

template <typename Container>
constexpr auto begin(Container& container) -> decltype(container.begin())
{
    return container.begin();
}

template <typename Container>
constexpr auto end(Container& container) -> decltype(container.end())
{
    return container.end();
}

template <typename T, u64 N>
constexpr T* begin(T (&array)[N])
{
    return array;
}

template <typename T, u64 N>
constexpr T* end(T (&array)[N])
{
    return array + N;
}

}  // namespace Opal
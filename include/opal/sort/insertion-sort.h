#pragma once

#include "opal/common.h"
#include "opal/container/array-view.h"
#include "opal/types.h"

namespace Opal
{
namespace Impl
{

/** Sort the inclusive range [lo, hi] of array in place. */
template <typename T>
void InsertionSortRange(ArrayView<T>& array, u64 lo, u64 hi)
{
    for (u64 i = lo + 1; i <= hi; ++i)
    {
        for (u64 j = i; j > lo && array.At(j) < array.At(j - 1); --j)
        {
            Swap(array.At(j - 1), array.At(j));
        }
    }
}

/** Sort the inclusive range [lo, hi] of array in place. Not stable. */
template <typename T>
void InsertionSortRangeImproved(ArrayView<T>& array, u64 lo, u64 hi)
{
    if (hi <= lo)
    {
        return;
    }

    u64 min_index = lo;
    for (u64 i = lo + 1; i <= hi; ++i)
    {
        if (array.At(i) < array.At(min_index))
        {
            min_index = i;
        }
    }
    Swap(array.At(min_index), array.At(lo));

    for (u64 i = lo + 2; i <= hi; ++i)
    {
        T value = Move(array.At(i));
        u64 j = i;
        for (; value < array.At(j - 1); --j)
        {
            array.At(j) = Move(array.At(j - 1));
        }
        array.At(j) = Move(value);
    }
}

}  // namespace Impl

/**
 * Sort the elements in ascending order using insertion sort. This sort is stable and allocates nothing.
 * @tparam T The type of the elements. Must be less-than comparable and swappable.
 * @param in_out_values The elements to sort.
 */
template <typename T>
void InsertionSort(ArrayView<T>& in_out_values)
{
    if (in_out_values.GetSize() < 2)
    {
        return;
    }
    Impl::InsertionSortRange(in_out_values, 0, in_out_values.GetSize() - 1);
}

/**
 * Sort the elements in ascending order using insertion sort. Allocates nothing. Faster than InsertionSort on the same input,
 * but not stable: equal elements may come out in a different order than they went in.
 * @tparam T The type of the elements. Must be less-than comparable, movable and swappable.
 * @param in_out_values The elements to sort.
 */
template <typename T>
void InsertionSortImproved(ArrayView<T>& in_out_values)
{
    if (in_out_values.GetSize() < 2)
    {
        return;
    }
    Impl::InsertionSortRangeImproved(in_out_values, 0, in_out_values.GetSize() - 1);
}

}  // namespace Opal

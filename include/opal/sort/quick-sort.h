#pragma once

#include "opal/assert.h"
#include "opal/common.h"
#include "opal/container/array-view.h"
#include "opal/rng.h"
#include "opal/sort/insertion-sort.h"
#include "opal/types.h"

namespace Opal
{
namespace Impl
{

template <typename T>
void QuickSortShuffle(ArrayView<T>& array, RNG& rng)
{
    OPAL_ASSERT(array.GetSize() <= 0xFFFFFFFFull, "Array too large to shuffle");
    for (u64 i = array.GetSize() - 1; i > 0; --i)
    {
        const u64 j = rng.RandomU32(0, static_cast<u32>(i) + 1);
        Swap(array.At(i), array.At(j));
    }
}

/** Partition the inclusive range [lo, hi] around array[lo] and return the resting index of that element. */
template <typename T>
u64 QuickSortPartition(ArrayView<T>& array, u64 lo, u64 hi)
{
    u64 i = lo;
    u64 j = hi + 1;
    const T pivot = array.At(lo);
    while (true)
    {
        while (array.At(++i) < pivot && i != hi) {}
        while (pivot < array.At(--j) && j != lo) {}
        if (i >= j)
        {
            break;
        }
        Swap(array.At(i), array.At(j));
    }
    Swap(array.At(lo), array.At(j));
    return j;
}

template <typename T>
void QuickSortRange(ArrayView<T>& array, u64 lo, u64 hi)
{
    if (hi <= lo)
    {
        return;
    }
    const u64 j = QuickSortPartition(array, lo, hi);
    if (j > lo)
    {
        QuickSortRange(array, lo, j - 1);
    }
    QuickSortRange(array, j + 1, hi);
}

/**
 * Partition the inclusive range [lo, hi] around the median of its first, middle and last element and return the resting
 * index of that element. Sorting those three first puts a sentinel at each end, so the scans need no bounds check.
 */
template <typename T>
u64 QuickSortPartitionImproved(ArrayView<T>& array, u64 lo, u64 hi)
{
    const u64 mid = lo + (hi - lo) / 2;
    if (array.At(hi) < array.At(lo))
    {
        Swap(array.At(hi), array.At(lo));
    }
    if (array.At(mid) < array.At(lo))
    {
        Swap(array.At(mid), array.At(lo));
    }
    if (array.At(hi) < array.At(mid))
    {
        Swap(array.At(mid), array.At(hi));
    }
    Swap(array.At(mid), array.At(lo));

    u64 i = lo;
    u64 j = hi + 1;
    const T pivot = array.At(lo);
    while (true)
    {
        while (array.At(++i) < pivot) {}
        while (pivot < array.At(--j)) {}
        if (i >= j)
        {
            break;
        }
        Swap(array.At(i), array.At(j));
    }
    Swap(array.At(lo), array.At(j));
    return j;
}

template <typename T>
void QuickSortRangeImproved(ArrayView<T>& array, u64 lo, u64 hi)
{
    constexpr u64 k_small_range_cutoff = 32;
    if (hi <= lo)
    {
        return;
    }
    if (hi - lo <= k_small_range_cutoff)
    {
        InsertionSortRangeImproved(array, lo, hi);
        return;
    }
    const u64 j = QuickSortPartitionImproved(array, lo, hi);
    if (j > lo)
    {
        QuickSortRangeImproved(array, lo, j - 1);
    }
    QuickSortRangeImproved(array, j + 1, hi);
}

/**
 * Partition the inclusive range [lo, hi] into elements smaller than, equal to and larger than array[lo], then recurse on
 * the two outer parts only.
 */
template <typename T>
void QuickSortRangeThreeWay(ArrayView<T>& array, u64 lo, u64 hi)
{
    if (hi <= lo)
    {
        return;
    }
    u64 lt = lo;
    u64 i = lo + 1;
    u64 gt = hi;
    const T pivot = array.At(lo);
    while (i <= gt)
    {
        if (array.At(i) < pivot)
        {
            Swap(array.At(i), array.At(lt));
            ++i;
            ++lt;
        }
        else if (pivot < array.At(i))
        {
            Swap(array.At(i), array.At(gt));
            --gt;
        }
        else
        {
            ++i;
        }
    }
    if (lt > lo)
    {
        QuickSortRangeThreeWay(array, lo, lt - 1);
    }
    QuickSortRangeThreeWay(array, gt + 1, hi);
}

}  // namespace Impl

/**
 * Sort the elements in ascending order using quick sort. Allocates nothing and is not stable. The elements are shuffled
 * first, so an already ordered input does not hit the quadratic worst case.
 * @tparam T The type of the elements. Must be less-than comparable, copy constructible and swappable.
 * @param in_out_values The elements to sort.
 * @param rng The generator to draw the shuffle from. If nullptr, a default seeded generator is used, which makes the
 *            shuffle, and therefore the whole sort, deterministic across runs.
 */
template <typename T>
void QuickSort(ArrayView<T>& in_out_values, RNG* rng = nullptr)
{
    if (in_out_values.GetSize() < 2)
    {
        return;
    }
    RNG default_rng;
    Impl::QuickSortShuffle(in_out_values, rng == nullptr ? default_rng : *rng);
    Impl::QuickSortRange(in_out_values, 0, in_out_values.GetSize() - 1);
}

/**
 * Sort the elements in ascending order using quick sort. Allocates nothing and is not stable. Faster than QuickSort on
 * the same input, and needs no shuffle: the pivot is chosen so that an already ordered input is not a worst case.
 * @tparam T The type of the elements. Must be less-than comparable, copy constructible, movable and swappable.
 * @param in_out_values The elements to sort.
 */
template <typename T>
void QuickSortImproved(ArrayView<T>& in_out_values)
{
    if (in_out_values.GetSize() < 2)
    {
        return;
    }
    Impl::QuickSortRangeImproved(in_out_values, 0, in_out_values.GetSize() - 1);
}

/**
 * Sort the elements in ascending order using three way quick sort. Allocates nothing and is not stable. Preferred when
 * the input holds few distinct values, since elements equal to the pivot are not sorted again.
 * @tparam T The type of the elements. Must be less-than comparable, copy constructible and swappable.
 * @param in_out_values The elements to sort.
 * @param rng The generator to draw the shuffle from. If nullptr, a default seeded generator is used, which makes the
 *            shuffle, and therefore the whole sort, deterministic across runs.
 */
template <typename T>
void QuickSortThreeWay(ArrayView<T>& in_out_values, RNG* rng = nullptr)
{
    if (in_out_values.GetSize() < 2)
    {
        return;
    }
    RNG default_rng;
    Impl::QuickSortShuffle(in_out_values, rng == nullptr ? default_rng : *rng);
    Impl::QuickSortRangeThreeWay(in_out_values, 0, in_out_values.GetSize() - 1);
}

}  // namespace Opal

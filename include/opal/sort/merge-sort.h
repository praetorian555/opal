#pragma once

#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/error-codes.h"
#include "opal/sort/insertion-sort.h"
#include "opal/types.h"

namespace Opal
{
namespace Impl
{

/** Merge the sorted inclusive ranges [lo, mid] and [mid + 1, hi] of dst into dst, using src as working space. */
template <typename T>
void MergeSortMerge(ArrayView<T>& dst, ArrayView<T>& src, u64 lo, u64 mid, u64 hi)
{
    for (u64 k = lo; k <= hi; ++k)
    {
        src.At(k) = dst.At(k);
    }

    u64 i = lo;
    u64 j = mid + 1;
    for (u64 k = lo; k <= hi; ++k)
    {
        if (i > mid)
        {
            dst.At(k) = src.At(j++);
        }
        else if (j > hi)
        {
            dst.At(k) = src.At(i++);
        }
        else if (src.At(j) < src.At(i))
        {
            dst.At(k) = src.At(j++);
        }
        else
        {
            dst.At(k) = src.At(i++);
        }
    }
}

template <typename T>
void MergeSortRange(ArrayView<T>& array, ArrayView<T>& aux, u64 lo, u64 hi)
{
    if (hi <= lo)
    {
        return;
    }
    const u64 mid = lo + (hi - lo) / 2;
    MergeSortRange(array, aux, lo, mid);
    MergeSortRange(array, aux, mid + 1, hi);
    MergeSortMerge(array, aux, lo, mid, hi);
}

/** Merge the sorted inclusive ranges [lo, mid] and [mid + 1, hi] of src into dst. */
template <typename T>
void MergeSortMergeImproved(ArrayView<T>& dst, ArrayView<T>& src, u64 lo, u64 mid, u64 hi)
{
    u64 i = lo;
    u64 j = mid + 1;
    for (u64 k = lo; k <= hi; ++k)
    {
        if (i > mid)
        {
            dst.At(k) = src.At(j++);
        }
        else if (j > hi)
        {
            dst.At(k) = src.At(i++);
        }
        else if (src.At(j) < src.At(i))
        {
            dst.At(k) = src.At(j++);
        }
        else
        {
            dst.At(k) = src.At(i++);
        }
    }
}

/**
 * Sort the inclusive range [lo, hi] into dst, reading from src. The two views swap roles on each level, so both must
 * hold the same elements when the recursion starts.
 */
template <typename T>
void MergeSortRangeImproved(ArrayView<T>& dst, ArrayView<T>& src, u64 lo, u64 hi)
{
    constexpr u64 k_small_range_cutoff = 15;
    if (hi <= lo || hi - lo <= k_small_range_cutoff)
    {
        InsertionSortRangeImproved(dst, lo, hi);
        return;
    }
    const u64 mid = lo + (hi - lo) / 2;
    MergeSortRangeImproved(src, dst, lo, mid);
    MergeSortRangeImproved(src, dst, mid + 1, hi);
    if (src.At(mid + 1) < src.At(mid))
    {
        MergeSortMergeImproved(dst, src, lo, mid, hi);
    }
    else
    {
        for (u64 k = lo; k <= hi; ++k)
        {
            dst.At(k) = src.At(k);
        }
    }
}

}  // namespace Impl

/**
 * Sort the elements in ascending order using top down merge sort. This sort is stable and runs in O(n log n) on every
 * input.
 * @tparam T The type of the elements. Must be less-than comparable, default constructible and copy assignable.
 * @param in_out_values The elements to sort.
 * @param scratch_allocator The allocator to use for temporary storage. If nullptr, the default allocator is used.
 * @return ErrorCode::Success if the sort was successful, ErrorCode::OutOfMemory if the temporary storage could not be
 *         allocated.
 */
template <typename T>
ErrorCode MergeSort(ArrayView<T>& in_out_values, AllocatorBase* scratch_allocator = nullptr)
{
    if (in_out_values.GetSize() < 2)
    {
        return ErrorCode::Success;
    }

    Expected<DynamicArray<T>, ErrorCode> aux_result = DynamicArray<T>::Create(in_out_values.GetSize(), scratch_allocator);
    if (!aux_result.HasValue())
    {
        return aux_result.GetError();
    }
    ArrayView<T> aux(aux_result.GetValue());

    Impl::MergeSortRange(in_out_values, aux, 0, in_out_values.GetSize() - 1);
    return ErrorCode::Success;
}

/**
 * Sort the elements in ascending order using bottom up merge sort. This sort is stable, runs in O(n log n) on every
 * input, and recurses nowhere.
 * @tparam T The type of the elements. Must be less-than comparable, default constructible and copy assignable.
 * @param in_out_values The elements to sort.
 * @param scratch_allocator The allocator to use for temporary storage. If nullptr, the default allocator is used.
 * @return ErrorCode::Success if the sort was successful, ErrorCode::OutOfMemory if the temporary storage could not be
 *         allocated.
 */
template <typename T>
ErrorCode MergeSortBottomUp(ArrayView<T>& in_out_values, AllocatorBase* scratch_allocator = nullptr)
{
    if (in_out_values.GetSize() < 2)
    {
        return ErrorCode::Success;
    }

    Expected<DynamicArray<T>, ErrorCode> aux_result = DynamicArray<T>::Create(in_out_values.GetSize(), scratch_allocator);
    if (!aux_result.HasValue())
    {
        return aux_result.GetError();
    }
    ArrayView<T> aux(aux_result.GetValue());

    const u64 size = in_out_values.GetSize();
    for (u64 width = 1; width < size; width *= 2)
    {
        for (u64 lo = 0; lo < size - width; lo += 2 * width)
        {
            const u64 hi = lo + 2 * width - 1;
            Impl::MergeSortMerge(in_out_values, aux, lo, lo + width - 1, hi < size - 1 ? hi : size - 1);
        }
    }
    return ErrorCode::Success;
}

/**
 * Sort the elements in ascending order using merge sort. Faster than MergeSort on the same input, but not stable.
 * @tparam T The type of the elements. Must be less-than comparable, copy constructible, copy assignable, movable and
 *           swappable.
 * @param in_out_values The elements to sort.
 * @param scratch_allocator The allocator to use for temporary storage. If nullptr, the default allocator is used.
 * @return ErrorCode::Success if the sort was successful, ErrorCode::OutOfMemory if the temporary storage could not be
 *         allocated.
 */
template <typename T>
ErrorCode MergeSortImproved(ArrayView<T>& in_out_values, AllocatorBase* scratch_allocator = nullptr)
{
    if (in_out_values.GetSize() < 2)
    {
        return ErrorCode::Success;
    }

    Expected<DynamicArray<T>, ErrorCode> aux_result =
        DynamicArray<T>::Create(in_out_values.GetData(), in_out_values.GetSize(), scratch_allocator);
    if (!aux_result.HasValue())
    {
        return aux_result.GetError();
    }
    ArrayView<T> aux(aux_result.GetValue());

    Impl::MergeSortRangeImproved(in_out_values, aux, 0, in_out_values.GetSize() - 1);
    return ErrorCode::Success;
}

}  // namespace Opal

#pragma once

#include "opal/common.h"
#include "opal/container/array-view.h"
#include "opal/types.h"

namespace Opal
{
namespace Impl
{

/** Restore the heap order of the subtree rooted at the one based index k, over the first size elements. */
template <typename T>
void HeapSortSink(ArrayView<T>& array, u64 k, u64 size)
{
    while (2 * k <= size)
    {
        u64 j = 2 * k;
        if (j < size && array.At(j - 1) < array.At(j))
        {
            ++j;
        }
        if (!(array.At(k - 1) < array.At(j - 1)))
        {
            break;
        }
        Swap(array.At(k - 1), array.At(j - 1));
        k = j;
    }
}

}  // namespace Impl

/**
 * Sort the elements in ascending order using heap sort. Allocates nothing and runs in O(n log n) on every input,
 * but is not stable.
 * @tparam T The type of the elements. Must be less-than comparable and swappable.
 * @param in_out_values The elements to sort.
 */
template <typename T>
void HeapSort(ArrayView<T>& in_out_values)
{
    const u64 size = in_out_values.GetSize();
    if (size < 2)
    {
        return;
    }

    for (u64 k = size / 2; k >= 1; --k)
    {
        Impl::HeapSortSink(in_out_values, k, size);
    }

    u64 k = size;
    while (k > 1)
    {
        Swap(in_out_values.At(0), in_out_values.At(k - 1));
        --k;
        Impl::HeapSortSink(in_out_values, 1, k);
    }
}

}  // namespace Opal

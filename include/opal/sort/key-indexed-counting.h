#pragma once

#include "opal/container/array-view.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Sort the input values based on the keys using key-indexed counting. This sort is stable. The keys are assumed to be in the range [0, k_number_of_unique_keys).
 * @tparam k_number_of_unique_keys The number of unique keys in the input keys.
 * @tparam Key The type of the keys.
 * @tparam Value The type of the values.
 * @tparam Allocator The type of the allocator to use for temporary storage.
 * @param in_keys The input keys. Should be the same size as in_out_value.
 * @param in_out_value The input values to sort. Should be the same size as in_keys.
 * @param scratch_allocator The allocator to use for temporary storage. If nullptr, the default allocator is used.
 * @return ErrorCode::Success if the sort was successful, ErrorCode::BadInput if the input keys and values are not the same size.
 */
template <u64 k_number_of_unique_keys, typename Key, typename Value, typename Allocator = AllocatorBase>
ErrorCode KeyIndexedCounting(const ArrayView<Key>& in_keys, ArrayView<Value>& in_out_value, Allocator* scratch_allocator = nullptr)
{
    if (in_out_value.GetSize() != in_keys.GetSize())
    {
        return ErrorCode::InvalidArgument;
    }
    if (in_keys.GetSize() == 0)
    {
        return ErrorCode::Success;
    }

    DynamicArray<u64> count(k_number_of_unique_keys + 1, scratch_allocator);
    DynamicArray<Value> aux(in_out_value.GetSize(), scratch_allocator);

    // Calculate frequency of each key
    for (u64 i = 0; i < in_keys.GetSize(); ++i)
    {
        OPAL_ASSERT(in_keys[i] >= 0 && in_keys[i] < k_number_of_unique_keys, "Key out of range");
        count[in_keys[i] + 1]++;
    }

    // Transform count into indices
    for (u64 r = 0; r < k_number_of_unique_keys; ++r)
    {
        count[r + 1] += count[r];
    }

    // Distribute the input values
    for (u64 i = 0; i < in_keys.GetSize(); ++i)
    {
        aux[count[in_keys[i]]++] = in_out_value[i];
    }

    // Copy back
    for (u64 i = 0; i < in_out_value.GetSize(); ++i)
    {
        in_out_value[i] = aux[i];
    }
    return ErrorCode::Success;
}

}  // namespace Opal

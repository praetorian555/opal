#pragma once

#include <type_traits>

#include "opal/defines.h"
#include "opal/types.h"

namespace Opal
{

/**
 * Calculates 64-bit hash from an array of bytes.
 * @param data Pointer to data to use to create a hash.
 * @param size Number of bytes of data to be used to create hash.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
u64 CalculateHashFromPointerArray(const u8* data, u64 size, u64 seed = 0);

/**
 * Calculates 64-bit hash from the POD object.
 * @tparam T Object type.
 * @param value Object.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
// TODO: Implement requirement that T is plain old data from scratch
template <typename T>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
u64 CalculateHashFromObject(const T& value, u64 seed = 0);

}  // namespace Opal

template <typename T>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
Opal::u64 Opal::CalculateHashFromObject(const T& value, Opal::u64 seed)
{
    return CalculateHashFromPointerArray(reinterpret_cast<const u8*>(&value), sizeof(T), seed);
}

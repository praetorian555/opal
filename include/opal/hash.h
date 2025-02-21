#pragma once

#include <type_traits>

#include "opal/defines.h"
#include "opal/export.h"
#include "opal/types.h"

namespace Opal
{
namespace Hash
{
/**
 * Calculates 64-bit hash from an array of bytes.
 * @param data Pointer to data to use to create a hash.
 * @param size Number of bytes of data to be used to create hash.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
OPAL_EXPORT u64 CalcRawArray(const u8* data, u64 size, u64 seed = 0);

/**
 * Calculates 64-bit hash from the POD object.
 * @tparam T Object type.
 * @param value Object.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
template <typename T>
u64 CalcPOD(const T& value, u64 seed = 0);

/**
 * Calculates 64-bit hash using container data.
 * @tparam Container Type of the container.
 * @param container Container object to use.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
template <typename Container>
u64 CalcContainer(const Container& container, u64 seed = 0);

}  // namespace Hash
}  // namespace Opal

template <typename T>
Opal::u64 Opal::Hash::CalcPOD(const T& value, Opal::u64 seed)
{
    return CalcRawArray(reinterpret_cast<const u8*>(&value), sizeof(T), seed);
}

template <typename Container>
Opal::u64 Opal::Hash::CalcContainer(const Container& container, Opal::u64 seed)
{
    return CalcRawArray(reinterpret_cast<const u8*>(container.GetData()), container.GetSize(), seed);
}

#pragma once

#include "opal/exceptions.h"
#include "opal/export.h"
#include "opal/types.h"
#include "opal/type-traits.h"

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
 * @tparam T Type of the container.
 * @param range Container object to use.
 * @param seed Specific seed to use. Default is 0.
 * @return Returns 64-bit hash value.
 */
template <typename T>
    requires Range<T>
u64 CalcRange(const T& range, u64 seed = 0);

}  // namespace Hash

/**
 * @brief Class used to generate 64-bit hash value for a given type.
 * @tparam T Type of value for which to generate hash.
 */
template <typename T>
struct Hasher
{
    u64 operator()(const T&) const
    {
        throw NotImplementedException(__FUNCTION__);
    }
};

/**
 * Specialization for plain old data types.
 * @tparam T Plain old data type.
 */
template <typename T>
    requires IsPOD<T>
struct Hasher<T>
{
    u64 operator()(const T& value) const
    {
        return Hash::CalcPOD(value);
    }
};

/**
 * Specialization for ranges.
 * @tparam T Range type.
 */
template <typename T>
    requires Range<T>
struct Hasher<T>
{
    u64 operator()(const T& value) const
    {
        return Hash::CalcRange(value);
    }
};

}  // namespace Opal

template <typename T>
Opal::u64 Opal::Hash::CalcPOD(const T& value, Opal::u64 seed)
{
    return CalcRawArray(reinterpret_cast<const u8*>(&value), sizeof(T), seed);
}

template <typename T>
    requires Opal::Range<T>
Opal::u64 Opal::Hash::CalcRange(const T& range, Opal::u64 seed)
{
    const size_t size = end(range) - begin(range);
    if (range.empty())
    {
        return CalcPOD<T>(range);
    }
    return CalcRawArray(reinterpret_cast<const u8*>(&(*begin(range))), size, seed);
}

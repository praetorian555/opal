#pragma once

#include "opal/export.h"
#include "opal/types.h"
#include "opal/type-traits.h"
#include "opal/casts.h"

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
 * Calculates 64-bit hash using container data. Ranges holding the same elements produce the same hash regardless of the container they
 * come from. Only meaningful for ranges of plain old data, since the bytes of the elements are what gets hashed.
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
 *
 * Specialize this for any type used as a `HashSet` or `HashMap` key that is not covered below. Equal keys must produce equal hashes, so a
 * hasher should read the members that carry the value rather than the object's bytes. Reaching this primary template fails the build, which
 * is what a type with padding does.
 *
 * @tparam T Type of value for which to generate hash.
 */
template <typename T>
struct Hasher
{
    static_assert(k_always_false_value<T>,
                  "No Hasher for this type. Its bytes do not determine its value, which is the case for padding and for floating point "
                  "members, so hashing them would let two equal keys hash differently. Specialize Opal::Hasher for it.");
};

/**
 * Specialization for types whose value is fully determined by their bytes.
 *
 * Deliberately narrower than plain old data. A type with padding is trivially copyable and standard layout, but its padding bytes are
 * indeterminate, so two objects that compare equal can hash differently. Such a type falls through to the primary template and has to be
 * given a hasher of its own.
 *
 * @tparam T Type with a unique object representation.
 */
template <typename T>
    requires HasUniqueObjectRepresentations<T> && (!Range<T>)
struct Hasher<T>
{
    u64 operator()(const T& value) const
    {
        return Hash::CalcPOD(value);
    }
};

/**
 * Specialization for floating-point types, which have no unique object representation of their own: positive and negative zero compare
 * equal while their bytes differ. Both hash alike here.
 * @tparam T Floating-point type.
 */
template <typename T>
    requires FloatingPoint<T>
struct Hasher<T>
{
    u64 operator()(const T& value) const
    {
        return Hash::CalcPOD(value == T{0} ? T{0} : value);
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
    if (range.empty())
    {
        return CalcRawArray(nullptr, 0, seed);
    }
    const u64 count = Narrow<u64>(end(range) - begin(range));
    return CalcRawArray(reinterpret_cast<const u8*>(&(*begin(range))), count * sizeof(*begin(range)), seed);
}

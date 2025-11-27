#pragma once

#include "types.h"

namespace Opal
{

template <typename T>
struct RemoveReference
{
    using Type = T;
};

template <typename T>
struct RemoveReference<T&>
{
    using Type = T;
};

template <typename T>
struct RemoveReference<T&&>
{
    using Type = T;
};

template <typename T>
struct RemoveConst
{
    using Type = T;
};

template <typename T>
struct RemoveConst<const T>
{
    using Type = T;
};

template <typename T>
using RemoveReferenceType = typename RemoveReference<T>::Type;

template <typename T>
RemoveReferenceType<T>&& Move(T&& value)
{
    return static_cast<RemoveReferenceType<T>&&>(value);
}

template <typename T>
inline constexpr bool k_is_pointer_value = false;

template <typename T>
inline constexpr bool k_is_pointer_value<T*> = true;

template <typename T>
inline constexpr bool k_is_reference_value = false;

template <typename T>
inline constexpr bool k_is_reference_value<T&> = true;

template <typename T>
inline constexpr bool k_is_reference_value<T&&> = true;

template <typename T>
inline constexpr bool k_is_const_value = false;

template <typename T>
inline constexpr bool k_is_const_value<const T> = true;

template <typename T>
inline constexpr bool k_is_array_literal_value = false;

template <typename T, u64 N>
inline constexpr bool k_is_array_literal_value<T[N]> = true;

template <typename From, typename To, typename = void>
inline constexpr bool k_is_convertible_value = false;

template <typename From, typename To>
inline constexpr bool k_is_convertible_value<From, To, decltype(void(static_cast<To>(From{})))> = true;

template <typename From, typename To>
concept Convertible = k_is_convertible_value<From, To>;

template <bool k_test, typename T1, typename T2>
struct Conditional
{
    using Type = T1;
};

template <typename T1, typename T2>
struct Conditional<false, T1, T2>
{
    using Type = T2;
};

template <bool k_test, typename T1, typename T2>
using ConditionalType = typename Conditional<k_test, T1, T2>::Type;

template <typename, typename>
inline constexpr bool k_is_same_value = false;

template <typename T>
inline constexpr bool k_is_same_value<T, T> = true;

template <typename T>
concept Pointer = k_is_pointer_value<T>;

template <typename T>
concept Reference = k_is_reference_value<T>;

template <typename T>
concept ArrayLiteral = k_is_array_literal_value<T>;

template <typename T, typename U>
concept SameAs = k_is_same_value<T, U>;

template <typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> SameAs<bool>;
    { a != b } -> SameAs<bool>;
};

template <typename T>
concept DefaultConstructable = requires { T{}; };

template <typename T>
concept CopyConstructable = requires(T a) { T{a}; };

template <typename T>
concept MoveConstructable = requires(T a) { T{Move(a)}; };

template <typename T>
concept CopyAssignable = requires(T a, T b) { a = b; };

template <typename T>
concept MoveAssignable = requires(T a, T b) { a = Move(b); };

template <typename T>
concept Destructible = requires(T a) {
    {
        a.~T()
    };
};

template <typename T>
concept HasDifferenceSubType = requires { typename T::difference_type; };

template <typename T>
struct DifferenceTypeGetter
{
    using Type = void;
};

template <typename T>
    requires HasDifferenceSubType<T>
struct DifferenceTypeGetter<T>
{
    using Type = typename T::difference_type;
};

template <typename T>
struct DifferenceTypeGetter<T*>
{
    using Type = i64;
};

template <typename T>
concept HasReferenceSubType = requires { typename T::reference; };

template <typename T>
struct ReferenceTypeGetter
{
    using Type = void;
};

template <typename T>
    requires HasReferenceSubType<T>
struct ReferenceTypeGetter<T>
{
    using Type = typename T::reference;
};

template <typename T>
struct ReferenceTypeGetter<T*>
{
    using Type = T&;
};

/** Concept to get value sub-type from a given type if it exists. */

template <typename T>
concept HasValueSubType = requires { typename T::value_type; };

template <typename T>
struct ValueTypeGetter
{
    using Type = void;
};

template <typename T>
    requires HasValueSubType<T>
struct ValueTypeGetter<T>
{
    using Type = typename T::value_type;
};

template <typename I>
concept Iterator = requires(I i) {
    { *i } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
    { ++i } -> SameAs<I&>;
    { *i++ } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
} && CopyConstructable<I> && CopyAssignable<I> && Destructible<I>;

template <typename I>
concept InputIterator = Iterator<I> && EqualityComparable<I> && requires(I i) {
    { i++ };
};

template <typename I>
concept ForwardIterator = InputIterator<I> && DefaultConstructable<I> && requires(I i) {
    { i++ } -> SameAs<I>;
};

template <typename I>
concept BidirectionalIterator = ForwardIterator<I> && requires(I i) {
    {
        --i
    } -> SameAs<I&>;
    {
        i--
    } -> SameAs<I>;
    {
        *i--
    } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
};

template <typename I>
concept RandomAccessIterator = BidirectionalIterator<I> && requires(I i, I j, typename DifferenceTypeGetter<I>::Type n) {
    {
        i += n
    } -> SameAs<I&>;
    {
        i + n
    } -> SameAs<I>;
    {
        n + i
    } -> SameAs<I>;
    {
        i -= n
    } -> SameAs<I&>;
    {
        i - n
    } -> SameAs<I>;
    {
        i - j
    } -> Convertible<typename DifferenceTypeGetter<I>::Type>;
    {
        i[n]
    } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
    {
        i < j
    } -> SameAs<bool>;
    {
        i > j
    } -> SameAs<bool>;
    {
        i <= j
    } -> SameAs<bool>;
    {
        i >= j
    } -> SameAs<bool>;
};

template <typename T>
concept Range = requires(T& t) {
    {
        begin(t)
    } -> RandomAccessIterator;
    {
        end(t)
    } -> RandomAccessIterator;
    {
        t.empty()
    } -> SameAs<bool>;
};


/**
 * @brief Compile-time check if a type matches any of the provided candidate types.
 * @tparam Ref The reference type to check.
 * @tparam Candidates Pack of candidate types to compare against.
 * @return constexpr bool True if Ref matches any of the Candidates, false otherwise.
 */
template <typename Ref, typename ...Candidates>
inline constexpr bool k_is_any_value = (k_is_same_value<Ref, Candidates> || ...);

/**
 * @brief Concept that checks if a type is one of several candidate types.
 * @tparam Ref The reference type to check.
 * @tparam Candidates Pack of candidate types to compare against.
 * @example
 *   template <typename T>
 *   requires AnyOf<T, int, float, double>
 *   void processNumericType(T value) {
 *       // Implementation
 *   }
 *
 *   template <typename T>
 *   void processNumericType(T value) requires AnyOf<T, int, float, double> {
 *       // Implementation
 *   }
 *
 *   void processNumericType(AnyOf<int, float, double> auto value) {
 *       // Implementation
 *   }
 */
template <typename Ref, typename ...Candidates>
concept AnyOf = k_is_any_value<Ref, Candidates...>;

/**
 * @brief Concept that checks if a type is a floating point type.
 * @tparam T The type to be evaluated.
 * @example
 *      static_assert(FloatingPoint<float>);
 *      static_assert(FloatingPoint<f64>);
 *      static_assert(!FloatingPoint<int>);
 */
template <typename T>
concept FloatingPoint = AnyOf<T, f32, f64>;

/**
 * @brief Concept that checks if a type is an integral type.
 * @tparam T The type to be evaluated.
 * @example
 *      static_assert(Integral<int>);
 *      static_assert(Integral<u32>);
 *      static_assert(!Integral<double>);
 */
template <typename T>
concept Integral = AnyOf<T, u8, u16, u32, u64, i8, i16, i32, i64, long, long unsigned, char8, char16, uchar32>;

template <typename T>
concept SignedIntegral = AnyOf<T, i8, i16, i32, i64, long, char8, char16>;

template <typename T>
concept UnsignedIntegral = AnyOf<T, u8, u16, u32, u64, long unsigned, uchar32>;


/**
 * @brief Concept that checks if a type is either an integral or a floating point type.
 * @tparam T The type to be evaluated.
 * @example
 *      template <IntegralOrFloatingPoint T>
 *      T add(T a, T b) {
 *          return a + b;
 *      }
 *
 *      static_assert(IntegralOrFloatingPoint<int>);
 *      static_assert(IntegralOrFloatingPoint<double>);
 *      static_assert(!IntegralOrFloatingPoint<bool>);
 */
template <typename T>
concept IntegralOrFloatingPoint = Integral<T> || FloatingPoint<T>;

template <typename T>
concept IsTrivial = __is_trivial(T);

template <typename T>
concept IsStandardLayout = __is_standard_layout(T);

template <typename T>
concept IsPOD = IsTrivial<T> && IsStandardLayout<T>;

}  // namespace Opal

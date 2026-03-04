#pragma once

#include <utility>

#include "types.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Reference and const removal utilities.
// ------------------------------------------------------------------------------------------------

/** Strips lvalue and rvalue reference qualifiers from T. */
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

/** Strips top-level const qualifier from T. */
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

/** Alias for RemoveReference<T>::Type. */
template <typename T>
using RemoveReferenceType = typename RemoveReference<T>::Type;

/**
 * Casts value to an rvalue reference, enabling move semantics.
 * Equivalent to std::move.
 */
template <typename T>
RemoveReferenceType<T>&& Move(T&& value)
{
    return static_cast<RemoveReferenceType<T>&&>(value);
}

// ------------------------------------------------------------------------------------------------
// Compile-time type property checks.
// ------------------------------------------------------------------------------------------------

/** True if T is void. */
template <typename T>
inline constexpr bool k_is_void_value = false;

template <>
inline constexpr bool k_is_void_value<void> = true;

/** True if T is a pointer type. */
template <typename T>
inline constexpr bool k_is_pointer_value = false;

template <typename T>
inline constexpr bool k_is_pointer_value<T*> = true;

/** True if T is an lvalue or rvalue reference. */
template <typename T>
inline constexpr bool k_is_reference_value = false;

template <typename T>
inline constexpr bool k_is_reference_value<T&> = true;

template <typename T>
inline constexpr bool k_is_reference_value<T&&> = true;

/** True if T is const-qualified. */
template <typename T>
inline constexpr bool k_is_const_value = false;

template <typename T>
inline constexpr bool k_is_const_value<const T> = true;

/** True if T is a C-style array (e.g. int[5]). */
template <typename T>
inline constexpr bool k_is_array_literal_value = false;

template <typename T, u64 N>
inline constexpr bool k_is_array_literal_value<T[N]> = true;

/** True if From is implicitly convertible to To via static_cast. */
template <typename From, typename To, typename = void>
inline constexpr bool k_is_convertible_value = false;

template <typename From, typename To>
inline constexpr bool k_is_convertible_value<From, To, decltype(void(static_cast<To>(From{})))> = true;

/** True if To can be brace-initialized from From. */
template <typename From, typename To, typename = void>
inline constexpr bool k_is_constructible_value = false;

template <typename From, typename To>
inline constexpr bool k_is_constructible_value<From, To, decltype(void(To{std::declval<From>()}))> = true;

/** Concept: From is implicitly convertible to To. */
template <typename From, typename To>
concept Convertible = k_is_convertible_value<From, To>;

/** Concept: To can be brace-initialized from From. */
template <typename From, typename To>
concept Constructible = k_is_constructible_value<From, To>;

// ------------------------------------------------------------------------------------------------
// Conditional type selection.
// ------------------------------------------------------------------------------------------------

/** Selects T1 if k_test is true, T2 otherwise. Equivalent to std::conditional. */
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

/** Alias for Conditional<k_test, T1, T2>::Type. */
template <bool k_test, typename T1, typename T2>
using ConditionalType = typename Conditional<k_test, T1, T2>::Type;

// ------------------------------------------------------------------------------------------------
// Type identity check.
// ------------------------------------------------------------------------------------------------

/** True if the two types are exactly the same. */
template <typename, typename>
inline constexpr bool k_is_same_value = false;

template <typename T>
inline constexpr bool k_is_same_value<T, T> = true;

// ------------------------------------------------------------------------------------------------
// Basic type concepts.
// ------------------------------------------------------------------------------------------------

/** Concept: T is a pointer type. */
template <typename T>
concept Pointer = k_is_pointer_value<T>;

/** Concept: T is a reference type (lvalue or rvalue). */
template <typename T>
concept Reference = k_is_reference_value<T>;

/** Concept: T is a C-style array literal. */
template <typename T>
concept ArrayLiteral = k_is_array_literal_value<T>;

/** Concept: T and U are exactly the same type. */
template <typename T, typename U>
concept SameAs = k_is_same_value<T, U>;

/** Concept: T supports == and != returning bool. */
template <typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> SameAs<bool>;
    { a != b } -> SameAs<bool>;
};

// ------------------------------------------------------------------------------------------------
// Construction, assignment, and lifetime concepts.
// ------------------------------------------------------------------------------------------------

/** Concept: T can be value-initialized with T{}. */
template <typename T>
concept DefaultConstructable = requires { T{}; };

/** Concept: T can be copy-constructed from a const lvalue. */
template <typename T>
concept CopyConstructable = requires(T a) { T{a}; };

/** Concept: T can be move-constructed from an rvalue. */
template <typename T>
concept MoveConstructable = requires(T a) { T{Move(a)}; };

/** Concept: T can be copy-assigned. */
template <typename T>
concept CopyAssignable = requires(T a, T b) { a = b; };

/** Concept: T provides a Clone() method. */
template <typename T>
concept Clonable = requires(T a) { a.Clone(); };

/** Concept: T can be move-assigned. */
template <typename T>
concept MoveAssignable = requires(T a, T b) { a = Move(b); };

/** Concept: T can be explicitly destroyed. */
template <typename T>
concept Destructible = requires(T a) {
    {
        a.~T()
    };
};

// ------------------------------------------------------------------------------------------------
// Sub-type getters for iterator support.
// ------------------------------------------------------------------------------------------------

/** Concept: T has a nested difference_type alias. */
template <typename T>
concept HasDifferenceSubType = requires { typename T::difference_type; };

/** Extracts the difference_type from T. Defaults to void if not present. Specializes to i64 for pointers. */
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

/** Concept: T has a nested reference alias. */
template <typename T>
concept HasReferenceSubType = requires { typename T::reference; };

/** Extracts the reference type from T. Defaults to void if not present. Specializes to T& for pointers. */
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

/** Concept: T has a nested value_type alias. */
template <typename T>
concept HasValueSubType = requires { typename T::value_type; };

/** Extracts the value_type from T. Defaults to void if not present. */
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

// ------------------------------------------------------------------------------------------------
// Iterator concepts.
// ------------------------------------------------------------------------------------------------

/** Concept: I satisfies the basic iterator requirements (dereference, increment, copy, assign, destroy). */
template <typename I>
concept Iterator = requires(I i) {
    { *i } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
    { ++i } -> SameAs<I&>;
    { *i++ } -> SameAs<typename ReferenceTypeGetter<I>::Type>;
} && CopyConstructable<I> && CopyAssignable<I> && Destructible<I>;

/** Concept: I is an Iterator that also supports equality comparison and post-increment. */
template <typename I>
concept InputIterator = Iterator<I> && EqualityComparable<I> && requires(I i) {
    { i++ };
};

/** Concept: I is an InputIterator that is also default-constructible and post-increment returns I. */
template <typename I>
concept ForwardIterator = InputIterator<I> && DefaultConstructable<I> && requires(I i) {
    { i++ } -> SameAs<I>;
};

/** Concept: I is a ForwardIterator that also supports decrement. */
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

/** Concept: I is a BidirectionalIterator that also supports arithmetic, subscript, and comparison. */
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

// ------------------------------------------------------------------------------------------------
// Range concept.
// ------------------------------------------------------------------------------------------------

/** Concept: T supports begin(), end() returning RandomAccessIterators, and empty(). */
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

// ------------------------------------------------------------------------------------------------
// Type matching utilities.
// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------
// Numeric type concepts.
// ------------------------------------------------------------------------------------------------

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

/** Concept: T is a signed integral type. */
template <typename T>
concept SignedIntegral = AnyOf<T, i8, i16, i32, i64, long, char8, char16>;

/** Concept: T is an unsigned integral type. */
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

// ------------------------------------------------------------------------------------------------
// Triviality and layout concepts.
// ------------------------------------------------------------------------------------------------

/** Concept: T is trivially copyable (can be safely memcpy'd). */
template <typename T>
concept IsTriviallyCopyable = __is_trivially_copyable(T);

/** Concept: T has standard layout (compatible with C struct layout). */
template <typename T>
concept IsStandardLayout = __is_standard_layout(T);

/** Concept: T is a POD (Plain Old Data) type, i.e. both trivially copyable and standard layout. */
template <typename T>
concept IsPOD = IsTriviallyCopyable<T> && IsStandardLayout<T>;

}  // namespace Opal

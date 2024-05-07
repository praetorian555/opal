#pragma once

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
using RemoveReferenceType = typename RemoveReference<T>::Type;

template <typename T>
RemoveReferenceType<T>&& Move(T&& value)
{
    return static_cast<RemoveReferenceType<T>&&>(value);
}

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
    { a.~T() };
};

template <typename I>
concept Iterator = requires(I i) {
    { *i } -> SameAs<typename I::reference>;
    { ++i } -> SameAs<I&>;
    { *i++ } -> SameAs<typename I::reference>;
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
    { --i } -> SameAs<I&>;
    { i-- } -> SameAs<I>;
    { *i-- } -> SameAs<typename I::reference>;
};

template <typename I>
concept RandomAccessIterator = BidirectionalIterator<I> && requires(I i, I j, typename I::difference_type n) {
    { i += n } -> SameAs<I&>;
    { i + n } -> SameAs<I>;
    { n + i } -> SameAs<I>;
    { i -= n } -> SameAs<I&>;
    { i - n } -> SameAs<I>;
    { i - j } -> SameAs<typename I::difference_type>;
    { i[n] } -> SameAs<typename I::reference>;
    { i < j } -> SameAs<bool>;
    { i > j } -> SameAs<bool>;
    { i <= j } -> SameAs<bool>;
    { i >= j } -> SameAs<bool>;
};

}  // namespace Opal

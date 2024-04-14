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
struct IsReference
{
    static constexpr bool k_value = false;
};

template <typename T>
struct IsReference<T&>
{
    static constexpr bool k_value = true;
};

template <typename T>
struct IsReference<T&&>
{
    static constexpr bool k_value = true;
};

template <typename T>
constexpr bool k_is_reference_value = IsReference<T>::k_value;

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

}  // namespace Opal

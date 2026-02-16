#pragma once

/**
 * Macro that defines bitwise operators for an enum class, enabling its use as a type-safe bitmask.
 * Provides: |, &, ^, |=, &=, ^=, ~, and ! operators.
 */
#define OPAL_ENUM_CLASS_FLAGS(Enum)                                                                                                    \
    inline Enum& operator|=(Enum& lhs, Enum rhs)                                                                                      \
    {                                                                                                                                  \
        return lhs = static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) | static_cast<__underlying_type(Enum)>(rhs));         \
    }                                                                                                                                  \
    inline Enum& operator&=(Enum& lhs, Enum rhs)                                                                                      \
    {                                                                                                                                  \
        return lhs = static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) & static_cast<__underlying_type(Enum)>(rhs));         \
    }                                                                                                                                  \
    inline Enum& operator^=(Enum& lhs, Enum rhs)                                                                                      \
    {                                                                                                                                  \
        return lhs = static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) ^ static_cast<__underlying_type(Enum)>(rhs));         \
    }                                                                                                                                  \
    inline constexpr Enum operator|(Enum lhs, Enum rhs)                                                                                \
    {                                                                                                                                  \
        return static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) | static_cast<__underlying_type(Enum)>(rhs));               \
    }                                                                                                                                  \
    inline constexpr Enum operator&(Enum lhs, Enum rhs)                                                                                \
    {                                                                                                                                  \
        return static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) & static_cast<__underlying_type(Enum)>(rhs));               \
    }                                                                                                                                  \
    inline constexpr Enum operator^(Enum lhs, Enum rhs)                                                                                \
    {                                                                                                                                  \
        return static_cast<Enum>(static_cast<__underlying_type(Enum)>(lhs) ^ static_cast<__underlying_type(Enum)>(rhs));               \
    }                                                                                                                                  \
    inline constexpr bool operator!(Enum e)                                                                                            \
    {                                                                                                                                  \
        return !static_cast<__underlying_type(Enum)>(e);                                                                               \
    }                                                                                                                                  \
    inline constexpr Enum operator~(Enum e)                                                                                            \
    {                                                                                                                                  \
        return static_cast<Enum>(~static_cast<__underlying_type(Enum)>(e));                                                            \
    }

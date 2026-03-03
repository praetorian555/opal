#pragma once

#include "container/scope-ptr.h"
#include "opal/common.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/string.h"

namespace Opal
{

/**
 * @brief Exception thrown when the user passes "help" or "--help" as a program argument.
 */
struct HelpRequestedException : Exception
{
    HelpRequestedException() : Exception("Help was requested") {}
};

/**
 * @brief Exception thrown when the user passes "version" or "--version" as a program argument.
 */
struct VersionRequestedException : Exception
{
    VersionRequestedException() : Exception("Version was requested") {}
};

namespace Impl
{

/**
 * @brief Base class for all program argument definitions. Stores the argument name, description, and whether it is
 * optional. Subclasses implement SetValue to parse and store the argument value.
 */
struct ProgramArgumentDefinition
{
    StringUtf8 m_name;
    StringUtf8 m_description;
    bool m_is_optional;

    ProgramArgumentDefinition(StringUtf8 in_name, StringUtf8 in_desc, bool in_is_optional)
        : m_name(std::move(in_name)), m_description(std::move(in_desc)), m_is_optional(in_is_optional)
    {
        if (m_name.IsEmpty())
        {
            throw InvalidArgumentException(__FUNCTION__, "name of the argument can't be empty");
        }
        if (m_description.IsEmpty())
        {
            throw InvalidArgumentException(__FUNCTION__, "description of the argument can't be empty");
        }
    }

    virtual ~ProgramArgumentDefinition() = default;

    /**
     * @brief Parse the string and store the result in the destination variable.
     * @param str String representation of the argument value.
     */
    virtual void SetValue(const StringUtf8& str) = 0;

    /**
     * @brief Return the list of allowed value strings for this argument. Used by ShowHelp to display possible values.
     *        Returns possible values converted to strings, or mapping keys for enum/mapped arguments.
     * @return Array of allowed value strings, or empty array if any value is accepted.
     */
    virtual DynamicArray<StringUtf8> GetAllowedValues() const { return {}; }

    static bool IsValueAllowed(const StringUtf8& value, const ArrayView<StringUtf8>& values);
};

/**
 * @brief Typed base class for scalar program argument definitions. Stores the destination reference, possible values,
 *        and possible value mappings. Provides common logic for value validation and mapping lookup.
 * @tparam T Type of the argument value (e.g., i32, StringUtf8, bool, or an enum type).
 */
template <typename T>
struct TypedProgramArgumentDefinitionBase : ProgramArgumentDefinition
{
    DynamicArray<T> m_possible_values;
    HashMap<StringUtf8, T> m_possible_value_mappings;
    Ref<T> m_dest_value;

    TypedProgramArgumentDefinitionBase(StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<T> possible_values,
                                       HashMap<StringUtf8, T> possible_value_mappings, Ref<T> dest_value)
        : ProgramArgumentDefinition(std::move(name), std::move(desc), is_optional),
          m_possible_values(std::move(possible_values)),
          m_possible_value_mappings(std::move(possible_value_mappings)),
          m_dest_value(std::move(dest_value))
    {
    }

    /** @brief Write the parsed value to the destination variable. */
    void SetDestinationValue(T value) { *m_dest_value = std::move(value); }

    /** @brief Check if an explicit list of possible values has been provided. */
    [[nodiscard]] bool HasPossibleValues() const { return !m_possible_values.IsEmpty(); }
    /** @brief Check if a string-to-value mapping has been provided. */
    [[nodiscard]] bool HasPossibleValueMappings() const { return m_possible_value_mappings.GetSize() != 0; }

    /**
     * @brief Check if a value is in the list of possible values. Returns true if no possible values are set.
     * @param value Value to check.
     * @return True if the value is allowed.
     */
    bool IsPossibleValue(const T& value)
    {
        if (m_possible_values.IsEmpty())
        {
            return true;
        }
        for (const T& possible_value : m_possible_values)
        {
            if (value == possible_value)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Look up a value by its string key in the possible value mappings.
     * @param key String key to look up.
     * @return Reference to the mapped value.
     * @throws InvalidArgumentException if the key is not found in the mappings.
     */
    const T& GetValueFromMapping(const StringUtf8& key)
    {
        const auto& it = m_possible_value_mappings.Find(key);
        if (it != m_possible_value_mappings.end())
        {
            return it->value;
        }
        throw InvalidArgumentException(__FUNCTION__, "No mapping for the given value");
    }

    DynamicArray<StringUtf8> GetAllowedValues() const override
    {
        DynamicArray<StringUtf8> result;
        if (!m_possible_values.IsEmpty())
        {
            for (const T& val : m_possible_values)
            {
                if constexpr (SameAs<T, StringUtf8>)
                {
                    result.PushBack(Clone(val));
                }
                else if constexpr (Integral<T>)
                {
                    result.PushBack(NumberToString(val));
                }
            }
        }
        else if (m_possible_value_mappings.GetSize() != 0)
        {
            for (const auto& pair : m_possible_value_mappings)
            {
                result.PushBack(Clone(pair.key));
            }
        }
        return result;
    }
};

/**
 * @brief Primary template for typed program argument definitions. This is a fallback that throws on SetValue,
 *        meaning the type is not directly supported. Specializations below handle bool, integral types, enums,
 *        strings, and dynamic arrays.
 * @tparam T Type of the argument value.
 */
template <typename T>
struct TypedProgramArgumentDefinition final : TypedProgramArgumentDefinitionBase<T>
{
    TypedProgramArgumentDefinition(Ref<T> dest, StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<T> possible_values,
                                   HashMap<StringUtf8, T> possible_value_mappings)
        : TypedProgramArgumentDefinitionBase<T>(std::move(name), std::move(desc), is_optional, std::move(possible_values),
                                                std::move(possible_value_mappings), std::move(dest))
    {
    }

    void SetValue(const StringUtf8&) override { throw InvalidArgumentException(__FUNCTION__, "Value type not supported"); }
};

/** @brief Concept that checks if T is an enum type. */
template <typename T>
concept IsEnum = std::is_enum_v<T>;

/** @brief Helper trait to detect DynamicArray types. */
template <typename T>
inline constexpr bool k_is_dynamic_array_value = false;
template <typename T>
inline constexpr bool k_is_dynamic_array_value<DynamicArray<T>> = true;
/** @brief Concept that checks if T is a DynamicArray instantiation. */
template <typename T>
concept IsDynamicArray = k_is_dynamic_array_value<T>;

/** @brief Concept for types that can be elements of a DynamicArray program argument (integral, enum, or string, but not bool). */
template <typename T>
concept ValidDynamicArrayElement = (Integral<T> || IsEnum<T> || SameAs<T, StringUtf8>) && !SameAs<T, bool>;

/**
 * @brief Specialization for enum types. Requires a string-to-enum mapping to be provided.
 *        SetValue looks up the string in the mapping and stores the corresponding enum value.
 * @tparam T Enum type.
 */
template <IsEnum T>
struct TypedProgramArgumentDefinition<T> final : TypedProgramArgumentDefinitionBase<T>
{
    TypedProgramArgumentDefinition(Ref<T> dest, StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<T> possible_values,
                                   HashMap<StringUtf8, T> possible_value_mappings)
        : TypedProgramArgumentDefinitionBase<T>(std::move(name), std::move(desc), is_optional, std::move(possible_values),
                                                std::move(possible_value_mappings), std::move(dest))
    {
        if (TypedProgramArgumentDefinitionBase<T>::HasPossibleValues())
        {
            throw InvalidArgumentException(__FUNCTION__,
                                           "Enum program argument needs to have populated possible_values_mapping, not possible_values");
        }
        if (!TypedProgramArgumentDefinitionBase<T>::HasPossibleValueMappings())
        {
            throw InvalidArgumentException(__FUNCTION__, "Enum program argument needs to have populated possible_values_mapping");
        }
    }

    void SetValue(const StringUtf8& str) override
    {
        const T& value = TypedProgramArgumentDefinitionBase<T>::GetValueFromMapping(str);
        TypedProgramArgumentDefinitionBase<T>::SetDestinationValue(Opal::Clone(value));
    }
};

/**
 * @brief Specialization for integral types. Parses the string to a number using StringToNumber.
 *        Optionally validates against possible values or mappings.
 * @tparam T Integral type (i32, u64, etc.).
 */
template <Integral T>
struct TypedProgramArgumentDefinition<T> final : TypedProgramArgumentDefinitionBase<T>
{
    using Super = TypedProgramArgumentDefinitionBase<T>;

    TypedProgramArgumentDefinition(Ref<T> dest, StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<T> possible_values,
                                   HashMap<StringUtf8, T> possible_value_mappings)
        : Super(std::move(name), std::move(desc), is_optional, std::move(possible_values), std::move(possible_value_mappings),
                std::move(dest))
    {
        if (Super::HasPossibleValues() && Super::HasPossibleValueMappings())
        {
            throw InvalidArgumentException(__FUNCTION__,
                                           "Integer program argument can't have both possible values and possible value mappings");
        }
    }

    void SetValue(const StringUtf8& str) override
    {
        const T value = StringToNumber<T>(str);
        Super::SetDestinationValue(value);
    }
};

/**
 * @brief Specialization for bool. Acts as a flag: presence of the argument sets the value to true.
 *        Ignores possible values and mappings.
 */
template <>
struct TypedProgramArgumentDefinition<bool> final : TypedProgramArgumentDefinitionBase<bool>
{
    TypedProgramArgumentDefinition(Ref<bool> dest, StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<bool>,
                                   HashMap<StringUtf8, bool>)
        : TypedProgramArgumentDefinitionBase(std::move(name), std::move(desc), is_optional, {}, HashMap<StringUtf8, bool>(),
                                             std::move(dest))
    {
    }

    void SetValue(const StringUtf8&) override { SetDestinationValue(true); }
};

/**
 * @brief Specialization for StringUtf8. Strips surrounding quotes, then validates against possible values or mappings
 *        if provided.
 */
template <>
struct TypedProgramArgumentDefinition<StringUtf8> final : TypedProgramArgumentDefinitionBase<StringUtf8>
{
    TypedProgramArgumentDefinition(Ref<StringUtf8> dest, StringUtf8 name, StringUtf8 desc, bool is_optional,
                                   DynamicArray<StringUtf8> possible_values, HashMap<StringUtf8, StringUtf8> possible_value_mappings)
        : TypedProgramArgumentDefinitionBase(std::move(name), std::move(desc), is_optional, std::move(possible_values),
                                             std::move(possible_value_mappings), std::move(dest))
    {
        if (HasPossibleValues() && HasPossibleValueMappings())
        {
            throw InvalidArgumentException(
                __FUNCTION__, "String program argument has both possible values and possible value mappings, please use only one of these");
        }
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str.Clone();
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = std::move(GetSubString(str, 1, str.GetSize() - 2).GetValue());
        }
        if (HasPossibleValues())
        {
            if (!IsPossibleValue(trimmed))
            {
                throw InvalidArgumentException(__FUNCTION__, "String program argument is not one of the possible values");
            }
            SetDestinationValue(std::move(trimmed));
            return;
        }
        if (HasPossibleValueMappings())
        {
            const StringUtf8& mapping = GetValueFromMapping(str);
            SetDestinationValue(Clone(mapping));
            return;
        }
        SetDestinationValue(std::move(trimmed));
    }
};

/**
 * @brief Specialization for DynamicArray arguments. Accepts a comma-separated list of values.
 *        Each element is parsed according to its type (string, integer, or enum via mapping).
 *        Inherits directly from ProgramArgumentDefinition since it manages its own possible values and mappings.
 * @tparam E Element type of the array (must satisfy ValidDynamicArrayElement).
 */
template <ValidDynamicArrayElement E>
struct TypedProgramArgumentDefinition<DynamicArray<E>> final : ProgramArgumentDefinition
{
    Ref<DynamicArray<E>> m_dest_value;
    DynamicArray<E> m_possible_values;
    HashMap<StringUtf8, E> m_possible_value_mappings;

    TypedProgramArgumentDefinition(Ref<DynamicArray<E>> dest, StringUtf8 name, StringUtf8 desc, bool is_optional,
                                   DynamicArray<E> possible_values, HashMap<StringUtf8, E> possible_value_mappings)
        : ProgramArgumentDefinition(std::move(name), std::move(desc), is_optional),
          m_dest_value(std::move(dest)),
          m_possible_values(std::move(possible_values)),
          m_possible_value_mappings(std::move(possible_value_mappings))
    {
        if constexpr (IsEnum<E>)
        {
            if (!m_possible_values.IsEmpty())
            {
                throw InvalidArgumentException(
                    __FUNCTION__, "Enum array program argument needs to have populated possible_values_mapping, not possible_values");
            }
            if (m_possible_value_mappings.GetSize() == 0)
            {
                throw InvalidArgumentException(__FUNCTION__, "Enum array program argument needs to have populated possible_values_mapping");
            }
        }
        else
        {
            if (!m_possible_values.IsEmpty() && m_possible_value_mappings.GetSize() != 0)
            {
                throw InvalidArgumentException(__FUNCTION__,
                                               "Array program argument can't have both possible values and possible value mappings");
            }
        }
    }

    [[nodiscard]] bool HasPossibleValues() const { return !m_possible_values.IsEmpty(); }
    [[nodiscard]] bool HasPossibleValueMappings() const { return m_possible_value_mappings.GetSize() != 0; }

    bool IsPossibleValue(const E& value)
    {
        if (m_possible_values.IsEmpty())
        {
            return true;
        }
        for (const E& possible_value : m_possible_values)
        {
            if (value == possible_value)
            {
                return true;
            }
        }
        return false;
    }

    const E& GetValueFromMapping(const StringUtf8& key)
    {
        const auto& it = m_possible_value_mappings.Find(key);
        if (it != m_possible_value_mappings.end())
        {
            return it->value;
        }
        throw InvalidArgumentException(__FUNCTION__, "No mapping for the given value");
    }

    DynamicArray<StringUtf8> GetAllowedValues() const override
    {
        DynamicArray<StringUtf8> result;
        if (!m_possible_values.IsEmpty())
        {
            for (const E& val : m_possible_values)
            {
                if constexpr (SameAs<E, StringUtf8>)
                    result.PushBack(Opal::Clone(val));
                else if constexpr (Integral<E>)
                    result.PushBack(NumberToString(val));
            }
        }
        else if (m_possible_value_mappings.GetSize() != 0)
        {
            for (const auto& pair : m_possible_value_mappings)
                result.PushBack(Opal::Clone(pair.key));
        }
        return result;
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str.Clone();
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = std::move(GetSubString(str, 1, str.GetSize() - 2).GetValue());
        }
        DynamicArray<StringUtf8> elements;
        SplitToArray<StringUtf8>(trimmed, StringUtf8(","), elements);
        for (const auto& element : elements)
        {
            if constexpr (SameAs<E, StringUtf8>)
            {
                if (HasPossibleValues())
                {
                    if (!IsPossibleValue(element))
                    {
                        throw InvalidArgumentException(__FUNCTION__, "Value is not one of the possible values");
                    }
                    m_dest_value->PushBack(Opal::Clone(element));
                }
                else if (HasPossibleValueMappings())
                {
                    const StringUtf8& mapping = GetValueFromMapping(element);
                    m_dest_value->PushBack(Opal::Clone(mapping));
                }
                else
                {
                    m_dest_value->PushBack(Opal::Clone(element));
                }
            }
            else if constexpr (IsEnum<E>)
            {
                const E& value = GetValueFromMapping(element);
                m_dest_value->PushBack(Opal::Clone(value));
            }
            else if constexpr (Integral<E>)
            {
                const E value = StringToNumber<E>(element);
                if (!IsPossibleValue(value))
                {
                    throw InvalidArgumentException(__FUNCTION__, "Value is not one of the possible values");
                }
                m_dest_value->PushBack(Opal::Clone(value));
            }
        }
    }
};

}  // namespace Impl

/**
 * @brief Builder for defining and parsing command-line program arguments.
 *
 * Supports scalar types (bool, integers, strings, enums) and array types (DynamicArray of integers, strings, or enums).
 * Arguments can be required or optional, and can be constrained to a set of possible values or string-to-value mappings.
 *
 * Built-in arguments:
 *  - "help" / "--help": prints help text and throws HelpRequestedException.
 *  - "version" / "--version": prints version info and throws VersionRequestedException.
 *
 * Usage:
 * @code
 *     i32 threads = 1;
 *     StringUtf8 mode;
 *     ProgramArgumentsBuilder builder;
 *     builder.AddProgramDescription("My tool")
 *            .AddArgument("threads", "Number of threads", Ref(threads), true)
 *            .AddArgument("mode", "Operating mode", Ref(mode), false, DynamicArray<StringUtf8>{"fast", "slow"})
 *            .Build(argv, argc);
 * @endcode
 */
struct OPAL_EXPORT ProgramArgumentsBuilder
{
    /**
     * @brief Set the program description displayed in help output.
     * @param description Text describing what the program does.
     * @return Reference to this builder for chaining.
     */
    ProgramArgumentsBuilder& AddProgramDescription(StringUtf8 description);

    /**
     * @brief Add a usage example displayed in help output.
     * @param example Example command-line invocation string.
     * @return Reference to this builder for chaining.
     */
    ProgramArgumentsBuilder& AddUsageExample(StringUtf8 example);

    /**
     * @brief Set the program version displayed when "--version" is passed.
     * @param major Major version number.
     * @param minor Minor version number.
     * @param patch Patch version number.
     * @return Reference to this builder for chaining.
     */
    ProgramArgumentsBuilder& SetVersion(u32 major, u32 minor, u32 patch);

    /**
     * @brief Add a scalar argument with no value constraints.
     * @tparam T Argument type (non-enum, non-array).
     * @param name Argument name used on the command line (e.g., "threads").
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the variable where the parsed value will be stored.
     * @param is_optional If true, the argument is not required.
     * @return Reference to this builder for chaining.
     */
    template <typename T>
        requires(!Impl::IsEnum<T> && !Impl::IsDynamicArray<T>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional);

    /**
     * @brief Add a scalar argument constrained to a set of possible values.
     * @tparam T Argument type (non-enum, non-array).
     * @param name Argument name used on the command line.
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the variable where the parsed value will be stored.
     * @param is_optional If true, the argument is not required.
     * @param possible_values List of allowed values. The parsed value must be one of these.
     * @return Reference to this builder for chaining.
     */
    template <typename T>
        requires((!Impl::IsEnum<T> || SameAs<T, bool>) && !Impl::IsDynamicArray<T>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                         DynamicArray<T> possible_values);

    /**
     * @brief Add a scalar argument with a string-to-value mapping (for enums or strings).
     * @tparam T Argument type (enum or StringUtf8).
     * @param name Argument name used on the command line.
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the variable where the parsed value will be stored.
     * @param is_optional If true, the argument is not required.
     * @param possible_value_mappings Map from string keys to values. The user provides a key, which is resolved to
     *        the corresponding value.
     * @return Reference to this builder for chaining.
     */
    template <typename T>
        requires(Impl::IsEnum<T> || SameAs<T, StringUtf8>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                         HashMap<StringUtf8, T> possible_value_mappings);

    /**
     * @brief Add an array argument with no value constraints. Values are provided as a comma-separated list.
     * @tparam E Element type of the array (integral or StringUtf8, not bool).
     * @param name Argument name used on the command line.
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the DynamicArray where parsed values will be appended.
     * @param is_optional If true, the argument is not required.
     * @return Reference to this builder for chaining.
     */
    template <typename E>
        requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional);

    /**
     * @brief Add an array argument constrained to a set of possible values. Values are provided as a comma-separated
     *        list.
     * @tparam E Element type of the array (integral or StringUtf8, not bool).
     * @param name Argument name used on the command line.
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the DynamicArray where parsed values will be appended.
     * @param is_optional If true, the argument is not required.
     * @param possible_values List of allowed element values.
     * @return Reference to this builder for chaining.
     */
    template <typename E>
        requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional,
                                         DynamicArray<E> possible_values);

    /**
     * @brief Add an array argument with a string-to-value mapping (for enums or strings). Values are provided as a
     *        comma-separated list of mapping keys.
     * @tparam E Element type of the array (enum or StringUtf8).
     * @param name Argument name used on the command line.
     * @param desc Human-readable description shown in help output.
     * @param destination Reference to the DynamicArray where parsed values will be appended.
     * @param is_optional If true, the argument is not required.
     * @param possible_value_mappings Map from string keys to element values.
     * @return Reference to this builder for chaining.
     */
    template <typename E>
        requires(Impl::IsEnum<E> || SameAs<E, StringUtf8>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional,
                                         HashMap<StringUtf8, E> possible_value_mappings);

    /**
     * @brief Parse command-line arguments and populate all registered destination variables.
     *        Throws HelpRequestedException or VersionRequestedException for built-in arguments.
     *        Throws InvalidArgumentException if a required argument is missing.
     * @param arguments Array of C-strings from main (argv).
     * @param count Number of elements in the arguments array (argc).
     */
    void Build(const char** arguments, u32 count);

private:
    void ShowHelp();
    void ShowVersion(const char* program_name);

    u32 m_major_version = 0;
    u32 m_minor_version = 0;
    u32 m_patch_version = 0;

    StringUtf8 m_program_description;
    DynamicArray<StringUtf8> m_usage_examples;
    DynamicArray<ScopePtr<Impl::ProgramArgumentDefinition>> m_argument_definitions;
    bool m_has_required_argument = false;
    bool m_has_optional_argument = false;
};

template <typename T>
    requires(!Impl::IsEnum<T> && !Impl::IsDynamicArray<T>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def = MakeScoped<Impl::TypedProgramArgumentDefinition<T>>(
        allocator, std::move(destination), std::move(name), std::move(desc), is_optional, DynamicArray<T>{}, HashMap<StringUtf8, T>{});
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

template <typename T>
    requires((!Impl::IsEnum<T> || SameAs<T, bool>) && !Impl::IsDynamicArray<T>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                                              DynamicArray<T> possible_values)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def =
        MakeScoped<Impl::TypedProgramArgumentDefinition<T>>(allocator, std::move(destination), std::move(name), std::move(desc),
                                                            is_optional, std::move(possible_values), HashMap<StringUtf8, T>{});
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

template <typename T>
    requires(Impl::IsEnum<T> || SameAs<T, StringUtf8>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                                              HashMap<StringUtf8, T> possible_value_mappings)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def =
        MakeScoped<Impl::TypedProgramArgumentDefinition<T>>(allocator, std::move(destination), std::move(name), std::move(desc),
                                                            is_optional, DynamicArray<T>{}, std::move(possible_value_mappings));
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

template <typename E>
    requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination,
                                                              bool is_optional)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def = MakeScoped<Impl::TypedProgramArgumentDefinition<DynamicArray<E>>>(
        allocator, std::move(destination), std::move(name), std::move(desc), is_optional, DynamicArray<E>{}, HashMap<StringUtf8, E>{});
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

template <typename E>
    requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination,
                                                              bool is_optional, DynamicArray<E> possible_values)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def = MakeScoped<Impl::TypedProgramArgumentDefinition<DynamicArray<E>>>(
        allocator, std::move(destination), std::move(name), std::move(desc), is_optional, std::move(possible_values),
        HashMap<StringUtf8, E>{});
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

template <typename E>
    requires(Impl::IsEnum<E> || SameAs<E, StringUtf8>)
ProgramArgumentsBuilder& ProgramArgumentsBuilder::AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination,
                                                              bool is_optional, HashMap<StringUtf8, E> possible_value_mappings)
{
    AllocatorBase* allocator = GetDefaultAllocator();
    ScopePtr<Impl::ProgramArgumentDefinition> arg_def = MakeScoped<Impl::TypedProgramArgumentDefinition<DynamicArray<E>>>(
        allocator, std::move(destination), std::move(name), std::move(desc), is_optional, DynamicArray<E>{},
        std::move(possible_value_mappings));
    m_argument_definitions.PushBack(std::move(arg_def));
    m_has_optional_argument = is_optional;
    m_has_required_argument = !is_optional;
    return *this;
}

}  // namespace Opal

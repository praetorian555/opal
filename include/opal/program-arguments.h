#pragma once

#include "container/scope-ptr.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/string.h"

namespace Opal
{

struct HelpRequestedException : Exception
{
    HelpRequestedException() : Exception("Help was requested") {}
};

struct VersionRequestedException : Exception
{
    VersionRequestedException() : Exception("Version was requested") {}
};

namespace Impl
{
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
    virtual void SetValue(const StringUtf8& str) = 0;

    static bool IsValueAllowed(const StringUtf8& value, const ArrayView<StringUtf8>& values);
};

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

    void SetDestinationValue(const T& value) { *m_dest_value = value; }

    [[nodiscard]] bool HasPossibleValues() const { return !m_possible_values.IsEmpty(); }
    [[nodiscard]] bool HasPossibleValueMappings() const { return m_possible_value_mappings.GetSize() != 0; }

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

    const T& GetValueFromMapping(const StringUtf8& key)
    {
        const auto& it = m_possible_value_mappings.Find(key);
        if (it != m_possible_value_mappings.end())
        {
            return it->value;
        }
        throw InvalidArgumentException(__FUNCTION__, "No mapping for the given value");
    }
};

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

template <typename T>
concept IsEnum = std::is_enum_v<T>;

template <typename T>
inline constexpr bool k_is_dynamic_array_value = false;
template <typename T>
inline constexpr bool k_is_dynamic_array_value<DynamicArray<T>> = true;
template <typename T>
concept IsDynamicArray = k_is_dynamic_array_value<T>;

template <typename T>
concept ValidDynamicArrayElement = (Integral<T> || IsEnum<T> || SameAs<T, StringUtf8>) && !SameAs<T, bool>;

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
        TypedProgramArgumentDefinitionBase<T>::SetDestinationValue(value);
    }
};

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
            throw InvalidArgumentException(__FUNCTION__, "Integer program argument can't have both possible values and possible value mappings");
        }
    }

    void SetValue(const StringUtf8& str) override
    {
        const T value = StringToNumber<T>(str);
        Super::SetDestinationValue(value);
    }
};

template <>
struct TypedProgramArgumentDefinition<bool> final : TypedProgramArgumentDefinitionBase<bool>
{
    TypedProgramArgumentDefinition(Ref<bool> dest, StringUtf8 name, StringUtf8 desc, bool is_optional, DynamicArray<bool>,
                                   HashMap<StringUtf8, bool>)
        : TypedProgramArgumentDefinitionBase(std::move(name), std::move(desc), is_optional, {}, HashMap<StringUtf8, bool>({}),
                                             std::move(dest))
    {
    }

    void SetValue(const StringUtf8&) override { SetDestinationValue(true); }
};

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
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
        }
        if (HasPossibleValues())
        {
            if (!IsPossibleValue(trimmed))
            {
                throw InvalidArgumentException(__FUNCTION__, "String program argument is not one of the possible values");
            }
            SetDestinationValue(trimmed);
            return;
        }
        if (HasPossibleValueMappings())
        {
            const StringUtf8& mapping = GetValueFromMapping(str);
            SetDestinationValue(mapping);
            return;
        }
        SetDestinationValue(trimmed);
    }
};

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
                throw InvalidArgumentException(__FUNCTION__,
                                               "Enum array program argument needs to have populated possible_values_mapping, not possible_values");
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

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
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
                    m_dest_value->PushBack(element);
                }
                else if (HasPossibleValueMappings())
                {
                    const StringUtf8& mapping = GetValueFromMapping(element);
                    m_dest_value->PushBack(mapping);
                }
                else
                {
                    m_dest_value->PushBack(element);
                }
            }
            else if constexpr (IsEnum<E>)
            {
                const E& value = GetValueFromMapping(element);
                m_dest_value->PushBack(value);
            }
            else if constexpr (Integral<E>)
            {
                const E value = StringToNumber<E>(element);
                if (!IsPossibleValue(value))
                {
                    throw InvalidArgumentException(__FUNCTION__, "Value is not one of the possible values");
                }
                m_dest_value->PushBack(value);
            }
        }
    }
};

}  // namespace Impl

struct OPAL_EXPORT ProgramArgumentsBuilder
{
    ProgramArgumentsBuilder& AddProgramDescription(const StringUtf8& description);
    ProgramArgumentsBuilder& AddUsageExample(const StringUtf8& example);

    ProgramArgumentsBuilder& SetVersion(u32 major, u32 minor, u32 patch);

    template <typename T>
        requires(!Impl::IsEnum<T> && !Impl::IsDynamicArray<T>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional);

    template <typename T>
        requires((!Impl::IsEnum<T> || SameAs<T, bool>) && !Impl::IsDynamicArray<T>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                         DynamicArray<T> possible_values);

    template <typename T>
        requires(Impl::IsEnum<T> || SameAs<T, StringUtf8>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<T> destination, bool is_optional,
                                         HashMap<StringUtf8, T> possible_value_mappings);

    template <typename E>
        requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional);

    template <typename E>
        requires((Integral<E> || SameAs<E, StringUtf8>) && !SameAs<E, bool>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional,
                                         DynamicArray<E> possible_values);

    template <typename E>
        requires(Impl::IsEnum<E> || SameAs<E, StringUtf8>)
    ProgramArgumentsBuilder& AddArgument(StringUtf8 name, StringUtf8 desc, Ref<DynamicArray<E>> destination, bool is_optional,
                                         HashMap<StringUtf8, E> possible_value_mappings);

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

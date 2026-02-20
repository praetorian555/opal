#pragma once

#include "container/scope-ptr.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/string.h"
#include "opal/export.h"

namespace Opal
{

struct OPAL_EXPORT ProgramArgumentDefinitionDesc
{
    StringUtf8 name;
    StringUtf8 desc;
    bool is_optional;
    DynamicArray<StringUtf8> possible_values;
};

struct OPAL_EXPORT ProgramArgumentDefinition
{
    StringUtf8 name;
    StringUtf8 description;
    bool is_optional;
    DynamicArray<StringUtf8> possible_values;

    ProgramArgumentDefinition(const ProgramArgumentDefinitionDesc& desc)
        : name(desc.name), description(desc.desc), is_optional(desc.is_optional), possible_values(desc.possible_values)
    {
    }

    virtual ~ProgramArgumentDefinition() = default;
    virtual void SetValue(const StringUtf8& str) = 0;

    bool IsValueAllowed(const StringUtf8& value) const
    {
        if (possible_values.IsEmpty())
        {
            return true;
        }
        for (const auto& pv : possible_values)
        {
            if (pv == value)
            {
                return true;
            }
        }
        return false;
    }
};

template class OPAL_EXPORT DynamicArray<ProgramArgumentDefinition*>;
template class OPAL_EXPORT DynamicArray<StringUtf8>;

template <typename T>
struct TypedProgramArgumentDefinition final : ProgramArgumentDefinition
{
    T* destination;

    TypedProgramArgumentDefinition(T* dest, const ProgramArgumentDefinitionDesc& desc) : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8&) override { throw NotImplementedException(__FUNCTION__); }
};

template <>
struct TypedProgramArgumentDefinition<StringUtf8> final : ProgramArgumentDefinition
{
    StringUtf8* destination;

    TypedProgramArgumentDefinition(StringUtf8* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
        }
        if (!IsValueAllowed(trimmed))
        {
            throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
        }
        *destination = trimmed;
    }
};

template <>
struct TypedProgramArgumentDefinition<DynamicArray<StringUtf8>> final : ProgramArgumentDefinition
{
    DynamicArray<StringUtf8>* destination;

    TypedProgramArgumentDefinition(DynamicArray<StringUtf8>* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
        }
        DynamicArray<StringUtf8> elements(GetScratchAllocator());
        SplitToArray<StringUtf8>(trimmed, ",", elements);
        for (const auto& element : elements)
        {
            if (!IsValueAllowed(element))
            {
                throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
            }
            destination->PushBack(element);
        }
    }
};

template <>
struct TypedProgramArgumentDefinition<i32> final : ProgramArgumentDefinition
{
    i32* destination;

    TypedProgramArgumentDefinition(i32* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        if (!IsValueAllowed(str))
        {
            throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
        }
        *destination = StringToI32(str);
    }
};

template <>
struct TypedProgramArgumentDefinition<DynamicArray<i32>> final : ProgramArgumentDefinition
{
    DynamicArray<i32>* destination;

    TypedProgramArgumentDefinition(DynamicArray<i32>* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
        }
        DynamicArray<StringUtf8> values(GetScratchAllocator());
        SplitToArray<StringUtf8>(trimmed, ",", values);
        for (const auto& value : values)
        {
            if (!IsValueAllowed(value))
            {
                throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
            }
            destination->PushBack(StringToI32(value));
        }
    }
};

template <>
struct TypedProgramArgumentDefinition<u32> final : ProgramArgumentDefinition
{
    u32* destination;

    TypedProgramArgumentDefinition(u32* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        if (!IsValueAllowed(str))
        {
            throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
        }
        *destination = StringToU32(str);
    }
};

template <>
struct TypedProgramArgumentDefinition<DynamicArray<u32>> final : ProgramArgumentDefinition
{
    DynamicArray<u32>* destination;

    TypedProgramArgumentDefinition(DynamicArray<u32>* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8& str) override
    {
        StringUtf8 trimmed = str;
        if (!str.IsEmpty() && str[0] == '\"')
        {
            trimmed = GetSubString(str, 1, str.GetSize() - 2).GetValue();
        }
        DynamicArray<StringUtf8> values(GetScratchAllocator());
        SplitToArray<StringUtf8>(trimmed, ",", values);
        for (const auto& value : values)
        {
            if (!IsValueAllowed(value))
            {
                throw InvalidArgumentException(__FUNCTION__, "value is not one of the possible values");
            }
            destination->PushBack(StringToU32(value));
        }
    }
};

template <>
struct TypedProgramArgumentDefinition<bool> final : ProgramArgumentDefinition
{
    bool* destination;

    TypedProgramArgumentDefinition(bool* dest, const ProgramArgumentDefinitionDesc& desc)
        : ProgramArgumentDefinition(desc), destination(dest)
    {
    }

    void SetValue(const StringUtf8&) override { *destination = true; }
};

struct OPAL_EXPORT ProgramArgumentsBuilder
{
    ProgramArgumentsBuilder& AddProgramDescription(const StringUtf8& description);
    ProgramArgumentsBuilder& AddUsageExample(const StringUtf8& example);

    template <typename T>
    ProgramArgumentsBuilder& AddArgumentDefinition(T& field, const ProgramArgumentDefinitionDesc& desc)
    {
        if (desc.name.IsEmpty())
        {
            throw InvalidArgumentException(__FUNCTION__, "name of the argument can't be empty");
        }
        if (desc.desc.IsEmpty())
        {
            throw InvalidArgumentException(__FUNCTION__, "description of the argument can't be empty");
        }
        AllocatorBase* allocator = GetDefaultAllocator();
        ScopePtr<ProgramArgumentDefinition> arg_def(allocator, New<TypedProgramArgumentDefinition<T>>(allocator, &field, desc));
        m_argument_definitions.PushBack(std::move(arg_def));
        if (desc.is_optional)
        {
            m_has_optional_argument = true;
        }
        else
        {
            m_has_required_argument = true;
        }
        return *this;
    }

    bool Build(const char** arguments, u32 count);

private:
    void ShowHelp();

    StringUtf8 m_program_description;
    DynamicArray<StringUtf8> m_usage_examples;
    DynamicArray<ScopePtr<ProgramArgumentDefinition>> m_argument_definitions;
    bool m_has_required_argument = false;
    bool m_has_optional_argument = false;
};

}  // namespace Opal

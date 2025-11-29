#pragma once

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
};

struct OPAL_EXPORT ProgramArgumentDefinition
{
    StringUtf8 name;
    StringUtf8 description;
    bool is_optional;

    ProgramArgumentDefinition(const ProgramArgumentDefinitionDesc& desc)
        : name(desc.name), description(desc.desc), is_optional(desc.is_optional)
    {
    }

    virtual ~ProgramArgumentDefinition() = default;
    virtual void SetValue(const StringUtf8& str) = 0;
};

template class OPAL_EXPORT DynamicArray<ProgramArgumentDefinition*>;

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
        SplitToArray<StringUtf8>(trimmed, ",", *destination);
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

    void SetValue(const StringUtf8& str) override { *destination = StringToI32(str); }
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

    void SetValue(const StringUtf8& str) override { *destination = StringToU32(str); }
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
        ProgramArgumentDefinition* arg_def = New<TypedProgramArgumentDefinition<T>>(&field, desc);
        m_argument_definitions.PushBack(arg_def);
        return *this;
    }

    bool Build(const char** arguments, u32 count);

private:
    DynamicArray<ProgramArgumentDefinition*> m_argument_definitions;
};

}  // namespace Opal

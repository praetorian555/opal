#include "opal/program-arguments.h"

#include "opal/container/hash-map.h"
#include "opal/hash.h"

template <>
struct std::hash<Opal::StringUtf8>
{
    std::size_t operator()(const Opal::StringUtf8& key) const noexcept
    {
        return Opal::Hash::CalcRange(key);
    }
};

bool Opal::ProgramArgumentsBuilder::Build(const char** arguments, u32 count)
{
    DynamicArray<StringUtf8> names(GetScratchAllocator());
    DynamicArray<StringUtf8> values(GetScratchAllocator());

    for (u32 i = 1; i < count; ++i)
    {
        StringUtf8 name(GetScratchAllocator());
        StringUtf8 value(GetScratchAllocator());
        Split<StringUtf8>(arguments[i], "=", name, value);
        names.PushBack(name);
        values.PushBack(value);
    }

    HashMap<StringUtf8, bool> visited;
    for (u32 i = 0; i < names.GetSize(); ++i)
    {
        const StringUtf8& name = names[i];
        for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
        {
            ProgramArgumentDefinition* arg_def = m_argument_definitions[j];
            if (arg_def->name == name)
            {
                arg_def->SetValue(values[i]);
                visited[arg_def->name] = true;
            }
        }
    }

    for (ProgramArgumentDefinition* def : m_argument_definitions)
    {
        if (!def->is_optional && !visited[def->name])
        {
            // TODO: Print help
            return false;
        }
    }
    return true;
}
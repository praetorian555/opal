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

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddProgramDescription(const StringUtf8& description)
{
    m_program_description = description;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddUsageExample(const StringUtf8& example)
{
    m_usage_examples.PushBack(example);
    return *this;
}

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

    for (const auto& name : names)
    {
        if (name == "help" || name == "--help")
        {
            ShowHelp();
            return false;
        }
    }

    HashMapDeprecated<StringUtf8, bool> visited;
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
                break;
            }

        }
    }

    for (ProgramArgumentDefinition* def : m_argument_definitions)
    {
        if (!def->is_optional && !visited[def->name])
        {
            printf("Required argument '%s' not provided, here is the information on how to use the program:\n\n", *def->name);
            ShowHelp();
            return false;
        }
    }
    return true;
}

void Opal::ProgramArgumentsBuilder::ShowHelp()
{
    printf("%s\n\n", *m_program_description);
    if (!m_usage_examples.IsEmpty())
    {
        printf("Usage examples:\n");
        for (const StringUtf8& example : m_usage_examples)
        {
            printf("\t%s\n", *example);
        }
        printf("\n");
    }
    if (m_has_required_argument)
    {
        printf("Required arguments:\n");
        for (const ProgramArgumentDefinition* def : m_argument_definitions)
        {
            if (!def->is_optional)
            {
                printf("\t%-30s%s\n", *def->name, *def->description);
            }
        }
        printf("\n");
    }
    if (m_has_optional_argument)
    {
        printf("Optional arguments:\n");
        for (const ProgramArgumentDefinition* def : m_argument_definitions)
        {
            if (def->is_optional)
            {
                printf("\t%-30s%s\n", *def->name, *def->description);
            }
        }
        printf("\n");
    }
}
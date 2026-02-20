#include "opal/program-arguments.h"

#include "opal/container/hash-map.h"
#include "opal/hash.h"

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

    HashMap<StringUtf8, bool> visited;
    for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
    {
        ProgramArgumentDefinition* arg_def = m_argument_definitions[j].Get();
        visited.Insert(arg_def->name, false);
    }
    for (u32 i = 0; i < names.GetSize(); ++i)
    {
        const StringUtf8& name = names[i];
        for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
        {
            ProgramArgumentDefinition* arg_def = m_argument_definitions[j].Get();
            if (arg_def->name == name)
            {
                arg_def->SetValue(values[i]);
                visited.Insert(arg_def->name, true);
                break;
            }
        }
    }

    for (const auto& def : m_argument_definitions)
    {
        if (!def->is_optional && !visited.GetValue(def->name))
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
        for (const auto& def : m_argument_definitions)
        {
            if (!def->is_optional)
            {
                if (def->possible_values.IsEmpty())
                {
                    printf("\t%-30s%s\n", *def->name, *def->description);
                }
                else
                {
                    StringUtf8 values_str;
                    for (u32 i = 0; i < def->possible_values.GetSize(); ++i)
                    {
                        if (i > 0)
                        {
                            values_str += ", ";
                        }
                        values_str += def->possible_values[i];
                    }
                    printf("\t%-30s%s (values: %s)\n", *def->name, *def->description, *values_str);
                }
            }
        }
        printf("\n");
    }
    if (m_has_optional_argument)
    {
        printf("Optional arguments:\n");
        for (const auto& def : m_argument_definitions)
        {
            if (def->is_optional)
            {
                if (def->possible_values.IsEmpty())
                {
                    printf("\t%-30s%s\n", *def->name, *def->description);
                }
                else
                {
                    StringUtf8 values_str;
                    for (u32 i = 0; i < def->possible_values.GetSize(); ++i)
                    {
                        if (i > 0)
                        {
                            values_str += ", ";
                        }
                        values_str += def->possible_values[i];
                    }
                    printf("\t%-30s%s (values: %s)\n", *def->name, *def->description, *values_str);
                }
            }
        }
        printf("\n");
    }
}
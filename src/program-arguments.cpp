#include "opal/program-arguments.h"

#include "opal/container/hash-map.h"
#include "opal/logging.h"

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

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::SetVersion(u32 major, u32 minor, u32 patch)
{
    m_major_version = major;
    m_minor_version = minor;
    m_patch_version = patch;
    return *this;
}

void Opal::ProgramArgumentsBuilder::Build(const char** arguments, u32 count)
{
    DynamicArray<StringUtf8> names;
    DynamicArray<StringUtf8> values;

    GetLogger().RegisterCategory("ProgramArguments", LogLevel::Info);

    for (u32 i = 1; i < count; ++i)
    {
        StringUtf8 name;
        StringUtf8 value;
        Split<StringUtf8>(arguments[i], "=", name, value);
        names.PushBack(name);
        values.PushBack(value);
    }

    for (const auto& name : names)
    {
        if (name == "help" || name == "--help")
        {
            ShowHelp();
            throw HelpRequestedException();
        }
        if (name == "version" || name == "--version")
        {
            ShowVersion(arguments[0]);
            throw VersionRequestedException();
        }
    }

    HashMap<StringUtf8, bool> visited;
    for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
    {
        const Impl::ProgramArgumentDefinition* arg_def = m_argument_definitions[j].Get();
        visited.Insert(arg_def->m_name, false);
    }
    for (u32 i = 0; i < names.GetSize(); ++i)
    {
        const StringUtf8& name = names[i];
        for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
        {
            Impl::ProgramArgumentDefinition* arg_def = m_argument_definitions[j].Get();
            if (arg_def->m_name == name)
            {
                arg_def->SetValue(values[i]);
                visited.Insert(arg_def->m_name, true);
                break;
            }
        }
    }

    for (const auto& def : m_argument_definitions)
    {
        if (!def->m_is_optional && !visited.GetValue(def->m_name))
        {
            GetLogger().Error("ProgramArguments",
                              "Required argument '%s' not provided, here is the information on how to use the program:\n\n", *def->m_name);
            ShowHelp();
            throw InvalidArgumentException(__FUNCTION__, "required argument not provided");
        }
    }
}

void Opal::ProgramArgumentsBuilder::ShowHelp()
{
    GetLogger().Info("ProgramArguments", "%s\n\n", *m_program_description);
    if (!m_usage_examples.IsEmpty())
    {
        GetLogger().Info("ProgramArguments", "Usage examples:\n");
        for (const StringUtf8& example : m_usage_examples)
        {
            GetLogger().Info("ProgramArguments", "\t%s\n", *example);
        }
        GetLogger().Info("ProgramArguments", "\n");
    }
    if (m_has_required_argument)
    {
        GetLogger().Info("ProgramArguments", "Required arguments:\n");
        for (const auto& def : m_argument_definitions)
        {
            if (!def->m_is_optional)
            {
                // if (def->m_possible_values.IsEmpty())
                // {
                //     GetLogger().Info("ProgramArguments", "\t%-30s%s\n", *def->name, *def->description);
                // }
                // else
                // {
                //     StringUtf8 values_str;
                //     for (u32 i = 0; i < def->possible_values.GetSize(); ++i)
                //     {
                //         if (i > 0)
                //         {
                //             values_str += ", ";
                //         }
                //         values_str += def->possible_values[i];
                //     }
                //     GetLogger().Info("ProgramArguments", "\t%-30s%s (values: %s)\n", *def->name, *def->description, *values_str);
                // }
            }
        }
        GetLogger().Info("ProgramArguments", "\n");
    }
    if (m_has_optional_argument)
    {
        GetLogger().Info("ProgramArguments", "Optional arguments:\n");
        for (const auto& def : m_argument_definitions)
        {
            if (def->m_is_optional)
            {
                // if (def->possible_values.IsEmpty())
                // {
                //     GetLogger().Info("ProgramArguments", "\t%-30s%s\n", *def->m_name, *def->m_description);
                // }
                // else
                // {
                //     StringUtf8 values_str;
                //     for (u32 i = 0; i < def->possible_values.GetSize(); ++i)
                //     {
                //         if (i > 0)
                //         {
                //             values_str += ", ";
                //         }
                //         values_str += def->possible_values[i];
                //     }
                //     GetLogger().Info("ProgramArguments", "\t%-30s%s (values: %s)\n", *def->m_name, *def->m_description, *values_str);
                // }
            }
        }
        GetLogger().Info("ProgramArguments", "\n");
    }
}

void Opal::ProgramArgumentsBuilder::ShowVersion(const char* program_name)
{
    Opal::GetLogger().Info("ProgramArguments", "%s v{}.{}.{}\n", program_name, m_major_version, m_minor_version, m_patch_version);
}
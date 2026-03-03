#include "opal/program-arguments.h"

#include "opal/container/hash-map.h"
#include "opal/logging.h"

static const char* SafeCStr(const Opal::StringUtf8& str)
{
    const char* ptr = *str;
    return ptr ? ptr : "";
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddProgramDescription(StringUtf8 description)
{
    m_program_description = std::move(description);
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddUsageExample(StringUtf8 example)
{
    m_usage_examples.PushBack(std::move(example));
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
        names.PushBack(std::move(name));
        values.PushBack(std::move(value));
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
        visited.Insert(arg_def->m_name.Clone(), false);
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
                visited.Insert(arg_def->m_name.Clone(), true);
                break;
            }
        }
    }

    for (const auto& def : m_argument_definitions)
    {
        if (!def->m_is_optional && !visited.GetValue(def->m_name))
        {
            GetLogger().Error("ProgramArguments",
                              "Required argument '{}' not provided, here is the information on how to use the program:\n\n", SafeCStr(def->m_name));
            ShowHelp();
            throw InvalidArgumentException(__FUNCTION__, "required argument not provided");
        }
    }
}

void Opal::ProgramArgumentsBuilder::ShowHelp()
{
    GetLogger().Info("ProgramArguments", "{}", SafeCStr(m_program_description));
    if (!m_usage_examples.IsEmpty())
    {
        GetLogger().Info("ProgramArguments", "Usage examples:");
        for (const StringUtf8& example : m_usage_examples)
        {
            GetLogger().Info("ProgramArguments", "\t{}", SafeCStr(example));
        }
    }
    // ReSharper disable once CppDFAConstantConditions
    if (m_has_required_argument)
    {
        GetLogger().Info("ProgramArguments", "Required arguments:");
        for (const auto& def : m_argument_definitions)
        {
            if (!def->m_is_optional)
            {
                StringUtf8 format_str("\t{:<30}{}");
                DynamicArray<StringUtf8> allowed = def->GetAllowedValues();
                if (!allowed.IsEmpty())
                {
                    format_str += " (allowed values: {})";
                    StringUtf8 values_str;
                    for (u32 i = 0; i < allowed.GetSize(); ++i)
                    {
                        if (i > 0)
                            values_str += ", ";
                        values_str += allowed[i];
                    }
                    GetLogger().Info("ProgramArguments", format_str, SafeCStr(def->m_name), SafeCStr(def->m_description), SafeCStr(values_str));
                }
                else
                {
                    GetLogger().Info("ProgramArguments", format_str, SafeCStr(def->m_name), SafeCStr(def->m_description));
                }
            }
        }
        GetLogger().Info("ProgramArguments", "");
    }
    // ReSharper disable once CppDFAConstantConditions
    if (m_has_optional_argument)
    {
        GetLogger().Info("ProgramArguments", "Optional arguments:");
        for (const auto& def : m_argument_definitions)
        {
            if (def->m_is_optional)
            {
                StringUtf8 format_str("\t{:<30}{}");
                DynamicArray<StringUtf8> allowed = def->GetAllowedValues();
                if (!allowed.IsEmpty())
                {
                    format_str += " (allowed values: {})";
                    StringUtf8 values_str;
                    for (u32 i = 0; i < allowed.GetSize(); ++i)
                    {
                        if (i > 0)
                            values_str += ", ";
                        values_str += allowed[i];
                    }
                    GetLogger().Info("ProgramArguments", format_str, SafeCStr(def->m_name), SafeCStr(def->m_description), SafeCStr(values_str));
                }
                else
                {
                    GetLogger().Info("ProgramArguments", format_str, SafeCStr(def->m_name), SafeCStr(def->m_description));
                }
            }
        }
    }
}

void Opal::ProgramArgumentsBuilder::ShowVersion(const char* program_name)
{
    Opal::GetLogger().Info("ProgramArguments", "{} v{}.{}.{}", program_name, m_major_version, m_minor_version, m_patch_version);
}
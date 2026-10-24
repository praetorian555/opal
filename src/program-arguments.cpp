#include "opal/program-arguments.h"

#include "opal/container/hash-map.h"
#include "opal/container/string-format.h"
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

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AllowUnknownArguments()
{
    m_allow_unknown_arguments = true;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::SetSilent(bool silent)
{
    m_silent = silent;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddPositional(StringUtf8 name, StringUtf8 desc,
                                                                            Ref<DynamicArray<StringUtf8>> destination, bool is_optional)
{
    if (name.IsEmpty() || desc.IsEmpty())
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "a positional argument needs a name and a description"));
    }
    if (!m_commands.IsEmpty())
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "a program with commands can't have a positional argument"));
    }
    m_positional_name = std::move(name);
    m_positional_description = std::move(desc);
    m_positional_destination = destination.GetPtr();
    m_positional_is_optional = is_optional;
    m_has_required_argument = m_has_required_argument || !is_optional;
    m_has_optional_argument = m_has_optional_argument || is_optional;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddPassthrough(StringUtf8 desc, Ref<DynamicArray<StringUtf8>> destination)
{
    if (desc.IsEmpty())
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "description of the argument can't be empty"));
    }
    m_passthrough_description = std::move(desc);
    m_passthrough_destination = destination.GetPtr();
    m_has_optional_argument = true;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddPrefixArgument(StringUtf8 prefix, StringUtf8 desc,
                                                                                Ref<DynamicArray<StringUtf8>> destination)
{
    if (prefix.IsEmpty() || desc.IsEmpty())
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "a prefix argument needs a prefix and a description"));
    }
    m_prefix_arguments.PushBack(PrefixArgument{std::move(prefix), std::move(desc), destination.GetPtr()});
    m_has_optional_argument = true;
    return *this;
}

Opal::ProgramArgumentsBuilder& Opal::ProgramArgumentsBuilder::AddCommand(StringUtf8 name, StringUtf8 desc)
{
    if (name.IsEmpty() || desc.IsEmpty())
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "a command needs a name and a description"));
    }
    if (m_positional_destination != nullptr)
    {
        OPAL_RAISE(InvalidArgumentException(__FUNCTION__, "a program with commands can't have a positional argument"));
    }
    m_commands.PushBack(Command{std::move(name), std::move(desc)});
    return *this;
}

bool Opal::ProgramArgumentsBuilder::IsProvided(const StringUtf8& name) const
{
    for (const StringUtf8& provided : m_provided)
    {
        if (provided == name)
        {
            return true;
        }
    }
    return false;
}

const Opal::StringUtf8& Opal::ProgramArgumentsBuilder::GetError() const
{
    return m_error;
}

const Opal::StringUtf8& Opal::ProgramArgumentsBuilder::GetCommand() const
{
    return m_command;
}

Opal::u32 Opal::ProgramArgumentsBuilder::GetCommandIndex() const
{
    return m_command_index;
}

Opal::ProgramArgumentsResult Opal::ProgramArgumentsBuilder::Build(const char** arguments, u32 count)
{
    DynamicArray<StringViewUtf8> views;
    for (u32 i = 1; i < count; ++i)
    {
        if (views.PushBack(StringViewUtf8(arguments[i])) != ErrorCode::Success) [[unlikely]]
        {
            return ProgramArgumentsResult::OutOfMemory;
        }
    }
    const ProgramArgumentsResult result =
        Build(count > 0 ? arguments[0] : "", ArrayView<const StringViewUtf8>(views.GetData(), views.GetSize()));
    // The program's name comes first in main's arguments.
    if (!m_command.IsEmpty())
    {
        ++m_command_index;
    }
    return result;
}

Opal::ProgramArgumentsResult Opal::ProgramArgumentsBuilder::Build(const char* program_name, ArrayView<const StringViewUtf8> arguments)
{
    m_error.Clear();
    m_provided.Clear();
    m_command.Clear();
    m_command_index = 0;
    GetLogger().RegisterCategory("ProgramArguments", LogLevel::Info);

    const auto is_command = [this](const StringUtf8& argument)
    {
        for (const Command& command : m_commands)
        {
            if (command.name == argument)
            {
                return true;
            }
        }
        return false;
    };

    // With a positional argument, a bare "help" or "version" may be a value, so only the dashed forms are reserved.
    for (const StringViewUtf8& view : arguments)
    {
        const StringUtf8 argument(view.GetData(), view.GetSize());
        if ((m_passthrough_destination != nullptr && argument == "--") || is_command(argument))
        {
            break;
        }
        const bool bare_reserved = m_positional_destination == nullptr;
        if (argument == "--help" || (bare_reserved && argument == "help"))
        {
            if (!m_silent)
            {
                ShowHelp();
            }
            return ProgramArgumentsResult::HelpRequested;
        }
        if (argument == "--version" || (bare_reserved && argument == "version"))
        {
            if (!m_silent)
            {
                ShowVersion(program_name);
            }
            return ProgramArgumentsResult::VersionRequested;
        }
    }

    const auto fail = [this](StringUtf8 message)
    {
        m_error = std::move(message);
        if (!m_silent)
        {
            GetLogger().Error("ProgramArguments", "{}, here is the information on how to use the program:\n\n", SafeCStr(m_error));
            ShowHelp();
        }
        return ProgramArgumentsResult::InvalidArgument;
    };
    const auto provide = [this](const StringUtf8& name)
    { return IsProvided(name) ? ErrorCode::Success : m_provided.PushBack(name.Clone()); };

    for (u64 i = 0; i < arguments.GetSize(); ++i)
    {
        const StringUtf8 argument(arguments[i].GetData(), arguments[i].GetSize());
        if (m_passthrough_destination != nullptr && argument == "--")
        {
            for (u64 j = i + 1; j < arguments.GetSize(); ++j)
            {
                if (m_passthrough_destination->PushBack(StringUtf8(arguments[j].GetData(), arguments[j].GetSize())) != ErrorCode::Success)
                    [[unlikely]]
                {
                    return ProgramArgumentsResult::OutOfMemory;
                }
            }
            break;
        }

        bool handled = false;
        for (const PrefixArgument& prefix : m_prefix_arguments)
        {
            if (argument.GetSize() > prefix.prefix.GetSize() && StartsWith(argument, prefix.prefix))
            {
                Expected<StringUtf8, ErrorCode> rest =
                    GetSubString(argument, prefix.prefix.GetSize(), argument.GetSize() - prefix.prefix.GetSize());
                if (!rest.HasValue() || prefix.destination->PushBack(std::move(rest.GetValue())) != ErrorCode::Success ||
                    provide(prefix.prefix) != ErrorCode::Success) [[unlikely]]
                {
                    return ProgramArgumentsResult::OutOfMemory;
                }
                handled = true;
                break;
            }
        }
        if (handled)
        {
            continue;
        }

        StringUtf8 name;
        StringUtf8 value;
        Split<StringUtf8>(argument, "=", name, value);
        for (u32 j = 0; j < m_argument_definitions.GetSize(); ++j)
        {
            Impl::ProgramArgumentDefinition* arg_def = m_argument_definitions[j].Get();
            if (arg_def->m_name != name)
            {
                continue;
            }
            const ErrorCode error = arg_def->SetValue(value);
            if (error == ErrorCode::OutOfMemory) [[unlikely]]
            {
                return ProgramArgumentsResult::OutOfMemory;
            }
            if (error != ErrorCode::Success)
            {
                StringUtf8 message;
                (void)AppendFormat(message, "value '{}' is not valid for argument '{}'", SafeCStr(value), SafeCStr(name));
                const DynamicArray<StringUtf8> allowed = arg_def->GetAllowedValues();
                for (u64 k = 0; k < allowed.GetSize(); ++k)
                {
                    (void)AppendFormat(message, "{}{}", k == 0 ? "; allowed values: " : ", ", SafeCStr(allowed[k]));
                }
                return fail(std::move(message));
            }
            if (provide(arg_def->m_name) != ErrorCode::Success) [[unlikely]]
            {
                return ProgramArgumentsResult::OutOfMemory;
            }
            handled = true;
            break;
        }
        if (handled)
        {
            continue;
        }

        if (!m_commands.IsEmpty() && !StartsWith(argument, '-'))
        {
            if (!is_command(argument))
            {
                StringUtf8 message;
                (void)AppendFormat(message, "unknown command '{}'", SafeCStr(argument));
                return fail(std::move(message));
            }
            m_command = argument.Clone();
            m_command_index = static_cast<u32>(i);
            break;
        }
        if (m_positional_destination != nullptr && !StartsWith(argument, '-'))
        {
            if (m_positional_destination->PushBack(argument.Clone()) != ErrorCode::Success ||
                provide(m_positional_name) != ErrorCode::Success) [[unlikely]]
            {
                return ProgramArgumentsResult::OutOfMemory;
            }
            continue;
        }
        if (!m_allow_unknown_arguments)
        {
            StringUtf8 message;
            (void)AppendFormat(message, "unknown argument '{}'", SafeCStr(argument));
            return fail(std::move(message));
        }
    }

    for (const auto& def : m_argument_definitions)
    {
        if (!def->m_is_optional && !IsProvided(def->m_name))
        {
            StringUtf8 message;
            (void)AppendFormat(message, "required argument '{}' not provided", SafeCStr(def->m_name));
            return fail(std::move(message));
        }
    }
    if (m_positional_destination != nullptr && !m_positional_is_optional && !IsProvided(m_positional_name))
    {
        StringUtf8 message;
        (void)AppendFormat(message, "required argument '{}' not provided", SafeCStr(m_positional_name));
        return fail(std::move(message));
    }
    if (!m_commands.IsEmpty() && m_command.IsEmpty())
    {
        return fail(StringUtf8("no command given"));
    }
    return ProgramArgumentsResult::Success;
}

Opal::StringUtf8 Opal::ProgramArgumentsBuilder::GetHelpText() const
{
    StringUtf8 text;
    (void)AppendFormat(text, "{}\n", SafeCStr(m_program_description));
    if (!m_usage_examples.IsEmpty())
    {
        (void)AppendFormat(text, "Usage examples:\n");
        for (const StringUtf8& example : m_usage_examples)
        {
            (void)AppendFormat(text, "\t{}\n", SafeCStr(example));
        }
    }
    const auto line = [&text](const StringUtf8& name, const StringUtf8& description, const DynamicArray<StringUtf8>& allowed)
    {
        (void)AppendFormat(text, "\t{:<30}{}", SafeCStr(name), SafeCStr(description));
        for (u64 i = 0; i < allowed.GetSize(); ++i)
        {
            (void)AppendFormat(text, "{}{}", i == 0 ? " (allowed values: " : ", ", SafeCStr(allowed[i]));
        }
        (void)AppendFormat(text, "{}\n", allowed.IsEmpty() ? "" : ")");
    };
    if (!m_commands.IsEmpty())
    {
        (void)AppendFormat(text, "Commands:\n");
        for (const Command& command : m_commands)
        {
            line(command.name, command.description, {});
        }
        (void)AppendFormat(text, "\n");
    }
    StringUtf8 positional_label;
    if (m_positional_destination != nullptr)
    {
        (void)AppendFormat(positional_label, "<{}>...", SafeCStr(m_positional_name));
    }
    for (const bool optional : {false, true})
    {
        if (!(optional ? m_has_optional_argument : m_has_required_argument))
        {
            continue;
        }
        (void)AppendFormat(text, "{}", optional ? "Optional arguments:\n" : "Required arguments:\n");
        for (const auto& def : m_argument_definitions)
        {
            if (def->m_is_optional == optional)
            {
                line(def->m_name, def->m_description, def->GetAllowedValues());
            }
        }
        if (m_positional_destination != nullptr && m_positional_is_optional == optional)
        {
            line(positional_label, m_positional_description, {});
        }
        if (optional)
        {
            for (const PrefixArgument& prefix : m_prefix_arguments)
            {
                StringUtf8 label;
                (void)AppendFormat(label, "{}...", SafeCStr(prefix.prefix));
                line(label, prefix.description, {});
            }
            if (m_passthrough_destination != nullptr)
            {
                line(StringUtf8("-- <argument>..."), m_passthrough_description, {});
            }
        }
        if (!optional)
        {
            (void)AppendFormat(text, "\n");
        }
    }
    return text;
}

void Opal::ProgramArgumentsBuilder::ShowHelp()
{
    const StringUtf8 text = GetHelpText();
    StringUtf8 line;
    for (u64 i = 0; i < text.GetSize(); ++i)
    {
        if (text[i] == '\n')
        {
            GetLogger().Info("ProgramArguments", "{}", SafeCStr(line));
            line.Clear();
        }
        else
        {
            line += text[i];
        }
    }
}

void Opal::ProgramArgumentsBuilder::ShowVersion(const char* program_name)
{
    Opal::GetLogger().Info("ProgramArguments", "{} v{}.{}.{}", program_name, m_major_version, m_minor_version, m_patch_version);
}

#include "opal/logging.h"

#if OPAL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <chrono>
#include <cstdio>
#include <cstring>

#include "opal/threading/thread.h"

namespace
{

Opal::Logger* g_logger = nullptr;

struct TimeParts
{
    char date[16];
    Opal::u64 date_len;
    char time[16];
    Opal::u64 time_len;
};

TimeParts GetTimeParts()
{
    TimeParts parts = {};
    const auto now = std::chrono::system_clock::now();
    const auto time_t_now = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm tm_buf = {};
#if defined(OPAL_PLATFORM_WINDOWS)
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    parts.date_len = static_cast<Opal::u64>(
        snprintf(parts.date, sizeof(parts.date), "%04d-%02d-%02d", tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday));
    parts.time_len = static_cast<Opal::u64>(snprintf(parts.time, sizeof(parts.time), "%02d:%02d:%02d.%03d", tm_buf.tm_hour, tm_buf.tm_min,
                                                     tm_buf.tm_sec, static_cast<int>(ms.count())));
    return parts;
}

}  // namespace

const char* Opal::LogLevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Verbose:
            return "Verbose";
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
        case LogLevel::Off:
            return "Off";
    }
    return "Unknown";
}

/*************************************************************************************************/
/** ConsoleSink **********************************************************************************/
/*************************************************************************************************/

Opal::ConsoleSink::ConsoleSink() : m_stdout(stdout) {}

void Opal::ConsoleSink::Write(LogLevel /*level*/, StringViewUtf8 /*category*/, StringViewUtf8 formatted_message)
{
    auto guard = m_stdout.Lock();
    fwrite(formatted_message.GetData(), 1, formatted_message.GetSize(), *guard.Deref());
}

void Opal::ConsoleSink::Flush()
{
    auto guard = m_stdout.Lock();
    fflush(*guard.Deref());
}

/*************************************************************************************************/
/** Logger ***************************************************************************************/
/*************************************************************************************************/

Opal::Logger::Logger() : m_pattern("[<date> <time>][<thread>][<level>][<category>] <message>\n")
{
    RegisterCategory("General", LogLevel::Info);
}

Opal::Logger::~Logger() = default;

void Opal::Logger::SetPattern(StringViewUtf8 pattern)
{
    m_pattern = StringUtf8(pattern.GetData(), pattern.GetSize());
}

Opal::StringViewUtf8 Opal::Logger::GetPattern() const
{
    return StringViewUtf8(m_pattern.GetData(), m_pattern.GetSize());
}

void Opal::Logger::RegisterCategory(StringViewUtf8 category, LogLevel level)
{
    StringUtf8 key(category.GetData(), category.GetSize());
    m_categories.Insert(std::move(key), level);
}

bool Opal::Logger::IsCategoryRegistered(StringViewUtf8 category) const
{
    StringUtf8 key(category.GetData(), category.GetSize());
    return m_categories.Contains(key);
}

void Opal::Logger::SetCategoryLevel(StringViewUtf8 category, LogLevel level)
{
    StringUtf8 key(category.GetData(), category.GetSize());
    OPAL_ASSERT(m_categories.Contains(key), "Category not registered");
    m_categories.GetValue(key) = level;
}

Opal::LogLevel Opal::Logger::GetCategoryLevel(StringViewUtf8 category) const
{
    StringUtf8 key(category.GetData(), category.GetSize());
    return m_categories.GetValue(key);
}

void Opal::Logger::AddSink(const SharedPtr<LogSink>& sink)
{
    m_sinks.PushBack(sink.Clone());
}

void Opal::Logger::RemoveSink(const SharedPtr<LogSink>& sink)
{
    for (auto it = m_sinks.begin(); it != m_sinks.end(); ++it)
    {
        if (*it == sink)
        {
            m_sinks.Erase(it);
            return;
        }
    }
}

void Opal::Logger::ClearSinks()
{
    m_sinks.Clear();
}

void Opal::Logger::Flush()
{
    for (u64 i = 0; i < m_sinks.GetSize(); ++i)
    {
        m_sinks[i]->Flush();
    }
}

void Opal::Logger::Emit(LogLevel level, StringViewUtf8 category, StringViewUtf8 message)
{
    constexpr u64 k_buffer_size = 2048;
    char8 buffer[k_buffer_size];
    char8* out = buffer;
    char8* out_end = buffer + k_buffer_size;

    const TimeParts time_parts = GetTimeParts();
    const char* level_str = LogLevelToString(level);
    ;
    const u64 thread_id = reinterpret_cast<u64>(GetCurrentThreadHandle().native_id);
    constexpr size_t k_thread_id_size = 16;
    char thread_id_str[k_thread_id_size] = "";
    snprintf(thread_id_str, k_thread_id_size, "%llu", thread_id);
    const size_t thread_id_len = strlen(thread_id_str);

    struct Specifier
    {
        const char* tag;
        u64 tag_len;
        const char* value;
        u64 value_len;
    };

    const u64 level_str_len = static_cast<u64>(strlen(level_str));
    const Specifier specifiers[] = {
        {"<date>", 6, time_parts.date, time_parts.date_len},
        {"<time>", 6, time_parts.time, time_parts.time_len},
        {"<thread>", 8, thread_id_str, thread_id_len},
        {"<level>", 7, level_str, level_str_len},
        {"<category>", 10, category.GetData(), category.GetSize()},
        {"<message>", 9, message.GetData(), message.GetSize()},
    };
    constexpr u64 k_specifier_count = 6;

    const char* pat = m_pattern.GetData();
    const char* pat_end = pat + m_pattern.GetSize();

    while (pat < pat_end && out < out_end)
    {
        if (*pat == '<')
        {
            bool matched = false;
            for (u64 s = 0; s < k_specifier_count; ++s)
            {
                const u64 remaining = static_cast<u64>(pat_end - pat);
                if (remaining >= specifiers[s].tag_len && memcmp(pat, specifiers[s].tag, specifiers[s].tag_len) == 0)
                {
                    const u64 copy_len = (specifiers[s].value_len < static_cast<u64>(out_end - out)) ? specifiers[s].value_len
                                                                                                     : static_cast<u64>(out_end - out);
                    memcpy(out, specifiers[s].value, copy_len);
                    out += copy_len;
                    pat += specifiers[s].tag_len;
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                *out++ = *pat++;
            }
        }
        else
        {
            *out++ = *pat++;
        }
    }

    const u64 length = static_cast<u64>(out - buffer);
    const StringViewUtf8 formatted_view(buffer, length);

    for (u64 i = 0; i < m_sinks.GetSize(); ++i)
    {
        m_sinks[i]->Write(level, category, formatted_view);
    }
}

void Opal::Logger::HandleFatal()
{
    Flush();
    throw FatalLogException("Fatal log message encountered");
}

/*************************************************************************************************/
/** Global access ********************************************************************************/
/*************************************************************************************************/

Opal::Logger& Opal::GetLogger()
{
    if (g_logger == nullptr)
    {
        throw LoggerNotInitializedException();
    }
    return *g_logger;
}

void Opal::SetLogger(Logger* logger)
{
    g_logger = logger;
}

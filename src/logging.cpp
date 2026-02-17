#include "opal/logging.h"

#include <chrono>
#include <cstdio>

#include "opal/threading/thread.h"

namespace
{

Opal::Logger* g_logger = nullptr;

Opal::u64 FormatTimestamp(char* buffer, Opal::u64 buffer_size)
{
    const auto now = std::chrono::system_clock::now();
    const auto time_t_now = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm tm_buf = {};
#if defined(OPAL_PLATFORM_WINDOWS)
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    const int written = snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                                 tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                                 tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, static_cast<int>(ms.count()));
    return static_cast<Opal::u64>(written);
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

Opal::Logger::Logger()
{
    RegisterCategory("General", LogLevel::Info);
}

Opal::Logger::~Logger() = default;

void Opal::Logger::RegisterCategory(StringViewUtf8 category, LogLevel level)
{
    StringUtf8 key(category.GetData(), category.GetSize());
    m_categories.Insert(Move(key), level);
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

    char8 timestamp[32];
    const u64 timestamp_len = FormatTimestamp(timestamp, sizeof(timestamp));
    const char* level_str = LogLevelToString(level);

    // Format: [TIMESTAMP] [LEVEL] [CATEGORY] Message\n
    auto result = std::format_to_n(buffer, k_buffer_size,
                                   "[{}][{}][{}][{}] {}\n",
                                   std::string_view(timestamp, timestamp_len),
                                   GetCurrentThreadHandle().native_id,
                                   level_str,
                                   std::string_view(category.GetData(), category.GetSize()),
                                   std::string_view(message.GetData(), message.GetSize()));
    const u64 length = static_cast<u64>(result.out - buffer);
    StringViewUtf8 formatted_view(buffer, length);

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

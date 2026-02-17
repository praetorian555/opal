#pragma once

#include <format>

#include "opal/assert.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/scope-ptr.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string.h"
#include "opal/container/string-view.h"
#include "opal/export.h"
#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

enum class LogLevel : u8
{
    Verbose,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
    Off,
};

struct OPAL_EXPORT LogSink
{
    virtual ~LogSink() = default;
    virtual void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) = 0;
    virtual void Flush() = 0;
};

class OPAL_EXPORT ConsoleSink : public LogSink
{
public:
    void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) override;
    void Flush() override;

private:
    Impl::PureMutex m_mutex;
};

struct FatalLogException : Exception
{
    FatalLogException(const char* message) : Exception(StringEx("Fatal log: ") + message) {}
};

OPAL_EXPORT const char* LogLevelToString(LogLevel level);

class OPAL_EXPORT Logger
{
public:
    Logger();
    ~Logger();

    void RegisterCategory(StringViewUtf8 category, LogLevel level = LogLevel::Info);
    bool IsCategoryRegistered(StringViewUtf8 category) const;
    void SetCategoryLevel(StringViewUtf8 category, LogLevel level);
    LogLevel GetCategoryLevel(StringViewUtf8 category) const;

    void AddSink(const SharedPtr<LogSink>& sink);
    void RemoveSink(const SharedPtr<LogSink>& sink);
    void ClearSinks();

    void Flush();

    template <typename... Args>
    void Log(LogLevel level, StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Verbose(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Debug(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Info(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Warning(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Error(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Fatal(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

private:
    void Emit(LogLevel level, StringViewUtf8 category, StringViewUtf8 message);
    void HandleFatal();

    HashMap<StringUtf8, LogLevel> m_categories;
    DynamicArray<SharedPtr<LogSink>> m_sinks;
};

OPAL_EXPORT Logger& GetLogger();
OPAL_EXPORT void SetLogger(ScopePtr<Logger> logger);

/*************************************************************************************************/
/** Template implementations *********************************************************************/
/*************************************************************************************************/

template <typename... Args>
void Logger::Log(LogLevel level, StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    if (!IsCategoryRegistered(category))
    {
        return;
    }
    if (level < GetCategoryLevel(category))
    {
        return;
    }
    if constexpr (sizeof...(Args) == 0)
    {
        Emit(level, category, fmt);
    }
    else
    {
        constexpr u64 k_buffer_size = 2048;
        char8 buffer[k_buffer_size];
        auto it = std::vformat_to(buffer, std::string_view(fmt.GetData(), fmt.GetSize()),
                                  std::make_format_args(args...));
        u64 length = static_cast<u64>(it - buffer);
        if (length > k_buffer_size)
        {
            length = k_buffer_size;
        }
        Emit(level, category, StringViewUtf8(buffer, length));
    }
    if (level == LogLevel::Fatal)
    {
        HandleFatal();
    }
}

template <typename... Args>
void Logger::Verbose(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Verbose, category, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Debug(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Debug, category, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Info(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Info, category, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Warning(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Warning, category, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Error(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Error, category, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Fatal(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Fatal, category, fmt, std::forward<Args>(args)...);
}

}  // namespace Opal

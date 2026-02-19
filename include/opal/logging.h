#pragma once

#include <cstdio>
#include <format>

#include "container/in-place-array.h"
#include "opal/assert.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
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
    ConsoleSink();
    void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) override;
    void Flush() override;

private:
    Mutex<FILE*> m_stdout;
};

struct FatalLogException : Exception
{
    FatalLogException(const char* message) : Exception(StringEx("Fatal log: ") + message) {}
};

struct LoggerNotInitializedException : Exception
{
    LoggerNotInitializedException() : Exception("Logger not initialized; call SetLogger before GetLogger") {}
};

struct UnregisteredCategoryException : Exception
{
    UnregisteredCategoryException(const char* category) : Exception(StringEx("Logging with unregistered category: ") + category) {}
};

OPAL_EXPORT const char* LogLevelToString(LogLevel level);

class OPAL_EXPORT Logger
{
public:
    static constexpr size_t k_max_message_size = 2048;

    Logger();
    ~Logger();

    void RegisterCategory(StringViewUtf8 category, LogLevel level = LogLevel::Info);
    bool IsCategoryRegistered(StringViewUtf8 category) const;
    void SetCategoryLevel(StringViewUtf8 category, LogLevel level);
    LogLevel GetCategoryLevel(StringViewUtf8 category) const;

    void SetPattern(StringViewUtf8 pattern);
    StringViewUtf8 GetPattern() const;

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
    StringUtf8 m_pattern;
};

OPAL_EXPORT Logger& GetLogger();
OPAL_EXPORT void SetLogger(Logger* logger);

/*************************************************************************************************/
/** Template implementations *********************************************************************/
/*************************************************************************************************/

struct BoundedFormatIterator
{
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    BoundedFormatIterator(InPlaceArray<char8, Logger::k_max_message_size>& out_buffer, size_t& out_size) : m_buffer(out_buffer), m_size(&out_size) {}

    BoundedFormatIterator& operator=(char c)
    {
        if (*m_size < m_buffer->GetSize())
        {
            m_buffer->At(*m_size).GetValue() = static_cast<char8>(c);
            *m_size += 1;
        }
        return *this;
    }

    BoundedFormatIterator& operator*() { return *this; }
    BoundedFormatIterator& operator++() { return *this; }
    BoundedFormatIterator operator++(int) { return *this; }

private:
    Ref<InPlaceArray<char8, Logger::k_max_message_size>> m_buffer;
    size_t* m_size;
};

template <typename... Args>
void Logger::Log(LogLevel level, StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    if (!IsCategoryRegistered(category))
    {
        throw UnregisteredCategoryException(category.GetData());
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
        InPlaceArray<char8, k_max_message_size> buffer;
        size_t written = 0;
        BoundedFormatIterator out(buffer, written);
        std::vformat_to(out, std::string_view(fmt.GetData(), fmt.GetSize()), std::make_format_args(args...));
        Emit(level, category, StringViewUtf8(buffer.GetData(), written));
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

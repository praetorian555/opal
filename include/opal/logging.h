#pragma once

#include <format>
#include <stdio.h>

#include "container/in-place-array.h"
#include "opal/assert.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/threading/mutex.h"
#include "opal/types.h"

namespace Opal
{

enum class LogLevel : u8
{
    Off = 0,
    Fatal,
    Error,
    Warning,
    Info,
    Verbose,
};

struct LogSink
{
    virtual ~LogSink() = default;
    virtual void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) = 0;
    virtual void Flush() = 0;
};

class ConsoleSink : public LogSink
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

/**
 * Called once the sinks have been flushed for a message logged at LogLevel::Fatal. It is not expected to return; the default one
 * throws FatalLogException.
 */
using FatalLogHandler = void (*)(StringViewUtf8 category, StringViewUtf8 message);

/**
 * Install the handler run after a fatal log message. Pass nullptr to restore the default.
 * @note Not thread-safe. Set it before the threads that log are started.
 */
void SetFatalLogHandler(FatalLogHandler handler);

/** @return The handler currently installed. Never nullptr. */
FatalLogHandler GetFatalLogHandler();

const char* LogLevelToString(LogLevel level);

class Logger
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

    void SetLogLevel(LogLevel level) { m_log_level = level; }

    void AddSink(const SharedPtr<LogSink>& sink);
    void RemoveSink(const SharedPtr<LogSink>& sink);
    void ClearSinks();

    void Flush();

    /**
     * Format and write a message to every sink.
     * @note Registering a category only sets a level for it. A message logged to an unregistered category is written, gated by the
     *       logger's own level alone.
     */
    template <typename... Args>
    void Log(LogLevel level, StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

    template <typename... Args>
    void Verbose(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args);

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
    void HandleFatal(StringViewUtf8 category, StringViewUtf8 message);

    LogLevel m_log_level;
    HashMap<StringUtf8, LogLevel> m_categories;
    DynamicArray<SharedPtr<LogSink>> m_sinks;
    StringUtf8 m_pattern;
};

Logger& GetLogger();
void SetLogger(Logger* logger);

/*************************************************************************************************/
/** Template implementations *********************************************************************/
/*************************************************************************************************/

struct BoundedFormatIterator
{
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = void;

    BoundedFormatIterator(InPlaceArray<char8, Logger::k_max_message_size>& out_buffer, size_t& out_size) : m_buffer(out_buffer), m_size(&out_size) {}

    BoundedFormatIterator& operator=(char c)
    {
        if (*m_size < m_buffer->GetSize())
        {
            m_buffer->At(*m_size) = static_cast<char8>(c);
            *m_size += 1;
        }
        return *this;
    }

    BoundedFormatIterator& operator*() { return *this; }
    BoundedFormatIterator& operator++() { return *this; }
    BoundedFormatIterator operator++(int) { return {m_buffer.Get(), *m_size}; }

private:
    Ref<InPlaceArray<char8, Logger::k_max_message_size>> m_buffer;
    size_t* m_size;
};

template <typename... Args>
void Logger::Log(LogLevel level, StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    if (level > m_log_level)
    {
        return;
    }
    if (IsCategoryRegistered(category) && level > GetCategoryLevel(category))
    {
        return;
    }
    if constexpr (sizeof...(Args) == 0)
    {
        Emit(level, category, fmt);
        if (level == LogLevel::Fatal)
        {
            HandleFatal(category, fmt);
        }
    }
    else
    {
        InPlaceArray<char8, k_max_message_size> buffer;
        size_t written = 0;
        BoundedFormatIterator out(buffer, written);
        std::vformat_to(std::move(out), std::string_view(fmt.GetData(), fmt.GetSize()), std::make_format_args(args...));
        const StringViewUtf8 message(buffer.GetData(), written);
        Emit(level, category, message);
        if (level == LogLevel::Fatal)
        {
            HandleFatal(category, message);
        }
    }
}

template <typename... Args>
void Logger::Verbose(StringViewUtf8 category, StringViewUtf8 fmt, Args&&... args)
{
    Log(LogLevel::Verbose, category, fmt, std::forward<Args>(args)...);
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

# Logging

Header: `opal/logging.h`

A lightweight logging system with pluggable sinks and per-category level filtering. One global logger per application, accessed via `Opal::GetLogger()`.

## Quick Start

```cpp
#include "opal/logging.h"

// Create and register a logger before use
Opal::Logger logger;
auto console_sink = Opal::MakeShared<Opal::LogSink, Opal::ConsoleSink>(nullptr);
logger.AddSink(console_sink);
Opal::SetLogger(&logger);

// Use the global logger
Opal::GetLogger().Info("General", "Application started");
Opal::GetLogger().Info("General", "Player {} joined at position ({}, {})", player_name, x, y);
```

## Log Levels

Levels are ordered by severity. A category's minimum level filters out anything below it.

| Level     | Purpose                                      |
|-----------|----------------------------------------------|
| `Verbose` | Detailed tracing, high-frequency events      |
| `Debug`   | Diagnostic information for development       |
| `Info`    | General operational messages                 |
| `Warning` | Unexpected situations that are recoverable   |
| `Error`   | Failures that affect functionality            |
| `Fatal`   | Unrecoverable errors, throws after logging   |
| `Off`     | Disables all logging for a category          |

## Categories

Each log message belongs to a category. Categories must be registered before use. Each category has its own minimum log level. A default `"General"` category is always registered at `LogLevel::Info`.

```cpp
auto& logger = Opal::GetLogger();

// Register categories with specific levels
logger.RegisterCategory("Rendering", Opal::LogLevel::Debug);
logger.RegisterCategory("Physics", Opal::LogLevel::Warning);
logger.RegisterCategory("Audio");  // Defaults to LogLevel::Info

// Log to a category
logger.Debug("Rendering", "Draw calls: {}", draw_call_count);    // Passes (Debug >= Debug)
logger.Debug("Physics", "Tick");                                  // Filtered (Debug < Warning)
logger.Warning("Physics", "Body {} exceeded velocity", body_id);  // Passes (Warning >= Warning)

// Change level at runtime
logger.SetCategoryLevel("Rendering", Opal::LogLevel::Warning);

// Query
bool registered = logger.IsCategoryRegistered("Rendering");       // true
Opal::LogLevel level = logger.GetCategoryLevel("Rendering");      // Warning
```

Logging to an unregistered category throws `UnregisteredCategoryException`.

```cpp
try
{
    logger.Info("NonExistent", "This will throw");
}
catch (const Opal::UnregisteredCategoryException& e)
{
    // e.What() contains "Logging with unregistered category: NonExistent"
}
```

## Message Formatting

Uses `std::format` syntax (C++20). Arguments are only formatted if the message passes the level check.

```cpp
logger.Info("General", "Simple message with no arguments");
logger.Info("General", "Player {} has {} health", player_name, health);
logger.Warning("General", "Frame time: {:.2f}ms", frame_time_ms);
logger.Error("General", "Error code: {:#x}", error_code);
```

Messages are formatted into a 2048-byte stack buffer. No heap allocations occur in the log path.

## Output Format

The logger formats each log line according to a configurable pattern before passing it to sinks. The pattern uses `<specifier>` tags that are replaced with actual values at log time.

### Available Specifiers

| Specifier    | Description                              | Example value          |
|--------------|------------------------------------------|------------------------|
| `<date>`     | Date in `YYYY-MM-DD` format              | `2026-02-17`           |
| `<time>`     | Time in `HH:MM:SS.mmm` format           | `14:30:05.123`         |
| `<thread>`   | Thread ID                                | `1234`                 |
| `<level>`    | Log level string                         | `Info`                 |
| `<category>` | Category name                            | `Rendering`            |
| `<message>`  | The formatted log message                | `Draw calls: 42`       |

### Default Pattern

```
[<date> <time>][<thread>][<level>][<category>] <message>\n
```

### Customizing the Pattern

```cpp
auto& logger = Opal::GetLogger();

// Compact format with only time and message
logger.SetPattern("[<time>][<level>] <message>\n");

// Custom separators
logger.SetPattern("<time> | <level> | <category> | <message>\n");

// Query the current pattern
Opal::StringViewUtf8 pattern = logger.GetPattern();
```

### Example Output (default pattern)

```
[2026-02-17 14:30:05.123][1234][Info][Rendering] Draw calls: 42
[2026-02-17 14:30:05.456][1234][Error][Physics] Collision solver failed for body 7
[2026-02-17 14:30:05.789][5678][Fatal][General] Out of memory!
```

## Sinks

Sinks are where log messages go. The logger can have multiple sinks. Each sink receives the fully formatted message string and is responsible for its own thread safety.

### Built-in: ConsoleSink

Writes all output to `stdout`. Thread-safe via an internal `Mutex<FILE*>`.

```cpp
auto console_sink = Opal::MakeShared<Opal::LogSink, Opal::ConsoleSink>(nullptr);
logger.AddSink(console_sink);
```

### Custom Sinks

Implement the `LogSink` interface:

```cpp
struct FileSink : public Opal::LogSink
{
    Opal::Mutex<FILE*> m_file;

    FileSink(const char* path) : m_file(fopen(path, "w")) {}

    ~FileSink() override
    {
        auto guard = m_file.Lock();
        if (*guard.Deref())
        {
            fclose(*guard.Deref());
        }
    }

    void Write(Opal::LogLevel level, Opal::StringViewUtf8 category,
               Opal::StringViewUtf8 formatted_message) override
    {
        auto guard = m_file.Lock();
        fwrite(formatted_message.GetData(), 1, formatted_message.GetSize(), *guard.Deref());
    }

    void Flush() override
    {
        auto guard = m_file.Lock();
        fflush(*guard.Deref());
    }
};

// Usage
auto file_sink = Opal::MakeShared<Opal::LogSink, FileSink>(nullptr, "game.log");
Opal::GetLogger().AddSink(file_sink);
```

### Managing Sinks

```cpp
auto sink = Opal::MakeShared<Opal::LogSink, Opal::ConsoleSink>(nullptr);

logger.AddSink(sink);       // Clones the SharedPtr internally
logger.RemoveSink(sink);    // Removes by pointer equality
logger.ClearSinks();        // Removes all sinks
logger.Flush();             // Flushes all sinks
```

`AddSink` clones the `SharedPtr`, so the caller retains ownership of the original.

## Fatal Behavior

When `Fatal` is called:

1. The message is formatted and written to all sinks.
2. All sinks are flushed.
3. A `FatalLogException` is thrown.

```cpp
try
{
    logger.Fatal("General", "Unrecoverable error: {}", details);
}
catch (const Opal::FatalLogException& e)
{
    // e.What() contains the exception message
}
```

## Global Logger

A single application-wide logger is accessible via free functions. The global logger is not initialized automatically — you must call `SetLogger` before any call to `GetLogger`. The logger is not owned by the library; the caller is responsible for its lifetime.

```cpp
Opal::Logger logger;
logger.RegisterCategory("MyApp", Opal::LogLevel::Debug);
auto sink = Opal::MakeShared<Opal::LogSink, Opal::ConsoleSink>(nullptr);
logger.AddSink(sink);
Opal::SetLogger(&logger);

// Somewhere else in the application
Opal::GetLogger().Info("MyApp", "Ready");

// Unregister when the logger goes out of scope
Opal::SetLogger(nullptr);
```

Calling `GetLogger()` before `SetLogger` throws `LoggerNotInitializedException`.

## Thread Safety

The `Logger` itself is not synchronized. Thread safety is delegated to individual sink implementations. `ConsoleSink` is thread-safe via an internal mutex. If you call logger methods from multiple threads, ensure your sinks handle concurrent `Write`/`Flush` calls.

## API Reference

### LogLevel

```cpp
enum class LogLevel : u8 { Verbose, Debug, Info, Warning, Error, Fatal, Off };
```

### LogSink

```cpp
struct LogSink
{
    virtual ~LogSink() = default;
    virtual void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) = 0;
    virtual void Flush() = 0;
};
```

### Logger

| Method | Description |
|--------|-------------|
| `RegisterCategory(category, level)` | Register a category with a minimum log level (default: `Info`) |
| `IsCategoryRegistered(category)` | Check if a category is registered |
| `SetCategoryLevel(category, level)` | Change a category's minimum level at runtime |
| `GetCategoryLevel(category)` | Get a category's current minimum level |
| `SetPattern(pattern)` | Set the output format pattern (see [Output Format](#output-format)) |
| `GetPattern()` | Get the current output format pattern |
| `AddSink(sink)` | Add a sink (clones the SharedPtr) |
| `RemoveSink(sink)` | Remove a sink by pointer equality |
| `ClearSinks()` | Remove all sinks |
| `Flush()` | Flush all sinks |
| `Log(level, category, fmt, args...)` | Log with explicit level |
| `Verbose(category, fmt, args...)` | Log at Verbose level |
| `Debug(category, fmt, args...)` | Log at Debug level |
| `Info(category, fmt, args...)` | Log at Info level |
| `Warning(category, fmt, args...)` | Log at Warning level |
| `Error(category, fmt, args...)` | Log at Error level |
| `Fatal(category, fmt, args...)` | Log at Fatal level, then throw |

### Free Functions

| Function | Description |
|----------|-------------|
| `GetLogger()` | Get the global logger; throws `LoggerNotInitializedException` if not set |
| `SetLogger(logger)` | Set the global logger (non-owning); pass `nullptr` to unset |
| `LogLevelToString(level)` | Convert a LogLevel to its string representation |
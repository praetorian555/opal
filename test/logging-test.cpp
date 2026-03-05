#include "test-helpers.h"

#include "opal/logging.h"

using namespace Opal;

/*************************************************************************************************/
/** Test sink that captures log output ***********************************************************/
/*************************************************************************************************/

struct TestSink : public LogSink
{
    struct Entry
    {
        LogLevel level;
        StringUtf8 category;
        StringUtf8 message;
    };

    DynamicArray<Entry> m_entries;
    i32 m_flush_count = 0;

    void Write(LogLevel level, StringViewUtf8 category, StringViewUtf8 formatted_message) override
    {
        Entry entry;
        entry.level = level;
        entry.category = StringUtf8(category.GetData(), category.GetSize());
        entry.message = StringUtf8(formatted_message.GetData(), formatted_message.GetSize());
        m_entries.PushBack(Move(entry));
    }

    void Flush() override { ++m_flush_count; }
};

/*************************************************************************************************/
/** LogLevel tests *******************************************************************************/
/*************************************************************************************************/

TEST_CASE("LogLevelToString", "[Logging]")
{
    REQUIRE(strcmp(LogLevelToString(LogLevel::Verbose), "Verbose") == 0);
    REQUIRE(strcmp(LogLevelToString(LogLevel::Info), "Info") == 0);
    REQUIRE(strcmp(LogLevelToString(LogLevel::Warning), "Warning") == 0);
    REQUIRE(strcmp(LogLevelToString(LogLevel::Error), "Error") == 0);
    REQUIRE(strcmp(LogLevelToString(LogLevel::Fatal), "Fatal") == 0);
    REQUIRE(strcmp(LogLevelToString(LogLevel::Off), "Off") == 0);
}

/*************************************************************************************************/
/** Category tests *******************************************************************************/
/*************************************************************************************************/

TEST_CASE("Categories", "[Logging]")
{
    Logger logger;

    SECTION("Default category General is registered")
    {
        REQUIRE(logger.IsCategoryRegistered("General"));
        REQUIRE(logger.GetCategoryLevel("General") == LogLevel::Info);
    }

    SECTION("Register custom category")
    {
        logger.RegisterCategory("Rendering", LogLevel::Verbose);
        REQUIRE(logger.IsCategoryRegistered("Rendering"));
        REQUIRE(logger.GetCategoryLevel("Rendering") == LogLevel::Verbose);
    }

    SECTION("Register category with default level")
    {
        logger.RegisterCategory("Audio");
        REQUIRE(logger.IsCategoryRegistered("Audio"));
        REQUIRE(logger.GetCategoryLevel("Audio") == LogLevel::Info);
    }

    SECTION("Unregistered category")
    {
        REQUIRE_FALSE(logger.IsCategoryRegistered("NonExistent"));
    }

    SECTION("Change category level at runtime")
    {
        logger.RegisterCategory("Physics", LogLevel::Warning);
        REQUIRE(logger.GetCategoryLevel("Physics") == LogLevel::Warning);

        logger.SetCategoryLevel("Physics", LogLevel::Verbose);
        REQUIRE(logger.GetCategoryLevel("Physics") == LogLevel::Verbose);
    }
}

/*************************************************************************************************/
/** Sink management tests ************************************************************************/
/*************************************************************************************************/

TEST_CASE("Sink management", "[Logging]")
{
    Logger logger;

    SECTION("Add sink and log to it")
    {
        auto sink = MakeShared<LogSink, TestSink>(nullptr);
        auto* test_sink = static_cast<TestSink*>(sink.Get());

        logger.AddSink(sink);
        logger.Info("General", "Hello {}", 42);

        REQUIRE(test_sink->m_entries.GetSize() == 1);
        REQUIRE(test_sink->m_entries[0].level == LogLevel::Info);
    }

    SECTION("Clear sinks")
    {
        auto sink = MakeShared<LogSink, TestSink>(nullptr);
        logger.AddSink(sink);
        logger.ClearSinks();

        // Add a new sink after clearing to verify old sinks are gone
        auto sink2 = MakeShared<LogSink, TestSink>(nullptr);
        auto* test_sink2 = static_cast<TestSink*>(sink2.Get());
        logger.AddSink(sink2);

        logger.Info("General", "Should only appear in sink2");
        REQUIRE(test_sink2->m_entries.GetSize() == 1);
    }

    SECTION("Flush sinks")
    {
        auto sink = MakeShared<LogSink, TestSink>(nullptr);
        auto* test_sink = static_cast<TestSink*>(sink.Get());

        logger.AddSink(sink);
        logger.Flush();

        REQUIRE(test_sink->m_flush_count == 1);
    }
}

/*************************************************************************************************/
/** Logging level filtering tests ****************************************************************/
/*************************************************************************************************/

TEST_CASE("Level filtering", "[Logging]")
{
    Logger logger;
    logger.SetLogLevel(LogLevel::Verbose);
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Messages below category level are filtered")
    {
        logger.RegisterCategory("Rendering", LogLevel::Warning);

        logger.Verbose("Rendering", "Verbose message");
        logger.Info("Rendering", "Info message");
        logger.Warning("Rendering", "Warning message");
        logger.Error("Rendering", "Error message");

        REQUIRE(test_sink->m_entries.GetSize() == 2);
        REQUIRE(test_sink->m_entries[0].level == LogLevel::Warning);
        REQUIRE(test_sink->m_entries[1].level == LogLevel::Error);
    }

    SECTION("Category level Off disables all logging")
    {
        logger.RegisterCategory("Silent", LogLevel::Off);

        logger.Verbose("Silent", "msg");
        logger.Info("Silent", "msg");
        logger.Warning("Silent", "msg");
        logger.Error("Silent", "msg");

        REQUIRE(test_sink->m_entries.GetSize() == 0);
    }

    SECTION("Category level Verbose allows all messages")
    {
        logger.RegisterCategory("Verbose", LogLevel::Verbose);

        logger.Verbose("Verbose", "msg");
        logger.Info("Verbose", "msg");
        logger.Warning("Verbose", "msg");
        logger.Error("Verbose", "msg");

        REQUIRE(test_sink->m_entries.GetSize() == 4);
    }
}

/*************************************************************************************************/
/** Message formatting tests *********************************************************************/
/*************************************************************************************************/

TEST_CASE("Message formatting", "[Logging]")
{
    Logger logger;
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Format string with no arguments")
    {
        logger.Info("General", "Simple message");
        REQUIRE(test_sink->m_entries.GetSize() == 1);

        // Check that formatted message contains expected parts
        StringViewUtf8 msg(test_sink->m_entries[0].message);
        // Should contain [Info] [General] Simple message
        REQUIRE(msg.GetSize() > 0);
    }

    SECTION("Format string with arguments")
    {
        logger.Info("General", "Value: {}", 42);
        REQUIRE(test_sink->m_entries.GetSize() == 1);
    }

    SECTION("Output format contains timestamp, level, category, and message")
    {
        logger.Info("General", "Test message");

        StringViewUtf8 msg(test_sink->m_entries[0].message);
        std::string_view sv(msg.GetData(), msg.GetSize());

        // Should start with [
        REQUIRE(sv[0] == '[');
        // Should contain [Info]
        REQUIRE(sv.find("[Info]") != std::string_view::npos);
        // Should contain [General]
        REQUIRE(sv.find("[General]") != std::string_view::npos);
        // Should contain the message
        REQUIRE(sv.find("Test message") != std::string_view::npos);
    }
}

/*************************************************************************************************/
/** Fatal behavior tests *************************************************************************/
/*************************************************************************************************/

TEST_CASE("Fatal behavior", "[Logging]")
{
    Logger logger;
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Fatal throws exception")
    {
        bool caught = false;
        try
        {
            logger.Fatal("General", "Fatal error!");
        }
        catch (const FatalLogException&)
        {
            caught = true;
        }
        REQUIRE(caught);
    }

    SECTION("Fatal flushes sinks before throwing")
    {
        try
        {
            logger.Fatal("General", "Fatal error!");
        }
        catch (const FatalLogException&)
        {
        }
        REQUIRE(test_sink->m_flush_count >= 1);
    }

    SECTION("Fatal message is written to sinks before throwing")
    {
        try
        {
            logger.Fatal("General", "Critical failure");
        }
        catch (const FatalLogException&)
        {
        }
        REQUIRE(test_sink->m_entries.GetSize() == 1);
        REQUIRE(test_sink->m_entries[0].level == LogLevel::Fatal);
    }
}

/*************************************************************************************************/
/** Multiple sinks tests *************************************************************************/
/*************************************************************************************************/

TEST_CASE("Multiple sinks", "[Logging]")
{
    Logger logger;

    auto sink1 = MakeShared<LogSink, TestSink>(nullptr);
    auto sink2 = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink1 = static_cast<TestSink*>(sink1.Get());
    auto* test_sink2 = static_cast<TestSink*>(sink2.Get());

    logger.AddSink(sink1);
    logger.AddSink(sink2);

    logger.Info("General", "Broadcast message");

    REQUIRE(test_sink1->m_entries.GetSize() == 1);
    REQUIRE(test_sink2->m_entries.GetSize() == 1);
}

/*************************************************************************************************/
/** Sink removal tests ***************************************************************************/
/*************************************************************************************************/

TEST_CASE("Remove sink", "[Logging]")
{
    Logger logger;
    auto sink1 = MakeShared<LogSink, TestSink>(nullptr);
    auto sink2 = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink1 = static_cast<TestSink*>(sink1.Get());
    auto* test_sink2 = static_cast<TestSink*>(sink2.Get());

    logger.AddSink(sink1);
    logger.AddSink(sink2);

    logger.RemoveSink(sink1);
    logger.Info("General", "After removal");

    REQUIRE(test_sink1->m_entries.GetSize() == 0);
    REQUIRE(test_sink2->m_entries.GetSize() == 1);
}

/*************************************************************************************************/
/** Pattern tests ********************************************************************************/
/*************************************************************************************************/

TEST_CASE("Pattern", "[Logging]")
{
    Logger logger;
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Default pattern")
    {
        StringViewUtf8 pattern = logger.GetPattern();
        std::string_view sv(pattern.GetData(), pattern.GetSize());
        REQUIRE(sv.find("<date>") != std::string_view::npos);
        REQUIRE(sv.find("<time>") != std::string_view::npos);
        REQUIRE(sv.find("<level>") != std::string_view::npos);
        REQUIRE(sv.find("<category>") != std::string_view::npos);
        REQUIRE(sv.find("<message>") != std::string_view::npos);
    }

    SECTION("SetPattern changes output format")
    {
        logger.SetPattern("<level>: <message>\n");
        logger.Info("General", "Hello");

        StringViewUtf8 result(test_sink->m_entries[0].message);
        std::string_view sv(result.GetData(), result.GetSize());
        REQUIRE(sv == "Info: Hello\n");
    }

    SECTION("GetPattern returns current pattern")
    {
        logger.SetPattern("<message>\n");
        StringViewUtf8 pattern = logger.GetPattern();
        std::string_view sv(pattern.GetData(), pattern.GetSize());
        REQUIRE(sv == "<message>\n");
    }
}

/*************************************************************************************************/
/** Global log level tests ***********************************************************************/
/*************************************************************************************************/

TEST_CASE("Global log level filtering", "[Logging]")
{
    Logger logger;
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Default log level is Info")
    {
        logger.RegisterCategory("Test", LogLevel::Verbose);

        logger.Verbose("Test", "msg");
        logger.Info("Test", "msg");

        REQUIRE(test_sink->m_entries.GetSize() == 1);
        REQUIRE(test_sink->m_entries[0].level == LogLevel::Info);
    }

    SECTION("SetLogLevel filters below global level")
    {
        logger.SetLogLevel(LogLevel::Warning);
        logger.RegisterCategory("Test", LogLevel::Verbose);

        logger.Verbose("Test", "msg");
        logger.Info("Test", "msg");
        logger.Warning("Test", "msg");
        logger.Error("Test", "msg");

        REQUIRE(test_sink->m_entries.GetSize() == 2);
        REQUIRE(test_sink->m_entries[0].level == LogLevel::Warning);
        REQUIRE(test_sink->m_entries[1].level == LogLevel::Error);
    }

    SECTION("Global level Off disables all logging")
    {
        logger.SetLogLevel(LogLevel::Off);

        logger.Info("General", "msg");
        logger.Warning("General", "msg");
        logger.Error("General", "msg");

        REQUIRE(test_sink->m_entries.GetSize() == 0);
    }
}

/*************************************************************************************************/
/** Global logger tests **************************************************************************/
/*************************************************************************************************/

TEST_CASE("Global logger", "[Logging]")
{
    SECTION("GetLogger returns a valid logger without calling SetLogger")
    {
        REQUIRE(GetLogger().IsCategoryRegistered("General"));
    }

    SECTION("Default logger supports logging")
    {
        REQUIRE_NOTHROW(GetLogger().Info("General", "Default logger works"));
    }

    SECTION("SetLogger replaces the global logger")
    {
        Logger custom_logger;
        custom_logger.RegisterCategory("Custom", LogLevel::Info);

        SetLogger(&custom_logger);
        REQUIRE(GetLogger().IsCategoryRegistered("Custom"));
        SetLogger(nullptr);
    }

    SECTION("SetLogger with nullptr restores the default logger")
    {
        Logger custom_logger;
        custom_logger.RegisterCategory("Temporary", LogLevel::Info);

        SetLogger(&custom_logger);
        REQUIRE(GetLogger().IsCategoryRegistered("Temporary"));

        SetLogger(nullptr);
        REQUIRE(GetLogger().IsCategoryRegistered("General"));
        REQUIRE_FALSE(GetLogger().IsCategoryRegistered("Temporary"));
    }
}

/*************************************************************************************************/
/** Unregistered category tests ******************************************************************/
/*************************************************************************************************/

TEST_CASE("Message larger than 2048 bytes", "[Logging]")
{
    Logger logger;
    logger.SetPattern("<message>\n");
    auto sink = MakeShared<LogSink, TestSink>(nullptr);
    auto* test_sink = static_cast<TestSink*>(sink.Get());
    logger.AddSink(sink);

    SECTION("Large message without format arguments is truncated without corruption")
    {
        constexpr u64 k_large_size = Logger::k_max_message_size + 1000;
        char8 raw[k_large_size];
        memset(raw, 'A', k_large_size);
        StringViewUtf8 large_msg(raw, k_large_size);

        logger.Info("General", large_msg);

        REQUIRE(test_sink->m_entries.GetSize() == 1);
        StringViewUtf8 result(test_sink->m_entries[0].message);
        REQUIRE(result.GetSize() == Logger::k_max_message_size);
        std::string_view sv(result.GetData(), result.GetSize());
        for (u64 i = 0; i < sv.size(); ++i)
        {
            REQUIRE(sv[i] == 'A');
        }
    }

    SECTION("Message exactly at 2048 bytes")
    {
        constexpr u64 k_exact_size = Logger::k_max_message_size - 1;
        char8 raw[k_exact_size];
        memset(raw, 'B', k_exact_size);
        StringViewUtf8 exact_msg(raw, k_exact_size);

        logger.Info("General", exact_msg);

        REQUIRE(test_sink->m_entries.GetSize() == 1);
        StringViewUtf8 result(test_sink->m_entries[0].message);
        REQUIRE(result.GetSize() == Logger::k_max_message_size);
        std::string_view sv(result.GetData(), result.GetSize());
        for (u64 i = 0; i < k_exact_size; ++i)
        {
            REQUIRE(sv[i] == 'B');
        }
        REQUIRE(sv[k_exact_size] == '\n');
    }

    SECTION("Large message with format arguments is truncated without corruption")
    {
        constexpr u64 k_large_size = 3000;
        char8 raw[k_large_size];
        memset(raw, 'C', k_large_size);
        StringUtf8 large_str(raw, k_large_size);

        logger.Info("General", "msg: {}", std::string_view(large_str.GetData(), large_str.GetSize()));

        REQUIRE(test_sink->m_entries.GetSize() == 1);
        StringViewUtf8 result(test_sink->m_entries[0].message);
        REQUIRE(result.GetSize() == Logger::k_max_message_size);
        REQUIRE(result[0] == 'm');
        REQUIRE(result[1] == 's');
        REQUIRE(result[2] == 'g');
        REQUIRE(result[3] == ':');
        REQUIRE(result[4] == ' ');
        for (u64 i = 5; i < Logger::k_max_message_size; ++i)
        {
            REQUIRE(result[i] == 'C');
        }
    }
}

TEST_CASE("Unregistered category throws", "[Logging]")
{
    Logger logger;
    logger.SetLogLevel(LogLevel::Verbose);

    SECTION("Log with unregistered category throws UnregisteredCategoryException")
    {
        REQUIRE_THROWS_AS(logger.Info("NonExistent", "Should throw"), UnregisteredCategoryException);
    }

    SECTION("Each log level throws for unregistered category")
    {
        REQUIRE_THROWS_AS(logger.Verbose("NonExistent", "msg"), UnregisteredCategoryException);
        REQUIRE_THROWS_AS(logger.Info("NonExistent", "msg"), UnregisteredCategoryException);
        REQUIRE_THROWS_AS(logger.Warning("NonExistent", "msg"), UnregisteredCategoryException);
        REQUIRE_THROWS_AS(logger.Error("NonExistent", "msg"), UnregisteredCategoryException);
        REQUIRE_THROWS_AS(logger.Fatal("NonExistent", "msg"), UnregisteredCategoryException);
    }

    SECTION("Log with registered category does not throw")
    {
        auto sink = MakeShared<LogSink, TestSink>(nullptr);
        logger.AddSink(sink);
        REQUIRE_NOTHROW(logger.Info("General", "Should not throw"));
    }
}
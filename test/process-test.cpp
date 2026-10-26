#include "opal/process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(OPAL_PLATFORM_WINDOWS)
// Catch2 is included after this header, and its std::min and std::max calls break on the min and max macros.
#define NOMINMAX
#include <Windows.h>
#undef GetEnvironmentVariable
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#elif defined(OPAL_PLATFORM_MACOS)
#include <sys/sysctl.h>
#endif

#include "opal/file-system.h"
#include "opal/paths.h"
#include "opal/threading/thread.h"
#include "opal/time.h"

#include "test-helpers.h"

using namespace Opal;

namespace
{

#if defined(OPAL_TEST_PROCESS_HELPER)
const StringUtf8 g_helper(OPAL_TEST_PROCESS_HELPER);
#else
// A build that cannot name the helper's path at compile time puts it next to the test executable.
StringUtf8 FindHelperNextToTests()
{
    Expected<StringUtf8, ErrorCode> executable = Paths::GetExecutablePath();
    if (!executable.HasValue())
    {
        return {};
    }
    Expected<StringUtf8, ErrorCode> directory = Paths::GetParentPath(executable.GetValue());
    if (!directory.HasValue())
    {
        return {};
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    Expected<StringUtf8, ErrorCode> path = Paths::Combine(directory.GetValue(), "opal_test_process_helper.exe");
#else
    Expected<StringUtf8, ErrorCode> path = Paths::Combine(directory.GetValue(), "opal_test_process_helper");
#endif
    return path.HasValue() ? Move(path).GetValue() : StringUtf8();
}

const StringUtf8 g_helper = FindHelperNextToTests();
#endif

ProcessDesc HelperDesc(ArrayView<const StringViewUtf8> arguments)
{
    ProcessDesc desc;
    desc.executable = g_helper;
    desc.arguments = arguments;
    return desc;
}

ProcessResult RunOrFail(const ProcessDesc& desc)
{
    Expected<ProcessResult, ErrorCode> result = RunProcess(desc);
    REQUIRE(result.HasValue());
    return Move(result).GetValue();
}

Process SpawnOrFail(const ProcessDesc& desc)
{
    Expected<Process, ErrorCode> process = Process::Spawn(desc);
    REQUIRE(process.HasValue());
    return Move(process).GetValue();
}

std::string ToStd(const StringUtf8& text)
{
    return std::string(text.GetData(), text.GetSize());
}

StringUtf8 TempPath(const char* name)
{
    Expected<StringUtf8, ErrorCode> cwd = Paths::GetCurrentWorkingDirectory();
    REQUIRE(cwd.HasValue());
    Expected<StringUtf8, ErrorCode> path = Paths::Combine(cwd.GetValue(), name);
    REQUIRE(path.HasValue());
    return Move(path).GetValue();
}

constexpr const char* k_parent_variable = "OPAL_PROCESS_TEST_PARENT";

/** Give the test process a variable only children that inherit will see. */
void SetParentVariable()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(SetEnvironmentVariableW(L"OPAL_PROCESS_TEST_PARENT", L"parent") != 0);
#else
    REQUIRE(setenv(k_parent_variable, "parent", 1) == 0);
#endif
}

/** "PATH=" followed by the test process's PATH. */
StringUtf8 ParentPathEntry()
{
    Expected<StringUtf8, ErrorCode> path = GetEnvironmentVariable("PATH");
    REQUIRE(path.HasValue());
    StringUtf8 entry("PATH=");
    REQUIRE(entry.Append(path.GetValue()) == ErrorCode::Success);
    return entry;
}

/** Read the id a "grandchild" helper writes, waiting up to ten seconds for it to appear. */
u32 ReadGrandchildId(const StringUtf8& pid_file)
{
    for (int attempt = 0; attempt < 1000; ++attempt)
    {
        Expected<StringUtf8, ErrorCode> text = ReadFileAsString(pid_file);
        if (text.HasValue() && !text.GetValue().IsEmpty())
        {
            return static_cast<u32>(strtoul(text.GetValue().GetData(), nullptr, 10));
        }
        SleepCurrentThread(10);
    }
    FAIL("the helper never wrote its grandchild's id");
    return 0;
}

/** True once the process with this id has ended, waiting up to five seconds. */
bool ProcessEnds(u32 id)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    HANDLE handle = OpenProcess(SYNCHRONIZE, FALSE, id);
    if (handle == nullptr)
    {
        return true;
    }
    const bool ended = WaitForSingleObject(handle, 5000) == WAIT_OBJECT_0;
    CloseHandle(handle);
    return ended;
#elif defined(OPAL_PLATFORM_MACOS)
    // A killed process stays a zombie until its new parent reaps it, which counts as ended here.
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(id)};
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        kinfo_proc info = {};
        size_t size = sizeof(info);
        if (sysctl(request, 4, &info, &size, nullptr, 0) != 0 || size == 0 || info.kp_proc.p_stat == SZOMB)
        {
            return true;
        }
        SleepCurrentThread(10);
    }
    return false;
#else
    // A killed process stays a zombie until its new parent reaps it, which counts as ended here.
    char path[64] = {};
    snprintf(path, sizeof(path), "/proc/%u/stat", id);
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        Expected<StringUtf8, ErrorCode> stat = ReadFileAsString(path);
        if (!stat.HasValue())
        {
            return true;
        }
        const char* name_end = strrchr(stat.GetValue().GetData(), ')');
        if (name_end != nullptr && name_end[1] == ' ' && name_end[2] == 'Z')
        {
            return true;
        }
        SleepCurrentThread(10);
    }
    return false;
#endif
}

}  // namespace

TEST_CASE("Process exit code", "[Process]")
{
    const StringViewUtf8 arguments[] = {"exit", "7"};
    const ProcessResult result = RunOrFail(HelperDesc(arguments));
    REQUIRE(result.exit_code == 7);
    REQUIRE_FALSE(result.was_killed);
    REQUIRE(result.stdout_text.IsEmpty());
    REQUIRE(result.stderr_text.IsEmpty());
}

TEST_CASE("Process arguments round-trip", "[Process]")
{
    const StringViewUtf8 arguments[] = {
        "echo-args", "", "plain", "two words", "quote\"inside", "trailing\\", "back\\\\slash\\\"quote", "tab\there", "utf8 \xC4\x8D\xC4\x87",
    };
    const ProcessResult result = RunOrFail(HelperDesc(arguments));
    REQUIRE(result.exit_code == 0);
    std::string expected;
    for (u64 i = 1; i < sizeof(arguments) / sizeof(arguments[0]); ++i)
    {
        expected += std::to_string(arguments[i].GetSize()) + ":" + std::string(arguments[i].GetData(), arguments[i].GetSize()) + "\n";
    }
    REQUIRE(ToStd(result.stdout_text) == expected);
}

TEST_CASE("Process captures large output on both streams", "[Process]")
{
    // Far more than a pipe buffer on each stream, written alternately, so a reader that drained one stream at a time would deadlock.
    const StringViewUtf8 arguments[] = {"write", "4194304", "3145728"};
    const ProcessResult result = RunOrFail(HelperDesc(arguments));
    REQUIRE(result.exit_code == 0);
    REQUIRE(result.stdout_text.GetSize() == 4194304);
    REQUIRE(result.stderr_text.GetSize() == 3145728);
    REQUIRE(result.stdout_text[0] == 'o');
    REQUIRE(result.stderr_text[result.stderr_text.GetSize() - 1] == 'e');
}

TEST_CASE("Process output modes", "[Process]")
{
    SECTION("stderr merged into stdout keeps the order of writes")
    {
        const StringViewUtf8 arguments[] = {"interleave"};
        ProcessDesc desc = HelperDesc(arguments);
        desc.stderr_mode = ProcessOutput::MergeWithStdout;
        const ProcessResult result = RunOrFail(desc);
        REQUIRE(ToStd(result.stdout_text) == "out1\nerr1\nout2\n");
        REQUIRE(result.stderr_text.IsEmpty());
    }
    SECTION("discarded streams capture nothing")
    {
        const StringViewUtf8 arguments[] = {"interleave"};
        ProcessDesc desc = HelperDesc(arguments);
        desc.stdout_mode = ProcessOutput::Discard;
        desc.stderr_mode = ProcessOutput::Discard;
        const ProcessResult result = RunOrFail(desc);
        REQUIRE(result.exit_code == 0);
        REQUIRE(result.stdout_text.IsEmpty());
        REQUIRE(result.stderr_text.IsEmpty());
    }
    SECTION("stdout cannot merge into itself")
    {
        const StringViewUtf8 arguments[] = {"interleave"};
        ProcessDesc desc = HelperDesc(arguments);
        desc.stdout_mode = ProcessOutput::MergeWithStdout;
        Expected<Process, ErrorCode> process = Process::Spawn(desc);
        REQUIRE_FALSE(process.HasValue());
        REQUIRE(process.GetError() == ErrorCode::InvalidArgument);
    }
    SECTION("stdin is empty")
    {
        const StringViewUtf8 arguments[] = {"stdin"};
        const ProcessResult result = RunOrFail(HelperDesc(arguments));
        REQUIRE(ToStd(result.stdout_text) == "0");
    }
}

TEST_CASE("Process environment", "[Process]")
{
    SECTION("entries are added to the inherited environment")
    {
        const StringViewUtf8 arguments[] = {"env", "OPAL_PROCESS_TEST_VALUE"};
        const StringViewUtf8 environment[] = {"OPAL_PROCESS_TEST_VALUE=hello world"};
        ProcessDesc desc = HelperDesc(arguments);
        desc.environment = environment;
        REQUIRE(ToStd(RunOrFail(desc).stdout_text) == "hello world");
    }
    SECTION("the parent's entries are inherited")
    {
        SetParentVariable();
        const StringViewUtf8 arguments[] = {"env", k_parent_variable};
        REQUIRE(ToStd(RunOrFail(HelperDesc(arguments)).stdout_text) == "parent");
    }
    SECTION("entries replace inherited ones of the same name")
    {
        SetParentVariable();
        const StringViewUtf8 arguments[] = {"env", k_parent_variable};
#if defined(OPAL_PLATFORM_WINDOWS)
        // Names compare without case on Windows.
        const StringViewUtf8 environment[] = {"opal_process_test_parent=replaced"};
#else
        const StringViewUtf8 environment[] = {"OPAL_PROCESS_TEST_PARENT=replaced"};
#endif
        ProcessDesc desc = HelperDesc(arguments);
        desc.environment = environment;
        REQUIRE(ToStd(RunOrFail(desc).stdout_text) == "replaced");
    }
    SECTION("a child that does not inherit sees only the given entries")
    {
        SetParentVariable();
        // PATH is passed on so a sanitizer runtime the helper loads from it is still found.
        const StringUtf8 path_entry = ParentPathEntry();
        const StringViewUtf8 environment[] = {path_entry, "OPAL_PROCESS_TEST_VALUE=given"};
        const StringViewUtf8 arguments_parent[] = {"env", k_parent_variable};
        ProcessDesc desc = HelperDesc(arguments_parent);
        desc.inherit_environment = false;
        desc.environment = environment;
        REQUIRE(ToStd(RunOrFail(desc).stdout_text) == "<unset>");
        const StringViewUtf8 arguments_given[] = {"env", "OPAL_PROCESS_TEST_VALUE"};
        desc.arguments = arguments_given;
        REQUIRE(ToStd(RunOrFail(desc).stdout_text) == "given");
    }
    SECTION("an entry without a name is rejected")
    {
        const StringViewUtf8 arguments[] = {"exit", "0"};
        for (const StringViewUtf8 entry : {StringViewUtf8("NOVALUE"), StringViewUtf8("=value")})
        {
            const StringViewUtf8 environment[] = {entry};
            ProcessDesc desc = HelperDesc(arguments);
            desc.environment = environment;
            Expected<Process, ErrorCode> process = Process::Spawn(desc);
            REQUIRE_FALSE(process.HasValue());
            REQUIRE(process.GetError() == ErrorCode::InvalidArgument);
        }
    }
    SECTION("GetEnvironmentVariable")
    {
        REQUIRE(GetEnvironmentVariable("PATH").HasValue());
        Expected<StringUtf8, ErrorCode> missing = GetEnvironmentVariable("OPAL_PROCESS_TEST_NEVER_SET");
        REQUIRE_FALSE(missing.HasValue());
        REQUIRE(missing.GetError() == ErrorCode::KeyNotFound);
    }
}

TEST_CASE("Process working directory", "[Process]")
{
    const StringUtf8 directory = TempPath("process-test-cwd");
    REQUIRE(CreateDirectories(directory) == ErrorCode::Success);
    const StringViewUtf8 arguments[] = {"cwd"};
    ProcessDesc desc = HelperDesc(arguments);
    desc.working_directory = directory;
    const ProcessResult result = RunOrFail(desc);
    const std::string cwd = ToStd(result.stdout_text);
    REQUIRE(cwd.size() >= strlen("process-test-cwd"));
    REQUIRE(cwd.substr(cwd.size() - strlen("process-test-cwd")) == "process-test-cwd");
    REQUIRE(DeleteDirectory(directory) == ErrorCode::Success);

    desc.working_directory = "this-directory-does-not-exist";
    Expected<Process, ErrorCode> missing = Process::Spawn(desc);
    REQUIRE_FALSE(missing.HasValue());
    REQUIRE(missing.GetError() == ErrorCode::PathNotFound);
}

TEST_CASE("Process timeout and kill", "[Process]")
{
    const StringViewUtf8 arguments[] = {"sleep", "60000"};
    Process process = SpawnOrFail(HelperDesc(arguments));
    REQUIRE(process.IsValid());
    REQUIRE(process.GetId() != 0);

    Expected<ProcessResult, ErrorCode> early = process.WaitFor(50);
    REQUIRE_FALSE(early.HasValue());
    REQUIRE(early.GetError() == ErrorCode::TimedOut);
    REQUIRE(process.IsValid());

    const f64 start = GetMilliSeconds();
    REQUIRE(process.Kill() == ErrorCode::Success);
    Expected<ProcessResult, ErrorCode> result = process.Wait();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue().was_killed);
    REQUIRE(result.GetValue().exit_code != 0);
    REQUIRE(GetMilliSeconds() - start < 10000);
    REQUIRE_FALSE(process.IsValid());

    Expected<ProcessResult, ErrorCode> again = process.Wait();
    REQUIRE_FALSE(again.HasValue());
    REQUIRE(again.GetError() == ErrorCode::InvalidArgument);
}

TEST_CASE("Process kills descendants", "[Process]")
{
    SECTION("through Kill")
    {
        const StringUtf8 pid_file = TempPath("process-test-grandchild-kill.txt");
        [[maybe_unused]] const ErrorCode removed = DeleteFile(pid_file);
        const StringViewUtf8 arguments[] = {"grandchild", pid_file};
        Process process = SpawnOrFail(HelperDesc(arguments));
        const u32 grandchild = ReadGrandchildId(pid_file);
        REQUIRE(process.Kill() == ErrorCode::Success);
        REQUIRE(process.Wait().HasValue());
        REQUIRE(ProcessEnds(grandchild));
        REQUIRE(DeleteFile(pid_file) == ErrorCode::Success);
    }
    SECTION("through the destructor")
    {
        const StringUtf8 pid_file = TempPath("process-test-grandchild-drop.txt");
        [[maybe_unused]] const ErrorCode removed = DeleteFile(pid_file);
        const StringViewUtf8 arguments[] = {"grandchild", pid_file};
        u32 grandchild = 0;
        {
            Process process = SpawnOrFail(HelperDesc(arguments));
            grandchild = ReadGrandchildId(pid_file);
        }
        REQUIRE(ProcessEnds(grandchild));
        REQUIRE(DeleteFile(pid_file) == ErrorCode::Success);
    }
}

TEST_CASE("Process moves", "[Process]")
{
    const StringViewUtf8 arguments[] = {"exit", "3"};
    Process first = SpawnOrFail(HelperDesc(arguments));
    Process second = Move(first);
    REQUIRE_FALSE(first.IsValid());
    REQUIRE(second.IsValid());
    Process third;
    third = Move(second);
    REQUIRE_FALSE(second.IsValid());
    Expected<ProcessResult, ErrorCode> result = third.Wait();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue().exit_code == 3);
    REQUIRE(first.Kill() == ErrorCode::InvalidArgument);
}

TEST_CASE("Process executable lookup", "[Process]")
{
    SECTION("a missing executable")
    {
        ProcessDesc desc;
        desc.executable = "opal-no-such-program-anywhere";
        Expected<Process, ErrorCode> process = Process::Spawn(desc);
        REQUIRE_FALSE(process.HasValue());
        REQUIRE(process.GetError() == ErrorCode::PathNotFound);
    }
    SECTION("LocateExecutable with a path and with a search path")
    {
        Expected<StringUtf8, ErrorCode> direct = LocateExecutable(g_helper);
        REQUIRE(direct.HasValue());

        Expected<StringUtf8, ErrorCode> directory = Paths::GetParentPath(g_helper);
        Expected<StringUtf8, ErrorCode> file_name = Paths::GetStem(g_helper);
        REQUIRE(directory.HasValue());
        REQUIRE(file_name.HasValue());
        Expected<StringUtf8, ErrorCode> found = LocateExecutable(file_name.GetValue(), directory.GetValue());
        REQUIRE(found.HasValue());
        REQUIRE(IsFile(found.GetValue()));
    }
    SECTION("the child's PATH is searched")
    {
        Expected<StringUtf8, ErrorCode> directory = Paths::GetParentPath(g_helper);
        Expected<StringUtf8, ErrorCode> file_name = Paths::GetStem(g_helper);
        REQUIRE(directory.HasValue());
        REQUIRE(file_name.HasValue());
        StringUtf8 path_entry("PATH=");
        REQUIRE(path_entry.Append(directory.GetValue()) == ErrorCode::Success);
#if defined(OPAL_PLATFORM_WINDOWS)
        REQUIRE(path_entry.Append(';') == ErrorCode::Success);
#else
        REQUIRE(path_entry.Append(':') == ErrorCode::Success);
#endif
        const StringUtf8 parent_entry = ParentPathEntry();
        REQUIRE(path_entry.Append(StringViewUtf8(parent_entry.GetData() + 5, parent_entry.GetSize() - 5)) == ErrorCode::Success);
        const StringViewUtf8 environment[] = {path_entry};
        const StringViewUtf8 arguments[] = {"exit", "5"};
        ProcessDesc desc;
        desc.executable = file_name.GetValue();
        desc.arguments = arguments;
        desc.environment = environment;
        REQUIRE(RunOrFail(desc).exit_code == 5);
    }
    SECTION("GetExecutablePath")
    {
        Expected<StringUtf8, ErrorCode> path = Paths::GetExecutablePath();
        REQUIRE(path.HasValue());
        REQUIRE(IsFile(path.GetValue()));
        REQUIRE(Paths::IsPathAbsolute(path.GetValue()));
    }
}

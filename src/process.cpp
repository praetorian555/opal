#include "opal/process.h"

#include <stdio.h>
#include <string.h>

#include <utility>

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#undef GetEnvironmentVariable
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#elif defined(OPAL_PLATFORM_LINUX)
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "opal/allocator.h"
#include "opal/container/dynamic-array.h"
#include "opal/file-system.h"
#include "opal/paths.h"
#include "opal/threading/atomic.h"
#include "opal/time.h"

#if defined(OPAL_PLATFORM_LINUX)
extern char** environ;
#endif

namespace
{

using namespace Opal;

#if defined(OPAL_PLATFORM_WINDOWS)
constexpr char8 k_path_list_separator = ';';
#else
constexpr char8 k_path_list_separator = ':';
#endif

bool IsSeparator(char8 c)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

bool HasSeparator(StringViewUtf8 path)
{
    for (u64 i = 0; i < path.GetSize(); ++i)
    {
        if (IsSeparator(path[i]))
        {
            return true;
        }
    }
    return false;
}

#if defined(OPAL_PLATFORM_WINDOWS)
bool HasExtension(StringViewUtf8 name)
{
    for (u64 i = name.GetSize(); i > 0; --i)
    {
        const char8 c = name[i - 1];
        if (c == '.')
        {
            return true;
        }
        if (IsSeparator(c))
        {
            return false;
        }
    }
    return false;
}
#endif

bool IsExecutableFile(StringViewUtf8 path)
{
    if (!IsFile(path))
    {
        return false;
    }
#if defined(OPAL_PLATFORM_LINUX)
    Expected<StringUtf8, ErrorCode> terminated = path.TryToString();
    return terminated.HasValue() && access(terminated.GetValue().GetData(), X_OK) == 0;
#else
    return true;
#endif
}

/** Name of an environment entry: what precedes the first '=' after the first character, which Windows allows to be '='. */
StringViewUtf8 EntryName(StringViewUtf8 entry)
{
    for (u64 i = 1; i < entry.GetSize(); ++i)
    {
        if (entry[i] == '=')
        {
            return StringViewUtf8(entry.GetData(), i);
        }
    }
    return StringViewUtf8();
}

bool NamesEqual(StringViewUtf8 a, StringViewUtf8 b)
{
    if (a.GetSize() != b.GetSize())
    {
        return false;
    }
    for (u64 i = 0; i < a.GetSize(); ++i)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        // Environment names are case-insensitive on Windows.
        const auto lower = [](char8 c) { return c >= 'A' && c <= 'Z' ? static_cast<char8>(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i]))
        {
            return false;
        }
#else
        if (a[i] != b[i])
        {
            return false;
        }
#endif
    }
    return true;
}

/** The PATH the child will see when the description sets one, or an empty view to use the parent's. */
StringViewUtf8 ChildSearchPath(const ProcessDesc& desc)
{
    for (const StringViewUtf8& entry : desc.environment)
    {
        if (NamesEqual(EntryName(entry), "PATH"))
        {
            return StringViewUtf8(entry.GetData() + 5, entry.GetSize() - 5);
        }
    }
    return StringViewUtf8();
}

ErrorCode ValidateEnvironment(const ProcessDesc& desc)
{
    for (const StringViewUtf8& entry : desc.environment)
    {
        if (EntryName(entry).GetSize() == 0)
        {
            return ErrorCode::InvalidArgument;
        }
    }
    return ErrorCode::Success;
}

/**
 * The inherited environment with the description's entries applied, as UTF-8 "NAME=VALUE" strings.
 */
ErrorCode BuildEnvironment(const ProcessDesc& desc, DynamicArray<StringUtf8>& out_entries)
{
    if (desc.inherit_environment)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        wchar_t* block = GetEnvironmentStringsW();
        if (block == nullptr)
        {
            return ErrorCode::OSFailure;
        }
        ErrorCode err = ErrorCode::Success;
        for (const wchar_t* cursor = block; *cursor != L'\0' && err == ErrorCode::Success; cursor += wcslen(cursor) + 1)
        {
            StringUtf8 entry;
            err = Transcode(StringViewWide(cursor), entry);
            if (err == ErrorCode::Success)
            {
                err = out_entries.PushBack(Move(entry));
            }
        }
        FreeEnvironmentStringsW(block);
        if (err != ErrorCode::Success)
        {
            return err;
        }
#else
        for (char** cursor = environ; *cursor != nullptr; ++cursor)
        {
            Expected<StringUtf8, ErrorCode> entry = StringViewUtf8(*cursor).TryToString();
            if (!entry.HasValue())
            {
                return entry.GetError();
            }
            if (const ErrorCode err = out_entries.PushBack(Move(entry).GetValue()); err != ErrorCode::Success)
            {
                return err;
            }
        }
#endif
    }
    for (const StringViewUtf8& entry : desc.environment)
    {
        const StringViewUtf8 name = EntryName(entry);
        Expected<StringUtf8, ErrorCode> copy = entry.TryToString();
        if (!copy.HasValue())
        {
            return copy.GetError();
        }
        bool replaced = false;
        for (StringUtf8& existing : out_entries)
        {
            if (NamesEqual(EntryName(existing), name))
            {
                existing = Move(copy).GetValue();
                replaced = true;
                break;
            }
        }
        if (!replaced)
        {
            if (const ErrorCode err = out_entries.PushBack(Move(copy).GetValue()); err != ErrorCode::Success)
            {
                return err;
            }
        }
    }
    return ErrorCode::Success;
}

#if defined(OPAL_PLATFORM_WINDOWS)

HANDLE ToHandle(u64 value)
{
    return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(value));
}

u64 FromHandle(HANDLE handle)
{
    return static_cast<u64>(reinterpret_cast<uintptr_t>(handle));
}

/** Append an argument quoted so CommandLineToArgvW and the C runtime read it back unchanged. */
ErrorCode AppendQuotedArgument(StringUtf8& command_line, StringViewUtf8 argument)
{
    bool needs_quotes = argument.GetSize() == 0;
    for (u64 i = 0; i < argument.GetSize() && !needs_quotes; ++i)
    {
        const char8 c = argument[i];
        needs_quotes = c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '"';
    }
    if (!needs_quotes)
    {
        return command_line.Append(argument);
    }
    ErrorCode err = command_line.Append('"');
    u64 backslashes = 0;
    for (u64 i = 0; i < argument.GetSize() && err == ErrorCode::Success; ++i)
    {
        const char8 c = argument[i];
        if (c == '\\')
        {
            ++backslashes;
            continue;
        }
        const u64 count = c == '"' ? backslashes * 2 + 1 : backslashes;
        for (u64 b = 0; b < count && err == ErrorCode::Success; ++b)
        {
            err = command_line.Append('\\');
        }
        backslashes = 0;
        if (err == ErrorCode::Success)
        {
            err = command_line.Append(c);
        }
    }
    for (u64 b = 0; b < backslashes * 2 && err == ErrorCode::Success; ++b)
    {
        err = command_line.Append('\\');
    }
    if (err == ErrorCode::Success)
    {
        err = command_line.Append('"');
    }
    return err;
}

/** Overlapped read state of one captured stream. */
struct PipeState
{
    static constexpr DWORD k_buffer_size = 64 * 1024;

    AllocatorBase* allocator = nullptr;
    OVERLAPPED overlapped = {};
    bool read_pending = false;
    u8 buffer[k_buffer_size];
};

/** Child-side handles made for one spawn, closed in the parent once the child has its copies. */
struct ChildHandles
{
    HANDLE handles[4] = {};
    u32 count = 0;

    ~ChildHandles()
    {
        for (u32 i = 0; i < count; ++i)
        {
            CloseHandle(handles[i]);
        }
    }

    void Add(HANDLE handle) { handles[count++] = handle; }
};

Atomic<u64> g_pipe_counter{0};

/** Make a pipe whose read end the parent reads with overlapped I/O and whose write end the child inherits. */
ErrorCode CreateCapturePipe(HANDLE& out_read, HANDLE& out_write)
{
    wchar_t name[96] = {};
    swprintf_s(name, L"\\\\.\\pipe\\opal-process-%lu-%llu", GetCurrentProcessId(),
               static_cast<unsigned long long>(g_pipe_counter.FetchAdd(1)));
    out_read = CreateNamedPipeW(name, PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
                                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, PipeState::k_buffer_size,
                                PipeState::k_buffer_size, 0, nullptr);
    if (out_read == INVALID_HANDLE_VALUE)
    {
        return ErrorCode::OSFailure;
    }
    SECURITY_ATTRIBUTES inheritable = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    out_write = CreateFileW(name, GENERIC_WRITE, 0, &inheritable, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (out_write == INVALID_HANDLE_VALUE)
    {
        CloseHandle(out_read);
        out_read = INVALID_HANDLE_VALUE;
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
}

HANDLE OpenNullDevice()
{
    SECURITY_ATTRIBUTES inheritable = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    return CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &inheritable, OPEN_EXISTING, 0, nullptr);
}

/** An inheritable duplicate of one of the parent's standard handles, or the null device when the parent has none. */
HANDLE DuplicateStandardHandle(DWORD which)
{
    const HANDLE source = GetStdHandle(which);
    HANDLE duplicate = nullptr;
    if (source == nullptr || source == INVALID_HANDLE_VALUE ||
        DuplicateHandle(GetCurrentProcess(), source, GetCurrentProcess(), &duplicate, 0, TRUE, DUPLICATE_SAME_ACCESS) == 0)
    {
        return OpenNullDevice();
    }
    return duplicate;
}

void FreePipeState(PipeState* state, HANDLE handle)
{
    if (state == nullptr)
    {
        return;
    }
    if (state->read_pending)
    {
        DWORD transferred = 0;
        CancelIoEx(handle, &state->overlapped);
        GetOverlappedResult(handle, &state->overlapped, &transferred, TRUE);
    }
    if (state->overlapped.hEvent != nullptr)
    {
        CloseHandle(state->overlapped.hEvent);
    }
    Delete(state->allocator, state);
}

#endif

}  // namespace

/*************************************************************************************************/
/** Free functions *******************************************************************************/
/*************************************************************************************************/

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::GetEnvironmentVariable(StringViewUtf8 name)
{
    using Result = Expected<StringUtf8, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide name_wide;
    if (const ErrorCode err = Transcode(name, name_wide); err != ErrorCode::Success)
    {
        return Result(err == ErrorCode::OutOfMemory ? ErrorCode::OutOfMemory : ErrorCode::InvalidArgument);
    }
    SetLastError(ERROR_SUCCESS);
    const DWORD size_needed = GetEnvironmentVariableW(name_wide.GetData(), nullptr, 0);
    if (size_needed == 0)
    {
        if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
        {
            return Result(ErrorCode::KeyNotFound);
        }
        return Result(StringUtf8());
    }
    StringWide value_wide;
    if (const ErrorCode err = value_wide.Resize(size_needed, L'\0'); err != ErrorCode::Success)
    {
        return Result(err);
    }
    const DWORD written = GetEnvironmentVariableW(name_wide.GetData(), value_wide.GetData(), size_needed);
    if (written == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
    {
        return Result(ErrorCode::KeyNotFound);
    }
    if (written >= size_needed)
    {
        return Result(ErrorCode::OSFailure);
    }
    if (const ErrorCode err = value_wide.Resize(written); err != ErrorCode::Success)
    {
        return Result(err);
    }
    StringUtf8 value;
    if (const ErrorCode err = Transcode(value_wide, value); err != ErrorCode::Success)
    {
        return Result(err == ErrorCode::OutOfMemory ? ErrorCode::OutOfMemory : ErrorCode::OSFailure);
    }
    return Result(Move(value));
#elif defined(OPAL_PLATFORM_LINUX)
    Expected<StringUtf8, ErrorCode> name_terminated = name.TryToString();
    if (!name_terminated.HasValue())
    {
        return Result(name_terminated.GetError());
    }
    const char* value = getenv(name_terminated.GetValue().GetData());
    if (value == nullptr)
    {
        return Result(ErrorCode::KeyNotFound);
    }
    return StringViewUtf8(value).TryToString();
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::LocateExecutable(StringViewUtf8 name, StringViewUtf8 search_path)
{
    using Result = Expected<StringUtf8, ErrorCode>;
    StringUtf8 file_name;
    ErrorCode err = file_name.Append(name);
#if defined(OPAL_PLATFORM_WINDOWS)
    if (err == ErrorCode::Success && !HasExtension(name))
    {
        err = file_name.Append(".exe");
    }
#endif
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    if (file_name.IsEmpty())
    {
        return Result(ErrorCode::PathNotFound);
    }
    if (HasSeparator(file_name))
    {
        return IsExecutableFile(file_name) ? Result(Move(file_name)) : Result(ErrorCode::PathNotFound);
    }

    StringUtf8 parent_path;
    if (search_path.GetSize() == 0)
    {
        Expected<StringUtf8, ErrorCode> path_variable = GetEnvironmentVariable("PATH");
        if (!path_variable.HasValue())
        {
            return Result(path_variable.GetError() == ErrorCode::KeyNotFound ? ErrorCode::PathNotFound : path_variable.GetError());
        }
        parent_path = Move(path_variable).GetValue();
        search_path = parent_path;
    }

    u64 start = 0;
    while (start <= search_path.GetSize())
    {
        u64 end = start;
        while (end < search_path.GetSize() && search_path[end] != k_path_list_separator)
        {
            ++end;
        }
        const StringViewUtf8 directory(search_path.GetData() + start, end - start);
        if (directory.GetSize() > 0)
        {
            Expected<StringUtf8, ErrorCode> candidate = Paths::Combine(directory, file_name);
            if (!candidate.HasValue())
            {
                return Result(candidate.GetError());
            }
            if (IsExecutableFile(candidate.GetValue()))
            {
                return Result(Move(candidate).GetValue());
            }
        }
        start = end + 1;
    }
    return Result(ErrorCode::PathNotFound);
}

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> Opal::RunProcess(const ProcessDesc& desc)
{
    Expected<Process, ErrorCode> process = Process::Spawn(desc);
    if (!process.HasValue())
    {
        return Expected<ProcessResult, ErrorCode>(process.GetError());
    }
    return process.GetValue().Wait();
}

/*************************************************************************************************/
/** Process **************************************************************************************/
/*************************************************************************************************/

Opal::Process::~Process()
{
    Release();
}

Opal::Process::Process(Process&& other) noexcept
    : m_process(other.m_process),
      m_job(other.m_job),
      m_id(other.m_id),
      m_exited(other.m_exited),
      m_was_killed(other.m_was_killed),
      m_exit_code(other.m_exit_code),
      m_stdout(other.m_stdout),
      m_stderr(other.m_stderr),
      m_stdout_text(Move(other.m_stdout_text)),
      m_stderr_text(Move(other.m_stderr_text))
{
    other.m_process = k_no_handle;
    other.m_job = k_no_handle;
    other.m_id = 0;
    other.m_stdout = Pipe{};
    other.m_stderr = Pipe{};
}

Opal::Process& Opal::Process::operator=(Process&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_process = other.m_process;
        m_job = other.m_job;
        m_id = other.m_id;
        m_exited = other.m_exited;
        m_was_killed = other.m_was_killed;
        m_exit_code = other.m_exit_code;
        m_stdout = other.m_stdout;
        m_stderr = other.m_stderr;
        m_stdout_text = Move(other.m_stdout_text);
        m_stderr_text = Move(other.m_stderr_text);
        other.m_process = k_no_handle;
        other.m_job = k_no_handle;
        other.m_id = 0;
        other.m_stdout = Pipe{};
        other.m_stderr = Pipe{};
    }
    return *this;
}

bool Opal::Process::IsValid() const
{
    return m_process != k_no_handle;
}

Opal::u32 Opal::Process::GetId() const
{
    return IsValid() ? m_id : 0;
}

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> Opal::Process::Wait()
{
    return WaitImpl(false, 0);
}

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> Opal::Process::WaitFor(u64 timeout_ms)
{
    return WaitImpl(true, timeout_ms);
}

#if defined(OPAL_PLATFORM_WINDOWS)

/*************************************************************************************************/
/** Windows **************************************************************************************/
/*************************************************************************************************/

namespace
{

/** Issue reads on a captured stream until one is left pending or the stream closes, appending what arrives. */
ErrorCode PumpPipe(u64& handle_value, void*& state_value, StringUtf8& sink, u64 no_handle)
{
    PipeState* state = static_cast<PipeState*>(state_value);
    const HANDLE handle = ToHandle(handle_value);
    const auto close_pipe = [&]()
    {
        FreePipeState(state, handle);
        CloseHandle(handle);
        handle_value = no_handle;
        state_value = nullptr;
    };

    if (state->read_pending)
    {
        DWORD transferred = 0;
        if (GetOverlappedResult(handle, &state->overlapped, &transferred, FALSE) == 0)
        {
            const DWORD error = GetLastError();
            if (error == ERROR_IO_INCOMPLETE)
            {
                return ErrorCode::Success;
            }
            state->read_pending = false;
            close_pipe();
            return error == ERROR_BROKEN_PIPE ? ErrorCode::Success : ErrorCode::OSFailure;
        }
        state->read_pending = false;
        if (const ErrorCode err = sink.Append(reinterpret_cast<const char8*>(state->buffer), transferred); err != ErrorCode::Success)
        {
            close_pipe();
            return err;
        }
    }

    while (true)
    {
        ResetEvent(state->overlapped.hEvent);
        if (ReadFile(handle, state->buffer, PipeState::k_buffer_size, nullptr, &state->overlapped) == 0)
        {
            const DWORD error = GetLastError();
            if (error == ERROR_IO_PENDING)
            {
                state->read_pending = true;
                return ErrorCode::Success;
            }
            close_pipe();
            return error == ERROR_BROKEN_PIPE ? ErrorCode::Success : ErrorCode::OSFailure;
        }
        DWORD transferred = 0;
        if (GetOverlappedResult(handle, &state->overlapped, &transferred, FALSE) == 0)
        {
            close_pipe();
            return ErrorCode::OSFailure;
        }
        if (const ErrorCode err = sink.Append(reinterpret_cast<const char8*>(state->buffer), transferred); err != ErrorCode::Success)
        {
            close_pipe();
            return err;
        }
    }
}

}  // namespace

Opal::Expected<Opal::Process, Opal::ErrorCode> Opal::Process::Spawn(const ProcessDesc& desc)
{
    using Result = Expected<Process, ErrorCode>;
    if (desc.stdout_mode == ProcessOutput::MergeWithStdout)
    {
        return Result(ErrorCode::InvalidArgument);
    }
    if (const ErrorCode err = ValidateEnvironment(desc); err != ErrorCode::Success)
    {
        return Result(err);
    }

    Expected<StringUtf8, ErrorCode> executable = LocateExecutable(desc.executable, ChildSearchPath(desc));
    if (!executable.HasValue())
    {
        return Result(executable.GetError());
    }

    StringUtf8 command_line;
    ErrorCode err = AppendQuotedArgument(command_line, executable.GetValue());
    for (const StringViewUtf8& argument : desc.arguments)
    {
        if (err == ErrorCode::Success)
        {
            err = command_line.Append(' ');
        }
        if (err == ErrorCode::Success)
        {
            err = AppendQuotedArgument(command_line, argument);
        }
    }
    StringWide command_line_wide;
    StringWide executable_wide;
    StringWide working_directory_wide;
    if (err == ErrorCode::Success)
    {
        err = Transcode(command_line, command_line_wide);
    }
    if (err == ErrorCode::Success)
    {
        err = Transcode(executable.GetValue(), executable_wide);
    }
    if (err == ErrorCode::Success && desc.working_directory.GetSize() > 0)
    {
        err = Transcode(desc.working_directory, working_directory_wide);
    }
    if (err != ErrorCode::Success)
    {
        return Result(err == ErrorCode::OutOfMemory ? ErrorCode::OutOfMemory : ErrorCode::InvalidArgument);
    }
    constexpr u64 k_max_command_line = 32767;
    if (command_line_wide.GetSize() >= k_max_command_line)
    {
        return Result(ErrorCode::InvalidArgument);
    }

    StringWide environment_block;
    const bool custom_environment = !desc.inherit_environment || desc.environment.GetSize() > 0;
    if (custom_environment)
    {
        DynamicArray<StringUtf8> entries;
        err = BuildEnvironment(desc, entries);
        for (const StringUtf8& entry : entries)
        {
            StringWide entry_wide;
            if (err == ErrorCode::Success)
            {
                err = Transcode(entry, entry_wide);
            }
            if (err == ErrorCode::Success)
            {
                err = environment_block.Append(entry_wide);
            }
            if (err == ErrorCode::Success)
            {
                err = environment_block.Append(L'\0');
            }
        }
        if (err == ErrorCode::Success && entries.IsEmpty())
        {
            err = environment_block.Append(L'\0');
        }
        if (err == ErrorCode::Success)
        {
            err = environment_block.Append(L'\0');
        }
        if (err != ErrorCode::Success)
        {
            return Result(err == ErrorCode::OutOfMemory ? ErrorCode::OutOfMemory : ErrorCode::InvalidArgument);
        }
    }

    ChildHandles child_handles;
    HANDLE stdout_read = INVALID_HANDLE_VALUE;
    HANDLE stderr_read = INVALID_HANDLE_VALUE;
    const auto close_reads = [&]()
    {
        if (stdout_read != INVALID_HANDLE_VALUE)
        {
            CloseHandle(stdout_read);
        }
        if (stderr_read != INVALID_HANDLE_VALUE)
        {
            CloseHandle(stderr_read);
        }
    };

    const HANDLE stdin_child = OpenNullDevice();
    if (stdin_child == INVALID_HANDLE_VALUE)
    {
        return Result(ErrorCode::OSFailure);
    }
    child_handles.Add(stdin_child);

    const auto make_output = [&](ProcessOutput mode, DWORD standard, HANDLE& out_read, HANDLE& out_child) -> ErrorCode
    {
        switch (mode)
        {
            case ProcessOutput::Capture:
            {
                if (const ErrorCode pipe_err = CreateCapturePipe(out_read, out_child); pipe_err != ErrorCode::Success)
                {
                    return pipe_err;
                }
                break;
            }
            case ProcessOutput::Inherit:
                out_child = DuplicateStandardHandle(standard);
                break;
            case ProcessOutput::Discard:
                out_child = OpenNullDevice();
                break;
            case ProcessOutput::MergeWithStdout:
                return ErrorCode::InvalidArgument;
        }
        if (out_child == INVALID_HANDLE_VALUE)
        {
            return ErrorCode::OSFailure;
        }
        child_handles.Add(out_child);
        return ErrorCode::Success;
    };

    HANDLE stdout_child = INVALID_HANDLE_VALUE;
    HANDLE stderr_child = INVALID_HANDLE_VALUE;
    err = make_output(desc.stdout_mode, STD_OUTPUT_HANDLE, stdout_read, stdout_child);
    if (err == ErrorCode::Success)
    {
        if (desc.stderr_mode == ProcessOutput::MergeWithStdout)
        {
            stderr_child = stdout_child;
        }
        else
        {
            err = make_output(desc.stderr_mode, STD_ERROR_HANDLE, stderr_read, stderr_child);
        }
    }
    if (err != ErrorCode::Success)
    {
        close_reads();
        return Result(err);
    }

    // Only the child's own standard handles are inherited, so spawns running in parallel do not leak pipe ends into each other.
    HANDLE inherited[3] = {stdin_child, stdout_child, stderr_child};
    const DWORD inherited_count = stderr_child == stdout_child ? 2 : 3;
    SIZE_T attribute_size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attribute_size);
    DynamicArray<u8> attribute_storage;
    if (attribute_storage.Resize(attribute_size) != ErrorCode::Success)
    {
        close_reads();
        return Result(ErrorCode::OutOfMemory);
    }
    auto* attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attribute_storage.GetData());
    if (InitializeProcThreadAttributeList(attributes, 1, 0, &attribute_size) == 0)
    {
        close_reads();
        return Result(ErrorCode::OSFailure);
    }
    if (UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited, inherited_count * sizeof(HANDLE), nullptr,
                                  nullptr) == 0)
    {
        DeleteProcThreadAttributeList(attributes);
        close_reads();
        return Result(ErrorCode::OSFailure);
    }

    STARTUPINFOEXW startup_info = {};
    startup_info.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    startup_info.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    startup_info.StartupInfo.hStdInput = stdin_child;
    startup_info.StartupInfo.hStdOutput = stdout_child;
    startup_info.StartupInfo.hStdError = stderr_child;
    startup_info.lpAttributeList = attributes;

    PROCESS_INFORMATION process_info = {};
    const BOOL created = CreateProcessW(executable_wide.GetData(), command_line_wide.GetData(), nullptr, nullptr, TRUE,
                                        CREATE_UNICODE_ENVIRONMENT | EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                                        custom_environment ? environment_block.GetData() : nullptr,
                                        working_directory_wide.IsEmpty() ? nullptr : working_directory_wide.GetData(),
                                        &startup_info.StartupInfo, &process_info);
    const DWORD create_error = GetLastError();
    DeleteProcThreadAttributeList(attributes);
    if (created == 0)
    {
        close_reads();
        const bool missing = create_error == ERROR_FILE_NOT_FOUND || create_error == ERROR_PATH_NOT_FOUND || create_error == ERROR_DIRECTORY;
        return Result(missing ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }

    // The job holds every process the child starts, so Kill reaches descendants. A parent that cannot nest jobs gets a process
    // without one, and Kill then ends only the child.
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (job != nullptr && AssignProcessToJobObject(job, process_info.hProcess) == 0)
    {
        CloseHandle(job);
        job = nullptr;
    }
    ResumeThread(process_info.hThread);
    CloseHandle(process_info.hThread);

    Process process;
    process.m_process = FromHandle(process_info.hProcess);
    process.m_job = job != nullptr ? FromHandle(job) : k_no_handle;
    process.m_id = process_info.dwProcessId;

    const auto attach = [&](HANDLE read, Pipe& pipe, StringUtf8& sink) -> ErrorCode
    {
        if (read == INVALID_HANDLE_VALUE)
        {
            return ErrorCode::Success;
        }
        AllocatorBase* allocator = GetDefaultAllocator();
        PipeState* state = New<PipeState>(allocator);
        if (state == nullptr)
        {
            CloseHandle(read);
            return ErrorCode::OutOfMemory;
        }
        state->allocator = allocator;
        state->overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        pipe.handle = FromHandle(read);
        pipe.state = state;
        if (state->overlapped.hEvent == nullptr)
        {
            return ErrorCode::OSFailure;
        }
        return PumpPipe(pipe.handle, pipe.state, sink, k_no_handle);
    };
    err = attach(stdout_read, process.m_stdout, process.m_stdout_text);
    stdout_read = INVALID_HANDLE_VALUE;
    const ErrorCode stderr_err = attach(stderr_read, process.m_stderr, process.m_stderr_text);
    stderr_read = INVALID_HANDLE_VALUE;
    if (err == ErrorCode::Success)
    {
        err = stderr_err;
    }
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    return Result(Move(process));
}

Opal::ErrorCode Opal::Process::Kill()
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    if (m_exited || WaitForSingleObject(ToHandle(m_process), 0) == WAIT_OBJECT_0)
    {
        return ErrorCode::Success;
    }
    const BOOL killed = m_job != k_no_handle ? TerminateJobObject(ToHandle(m_job), 1) : TerminateProcess(ToHandle(m_process), 1);
    if (killed == 0)
    {
        return ErrorCode::OSFailure;
    }
    m_was_killed = true;
    return ErrorCode::Success;
}

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> Opal::Process::WaitImpl(bool has_timeout, u64 timeout_ms)
{
    using Result = Expected<ProcessResult, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    const ULONGLONG deadline = GetTickCount64() + timeout_ms;
    bool deadline_passed = false;
    while (!m_exited || m_stdout.handle != k_no_handle || m_stderr.handle != k_no_handle)
    {
        if (deadline_passed)
        {
            return Result(ErrorCode::TimedOut);
        }
        HANDLE handles[3] = {};
        u32 kinds[3] = {};
        DWORD count = 0;
        if (m_stdout.handle != k_no_handle)
        {
            handles[count] = static_cast<PipeState*>(m_stdout.state)->overlapped.hEvent;
            kinds[count++] = 0;
        }
        if (m_stderr.handle != k_no_handle)
        {
            handles[count] = static_cast<PipeState*>(m_stderr.state)->overlapped.hEvent;
            kinds[count++] = 1;
        }
        if (!m_exited)
        {
            handles[count] = ToHandle(m_process);
            kinds[count++] = 2;
        }
        DWORD wait_ms = INFINITE;
        if (has_timeout)
        {
            const ULONGLONG now = GetTickCount64();
            wait_ms = now >= deadline ? 0 : static_cast<DWORD>(deadline - now);
        }
        const DWORD signaled = WaitForMultipleObjects(count, handles, FALSE, wait_ms);
        if (signaled == WAIT_TIMEOUT)
        {
            return Result(ErrorCode::TimedOut);
        }
        if (signaled >= WAIT_OBJECT_0 + count)
        {
            return Result(ErrorCode::OSFailure);
        }
        const u32 kind = kinds[signaled - WAIT_OBJECT_0];
        deadline_passed = has_timeout && GetTickCount64() >= deadline;
        if (kind == 2)
        {
            DWORD exit_code = 0;
            if (GetExitCodeProcess(ToHandle(m_process), &exit_code) == 0)
            {
                return Result(ErrorCode::OSFailure);
            }
            m_exit_code = static_cast<i32>(exit_code);
            m_exited = true;
            continue;
        }
        Pipe& stream = kind == 0 ? m_stdout : m_stderr;
        StringUtf8& sink = kind == 0 ? m_stdout_text : m_stderr_text;
        if (const ErrorCode err = PumpPipe(stream.handle, stream.state, sink, k_no_handle); err != ErrorCode::Success)
        {
            return Result(err);
        }
    }

    ProcessResult result;
    result.exit_code = m_exit_code;
    result.was_killed = m_was_killed;
    result.stdout_text = Move(m_stdout_text);
    result.stderr_text = Move(m_stderr_text);
    Release();
    return Result(Move(result));
}

void Opal::Process::Release()
{
    if (m_process != k_no_handle && !m_exited && WaitForSingleObject(ToHandle(m_process), 0) == WAIT_TIMEOUT)
    {
        if (m_job != k_no_handle)
        {
            TerminateJobObject(ToHandle(m_job), 1);
        }
        else
        {
            TerminateProcess(ToHandle(m_process), 1);
        }
    }
    for (Pipe* stream : {&m_stdout, &m_stderr})
    {
        if (stream->handle != k_no_handle)
        {
            FreePipeState(static_cast<PipeState*>(stream->state), ToHandle(stream->handle));
            CloseHandle(ToHandle(stream->handle));
        }
        *stream = Pipe{};
    }
    if (m_job != k_no_handle)
    {
        CloseHandle(ToHandle(m_job));
    }
    if (m_process != k_no_handle)
    {
        CloseHandle(ToHandle(m_process));
    }
    m_process = k_no_handle;
    m_job = k_no_handle;
    m_id = 0;
    m_exited = false;
    m_was_killed = false;
    m_exit_code = 0;
    m_stdout_text = StringUtf8();
    m_stderr_text = StringUtf8();
}

#elif defined(OPAL_PLATFORM_LINUX)

/*************************************************************************************************/
/** Linux ****************************************************************************************/
/*************************************************************************************************/

namespace
{

i32 ToFd(u64 value)
{
    return static_cast<i32>(value);
}

bool Contains(const DynamicArray<pid_t>& pids, pid_t pid)
{
    for (const pid_t candidate : pids)
    {
        if (candidate == pid)
        {
            return true;
        }
    }
    return false;
}

/** Add to tree every process whose parent is already in it, following the parent links in /proc. */
void CollectDescendants(DynamicArray<pid_t>& tree)
{
    DynamicArray<pid_t> pids;
    DynamicArray<pid_t> parents;
    DIR* proc = opendir("/proc");
    if (proc == nullptr)
    {
        return;
    }
    while (const dirent* entry = readdir(proc))
    {
        char* end = nullptr;
        const long pid = strtol(entry->d_name, &end, 10);
        if (end == entry->d_name || *end != '\0')
        {
            continue;
        }
        char path[64] = {};
        snprintf(path, sizeof(path), "/proc/%ld/stat", pid);
        const i32 fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0)
        {
            continue;
        }
        char stat[512] = {};
        const ssize_t size = read(fd, stat, sizeof(stat) - 1);
        close(fd);
        // The command name is in parentheses and may hold spaces or parentheses itself, so the fields after it start at the last ')'.
        const char* name_end = size > 0 ? strrchr(stat, ')') : nullptr;
        char state = 0;
        int parent = 0;
        if (name_end == nullptr || sscanf(name_end + 1, " %c %d", &state, &parent) != 2)
        {
            continue;
        }
        if (pids.PushBack(static_cast<pid_t>(pid)) != ErrorCode::Success || parents.PushBack(static_cast<pid_t>(parent)) != ErrorCode::Success)
        {
            break;
        }
    }
    closedir(proc);

    for (bool grew = true; grew;)
    {
        grew = false;
        for (u64 i = 0; i < pids.GetSize(); ++i)
        {
            if (Contains(tree, parents[i]) && !Contains(tree, pids[i]) && tree.PushBack(pids[i]) == ErrorCode::Success)
            {
                grew = true;
            }
        }
    }
}

/**
 * Kill a child's process group and every process descended from it. A descendant started through Process leads a group of its
 * own, so the group alone does not reach it. Processes are stopped as they are found, so none can fork between being found and
 * being killed.
 */
void KillTree(pid_t root)
{
    DynamicArray<pid_t> tree;
    if (tree.PushBack(root) == ErrorCode::Success)
    {
        for (u64 known = 0; known != tree.GetSize();)
        {
            known = tree.GetSize();
            for (const pid_t pid : tree)
            {
                kill(pid, SIGSTOP);
            }
            CollectDescendants(tree);
        }
    }
    kill(-root, SIGKILL);
    for (const pid_t pid : tree)
    {
        kill(pid, SIGKILL);
    }
}

/** Read everything a non-blocking stream has, closing it on end of file. */
ErrorCode DrainPipe(u64& handle, StringUtf8& sink, u64 no_handle)
{
    char8 buffer[64 * 1024];
    while (true)
    {
        const ssize_t count = read(ToFd(handle), buffer, sizeof(buffer));
        if (count > 0)
        {
            if (const ErrorCode err = sink.Append(buffer, static_cast<u64>(count)); err != ErrorCode::Success)
            {
                return err;
            }
            continue;
        }
        if (count < 0 && errno == EINTR)
        {
            continue;
        }
        if (count < 0 && errno == EAGAIN)
        {
            return ErrorCode::Success;
        }
        close(ToFd(handle));
        handle = no_handle;
        return count == 0 ? ErrorCode::Success : ErrorCode::OSFailure;
    }
}

i32 DecodeStatus(int status)
{
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }
    return -1;
}

/** Null-terminated copies of strings, and the pointer array execve-style calls take. */
struct CStringArray
{
    DynamicArray<StringUtf8> storage;
    DynamicArray<char*> pointers;

    ErrorCode Add(StringViewUtf8 value)
    {
        Expected<StringUtf8, ErrorCode> copy = value.TryToString();
        if (!copy.HasValue())
        {
            return copy.GetError();
        }
        return storage.PushBack(Move(copy).GetValue());
    }

    ErrorCode Finish()
    {
        for (StringUtf8& value : storage)
        {
            if (const ErrorCode err = pointers.PushBack(value.GetData()); err != ErrorCode::Success)
            {
                return err;
            }
        }
        return pointers.PushBack(nullptr);
    }
};

}  // namespace

Opal::Expected<Opal::Process, Opal::ErrorCode> Opal::Process::Spawn(const ProcessDesc& desc)
{
    using Result = Expected<Process, ErrorCode>;
    if (desc.stdout_mode == ProcessOutput::MergeWithStdout)
    {
        return Result(ErrorCode::InvalidArgument);
    }
    if (const ErrorCode err = ValidateEnvironment(desc); err != ErrorCode::Success)
    {
        return Result(err);
    }
    Expected<StringUtf8, ErrorCode> executable = LocateExecutable(desc.executable, ChildSearchPath(desc));
    if (!executable.HasValue())
    {
        return Result(executable.GetError());
    }

    CStringArray argv;
    ErrorCode err = argv.Add(desc.executable);
    for (const StringViewUtf8& argument : desc.arguments)
    {
        if (err == ErrorCode::Success)
        {
            err = argv.Add(argument);
        }
    }
    if (err == ErrorCode::Success)
    {
        err = argv.Finish();
    }
    CStringArray envp;
    if (err == ErrorCode::Success)
    {
        err = BuildEnvironment(desc, envp.storage);
    }
    if (err == ErrorCode::Success)
    {
        err = envp.Finish();
    }
    StringUtf8 working_directory;
    if (err == ErrorCode::Success && desc.working_directory.GetSize() > 0)
    {
        err = working_directory.Append(desc.working_directory);
    }
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }

    i32 null_fd = open("/dev/null", O_RDWR | O_CLOEXEC);
    if (null_fd < 0)
    {
        return Result(ErrorCode::OSFailure);
    }
    i32 stdout_pipe[2] = {-1, -1};
    i32 stderr_pipe[2] = {-1, -1};
    const auto close_all = [&]()
    {
        for (i32 fd : {null_fd, stdout_pipe[0], stdout_pipe[1], stderr_pipe[0], stderr_pipe[1]})
        {
            if (fd >= 0)
            {
                close(fd);
            }
        }
    };
    if ((desc.stdout_mode == ProcessOutput::Capture && pipe2(stdout_pipe, O_CLOEXEC) != 0) ||
        (desc.stderr_mode == ProcessOutput::Capture && pipe2(stderr_pipe, O_CLOEXEC) != 0))
    {
        close_all();
        return Result(ErrorCode::OSFailure);
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, null_fd, 0);
    switch (desc.stdout_mode)
    {
        case ProcessOutput::Capture:
            posix_spawn_file_actions_adddup2(&actions, stdout_pipe[1], 1);
            break;
        case ProcessOutput::Discard:
            posix_spawn_file_actions_adddup2(&actions, null_fd, 1);
            break;
        default:
            break;
    }
    switch (desc.stderr_mode)
    {
        case ProcessOutput::Capture:
            posix_spawn_file_actions_adddup2(&actions, stderr_pipe[1], 2);
            break;
        case ProcessOutput::Discard:
            posix_spawn_file_actions_adddup2(&actions, null_fd, 2);
            break;
        case ProcessOutput::MergeWithStdout:
            posix_spawn_file_actions_adddup2(&actions, 1, 2);
            break;
        default:
            break;
    }
    if (!working_directory.IsEmpty())
    {
        posix_spawn_file_actions_addchdir_np(&actions, working_directory.GetData());
    }

    // The child leads its own process group, so Kill reaches everything it starts, and starts with default signal handling.
    posix_spawnattr_t attributes;
    posix_spawnattr_init(&attributes);
    posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF);
    posix_spawnattr_setpgroup(&attributes, 0);
    sigset_t signals;
    sigemptyset(&signals);
    posix_spawnattr_setsigmask(&attributes, &signals);
    sigfillset(&signals);
    sigdelset(&signals, SIGKILL);
    sigdelset(&signals, SIGSTOP);
    posix_spawnattr_setsigdefault(&attributes, &signals);

    pid_t pid = 0;
    const int spawn_error =
        posix_spawn(&pid, executable.GetValue().GetData(), &actions, &attributes, argv.pointers.GetData(), envp.pointers.GetData());
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attributes);
    if (spawn_error != 0)
    {
        close_all();
        return Result(spawn_error == ENOENT || spawn_error == ENOTDIR ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }

    close(null_fd);
    null_fd = -1;
    if (stdout_pipe[1] >= 0)
    {
        close(stdout_pipe[1]);
        stdout_pipe[1] = -1;
    }
    if (stderr_pipe[1] >= 0)
    {
        close(stderr_pipe[1]);
        stderr_pipe[1] = -1;
    }

    Process process;
    process.m_process = static_cast<u64>(pid);
    process.m_id = static_cast<u32>(pid);
    // A pidfd lets one poll wait for the exit and the streams together; kernels before 5.3 fall back to polling the exit.
#if defined(SYS_pidfd_open)
    const long pidfd = syscall(SYS_pidfd_open, pid, 0);
#else
    const long pidfd = -1;
#endif
    process.m_job = pidfd >= 0 ? static_cast<u64>(pidfd) : k_no_handle;
    for (auto [read_fd, stream] : {std::pair{stdout_pipe[0], &process.m_stdout}, std::pair{stderr_pipe[0], &process.m_stderr}})
    {
        if (read_fd >= 0)
        {
            fcntl(read_fd, F_SETFL, fcntl(read_fd, F_GETFL) | O_NONBLOCK);
            stream->handle = static_cast<u64>(read_fd);
        }
    }
    return Result(Move(process));
}

Opal::ErrorCode Opal::Process::Kill()
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    if (!m_exited)
    {
        int status = 0;
        if (waitpid(static_cast<pid_t>(m_process), &status, WNOHANG) == static_cast<pid_t>(m_process))
        {
            m_exit_code = DecodeStatus(status);
            m_exited = true;
        }
    }
    if (m_exited)
    {
        return ErrorCode::Success;
    }
    KillTree(static_cast<pid_t>(m_process));
    m_was_killed = true;
    return ErrorCode::Success;
}

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> Opal::Process::WaitImpl(bool has_timeout, u64 timeout_ms)
{
    using Result = Expected<ProcessResult, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    const f64 deadline = GetMilliSeconds() + static_cast<f64>(timeout_ms);
    const pid_t pid = static_cast<pid_t>(m_process);
    bool deadline_passed = false;
    while (true)
    {
        if (!m_exited)
        {
            int status = 0;
            const pid_t reaped = waitpid(pid, &status, WNOHANG);
            if (reaped == pid)
            {
                m_exit_code = DecodeStatus(status);
                m_exited = true;
            }
            else if (reaped < 0 && errno != EINTR)
            {
                return Result(ErrorCode::OSFailure);
            }
        }
        if (m_exited && m_stdout.handle == k_no_handle && m_stderr.handle == k_no_handle)
        {
            break;
        }
        if (deadline_passed)
        {
            return Result(ErrorCode::TimedOut);
        }

        pollfd fds[3] = {};
        Pipe* streams[3] = {};
        nfds_t count = 0;
        for (Pipe* stream : {&m_stdout, &m_stderr})
        {
            if (stream->handle != k_no_handle)
            {
                fds[count] = pollfd{ToFd(stream->handle), POLLIN, 0};
                streams[count++] = stream;
            }
        }
        const bool poll_exit = !m_exited && m_job != k_no_handle;
        if (poll_exit)
        {
            fds[count++] = pollfd{ToFd(m_job), POLLIN, 0};
        }

        i32 wait_ms = -1;
        if (has_timeout)
        {
            const f64 remaining = deadline - GetMilliSeconds();
            wait_ms = remaining <= 0 ? 0 : static_cast<i32>(remaining) + 1;
        }
        if (!m_exited && !poll_exit)
        {
            wait_ms = wait_ms < 0 || wait_ms > 10 ? 10 : wait_ms;
        }
        const int ready = poll(fds, count, wait_ms);
        if (ready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return Result(ErrorCode::OSFailure);
        }
        for (nfds_t i = 0; i < count; ++i)
        {
            if (streams[i] != nullptr && fds[i].revents != 0)
            {
                StringUtf8& sink = streams[i] == &m_stdout ? m_stdout_text : m_stderr_text;
                if (const ErrorCode err = DrainPipe(streams[i]->handle, sink, k_no_handle); err != ErrorCode::Success)
                {
                    return Result(err);
                }
            }
        }
        deadline_passed = has_timeout && GetMilliSeconds() >= deadline;
    }

    ProcessResult result;
    result.exit_code = m_exit_code;
    result.was_killed = m_was_killed;
    result.stdout_text = Move(m_stdout_text);
    result.stderr_text = Move(m_stderr_text);
    Release();
    return Result(Move(result));
}

void Opal::Process::Release()
{
    if (m_process != k_no_handle && !m_exited)
    {
        const pid_t pid = static_cast<pid_t>(m_process);
        int status = 0;
        if (waitpid(pid, &status, WNOHANG) != pid)
        {
            KillTree(pid);
            while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
            {
            }
        }
    }
    for (Pipe* stream : {&m_stdout, &m_stderr})
    {
        if (stream->handle != k_no_handle)
        {
            close(ToFd(stream->handle));
        }
        *stream = Pipe{};
    }
    if (m_job != k_no_handle)
    {
        close(ToFd(m_job));
    }
    m_process = k_no_handle;
    m_job = k_no_handle;
    m_id = 0;
    m_exited = false;
    m_was_killed = false;
    m_exit_code = 0;
    m_stdout_text = StringUtf8();
    m_stderr_text = StringUtf8();
}

#else
#error "Platform not supported"
#endif

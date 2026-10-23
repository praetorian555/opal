#pragma once

#include "opal/container/array-view.h"
#include "opal/container/expected.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

/** Where a child process's output stream goes. */
enum class ProcessOutput : u8
{
    /** Collected and returned in ProcessResult. */
    Capture,
    /** Written to the parent's stream of the same kind. */
    Inherit,
    /** Thrown away. */
    Discard,
    /** For stderr only: interleaved into stdout in the order the child writes, as a terminal would show it. */
    MergeWithStdout,
};

/** What to start and how. */
struct ProcessDesc
{
    /**
     * Program to run: a path, or a bare name looked up on the child's PATH, which is the PATH entry in environment when it sets
     * one and the parent's PATH otherwise. On Windows a name without an extension gets ".exe".
     */
    StringViewUtf8 executable;
    /** Arguments after the program name, passed through unchanged. Batch files need an explicit "cmd /c" on Windows. */
    ArrayView<const StringViewUtf8> arguments;
    /** Working directory of the child. Empty uses the parent's. */
    StringViewUtf8 working_directory;
    /** When false the child starts from an empty environment holding only the entries below. */
    bool inherit_environment = true;
    /** "NAME=VALUE" entries added to the environment, replacing inherited entries of the same name. */
    ArrayView<const StringViewUtf8> environment;
    ProcessOutput stdout_mode = ProcessOutput::Capture;
    ProcessOutput stderr_mode = ProcessOutput::Capture;
};

/** How a child process ended and what it wrote. */
struct ProcessResult
{
    /** The exit code. On Linux a process ended by a signal reports 128 plus the signal number. */
    i32 exit_code = 0;
    /** True when Kill or the destructor of its Process ended it. */
    bool was_killed = false;
    /** Captured stdout, as the bytes the child wrote. Not validated as UTF-8. */
    StringUtf8 stdout_text;
    /** Captured stderr, as the bytes the child wrote. Not validated as UTF-8. */
    StringUtf8 stderr_text;
};

/**
 * A child process together with every process it starts. Its stdin is the null device.
 *
 * Destroying a Process whose child is still running kills the child and its descendants. Descendants that outlive a child that
 * exited on its own are left running.
 *
 * Methods called on a Process that is not valid return ErrorCode::InvalidArgument.
 */
class Process
{
public:
    /**
     * @brief Start a child process.
     * @param desc What to start and how.
     * @return The process, ErrorCode::PathNotFound when the executable or the working directory is not found,
     *         ErrorCode::InvalidArgument for an environment entry without '=' or a command line longer than the platform allows
     *         (32767 UTF-16 code units on Windows, where long argument lists belong in a response file), ErrorCode::OutOfMemory,
     *         or ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] static Expected<Process, ErrorCode> Spawn(const ProcessDesc& desc);

    /** @brief Constructs a process that is not valid. */
    Process() = default;
    ~Process();

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&& other) noexcept;
    Process& operator=(Process&& other) noexcept;

    /** @brief True from Spawn until a wait returns the result. */
    [[nodiscard]] bool IsValid() const;

    /** @brief The operating system's id for the child, or 0 when not valid. */
    [[nodiscard]] u32 GetId() const;

    /**
     * @brief Wait for the child to exit and for its captured streams to close, reading them meanwhile so a child that writes a lot
     * never blocks on a full pipe. A descendant that keeps a captured stream open delays the return until it closes it. Returning
     * the result leaves the Process not valid.
     * @return The result, ErrorCode::OutOfMemory when the captured output could not be stored, or ErrorCode::OSFailure. A
     *         non-zero exit code is a result, not an error.
     */
    [[nodiscard]] Expected<ProcessResult, ErrorCode> Wait();

    /**
     * @brief Like Wait, but gives up after a time limit. Giving up leaves the child running and keeps what it wrote so far for a
     * later wait.
     * @param timeout_ms Longest time to wait, in milliseconds.
     * @return What Wait returns, or ErrorCode::TimedOut when the limit passed first.
     */
    [[nodiscard]] Expected<ProcessResult, ErrorCode> WaitFor(u64 timeout_ms);

    /**
     * @brief Kill the child and its descendants. Does not wait; a wait afterwards returns the result with was_killed set. Killing a
     * child that already exited does nothing.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the operating system refuses.
     */
    ErrorCode Kill();

private:
    static constexpr u64 k_no_handle = ~0ull;

    /** Read end of a captured stream, with the platform's read state. */
    struct Pipe
    {
        u64 handle = k_no_handle;
        void* state = nullptr;
    };

    [[nodiscard]] Expected<ProcessResult, ErrorCode> WaitImpl(bool has_timeout, u64 timeout_ms);
    void Release();

    u64 m_process = k_no_handle;
    u64 m_job = k_no_handle;
    u32 m_id = 0;
    bool m_exited = false;
    bool m_was_killed = false;
    i32 m_exit_code = 0;
    Pipe m_stdout;
    Pipe m_stderr;
    StringUtf8 m_stdout_text;
    StringUtf8 m_stderr_text;
};

/**
 * @brief Start a child process and wait for it.
 * @param desc What to start and how.
 * @return What Process::Spawn or Process::Wait returns.
 */
[[nodiscard]] Expected<ProcessResult, ErrorCode> RunProcess(const ProcessDesc& desc);

/**
 * @brief Read an environment variable of the current process.
 * @param name Name of the variable.
 * @return The value, ErrorCode::KeyNotFound when the variable is not set, ErrorCode::OutOfMemory, or ErrorCode::OSFailure.
 */
[[nodiscard]] Expected<StringUtf8, ErrorCode> GetEnvironmentVariable(StringViewUtf8 name);

/**
 * @brief Find an executable the way Process::Spawn does.
 * @param name A path, returned as is when a file is there, or a bare name to look up. On Windows a name without an extension gets
 *        ".exe".
 * @param search_path Directories to look in, separated by ';' on Windows and ':' on Linux. Empty uses the PATH of the current process.
 * @return Path of the executable, ErrorCode::PathNotFound when none is found, ErrorCode::OutOfMemory, or ErrorCode::OSFailure.
 */
[[nodiscard]] Expected<StringUtf8, ErrorCode> LocateExecutable(StringViewUtf8 name, StringViewUtf8 search_path = {});

}  // namespace Opal

# Processes

Header: `opal/process.h`

Start child processes, capture what they write, wait for them with or without a time limit, and kill them together with every
process they start. Built for tools that run many short-lived programs, such as a build system running compilers: output is
collected without deadlocks, nothing leaks between processes started in parallel, and a cancelled run leaves no orphans behind.

## Quick Start

```cpp
#include "opal/process.h"

const Opal::StringViewUtf8 arguments[] = {"/nologo", "/c", "main.cpp"};
Opal::ProcessDesc desc;
desc.executable = "cl";
desc.arguments = arguments;

Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> result = Opal::RunProcess(desc);
if (!result.HasValue())
{
    // The process could not be started: ErrorCode::PathNotFound when "cl" is not on PATH.
}
else if (result.GetValue().exit_code != 0)
{
    // It ran and failed; its diagnostics are in result.GetValue().stdout_text.
}
```

A non-zero exit code is a result, not an error. `Expected` carries an error only when the process could not be started or waited
for.

## Describing a Process

`ProcessDesc` holds views, so the strings it points at have to outlive the `Spawn` call. Nothing is kept after it returns.

| Field                 | Meaning                                                                                             |
|-----------------------|-----------------------------------------------------------------------------------------------------|
| `executable`          | A path, or a bare name looked up on PATH (see below)                                                |
| `arguments`           | Arguments after the program name, passed through unchanged                                          |
| `working_directory`   | The child's working directory; empty uses the parent's                                              |
| `inherit_environment` | `false` starts the child from an empty environment                                                  |
| `environment`         | `"NAME=VALUE"` entries added to the environment, replacing inherited entries of the same name       |
| `stdout_mode`         | Where stdout goes: `Capture` (the default), `Inherit` or `Discard`                                  |
| `stderr_mode`         | As `stdout_mode`, plus `MergeWithStdout` to interleave stderr into stdout in the order it was written |

The child's stdin is always the null device, so a child that reads input sees end of file instead of hanging.

### Arguments

Arguments reach the child exactly as given: spaces, quotes, backslashes and non-ASCII text survive. On Windows, where a process
receives a single command line, Opal quotes each argument by the rules the C runtime and `CommandLineToArgvW` use to split it
again. Batch files are parsed by `cmd.exe` with different rules, so run them through `cmd /c` explicitly.

The command line on Windows is limited to 32767 UTF-16 code units. A longer one makes `Spawn` return `ErrorCode::InvalidArgument`;
pass long argument lists through a response file instead, as compilers and linkers accept.

### Finding the Executable

A bare name is looked up on the PATH the child will see: the PATH entry in `environment` when the description sets one, and the
parent's PATH otherwise. On Windows a name without an extension gets `.exe`. `LocateExecutable` runs the same lookup on its own:

```cpp
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> cl = Opal::LocateExecutable("cl");
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> tool = Opal::LocateExecutable("obsidian", "D:/tools/obsidian/bin");
```

### Environment

A build that must not depend on the shell it runs in passes `inherit_environment = false` and spells out every variable the tool
needs. Names compare without case on Windows, as the operating system does.

```cpp
const Opal::StringViewUtf8 environment[] = {"PATH=C:/msvc/bin", "VSLANG=1033"};
desc.inherit_environment = false;
desc.environment = environment;
```

`GetEnvironmentVariable` reads a variable of the current process, returning `ErrorCode::KeyNotFound` when it is not set.

## Waiting

`Spawn` starts the process and returns a `Process` handle. `Wait` blocks until the child exits and both captured streams are
closed, reading the streams the whole time, so a child that writes megabytes to stdout and stderr never stalls on a full pipe.
Returning the result releases the handle: `IsValid()` is then false.

```cpp
Opal::Expected<Opal::Process, Opal::ErrorCode> process = Opal::Process::Spawn(desc);
// ...
Opal::Expected<Opal::ProcessResult, Opal::ErrorCode> result = process.GetValue().WaitFor(30'000);
if (!result.HasValue() && result.GetError() == Opal::ErrorCode::TimedOut)
{
    process.GetValue().Kill();
    result = process.GetValue().Wait();   // result.GetValue().was_killed is true
}
```

`WaitFor` gives up after the given number of milliseconds and returns `ErrorCode::TimedOut`. The child keeps running, and what it
wrote so far is kept for the next wait.

A descendant that inherits a captured stream and keeps it open delays `Wait` until it closes it, because the output is only
complete once every writer is done.

Waiting blocks the calling thread. To run many processes at once, run each `Spawn` and `Wait` on a worker, for example a
`ThreadPool` task per process.

## Killing

`Kill` ends the child and every process it started, and does not wait; a wait afterwards returns the result with `was_killed` set.
Killing a child that already exited does nothing.

Destroying a `Process` whose child is still running kills it the same way, so dropping handles on cancellation leaves nothing
behind. Descendants that outlive a child that exited on its own are left running: a compiler that started a shared server process
for its siblings keeps working.

How descendants are reached differs by platform:

- **Windows:** the child runs in a job object, which every process it starts joins. Killing ends the whole job.
- **Linux:** the child leads its own process group, and killing also follows the parent links in `/proc` to reach descendants that
  started groups of their own. Each process found is stopped before it is killed, so none can start more processes in between. A
  descendant whose parent already exited has been adopted by `init` and is out of reach.

The exit code of a killed process is 1 on Windows and 137 (128 plus `SIGKILL`) on Linux.

## Output Encoding

`stdout_text` and `stderr_text` hold the bytes the child wrote, not validated as UTF-8. Many Windows tools write in the console's
code page rather than UTF-8. Setting `VSLANG=1033` in the environment of MSVC's tools makes them write English messages, which
keeps their output parseable.

## Errors

| Function           | Error                         | When                                                                    |
|--------------------|-------------------------------|-------------------------------------------------------------------------|
| `Spawn`            | `PathNotFound`                | The executable or the working directory does not exist                  |
| `Spawn`            | `InvalidArgument`             | An environment entry has no name, stdout is `MergeWithStdout`, or the command line is too long |
| `Wait`, `WaitFor`  | `TimedOut`                    | The time limit passed first (`WaitFor` only)                            |
| any method         | `InvalidArgument`             | The `Process` is not valid                                              |
| any                | `OutOfMemory`, `OSFailure`    | Allocation failed, or the operating system refused                      |

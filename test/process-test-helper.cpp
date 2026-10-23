// A program the process tests start. The first argument picks what it does:
//
//   echo-args ARGS...       print each argument as "<length>:<argument>" on its own line
//   write OUT ERR           write OUT bytes to stdout and ERR bytes to stderr, alternating in chunks
//   interleave              write "out1\n", "err1\n", "out2\n" to stdout, stderr, stdout, flushing after each
//   exit CODE               exit with CODE
//   sleep MS                sleep MS milliseconds, then exit with 0
//   env NAME                print the value of environment variable NAME, or "<unset>"
//   cwd                     print the working directory
//   stdin                   read stdin to its end and print how many bytes it held
//   grandchild PIDFILE      start "sleep 60000" as a child, write its id to PIDFILE, then sleep 60000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opal/file-system.h"
#include "opal/paths.h"
#include "opal/process.h"
#include "opal/threading/thread.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <shellapi.h>
#undef GetEnvironmentVariable
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#endif

#include "opal/container/dynamic-array.h"

using namespace Opal;

namespace
{

void WriteBytes(FILE* stream, char8 value, u64 count)
{
    char8 chunk[4096];
    memset(chunk, value, sizeof(chunk));
    const u64 size = count < sizeof(chunk) ? count : sizeof(chunk);
    fwrite(chunk, 1, size, stream);
    fflush(stream);
}

}  // namespace

int main(int argc, char* argv[])
{
#if defined(OPAL_PLATFORM_WINDOWS)
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdin), _O_BINARY);

    // The narrow argv is in the ANSI code page; rebuild it as UTF-8 from the wide command line.
    int wide_count = 0;
    wchar_t** wide_arguments = CommandLineToArgvW(GetCommandLineW(), &wide_count);
    DynamicArray<StringUtf8> utf8_arguments;
    DynamicArray<char*> utf8_pointers;
    for (int i = 0; i < wide_count; ++i)
    {
        StringUtf8 argument;
        if (Transcode(StringViewWide(wide_arguments[i]), argument) != ErrorCode::Success ||
            utf8_arguments.PushBack(Move(argument)) != ErrorCode::Success)
        {
            return 104;
        }
    }
    LocalFree(wide_arguments);
    for (StringUtf8& argument : utf8_arguments)
    {
        if (utf8_pointers.PushBack(argument.GetData()) != ErrorCode::Success)
        {
            return 104;
        }
    }
    argc = wide_count;
    argv = utf8_pointers.GetData();
#endif
    if (argc < 2)
    {
        return 100;
    }
    const StringViewUtf8 command(argv[1]);
    if (command == "echo-args")
    {
        for (int i = 2; i < argc; ++i)
        {
            printf("%zu:%s\n", strlen(argv[i]), argv[i]);
        }
        return 0;
    }
    if (command == "write" && argc == 4)
    {
        u64 out_left = strtoull(argv[2], nullptr, 10);
        u64 err_left = strtoull(argv[3], nullptr, 10);
        while (out_left > 0 || err_left > 0)
        {
            const u64 out_now = out_left < 4096 ? out_left : 4096;
            const u64 err_now = err_left < 4096 ? err_left : 4096;
            WriteBytes(stdout, 'o', out_now);
            WriteBytes(stderr, 'e', err_now);
            out_left -= out_now;
            err_left -= err_now;
        }
        return 0;
    }
    if (command == "interleave")
    {
        fputs("out1\n", stdout);
        fflush(stdout);
        fputs("err1\n", stderr);
        fflush(stderr);
        fputs("out2\n", stdout);
        fflush(stdout);
        return 0;
    }
    if (command == "exit" && argc == 3)
    {
        return atoi(argv[2]);
    }
    if (command == "sleep" && argc == 3)
    {
        SleepCurrentThread(strtoull(argv[2], nullptr, 10));
        return 0;
    }
    if (command == "env" && argc == 3)
    {
        Expected<StringUtf8, ErrorCode> value = GetEnvironmentVariable(argv[2]);
        fputs(value.HasValue() ? value.GetValue().GetData() : "<unset>", stdout);
        return 0;
    }
    if (command == "cwd")
    {
        Expected<StringUtf8, ErrorCode> cwd = Paths::GetCurrentWorkingDirectory();
        if (!cwd.HasValue())
        {
            return 101;
        }
        fputs(cwd.GetValue().GetData(), stdout);
        return 0;
    }
    if (command == "stdin")
    {
        u64 total = 0;
        char8 buffer[4096];
        size_t count = 0;
        while ((count = fread(buffer, 1, sizeof(buffer), stdin)) > 0)
        {
            total += count;
        }
        printf("%llu", static_cast<unsigned long long>(total));
        return 0;
    }
    if (command == "grandchild" && argc == 3)
    {
        const StringViewUtf8 sleep_arguments[] = {"sleep", "60000"};
        ProcessDesc desc;
        desc.executable = argv[0];
        desc.arguments = sleep_arguments;
        desc.stdout_mode = ProcessOutput::Discard;
        desc.stderr_mode = ProcessOutput::Discard;
        Expected<Process, ErrorCode> child = Process::Spawn(desc);
        if (!child.HasValue())
        {
            return 102;
        }
        char8 id[32] = {};
        snprintf(id, sizeof(id), "%u", child.GetValue().GetId());
        if (WriteStringToFile(argv[2], id) != ErrorCode::Success)
        {
            return 103;
        }
        SleepCurrentThread(60000);
        return 0;
    }
    return 100;
}

#include "opal/time.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <ctime>
#else
#error "Unsupported platform"
#endif

#include "opal/types.h"

namespace Opal
{

namespace PrivateTime
{
#if defined(OPAL_PLATFORM_WINDOWS)
LARGE_INTEGER g_frequency{.QuadPart = 0};
#endif
}  // namespace PrivateTime

f64 GetSeconds()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    if (PrivateTime::g_frequency.QuadPart == 0) [[unlikely]]
    {
        QueryPerformanceFrequency(&PrivateTime::g_frequency);
    }

    LARGE_INTEGER number_of_ticks;
    QueryPerformanceCounter(&number_of_ticks);
    const f64 seconds = static_cast<f64>(number_of_ticks.QuadPart) / static_cast<f64>(PrivateTime::g_frequency.QuadPart);
    return seconds;
#elif defined(OPAL_PLATFORM_LINUX)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<f64>(ts.tv_sec) + (static_cast<f64>(ts.tv_nsec) / 1'000'000'000);
#endif
}

f64 GetMilliSeconds()
{
    return GetSeconds() * 1'000;
}

f64 GetMicroSeconds()
{
    return GetSeconds() * 1'000'000;
}

f64 GetLastFileModifiedTimeInSeconds(const StringUtf8& file_path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide wide_file_path(file_path.GetSize() + 1, 0);
    const ErrorCode err = Transcode(file_path, wide_file_path);
    if (err != ErrorCode::Success)
    {
        return -1;
    }
    HANDLE file_handle = CreateFile(wide_file_path.GetData(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                          nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    FILETIME last_write_time;
    FILETIME creation_time;
    FILETIME access_time;
    if (GetFileTime(file_handle, &creation_time, &access_time, &last_write_time) == 0)
    {
        return -1;
    }
    CloseHandle(file_handle);

    ULARGE_INTEGER uli;
    uli.LowPart = last_write_time.dwLowDateTime;
    uli.HighPart = last_write_time.dwHighDateTime;
    return static_cast<f64>(uli.QuadPart / 10'000'000);

#elif defined(OPAL_PLATFORM_LINUX)
    return -1;
#endif
}

}  // namespace Opal

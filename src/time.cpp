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
}

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

}  // namespace Opal

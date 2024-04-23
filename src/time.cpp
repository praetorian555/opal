#include "opal/time.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "opal/types.h"

namespace Opal
{

namespace PrivateTime
{
LARGE_INTEGER g_frequency{.QuadPart = 0};
}

f64 GetSeconds()
{
    if (PrivateTime::g_frequency.QuadPart == 0) [[unlikely]]
    {
        QueryPerformanceFrequency(&PrivateTime::g_frequency);
    }

    LARGE_INTEGER number_of_ticks;
    QueryPerformanceCounter(&number_of_ticks);
    const f64 seconds = static_cast<f64>(number_of_ticks.QuadPart) / static_cast<f64>(PrivateTime::g_frequency.QuadPart);
    return seconds;
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

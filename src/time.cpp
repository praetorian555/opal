#include "opal/time.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <errno.h>
#include <sys/stat.h>
#else
#error "Unsupported platform"
#endif

#include <stdint.h>
#include <time.h>

#include "opal/types.h"

namespace
{

using Opal::i32;
using Opal::i64;
using Opal::u16;
using Opal::u32;
using Opal::u64;
using Opal::u8;

constexpr i64 k_nanoseconds_per_second = 1'000'000'000;
constexpr i64 k_nanoseconds_per_millisecond = 1'000'000;
constexpr i64 k_seconds_per_day = 86'400;
constexpr i64 k_seconds_per_hour = 3'600;
constexpr i64 k_seconds_per_minute = 60;

#if defined(OPAL_PLATFORM_WINDOWS)
/** 100-nanosecond intervals between the FILETIME epoch of 1601-01-01 and the Unix epoch. */
constexpr i64 k_filetime_epoch_offset = 116'444'736'000'000'000;
#endif

/** Rounds towards negative infinity, unlike the built-in operators, which round towards zero. */
i64 FloorDiv(i64 numerator, i64 denominator)
{
    const i64 quotient = numerator / denominator;
    const i64 remainder = numerator % denominator;
    return (remainder != 0 && ((remainder < 0) != (denominator < 0))) ? quotient - 1 : quotient;
}

i64 FloorMod(i64 numerator, i64 denominator)
{
    return numerator - FloorDiv(numerator, denominator) * denominator;
}

/**
 * Days elapsed since the epoch for a proleptic Gregorian date. Both directions are Howard Hinnant's
 * civil calendar algorithms, whose era arithmetic sidesteps the per-month tables and holds for any
 * year an i32 can name.
 */
i64 DaysFromCivil(i32 year, u8 month, u8 day)
{
    const i64 shifted_year = year - (month <= 2 ? 1 : 0);
    const i64 era = FloorDiv(shifted_year, 400);
    const u64 year_of_era = static_cast<u64>(shifted_year - era * 400);
    const i64 month_index = static_cast<i64>(month) + (month > 2 ? -3 : 9);
    const u64 day_of_year = static_cast<u64>((153 * month_index + 2) / 5 + day - 1);
    const u64 day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
    return era * 146'097 + static_cast<i64>(day_of_era) - 719'468;
}

void CivilFromDays(i64 days, i32& out_year, u8& out_month, u8& out_day)
{
    const i64 shifted_days = days + 719'468;
    const i64 era = FloorDiv(shifted_days, 146'097);
    const u64 day_of_era = static_cast<u64>(shifted_days - era * 146'097);
    const u64 year_of_era = (day_of_era - day_of_era / 1460 + day_of_era / 36'524 - day_of_era / 146'096) / 365;
    const i64 shifted_year = static_cast<i64>(year_of_era) + era * 400;
    const u64 day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
    const u64 month_index = (5 * day_of_year + 2) / 153;

    out_day = static_cast<u8>(day_of_year - (153 * month_index + 2) / 5 + 1);
    out_month = static_cast<u8>(month_index < 10 ? month_index + 3 : month_index - 9);
    out_year = static_cast<i32>(shifted_year + (out_month <= 2 ? 1 : 0));
}

bool AreFieldsInRange(const Opal::CalendarDate& date)
{
    return date.month >= 1 && date.month <= 12 && date.day >= 1 && date.day <= Opal::GetDaysInMonth(date.year, date.month) &&
           date.hour <= 23 && date.minute <= 59 && date.second <= 59 && date.millisecond <= 999;
}

/** Folds seconds and a millisecond field into a nanosecond count. False when that count overflows. */
bool TryMakeDateTime(i64 seconds, u16 milliseconds, Opal::DateTime& out_date_time)
{
    constexpr i64 k_max_seconds = INT64_MAX / k_nanoseconds_per_second;
    constexpr i64 k_min_seconds = INT64_MIN / k_nanoseconds_per_second;
    if (seconds > k_max_seconds || seconds < k_min_seconds)
    {
        return false;
    }

    const i64 nanoseconds_from_milliseconds = static_cast<i64>(milliseconds) * k_nanoseconds_per_millisecond;
    const i64 nanoseconds_from_seconds = seconds * k_nanoseconds_per_second;
    if (nanoseconds_from_seconds > INT64_MAX - nanoseconds_from_milliseconds)
    {
        return false;
    }

    out_date_time.nanoseconds_since_epoch = nanoseconds_from_seconds + nanoseconds_from_milliseconds;
    return true;
}

/** Splits an instant into whole seconds since the epoch and the millisecond field of that second. */
void SplitSeconds(Opal::DateTime date_time, i64& out_seconds, Opal::u16& out_milliseconds)
{
    out_seconds = FloorDiv(date_time.nanoseconds_since_epoch, k_nanoseconds_per_second);
    const i64 nanosecond_of_second = FloorMod(date_time.nanoseconds_since_epoch, k_nanoseconds_per_second);
    out_milliseconds = static_cast<Opal::u16>(nanosecond_of_second / k_nanoseconds_per_millisecond);
}

}  // namespace

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

DateTime GetCurrentDateTime()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    FILETIME file_time;
    GetSystemTimePreciseAsFileTime(&file_time);
    ULARGE_INTEGER ticks;
    ticks.LowPart = file_time.dwLowDateTime;
    ticks.HighPart = file_time.dwHighDateTime;
    return DateTime{(static_cast<i64>(ticks.QuadPart) - k_filetime_epoch_offset) * 100};
#elif defined(OPAL_PLATFORM_LINUX)
    struct timespec now = {};
    if (clock_gettime(CLOCK_REALTIME, &now) != 0)
    {
        return DateTime{};
    }
    return DateTime{static_cast<i64>(now.tv_sec) * k_nanoseconds_per_second + static_cast<i64>(now.tv_nsec)};
#endif
}

CalendarDate ToUtc(DateTime date_time)
{
    i64 seconds = 0;
    u16 milliseconds = 0;
    SplitSeconds(date_time, seconds, milliseconds);

    const i64 days = FloorDiv(seconds, k_seconds_per_day);
    const i64 second_of_day = FloorMod(seconds, k_seconds_per_day);

    CalendarDate date = {};
    CivilFromDays(days, date.year, date.month, date.day);
    date.hour = static_cast<u8>(second_of_day / k_seconds_per_hour);
    date.minute = static_cast<u8>((second_of_day / k_seconds_per_minute) % k_seconds_per_minute);
    date.second = static_cast<u8>(second_of_day % k_seconds_per_minute);
    date.millisecond = milliseconds;
    date.day_of_week = static_cast<u8>(FloorMod(days + 4, 7));
    date.day_of_year = static_cast<u16>(days - DaysFromCivil(date.year, 1, 1) + 1);
    return date;
}

Expected<CalendarDate, ErrorCode> ToLocal(DateTime date_time)
{
    using Result = Expected<CalendarDate, ErrorCode>;

    i64 seconds = 0;
    u16 milliseconds = 0;
    SplitSeconds(date_time, seconds, milliseconds);

    const time_t as_time_t = static_cast<time_t>(seconds);
    struct tm fields = {};
#if defined(OPAL_PLATFORM_WINDOWS)
    if (localtime_s(&fields, &as_time_t) != 0)
    {
        return Result(ErrorCode::OSFailure);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    if (localtime_r(&as_time_t, &fields) == nullptr)
    {
        return Result(ErrorCode::OSFailure);
    }
#endif

    CalendarDate date = {};
    date.year = fields.tm_year + 1900;
    date.month = static_cast<u8>(fields.tm_mon + 1);
    date.day = static_cast<u8>(fields.tm_mday);
    date.hour = static_cast<u8>(fields.tm_hour);
    date.minute = static_cast<u8>(fields.tm_min);
    date.second = static_cast<u8>(fields.tm_sec > 59 ? 59 : fields.tm_sec);
    date.millisecond = milliseconds;
    date.day_of_week = static_cast<u8>(fields.tm_wday);
    date.day_of_year = static_cast<u16>(fields.tm_yday + 1);
    return Result(date);
}

Expected<DateTime, ErrorCode> FromUtc(const CalendarDate& date)
{
    using Result = Expected<DateTime, ErrorCode>;
    if (!AreFieldsInRange(date))
    {
        return Result(ErrorCode::InvalidArgument);
    }

    const i64 days = DaysFromCivil(date.year, date.month, date.day);
    const i64 seconds = days * k_seconds_per_day + date.hour * k_seconds_per_hour + date.minute * k_seconds_per_minute + date.second;

    DateTime date_time = {};
    if (!TryMakeDateTime(seconds, date.millisecond, date_time))
    {
        return Result(ErrorCode::InvalidArgument);
    }
    return Result(date_time);
}

Expected<DateTime, ErrorCode> FromLocal(const CalendarDate& date)
{
    using Result = Expected<DateTime, ErrorCode>;
    if (!AreFieldsInRange(date))
    {
        return Result(ErrorCode::InvalidArgument);
    }

    struct tm fields = {};
    fields.tm_year = date.year - 1900;
    fields.tm_mon = date.month - 1;
    fields.tm_mday = date.day;
    fields.tm_hour = date.hour;
    fields.tm_min = date.minute;
    fields.tm_sec = date.second;
    fields.tm_isdst = -1;

    const time_t seconds = mktime(&fields);
    if (seconds == static_cast<time_t>(-1))
    {
        return Result(ErrorCode::OSFailure);
    }

    DateTime date_time = {};
    if (!TryMakeDateTime(static_cast<i64>(seconds), date.millisecond, date_time))
    {
        return Result(ErrorCode::InvalidArgument);
    }
    return Result(date_time);
}

bool IsLeapYear(i32 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

u8 GetDaysInMonth(i32 year, u8 month)
{
    constexpr u8 k_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12)
    {
        return 0;
    }
    if (month == 2 && IsLeapYear(year))
    {
        return 29;
    }
    return k_days[month - 1];
}

Expected<DateTime, ErrorCode> GetLastFileModifiedTime(const StringUtf8& file_path)
{
    using Result = Expected<DateTime, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide wide_file_path(file_path.GetSize() + 1, 0);
    if (Transcode(file_path, wide_file_path) != ErrorCode::Success)
    {
        return Result(ErrorCode::InvalidArgument);
    }

    WIN32_FILE_ATTRIBUTE_DATA attributes = {};
    if (GetFileAttributesExW(wide_file_path.GetData(), GetFileExInfoStandard, &attributes) == 0)
    {
        const DWORD error = GetLastError();
        const bool missing = error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
        return Result(missing ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }

    ULARGE_INTEGER ticks;
    ticks.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
    ticks.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
    return Result(DateTime{(static_cast<i64>(ticks.QuadPart) - k_filetime_epoch_offset) * 100});
#elif defined(OPAL_PLATFORM_LINUX)
    struct stat info = {};
    if (stat(*file_path, &info) != 0)
    {
        return Result(errno == ENOENT || errno == ENOTDIR ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }
    return Result(DateTime{static_cast<i64>(info.st_mtim.tv_sec) * k_nanoseconds_per_second + static_cast<i64>(info.st_mtim.tv_nsec)});
#endif
}

}  // namespace Opal

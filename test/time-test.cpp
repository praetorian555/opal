#include <stdio.h>

#include "test-helpers.h"

#include "opal/time.h"

using namespace Opal;

OPAL_DISABLE_OPTIMIZATION

TEST_CASE("Get time stamp", "[Time]")
{
    const f64 seconds = Opal::GetSeconds();
    REQUIRE(seconds >= 0);
    const f64 milliseconds = Opal::GetMilliSeconds();
    REQUIRE(milliseconds >= 0);
    const f64 microseconds = Opal::GetMicroSeconds();
    REQUIRE(microseconds >= 0);
}

TEST_CASE("Measuring short time", "[Time]")
{
    const f64 start = Opal::GetMicroSeconds();
    for (i64 i = 0; i < 100'000; i++)
    {
        i64 j = i * i;
        (void)j;
    }
    const f64 end = Opal::GetMicroSeconds();
    REQUIRE(end > start);
    const f64 duration = end - start;
    REQUIRE(duration > 0);
}

TEST_CASE("Measuring long time", "[Time]")
{
    const f64 start = Opal::GetMilliSeconds();
    for (i64 i = 0; i < 1'000'000'000; i++)
    {
        i64 j = i + i;
        (void)j;
    }
    const f64 end = Opal::GetMilliSeconds();
    REQUIRE(end > start);
    const f64 duration = end - start;
    REQUIRE(duration > 0);
}

OPAL_ENABLE_OPTIMIZATION

namespace
{

CalendarDate MakeDate(i32 year, u8 month, u8 day, u8 hour = 0, u8 minute = 0, u8 second = 0, u16 millisecond = 0)
{
    CalendarDate date = {};
    date.year = year;
    date.month = month;
    date.day = day;
    date.hour = hour;
    date.minute = minute;
    date.second = second;
    date.millisecond = millisecond;
    return date;
}

}  // namespace

TEST_CASE("Leap years", "[Time]")
{
    CHECK(IsLeapYear(2024));
    CHECK(IsLeapYear(2000));
    CHECK_FALSE(IsLeapYear(1900));
    CHECK_FALSE(IsLeapYear(2023));

    CHECK(GetDaysInMonth(2024, 2) == 29);
    CHECK(GetDaysInMonth(2023, 2) == 28);
    CHECK(GetDaysInMonth(1900, 2) == 28);
    CHECK(GetDaysInMonth(2026, 1) == 31);
    CHECK(GetDaysInMonth(2026, 4) == 30);
    CHECK(GetDaysInMonth(2026, 12) == 31);
    CHECK(GetDaysInMonth(2026, 0) == 0);
    CHECK(GetDaysInMonth(2026, 13) == 0);
}

TEST_CASE("Split a UTC instant into fields", "[Time]")
{
    SECTION("The epoch itself")
    {
        const CalendarDate date = ToUtc(DateTime{0});
        CHECK(date.year == 1970);
        CHECK(date.month == 1);
        CHECK(date.day == 1);
        CHECK(date.hour == 0);
        CHECK(date.minute == 0);
        CHECK(date.second == 0);
        CHECK(date.millisecond == 0);
        CHECK(date.day_of_week == 4);  // 1970-01-01 was a Thursday.
        CHECK(date.day_of_year == 1);
    }
    SECTION("A known instant")
    {
        // 2026-08-08T16:19:05.190Z
        const CalendarDate date = ToUtc(DateTime{1'786'205'945'190'000'000});
        CHECK(date.year == 2026);
        CHECK(date.month == 8);
        CHECK(date.day == 8);
        CHECK(date.hour == 16);
        CHECK(date.minute == 19);
        CHECK(date.second == 5);
        CHECK(date.millisecond == 190);
        CHECK(date.day_of_week == 6);  // A Saturday.
        CHECK(date.day_of_year == 220);
    }
    SECTION("Sub-second remainders do not leak into the seconds")
    {
        const CalendarDate date = ToUtc(DateTime{999'999'999});
        CHECK(date.second == 0);
        CHECK(date.millisecond == 999);
    }
    SECTION("Instants before the epoch count backwards")
    {
        const CalendarDate date = ToUtc(DateTime{-1});
        CHECK(date.year == 1969);
        CHECK(date.month == 12);
        CHECK(date.day == 31);
        CHECK(date.hour == 23);
        CHECK(date.minute == 59);
        CHECK(date.second == 59);
        CHECK(date.millisecond == 999);
    }
    SECTION("The leap day exists")
    {
        const Expected<DateTime, ErrorCode> instant = FromUtc(MakeDate(2024, 2, 29, 12));
        REQUIRE(instant.HasValue());
        const CalendarDate date = ToUtc(instant.GetValue());
        CHECK(date.month == 2);
        CHECK(date.day == 29);
        CHECK(date.day_of_year == 60);
    }
}

TEST_CASE("Build a UTC instant from fields", "[Time]")
{
    SECTION("The epoch itself")
    {
        const Expected<DateTime, ErrorCode> instant = FromUtc(MakeDate(1970, 1, 1));
        REQUIRE(instant.HasValue());
        CHECK(instant.GetValue().nanoseconds_since_epoch == 0);
    }
    SECTION("A known instant")
    {
        const Expected<DateTime, ErrorCode> instant = FromUtc(MakeDate(2026, 8, 8, 16, 19, 5, 190));
        REQUIRE(instant.HasValue());
        CHECK(instant.GetValue().nanoseconds_since_epoch == 1'786'205'945'190'000'000);
    }
    SECTION("Round trips over a spread of dates")
    {
        const CalendarDate dates[] = {MakeDate(1678, 1, 1),          MakeDate(1899, 12, 31, 23, 59, 59, 999),
                                      MakeDate(1969, 7, 20, 20, 17), MakeDate(2000, 2, 29, 12, 30, 30, 500),
                                      MakeDate(2026, 8, 8),          MakeDate(2262, 4, 11)};
        for (const CalendarDate& original : dates)
        {
            const Expected<DateTime, ErrorCode> instant = FromUtc(original);
            REQUIRE(instant.HasValue());
            const CalendarDate round_tripped = ToUtc(instant.GetValue());
            CHECK(round_tripped.year == original.year);
            CHECK(round_tripped.month == original.month);
            CHECK(round_tripped.day == original.day);
            CHECK(round_tripped.hour == original.hour);
            CHECK(round_tripped.minute == original.minute);
            CHECK(round_tripped.second == original.second);
            CHECK(round_tripped.millisecond == original.millisecond);
        }
    }
    SECTION("Every day of a leap year and the year after round trips")
    {
        for (i32 year = 2024; year <= 2025; ++year)
        {
            u16 days_seen = 0;
            for (u8 month = 1; month <= 12; ++month)
            {
                for (u8 day = 1; day <= GetDaysInMonth(year, month); ++day)
                {
                    const Expected<DateTime, ErrorCode> instant = FromUtc(MakeDate(year, month, day));
                    REQUIRE(instant.HasValue());
                    const CalendarDate date = ToUtc(instant.GetValue());
                    REQUIRE(date.year == year);
                    REQUIRE(date.month == month);
                    REQUIRE(date.day == day);
                    ++days_seen;
                    REQUIRE(date.day_of_year == days_seen);
                }
            }
            CHECK(days_seen == (year == 2024 ? 366 : 365));
        }
    }
    SECTION("Out of range fields are rejected")
    {
        CHECK(FromUtc(MakeDate(2026, 0, 1)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 13, 1)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 0)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 32)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2023, 2, 29)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 4, 31)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 1, 24)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 1, 0, 60)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 1, 0, 0, 60)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2026, 1, 1, 0, 0, 0, 1000)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
    }
    SECTION("Dates outside the representable range are rejected")
    {
        CHECK(FromUtc(MakeDate(1677, 1, 1)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(2263, 1, 1)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
        CHECK(FromUtc(MakeDate(9999, 1, 1)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
    }
}

TEST_CASE("Local time", "[Time]")
{
    SECTION("Splitting and rebuilding an instant returns it unchanged")
    {
        // A millisecond-aligned instant, so nothing is lost on the way through the fields.
        const DateTime original = DateTime{1'786'205'945'190'000'000};
        const Expected<CalendarDate, ErrorCode> local = ToLocal(original);
        REQUIRE(local.HasValue());

        const Expected<DateTime, ErrorCode> rebuilt = FromLocal(local.GetValue());
        REQUIRE(rebuilt.HasValue());
        CHECK(rebuilt.GetValue() == original);
    }
    SECTION("Local fields differ from UTC only by a whole number of minutes")
    {
        // Truncated to a whole second, so the fields carry the instant exactly and the difference below
        // is the zone offset alone.
        const DateTime now = DateTime{GetCurrentDateTime().nanoseconds_since_epoch / 1'000'000'000 * 1'000'000'000};
        const Expected<CalendarDate, ErrorCode> local = ToLocal(now);
        REQUIRE(local.HasValue());

        const Expected<DateTime, ErrorCode> as_utc = FromUtc(local.GetValue());
        REQUIRE(as_utc.HasValue());

        const i64 offset_nanoseconds = as_utc.GetValue().nanoseconds_since_epoch - now.nanoseconds_since_epoch;
        CHECK(offset_nanoseconds % (60 * 1'000'000'000LL) == 0);
        CHECK(offset_nanoseconds > -16 * 3'600 * 1'000'000'000LL);
        CHECK(offset_nanoseconds < 16 * 3'600 * 1'000'000'000LL);
    }
    SECTION("Out of range fields are rejected")
    {
        CHECK(FromLocal(MakeDate(2026, 2, 30)).GetErrorOr(ErrorCode::Success) == ErrorCode::InvalidArgument);
    }
}

TEST_CASE("Read the wall clock", "[Time]")
{
    const DateTime first = GetCurrentDateTime();
    const DateTime second = GetCurrentDateTime();
    CHECK(second >= first);

    // Anything outside this window means the clock or the epoch conversion is wrong, not that the
    // machine's clock is merely off.
    const CalendarDate date = ToUtc(first);
    CHECK(date.year >= 2024);
    CHECK(date.year < 2100);
    CHECK(date.month >= 1);
    CHECK(date.month <= 12);
    CHECK(date.day >= 1);
    CHECK(date.day <= 31);
}

TEST_CASE("Read a file's last modified time", "[Time]")
{
    SECTION("A file that exists reports a time near now")
    {
        const StringUtf8 path("time-test-modified-probe.txt");
        FILE* file = nullptr;
#if defined(OPAL_PLATFORM_WINDOWS)
        fopen_s(&file, *path, "wb");
#else
        file = fopen(*path, "wb");
#endif
        REQUIRE(file != nullptr);
        fwrite("probe", 1, 5, file);
        fclose(file);

        const Expected<DateTime, ErrorCode> modified = GetLastFileModifiedTime(path);
        remove(*path);

        REQUIRE(modified.HasValue());
        const i64 age_nanoseconds = GetCurrentDateTime().nanoseconds_since_epoch - modified.GetValue().nanoseconds_since_epoch;
        CHECK(age_nanoseconds > -60 * 1'000'000'000LL);
        CHECK(age_nanoseconds < 60 * 1'000'000'000LL);
    }
    SECTION("A missing file reports PathNotFound")
    {
        const Expected<DateTime, ErrorCode> modified = GetLastFileModifiedTime(StringUtf8("no-such-file-anywhere.txt"));
        CHECK_FALSE(modified.HasValue());
        CHECK(modified.GetErrorOr(ErrorCode::Success) == ErrorCode::PathNotFound);
    }
}

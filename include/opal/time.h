#pragma once

#include <compare>

#include "opal/container/expected.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

/*************************************************************************************************/
/** Monotonic clock ******************************************************************************/
/*************************************************************************************************/

/**
 * @brief Get number of seconds since the device started.
 * @note This clock only moves forward and is unrelated to the calendar. Use GetCurrentDateTime for
 *       a wall clock reading.
 * @return Number of seconds since the device started.
 */
f64 GetSeconds();

/**
 * @brief Get number of milliseconds since the device started.
 * @return Number of milliseconds since the device started.
 */
f64 GetMilliSeconds();

/**
 * @brief Get number of microseconds since the device started.
 * @return Number of microseconds since the device started.
 */
f64 GetMicroSeconds();

/*************************************************************************************************/
/** Wall clock ***********************************************************************************/
/*************************************************************************************************/

/**
 * An instant on the wall clock, counted in nanoseconds from 1970-01-01T00:00:00Z. Instants before
 * that point are negative.
 *
 * The range this can hold spans 1677-09-21 to 2262-04-11. Anything outside it is rejected rather
 * than wrapped.
 */
struct DateTime
{
    i64 nanoseconds_since_epoch = 0;

    bool operator==(const DateTime& other) const = default;
    auto operator<=>(const DateTime& other) const = default;
};

/**
 * An instant split into the fields a calendar names it by. The fields carry no time zone of their
 * own; which zone they are in is decided by the call that produced or consumes them.
 *
 * The default value is the epoch itself.
 */
struct CalendarDate
{
    /** Full year, for example 2026. */
    i32 year = 1970;
    /** Month of the year, 1 to 12. */
    u8 month = 1;
    /** Day of the month, 1 to 31. */
    u8 day = 1;
    /** Hour of the day, 0 to 23. */
    u8 hour = 0;
    /** Minute of the hour, 0 to 59. */
    u8 minute = 0;
    /** Second of the minute, 0 to 59. */
    u8 second = 0;
    /** Millisecond of the second, 0 to 999. */
    u16 millisecond = 0;
    /** Days since Sunday, 0 to 6. Derived on the way out, ignored on the way in. */
    u8 day_of_week = 4;
    /** Day of the year, 1 to 366. Derived on the way out, ignored on the way in. */
    u16 day_of_year = 1;
};

/**
 * Read the wall clock.
 * @return The current instant, or the epoch if the platform has no clock to read.
 */
DateTime GetCurrentDateTime();

/**
 * Split an instant into UTC calendar fields.
 * @param date_time Instant to split.
 * @return The fields naming that instant in UTC.
 */
CalendarDate ToUtc(DateTime date_time);

/**
 * Split an instant into calendar fields in the zone the system is configured for, accounting for
 * whichever daylight saving rule is in effect at that instant.
 * @param date_time Instant to split.
 * @return The fields, or ErrorCode::OSFailure when the local zone cannot be resolved.
 */
Expected<CalendarDate, ErrorCode> ToLocal(DateTime date_time);

/**
 * Build an instant from calendar fields read as UTC. The day_of_week and day_of_year fields are
 * ignored.
 * @param date Fields to read.
 * @return The instant, or ErrorCode::InvalidArgument when a field is out of range, the day does not
 *         exist in that month, or the result falls outside the range a DateTime can hold.
 */
Expected<DateTime, ErrorCode> FromUtc(const CalendarDate& date);

/**
 * Build an instant from calendar fields read as local time. The day_of_week and day_of_year fields
 * are ignored.
 *
 * An hour that daylight saving skips, or one it repeats, is resolved the way the platform resolves
 * it.
 * @param date Fields to read.
 * @return The instant, ErrorCode::InvalidArgument when a field is out of range or the day does not
 *         exist in that month, or ErrorCode::OSFailure when the local zone cannot be resolved.
 */
Expected<DateTime, ErrorCode> FromLocal(const CalendarDate& date);

/**
 * Check whether a year has 366 days in the proleptic Gregorian calendar.
 * @param year Full year, for example 2026.
 * @return True when the year is a leap year.
 */
bool IsLeapYear(i32 year);

/**
 * Get the number of days in a month.
 * @param year Full year, for example 2026.
 * @param month Month of the year, 1 to 12.
 * @return Days in that month, or 0 when the month is out of range.
 */
u8 GetDaysInMonth(i32 year, u8 month);

/**
 * Read the time a file was last written to.
 * @param file_path Path to the file to check.
 * @return The instant, ErrorCode::PathNotFound when no such file exists, or ErrorCode::OSFailure.
 */
Expected<DateTime, ErrorCode> GetLastFileModifiedTime(const StringUtf8& file_path);

}  // namespace Opal

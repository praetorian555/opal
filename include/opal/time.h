#pragma once

#include "opal/container/string.h"
#include "opal/types.h"

namespace Opal
{

/**
 * @brief Get number of seconds since the device started.
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

/**
 * Get number of seconds since January 1, 1601 (UTC) that represent a time when a file has been
 * last modified.
 * @param file_path Path to the file to check.
 * @return Returns elapsed seconds or -1 in case that file does not exist.
 */
f64 GetLastFileModifiedTimeInSeconds(const StringUtf8& file_path);

}  // namespace Opal
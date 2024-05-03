#pragma once

#include "opal/types.h"
#include "opal/export.h"

namespace Opal
{

/**
 * @brief Get amount of seconds since the device started.
 * @return Amount of seconds since the device started.
 */
OPAL_EXPORT f64 GetSeconds();

/**
 * @brief Get amount of milliseconds since the device started.
 * @return Amount of milliseconds since the device started.
 */
OPAL_EXPORT f64 GetMilliSeconds();

/**
 * @brief Get amount of microseconds since the device started.
 * @return Amount of microseconds since the device started.
 */
OPAL_EXPORT f64 GetMicroSeconds();

}  // namespace opal
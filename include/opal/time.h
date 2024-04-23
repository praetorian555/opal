#pragma once

#include "opal/types.h"

namespace Opal
{

/**
 * @brief Get amount of seconds since the device started.
 * @return Amount of seconds since the device started.
 */
f64 GetSeconds();

/**
 * @brief Get amount of milliseconds since the device started.
 * @return Amount of milliseconds since the device started.
 */
f64 GetMilliSeconds();

/**
 * @brief Get amount of microseconds since the device started.
 * @return Amount of microseconds since the device started.
 */
f64 GetMicroSeconds();

}  // namespace opal
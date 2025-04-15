#pragma once

#include <cassert>

#include "opal/defines.h"

#if defined(OPAL_DEBUG)
#define OPAL_ASSERT(condition, description) assert(condition && description)
#else
#define OPAL_ASSERT(condition, description)
#endif
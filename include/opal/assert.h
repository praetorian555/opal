#pragma once

#include <cassert>

#include "opal/defines.h"

namespace Opal
{

/** Called when a contract check fails. It is not expected to return; the default one writes to stderr and aborts. */
using ContractViolationHandler = void (*)(const char* message);

/**
 * Install the handler run when a contract check fails. Pass nullptr to restore the default.
 * @note Not thread-safe. Set it before the threads that could trip a check are started.
 */
void SetContractViolationHandler(ContractViolationHandler handler);

/** @return The handler currently installed. Never nullptr. */
ContractViolationHandler GetContractViolationHandler();

/** Runs the installed handler, then aborts if it returned anyway. */
[[noreturn]] void HandleContractViolation(const char* message);

}  // namespace Opal

#if defined(OPAL_DEBUG)
#define OPAL_ASSERT(condition, description) assert((condition) && description)
#else
#define OPAL_ASSERT(condition, description)
#endif

// Checks in every build, unlike OPAL_ASSERT, which is compiled out of release. For the contracts where losing the check would
// turn a caller's mistake into a silent out-of-bounds access rather than a diagnosable failure.
#define OPAL_VERIFY(condition, description)               \
    do                                                    \
    {                                                     \
        if (!(condition)) [[unlikely]]                    \
        {                                                 \
            ::Opal::HandleContractViolation(description); \
        }                                                 \
    } while (false)

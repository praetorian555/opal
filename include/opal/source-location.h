#pragma once

#include "opal/types.h"

namespace Opal
{

struct SourceLocation
{
    const char* file;
    const char* function;
    u32 line;
};

static consteval SourceLocation CurrentSourceLocation(const char* file = __builtin_FILE(), const char* function = __builtin_FUNCTION(),
                                                      u32 line = __builtin_LINE())
{
    return SourceLocation{file, function, line};
}

}  // namespace Opal

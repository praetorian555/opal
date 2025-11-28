#include "opal/container/string.h"

#include <cstdlib>

Opal::u32 Opal::StringToU32(const StringUtf8& str)
{
    char* end = nullptr;
    return Narrow<u32>(strtoul(*str, &end, 0));
}

Opal::i32 Opal::StringToI32(const Opal::StringUtf8& str)
{
    char* end = nullptr;
    return Narrow<i32>(strtol(*str, &end, 0));
}

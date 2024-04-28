#pragma once

#include "opal/types.h"

namespace Opal
{

enum class ErrorCode : u8
{
    Success = 0,
    OutOfBounds = 1,
    OutOfMemory = 2,
    BadInput = 3,
    EndOfString = 4,
    IncompleteSequence = 5,
    InsufficientSpace = 6
};

}

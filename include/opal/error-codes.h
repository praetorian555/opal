#pragma once

#include "opal/types.h"

namespace Opal
{

enum class ErrorCode : u8
{
    Success = 0,
    OutOfBounds = 1,
    OutOfMemory = 2,
    InvalidArgument = 3,
    EndOfString = 4,
    IncompleteSequence = 5,
    InsufficientSpace = 6,
    StringNotFound = 7,
    OSFailure = 8,
    SelfNotAllowed = 9,
    NotImplemented = 10,
    AlreadyExists = 11,
    PathNotFound = 12,
    NotEmpty
};

}

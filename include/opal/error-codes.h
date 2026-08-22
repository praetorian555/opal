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
    NotEmpty = 13,
    ChannelClosed = 14,
    ChannelEmpty = 15,
    NotDirectory = 16,
    /** The value is not of the requested type. */
    TypeMismatch = 17,
    /** No entry is stored under the requested key. */
    KeyNotFound = 18,
    /** A non-blocking operation had to wait, so it did nothing instead. */
    WouldBlock = 19,
    /** The peer closed the connection in an orderly way. */
    ConnectionClosed = 20,
    /** The remote host actively refused the connection. */
    ConnectionRefused = 21,
    /** The connection was torn down abruptly. */
    ConnectionReset = 22,
    /** The address is already bound by another socket. */
    AddressInUse = 23,
    /** The operation ran out of the time it was given. */
    TimedOut = 24,
};

}

#pragma once

#include "container/string.h"

namespace Opal
{

using StringEx = StringUtf8;

struct Exception
{
    Exception(const StringEx& message) : m_message(message) {}
    Exception(StringEx&& message) : m_message(Move(message)) {}
    virtual ~Exception() = default;

    const StringEx& What() const { return m_message; }

private:
    StringEx m_message;
};

struct PathNotFoundException : Exception
{
    PathNotFoundException(const char* path) : Exception(StringEx("Path not found: ") + path) {}
};

struct PathAlreadyExistsException : Exception
{
    PathAlreadyExistsException(const char* path) : Exception(StringEx("Something already exists on path: ") + path) {}
};

struct DirectoryNotEmptyException : Exception
{
    DirectoryNotEmptyException(const char* path) : Exception(StringEx("Directory not empty: ") + path) {}
};

struct NotImplementedException : Exception
{
    NotImplementedException(const char* function) : Exception(StringEx("Function not implemented: ") + function) {}
};

}  // namespace Opal
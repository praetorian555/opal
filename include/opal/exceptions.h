#pragma once

#include <stdio.h>

#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

class StringEx
{
public:
    StringEx(const char* str)
    {
        while (*str != '\0' && m_length < k_capacity - 1)
        {
            m_data[m_length++] = *str++;
        }
    }

    StringEx& operator+(const char* str)
    {
        while (*str != '\0' && m_length < k_capacity - 1)
        {
            m_data[m_length++] = *str++;
        }
        return *this;
    }

    StringEx& operator+(i64 nb)
    {
        constexpr i64 k_buffer_size = 64;
        char buffer[k_buffer_size] = {};
        snprintf(buffer, k_buffer_size - 1, "%d", static_cast<i32>(nb));
        return *this + buffer;
    }

    StringEx& operator+(u64 nb)
    {
        constexpr i64 k_buffer_size = 64;
        char buffer[k_buffer_size] = {};
        snprintf(buffer, k_buffer_size - 1, "%u", static_cast<u32>(nb));
        return *this + buffer;
    }


    const char* operator*() const { return m_data; }

private:
    static constexpr i64 k_capacity = 1024;
    char m_data[k_capacity] = {};
    i64 m_length = 0;
};

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

struct NotDirectoryException : Exception
{
    NotDirectoryException(const char* path) : Exception(StringEx("Path ") + path + " does not point to a directory") {}
};

struct NotImplementedException : Exception
{
    NotImplementedException(const char* function) : Exception(StringEx("Function not implemented: ") + function) {}
};

struct InvalidArgumentException : Exception
{
    InvalidArgumentException(const char* function, const char* argument, i64 value)
        : Exception(StringEx("Invalid argument ") + argument + " in function " + function + " with value " + value)
    {
    }

    InvalidArgumentException(const char* function, const char* argument, u64 value)
        : Exception(StringEx("Invalid argument ") + argument + " in function " + function + " with value " + static_cast<i64>(value))
    {
    }
};

struct OutOfMemoryException : Exception
{
    OutOfMemoryException(const char* allocator_name, i64 size)
        : Exception(StringEx("Out of memory in allocator ") + allocator_name + " trying to allocate " + size + " bytes.")
    {
    }

    OutOfMemoryException(const char* allocator_name, u64 size)
        : Exception(StringEx("Out of memory in allocator ") + allocator_name + " trying to allocate " + static_cast<i64>(size) + " bytes.")
    {
    }

    OutOfMemoryException(const char* custom_message) : Exception(StringEx("Out of memory: ") + custom_message) {}
};

struct OutOfBoundsException : Exception
{
    OutOfBoundsException(u64 value, u64 lower_bound, u64 upper_bound)
        : Exception(StringEx("Out of bounds with value ") + value + " and range [" + lower_bound + ", " + upper_bound + "]")
    {
    }

    OutOfBoundsException(i64 value, i64 lower_bound, i64 upper_bound)
        : Exception(StringEx("Out of bounds with value ") + value + " and range [" + lower_bound + ", " + upper_bound + "]")
    {
    }

    OutOfBoundsException(const char* custom_message) : Exception(StringEx("Out of bounds: ") + custom_message) {}
};

}  // namespace Opal
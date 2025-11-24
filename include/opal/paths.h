#pragma once

#include "opal/container/string.h"

namespace Opal::Paths
{

/**
 * @brief Get current working directory.
 * @return Returns path to the current working directory. Uses default allocator.
 * @throw OutOfMemoryException when memory allocation is needed in the scratch allocator but there is not enough space.
 * @throw Exception when there is an issue getting the value from the OS.
 */
StringUtf8 OPAL_EXPORT GetCurrentWorkingDirectory();

/**
 * @brief Set current working directory.
 * @note Not thread-safe.
 * @param path Path to the new working directory. This must be an existing directory.
* @throw OutOfMemoryException when memory allocation is needed in the scratch allocator but there is not enough space.
 * @throw Exception when there is an issue setting the value from the OS.
 */
void OPAL_EXPORT SetCurrentWorkingDirectory(const StringUtf8& path);

/**
 * @brief Normalize the path. This will remove redundant separators, switch separators with preferred separators, resolve relative paths,
 * remove trailing separators, resolve symlinks (..), etc.
 * @param path Path to normalize.
 * @note Uses default allocator for output path.
 * @note Uses scratch allocator for temporary allocations.
 * @throw OutOfMemoryException If either the default or scratch allocators run out of memory.
 * @return Returns normalized path in case of a success.
 */
StringUtf8 OPAL_EXPORT NormalizePath(const StringUtf8& path);

/**
 * @brief Check if the path is absolute.
 * @param path Path to check.
 * @return True if the path is absolute, otherwise false.
 */
bool OPAL_EXPORT IsPathAbsolute(const StringUtf8& path);

/**
 * @brief Get name of the file with the extension from the path.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param path Path to the file.
 * @return Name of the file with the extension in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for the
 * result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetFileName(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Get name of the file without the extension from the path.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param path Path to the file.
 * @return Name of the file without the extension in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for
 * the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetStem(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Get extension of the file from the path.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param path Path to the file.
 * @return Extension of the file in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetExtension(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Get parent path of the file from the path.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param path Path to the file.
 * @return Parent path of the file in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetParentPath(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Combine paths.
 * @tparam Args Types of path components. It needs to be types compatible with StringUtf8.
 * @param args Path components.
 * @throw OutOfMemoryException when there is not enough memory for appending the path.
 * @return Returns combined path.
 */
template <typename... Args>
StringUtf8 Combine(Args... args);

}  // namespace Opal::Paths

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wunused-value")
template <typename... Args>
Opal::StringUtf8 Opal::Paths::Combine(Args... args)
{
    StringUtf8 result(GetDefaultAllocator());

    OPAL_DISABLE_WARNING("-Wunused-but-set-variable")
    auto append = [&result](const auto& part)
    {
        if (result.IsEmpty())
        {
            result.Append(part);
        }
        else
        {
            if (result.Back().GetValue() != '/' || result.Back().GetValue() != '\\')
            {
#if defined(OPAL_PLATFORM_WINDOWS)
                constexpr StringUtf8::value_type k_separator = '\\';
#elif defined(OPAL_PLATFORM_LINUX)
                constexpr StringUtf8::value_type k_separator = '/';
#else
                constexpr StringUtf8::value_type k_separator = '/';
#endif
                result.Append(k_separator);
            }
            result.Append(part);
        }
    };

    (append(args), ...);
    return result;
}
OPAL_END_DISABLE_WARNINGS
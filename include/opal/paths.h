#pragma once

#include "opal/container/string.h"

namespace Opal::Paths
{

/**
 * @brief Get current working directory.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @return Returns current working directory in case of a success. Returns ErrorCode::OSFailure in case that it can't get the current
 * working directory. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetCurrentWorkingDirectory(AllocatorBase* allocator = nullptr);

/**
 * @brief Set current working directory.
 * @note Not thread-safe.
 * @param path Path to the new working directory. This must be an existing directory.
 * @param allocator Allocator to use for allocating the temporary function data. If nullptr, the default allocator will be used.
 * @return ErrorCode::Success if the operation was successful, otherwise ErrorCode::OSFailure.
 */
ErrorCode OPAL_EXPORT SetCurrentWorkingDirectory(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Normalize the path. This will remove redundant separators, switch separators with preferred separators, resolve relative paths,
 * remove trailing separators, resolve symlinks (..), etc.
 * @param path Path to normalize.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @return Returns normalized path in case of a success. Returns ErrorCode::OSFailure in case that it can't get the current working directory.
 * Returns ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT NormalizePath(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Check if the path is absolute.
 * @param path Path to check.
 * @return True if the path is absolute, otherwise false.
 */
bool IsPathAbsolute(const StringUtf8& path);

/**
 * @brief Get name of the file with the extension from the path.
 * @param path Path to the file.
 * @return Name of the file with the extension.
 */
static StringUtf8 GetFileName(const StringUtf8& path);

/**
 * @brief Get name of the file without the extension from the path.
 * @param path Path to the file.
 * @return Name of the file without the extension.
 */
static StringUtf8 GetStem(const StringUtf8& path);

/**
 * @brief Get extension of the file from the path.
 * @param path Path to the file.
 * @return Extension of the file.
 */
static StringUtf8 GetExtension(const StringUtf8& path);

/**
 * @brief Get parent path of the file from the path.
 * @param path Path to the file.
 * @return Parent path of the file.
 */
static StringUtf8 GetParentPath(const StringUtf8& path);

}  // namespace Opal::Paths

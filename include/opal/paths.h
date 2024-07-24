#pragma once

#include "opal/container/string.h"

namespace Opal::Paths
{

Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetCurrentWorkingDirectory();

/**
 * @brief Set current working directory.
 * @note Not thread-safe.
 * @param path Path to the new working directory. This must be an existing directory.
 * @return ErrorCode::Success if the operation was successful, otherwise ErrorCode::OSFailure.
 */
ErrorCode OPAL_EXPORT SetCurrentWorkingDirectory(const StringUtf8& path);

/**
 * @brief Normalize the path. This will remove redundant separators, switch separators with preferred separators, resolve relative paths,
 * and remove trailing separators.
 * @param path Path to normalize.
 * @param allocator Allocator to use for temporary memory allocation.
 * @return Normalized path.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT NormalizePath(const StringUtf8& path, AllocatorBase* allocator = nullptr);

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

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
 * @return Returns normalized path in case of a success. Returns ErrorCode::OSFailure in case that it can't get the current working
 * directory. Returns ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
Expected<StringUtf8, ErrorCode> OPAL_EXPORT NormalizePath(const StringUtf8& path, AllocatorBase* allocator = nullptr);

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
 * @brief Check if the path exists.
 * @param path Path to check.
 * @param allocator Allocator to use for allocating the temporary function data. If nullptr, the default allocator will be used.
 * @return True if the path exists, otherwise false, including the errors.
 */
bool OPAL_EXPORT Exists(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Combine paths.
 * @tparam Args Types of path components. It needs to be types compatible with StringUtf8.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param args Path components.
 * @return Combined path in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
template <typename ...Args>
Expected<StringUtf8, ErrorCode> Combine(AllocatorBase* allocator, Args... args);

}  // namespace Opal::Paths

template <typename ...Args>
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::Combine(AllocatorBase* allocator, Args... args)
{
    StringUtf8 result(allocator);
    ErrorCode err = ErrorCode::Success;

    auto append = [&result, &err](const auto& part)
    {
        constexpr StringUtf8::CodeUnitType k_separator = '\\';
        if (result.IsEmpty())
        {
            err = result.Append(part);
            if (err != ErrorCode::Success)
            {
                return err;
            }
        }
        else
        {
            if (result.Back().GetValue() != '/' || result.Back().GetValue() != '\\')
            {
                err = result.Append(k_separator);
                if (err != ErrorCode::Success)
                {
                    return err;
                }
            }

            err = result.Append(part);
            if (err != ErrorCode::Success)
            {
                return err;
            }
        }
        return ErrorCode::Success;
    };

    (... && (append(args) == ErrorCode::Success));
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
}

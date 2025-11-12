#pragma once

#include "opal/container/string.h"

namespace Opal::Paths
{

/**
 * @brief Get current working directory.
 * @param out_path String to store the current working directory.
 * @return Returns ErrorCode::Success if the current working directory is obtained. Returns ErrorCode::OSFailure in case that it can't get
 * the current working directory. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
ErrorCode OPAL_EXPORT GetCurrentWorkingDirectory(StringUtf8& out_path);

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
 * @brief Check if specified path is a directory.
 * @param path Path to check.
 * @param allocator Allocator to use for internal allocations if needed.
 * @return Returns true if path is a directory, false if path does not exist or its not a directory.
 */
bool OPAL_EXPORT IsDirectory(const StringUtf8& path, AllocatorBase* allocator = nullptr);

/**
 * @brief Check if specified path is a file.
 * @param path Path to check.
 * @param allocator Allocator to use for internal allocations if needed.
 * @return Returns true if path is a file, false if path does not exist or its not a directory.
 */
bool OPAL_EXPORT IsFile(const StringUtf8& path, AllocatorBase* allocator = nullptr);

struct DirectoryContentsDesc
{
    bool include_directories = true;
    bool recursive = false;
};

/**
 * @brief Collect paths to directories or files that are inside the specified directory.
 * @param path Path to the directory.
 * @param out_contents Output array that will store found contents.
 * @param desc Descriptor used to configure the search.
 * @param allocator Used for internal allocations.
 * @return Returns true if there were no problems. Returns false if the path is not to the directory or
 * if directory does not exist.
 */
bool OPAL_EXPORT CollectDirectoryContents(const StringUtf8& path, DynamicArray<StringUtf8>& out_contents,
                                          const DirectoryContentsDesc& desc = {}, AllocatorBase* allocator = nullptr);

/**
 * @brief Combine paths.
 * @tparam Args Types of path components. It needs to be types compatible with StringUtf8.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param args Path components.
 * @return Combined path in case of a success. ErrorCode::OutOfMemory in case that it can't allocate memory for the result.
 */
template <typename... Args>
Expected<StringUtf8, ErrorCode> Combine(AllocatorBase* allocator, Args... args);

}  // namespace Opal::Paths

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wunused - value)
template <typename... Args>
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::Combine(AllocatorBase* allocator, Args... args)
{
    StringUtf8 result(allocator);
    ErrorCode err = ErrorCode::Success;

    auto append = [&result, &err](const auto& part)
    {
        constexpr StringUtf8::value_type k_separator = '\\';
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
OPAL_END_DISABLE_WARNINGS
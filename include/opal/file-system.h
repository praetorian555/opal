#pragma once

#include "opal/export.h"
#include "opal/container/string.h"

namespace Opal
{

/**
 * @brief Creates a file.
 * @param path Path to file to create.
 * @param override If true will override existing file on disk.
 * @param allocator Scratch allocator.
 * @return Returns ErrorCode::Success in case of a success, or ErrorCode::OSFailure if there was OS issue with creating a file. Other errors
 * are possible if transcoding of the path is necessary.
 */
ErrorCode OPAL_EXPORT CreateFile(const StringUtf8& path, bool override = false, AllocatorBase* allocator = nullptr);

/**
 * @brief Deletes a file.
 * @param path Path to the file to delete.
 * @param allocator Scratch allocator.
* @return Returns ErrorCode::Success in case of a success, or ErrorCode::OSFailure if there was an issue with deleting a file. Other errors
 * are possible if transcoding of the path is necessary.
 */
ErrorCode OPAL_EXPORT DeleteFile(const StringUtf8& path, AllocatorBase* allocator = nullptr);

ErrorCode OPAL_EXPORT CreateDirectory(const StringUtf8& path, AllocatorBase* allocator = nullptr);
ErrorCode OPAL_EXPORT DeleteDirectory(const StringUtf8& path, AllocatorBase* allocator = nullptr);

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

}  // namespace Opal
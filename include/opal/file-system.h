#pragma once

#include "opal/export.h"
#include "opal/container/string.h"

namespace Opal
{

/**
 * @brief Creates a file.
 * @param path Path to file to create.
 * @param fail_if_already_exists If true when something already exists at the path don't throw an exception. Default value is false.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @throw PathNotFoundException when path does not exist.
 * @throw PathAlreadyExistsException when something already exists on the given path.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT CreateFile(const StringUtf8& path, bool fail_if_already_exists = false, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Deletes a file.
 * @param path Path to the file to delete.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @throw PathNotFoundException when path does not exist.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT DeleteFile(const StringUtf8& path, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Creates a directory at a specified path.
 * @param path Path on which to create a directory.
 * @param throw_if_exists If true throw exception if something already exists. Default is false.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @throw PathNotFoundException when path does not exist.
 * @throw PathAlreadyExistsException when something already exists on the given path.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT CreateDirectory(const StringUtf8& path, bool throw_if_exists = false, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Deletes a directory.
 * @param path Path to the directory.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @throw PathNotFoundException when path does not exist.
 * @throw DirectoryNotEmptyException when directory is not empty.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT DeleteDirectory(const StringUtf8& path, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Check if the path exists.
 * @param path Path to check.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @return True if the path exists, otherwise false, including the errors.
 */
bool OPAL_EXPORT Exists(const StringUtf8& path, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Check if specified path is a directory.
 * @param path Path to check.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @return Returns true if path is a directory, false if path does not exist or its not a directory.
 */
bool OPAL_EXPORT IsDirectory(const StringUtf8& path, AllocatorBase* scratch_allocator = nullptr);

/**
 * @brief Check if specified path is a file.
 * @param path Path to check.
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @return Returns true if path is a file, false if path does not exist or its not a directory.
 */
bool OPAL_EXPORT IsFile(const StringUtf8& path, AllocatorBase* scratch_allocator = nullptr);

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
 * @param scratch_allocator Scratch allocator. If nullptr, default allocator will be used. Default is nullptr.
 * @return Returns true if there were no problems. Returns false if the path is not to the directory or
 * if directory does not exist.
 */
bool OPAL_EXPORT CollectDirectoryContents(const StringUtf8& path, DynamicArray<StringUtf8>& out_contents,
                                          const DirectoryContentsDesc& desc = {}, AllocatorBase* scratch_allocator = nullptr);

}  // namespace Opal
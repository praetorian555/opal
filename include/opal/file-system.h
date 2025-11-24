#pragma once

#include "opal/container/string.h"
#include "opal/export.h"

namespace Opal
{

/**
 * @brief Creates a file.
 * @param path Path to file to create.
 * @param fail_if_already_exists If true when something already exists at the path don't throw an exception. Default value is false.
 * @note Might use current scratch allocator on some platforms.
 * @throw PathNotFoundException when path does not exist.
 * @throw PathAlreadyExistsException when something already exists on the given path.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT CreateFile(const StringUtf8& path, bool fail_if_already_exists = false);

/**
 * @brief Deletes a file.
 * @param path Path to the file to delete.
 * @note Might use current scratch allocator on some platforms.
 * @throw PathNotFoundException when path does not exist.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT DeleteFile(const StringUtf8& path);

/**
 * @brief Creates a directory at a specified path.
 * @param path Path on which to create a directory.
 * @param throw_if_exists If true throw exception if something already exists. Default is false.
 * @note Might use current scratch allocator on some platforms.
 * @throw PathNotFoundException when path does not exist.
 * @throw PathAlreadyExistsException when something already exists on the given path.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT CreateDirectory(const StringUtf8& path, bool throw_if_exists = false);

/**
 * @brief Deletes a directory.
 * @param path Path to the directory.
 * @note Might use current scratch allocator on some platforms.
 * @throw PathNotFoundException when path does not exist.
 * @throw DirectoryNotEmptyException when directory is not empty.
 * @throw Exception when any other error occurs.
 */
void OPAL_EXPORT DeleteDirectory(const StringUtf8& path);

/**
 * @brief Check if the path exists.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return True if the path exists, otherwise false, including the errors.
 */
bool OPAL_EXPORT Exists(const StringUtf8& path);

/**
 * @brief Check if specified path is a directory.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return Returns true if path is a directory, false if path does not exist or its not a directory.
 */
bool OPAL_EXPORT IsDirectory(const StringUtf8& path);

/**
 * @brief Check if specified path is a file.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return Returns true if path is a file, false if path does not exist or it's not a directory.
 */
bool OPAL_EXPORT IsFile(const StringUtf8& path);

struct DirectoryContentsDesc
{
    bool include_directories = true;
    bool recursive = false;
};

struct DirectoryEntry
{
    StringUtf8 path;
    bool is_directory = false;
};

/**
 * @brief Collect paths to directories or files that are inside the specified directory.
 * @param path Path to the directory.
 * @param desc Descriptor used to configure the search.
 * @note Will use current default allocator for allocation of the output entries.
 * @note Will use current scratch allocator for internal allocations.
 * @return Returns array of DirectoryEntry objects.
 * @throw PathNotFoundException when path does not exist.
 * @throw NotDirectoryException if a given path is not to a directory.
 */
DynamicArray<DirectoryEntry> OPAL_EXPORT CollectDirectoryContents(const StringUtf8& path, const DirectoryContentsDesc& desc = {});

}  // namespace Opal
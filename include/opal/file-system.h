#pragma once

#include "opal/container/expected.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"

// A function here that takes a path reports one longer than the platform accepts as ErrorCode::InvalidArgument, or false for the
// ones that return bool.

namespace Opal
{

/**
 * @brief Creates a file.
 * @param path Path to file to create.
 * @param fail_if_already_exists If false, a path that is already taken is reported as success. Default value is false.
 * @note Might use current scratch allocator on some platforms.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist,
 *         ErrorCode::AlreadyExists when something is already on the path and fail_if_already_exists is set, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode CreateFile(StringViewUtf8 path, bool fail_if_already_exists = false);

/**
 * @brief Deletes a file.
 * @param path Path to the file to delete.
 * @note Might use current scratch allocator on some platforms.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the path does not exist, or ErrorCode::OSFailure for any
 *         other failure.
 */
[[nodiscard]] ErrorCode DeleteFile(StringViewUtf8 path);

/**
 * @brief Creates a directory at a specified path.
 * @param path Path on which to create a directory.
 * @param fail_if_already_exists If false, a path that is already taken is reported as success. Default value is false.
 * @note Might use current scratch allocator on some platforms.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist,
 *         ErrorCode::AlreadyExists when something is already on the path and fail_if_already_exists is set, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode CreateDirectory(StringViewUtf8 path, bool fail_if_already_exists = false);

/**
 * @brief Deletes a directory.
 * @param path Path to the directory.
 * @note Might use current scratch allocator on some platforms.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the path does not exist, ErrorCode::NotEmpty when the
 *         directory still has contents, or ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode DeleteDirectory(StringViewUtf8 path);

/**
 * @brief Check if the path exists.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return True if the path exists, otherwise false, including the errors.
 */
bool Exists(StringViewUtf8 path);

/**
 * @brief Check if specified path is a directory.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return Returns true if path is a directory, false if path does not exist or its not a directory.
 */
bool IsDirectory(StringViewUtf8 path);

/**
 * @brief Check if specified path is a file.
 * @param path Path to check.
 * @note Might use current scratch allocator on some platforms.
 * @return Returns true if path is a file, false if path does not exist or it's not a directory.
 */
bool IsFile(StringViewUtf8 path);

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
 * @return Array of DirectoryEntry objects, ErrorCode::PathNotFound when the path does not exist,
 *         ErrorCode::NotDirectory when the path is not a directory, ErrorCode::OutOfMemory when the result could not
 *         be grown, or ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] Expected<DynamicArray<DirectoryEntry>, ErrorCode> CollectDirectoryContents(
    StringUtf8 path, const DirectoryContentsDesc& desc = {});

/**
 * @brief Read the entire contents of a file as a UTF-8 string.
 * @param path Path to the file to read.
 * @return String containing the file contents, ErrorCode::PathNotFound when the file does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] Expected<StringUtf8, ErrorCode> ReadFileAsString(StringViewUtf8 path);

/**
 * @brief Read the entire contents of a file as a byte array.
 * @param path Path to the file to read.
 * @return Array of bytes containing the file contents, ErrorCode::PathNotFound when the file does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] Expected<DynamicArray<u8>, ErrorCode> ReadFileAsBytes(StringViewUtf8 path);

/**
 * @brief Write a string to a file, replacing any existing content. Creates the file if it does not exist.
 * @param path Path to the file to write.
 * @param content String content to write.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode WriteStringToFile(StringViewUtf8 path, StringViewUtf8 content);

/**
 * @brief Write bytes to a file, replacing any existing content. Creates the file if it does not exist.
 * @param path Path to the file to write.
 * @param content Byte data to write.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode WriteBytesToFile(StringViewUtf8 path, ArrayView<const u8> content);

/**
 * @brief Append a string to a file. Creates the file if it does not exist.
 * @param path Path to the file to append to.
 * @param content String content to append.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode AppendStringToFile(StringViewUtf8 path, StringViewUtf8 content);

/**
 * @brief Append bytes to a file. Creates the file if it does not exist.
 * @param path Path to the file to append to.
 * @param content Byte data to append.
 * @return ErrorCode::Success, ErrorCode::PathNotFound when the parent directory does not exist, or
 *         ErrorCode::OSFailure for any other failure.
 */
[[nodiscard]] ErrorCode AppendBytesToFile(StringViewUtf8 path, ArrayView<const u8> content);

}  // namespace Opal
#pragma once

#include "opal/container/string-view.h"
#include "opal/container/string.h"

namespace Opal::Paths
{

/**
 * @brief Get current working directory.
 * @note Uses default allocator for the output path.
 * @return Path to the current working directory, ErrorCode::OutOfMemory when the result could not be grown, or
 *         ErrorCode::OSFailure when the OS would not report it.
 */
[[nodiscard]] Expected<StringUtf8, ErrorCode> OPAL_EXPORT GetCurrentWorkingDirectory();

/**
 * @brief Set current working directory.
 * @note Not thread-safe.
 * @param path Path to the new working directory. This must be an existing directory.
 * @return ErrorCode::Success, ErrorCode::OutOfMemory when normalizing the path could not allocate, or
 *         ErrorCode::OSFailure when the OS would not accept it.
 */
[[nodiscard]] ErrorCode OPAL_EXPORT SetCurrentWorkingDirectory(const StringUtf8& path);

/**
 * @brief Normalize the path. This will remove redundant separators, switch separators with preferred separators, resolve relative paths,
 * remove trailing separators, resolve symlinks (..), etc.
 * @param path Path to normalize.
 * @note Uses default allocator for output path.
 * @note Uses scratch allocator for temporary allocations.
 * @return Normalized path, ErrorCode::OutOfMemory when it could not be built, or ErrorCode::OSFailure when a relative
 *         path needed the working directory and the OS would not report it.
 */
[[nodiscard]] Expected<StringUtf8, ErrorCode> OPAL_EXPORT NormalizePath(const StringUtf8& path);

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
 * @brief Combine paths, inserting a separator between components that do not already have one.
 * @tparam Args Types of path components. It needs to be types compatible with StringUtf8.
 * @param args Path components.
 * @return Combined path, or ErrorCode::OutOfMemory when it could not be grown.
 */
template <typename... Args>
    requires(Opal::Constructible<Args, Opal::StringViewUtf8> && ...)
[[nodiscard]] Expected<StringUtf8, ErrorCode> Combine(Args&&... args);

}  // namespace Opal::Paths

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wunused-value")
template <typename... Args>
    requires(Opal::Constructible<Args, Opal::StringViewUtf8> && ...)
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::Combine(Args&&... args)
{
    StringUtf8 result;
    ErrorCode error = ErrorCode::Success;

    // Combine() with no components never calls the lambda.
    OPAL_DISABLE_WARNING("-Wunused-but-set-variable")
    auto append = [&result, &error](const auto& part)
    {
        if (error != ErrorCode::Success) [[unlikely]]
        {
            return;
        }
        StringViewUtf8 part_view(part);
        if (result.IsEmpty())
        {
            error = result.Append(part_view.GetData(), part_view.GetSize());
            return;
        }
        if (result.Back().GetValue() != '/' && result.Back().GetValue() != '\\')
        {
#if defined(OPAL_PLATFORM_WINDOWS)
            constexpr StringUtf8::value_type k_separator = '\\';
#elif defined(OPAL_PLATFORM_LINUX)
            constexpr StringUtf8::value_type k_separator = '/';
#else
            constexpr StringUtf8::value_type k_separator = '/';
#endif
            error = result.Append(k_separator);
            if (error != ErrorCode::Success) [[unlikely]]
            {
                return;
            }
        }
        error = result.Append(part_view.GetData(), part_view.GetSize());
    };

    (append(args), ...);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Expected<StringUtf8, ErrorCode>(error);
    }
    return Expected<StringUtf8, ErrorCode>(Move(result));
}
OPAL_END_DISABLE_WARNINGS

#include "opal/paths.h"

#include "../third-party/catch2/include/catch2/catch2.hpp"
#include "opal/defines.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <limits.h>
#include <unistd.h>
#endif

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetCurrentWorkingDirectory()
{
    using Result = Expected<StringUtf8, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD size_needed = GetCurrentDirectoryW(0, nullptr);
    if (size_needed == 0)
    {
        return Result(ErrorCode::OSFailure);
    }
    // The sized String constructors throw on a failed allocation, so the buffers are grown through Resize instead.
    StringWide buffer;
    ErrorCode err = buffer.Resize(static_cast<StringWide::size_type>(size_needed - 1), L'\0');
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    const DWORD written_size = GetCurrentDirectoryW(size_needed, buffer.GetData());
    if (written_size == 0)
    {
        return Result(ErrorCode::OSFailure);
    }
    StringUtf8 out_path;
    err = out_path.Resize(MAX_PATH, '\0');
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    err = Transcode(buffer, out_path);
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    return Result(Move(out_path));
#elif defined(OPAL_PLATFORM_LINUX)
    StringUtf8 out_path(GetDefaultAllocator());
    const ErrorCode err = out_path.Resize(PATH_MAX, '\0');
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    if (getcwd(out_path.GetData(), out_path.GetSize()) == nullptr)
    {
        return Result(ErrorCode::OSFailure);
    }
    out_path.Trim();
    return Result(Move(out_path));
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::Paths::SetCurrentWorkingDirectory(const StringUtf8& path)
{
    Expected<StringUtf8, ErrorCode> normalized_path = NormalizePath(path);
    if (!normalized_path.HasValue())
    {
        return normalized_path.GetError();
    }

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    ErrorCode err = path_wide.Resize(MAX_PATH, 0);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    err = Transcode(normalized_path.GetValue(), path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const BOOL result = SetCurrentDirectoryW(*path_wide);
    if (result == 0)
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#elif defined(OPAL_PLATFORM_LINUX)
    if (chdir(*normalized_path.GetValue()) == -1)
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::NormalizePath(const StringUtf8& path)
{
    using Result = Expected<StringUtf8, ErrorCode>;
    // Records the first failed append; every later one is skipped and the code is returned at the next checkpoint.
    ErrorCode append_error = ErrorCode::Success;
    auto Append = [&append_error](StringUtf8& target, const auto& value)
    {
        if (append_error == ErrorCode::Success) [[likely]]
        {
            append_error = target.Append(value);
        }
    };

    Opal::AllocatorBase* default_allocator = GetDefaultAllocator();
#if defined(OPAL_PLATFORM_WINDOWS)
    constexpr StringUtf8::value_type k_preferred_separator = '\\';
#elif defined(OPAL_PLATFORM_LINUX)
    constexpr StringUtf8::value_type k_preferred_separator = '/';
#else
#error "Platform not supported"
#endif

    if (path.IsEmpty())
    {
        return Result(StringUtf8());
    }

    // Clone throws on a failed allocation, so the copy is made through Append instead.
    StringUtf8 original_path;
    Append(original_path, path);
    if (append_error != ErrorCode::Success) [[unlikely]]
    {
        return Result(append_error);
    }

    // If path is not absolute we need to make it absolute
    if (!IsPathAbsolute(path))
    {
        // If the path is relative, attach the current working directory to it before normalization
        Expected<StringUtf8, ErrorCode> current_working_directory = GetCurrentWorkingDirectory();
        if (!current_working_directory.HasValue())
        {
            return Result(current_working_directory.GetError());
        }
        original_path.Erase();
        Append(original_path, current_working_directory.GetValue());
        Append(original_path, k_preferred_separator);
        Append(original_path, path);
        if (append_error != ErrorCode::Success) [[unlikely]]
        {
            return Result(append_error);
        }
    }

    bool prev_is_separator = false;
    StringUtf8::size_type start = 0;
    StringUtf8 root(default_allocator);

#if defined(OPAL_PLATFORM_WINDOWS)
    // Figure out what is the root of the path and where does the relative part starts if the path starts with disk name and colon
    if (original_path.GetSize() >= 2 && original_path[1] == ':')
    {
        // Path is absolute and starts with a drive letter
        Append(root, original_path[0]);
        Append(root, original_path[1]);
        Append(root, k_preferred_separator);
        prev_is_separator = true;
        start = 2;
    }
#endif

#if defined(OPAL_PLATFORM_LINUX) || defined(OPAL_PLATFORM_WINDOWS)
    // Figure out what is the root of the path and where does the relative part starts if the path starts with a separator
    if (original_path.GetSize() >= 1 && (original_path[0] == '\\' || original_path[0] == '/'))
    {
        // Path is absolute but starts only with a separator
        Append(root, k_preferred_separator);
        prev_is_separator = true;
        start = 1;
    }
#endif
    if (append_error != ErrorCode::Success) [[unlikely]]
    {
        return Result(append_error);
    }

    StringUtf8 relative;
    // Remove redundant separators and switch to using preferred separators
    for (StringUtf8::size_type i = start; i < original_path.GetSize(); ++i)
    {
        if (original_path[i] == '\\' || original_path[i] == '/')
        {
            if (prev_is_separator)
            {
                continue;
            }

            prev_is_separator = true;
            Append(relative, k_preferred_separator);
        }
        else
        {
            prev_is_separator = false;
            Append(relative, original_path[i]);
        }
    }

    StringUtf8 pattern;
    Append(pattern, k_preferred_separator);
    Append(pattern, '.');
    Append(pattern, k_preferred_separator);
    if (append_error != ErrorCode::Success) [[unlikely]]
    {
        return Result(append_error);
    }
    const StringUtf8::size_type pos = Find(relative, pattern);
    if (pos != StringUtf8::k_npos)
    {
        // Remove ./
        if (pos + 3 < relative.GetSize())
        {
            relative.Erase(pos + 1, 2);
        }
        else
        {
            relative.Erase(pos);
        }
    }
    if (relative.GetSize() > 1 && relative[0] == '.' && relative[1] == k_preferred_separator)
    {
        relative.Erase(0, 2);
    }
    if (relative.GetSize() > 1 && *(relative.End() - 1) == '.' && *(relative.End() - 2) == k_preferred_separator)
    {
        relative.Erase(relative.GetSize() - 2, 2);
    }

    // If there is a separator at the end of the path, remove it
    if (!relative.IsEmpty() && relative.Back().GetValue() == k_preferred_separator)
    {
        relative.Erase(relative.End() - 1);
    }

    // Points to either separator, the first element in the relative path or one after last element in the relative path
    StringUtf8::size_type end = relative.GetSize();
    StringUtf8::size_type erase_end = 0;

    i32 skip_count = 0;

    while (end != 0)
    {
        // component_start either points to the separator or to the first element in the relative path
        StringUtf8::size_type component_start = ReverseFind(relative, k_preferred_separator, end - 1);
        if (component_start == StringUtf8::k_npos)
        {
            component_start = 0;
        }
        const bool is_not_first_char = component_start != 0;

        // Here we want to compare from the first char after the separator to the end of the component, which is either
        // the last element before next separator or the last element in the relative path
        const StringUtf8::size_type compare_start = is_not_first_char ? component_start + 1 : 0;
        const StringUtf8::size_type compare_count = is_not_first_char ? end - component_start - 1 : end;
        auto compare_result = Compare(relative, compare_start, compare_count, "..");
        OPAL_ASSERT(compare_result.HasValue(), "This must always succeed");
        if (compare_result.GetValue() == 0)
        {
            ++skip_count;
            if (skip_count == 1)
            {
                erase_end = end;
            }
        }
        else
        {
            if (skip_count == 1)
            {
                const StringUtf8::size_type erase_count = is_not_first_char ? erase_end - component_start : erase_end + 1;
                relative.Erase(component_start, erase_count);
            }
            if (skip_count != 0)
            {
                --skip_count;
            }
        }
        end = component_start;
    }
    if (skip_count > 0)
    {
        const StringUtf8::size_type erase_count = erase_end + 1;
        relative.Erase(0, erase_count);
    }
    Append(root, relative);
    if (append_error != ErrorCode::Success) [[unlikely]]
    {
        return Result(append_error);
    }
    return Result(Move(root));
}

bool Opal::Paths::IsPathAbsolute(const StringUtf8& path)
{
    if (path.IsEmpty())
    {
        return false;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    if (path.GetSize() >= 2 && path[1] == ':')
    {
        return true;
    }
    if (path.GetSize() >= 1 && (path[0] == '\\' || path[0] == '/'))
    {
        return true;
    }
    return false;
#elif defined(OPAL_PLATFORM_LINUX)
    if (path.GetSize() >= 1 && (path[0] == '\\' || path[0] == '/'))
    {
        return true;
    }
    return false;
#else
#error "Platform not supported"
#endif
}
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetFileName(const StringUtf8& path, AllocatorBase* allocator)
{
    StringUtf8::size_type last_separator = ReverseFind(path, '\\');
    const StringUtf8::size_type other_last_separator = ReverseFind(path, '/');
    if (last_separator == StringUtf8::k_npos)
    {
        last_separator = other_last_separator;
    }
    if (other_last_separator != StringUtf8::k_npos && other_last_separator > last_separator)
    {
        last_separator = other_last_separator;
    }
    if (last_separator == StringUtf8::k_npos)
    {
        return Expected<StringUtf8, ErrorCode>(path.Clone(allocator));
    }
    if (last_separator == path.GetSize() - 1)
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(allocator));
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(path, last_separator + 1, StringUtf8::k_npos);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(std::move(result));
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetStem(const StringUtf8& path, AllocatorBase* allocator)
{
    Expected<StringUtf8, ErrorCode> file_name = GetFileName(path, allocator);
    if (!file_name.HasValue())
    {
        return Expected<StringUtf8, ErrorCode>(file_name.GetError());
    }
    const StringUtf8::size_type last_dot = ReverseFind(file_name.GetValue(), '.');
    if (last_dot == StringUtf8::k_npos || last_dot == 0 ||
        (last_dot == 1 && file_name.GetValue()[0] == '.' && file_name.GetValue().GetSize() == 2))
    {
        return Expected<StringUtf8, ErrorCode>(std::move(file_name.GetValue()));
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(file_name.GetValue(), 0, last_dot);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(std::move(result));
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetExtension(const StringUtf8& path, AllocatorBase* allocator)
{
    Expected<StringUtf8, ErrorCode> file_name = GetFileName(path, allocator);
    if (!file_name.HasValue())
    {
        return Expected<StringUtf8, ErrorCode>(file_name.GetError());
    }
    const StringUtf8::size_type last_dot = ReverseFind(file_name.GetValue(), '.');
    if (last_dot == StringUtf8::k_npos || last_dot == 0 ||
        (last_dot == 1 && file_name.GetValue()[0] == '.' && file_name.GetValue().GetSize() == 2))
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(allocator));
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(file_name.GetValue(), last_dot, StringUtf8::k_npos);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(std::move(result));
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetParentPath(const StringUtf8& path, AllocatorBase* allocator)
{
    StringUtf8::size_type last_separator = ReverseFind(path, '\\');
    const StringUtf8::size_type other_last_separator = ReverseFind(path, '/');
    if (last_separator == StringUtf8::k_npos)
    {
        last_separator = other_last_separator;
    }
    if (other_last_separator != StringUtf8::k_npos && other_last_separator > last_separator)
    {
        last_separator = other_last_separator;
    }
    if (last_separator == StringUtf8::k_npos)
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(allocator));
    }
    if (last_separator == 0)
    {
        return Expected<StringUtf8, ErrorCode>(path.Clone(allocator));
    }
    if (last_separator == 2 && path[1] == ':')
    {
        return Expected<StringUtf8, ErrorCode>(path.Clone(allocator));
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(path, 0, last_separator);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(std::move(result));
}

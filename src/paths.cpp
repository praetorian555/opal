#include "opal/paths.h"

#include "opal/defines.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <unistd.h>
#include <climits>
#endif

Opal::ErrorCode Opal::Paths::GetCurrentWorkingDirectory(Opal::StringUtf8& out_path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD size_needed = GetCurrentDirectoryW(0, nullptr);
    StringWide buffer(static_cast<StringWide::size_type>(size_needed - 1), L'\0', &out_path.GetAllocator());
    if (buffer.GetSize() != size_needed - 1)
    {
        return ErrorCode::OutOfMemory;
    }
    const DWORD written_size = GetCurrentDirectoryW(size_needed, buffer.GetData());
    if (written_size == 0)
    {
        return ErrorCode::OSFailure;
    }
    out_path.Resize(MAX_PATH);
    const ErrorCode err = Transcode(buffer, out_path);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    return ErrorCode::Success;
#elif defined(OPAL_PLATFORM_LINUX)
    const ErrorCode err = out_path.Resize(PATH_MAX);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    if (getcwd(out_path.GetData(), out_path.GetSize()) == nullptr)
    {
        return ErrorCode::OSFailure;
    }
    out_path.Trim();
    return ErrorCode::Success;
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::Paths::SetCurrentWorkingDirectory(const StringUtf8& path, AllocatorBase* allocator)
{
    auto normalized_path = NormalizePath(path, allocator);
    if (!normalized_path.HasValue())
    {
        return normalized_path.GetError();
    }

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(MAX_PATH, 0, allocator);
    if (path_wide.GetSize() != MAX_PATH)
    {
        return ErrorCode::OutOfMemory;
    }
    const ErrorCode err = Transcode(normalized_path.GetValue(), path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const BOOL result = SetCurrentDirectoryW(path_wide.GetData());
    if (result == 0)
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#elif defined(OPAL_PLATFORM_LINUX)
    if (chdir(normalized_path.GetValue().GetData()) == -1)
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#else
#error "Platform not supported"
#endif
}
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::NormalizePath(const StringUtf8& path, AllocatorBase* allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    constexpr StringUtf8::value_type k_preferred_separator = '\\';
#elif defined(OPAL_PLATFORM_LINUX)
    constexpr StringUtf8::value_type k_preferred_separator = '/';
#else
#error "Platform not supported"
#endif

    if (path.IsEmpty())
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(allocator));
    }

    StringUtf8 original_path(path, allocator);

    ErrorCode err = ErrorCode::Success;
    // If path is not absolute we need to make it absolute
    if (!IsPathAbsolute(path))
    {
        // If the path is relative, attach the current working directory to it before normalization
        StringUtf8 current_working_directory(allocator);
        err = GetCurrentWorkingDirectory(current_working_directory);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        original_path.Erase();
        err = original_path.Append(current_working_directory);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        err = original_path.Append(k_preferred_separator);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        err = original_path.Append(path);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
    }

    bool prev_is_separator = false;
    StringUtf8::size_type start = 0;
    StringUtf8 root(allocator);

#if defined(OPAL_PLATFORM_WINDOWS)
    // Figure out what is the root of the path and where does the relative part starts if the path starts with disk name and colon
    if (original_path.GetSize() >= 2 && original_path[1] == ':')
    {
        // Path is absolute and starts with a drive letter
        err = root.Append(original_path[0]);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        err = root.Append(original_path[1]);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        err = root.Append(k_preferred_separator);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        prev_is_separator = true;
        start = 2;
    }
#endif

#if defined(OPAL_PLATFORM_LINUX) || defined(OPAL_PLATFORM_WINDOWS)
    // Figure out what is the root of the path and where does the relative part starts if the path starts with a separator
    if (original_path.GetSize() >= 1 && (original_path[0] == '\\' || original_path[0] == '/'))
    {
        // Path is absolute but starts only with a separator
        err = root.Append(k_preferred_separator);
        if (err != ErrorCode::Success)
        {
            return Expected<StringUtf8, ErrorCode>(err);
        }
        prev_is_separator = true;
        start = 1;
    }
#endif

    StringUtf8 relative(allocator);
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
            err = relative.Append(k_preferred_separator);
            if (err != ErrorCode::Success)
            {
                return Expected<StringUtf8, ErrorCode>(err);
            }
        }
        else
        {
            prev_is_separator = false;
            err = relative.Append(original_path[i]);
            if (err != ErrorCode::Success)
            {
                return Expected<StringUtf8, ErrorCode>(err);
            }
        }
    }

    StringUtf8 pattern(allocator);
    pattern.Append(k_preferred_separator);
    pattern.Append('.');
    pattern.Append(k_preferred_separator);
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
    return Expected<StringUtf8, ErrorCode>(root + relative);
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
        return Expected<StringUtf8, ErrorCode>(StringUtf8(path, allocator));
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
    return Expected<StringUtf8, ErrorCode>(result);
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
        return Expected<StringUtf8, ErrorCode>(file_name.GetValue());
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(file_name.GetValue(), 0, last_dot);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
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
    return Expected<StringUtf8, ErrorCode>(result);
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
        return Expected<StringUtf8, ErrorCode>(StringUtf8(path, allocator));
    }
    if (last_separator == 2 && path[1] == ':')
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(path, allocator));
    }
    StringUtf8 result(allocator);
    const ErrorCode err = result.Append(path, 0, last_separator);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
}

bool Opal::Paths::Exists(const StringUtf8& path, AllocatorBase* allocator)
{
    if (path.IsEmpty())
    {
        return false;
    }
    Expected<StringUtf8, ErrorCode> result = NormalizePath(path, allocator);
    if (!result.HasValue())
    {
        return false;
    }

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(result.GetValue().GetSize() * 2, L'\0', allocator);
    const ErrorCode err = Transcode(result.GetValue(), path_wide);
    if (err != ErrorCode::Success)
    {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    return (attributes != INVALID_FILE_ATTRIBUTES);
#elif defined(OPAL_PLATFORM_LINUX)
    OPAL_ASSERT(result.GetValue().GetData() != nullptr, "Path must not be null");
    if (access(result.GetValue().GetData(), F_OK) == 0)
    {
        return true;
    }
    return false;
#else
#error "Platform not supported"
#endif
}

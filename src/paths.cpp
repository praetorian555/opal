#include "opal/paths.h"

#include <filesystem>
#include "opal/defines.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#else
#include <filesystem>
#endif

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetCurrentWorkingDirectory(AllocatorBase* allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD size_needed = GetCurrentDirectory(0, nullptr);
    StringLocale buffer(size_needed - 1, 0, allocator);
    if (buffer.GetSize() != size_needed - 1)
    {
        return Expected<StringUtf8, ErrorCode>(ErrorCode::OutOfMemory);
    }
    const DWORD written_size = GetCurrentDirectory(size_needed, buffer.GetData());
    if (written_size == 0)
    {
        return Expected<StringUtf8, ErrorCode>(ErrorCode::OSFailure);
    }
    StringUtf8 result(size_needed - 1, 0);
    const ErrorCode err = Transcode(buffer, result);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
#else
    std::error_code std_err;
    const std::filesystem::path std_path = std::filesystem::current_path(std_err);
    if (std_err != std::error_code())
    {
        return Expected<StringUtf8, ErrorCode>(ErrorCode::OSFailure);
    }
    const StringLocale path_locale(std_path.string().c_str(), std_path.string().size(), allocator);
    StringUtf8 result(path_locale.GetSize(), 0, allocator);
    const ErrorCode err = Transcode(path_locale, result);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
#endif
}

Opal::ErrorCode Opal::Paths::SetCurrentWorkingDirectory(const StringUtf8& path, AllocatorBase* allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringLocale path_locale(path.GetSize() * MB_CUR_MAX, 0, allocator);
    const ErrorCode err = Transcode(path, path_locale);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const BOOL result = SetCurrentDirectory(path_locale.GetData());
    if (result == 0)
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#else
    StringLocale path_locale(path.GetSize() * MB_CUR_MAX, 0);
    const ErrorCode err = Transcode(path, path_locale);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const std::filesystem::path std_path(path_locale.GetData());
    std::error_code std_err;
    std::filesystem::current_path(std_path, std_err);
    if (std_err != std::error_code())
    {
        return ErrorCode::OSFailure;
    }
    return ErrorCode::Success;
#endif
}
Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::NormalizePath(const StringUtf8& path, AllocatorBase* allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    constexpr StringUtf8::CodeUnitType k_preferred_separator = '\\';
#else
    constexpr StringUtf8::CodeUnitType k_preferred_separator = '/';
#endif

    if (path.IsEmpty())
    {
        return Expected<StringUtf8, ErrorCode>(StringUtf8(allocator));
    }

    StringUtf8 original_path(path, allocator);
    StringUtf8 root(allocator);
    StringUtf8 relative(allocator);

    ErrorCode err = ErrorCode::Success;
    if (!IsPathAbsolute(path))
    {
        // If the path is relative, attach the current working directory to it before normalization
        auto result = GetCurrentWorkingDirectory();
        if (!result.HasValue())
        {
            return Expected<StringUtf8, ErrorCode>(result.GetError());
        }
        original_path.Erase();
        err = original_path.Append(result.GetValue());
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
    StringUtf8::SizeType start = 0;

#if defined(OPAL_PLATFORM_WINDOWS)
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
#else
#error Not implemented
#endif

    // Remove redundant separators and switch to using preferred separators
    for (StringUtf8::SizeType i = start; i < original_path.GetSize(); ++i)
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
    if (!relative.IsEmpty() && relative.Back().GetValue() == '\\')
    {
        relative.Erase(relative.End() - 1);
    }

    // Points to either separator, the first element in the relative path or one after last element in the relative path
    StringUtf8::SizeType end = relative.GetSize();
    StringUtf8::SizeType erase_end = 0;

    i32 skip_count = 0;

    while (end != 0)
    {
        // component_start either points to the separator or to the first element in the relative path
        StringUtf8::SizeType component_start = ReverseFind(relative, '\\', end - 1);
        if (component_start == StringUtf8::k_npos)
        {
            component_start = 0;
        }
        const bool is_not_first_char = component_start != 0;

        // Here we want to compare from the first char after the separator to the end of the component, which is either
        // the last element before next separator or the last element in the relative path
        const StringUtf8::SizeType compare_start = is_not_first_char ? component_start + 1 : 0;
        const StringUtf8::SizeType compare_count = is_not_first_char ? end - component_start - 1 : end;
        auto compare_result = Compare(relative, compare_start, compare_count, u8"..");
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
                const StringUtf8::SizeType erase_count = is_not_first_char ? erase_end - component_start : erase_end + 1;
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
        const StringUtf8::SizeType erase_count = erase_end + 1;
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
#else
#error "Not implemented"
#endif
}

#include "opal/paths.h"

#include "opal/defines.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#else
#include <filesystem>
#endif

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::Paths::GetCurrentWorkingDirectory()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD size_needed = GetCurrentDirectory(0, nullptr);
    StringLocale buffer(size_needed - 1, 0);
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
    const StringLocale path_locale(std_path.string().c_str(), std_path.string().size());
    StringUtf8 result(path_locale.GetSize(), 0);
    const ErrorCode err = Transcode(path_locale, result);
    if (err != ErrorCode::Success)
    {
        return Expected<StringUtf8, ErrorCode>(err);
    }
    return Expected<StringUtf8, ErrorCode>(result);
#endif
}

Opal::ErrorCode Opal::Paths::SetCurrentWorkingDirectory(const StringUtf8& path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringLocale path_locale(path.GetSize() * MB_CUR_MAX, 0);
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

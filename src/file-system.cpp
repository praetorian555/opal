#include "opal/file-system.h"

#include <stdio.h>
#include <string.h>

#include "opal/exceptions.h"
#include "opal/paths.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#elif defined(OPAL_PLATFORM_LINUX)
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "opal/container/dynamic-array.h"
#include "opal/threading/atomic.h"
#include "platform-path.h"

#if defined(OPAL_PLATFORM_WINDOWS)
using Opal::Impl::ToWidePath;
#elif defined(OPAL_PLATFORM_LINUX)
using Opal::Impl::NativePath;
using Opal::Impl::ToNativePath;
#endif

namespace
{

#if defined(OPAL_PLATFORM_WINDOWS)
// 100-nanosecond intervals between the FILETIME epoch (1601-01-01) and the Unix epoch.
constexpr Opal::i64 k_filetime_epoch_offset = 116'444'736'000'000'000;

Opal::FileStatus ToFileStatus(const FILETIME& last_write, DWORD size_high, DWORD size_low, DWORD attributes)
{
    ULARGE_INTEGER ticks;
    ticks.LowPart = last_write.dwLowDateTime;
    ticks.HighPart = last_write.dwHighDateTime;
    const bool is_directory = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return Opal::FileStatus{.modified = Opal::DateTime{(static_cast<Opal::i64>(ticks.QuadPart) - k_filetime_epoch_offset) * 100},
                            .size = is_directory ? 0 : (static_cast<Opal::u64>(size_high) << 32) | size_low,
                            .is_directory = is_directory};
}
#elif defined(OPAL_PLATFORM_LINUX)
Opal::FileStatus ToFileStatus(const struct stat& info)
{
    const bool is_directory = S_ISDIR(info.st_mode);
    return Opal::FileStatus{
        .modified = Opal::DateTime{static_cast<Opal::i64>(info.st_mtim.tv_sec) * 1'000'000'000 + static_cast<Opal::i64>(info.st_mtim.tv_nsec)},
        .size = is_directory ? 0 : static_cast<Opal::u64>(info.st_size),
        .is_directory = is_directory};
}
#endif

}  // namespace

Opal::ErrorCode Opal::CreateFile(StringViewUtf8 path, bool fail_if_already_exists)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode transcode_err = ToWidePath(path, path_wide);
    if (transcode_err != ErrorCode::Success)
    {
        return transcode_err;
    }

    constexpr DWORD k_access = GENERIC_READ | GENERIC_WRITE;
    constexpr DWORD k_shared_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    constexpr DWORD k_creation_flags = CREATE_NEW;

    HANDLE file_handle =
        CreateFileW(path_wide.GetData(), k_access, k_shared_mode, nullptr, k_creation_flags, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_handle);
        return ErrorCode::Success;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_PATH_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    if (win32_err == ERROR_FILE_EXISTS)
    {
        return fail_if_already_exists ? ErrorCode::AlreadyExists : ErrorCode::Success;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return error;
    }
    const i32 flags = O_CREAT | O_EXCL;
    i32 fd = open(native_path.data, flags, 0644);
    if (fd != -1)
    {
        close(fd);
        return ErrorCode::Success;
    }
    if (errno == EEXIST)
    {
        return fail_if_already_exists ? ErrorCode::AlreadyExists : ErrorCode::Success;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::DeleteFile(StringViewUtf8 path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    if (DeleteFileW(path_wide.GetData()) != 0)
    {
        return ErrorCode::Success;
    }
    const DWORD win32_error = GetLastError();
    if (win32_error == ERROR_FILE_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return error;
    }
    i32 result = remove(native_path.data);
    if (result == 0)
    {
        return ErrorCode::Success;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::CreateDirectory(StringViewUtf8 path, bool fail_if_already_exists)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const BOOL result = CreateDirectoryW(*path_wide, nullptr);
    if (result != 0)
    {
        return ErrorCode::Success;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS)
    {
        return fail_if_already_exists ? ErrorCode::AlreadyExists : ErrorCode::Success;
    }
    if (error == ERROR_PATH_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return error;
    }
    if (mkdir(native_path.data, 0777) == 0)
    {
        return ErrorCode::Success;
    }
    if (errno == EEXIST)
    {
        return fail_if_already_exists ? ErrorCode::AlreadyExists : ErrorCode::Success;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::DeleteDirectory(StringViewUtf8 path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }
    const BOOL status = RemoveDirectoryW(*path_wide);
    if (status != 0)
    {
        return ErrorCode::Success;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_FILE_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    if (win32_err == ERROR_DIR_NOT_EMPTY)
    {
        return ErrorCode::NotEmpty;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return error;
    }
    if (rmdir(native_path.data) == 0)
    {
        return ErrorCode::Success;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    if (errno == ENOTEMPTY)
    {
        return ErrorCode::NotEmpty;
    }
    return ErrorCode::OSFailure;
#else
#error "Platform not supported"
#endif
}

bool Opal::Exists(StringViewUtf8 path)
{
    if (path.IsEmpty())
    {
        return false;
    }
    Expected<StringUtf8, ErrorCode> normalized = Paths::NormalizePath(path);
    if (!normalized.HasValue())
    {
        return false;
    }
    const StringUtf8& result = normalized.GetValue();

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(result, path_wide);
    if (err != ErrorCode::Success)
    {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    return (attributes != INVALID_FILE_ATTRIBUTES);
#elif defined(OPAL_PLATFORM_LINUX)
    if (access(*result, F_OK) == 0)
    {
        return true;
    }
    return false;
#else
#error "Platform not supported"
#endif
}

bool Opal::IsDirectory(StringViewUtf8 path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (ToNativePath(path, native_path) != ErrorCode::Success)
    {
        return false;
    }
    struct stat buff;
    if (stat(native_path.data, &buff) == 0)
    {
        return S_ISDIR(buff.st_mode) != 0;
    }
    return false;
#else
#error "Platform not supported"
#endif
}

bool Opal::IsFile(StringViewUtf8 path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (ToNativePath(path, native_path) != ErrorCode::Success)
    {
        return false;
    }
    struct stat buff;
    if (stat(native_path.data, &buff) == 0)
    {
        return S_ISREG(buff.st_mode) != 0;
    }
    return false;
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::DynamicArray<Opal::DirectoryEntry>, Opal::ErrorCode> Opal::CollectDirectoryContents(StringUtf8 path,
                                                                                                        const DirectoryContentsDesc& desc)
{
    using Result = Expected<DynamicArray<DirectoryEntry>, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    ErrorCode err = ToWidePath(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return Result(ErrorCode::PathNotFound);
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return Result(ErrorCode::NotDirectory);
    }

    DynamicArray<StringWide> directories;
    if (directories.PushBack(Move(path_wide)) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }

    DynamicArray<DirectoryEntry> out_contents;
    while (!directories.IsEmpty())
    {
        const StringWide dir_wide = Move(directories.Back());
        directories.PopBack();

        WIN32_FIND_DATAW find_data;
        HANDLE find_handle = FindFirstFileW(*(dir_wide + L"\\*"), &find_data);
        if (find_handle == INVALID_HANDLE_VALUE)
        {
            return Result(ErrorCode::NotDirectory);
        }
        ErrorCode walk_err = ErrorCode::Success;
        do
        {
            // A view, so that skipping the two special entries costs no allocation and cannot fail.
            const StringViewWide child_name_wide(find_data.cFileName);
            if (child_name_wide == L"." || child_name_wide == L"..")
            {
                continue;
            }
            StringWide child_path_wide;
            err = child_path_wide.Append(dir_wide);
            if (err == ErrorCode::Success)
            {
                err = child_path_wide.Append(L'\\');
            }
            if (err == ErrorCode::Success)
            {
                err = child_path_wide.Append(find_data.cFileName);
            }
            if (err != ErrorCode::Success) [[unlikely]]
            {
                walk_err = err;
                break;
            }
            StringUtf8 child_path;
            err = child_path.Resize(child_path_wide.GetSize(), '\0');
            if (err == ErrorCode::Success)
            {
                err = Transcode(child_path_wide, child_path);
            }
            if (err != ErrorCode::Success)
            {
                walk_err = err;
                break;
            }
            const bool is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            const FileStatus status =
                ToFileStatus(find_data.ftLastWriteTime, find_data.nFileSizeHigh, find_data.nFileSizeLow, find_data.dwFileAttributes);
            if (is_directory && desc.include_directories)
            {
                if (out_contents.PushBack({.path = std::move(child_path), .is_directory = true, .status = status}) != ErrorCode::Success)
                    [[unlikely]]
                {
                    walk_err = ErrorCode::OutOfMemory;
                    break;
                }
            }
            if (!is_directory)
            {
                if (out_contents.PushBack({.path = std::move(child_path), .is_directory = false, .status = status}) != ErrorCode::Success)
                    [[unlikely]]
                {
                    walk_err = ErrorCode::OutOfMemory;
                    break;
                }
            }
            if (desc.recursive && is_directory)
            {
                if (directories.PushBack(Move(child_path_wide)) != ErrorCode::Success) [[unlikely]]
                {
                    walk_err = ErrorCode::OutOfMemory;
                    break;
                }
            }
        } while (FindNextFileW(find_handle, &find_data) != 0);
        FindClose(find_handle);
        if (walk_err != ErrorCode::Success)
        {
            return Result(walk_err);
        }
    }
    return Result(Move(out_contents));
#elif defined(OPAL_PLATFORM_LINUX)
    DynamicArray<StringUtf8> directories;
    if (directories.PushBack(std::move(path)) != ErrorCode::Success) [[unlikely]]
    {
        return Result(ErrorCode::OutOfMemory);
    }

    DynamicArray<DirectoryEntry> out_contents;

    while (!directories.IsEmpty())
    {
        StringUtf8 dir_path = std::move(directories.Back());
        directories.PopBack();
        DIR* dir = opendir(*dir_path);
        if (dir == nullptr)
        {
            if (errno == ENOENT)
            {
                return Result(ErrorCode::PathNotFound);
            }
            if (errno == ENOTDIR)
            {
                return Result(ErrorCode::NotDirectory);
            }
            return Result(ErrorCode::OSFailure);
        }

        ErrorCode walk_err = ErrorCode::Success;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            StringViewUtf8 entry_name(entry->d_name);
            if (entry_name == "." || entry_name == "..")
            {
                continue;
            }

            Expected<StringUtf8, ErrorCode> combined = Paths::Combine(dir_path, entry_name);
            if (!combined.HasValue()) [[unlikely]]
            {
                walk_err = combined.GetError();
                break;
            }
            StringUtf8 entry_path = std::move(combined).GetValue();
            struct stat statbuf;
            if (stat(*entry_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    if (out_contents.PushBack({.path = std::move(entry_path), .is_directory = false, .status = ToFileStatus(statbuf)}) !=
                        ErrorCode::Success) [[unlikely]]
                    {
                        walk_err = ErrorCode::OutOfMemory;
                        break;
                    }
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    if (desc.include_directories)
                    {
                        if (out_contents.PushBack({.path = entry_path.Clone(), .is_directory = true, .status = ToFileStatus(statbuf)}) !=
                            ErrorCode::Success) [[unlikely]]
                        {
                            walk_err = ErrorCode::OutOfMemory;
                            break;
                        }
                    }
                    if (desc.recursive)
                    {
                        if (directories.PushBack(std::move(entry_path)) != ErrorCode::Success) [[unlikely]]
                        {
                            walk_err = ErrorCode::OutOfMemory;
                            break;
                        }
                    }
                }
            }
        }
        closedir(dir);
        if (walk_err != ErrorCode::Success)
        {
            return Result(walk_err);
        }
    }
    return Result(Move(out_contents));
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::ReadFileAsString(StringViewUtf8 path)
{
    using Result = Expected<StringUtf8, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode transcode_err = ToWidePath(path, path_wide);
    if (transcode_err != ErrorCode::Success)
    {
        return Result(transcode_err);
    }

    constexpr DWORD k_share_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    HANDLE file_handle = CreateFileW(path_wide.GetData(), GENERIC_READ, k_share_mode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        const DWORD win32_err = GetLastError();
        if (win32_err == ERROR_FILE_NOT_FOUND || win32_err == ERROR_PATH_NOT_FOUND)
        {
            return Result(ErrorCode::PathNotFound);
        }
        return Result(ErrorCode::OSFailure);
    }

    LARGE_INTEGER file_size;
    if (GetFileSizeEx(file_handle, &file_size) == 0)
    {
        CloseHandle(file_handle);
        return Result(ErrorCode::OSFailure);
    }

    StringUtf8 result;
    const ErrorCode resize_err = result.Resize(static_cast<u64>(file_size.QuadPart), '\0');
    if (resize_err != ErrorCode::Success)
    {
        CloseHandle(file_handle);
        return Result(resize_err);
    }
    if (file_size.QuadPart > 0)
    {
        DWORD bytes_read = 0;
        if (ReadFile(file_handle, result.GetData(), static_cast<DWORD>(file_size.QuadPart), &bytes_read, nullptr) == 0)
        {
            CloseHandle(file_handle);
            return Result(ErrorCode::OSFailure);
        }
    }

    CloseHandle(file_handle);
    return Result(Move(result));
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return Result(error);
    }
    FILE* file = fopen(native_path.data, "rb");
    if (file == nullptr)
    {
        if (errno == ENOENT)
        {
            return Result(ErrorCode::PathNotFound);
        }
        return Result(ErrorCode::OSFailure);
    }

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < 0)
    {
        fclose(file);
        return Result(ErrorCode::OSFailure);
    }

    StringUtf8 result;
    const ErrorCode resize_err = result.Resize(static_cast<u64>(file_size), '\0');
    if (resize_err != ErrorCode::Success)
    {
        fclose(file);
        return Result(resize_err);
    }
    if (file_size > 0)
    {
        const u64 read_count = fread(result.GetData(), 1, static_cast<u64>(file_size), file);
        if (read_count != static_cast<u64>(file_size))
        {
            fclose(file);
            return Result(ErrorCode::OSFailure);
        }
    }

    fclose(file);
    return Result(Move(result));
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::DynamicArray<Opal::u8>, Opal::ErrorCode> Opal::ReadFileAsBytes(StringViewUtf8 path)
{
    using Result = Expected<DynamicArray<u8>, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode transcode_err = ToWidePath(path, path_wide);
    if (transcode_err != ErrorCode::Success)
    {
        return Result(transcode_err);
    }

    constexpr DWORD k_share_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    HANDLE file_handle = CreateFileW(path_wide.GetData(), GENERIC_READ, k_share_mode, nullptr,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        const DWORD win32_err = GetLastError();
        if (win32_err == ERROR_FILE_NOT_FOUND || win32_err == ERROR_PATH_NOT_FOUND)
        {
            return Result(ErrorCode::PathNotFound);
        }
        return Result(ErrorCode::OSFailure);
    }

    LARGE_INTEGER file_size;
    if (GetFileSizeEx(file_handle, &file_size) == 0)
    {
        CloseHandle(file_handle);
        return Result(ErrorCode::OSFailure);
    }

    DynamicArray<u8> result;
    const ErrorCode resize_err = result.Resize(static_cast<u64>(file_size.QuadPart), static_cast<u8>(0));
    if (resize_err != ErrorCode::Success)
    {
        CloseHandle(file_handle);
        return Result(resize_err);
    }
    if (file_size.QuadPart > 0)
    {
        DWORD bytes_read = 0;
        if (ReadFile(file_handle, result.GetData(), static_cast<DWORD>(file_size.QuadPart), &bytes_read, nullptr) == 0)
        {
            CloseHandle(file_handle);
            return Result(ErrorCode::OSFailure);
        }
    }

    CloseHandle(file_handle);
    return Result(Move(result));
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return Result(error);
    }
    FILE* file = fopen(native_path.data, "rb");
    if (file == nullptr)
    {
        if (errno == ENOENT)
        {
            return Result(ErrorCode::PathNotFound);
        }
        return Result(ErrorCode::OSFailure);
    }

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < 0)
    {
        fclose(file);
        return Result(ErrorCode::OSFailure);
    }

    DynamicArray<u8> result;
    const ErrorCode resize_err = result.Resize(static_cast<u64>(file_size), static_cast<u8>(0));
    if (resize_err != ErrorCode::Success)
    {
        fclose(file);
        return Result(resize_err);
    }
    if (file_size > 0)
    {
        const u64 read_count = fread(result.GetData(), 1, static_cast<u64>(file_size), file);
        if (read_count != static_cast<u64>(file_size))
        {
            fclose(file);
            return Result(ErrorCode::OSFailure);
        }
    }

    fclose(file);
    return Result(Move(result));
#else
#error "Platform not supported"
#endif
}

namespace
{

#if defined(OPAL_PLATFORM_WINDOWS)
Opal::ErrorCode WriteToFileWin32(Opal::StringViewUtf8 path, const void* data, Opal::u64 size, DWORD creation_disposition)
{
    using namespace Opal;

    StringWide path_wide;
    const ErrorCode transcode_err = ToWidePath(path, path_wide);
    if (transcode_err != ErrorCode::Success)
    {
        return transcode_err;
    }

    constexpr DWORD k_share_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    HANDLE file_handle =
        CreateFileW(path_wide.GetData(), GENERIC_WRITE, k_share_mode, nullptr, creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        const DWORD win32_err = GetLastError();
        if (win32_err == ERROR_PATH_NOT_FOUND)
        {
            return ErrorCode::PathNotFound;
        }
        return ErrorCode::OSFailure;
    }

    if (creation_disposition == OPEN_ALWAYS)
    {
        if (SetFilePointer(file_handle, 0, nullptr, FILE_END) == INVALID_SET_FILE_POINTER)
        {
            CloseHandle(file_handle);
            return ErrorCode::OSFailure;
        }
    }

    if (size > 0)
    {
        DWORD bytes_written = 0;
        if (WriteFile(file_handle, data, static_cast<DWORD>(size), &bytes_written, nullptr) == 0)
        {
            CloseHandle(file_handle);
            return ErrorCode::OSFailure;
        }
    }

    CloseHandle(file_handle);
    return ErrorCode::Success;
}
#elif defined(OPAL_PLATFORM_LINUX)
Opal::ErrorCode WriteToFileLinux(Opal::StringViewUtf8 path, const void* data, Opal::u64 size, const char* mode)
{
    using namespace Opal;

    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return error;
    }
    FILE* file = fopen(native_path.data, mode);
    if (file == nullptr)
    {
        if (errno == ENOENT)
        {
            return ErrorCode::PathNotFound;
        }
        return ErrorCode::OSFailure;
    }

    if (size > 0)
    {
        const u64 write_count = fwrite(data, 1, size, file);
        if (write_count != size)
        {
            fclose(file);
            return ErrorCode::OSFailure;
        }
    }

    fclose(file);
    return ErrorCode::Success;
}
#endif

}  // namespace

Opal::ErrorCode Opal::WriteStringToFile(StringViewUtf8 path, StringViewUtf8 content)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return WriteToFileWin32(path, content.GetData(), content.GetSize(), CREATE_ALWAYS);
#elif defined(OPAL_PLATFORM_LINUX)
    return WriteToFileLinux(path, content.GetData(), content.GetSize(), "wb");
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::WriteBytesToFile(StringViewUtf8 path, ArrayView<const u8> content)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return WriteToFileWin32(path, content.GetData(), content.GetSize(), CREATE_ALWAYS);
#elif defined(OPAL_PLATFORM_LINUX)
    return WriteToFileLinux(path, content.GetData(), content.GetSize(), "wb");
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::AppendStringToFile(StringViewUtf8 path, StringViewUtf8 content)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return WriteToFileWin32(path, content.GetData(), content.GetSize(), OPEN_ALWAYS);
#elif defined(OPAL_PLATFORM_LINUX)
    return WriteToFileLinux(path, content.GetData(), content.GetSize(), "ab");
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::AppendBytesToFile(StringViewUtf8 path, ArrayView<const u8> content)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return WriteToFileWin32(path, content.GetData(), content.GetSize(), OPEN_ALWAYS);
#elif defined(OPAL_PLATFORM_LINUX)
    return WriteToFileLinux(path, content.GetData(), content.GetSize(), "ab");
#else
#error "Platform not supported"
#endif
}

Opal::Expected<Opal::FileStatus, Opal::ErrorCode> Opal::GetFileStatus(StringViewUtf8 path)
{
    using Result = Expected<FileStatus, ErrorCode>;
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide;
    const ErrorCode transcode_err = ToWidePath(path, path_wide);
    if (transcode_err != ErrorCode::Success)
    {
        return Result(transcode_err == ErrorCode::OutOfMemory ? ErrorCode::OutOfMemory : ErrorCode::InvalidArgument);
    }
    WIN32_FILE_ATTRIBUTE_DATA attributes = {};
    if (GetFileAttributesExW(path_wide.GetData(), GetFileExInfoStandard, &attributes) == 0)
    {
        const DWORD win32_err = GetLastError();
        const bool missing = win32_err == ERROR_FILE_NOT_FOUND || win32_err == ERROR_PATH_NOT_FOUND;
        return Result(missing ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }
    return Result(ToFileStatus(attributes.ftLastWriteTime, attributes.nFileSizeHigh, attributes.nFileSizeLow, attributes.dwFileAttributes));
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath native_path;
    if (const ErrorCode error = ToNativePath(path, native_path); error != ErrorCode::Success)
    {
        return Result(error);
    }
    struct stat info = {};
    if (stat(native_path.data, &info) != 0)
    {
        return Result(errno == ENOENT || errno == ENOTDIR ? ErrorCode::PathNotFound : ErrorCode::OSFailure);
    }
    return Result(ToFileStatus(info));
#else
#error "Platform not supported"
#endif
}

Opal::ErrorCode Opal::CreateDirectories(StringViewUtf8 path)
{
    if (IsDirectory(path))
    {
        return ErrorCode::Success;
    }
    if (Exists(path))
    {
        return ErrorCode::AlreadyExists;
    }
    // Intermediate failures are ignored: a root, drive or share prefix cannot be created but exists, and a prefix that truly
    // could not be made shows up as PathNotFound on the next level.
    for (u64 i = 1; i < path.GetSize(); ++i)
    {
        const char8 c = path[i];
        if (c != '/' && c != '\\')
        {
            continue;
        }
        if (i == 2 && path[1] == ':')
        {
            continue;
        }
        const StringViewUtf8 prefix(path.GetData(), i);
        if (!IsDirectory(prefix))
        {
            [[maybe_unused]] const ErrorCode ignored = CreateDirectory(prefix);
        }
    }
    const ErrorCode err = CreateDirectory(path, false);
    if (err == ErrorCode::Success && !IsDirectory(path))
    {
        return ErrorCode::AlreadyExists;
    }
    return err;
}

Opal::ErrorCode Opal::RenameFile(StringViewUtf8 from, StringViewUtf8 to)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide from_wide;
    StringWide to_wide;
    ErrorCode err = ToWidePath(from, from_wide);
    if (err == ErrorCode::Success)
    {
        err = ToWidePath(to, to_wide);
    }
    if (err != ErrorCode::Success)
    {
        return err;
    }
    if (MoveFileExW(from_wide.GetData(), to_wide.GetData(), MOVEFILE_REPLACE_EXISTING) != 0)
    {
        return ErrorCode::Success;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_FILE_NOT_FOUND || win32_err == ERROR_PATH_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    NativePath from_native;
    NativePath to_native;
    ErrorCode err = ToNativePath(from, from_native);
    if (err == ErrorCode::Success)
    {
        err = ToNativePath(to, to_native);
    }
    if (err != ErrorCode::Success)
    {
        return err;
    }
    if (rename(from_native.data, to_native.data) == 0)
    {
        return ErrorCode::Success;
    }
    return errno == ENOENT ? ErrorCode::PathNotFound : ErrorCode::OSFailure;
#else
#error "Platform not supported"
#endif
}

Opal::Expected<bool, Opal::ErrorCode> Opal::WriteStringToFileIfChanged(StringViewUtf8 path, StringViewUtf8 content)
{
    using Result = Expected<bool, ErrorCode>;
    {
        Expected<StringUtf8, ErrorCode> existing = ReadFileAsString(path);
        if (existing.HasValue())
        {
            const StringUtf8& current = existing.GetValue();
            if (current.GetSize() == content.GetSize() && (content.GetSize() == 0 || memcmp(current.GetData(), content.GetData(), content.GetSize()) == 0))
            {
                return Result(false);
            }
        }
        else if (existing.GetError() == ErrorCode::OutOfMemory)
        {
            return Result(ErrorCode::OutOfMemory);
        }
    }

    static Atomic<u64> s_temp_counter{0};
#if defined(OPAL_PLATFORM_WINDOWS)
    const u64 process_id = GetCurrentProcessId();
#elif defined(OPAL_PLATFORM_LINUX)
    const u64 process_id = static_cast<u64>(getpid());
#endif
    char8 suffix[64] = {};
    snprintf(suffix, sizeof(suffix), ".tmp.%llu.%llu", process_id, s_temp_counter.FetchAdd(1));
    StringUtf8 temp_path;
    ErrorCode err = temp_path.Append(path);
    if (err == ErrorCode::Success)
    {
        err = temp_path.Append(StringViewUtf8(suffix));
    }
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }

    err = WriteStringToFile(temp_path, content);
    if (err != ErrorCode::Success)
    {
        return Result(err);
    }
    err = RenameFile(temp_path, path);
    if (err != ErrorCode::Success)
    {
        [[maybe_unused]] const ErrorCode ignored = DeleteFile(temp_path);
        return Result(err);
    }
    return Result(true);
}

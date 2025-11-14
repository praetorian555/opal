#include "opal/file-system.h"

#include <stdio.h>

#include "opal/paths.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include <Windows.h>
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#elif defined(OPAL_PLATFORM_LINUX)
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

Opal::ErrorCode Opal::CreateFile(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    Opal::StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return err;
    }

    constexpr DWORD k_access = GENERIC_READ | GENERIC_WRITE;
    constexpr DWORD k_shared_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    const DWORD creation_flags = CREATE_NEW;

    HANDLE file_handle = CreateFileW(path_wide.GetData(), k_access, k_shared_mode, nullptr, creation_flags, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        return ErrorCode::Success;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_PATH_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    if (win32_err == ERROR_FILE_EXISTS)
    {
        return ErrorCode::AlreadyExists;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    i32 flags = O_CREAT | O_EXCL;
    i32 fd = open(path.GetData(), flags, 0644);
    if (fd != -1)
    {
        close(fd);
        return ErrorCode::Success;
    }
    if (errno == EEXIST)
    {
        return ErrorCode::AlreadyExists;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#else
    return ErrorCode::NotImplemented;
#endif
}

Opal::ErrorCode Opal::DeleteFile(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
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
    i32 result = remove(path.GetData());
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
    return ErrorCode::NotImplemented;
#endif
}

Opal::ErrorCode Opal::CreateDirectory(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
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
        return ErrorCode::AlreadyExists;
    }
    if (error == ERROR_PATH_NOT_FOUND)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    if (mkdir(*path, 0777) == 0)
    {
        return ErrorCode::Success;
    }
    // Path already exists
    if (errno == EEXIST)
    {
        return ErrorCode::AlreadyExists;
    }
    if (errno == ENOENT)
    {
        return ErrorCode::PathNotFound;
    }
    return ErrorCode::OSFailure;
#else
    return ErrorCode::NotImplemented;
#endif
}

Opal::ErrorCode Opal::DeleteDirectory(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
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
    if (rmdir(*path) == 0)
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
    return ErrorCode::NotImplemented;
#endif
}

bool Opal::Exists(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
    if (path.IsEmpty())
    {
        return false;
    }
    Expected<StringUtf8, ErrorCode> result = Paths::NormalizePath(path, scratch_allocator);
    if (!result.HasValue())
    {
        return false;
    }

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(result.GetValue().GetSize() * 2, L'\0', scratch_allocator);
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

bool Opal::IsDirectory(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
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
#else
    // TODO: Implement
    return false;
#endif
}

bool Opal::IsFile(const StringUtf8& path, AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    const ErrorCode err = Transcode(path, path_wide);
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
#else
    // TODO: Implement
    return false;
#endif
}

bool Opal::CollectDirectoryContents(const StringUtf8& path, DynamicArray<StringUtf8>& out_contents, const DirectoryContentsDesc& desc,
                                    AllocatorBase* scratch_allocator)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return false;
    }

    DynamicArray<StringWide> directories;
    path_wide += L"\\*";
    directories.PushBack(Move(path_wide));
    while (!directories.IsEmpty())
    {
        StringWide dir_wide = Move(directories.Back().GetValue());
        directories.PopBack();

        WIN32_FIND_DATAW find_data;
        HANDLE find_handle = FindFirstFileW(*dir_wide, &find_data);
        if (find_handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        do
        {
            StringWide child_path_wide = find_data.cFileName;
            if (child_path_wide == L"." || child_path_wide == L"..")
            {
                continue;
            }
            StringUtf8 child_path(child_path_wide.GetSize(), '\0');
            err = Transcode(child_path_wide, child_path);
            if (err != ErrorCode::Success)
            {
                return false;
            }
            const bool is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            if (is_directory && desc.include_directories)
            {
                out_contents.PushBack(child_path);
            }
            if (!is_directory)
            {
                out_contents.PushBack(child_path);
            }
            if (desc.recursive && is_directory)
            {
                child_path_wide += L"\\*";
                directories.PushBack(Move(child_path_wide));
            }
        } while (FindNextFileW(find_handle, &find_data) != 0);
    }
    return true;
#else
    // TODO: Implement
    return false;
#endif
}

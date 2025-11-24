#include "opal/file-system.h"

#include <stdio.h>

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

void Opal::CreateFile(const StringUtf8& path, bool fail_if_already_exists)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    Opal::StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
    if (Transcode(path, path_wide) != ErrorCode::Success)
    {
        // TODO: Remove this once the Transcode starts throwing
        throw Exception("Failed to transcode path!");
    }

    constexpr DWORD k_access = GENERIC_READ | GENERIC_WRITE;
    constexpr DWORD k_shared_mode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    constexpr DWORD k_creation_flags = CREATE_NEW;

    HANDLE file_handle =
        CreateFileW(path_wide.GetData(), k_access, k_shared_mode, nullptr, k_creation_flags, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        return;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_PATH_NOT_FOUND)
    {
        throw PathNotFoundException(*path);
    }
    if (win32_err == ERROR_FILE_EXISTS)
    {
        if (!fail_if_already_exists)
        {
            return;
        }
        throw PathAlreadyExistsException(*path);
    }
    throw Exception("Failed to create a file!");
#elif defined(OPAL_PLATFORM_LINUX)
    const i32 flags = O_CREAT | O_EXCL;
    i32 fd = open(path.GetData(), flags, 0644);
    if (fd != -1)
    {
        close(fd);
        return;
    }
    if (errno == EEXIST)
    {
        if (!fail_if_already_exists)
        {
            return;
        }
        throw PathAlreadyExistsException(*path);
    }
    if (errno == ENOENT)
    {
        throw PathNotFoundException(*path);
    }
    throw Exception("Failed to create a file!");
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::DeleteFile(const StringUtf8& path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
    const ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        // TODO: Remove this once the Transcode starts throwing
        throw Exception("Failed to transcode path!");
    }
    if (DeleteFileW(path_wide.GetData()) != 0)
    {
        return;
    }
    const DWORD win32_error = GetLastError();
    if (win32_error == ERROR_FILE_NOT_FOUND)
    {
        throw PathNotFoundException(*path);
    }
    throw Exception("Failed to delete a file!");
#elif defined(OPAL_PLATFORM_LINUX)
    i32 result = remove(path.GetData());
    if (result == 0)
    {
        return;
    }
    if (errno == ENOENT)
    {
        throw PathNotFoundException(*path);
    }
    throw Exception("Failed to delete a file!");
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::CreateDirectory(const StringUtf8& path, bool throw_if_exists)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
    const ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        // TODO: Remove this once the Transcode starts throwing
        throw Exception("Failed to transcode path!");
    }
    const BOOL result = CreateDirectoryW(*path_wide, nullptr);
    if (result != 0)
    {
        return;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS)
    {
        if (!throw_if_exists)
        {
            return;
        }
        throw PathAlreadyExistsException(*path);
    }
    if (error == ERROR_PATH_NOT_FOUND)
    {
        throw PathNotFoundException(*path);
    }
    throw Exception("Failed to create a directory!");
#elif defined(OPAL_PLATFORM_LINUX)
    if (mkdir(*path, 0777) == 0)
    {
        return;
    }
    // Path already exists
    if (errno == EEXIST)
    {
        if (!throw_if_exists)
        {
            return;
        }
        throw PathAlreadyExistsException(*path);
    }
    if (errno == ENOENT)
    {
        throw PathNotFoundException(*path);
    }
    throw Exception("Failed to create a directory!");
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

void Opal::DeleteDirectory(const StringUtf8& path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
    const ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        // TODO: Remove this once the Transcode starts throwing
        throw Exception("Failed to transcode path!");
    }
    const BOOL status = RemoveDirectoryW(*path_wide);
    if (status != 0)
    {
        return;
    }
    const DWORD win32_err = GetLastError();
    if (win32_err == ERROR_FILE_NOT_FOUND)
    {
        throw PathNotFoundException(*path);
    }
    if (win32_err == ERROR_DIR_NOT_EMPTY)
    {
        throw DirectoryNotEmptyException(*path);
    }
    throw Exception("Failed to delete a directory!");
#elif defined(OPAL_PLATFORM_LINUX)
    if (rmdir(*path) == 0)
    {
        return;
    }
    if (errno == ENOENT)
    {
        throw PathNotFoundException(*path);
    }
    if (errno == ENOTEMPTY)
    {
        throw DirectoryNotEmptyException(*path);
    }
    throw Exception("Failed to delete a directory!");
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

bool Opal::Exists(const StringUtf8& path)
{
    if (path.IsEmpty())
    {
        return false;
    }
    Expected<StringUtf8, ErrorCode> result = Paths::NormalizePath(path, GetScratchAllocator());
    if (!result.HasValue())
    {
        return false;
    }

#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(result.GetValue().GetSize() * 2, L'\0', GetScratchAllocator());
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

bool Opal::IsDirectory(const StringUtf8& path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
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
#elif defined(OPAL_PLATFORM_LINUX)
    struct stat buff;
    if (stat(*path, &buff) == 0)
    {
        return S_ISDIR(buff.st_mode) != 0;
    }
    return false;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

bool Opal::IsFile(const StringUtf8& path)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', GetScratchAllocator());
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
#elif defined(OPAL_PLATFORM_LINUX)
    struct stat buff;
    if (stat(*path, &buff) == 0)
    {
        return S_ISREG(buff.st_mode) != 0;
    }
    return false;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

Opal::DynamicArray<Opal::DirectoryEntry> Opal::CollectDirectoryContents(const StringUtf8& path, const DirectoryContentsDesc& desc)
{
    AllocatorBase* scratch_allocator = GetScratchAllocator();
    AllocatorBase* output_allocator = GetDefaultAllocator();
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide path_wide(path.GetSize() * 2, L'\0', scratch_allocator);
    ErrorCode err = Transcode(path, path_wide);
    if (err != ErrorCode::Success)
    {
        throw Exception("Failed to transcode path!");
    }
    const DWORD attributes = GetFileAttributesW(path_wide.GetData());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        throw PathNotFoundException(*path);
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        throw NotDirectoryException(*path);
    }

    DynamicArray<StringWide> directories(scratch_allocator);
    directories.PushBack(Move(path_wide));

    DynamicArray<DirectoryEntry> out_contents(output_allocator);
    while (!directories.IsEmpty())
    {
        const StringWide dir_wide = Move(directories.Back().GetValue());
        directories.PopBack();

        WIN32_FIND_DATAW find_data;
        HANDLE find_handle = FindFirstFileW(*(dir_wide + L"\\*"), &find_data);
        if (find_handle == INVALID_HANDLE_VALUE)
        {
            throw NotDirectoryException("");
        }
        do
        {
            const StringWide child_name_wide(find_data.cFileName, scratch_allocator);
            if (child_name_wide == L"." || child_name_wide == L"..")
            {
                continue;
            }
            StringWide child_path_wide(dir_wide + L"\\" + find_data.cFileName, scratch_allocator);
            if (child_path_wide == L"." || child_path_wide == L"..")
            {
                continue;
            }
            StringUtf8 child_path(child_path_wide.GetSize(), '\0', output_allocator);
            err = Transcode(child_path_wide, child_path);
            if (err != ErrorCode::Success)
            {
                throw Exception("Failed to transcode path!");
            }
            const bool is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            if (is_directory && desc.include_directories)
            {
                out_contents.PushBack({.path = child_path, .is_directory = true});
            }
            if (!is_directory)
            {
                out_contents.PushBack({.path = child_path, .is_directory = false});
            }
            if (desc.recursive && is_directory)
            {
                directories.PushBack(Move(child_path_wide));
            }
        } while (FindNextFileW(find_handle, &find_data) != 0);
    }
    return out_contents;
#elif defined(OPAL_PLATFORM_LINUX)
    DynamicArray<StringUtf8> directories(scratch_allocator);
    directories.PushBack(path);

    DynamicArray<DirectoryEntry> out_contents(output_allocator);

    while (!directories.IsEmpty())
    {
        StringUtf8 dir_path = directories.Back().GetValue();
        directories.PopBack();
        DIR* dir = opendir(*dir_path);
        if (dir == nullptr)
        {
            if (errno == ENOENT)
            {
                throw PathNotFoundException(*dir_path);
            }
            if (errno == ENOTDIR)
            {
                throw NotDirectoryException(*dir_path);
            }
            else
            {
                throw Exception("Failed to open directory!");
            }
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            StringUtf8 entry_name(entry->d_name, scratch_allocator);
            if (entry_name == "." || entry_name == "..")
            {
                continue;
            }

            StringUtf8 entry_path = Paths::Combine(output_allocator, dir_path, entry_name).GetValue();
            struct stat statbuf;
            if (stat(*entry_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    out_contents.PushBack({.path = entry_path, .is_directory = false});
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    if (desc.include_directories)
                    {
                        out_contents.PushBack({.path = entry_path, .is_directory = true});
                    }
                    if (desc.recursive)
                    {
                        directories.PushBack(Move(entry_path));
                    }
                }
            }
        }
    }
    return out_contents;
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}

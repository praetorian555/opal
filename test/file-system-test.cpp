#include "opal/file-system.h"

#include <stdio.h>

#include "opal/exceptions.h"
#include "opal/paths.h"
#include "opal/container/hash-set.h"
#include "opal/container/hash-map.h"

#include "test-helpers.h"

using namespace Opal;

namespace
{

void WriteDataToFile(const StringUtf8& path, const void* data, Opal::u64 size)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    FILE* f = nullptr;
    fopen_s(&f, *path, "wb");
#else
    FILE* f = fopen(*path, "wb");
#endif
    REQUIRE(f != nullptr);
    fwrite(data, 1, size, f);
    fclose(f);
}

// Unwrap the success case so the tests that are not about failure stay about what they read back.
StringUtf8 CwdOrFail()
{
    Expected<StringUtf8, ErrorCode> result = Paths::GetCurrentWorkingDirectory();
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

template <typename... Args>
StringUtf8 CombineOrFail(Args&&... args)
{
    Expected<StringUtf8, ErrorCode> result = Paths::Combine(std::forward<Args>(args)...);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

StringUtf8 ReadStringOrFail(const StringUtf8& path)
{
    Expected<StringUtf8, ErrorCode> result = ReadFileAsString(path);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

DynamicArray<u8> ReadBytesOrFail(const StringUtf8& path)
{
    Expected<DynamicArray<u8>, ErrorCode> result = ReadFileAsBytes(path);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

DynamicArray<DirectoryEntry> CollectOrFail(StringUtf8 path, const DirectoryContentsDesc& desc = {})
{
    Expected<DynamicArray<DirectoryEntry>, ErrorCode> result = CollectDirectoryContents(std::move(path), desc);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

/**
 * Remembers every path a test builds and removes whatever still exists when the scope ends.
 *
 * A failed REQUIRE aborts the rest of the section, so a test that creates a file and deletes it on the last line
 * leaves that file behind the moment anything before it fails. The next run then fails on its own
 * REQUIRE(!Exists(path)) precondition and reports a defect in code that is fine. This guard makes the leftovers go
 * away regardless of where a section stops.
 */
class ScopedTestPaths
{
public:
    ScopedTestPaths() = default;
    ScopedTestPaths(const ScopedTestPaths&) = delete;
    ScopedTestPaths& operator=(const ScopedTestPaths&) = delete;

    ~ScopedTestPaths()
    {
        // Reverse order, so a directory is only reached once the entries registered under it are gone.
        for (u64 i = m_paths.GetSize(); i > 0; --i)
        {
            const StringUtf8& path = m_paths[i - 1];
            if (!Exists(path))
            {
                continue;
            }
            if (IsDirectory(path))
            {
                (void)DeleteDirectory(path);
            }
            else
            {
                (void)DeleteFile(path);
            }
        }
    }

    /** Combine the parts into a path and remember it for cleanup. */
    template <typename... Args>
    StringUtf8 Path(Args&&... args)
    {
        StringUtf8 path = CombineOrFail(std::forward<Args>(args)...);
        REQUIRE(m_paths.PushBack(path.Clone()) == ErrorCode::Success);
        return path;
    }

private:
    DynamicArray<StringUtf8> m_paths;
};

}  // namespace

CATCH_TRANSLATE_EXCEPTION(const Opal::Exception& ex)
{
    return *ex.What();
}

TEST_CASE("Exists", "[FileSystem]")
{
    SECTION("Empty path")
    {
        auto exists = Exists("");
        REQUIRE(!exists);
    }
    SECTION("Non-existing paths")
    {
        auto exists = Exists("this-does-not-exist");
        REQUIRE(!exists);
    }
    SECTION("Existing paths")
    {
        auto exists = Exists(".");
        REQUIRE(exists);
    }
}

TEST_CASE("Creating and deleting a file", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();
    SECTION("Create and delete a file")
    {
        path = temp.Path(path, "example.txt");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateFile(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        REQUIRE(DeleteFile(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a file if part of the path does not exist")
    {
        path = temp.Path(path, "test-dir", "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE(CreateFile(path) == ErrorCode::PathNotFound);
    }
    SECTION("Try to create a file that already exists")
    {
        path = temp.Path(path, "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE(CreateFile(path) == ErrorCode::Success);
        REQUIRE(Exists(path));
        REQUIRE(CreateFile(path, true) == ErrorCode::AlreadyExists);
        REQUIRE(CreateFile(path) == ErrorCode::Success);
        REQUIRE(DeleteFile(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existent file")
    {
        path = temp.Path(path, "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE(DeleteFile(path) == ErrorCode::PathNotFound);
    }
}

TEST_CASE("Creating and destroying directory", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();
    SECTION("Create and delete a directory")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory if part of the path does not exist")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        path = temp.Path(path, "test-dir-2");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::PathNotFound);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory that already exist")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        REQUIRE(CreateDirectory(path, true) == ErrorCode::AlreadyExists);
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existent directory")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(DeleteDirectory(path) == ErrorCode::PathNotFound);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-empty directory")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        const StringUtf8 file_path = temp.Path(path, "test-file");
        REQUIRE(!Opal::Exists(file_path));
        REQUIRE(CreateFile(file_path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(file_path));
        REQUIRE(DeleteDirectory(path) == ErrorCode::NotEmpty);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
    }
}

template<> struct std::hash<Opal::StringUtf8> {
    size_t operator()(const Opal::StringUtf8& s) const noexcept {
        return Opal::Hash::CalcRawArray(reinterpret_cast<const Opal::u8*>(s.GetData()), s.GetSize());
    }
};

TEST_CASE("Iterate over directory contents", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();
    SECTION("Directory doesn't exist")
    {
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(Opal::CollectDirectoryContents(std::move(path)).GetError() == ErrorCode::PathNotFound);
    }
    SECTION("Path is not to directory")
    {
        path = temp.Path( path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateFile(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        REQUIRE(Opal::CollectDirectoryContents(path.Clone()).GetError() == ErrorCode::NotDirectory);
        REQUIRE(DeleteFile(path) == ErrorCode::Success);
    }
    SECTION("Collect child contents, no recursive search")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = temp.Path(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE(CreateFile(first_file) == ErrorCode::Success);
        StringUtf8 another_dir = temp.Path( path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE(CreateDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = temp.Path(another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE(CreateFile(another_file) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        children = CollectOrFail(path.Clone());
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE(DeleteFile(first_file) == ErrorCode::Success);
        REQUIRE(DeleteFile(another_file) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = temp.Path(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE(CreateFile(first_file) == ErrorCode::Success);
        StringUtf8 another_dir = temp.Path(path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE(CreateDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = temp.Path( another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE(CreateFile(another_file) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        children = CollectOrFail(path.Clone(), {.recursive = true});
        REQUIRE(children.GetSize() == 3);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE(DeleteFile(first_file) == ErrorCode::Success);
        REQUIRE(DeleteFile(another_file) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search, ignore directories")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = temp.Path(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE(CreateDirectory(path) == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = temp.Path(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE(CreateFile(first_file) == ErrorCode::Success);
        StringUtf8 another_dir = temp.Path(path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE(CreateDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = temp.Path(another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE(CreateFile(another_file) == ErrorCode::Success);
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        children = CollectOrFail(path.Clone(), {.include_directories = false, .recursive = true});
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE(DeleteFile(first_file) == ErrorCode::Success);
        REQUIRE(DeleteFile(another_file) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(another_dir) == ErrorCode::Success);
        REQUIRE(DeleteDirectory(path) == ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
}

TEST_CASE("ReadFileAsString", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Read non-existent file")
    {
        StringUtf8 file_path = temp.Path(path, "non-existent-file.txt");
        REQUIRE(!Exists(file_path));
        REQUIRE(ReadFileAsString(file_path).GetError() == ErrorCode::PathNotFound);
    }
    SECTION("Read empty file")
    {
        StringUtf8 file_path = temp.Path(path, "empty-file.txt");
        REQUIRE(CreateFile(file_path) == ErrorCode::Success);
        StringUtf8 content;
        content = ReadStringOrFail(file_path);
        REQUIRE(content.IsEmpty());
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Read file with content")
    {
        StringUtf8 file_path = temp.Path(path, "test-read.txt");
        REQUIRE(!Exists(file_path));

        const char* expected = "Hello, Opal!";
        WriteDataToFile(file_path, expected, strlen(expected));

        StringUtf8 content;
        content = ReadStringOrFail(file_path);
        REQUIRE(content == expected);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Read file with multiple lines")
    {
        StringUtf8 file_path = temp.Path(path, "test-multiline.txt");
        REQUIRE(!Exists(file_path));

        const char* expected = "line1\nline2\nline3\n";
        WriteDataToFile(file_path, expected, strlen(expected));

        StringUtf8 content;
        content = ReadStringOrFail(file_path);
        REQUIRE(content == expected);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("ReadFileAsBytes", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Read non-existent file")
    {
        StringUtf8 file_path = temp.Path(path, "non-existent-file.bin");
        REQUIRE(!Exists(file_path));
        REQUIRE(ReadFileAsBytes(file_path).GetError() == ErrorCode::PathNotFound);
    }
    SECTION("Read empty file")
    {
        StringUtf8 file_path = temp.Path(path, "empty-file.bin");
        REQUIRE(CreateFile(file_path) == ErrorCode::Success);
        DynamicArray<u8> content;
        content = ReadBytesOrFail(file_path);
        REQUIRE(content.IsEmpty());
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Read file with content")
    {
        StringUtf8 file_path = temp.Path(path, "test-read.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        WriteDataToFile(file_path, expected, sizeof(expected));

        DynamicArray<u8> content;
        content = ReadBytesOrFail(file_path);
        REQUIRE(content.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(content[i] == expected[i]);
        }
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Read file with binary data")
    {
        StringUtf8 file_path = temp.Path(path, "test-binary.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x00, 0xFF, 0x01, 0xFE, 0x80};
        WriteDataToFile(file_path, expected, sizeof(expected));

        DynamicArray<u8> content;
        content = ReadBytesOrFail(file_path);
        REQUIRE(content.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(content[i] == expected[i]);
        }
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("WriteStringToFile", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Write to non-existent directory")
    {
        StringUtf8 file_path = temp.Path(path, "no-such-dir", "file.txt");
        REQUIRE(WriteStringToFile(file_path, StringUtf8("data")) == ErrorCode::PathNotFound);
    }
    SECTION("Write creates new file")
    {
        StringUtf8 file_path = temp.Path(path, "write-new.txt");
        REQUIRE(!Exists(file_path));

        StringUtf8 content("Hello, Opal!");
        REQUIRE(WriteStringToFile(file_path, content) == ErrorCode::Success);
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back == content);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Write overwrites existing file")
    {
        StringUtf8 file_path = temp.Path(path, "write-overwrite.txt");

        REQUIRE(WriteStringToFile(file_path, StringUtf8("original content")) == ErrorCode::Success);
        REQUIRE(WriteStringToFile(file_path, StringUtf8("new")) == ErrorCode::Success);

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back == "new");
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Write empty string")
    {
        StringUtf8 file_path = temp.Path(path, "write-empty.txt");

        REQUIRE(WriteStringToFile(file_path, StringUtf8()) == ErrorCode::Success);
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back.IsEmpty());
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("WriteBytesToFile", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Write to non-existent directory")
    {
        StringUtf8 file_path = temp.Path(path, "no-such-dir", "file.bin");
        const u8 data[] = {0x01};
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(data)) == ErrorCode::PathNotFound);
    }
    SECTION("Write and read back bytes")
    {
        StringUtf8 file_path = temp.Path(path, "write-bytes.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x00, 0xFF, 0x42, 0x80};
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(expected)) == ErrorCode::Success);

        DynamicArray<u8> read_back;
        read_back = ReadBytesOrFail(file_path);
        REQUIRE(read_back.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(read_back[i] == expected[i]);
        }
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Write overwrites existing bytes")
    {
        StringUtf8 file_path = temp.Path(path, "write-bytes-over.bin");

        const u8 original[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(original)) == ErrorCode::Success);

        const u8 replacement[] = {0xAA, 0xBB};
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(replacement)) == ErrorCode::Success);

        DynamicArray<u8> read_back;
        read_back = ReadBytesOrFail(file_path);
        REQUIRE(read_back.GetSize() == sizeof(replacement));
        REQUIRE(read_back[0] == 0xAA);
        REQUIRE(read_back[1] == 0xBB);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("AppendStringToFile", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Append to non-existent directory")
    {
        StringUtf8 file_path = temp.Path(path, "no-such-dir", "file.txt");
        REQUIRE(AppendStringToFile(file_path, StringUtf8("data")) == ErrorCode::PathNotFound);
    }
    SECTION("Append creates new file")
    {
        StringUtf8 file_path = temp.Path(path, "append-new.txt");
        REQUIRE(!Exists(file_path));

        REQUIRE(AppendStringToFile(file_path, StringUtf8("Hello")) == ErrorCode::Success);
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back == "Hello");
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Append adds to existing content")
    {
        StringUtf8 file_path = temp.Path(path, "append-existing.txt");

        REQUIRE(WriteStringToFile(file_path, StringUtf8("Hello")) == ErrorCode::Success);
        REQUIRE(AppendStringToFile(file_path, StringUtf8(", World!")) == ErrorCode::Success);

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back == "Hello, World!");
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Multiple appends")
    {
        StringUtf8 file_path = temp.Path(path, "append-multi.txt");

        REQUIRE(AppendStringToFile(file_path, StringUtf8("line1\n")) == ErrorCode::Success);
        REQUIRE(AppendStringToFile(file_path, StringUtf8("line2\n")) == ErrorCode::Success);
        REQUIRE(AppendStringToFile(file_path, StringUtf8("line3\n")) == ErrorCode::Success);

        StringUtf8 read_back;
        read_back = ReadStringOrFail(file_path);
        REQUIRE(read_back == "line1\nline2\nline3\n");
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("AppendBytesToFile", "[FileSystem]")
{
    ScopedTestPaths temp;
    StringUtf8 path;
    path = CwdOrFail();

    SECTION("Append to non-existent directory")
    {
        StringUtf8 file_path = temp.Path(path, "no-such-dir", "file.bin");
        const u8 data[] = {0x01};
        REQUIRE(AppendBytesToFile(file_path, ArrayView<const u8>(data)) == ErrorCode::PathNotFound);
    }
    SECTION("Append creates new file")
    {
        StringUtf8 file_path = temp.Path(path, "append-bytes-new.bin");
        REQUIRE(!Exists(file_path));

        const u8 data[] = {0x01, 0x02};
        REQUIRE(AppendBytesToFile(file_path, ArrayView<const u8>(data)) == ErrorCode::Success);

        DynamicArray<u8> read_back;
        read_back = ReadBytesOrFail(file_path);
        REQUIRE(read_back.GetSize() == sizeof(data));
        REQUIRE(read_back[0] == 0x01);
        REQUIRE(read_back[1] == 0x02);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
    SECTION("Append adds to existing bytes")
    {
        StringUtf8 file_path = temp.Path(path, "append-bytes-existing.bin");

        const u8 first[] = {0xAA, 0xBB};
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(first)) == ErrorCode::Success);

        const u8 second[] = {0xCC, 0xDD};
        REQUIRE(AppendBytesToFile(file_path, ArrayView<const u8>(second)) == ErrorCode::Success);

        DynamicArray<u8> read_back;
        read_back = ReadBytesOrFail(file_path);
        REQUIRE(read_back.GetSize() == 4);
        REQUIRE(read_back[0] == 0xAA);
        REQUIRE(read_back[1] == 0xBB);
        REQUIRE(read_back[2] == 0xCC);
        REQUIRE(read_back[3] == 0xDD);
        REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
    }
}

TEST_CASE("File system reports a failed allocation", "[FileSystem]")
{
    ScopedTestPaths temp;
    const StringUtf8 cwd = CwdOrFail();
    const StringUtf8 file_path = temp.Path(cwd, "out-of-memory-probe.txt");
    // Long enough that reading it back cannot fit in a string's inline storage.
    StringUtf8 contents;
    REQUIRE(contents.Resize(256, 'x') == ErrorCode::Success);
    REQUIRE(WriteStringToFile(file_path, contents) == ErrorCode::Success);

    SECTION("Reading reports the failure instead of throwing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(ReadFileAsString(file_path).GetError() == ErrorCode::OutOfMemory);
        REQUIRE(ReadFileAsBytes(file_path).GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Collecting directory contents reports the failure instead of throwing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(CollectDirectoryContents(cwd.Clone()).GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Exists reports false rather than failing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(!Exists(file_path));
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    // These only allocate on Windows, where the path has to be widened first. The Linux implementations hand the
    // path straight to the syscall and have nothing that can fail.
    SECTION("Create and delete report the failure instead of throwing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(CreateFile(file_path) == ErrorCode::OutOfMemory);
        REQUIRE(DeleteFile(file_path) == ErrorCode::OutOfMemory);
        REQUIRE(CreateDirectory(file_path) == ErrorCode::OutOfMemory);
        REQUIRE(DeleteDirectory(file_path) == ErrorCode::OutOfMemory);
    }
    SECTION("Writing reports the failure instead of throwing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        const u8 data[] = {0x01, 0x02};
        REQUIRE(WriteStringToFile(file_path, StringUtf8("x")) == ErrorCode::OutOfMemory);
        REQUIRE(WriteBytesToFile(file_path, ArrayView<const u8>(data)) == ErrorCode::OutOfMemory);
        REQUIRE(AppendStringToFile(file_path, StringUtf8("x")) == ErrorCode::OutOfMemory);
        REQUIRE(AppendBytesToFile(file_path, ArrayView<const u8>(data)) == ErrorCode::OutOfMemory);
    }
    SECTION("Type queries report false rather than failing")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(!IsDirectory(file_path));
        REQUIRE(!IsFile(file_path));
    }
#endif

    REQUIRE(DeleteFile(file_path) == ErrorCode::Success);
}

#include "opal/file-system.h"

#include <cstdio>

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
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());
    SECTION("Create and delete a file")
    {
        path = Paths::Combine(path, "example.txt");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_NOTHROW(DeleteFile(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a file if part of the path does not exist")
    {
        path = Paths::Combine(path, "test-dir", "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE_THROWS_AS(CreateFile(path), Opal::PathNotFoundException);
    }
    SECTION("Try to create a file that already exists")
    {
        path = Paths::Combine(path, "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE(Exists(path));
        REQUIRE_THROWS_AS(CreateFile(path, true), Opal::PathAlreadyExistsException);
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE_NOTHROW(DeleteFile(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existent file")
    {
        path = Paths::Combine(path, "example.txt");
        REQUIRE(!Exists(path));
        REQUIRE_THROWS_AS(DeleteFile(path), Opal::PathNotFoundException);
    }
}

TEST_CASE("Creating and destroying directory", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());
    SECTION("Create and delete a directory")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory if part of the path does not exist")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        path = Paths::Combine(path, "test-dir-2");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_THROWS_AS(CreateDirectory(path), Opal::PathNotFoundException);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory that already exist")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_THROWS_AS(CreateDirectory(path, true), Opal::PathAlreadyExistsException);
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existent directory")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_THROWS_AS(DeleteDirectory(path), Opal::PathNotFoundException);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-empty directory")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        const StringUtf8 file_path = Paths::Combine(path, "test-file");
        REQUIRE(!Opal::Exists(file_path));
        REQUIRE_NOTHROW(CreateFile(file_path));
        REQUIRE(Opal::Exists(file_path));
        REQUIRE_THROWS_AS(DeleteDirectory(path), Opal::DirectoryNotEmptyException);
        REQUIRE_NOTHROW(DeleteFile(file_path));
        REQUIRE_NOTHROW(DeleteDirectory(path));
    }
}

template<> struct std::hash<Opal::StringUtf8> {
    std::size_t operator()(const Opal::StringUtf8& s) const noexcept {
        return Opal::Hash::CalcRawArray(reinterpret_cast<const Opal::u8*>(s.GetData()), s.GetSize());
    }
};

TEST_CASE("Iterate over directory contents", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());
    SECTION("Directory doesn't exist")
    {
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_THROWS_AS(children = Opal::CollectDirectoryContents(std::move(path)), PathNotFoundException);
    }
    SECTION("Path is not to directory")
    {
        path = Paths::Combine( path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE(Opal::Exists(path));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_THROWS_AS(children = Opal::CollectDirectoryContents(path.Clone()), NotDirectoryException);
        REQUIRE_NOTHROW(DeleteFile(path));
    }
    SECTION("Collect child contents, no recursive search")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine( path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path.Clone()));
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine( another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path.Clone(), {.recursive = true}));
        REQUIRE(children.GetSize() == 3);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search, ignore directories")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = Paths::Combine(path, "test-dir");
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(path, "test-file");
        dir_paths.Insert(first_file.Clone());
        path_types.Insert(first_file.Clone(), false);
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(path, "another-dir");
        dir_paths.Insert(another_dir.Clone());
        path_types.Insert(another_dir.Clone(), true);
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(another_dir, "another-file");
        dir_paths.Insert(another_file.Clone());
        path_types.Insert(another_file.Clone(), false);
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path.Clone(), {.include_directories = false, .recursive = true}));
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types.GetValue(e.path) == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.Erase(e.path);
        }
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
}

TEST_CASE("ReadFileAsString", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Read non-existent file")
    {
        StringUtf8 file_path = Paths::Combine(path, "non-existent-file.txt");
        REQUIRE(!Exists(file_path));
        REQUIRE_THROWS_AS(ReadFileAsString(file_path), PathNotFoundException);
    }
    SECTION("Read empty file")
    {
        StringUtf8 file_path = Paths::Combine(path, "empty-file.txt");
        REQUIRE_NOTHROW(CreateFile(file_path));
        StringUtf8 content;
        REQUIRE_NOTHROW(content = ReadFileAsString(file_path));
        REQUIRE(content.IsEmpty());
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Read file with content")
    {
        StringUtf8 file_path = Paths::Combine(path, "test-read.txt");
        REQUIRE(!Exists(file_path));

        const char* expected = "Hello, Opal!";
        WriteDataToFile(file_path, expected, strlen(expected));

        StringUtf8 content;
        REQUIRE_NOTHROW(content = ReadFileAsString(file_path));
        REQUIRE(content == expected);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Read file with multiple lines")
    {
        StringUtf8 file_path = Paths::Combine(path, "test-multiline.txt");
        REQUIRE(!Exists(file_path));

        const char* expected = "line1\nline2\nline3\n";
        WriteDataToFile(file_path, expected, strlen(expected));

        StringUtf8 content;
        REQUIRE_NOTHROW(content = ReadFileAsString(file_path));
        REQUIRE(content == expected);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

TEST_CASE("ReadFileAsBytes", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Read non-existent file")
    {
        StringUtf8 file_path = Paths::Combine(path, "non-existent-file.bin");
        REQUIRE(!Exists(file_path));
        REQUIRE_THROWS_AS(ReadFileAsBytes(file_path), PathNotFoundException);
    }
    SECTION("Read empty file")
    {
        StringUtf8 file_path = Paths::Combine(path, "empty-file.bin");
        REQUIRE_NOTHROW(CreateFile(file_path));
        DynamicArray<u8> content;
        REQUIRE_NOTHROW(content = ReadFileAsBytes(file_path));
        REQUIRE(content.IsEmpty());
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Read file with content")
    {
        StringUtf8 file_path = Paths::Combine(path, "test-read.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        WriteDataToFile(file_path, expected, sizeof(expected));

        DynamicArray<u8> content;
        REQUIRE_NOTHROW(content = ReadFileAsBytes(file_path));
        REQUIRE(content.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(content[i] == expected[i]);
        }
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Read file with binary data")
    {
        StringUtf8 file_path = Paths::Combine(path, "test-binary.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x00, 0xFF, 0x01, 0xFE, 0x80};
        WriteDataToFile(file_path, expected, sizeof(expected));

        DynamicArray<u8> content;
        REQUIRE_NOTHROW(content = ReadFileAsBytes(file_path));
        REQUIRE(content.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(content[i] == expected[i]);
        }
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

TEST_CASE("WriteStringToFile", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Write to non-existent directory")
    {
        StringUtf8 file_path = Paths::Combine(path, "no-such-dir", "file.txt");
        REQUIRE_THROWS_AS(WriteStringToFile(file_path, StringUtf8("data")), PathNotFoundException);
    }
    SECTION("Write creates new file")
    {
        StringUtf8 file_path = Paths::Combine(path, "write-new.txt");
        REQUIRE(!Exists(file_path));

        StringUtf8 content("Hello, Opal!");
        REQUIRE_NOTHROW(WriteStringToFile(file_path, content));
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back == content);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Write overwrites existing file")
    {
        StringUtf8 file_path = Paths::Combine(path, "write-overwrite.txt");

        REQUIRE_NOTHROW(WriteStringToFile(file_path, StringUtf8("original content")));
        REQUIRE_NOTHROW(WriteStringToFile(file_path, StringUtf8("new")));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back == "new");
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Write empty string")
    {
        StringUtf8 file_path = Paths::Combine(path, "write-empty.txt");

        REQUIRE_NOTHROW(WriteStringToFile(file_path, StringUtf8()));
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back.IsEmpty());
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

TEST_CASE("WriteBytesToFile", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Write to non-existent directory")
    {
        StringUtf8 file_path = Paths::Combine(path, "no-such-dir", "file.bin");
        const u8 data[] = {0x01};
        REQUIRE_THROWS_AS(WriteBytesToFile(file_path, ArrayView<const u8>(data)), PathNotFoundException);
    }
    SECTION("Write and read back bytes")
    {
        StringUtf8 file_path = Paths::Combine(path, "write-bytes.bin");
        REQUIRE(!Exists(file_path));

        const u8 expected[] = {0x00, 0xFF, 0x42, 0x80};
        REQUIRE_NOTHROW(WriteBytesToFile(file_path, ArrayView<const u8>(expected)));

        DynamicArray<u8> read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsBytes(file_path));
        REQUIRE(read_back.GetSize() == sizeof(expected));
        for (u64 i = 0; i < sizeof(expected); ++i)
        {
            REQUIRE(read_back[i] == expected[i]);
        }
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Write overwrites existing bytes")
    {
        StringUtf8 file_path = Paths::Combine(path, "write-bytes-over.bin");

        const u8 original[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        REQUIRE_NOTHROW(WriteBytesToFile(file_path, ArrayView<const u8>(original)));

        const u8 replacement[] = {0xAA, 0xBB};
        REQUIRE_NOTHROW(WriteBytesToFile(file_path, ArrayView<const u8>(replacement)));

        DynamicArray<u8> read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsBytes(file_path));
        REQUIRE(read_back.GetSize() == sizeof(replacement));
        REQUIRE(read_back[0] == 0xAA);
        REQUIRE(read_back[1] == 0xBB);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

TEST_CASE("AppendStringToFile", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Append to non-existent directory")
    {
        StringUtf8 file_path = Paths::Combine(path, "no-such-dir", "file.txt");
        REQUIRE_THROWS_AS(AppendStringToFile(file_path, StringUtf8("data")), PathNotFoundException);
    }
    SECTION("Append creates new file")
    {
        StringUtf8 file_path = Paths::Combine(path, "append-new.txt");
        REQUIRE(!Exists(file_path));

        REQUIRE_NOTHROW(AppendStringToFile(file_path, StringUtf8("Hello")));
        REQUIRE(Exists(file_path));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back == "Hello");
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Append adds to existing content")
    {
        StringUtf8 file_path = Paths::Combine(path, "append-existing.txt");

        REQUIRE_NOTHROW(WriteStringToFile(file_path, StringUtf8("Hello")));
        REQUIRE_NOTHROW(AppendStringToFile(file_path, StringUtf8(", World!")));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back == "Hello, World!");
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Multiple appends")
    {
        StringUtf8 file_path = Paths::Combine(path, "append-multi.txt");

        REQUIRE_NOTHROW(AppendStringToFile(file_path, StringUtf8("line1\n")));
        REQUIRE_NOTHROW(AppendStringToFile(file_path, StringUtf8("line2\n")));
        REQUIRE_NOTHROW(AppendStringToFile(file_path, StringUtf8("line3\n")));

        StringUtf8 read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsString(file_path));
        REQUIRE(read_back == "line1\nline2\nline3\n");
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

TEST_CASE("AppendBytesToFile", "[FileSystem]")
{
    StringUtf8 path;
    REQUIRE_NOTHROW(path = Paths::GetCurrentWorkingDirectory());

    SECTION("Append to non-existent directory")
    {
        StringUtf8 file_path = Paths::Combine(path, "no-such-dir", "file.bin");
        const u8 data[] = {0x01};
        REQUIRE_THROWS_AS(AppendBytesToFile(file_path, ArrayView<const u8>(data)), PathNotFoundException);
    }
    SECTION("Append creates new file")
    {
        StringUtf8 file_path = Paths::Combine(path, "append-bytes-new.bin");
        REQUIRE(!Exists(file_path));

        const u8 data[] = {0x01, 0x02};
        REQUIRE_NOTHROW(AppendBytesToFile(file_path, ArrayView<const u8>(data)));

        DynamicArray<u8> read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsBytes(file_path));
        REQUIRE(read_back.GetSize() == sizeof(data));
        REQUIRE(read_back[0] == 0x01);
        REQUIRE(read_back[1] == 0x02);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
    SECTION("Append adds to existing bytes")
    {
        StringUtf8 file_path = Paths::Combine(path, "append-bytes-existing.bin");

        const u8 first[] = {0xAA, 0xBB};
        REQUIRE_NOTHROW(WriteBytesToFile(file_path, ArrayView<const u8>(first)));

        const u8 second[] = {0xCC, 0xDD};
        REQUIRE_NOTHROW(AppendBytesToFile(file_path, ArrayView<const u8>(second)));

        DynamicArray<u8> read_back;
        REQUIRE_NOTHROW(read_back = ReadFileAsBytes(file_path));
        REQUIRE(read_back.GetSize() == 4);
        REQUIRE(read_back[0] == 0xAA);
        REQUIRE(read_back[1] == 0xBB);
        REQUIRE(read_back[2] == 0xCC);
        REQUIRE(read_back[3] == 0xDD);
        REQUIRE_NOTHROW(DeleteFile(file_path));
    }
}

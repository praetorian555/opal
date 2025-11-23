#include "opal/file-system.h"

#include "opal/exceptions.h"
#include "opal/paths.h"
#include "opal/container/hash-set.h"
#include "opal/container/hash-map.h"

#include "test-helpers.h"

using namespace Opal;

CATCH_TRANSLATE_EXCEPTION(const Opal::Exception& ex)
{
    return *ex.What();
}

TEST_CASE("Exists", "[FileSystem]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto exists = Exists("a/b/c/d", &allocator);
        REQUIRE(!exists);
    }
    SECTION("Empty path")
    {
        auto exists = Exists("", nullptr);
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
    ErrorCode err = Paths::GetCurrentWorkingDirectory(path);
    REQUIRE(err == Opal::ErrorCode::Success);
    SECTION("Create and delete a file")
    {
        path = Opal::Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_NOTHROW(DeleteFile(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a file if part of the path does not exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir", "example.txt").GetValue();
        REQUIRE(!Exists(path));
        REQUIRE_THROWS_AS(CreateFile(path), Opal::PathNotFoundException);
    }
    SECTION("Try to create a file that already exists")
    {
        path = Paths::Combine(nullptr, path, "example.txt").GetValue();
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
        path = Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Exists(path));
        REQUIRE_THROWS_AS(DeleteFile(path), Opal::PathNotFoundException);
    }
}

TEST_CASE("Creating and destroying directory", "[FileSystem]")
{
    StringUtf8 path;
    ErrorCode err = Paths::GetCurrentWorkingDirectory(path);
    REQUIRE(err == Opal::ErrorCode::Success);
    SECTION("Create and delete a directory")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory if part of the path does not exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        path = Paths::Combine(nullptr, path, "test-dir-2").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_THROWS_AS(CreateDirectory(path), Opal::PathNotFoundException);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory that already exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
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
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_THROWS_AS(DeleteDirectory(path), Opal::PathNotFoundException);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-empty directory")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        const StringUtf8 file_path = Paths::Combine(nullptr, path, "test-file").GetValue();
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
    ErrorCode err = Paths::GetCurrentWorkingDirectory(path);
    REQUIRE(err == Opal::ErrorCode::Success);
    SECTION("Directory doesn't exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_THROWS_AS(children = Opal::CollectDirectoryContents(path), PathNotFoundException);
    }
    SECTION("Path is not to directory")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateFile(path));
        REQUIRE(Opal::Exists(path));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_THROWS_AS(children = Opal::CollectDirectoryContents(path), NotDirectoryException);
        REQUIRE_NOTHROW(DeleteFile(path));
    }
    SECTION("Collect child contents, no recursive search")
    {
        HashSet<StringUtf8> dir_paths;
        HashMap<StringUtf8, bool> path_types;
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        dir_paths.Insert(first_file);
        path_types.insert(std::make_pair(first_file, false));
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        dir_paths.Insert(another_dir);
        path_types.insert(std::make_pair(another_dir, true));
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        dir_paths.Insert(another_file);
        path_types.insert(std::make_pair(another_file, false));
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path));
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types[e.path] == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.erase(e.path);
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
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        dir_paths.Insert(first_file);
        path_types.insert(std::make_pair(first_file, false));
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        dir_paths.Insert(another_dir);
        path_types.insert(std::make_pair(another_dir, true));
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        dir_paths.Insert(another_file);
        path_types.insert(std::make_pair(another_file, false));
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path, {.recursive = true}));
        REQUIRE(children.GetSize() == 3);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types[e.path] == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.erase(e.path);
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
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        dir_paths.Insert(first_file);
        path_types.insert(std::make_pair(first_file, false));
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        dir_paths.Insert(another_dir);
        path_types.insert(std::make_pair(another_dir, true));
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        dir_paths.Insert(another_file);
        path_types.insert(std::make_pair(another_file, false));
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path, {.include_directories = false, .recursive = true}));
        REQUIRE(children.GetSize() == 2);
        for (const DirectoryEntry& e : children)
        {
            REQUIRE(dir_paths.Contains(e.path));
            REQUIRE(path_types[e.path] == e.is_directory);
            dir_paths.Erase(e.path);
            path_types.erase(e.path);
        }
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
}

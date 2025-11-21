#include "opal/file-system.h"

#include "opal/defines.h"
#include "opal/exceptions.h"
#include "opal/paths.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

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
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path));
        REQUIRE(children.GetSize() == 2);
        REQUIRE(children[0].path == another_dir);
        REQUIRE(children[0].is_directory == true);
        REQUIRE(children[1].path == first_file);
        REQUIRE(children[1].is_directory == false);
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path, {.recursive = true}));
        REQUIRE(children.GetSize() == 3);
        REQUIRE(children[0].path == another_dir);
        REQUIRE(children[0].is_directory == true);
        REQUIRE(children[1].path == first_file);
        REQUIRE(children[1].is_directory == false);
        REQUIRE(children[2].path == another_file);
        REQUIRE(children[2].is_directory == false);
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Collect child contents, do recursive search, ignore directories")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        REQUIRE_NOTHROW(CreateDirectory(path));
        REQUIRE(Opal::Exists(path));
        StringUtf8 first_file = Paths::Combine(nullptr, path, "test-file").GetValue();
        REQUIRE(!Opal::Exists(first_file));
        REQUIRE_NOTHROW(CreateFile(first_file));
        StringUtf8 another_dir = Paths::Combine(nullptr, path, "another-dir").GetValue();
        REQUIRE_NOTHROW(CreateDirectory(another_dir));
        REQUIRE(Opal::Exists(another_dir));
        const StringUtf8 another_file = Paths::Combine(nullptr, another_dir, "another-file").GetValue();
        REQUIRE_NOTHROW(CreateFile(another_file));
        REQUIRE(Opal::Exists(another_file));
        DynamicArray<DirectoryEntry> children;
        REQUIRE_NOTHROW(children = Opal::CollectDirectoryContents(path, {.include_directories = false, .recursive = true}));
        REQUIRE(children.GetSize() == 2);
        REQUIRE(children[0].path == first_file);
        REQUIRE(children[0].is_directory == false);
        REQUIRE(children[1].path == another_file);
        REQUIRE(children[1].is_directory == false);
        REQUIRE_NOTHROW(DeleteFile(first_file));
        REQUIRE_NOTHROW(DeleteFile(another_file));
        REQUIRE_NOTHROW(DeleteDirectory(another_dir));
        REQUIRE_NOTHROW(DeleteDirectory(path));
        REQUIRE(!Opal::Exists(path));
    }
}

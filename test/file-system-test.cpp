#include "opal/file-system.h"

#include "opal/defines.h"
#include "opal/exceptions.h"
#include "opal/paths.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

using namespace Opal;

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

TEST_CASE("Creating and destroying directory", "FileSystem")
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
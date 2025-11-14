#include "opal/file-system.h"

#include "opal/defines.h"
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
        err = CreateFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a file if part of the path does not exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir", "example.txt").GetValue();
        REQUIRE(!Exists(path));
        err = CreateFile(path);
        REQUIRE(err == ErrorCode::PathNotFound);
    }
    SECTION("Try to create a file that already exists")
    {
        path = Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Exists(path));
        err = CreateFile(path);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(Exists(path));
        err = CreateFile(path);
        REQUIRE(err == ErrorCode::AlreadyExists);
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existant file")
    {
        path = Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Exists(path));
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::PathNotFound);
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
        err = CreateDirectory(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = DeleteDirectory(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory if part of the path does not exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        path = Paths::Combine(nullptr, path, "test-dir-2").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateDirectory(path);
        REQUIRE(err == ErrorCode::PathNotFound);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to create a directory that already exist")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateDirectory(path);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = CreateDirectory(path);
        REQUIRE(err == ErrorCode::AlreadyExists);
        REQUIRE(Opal::Exists(path));
        err = DeleteDirectory(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-existent directory")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = DeleteDirectory(path);
        REQUIRE(err == ErrorCode::PathNotFound);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to delete non-empty directory")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateDirectory(path);
        REQUIRE(Opal::Exists(path));
        const StringUtf8 file_path = Paths::Combine(nullptr, path, "test-file").GetValue();
        REQUIRE(!Opal::Exists(file_path));
        err = CreateFile(file_path);
        REQUIRE(Opal::Exists(file_path));
        err = DeleteDirectory(path);
        REQUIRE(err == ErrorCode::NotEmpty);
        err = DeleteFile(file_path);
        REQUIRE(err == ErrorCode::Success);
        err = DeleteDirectory(path);
        REQUIRE(err == ErrorCode::Success);
    }
}
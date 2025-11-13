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
    SECTION("Create a file")
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
    SECTION("Try to override default")
    {
        path = Opal::Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = CreateFile(path);
        REQUIRE(err == Opal::ErrorCode::OSFailure);
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("Try to force override")
    {
        path = Opal::Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = CreateFile(path, true);
        REQUIRE(err == Opal::ErrorCode::Success);
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
    }
    SECTION("No scratch memory")
    {
        path = Opal::Paths::Combine(nullptr, path, "example.txt").GetValue();
        REQUIRE(!Opal::Exists(path));
        NullAllocator allocator;
        err = CreateFile(path, false, &allocator);
#if defined(OPAL_PLATFORM_WINDOWS)
        REQUIRE(err != ErrorCode::Success);
#else
        REQUIRE(err == ErrorCode::Success);
        err = DeleteFile(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(!Opal::Exists(path));
#endif
    }
}

TEST_CASE("Creating and destroying directory", "FileSystem")
{
    StringUtf8 path;
    ErrorCode err = Paths::GetCurrentWorkingDirectory(path);
    REQUIRE(err == Opal::ErrorCode::Success);
    SECTION("Create a directory")
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
    SECTION("Delete folder with contents")
    {
        path = Paths::Combine(nullptr, path, "test-dir").GetValue();
        REQUIRE(!Opal::Exists(path));
        err = CreateDirectory(path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        const StringUtf8 file_path = Paths::Combine(nullptr, path, "file.txt").GetValue();
        err = CreateFile(file_path);
        REQUIRE(err == Opal::ErrorCode::Success);
        REQUIRE(Opal::Exists(path));
        err = DeleteDirectory(path);
        REQUIRE(err == Opal::ErrorCode::OSFailure);
        err = DeleteDirectory(path, false);
        REQUIRE(err == Opal::ErrorCode::Success);
    }
}
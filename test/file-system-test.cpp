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
        path += "\\example.txt";
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
        path += "\\example.txt";
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
        path += "\\example.txt";
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
        path += "\\example.txt";
        REQUIRE(!Opal::Exists(path));
        NullAllocator allocator;
        err = CreateFile(path, false, &allocator);
        REQUIRE(err != ErrorCode::Success);
    }
}
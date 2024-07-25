#include "catch2/catch2.hpp"

#include <filesystem>

#include "opal/paths.h"

using namespace Opal;

TEST_CASE("Get current working directory", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto cwd = Paths::GetCurrentWorkingDirectory(&allocator);
        REQUIRE(!cwd.HasValue());
        REQUIRE(cwd.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("All good")
    {
        auto cwd = Paths::GetCurrentWorkingDirectory();
        REQUIRE(cwd.HasValue());
        ;
        REQUIRE(cwd.GetValue().GetSize() > 0);

        auto ref_path = std::filesystem::current_path();
        auto ref_path_str = ref_path.string();
        const StringLocale ref_path_locale(ref_path_str.c_str(), ref_path_str.size());
        StringUtf8 ref_path_utf8(ref_path_locale.GetSize(), 0);
        auto err = Transcode(ref_path_locale, ref_path_utf8);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(cwd.GetValue() == ref_path_utf8);
    }
}

TEST_CASE("Set current working directory", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto set_cwd_err = Paths::SetCurrentWorkingDirectory(u8"a/b/c/d", &allocator);
        REQUIRE(set_cwd_err == ErrorCode::OutOfMemory);
    }
    SECTION("All good")
    {
        auto cwd = Paths::GetCurrentWorkingDirectory();
        REQUIRE(cwd.HasValue());
        REQUIRE(cwd.GetValue().GetSize() > 0);

        auto new_path = cwd.GetValue();
        new_path.Append(u8"\\..");
        auto set_cwd_err = Paths::SetCurrentWorkingDirectory(new_path);
        REQUIRE(set_cwd_err == ErrorCode::Success);

        new_path.Erase(ReverseFind(new_path, u8"\\"));
        new_path.Erase(ReverseFind(new_path, u8"\\"));
        auto new_cwd = Paths::GetCurrentWorkingDirectory();
        REQUIRE(new_cwd.HasValue());
        REQUIRE(new_cwd.GetValue().GetSize() > 0);
        REQUIRE(new_cwd.GetValue() == new_path);
    }
}

TEST_CASE("Normalize path", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto normalized = Paths::NormalizePath(u8"a/b/c/d", &allocator);
        REQUIRE(!normalized.HasValue());
        REQUIRE(normalized.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Empty path")
    {
        auto normalized = Paths::NormalizePath(u8"", nullptr);
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"");
    }
    SECTION("Absolute path root")
    {
        auto normalized = Paths::NormalizePath(u8"/", nullptr);
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"\\");
        normalized = Paths::NormalizePath(u8"C://", nullptr);
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\");
    }
    SECTION("Absolute path with separators")
    {
        auto normalized = Paths::NormalizePath(u8"C:/Users/\\//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\Users\\test");

        normalized = Paths::NormalizePath(u8"/Users/\\//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"\\Users\\test");
    }
    SECTION("With symlinks")
    {
        auto normalized = Paths::NormalizePath(u8"C:/Users/..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\test");

        normalized = Paths::NormalizePath(u8"C:/Users/test/test/../..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\Users\\test");

        normalized = Paths::NormalizePath(u8"C:/Users/a/../b/..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\Users\\test");

        normalized = Paths::NormalizePath(u8"C:/..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == u8"C:\\test");
    }
    SECTION("Relative paths")
    {
        auto normalized = Paths::NormalizePath(u8"test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == Paths::GetCurrentWorkingDirectory().GetValue() + u8"\\test");

        normalized = Paths::NormalizePath(u8"test/test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == Paths::GetCurrentWorkingDirectory().GetValue() + u8"\\test\\test");

        normalized = Paths::NormalizePath(u8"test/test/../..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == Paths::GetCurrentWorkingDirectory().GetValue() + u8"\\test");

        normalized = Paths::NormalizePath(u8"test/a/../b/..//test//");
        REQUIRE(normalized.HasValue());
        REQUIRE(normalized.GetValue() == Paths::GetCurrentWorkingDirectory().GetValue() + u8"\\test\\test");
    }
}

TEST_CASE("Get parent path", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto parent = Paths::GetParentPath(u8"a/b/c/d", &allocator);
        REQUIRE(!parent.HasValue());
        REQUIRE(parent.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Parent of empty path")
    {
        auto parent = Paths::GetParentPath(u8"", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"");
    }
    SECTION("Parent of root")
    {
        auto parent = Paths::GetParentPath(u8"/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"/");

        parent = Paths::GetParentPath(u8"C:/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"C:/");
    }
    SECTION("Parent of a file")
    {
        auto parent = Paths::GetParentPath(u8"C:/Users/test.txt", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"C:/Users");

        parent = Paths::GetParentPath(u8"C:/Users/test", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"C:/Users");

        parent = Paths::GetParentPath(u8"C:/Users/test/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == u8"C:/Users/test");
    }
}

TEST_CASE("Is path absolute", "[Paths]")
{
    SECTION("Empty string")
    {
        REQUIRE(!Paths::IsPathAbsolute(u8""));
    }
    SECTION("Relative paths")
    {
        REQUIRE(!Paths::IsPathAbsolute(u8"test"));
        REQUIRE(!Paths::IsPathAbsolute(u8"test/test"));
        REQUIRE(!Paths::IsPathAbsolute(u8"test/test/../..//test//"));
        REQUIRE(!Paths::IsPathAbsolute(u8"test/a/../b/..//test//"));
    }
    SECTION("Absolute paths")
    {
        REQUIRE(Paths::IsPathAbsolute(u8"/"));
        REQUIRE(Paths::IsPathAbsolute(u8"C:/"));
        REQUIRE(Paths::IsPathAbsolute(u8"C:/Users/test.txt"));
        REQUIRE(Paths::IsPathAbsolute(u8"C:/Users/test"));
        REQUIRE(Paths::IsPathAbsolute(u8"C:/Users/test/"));
    }
}

TEST_CASE("Get file name", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto file_name = Paths::GetFileName(u8"a/b/c/d", &allocator);
        REQUIRE(!file_name.HasValue());
        REQUIRE(file_name.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Relative paths")
    {
        auto file_name = Paths::GetFileName(u8"test");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"test");

        file_name = Paths::GetFileName(u8"test/test2");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"test2");

        file_name = Paths::GetFileName(u8"test/");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"");

        file_name = Paths::GetFileName(u8"test/..");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"..");

        file_name = Paths::GetFileName(u8"test/.");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8".");
    }
    SECTION("Absolute paths")
    {
        auto file_name = Paths::GetFileName(u8"C:/Users/test.txt");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"test.txt");

        file_name = Paths::GetFileName(u8"C:/Users/test");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"test");

        file_name = Paths::GetFileName(u8"C:/Users/test/");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"");

        file_name = Paths::GetFileName(u8"C:/Users/test/..");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8"..");

        file_name = Paths::GetFileName(u8"C:/Users/test/.");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == u8".");
    }
}

TEST_CASE("Get stem", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto stem = Paths::GetStem(u8"a/b/c/d", &allocator);
        REQUIRE(!stem.HasValue());
        REQUIRE(stem.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Normal file names")
    {
        auto stem = Paths::GetStem(u8"test.txt");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"test");

        stem = Paths::GetStem(u8"test");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"test");

        stem = Paths::GetStem(u8"test/test2");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"test2");

        stem = Paths::GetStem(u8"test/");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"");

        stem = Paths::GetStem(u8"test/..");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"..");

        stem = Paths::GetStem(u8"test/..txt");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8".");

        stem = Paths::GetStem(u8"test/.");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8".");

        stem = Paths::GetStem(u8"test/.ignore");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8".ignore");

        stem = Paths::GetStem(u8"test/foo.bar.baz");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == u8"foo.bar");
    }
}

TEST_CASE("Get extension", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        auto extension = Paths::GetExtension(u8"a/b/c/d", &allocator);
        REQUIRE(!extension.HasValue());
        REQUIRE(extension.GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Normal paths")
    {
        auto extension = Paths::GetExtension(u8"test.txt");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8".txt");

        extension = Paths::GetExtension(u8"test");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/test2");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/..");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/..txt");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8".txt");

        extension = Paths::GetExtension(u8"test/.");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/.ignore");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8"");

        extension = Paths::GetExtension(u8"test/foo.bar.baz");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == u8".baz");
    }
}

TEST_CASE("Combining paths", "[Paths]")
{
    auto result = Paths::Combine(nullptr, u8"a", u8"b", u8"c");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == u8"a\\b\\c");

    result = Paths::Combine(nullptr, u8"a", u8"b", u8"c", u8"");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == u8"a\\b\\c\\");

    result = Paths::Combine(nullptr);
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == u8"");

    NullAllocator allocator;
    result = Paths::Combine(&allocator, u8"a", u8"b", u8"c");
    REQUIRE(!result.HasValue());
    REQUIRE(result.GetError() == ErrorCode::OutOfMemory);
}

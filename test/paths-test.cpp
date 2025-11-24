#include <filesystem>

#include "test-helpers.h"

#include "opal/paths.h"

#include <complex>

using namespace Opal;

#if defined(OPAL_PLATFORM_WINDOWS)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

TEST_CASE("Get current working directory", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        StringUtf8 cwd;
        PushDefault pd(&allocator);
        REQUIRE_THROWS_AS(cwd = Paths::GetCurrentWorkingDirectory(), OutOfMemoryException);
    }
    SECTION("All good")
    {
        StringUtf8 cwd;
        REQUIRE_NOTHROW(cwd = Paths::GetCurrentWorkingDirectory());
        REQUIRE(cwd.GetSize() > 0);

        auto ref_path = std::filesystem::current_path();
        auto ref_path_str = ref_path.string();
        const StringLocale ref_path_locale(ref_path_str.c_str(), ref_path_str.size());
        StringUtf8 ref_path_utf8(ref_path_locale.GetSize(), 0);
        ErrorCode err = Transcode(ref_path_locale, ref_path_utf8);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(cwd == ref_path_utf8);
    }
}

TEST_CASE("Set current working directory", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE_THROWS_AS(Paths::SetCurrentWorkingDirectory("a/b/c/d"), OutOfMemoryException);
    }
    SECTION("All good")
    {
        StringUtf8 cwd;
        REQUIRE_NOTHROW(cwd = Paths::GetCurrentWorkingDirectory());
        REQUIRE(cwd.GetSize() > 0);

        auto new_path = cwd;
        new_path.Append("\\..");
        REQUIRE_NOTHROW(Paths::SetCurrentWorkingDirectory(new_path));

        new_path.Erase(ReverseFind(new_path, "\\"));
        new_path.Erase(ReverseFind(new_path, PATH_SEPARATOR));
        StringUtf8 new_cwd;
        REQUIRE_NOTHROW(new_cwd = Paths::GetCurrentWorkingDirectory());
        REQUIRE(new_cwd.GetSize() > 0);
        REQUIRE(new_cwd == new_path);
    }
}

TEST_CASE("Normalize path", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        StringUtf8 normalized;
        REQUIRE_THROWS_AS(normalized = Paths::NormalizePath("a/b/c/d"), OutOfMemoryException);
    }
    SECTION("Empty path")
    {
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath(""));
        REQUIRE(normalized == "");
    }
    SECTION("Absolute path root")
    {
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/"));
        REQUIRE(normalized == PATH_SEPARATOR);
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("\\"));
        REQUIRE(normalized == PATH_SEPARATOR);
#if defined(OPAL_PLATFORM_WINDOWS)
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C://"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR);
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:\\\\"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR);
#endif
    }
    SECTION("Absolute path with separators")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:/Users/\\//test//"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/Users/\\//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");
#else
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/Users/\\//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");
#endif
    }
    SECTION("With symlinks")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:/Users/..//test//"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:/Users/test/test/../..//test//"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:/Users/a/../b/..//test//"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("C:/..//test//"));
        REQUIRE(normalized == "C:" PATH_SEPARATOR "test");
#else
        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/Users/..//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/Users/test/test/../..//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/Users/a/../b/..//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("/..//test//"));
        REQUIRE(normalized == PATH_SEPARATOR "test");
#endif
    }
    SECTION("Relative paths")
    {
        StringUtf8 cwd;
        REQUIRE_NOTHROW(cwd = Paths::GetCurrentWorkingDirectory());

        StringUtf8 normalized;
        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test//"));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test/test//"));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test" + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test/test/../..//test//"));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test/a/../b/..//test//"));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test" + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test/."));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("test/./"));
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        REQUIRE_NOTHROW(normalized = Paths::NormalizePath("."));
        REQUIRE(normalized == cwd);
    }
}

TEST_CASE("Get parent path", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        Expected<StringUtf8, ErrorCode> parent;
        REQUIRE_THROWS_AS(parent = Paths::GetParentPath("a/b/c/d", &allocator), OutOfMemoryException);
    }
    SECTION("Parent of empty path")
    {
        auto parent = Paths::GetParentPath("", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "");
    }
    SECTION("Parent of root")
    {
        auto parent = Paths::GetParentPath("/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "/");

#if defined(OPAL_PLATFORM_WINDOWS)
        parent = Paths::GetParentPath("C:/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "C:/");
#endif
    }
    SECTION("Parent of a file")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        auto parent = Paths::GetParentPath("C:/Users/test.txt", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "C:/Users");

        parent = Paths::GetParentPath("C:/Users/test", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "C:/Users");

        parent = Paths::GetParentPath("C:/Users/test/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "C:/Users/test");
#else
        auto parent = Paths::GetParentPath("/Users/test.txt", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "/Users");

        parent = Paths::GetParentPath("/Users/test", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "/Users");

        parent = Paths::GetParentPath("/Users/test/", nullptr);
        REQUIRE(parent.HasValue());
        REQUIRE(parent.GetValue() == "/Users/test");
#endif
    }
}

TEST_CASE("Is path absolute", "[Paths]")
{
    SECTION("Empty string")
    {
        REQUIRE(!Paths::IsPathAbsolute(""));
    }
    SECTION("Relative paths")
    {
        REQUIRE(!Paths::IsPathAbsolute("test"));
        REQUIRE(!Paths::IsPathAbsolute("test/test"));
        REQUIRE(!Paths::IsPathAbsolute("test/test/../..//test//"));
        REQUIRE(!Paths::IsPathAbsolute("test/a/../b/..//test//"));
    }
    SECTION("Absolute paths")
    {
        REQUIRE(Paths::IsPathAbsolute("/"));
#if defined(OPAL_PLATFORM_WINDOWS)
        REQUIRE(Paths::IsPathAbsolute("C:/"));
        REQUIRE(Paths::IsPathAbsolute("C:/Users/test.txt"));
        REQUIRE(Paths::IsPathAbsolute("C:/Users/test"));
        REQUIRE(Paths::IsPathAbsolute("C:/Users/test/"));
#endif
    }
}

TEST_CASE("Get file name", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        Expected<StringUtf8, ErrorCode> file_name;
        REQUIRE_THROWS_AS(file_name = Paths::GetFileName("a/b/c/d", &allocator), OutOfMemoryException);
    }
    SECTION("Relative paths")
    {
        auto file_name = Paths::GetFileName("test");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test");

        file_name = Paths::GetFileName("test/test2");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test2");

        file_name = Paths::GetFileName("test/");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "");

        file_name = Paths::GetFileName("test/..");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "..");

        file_name = Paths::GetFileName("test/.");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == ".");
    }
    SECTION("Absolute paths")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        auto file_name = Paths::GetFileName("C:/Users/test.txt");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test.txt");

        file_name = Paths::GetFileName("C:/Users/test");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test");

        file_name = Paths::GetFileName("C:/Users/test/");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "");

        file_name = Paths::GetFileName("C:/Users/test/..");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "..");

        file_name = Paths::GetFileName("C:/Users/test/.");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == ".");
#else
        auto file_name = Paths::GetFileName("/Users/test.txt");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test.txt");

        file_name = Paths::GetFileName("/Users/test");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "test");

        file_name = Paths::GetFileName("/Users/test/");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "");

        file_name = Paths::GetFileName("/Users/test/..");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == "..");

        file_name = Paths::GetFileName("/Users/test/.");
        REQUIRE(file_name.HasValue());
        REQUIRE(file_name.GetValue() == ".");
#endif
    }
}

TEST_CASE("Get stem", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        Expected<StringUtf8, ErrorCode> stem;
        REQUIRE_THROWS_AS(stem = Paths::GetStem("a/b/c/d", &allocator), OutOfMemoryException);
    }
    SECTION("Normal file names")
    {
        auto stem = Paths::GetStem("test.txt");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "test");

        stem = Paths::GetStem("test");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "test");

        stem = Paths::GetStem("test/test2");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "test2");

        stem = Paths::GetStem("test/");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "");

        stem = Paths::GetStem("test/..");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "..");

        stem = Paths::GetStem("test/..txt");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == ".");

        stem = Paths::GetStem("test/.");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == ".");

        stem = Paths::GetStem("test/.ignore");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == ".ignore");

        stem = Paths::GetStem("test/foo.bar.baz");
        REQUIRE(stem.HasValue());
        REQUIRE(stem.GetValue() == "foo.bar");
    }
}

TEST_CASE("Get extension", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        Expected<StringUtf8, ErrorCode> extension;
        REQUIRE_THROWS_AS(extension = Paths::GetExtension("a/b/c/d", &allocator), OutOfMemoryException);
    }
    SECTION("Normal paths")
    {
        auto extension = Paths::GetExtension("test.txt");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == ".txt");

        extension = Paths::GetExtension("test");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/test2");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/..");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/..txt");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == ".txt");

        extension = Paths::GetExtension("test/.");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/.ignore");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == "");

        extension = Paths::GetExtension("test/foo.bar.baz");
        REQUIRE(extension.HasValue());
        REQUIRE(extension.GetValue() == ".baz");
    }
}

TEST_CASE("Combining paths", "[Paths]")
{
    auto result = Paths::Combine(nullptr, "a", "b", "c");
    REQUIRE(result.HasValue());
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(result.GetValue() == "a\\b\\c");
#else
    REQUIRE(result.GetValue() == "a/b/c");
#endif

    result = Paths::Combine(nullptr, "a", "b", "c", "");
    REQUIRE(result.HasValue());
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(result.GetValue() == "a\\b\\c\\");
#else
    REQUIRE(result.GetValue() == "a/b/c/");
#endif

    result = Paths::Combine(nullptr);
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "");

    NullAllocator allocator;
    REQUIRE_THROWS_AS(result = Paths::Combine(&allocator, "a", "b", "c"), OutOfMemoryException);
}

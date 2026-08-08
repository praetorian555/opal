#include <filesystem>

#include "test-helpers.h"

#include "opal/paths.h"

using namespace Opal;

#if defined(OPAL_PLATFORM_WINDOWS)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

namespace
{
// The sections below that are not about failure unwrap through these; the failure sections assert on the code.
StringUtf8 CwdOrFail()
{
    Expected<StringUtf8, ErrorCode> result = Paths::GetCurrentWorkingDirectory();
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}

StringUtf8 NormalizeOrFail(const StringUtf8& path)
{
    Expected<StringUtf8, ErrorCode> result = Paths::NormalizePath(path);
    REQUIRE(result.HasValue());
    return std::move(result).GetValue();
}
}  // namespace

TEST_CASE("Get current working directory", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        PushDefault pd(&allocator);
        REQUIRE(Paths::GetCurrentWorkingDirectory().GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("All good")
    {
        StringUtf8 cwd;
        cwd = CwdOrFail();
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
        REQUIRE(Paths::SetCurrentWorkingDirectory("a/b/c/d") == ErrorCode::OutOfMemory);
    }
    SECTION("All good")
    {
        StringUtf8 cwd;
        cwd = CwdOrFail();
        REQUIRE(cwd.GetSize() > 0);

        auto new_path = std::move(cwd);
        new_path.Append("\\..");
        REQUIRE(Paths::SetCurrentWorkingDirectory(new_path) == ErrorCode::Success);

        new_path.Erase(ReverseFind(new_path, "\\"));
        new_path.Erase(ReverseFind(new_path, PATH_SEPARATOR));
        StringUtf8 new_cwd;
        new_cwd = CwdOrFail();
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
        REQUIRE(Paths::NormalizePath("a/b/c/d").GetError() == ErrorCode::OutOfMemory);
    }
    SECTION("Empty path")
    {
        StringUtf8 normalized;
        normalized = NormalizeOrFail("");
        REQUIRE(normalized == "");
    }
    SECTION("Absolute path root")
    {
        StringUtf8 normalized;
        normalized = NormalizeOrFail("/");
        REQUIRE(normalized == PATH_SEPARATOR);
        normalized = NormalizeOrFail("\\");
        REQUIRE(normalized == PATH_SEPARATOR);
#if defined(OPAL_PLATFORM_WINDOWS)
        normalized = NormalizeOrFail("C://");
        REQUIRE(normalized == "C:" PATH_SEPARATOR);
        normalized = NormalizeOrFail("C:\\\\");
        REQUIRE(normalized == "C:" PATH_SEPARATOR);
#endif
    }
    SECTION("Absolute path with separators")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        StringUtf8 normalized;
        normalized = NormalizeOrFail("C:/Users/\\//test//");
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("/Users/\\//test//");
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");
#else
        StringUtf8 normalized;
        normalized = NormalizeOrFail("/Users/\\//test//");
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");
#endif
    }
    SECTION("With symlinks")
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        StringUtf8 normalized;
        normalized = NormalizeOrFail("C:/Users/..//test//");
        REQUIRE(normalized == "C:" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("C:/Users/test/test/../..//test//");
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("C:/Users/a/../b/..//test//");
        REQUIRE(normalized == "C:" PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("C:/..//test//");
        REQUIRE(normalized == "C:" PATH_SEPARATOR "test");
#else
        StringUtf8 normalized;
        normalized = NormalizeOrFail("/Users/..//test//");
        REQUIRE(normalized == PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("/Users/test/test/../..//test//");
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("/Users/a/../b/..//test//");
        REQUIRE(normalized == PATH_SEPARATOR "Users" PATH_SEPARATOR "test");

        normalized = NormalizeOrFail("/..//test//");
        REQUIRE(normalized == PATH_SEPARATOR "test");
#endif
    }
    SECTION("Relative paths")
    {
        StringUtf8 cwd;
        cwd = CwdOrFail();

        StringUtf8 normalized;
        normalized = NormalizeOrFail("test//");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail("test/test//");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test" + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail("test/test/../..//test//");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail("test/a/../b/..//test//");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test" + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail("test/.");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail("test/./");
        REQUIRE(normalized == cwd + PATH_SEPARATOR + "test");

        normalized = NormalizeOrFail(".");
        REQUIRE(normalized == cwd);
    }
}

TEST_CASE("Get parent path", "[Paths]")
{
    SECTION("No memory")
    {
        NullAllocator allocator;
        Expected<StringUtf8, ErrorCode> parent = Paths::GetParentPath("aaaaaaaaaaaa/bbbbbbbbbbbb/cccccccccccc/d", &allocator);
        REQUIRE(!parent.HasValue());
        REQUIRE(parent.GetError() == ErrorCode::OutOfMemory);
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
        Expected<StringUtf8, ErrorCode> file_name = Paths::GetFileName("a/bbbbbbbbbbbbbbbbbbbbbbbbbb", &allocator);
        REQUIRE(!file_name.HasValue());
        REQUIRE(file_name.GetError() == ErrorCode::OutOfMemory);
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
        Expected<StringUtf8, ErrorCode> stem = Paths::GetStem("a/bbbbbbbbbbbbbbbbbbbbbbbbbb", &allocator);
        REQUIRE(!stem.HasValue());
        REQUIRE(stem.GetError() == ErrorCode::OutOfMemory);
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
        Expected<StringUtf8, ErrorCode> extension = Paths::GetExtension("a/bbbbbbbbbbbbbbbbbbbbbbbbbb", &allocator);
        REQUIRE(!extension.HasValue());
        REQUIRE(extension.GetError() == ErrorCode::OutOfMemory);
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
    auto result = Paths::Combine("a", "b", "c");
    REQUIRE(result.HasValue());
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(result.GetValue() == "a\\b\\c");
#else
    REQUIRE(result.GetValue() == "a/b/c");
#endif

    result = Paths::Combine("a", "b", "c", "");
    REQUIRE(result.HasValue());
#if defined(OPAL_PLATFORM_WINDOWS)
    REQUIRE(result.GetValue() == "a\\b\\c\\");
#else
    REQUIRE(result.GetValue() == "a/b/c/");
#endif

    result = Paths::Combine();
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "");

    result = Paths::Combine("a/", "b");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "a/b");

    result = Paths::Combine("a\\", "b");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "a\\b");

    result = Paths::Combine("a/", "b\\", "c");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "a/b\\c");

    result = Paths::Combine("a/", "");
    REQUIRE(result.HasValue());
    REQUIRE(result.GetValue() == "a/");

    NullAllocator allocator;
    PushDefault pd(&allocator);
    result = Paths::Combine("aaaaaaaaaa", "bbbbbbbbbb", "cccccccccc");
    REQUIRE(!result.HasValue());
    REQUIRE(result.GetError() == ErrorCode::OutOfMemory);
}

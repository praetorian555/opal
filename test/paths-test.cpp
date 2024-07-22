#include "catch2/catch2.hpp"

#include <filesystem>

#include "opal/paths.h"

using namespace Opal;

TEST_CASE("Get current working directory", "[Paths]")
{
    auto cwd = Paths::GetCurrentWorkingDirectory();
    REQUIRE(cwd.HasValue());;
    REQUIRE(cwd.GetValue().GetSize() > 0);

    auto ref_path = std::filesystem::current_path();
    auto ref_path_str = ref_path.string();
    const StringLocale ref_path_locale(ref_path_str.c_str(), ref_path_str.size());
    StringUtf8 ref_path_utf8(ref_path_locale.GetSize(), 0);
    auto err = Transcode(ref_path_locale, ref_path_utf8);
    REQUIRE(err == ErrorCode::Success);
    REQUIRE(cwd.GetValue() == ref_path_utf8);
}

TEST_CASE("Set current working directory", "[Paths]")
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

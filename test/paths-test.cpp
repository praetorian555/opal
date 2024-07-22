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
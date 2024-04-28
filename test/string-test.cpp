#include "catch2/catch2.hpp"

#include <iostream>

#include "opal/container/string.h"

using namespace Opal;

TEST_CASE("From UTF8 to UTF32", "[String]")
{
    char arr[] = "での日本語文字コードを扱うために使用されている従来の";
    (void)arr;
    StringUtf8 utf8(u8"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf8, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to UTF8", "[String]")
{
    StringUtf8 utf8(u8"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf8 utf8_result;
    utf8_result.Resize(200);
    ErrorCode error = Transcode(utf32, utf8_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf8_result == utf8);
}

TEST_CASE("From UTF16 to UTF32", "[String]")
{
    StringUtf16 utf16(u"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf16, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to UTF16", "[String]")
{
    StringUtf16 utf16(u"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf16 utf16_result;
    utf16_result.Resize(200);
    ErrorCode error = Transcode(utf32, utf16_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf16_result == utf16);
}

TEST_CASE("From native wide to UTF32", "[String]")
{
    StringWide utf_wide(L"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf_wide, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to native wide", "[String]")
{
    StringWide wide(L"での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringWide wide_result;
    wide_result.Resize(200);
    ErrorCode error = Transcode(utf32, wide_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(wide_result == wide);
}

TEST_CASE("From locale to UTF32", "[String]")
{
    StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(str_locale, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to locale", "[String]")
{
    StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringLocale locale_result;
    locale_result.Resize(200);
    ErrorCode error = Transcode(utf32, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
    printf("%s\n", str_locale.GetData());
}

TEST_CASE("From wide to locale", "[String]")
{
    StringWide wide(L"での日本語文字コードを扱うために使用されている従来の");
    StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");

    StringLocale locale_result;
    locale_result.Resize(200);
    ErrorCode error = Transcode(wide, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
    printf("%s\n", locale_result.GetData());
}

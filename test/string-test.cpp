#include "catch2/catch2.hpp"

#include <iostream>

#include "opal/container/string.h"

using namespace Opal;

DefaultAllocator g_da;

TEST_CASE("Construction", "[String]")
{
    SECTION("Short string")
    {
        const char ref[] = "Hello there";
        (void)ref;
        SECTION("Default constructor")
        {
            StringUtf8<DefaultAllocator> str(g_da);
            REQUIRE(str.GetSize() == 0);
            REQUIRE(str.GetCapacity() == 0);
            REQUIRE(str.GetData() == nullptr);
        }
        SECTION("Count and value")
        {
            StringUtf8<DefaultAllocator> str(5, 'd', g_da);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 5);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < 5; i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("C array")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            REQUIRE(str.GetSize() == 12);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            StringLocale<DefaultAllocator> str(ref, 5, g_da);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 5);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(str.GetData()[0] == 'H');
            REQUIRE(str.GetData()[1] == 'e');
            REQUIRE(str.GetData()[2] == 'l');
            REQUIRE(str.GetData()[3] == 'l');
            REQUIRE(str.GetData()[4] == 'o');
        }
        SECTION("Copy")
        {
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first);
            REQUIRE(second.GetCapacity() == 12);
            REQUIRE(second.GetSize() == 12);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() == &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy with different allocator")
        {
            DefaultAllocator da2;
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first, da2);
            REQUIRE(second.GetCapacity() == 12);
            REQUIRE(second.GetSize() == 12);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy substring")
        {
            DefaultAllocator da2;
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first, 6, da2);
            REQUIRE(second.GetCapacity() == 6);
            REQUIRE(second.GetSize() == 6);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp("there", second.GetData()) == 0);
        }
    }
    SECTION("Long string")
    {
        const char ref[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        SECTION("Count and value")
        {
            StringUtf8<DefaultAllocator> str(50, 'd', g_da);
            REQUIRE(str.GetSize() == 50);
            REQUIRE(str.GetCapacity() == 50);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < 50; i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("C array")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            REQUIRE(str.GetSize() == 575);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            constexpr u64 k_sub_str_size = 75;
            StringLocale<DefaultAllocator> str(ref, k_sub_str_size, g_da);
            REQUIRE(str.GetSize() == k_sub_str_size);
            REQUIRE(str.GetCapacity() == k_sub_str_size);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < k_sub_str_size; i++)
            {
                REQUIRE(ref[i] == str.GetData()[i]);
            }
        }
        SECTION("Copy")
        {
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first);
            REQUIRE(second.GetCapacity() == 575);
            REQUIRE(second.GetSize() == 575);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() == &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy with different allocator")
        {
            DefaultAllocator da2;
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first, da2);
            REQUIRE(second.GetCapacity() == 575);
            REQUIRE(second.GetSize() == 575);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy substring")
        {
            DefaultAllocator da2;
            StringLocale<DefaultAllocator> first(ref, g_da);
            StringLocale<DefaultAllocator> second(first, 6, da2);
            REQUIRE(second.GetCapacity() == 569);
            REQUIRE(second.GetSize() == 569);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (i32 i = 0; i < 569; i++)
            {
                REQUIRE(ref[6 + i] == second.GetData()[i]);
            }
        }
    }
}

TEST_CASE("Assignment", "[String]")
{
    SECTION("Short string")
    {
        const char ref[] = "Hello there";
        StringLocale<DefaultAllocator> str1(ref, g_da);
        StringLocale<DefaultAllocator> str2(g_da);
        str2 = str1;
        REQUIRE(strcmp(str2.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str2.GetData());
        REQUIRE(str2.GetSize() == 12);
        REQUIRE(str2.GetCapacity() == 12);
        StringLocale<DefaultAllocator> str3(g_da);
        str3 = Move(str1);
        REQUIRE(strcmp(str3.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str3.GetData());
        REQUIRE(str3.GetSize() == 12);
        REQUIRE(str3.GetCapacity() == 12);
        REQUIRE(str1.GetData() == nullptr);
        REQUIRE(str1.GetCapacity() == 0);
        REQUIRE(str1.GetSize() == 0);
    }
    SECTION("Long string")
    {
        const char ref[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        StringLocale<DefaultAllocator> str1(ref, g_da);
        StringLocale<DefaultAllocator> str2(g_da);
        str2 = str1;
        REQUIRE(strcmp(str2.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str2.GetData());
        REQUIRE(str2.GetSize() == 575);
        REQUIRE(str2.GetCapacity() == 575);
        StringLocale<DefaultAllocator> str3(g_da);
        str3 = Move(str1);
        REQUIRE(strcmp(str3.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str3.GetData());
        REQUIRE(str3.GetSize() == 575);
        REQUIRE(str3.GetCapacity() == 575);
        REQUIRE(str1.GetData() == nullptr);
        REQUIRE(str1.GetCapacity() == 0);
        REQUIRE(str1.GetSize() == 0);
    }
}

TEST_CASE("Comparison", "[String]")
{
    SECTION("Short string")
    {
        const char ref[] = "Hello there";
        StringLocale<DefaultAllocator> str1(ref, g_da);
        StringLocale<DefaultAllocator> str2(ref, g_da);
        StringLocale<DefaultAllocator> str3("Test", g_da);
        REQUIRE(str1 == str2);
        REQUIRE(str1 != str3);
    }
    SECTION("Long string")
    {
        const char ref[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        StringLocale<DefaultAllocator> str1(ref, g_da);
        StringLocale<DefaultAllocator> str2(ref, g_da);
        StringLocale<DefaultAllocator> str3("Test", g_da);
        REQUIRE(str1 == str2);
        REQUIRE(str1 != str3);
    }
}

TEST_CASE("Assign", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Count and value smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            str.Assign(5, 'd');
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 5);
            for (i32 i = 0; i < 5; i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("Count and value larger then current capacity")
        {
            StringUtf8<DefaultAllocator> str(g_da);
            str.Assign(5, 'd');
            REQUIRE(str.GetCapacity() == 5);
            REQUIRE(str.GetSize() == 5);
            for (i32 i = 0; i < 5; i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
    }
    SECTION("Long string")
    {

    }
}

TEST_CASE("From UTF8 to UTF32", "[String]")
{
    StringUtf8<DefaultAllocator> utf8(u8"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf32<DefaultAllocator> utf32_result(g_da);
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf8, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to UTF8", "[String]")
{
    StringUtf8<DefaultAllocator> utf8(u8"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf8<DefaultAllocator> utf8_result(g_da);
    utf8_result.Resize(200);
    ErrorCode error = Transcode(utf32, utf8_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf8_result == utf8);
}

TEST_CASE("From UTF16 to UTF32", "[String]")
{
    StringUtf16<DefaultAllocator> utf16(u"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf32<DefaultAllocator> utf32_result(g_da);
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf16, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to UTF16", "[String]")
{
    StringUtf16<DefaultAllocator> utf16(u"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf16<DefaultAllocator> utf16_result(g_da);
    utf16_result.Resize(200);
    ErrorCode error = Transcode(utf32, utf16_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf16_result == utf16);
}

TEST_CASE("From native wide to UTF32", "[String]")
{
    StringWide<DefaultAllocator> utf_wide(L"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf32<DefaultAllocator> utf32_result(g_da);
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf_wide, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to native wide", "[String]")
{
    StringWide<DefaultAllocator> wide(L"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringWide<DefaultAllocator> wide_result(g_da);
    wide_result.Resize(200);
    ErrorCode error = Transcode(utf32, wide_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(wide_result == wide);
}

TEST_CASE("From locale to UTF32", "[String][FromLocale]")
{
    StringLocale<DefaultAllocator> str_locale("での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringUtf32<DefaultAllocator> utf32_result(g_da);
    utf32_result.Resize(200);
    ErrorCode error = Transcode(str_locale, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to locale", "[String]")
{
    StringLocale<DefaultAllocator> str_locale("での日本語文字コードを扱うために使用されている従来の", g_da);
    StringUtf32<DefaultAllocator> utf32(U"での日本語文字コードを扱うために使用されている従来の", g_da);

    StringLocale<DefaultAllocator> locale_result(g_da);
    locale_result.Resize(200);
    ErrorCode error = Transcode(utf32, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
    printf("%s\n", str_locale.GetData());
}

TEST_CASE("From wide to locale", "[String]")
{
    StringWide<DefaultAllocator> wide(L"での日本語文字コードを扱うために使用されている従来の", g_da);
    StringLocale<DefaultAllocator> str_locale("での日本語文字コードを扱うために使用されている従来の", g_da);

    StringLocale<DefaultAllocator> locale_result(g_da);
    locale_result.Resize(200);
    ErrorCode error = Transcode(wide, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
    printf("%s\n", locale_result.GetData());
}

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
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("C array")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            StringLocale<DefaultAllocator> str(ref, 5, g_da);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 6);
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
            REQUIRE(second.GetSize() == 11);
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
            REQUIRE(second.GetSize() == 11);
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
            REQUIRE(second.GetSize() == 5);
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
            REQUIRE(str.GetCapacity() == 51);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("C array")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            constexpr u64 k_sub_str_size = 75;
            StringLocale<DefaultAllocator> str(ref, k_sub_str_size, g_da);
            REQUIRE(str.GetSize() == k_sub_str_size);
            REQUIRE(str.GetCapacity() == k_sub_str_size + 1);
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
            REQUIRE(second.GetSize() == 574);
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
            REQUIRE(second.GetSize() == 574);
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
            REQUIRE(second.GetSize() == 568);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (i32 i = 0; i < second.GetSize(); i++)
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
        REQUIRE(str2.GetSize() == 11);
        REQUIRE(str2.GetCapacity() == 12);
        StringLocale<DefaultAllocator> str3(g_da);
        str3 = Move(str1);
        REQUIRE(strcmp(str3.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str3.GetData());
        REQUIRE(str3.GetSize() == 11);
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
        REQUIRE(str2.GetSize() == 574);
        REQUIRE(str2.GetCapacity() == 575);
        StringLocale<DefaultAllocator> str3(g_da);
        str3 = Move(str1);
        REQUIRE(strcmp(str3.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str3.GetData());
        REQUIRE(str3.GetSize() == 574);
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
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("String literal and count smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Goodbye and get lost", g_da);
            str.Assign(u8"Hello there", 5);
            REQUIRE(str.GetCapacity() == 21);
            REQUIRE(str.GetSize() == 5);
            for (i32 i = 0; i < 5; i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello"[i]);
            }
        }
        SECTION("String literal and count larger then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Other", g_da);
            str.Assign(u8"Hello there", 10);
            REQUIRE(str.GetCapacity() == 11);
            REQUIRE(str.GetSize() == 10);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
        SECTION("Only string literal")
        {
            StringUtf8<DefaultAllocator> str(u8"Other", g_da);
            str.Assign(u8"Hello there");
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
        SECTION("Copy other string")
        {
            StringUtf8<DefaultAllocator> ref(u8"Hello there", g_da);
            StringUtf8<DefaultAllocator> copy(g_da);
            copy.Assign(ref);
            REQUIRE(copy.GetSize() == ref.GetSize());
            REQUIRE(copy.GetCapacity() == ref.GetCapacity());
            for (i32 i = 0; i < ref.GetSize(); i++)
            {
                REQUIRE(copy.GetData()[i] == ref.GetData()[i]);
            }
        }
        SECTION("Copy part of the other string")
        {
            StringUtf8<DefaultAllocator> ref(u8"Hello there", g_da);
            StringUtf8<DefaultAllocator> copy1(g_da);
            copy1.Assign(ref, 6, 10);
            REQUIRE(copy1.GetSize() == 5);
            REQUIRE(copy1.GetCapacity() == 6);
            for (i32 i = 0; i < copy1.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8<DefaultAllocator> copy2(g_da);
            copy2.Assign(ref, 6, 3);
            REQUIRE(copy2.GetSize() == 3);
            REQUIRE(copy2.GetCapacity() == 4);
            for (i32 i = 0; i < copy2.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8<DefaultAllocator> copy3(g_da);
            ErrorCode err = copy3.Assign(ref, 15, 3);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(copy3.GetSize() == 0);
            REQUIRE(copy3.GetCapacity() == 0);
        }
        SECTION("Move string")
        {
            StringUtf8<DefaultAllocator> ref(u8"Hello there", g_da);
            StringUtf8<DefaultAllocator> copy(g_da);
            copy.Assign(Move(ref));
            REQUIRE(copy.GetSize() == 11);
            REQUIRE(copy.GetCapacity() == 12);
            REQUIRE(copy.GetData() != nullptr);
            REQUIRE(ref.GetSize() == 0);
            REQUIRE(ref.GetCapacity() == 0);
            REQUIRE(ref.GetData() == nullptr);
        }
    }
    SECTION("Long string")
    {
        const c8 ref_str[] =
            u8"Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
            u8"dummy "
            u8"text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            u8"has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It "
            u8"was "
            u8"popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with "
            u8"desktop "
            u8"publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        SECTION("Count and value smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(ref_str, g_da);
            str.Assign(50, 'd');
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 50);
            for (i32 i = 0; i < 50; i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("Count and value larger then current capacity")
        {
            StringUtf8<DefaultAllocator> str(g_da);
            str.Assign(50, 'd');
            REQUIRE(str.GetCapacity() == 51);
            REQUIRE(str.GetSize() == 50);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("String literal and count smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(ref_str, g_da);
            str.Assign(ref_str, 50);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 50);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == ref_str[i]);
            }
        }
        SECTION("String literal and count larger then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Other", g_da);
            str.Assign(ref_str, 50);
            REQUIRE(str.GetCapacity() == 51);
            REQUIRE(str.GetSize() == 50);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == ref_str[i]);
            }
        }
        SECTION("Only string literal")
        {
            StringUtf8<DefaultAllocator> str(u8"Other", g_da);
            str.Assign(ref_str);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == ref_str[i]);
            }
        }
        SECTION("Copy other string")
        {
            StringUtf8<DefaultAllocator> ref(ref_str, g_da);
            StringUtf8<DefaultAllocator> copy(g_da);
            copy.Assign(ref);
            REQUIRE(copy.GetSize() == 574);
            REQUIRE(copy.GetCapacity() == 575);
            for (i32 i = 0; i < 575; i++)
            {
                REQUIRE(copy.GetData()[i] == ref.GetData()[i]);
            }
        }
        SECTION("Copy part of the other string")
        {
            StringUtf8<DefaultAllocator> ref(ref_str, g_da);
            StringUtf8<DefaultAllocator> copy1(g_da);
            copy1.Assign(ref, 6, 10);
            REQUIRE(copy1.GetSize() == 10);
            REQUIRE(copy1.GetCapacity() == 11);
            for (i32 i = 0; i < copy1.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8<DefaultAllocator> copy2(g_da);
            copy2.Assign(ref, 6, 3);
            REQUIRE(copy2.GetSize() == 3);
            REQUIRE(copy2.GetCapacity() == 4);
            for (i32 i = 0; i < copy2.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8<DefaultAllocator> copy3(g_da);
            ErrorCode err = copy3.Assign(ref, 600, 3);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(copy3.GetSize() == 0);
            REQUIRE(copy3.GetCapacity() == 0);
        }
        SECTION("Move string")
        {
            StringUtf8<DefaultAllocator> ref(ref_str, g_da);
            StringUtf8<DefaultAllocator> copy(g_da);
            copy.Assign(Move(ref));
            REQUIRE(copy.GetSize() == 574);
            REQUIRE(copy.GetCapacity() == 575);
            REQUIRE(copy.GetData() != nullptr);
            REQUIRE(ref.GetSize() == 0);
            REQUIRE(ref.GetCapacity() == 0);
            REQUIRE(ref.GetData() == nullptr);
        }
    }
}

TEST_CASE("Accessors", "[String]")
{
    SECTION("Short string")
    {
        SECTION("At")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.At(0).GetValue() == 'H');
            REQUIRE(str.At(1).GetValue() == 'e');
            REQUIRE(str.At(2).GetValue() == 'l');
            REQUIRE(str.At(3).GetValue() == 'l');
            REQUIRE(str.At(4).GetValue() == 'o');
            REQUIRE(str.At(5).GetValue() == ' ');
            REQUIRE(str.At(6).GetValue() == 't');
            REQUIRE(str.At(7).GetValue() == 'h');
            REQUIRE(str.At(8).GetValue() == 'e');
            REQUIRE(str.At(9).GetValue() == 'r');
            REQUIRE(str.At(10).GetValue() == 'e');
        }
        SECTION("Out of bounds At")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(!str.At(12).HasValue());
            REQUIRE(str.At(12).GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const At")
        {
            const StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.At(0).GetValue() == 'H');
            REQUIRE(str.At(1).GetValue() == 'e');
            REQUIRE(str.At(2).GetValue() == 'l');
            REQUIRE(str.At(3).GetValue() == 'l');
            REQUIRE(str.At(4).GetValue() == 'o');
            REQUIRE(str.At(5).GetValue() == ' ');
            REQUIRE(str.At(6).GetValue() == 't');
            REQUIRE(str.At(7).GetValue() == 'h');
            REQUIRE(str.At(8).GetValue() == 'e');
            REQUIRE(str.At(9).GetValue() == 'r');
            REQUIRE(str.At(10).GetValue() == 'e');
        }
        SECTION("Out of bounds const At")
        {
            const StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(!str.At(11).HasValue());
            REQUIRE(str.At(11).GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Array operator")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Const array operator")
        {
            const StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Front")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.Front().GetValue() == 'H');
        }
        SECTION("Bad Front")
        {
            StringUtf8<DefaultAllocator> str(g_da);
            REQUIRE(!str.Front().HasValue());
            REQUIRE(str.Front().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const Front")
        {
            const StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.Front().GetValue() == 'H');
        }
        SECTION("Const bad Front")
        {
            const StringUtf8<DefaultAllocator> str(g_da);
            REQUIRE(!str.Front().HasValue());
            REQUIRE(str.Front().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Back")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.Back().GetValue() == 'e');
        }
        SECTION("Bad Back")
        {
            StringUtf8<DefaultAllocator> str(g_da);
            REQUIRE(!str.Back().HasValue());
            REQUIRE(str.Back().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const Back")
        {
            const StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            REQUIRE(str.Back().GetValue() == 'e');
        }
        SECTION("Const bad Back")
        {
            const StringUtf8<DefaultAllocator> str(g_da);
            REQUIRE(!str.Back().HasValue());
            REQUIRE(str.Back().GetError() == ErrorCode::OutOfBounds);
        }
    }
    SECTION("Long string")
    {
        SECTION("At") {}
        SECTION("Out of bounds At") {}
        SECTION("Const At") {}
        SECTION("Out of bounds const At") {}
        SECTION("Array operator") {}
        SECTION("Const array operator") {}
        SECTION("Front") {}
        SECTION("Bad Front") {}
        SECTION("Const Front") {}
        SECTION("Const bad Front") {}
        SECTION("Back") {}
        SECTION("Bad Back") {}
        SECTION("Const Back") {}
        SECTION("Const bad Back") {}
    }
}

TEST_CASE("Reserve", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Reserve larger then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            str.Reserve(20);
            REQUIRE(str.GetCapacity() == 20);
            REQUIRE(str.GetSize() == 11);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
        SECTION("Reserve smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(u8"Hello there", g_da);
            str.Reserve(5);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
    }
    SECTION("Long string")
    {
        const c8 ref_str[] =
            u8"Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
            u8"dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
            u8"book. "
            u8"It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. "
            u8"It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with "
            u8"desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        SECTION("Reserve larger then current capacity")
        {

            StringUtf8<DefaultAllocator> str(ref_str, g_da);
            str.Reserve(600);
            REQUIRE(str.GetCapacity() == 600);
            REQUIRE(str.GetSize() == 574);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == ref_str[i]);
            }
        }
        SECTION("Reserve smaller then current capacity")
        {
            StringUtf8<DefaultAllocator> str(ref_str, g_da);
            str.Reserve(500);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == ref_str[i]);
            }
        }
    }
}

TEST_CASE("Resize", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Resize to zero")
        {
            StringLocale<DefaultAllocator> str("Hello there", g_da);
            str.Resize(0);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 0);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(std::strlen(str.GetData()) == 0);
        }
        SECTION("New size smaller then old")
        {
            StringLocale<DefaultAllocator> str("Hello there", g_da);
            str.Resize(5);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("New size larger then old")
        {
            StringLocale<DefaultAllocator> str("Hello there", g_da);
            str.Resize(20);
            REQUIRE(str.GetCapacity() == 21);
            REQUIRE(str.GetSize() == 20);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("New size larger then old with non-zero value")
        {
            StringLocale<DefaultAllocator> str("Hello there", g_da);
            str.Resize(20, 'd');
            REQUIRE(str.GetCapacity() == 21);
            REQUIRE(str.GetSize() == 20);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(str.GetData(), "Hello thereddddddddd") == 0);
        }
    }
    SECTION("Long string") {}
}

TEST_CASE("Append", "[String]")
{
    SECTION("Short string")
    {
        SECTION("String literal no change to capacity")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            str.Reserve(200);
            str.Append(" there");
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("String literal increase capacity")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            ErrorCode err = str.Append(" there");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("Null string literal")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            ErrorCode err = str.Append(nullptr);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Empty string literal")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            ErrorCode err = str.Append("");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string literal no change to capacity")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            str.Reserve(200);
            str.Append(" there", 5);
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Sub string literal increase capacity")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            ErrorCode err = str.Append(" there", 5);
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 11);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Null sub string literal")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            ErrorCode err = str.Append(nullptr, 5);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Count and value")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            str.Reserve(200);
            str.Append(5, 'd');
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Helloddddd") == 0);
        }
        SECTION("Zero count and value")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            str.Reserve(200);
            str.Append(0, 'd');
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("String")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("Empty string")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            StringLocale<DefaultAllocator> str2("", g_da);
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2, 0, 5);
            REQUIRE(str.GetCapacity() == 11);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Sub string bad position")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            ErrorCode err = str.Append(str2, 6, 5);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string zero count")
        {
            StringLocale<DefaultAllocator> str("Hello", g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2, 0, 0);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
    }
    SECTION("Long string")
    {
        const char ref[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
            "dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        const char ref_final[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
            "dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum. there";
        SECTION("String literal no change to capacity")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            str.Reserve(1000);
            str.Append(" there");
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("String literal increase capacity")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            ErrorCode err = str.Append(" there");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 581);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("Null string literal")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            ErrorCode err = str.Append(nullptr);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Empty string literal")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            ErrorCode err = str.Append("");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string literal no change to capacity")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            str.Reserve(1000);
            str.Append(" there", 5);
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Sub string literal increase capacity")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            ErrorCode err = str.Append(" there", 5);
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 580);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Null sub string literal")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            ErrorCode err = str.Append(nullptr, 5);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Count and value")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            str.Reserve(1000);
            str.Append(5, 'd');
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Zero count and value")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            str.Reserve(1000);
            str.Append(0, 'd');
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("String")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 581);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("Empty string")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            StringLocale<DefaultAllocator> str2("", g_da);
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2, 0, 5);
            REQUIRE(str.GetCapacity() == 580);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Sub string bad position")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            ErrorCode err = str.Append(str2, 6, 5);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string zero count")
        {
            StringLocale<DefaultAllocator> str(ref, g_da);
            StringLocale<DefaultAllocator> str2(" there", g_da);
            str.Append(str2, 0, 0);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
    }
}

TEST_CASE("Iterator", "[String]")
{
    using StringType = StringLocale<DefaultAllocator>;
    using ItType = StringType::IteratorType;
    SECTION("Difference")
    {
        StringType str("Hello there", g_da);
        ItType it1 = str.Begin();
        ItType it2 = str.End();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Increment")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
        ItType prev = it++;
        REQUIRE(it - prev == 1);
    }
    SECTION("Decrement")
    {
        StringType str("Hello there", g_da);
        ItType it = str.End();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
    }
    SECTION("Post decrement")
    {
        StringType str("Hello there", g_da);
        ItType it = str.End();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
        ItType prev = it--;
        REQUIRE(prev - it == 1);
    }
    SECTION("Add")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        ItType it2 = str.Begin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        StringType str("Hello there", g_da);
        ItType it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        StringType str("Hello there", g_da);
        ItType it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Access")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringType str("Hello there", g_da);
        ItType it1 = str.Begin();
        ItType it2 = str.Begin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        StringType str("Hello there", g_da);
        StringType dst(g_da);
        for (ItType it = str.Begin(); it != str.End(); ++it)
        {
            dst.Append(*it);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
    }
    SECTION("Modern for loop")
    {
        StringType str("Hello there", g_da);
        StringType dst(g_da);
        for (auto c : str)
        {
            dst.Append(c);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
    }
}

TEST_CASE("Const iterator", "[String]")
{
    using StringType = StringLocale<DefaultAllocator>;
    using ItType = StringType::ConstIteratorType;
    SECTION("Difference")
    {
        StringType str("Hello there", g_da);
        ItType it1 = str.ConstBegin();
        ItType it2 = str.ConstEnd();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Increment")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
        ItType prev = it++;
        REQUIRE(it - prev == 1);
    }
    SECTION("Decrement")
    {
        StringType str("Hello there", g_da);
        ItType it = str.ConstEnd();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
    }
    SECTION("Post decrement")
    {
        StringType str("Hello there", g_da);
        ItType it = str.ConstEnd();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
        ItType prev = it--;
        REQUIRE(prev - it == 1);
    }
    SECTION("Add")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        ItType it2 = str.ConstBegin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringType str("Hello there", g_da);
        ItType it = str.ConstBegin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Access")
    {
        const StringType str("Hello there", g_da);
        ItType it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringType str("Hello there", g_da);
        ItType it = str.ConstBegin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringType str("Hello there", g_da);
        ItType it1 = str.ConstBegin();
        ItType it2 = str.ConstBegin();
        REQUIRE(it1 == it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it1 >= it2);
        REQUIRE_FALSE(it1 != it2);
        REQUIRE_FALSE(it1 < it2);
        REQUIRE_FALSE(it1 > it2);

        it2++;
        REQUIRE(it1 != it2);
        REQUIRE(it1 < it2);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 > it1);
        REQUIRE(it2 >= it1);
        REQUIRE_FALSE(it1 == it2);
    }
    SECTION("For loop")
    {
        const StringType str("Hello there", g_da);
        StringType dst(g_da);
        for (ItType it = str.Begin(); it != str.End(); ++it)
        {
            dst.Append(*it);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
    }
    SECTION("Modern for loop")
    {
        const StringType str("Hello there", g_da);
        StringType dst(g_da);
        for (auto c : str)
        {
            dst.Append(c);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
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

TEST_CASE("Lexicographical compare", "[String]")
{
    using StringType = StringLocale<DefaultAllocator>;
    SECTION("Two strings")
    {
        SECTION("Both empty")
        {
            StringType a("", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, b) == 0);
        }
        SECTION("First empty")
        {
            StringType a("", g_da);
            StringType b("aa", g_da);
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Second empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("First shorter and equal")
        {
            StringType a("aaa", g_da);
            StringType b("aaabbb", g_da);
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringType a("aaabbb", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringType a("aaa", g_da);
            StringType b("bbbb", g_da);
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("First shorter and larger")
        {
            StringType a("bbb", g_da);
            StringType b("aaaa", g_da);
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringType a("bbbb", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringType a("aaaa", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Equal size and same")
        {
            StringType a("aaa", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, b) == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringType a("aaa", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringType a("ccc", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, b) == 1);
        }
    }
    SECTION("First substring second string")
    {
        SECTION("Both empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, 2, 0, b).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringType a("aa", g_da);
            StringType b("aa", g_da);
            REQUIRE(Compare(a, 2, 0, b).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, 1, 1, b).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringType a("bbbaaa", g_da);
            StringType b("aaabbb", g_da);
            REQUIRE(Compare(a, 3, 3, b).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringType a("cccaaabbb", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, 3, 6, b).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringType a("caaa", g_da);
            StringType b("bbbb", g_da);
            REQUIRE(Compare(a, 1, 3, b).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringType a("ccbbb", g_da);
            StringType b("aaaa", g_da);
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringType a("ccbbbb", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, 2, 4, b).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringType a("caaaa", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, 1, 4, b).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringType a("ccaaa", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringType a("caaa", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, 1, 3, b).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringType a("aaccc", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 1);
        }
    }
    SECTION("Both substrings")
    {
        SECTION("Both empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, 2, 0, b, 0, 0).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringType a("aa", g_da);
            StringType b("aa", g_da);
            REQUIRE(Compare(a, 2, 0, b, 0, 2).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, 1, 1, b, 0, 0).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringType a("bbbaaa", g_da);
            StringType b("aaabbb", g_da);
            REQUIRE(Compare(a, 3, 3, b, 0, 6).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringType a("cccaaabbb", g_da);
            StringType b("caaa", g_da);
            REQUIRE(Compare(a, 3, 6, b, 1, 3).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringType a("caaa", g_da);
            StringType b("abbbb", g_da);
            REQUIRE(Compare(a, 1, 3, b, 1, 4).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringType a("ccbbb", g_da);
            StringType b("aaaaaa", g_da);
            REQUIRE(Compare(a, 2, 3, b, 2, 4).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringType a("ccbbbb", g_da);
            StringType b("aaa", g_da);
            REQUIRE(Compare(a, 2, 4, b, 0, 3).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringType a("caaaa", g_da);
            StringType b("dfdbbb", g_da);
            REQUIRE(Compare(a, 1, 4, b, 3, 3).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringType a("ccaaa", g_da);
            StringType b("baaa", g_da);
            REQUIRE(Compare(a, 2, 3, b, 1, 3).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringType a("caaa", g_da);
            StringType b("bbb", g_da);
            REQUIRE(Compare(a, 1, 3, b, 0, 3).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringType a("aaccc", g_da);
            StringType b("abbb", g_da);
            REQUIRE(Compare(a, 2, 3, b, 1, 3).GetValue() == 1);
        }
    }
    SECTION("First substring second string literal")
    {
        SECTION("Both empty")
        {
            StringType a("aa", g_da);
            REQUIRE(Compare(a, 2, 0, "").GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringType a("aa", g_da);
            REQUIRE(Compare(a, 2, 0, "aa").GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringType a("aa", g_da);
            REQUIRE(Compare(a, 1, 1, "").GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringType a("bbbaaa", g_da);
            REQUIRE(Compare(a, 3, 3, "aaabbb").GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringType a("cccaaabbb", g_da);
            REQUIRE(Compare(a, 3, 6, "aaa").GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringType a("caaa", g_da);
            REQUIRE(Compare(a, 1, 3, "bbbb").GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringType a("ccbbb", g_da);
            REQUIRE(Compare(a, 2, 3, "aaaa").GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringType a("ccbbbb", g_da);
            REQUIRE(Compare(a, 2, 4, "aaa").GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringType a("caaaa", g_da);
            REQUIRE(Compare(a, 1, 4, "bbb").GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringType a("ccaaa", g_da);
            REQUIRE(Compare(a, 2, 3, "aaa").GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringType a("caaa", g_da);
            REQUIRE(Compare(a, 1, 3, "bbb").GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringType a("aaccc", g_da);
            REQUIRE(Compare(a, 2, 3, "bbb").GetValue() == 1);
        }
    }
    SECTION("First substring second substring literal")
    {
        SECTION("Both empty")
        {
            StringType a("aa", g_da);
            StringType b("", g_da);
            REQUIRE(Compare(a, 2, 0, "a",  0).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringType a("aa", g_da);
            StringType b("aa", g_da);
            REQUIRE(Compare(a, 2, 0, "aab", 2).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringType a("aa", g_da);
            REQUIRE(Compare(a, 1, 1, "", 0).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringType a("bbbaaa", g_da);
            REQUIRE(Compare(a, 3, 3, "aaabbba", 6).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringType a("cccaaabbb", g_da);
            REQUIRE(Compare(a, 3, 6, "aaac", 3).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringType a("caaa", g_da);
            REQUIRE(Compare(a, 1, 3, "bbbba", 4).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringType a("ccbbb", g_da);
            REQUIRE(Compare(a, 2, 3, "aaaabb", 4).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringType a("ccbbbb", g_da);
            REQUIRE(Compare(a, 2, 4, "aaa", 3).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringType a("caaaa", g_da);
            REQUIRE(Compare(a, 1, 4, "bbbdfd", 3).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringType a("ccaaa", g_da);
            REQUIRE(Compare(a, 2, 3, "aaaab", 3).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringType a("caaa", g_da);
            REQUIRE(Compare(a, 1, 3, "bbb", 3).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringType a("aaccc", g_da);
            REQUIRE(Compare(a, 2, 3, "bbb", 3).GetValue() == 1);
        }
    }
}

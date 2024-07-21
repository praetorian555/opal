#include "catch2/catch2.hpp"

#include <iostream>

#include "opal/container/string.h"

using namespace Opal;

TEST_CASE("Construction", "[String]")
{
    SECTION("Short string")
    {
        const char ref[] = "Hello there";
        (void)ref;
        SECTION("Default constructor")
        {
            StringUtf8 str;
            REQUIRE(str.GetSize() == 0);
            REQUIRE(str.GetCapacity() == 0);
            REQUIRE(str.GetData() == nullptr);
        }
        SECTION("Default constructor with allocator")
        {
            DefaultAllocator da;
            StringUtf8 str(&da);
            REQUIRE(str.GetSize() == 0);
            REQUIRE(str.GetCapacity() == 0);
            REQUIRE(str.GetData() == nullptr);
        }
        SECTION("Count and value")
        {
            StringUtf8 str(5, 'd');
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("Count and value with allocator")
        {
            DefaultAllocator da;
            StringUtf8 str(5, 'd', &da);
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
            StringLocale str(ref);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("C array with allocator")
        {
            DefaultAllocator da;
            StringLocale str(ref, &da);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            StringLocale str(ref, 5);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(str.GetData()[0] == 'H');
            REQUIRE(str.GetData()[1] == 'e');
            REQUIRE(str.GetData()[2] == 'l');
            REQUIRE(str.GetData()[3] == 'l');
            REQUIRE(str.GetData()[4] == 'o');
        }
        SECTION("Substring of a string literal with allocator")
        {
            DefaultAllocator da;
            StringLocale str(ref, 5, &da);
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
            StringLocale first(ref);
            StringLocale second(first);
            REQUIRE(second.GetCapacity() == 12);
            REQUIRE(second.GetSize() == 11);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() == &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy with allocator")
        {
            DefaultAllocator da;
            StringLocale first(ref);
            StringLocale second(first, &da);
            REQUIRE(second.GetCapacity() == 12);
            REQUIRE(second.GetSize() == 11);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy substring")
        {
            DefaultAllocator da;
            StringLocale first(ref);
            StringLocale second(first, 6, &da);
            REQUIRE(second.GetCapacity() == 6);
            REQUIRE(second.GetSize() == 5);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp("there", second.GetData()) == 0);
        }
        SECTION("Copy substring with allocator")
        {
            DefaultAllocator da1;
            DefaultAllocator da2;
            StringLocale first(ref);
            StringLocale second(first, 6, &da2);
            REQUIRE(second.GetCapacity() == 6);
            REQUIRE(second.GetSize() == 5);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp("there", second.GetData()) == 0);
        }
        SECTION("Move")
        {
            StringLocale first(ref);
            StringLocale second(Move(first));
            REQUIRE(second.GetCapacity() == 12);
            REQUIRE(second.GetSize() == 11);
            REQUIRE(second.GetData() != nullptr);
            REQUIRE(first.GetCapacity() == 0);
            REQUIRE(first.GetSize() == 0);
            REQUIRE(first.GetData() == nullptr);
            REQUIRE(strcmp(ref, second.GetData()) == 0);
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
            StringUtf8 str(50, 'd');
            REQUIRE(str.GetSize() == 50);
            REQUIRE(str.GetCapacity() == 51);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("Count and value with iterator")
        {
            StringUtf8 str(50, 'd');
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
            StringLocale str(ref);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("C array with allocator")
        {
            DefaultAllocator da;
            StringLocale str(ref, &da);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(ref, str.GetData()) == 0);
        }
        SECTION("Substring of a string literal")
        {
            constexpr u64 k_sub_str_size = 75;
            StringLocale str(ref, k_sub_str_size);
            REQUIRE(str.GetSize() == k_sub_str_size);
            REQUIRE(str.GetCapacity() == k_sub_str_size + 1);
            REQUIRE(str.GetData() != nullptr);
            for (i32 i = 0; i < k_sub_str_size; i++)
            {
                REQUIRE(ref[i] == str.GetData()[i]);
            }
        }
        SECTION("Substring of a string literal with allocator")
        {
            DefaultAllocator da;
            constexpr u64 k_sub_str_size = 75;
            StringLocale str(ref, k_sub_str_size, &da);
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
            StringLocale first(ref);
            StringLocale second(first);
            REQUIRE(second.GetCapacity() == 575);
            REQUIRE(second.GetSize() == 574);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() == &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy with allocator")
        {
            DefaultAllocator da;
            StringLocale first(ref);
            StringLocale second(first, &da);
            REQUIRE(second.GetCapacity() == 575);
            REQUIRE(second.GetSize() == 574);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
        SECTION("Copy substring")
        {
            DefaultAllocator da;
            StringLocale first(ref);
            StringLocale second(first, 6, &da);
            REQUIRE(second.GetCapacity() == 569);
            REQUIRE(second.GetSize() == 568);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (i32 i = 0; i < second.GetSize(); i++)
            {
                REQUIRE(ref[6 + i] == second.GetData()[i]);
            }
        }
        SECTION("Copy substring with allocator")
        {
            DefaultAllocator da;
            StringLocale first(ref);
            StringLocale second(first, 6, &da);
            REQUIRE(second.GetCapacity() == 569);
            REQUIRE(second.GetSize() == 568);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (i32 i = 0; i < second.GetSize(); i++)
            {
                REQUIRE(ref[6 + i] == second.GetData()[i]);
            }
        }
        SECTION("Move")
        {
            StringLocale first(ref);
            StringLocale second(Move(first));
            REQUIRE(second.GetCapacity() == 575);
            REQUIRE(second.GetSize() == 574);
            REQUIRE(second.GetData() != nullptr);
            REQUIRE(first.GetCapacity() == 0);
            REQUIRE(first.GetSize() == 0);
            REQUIRE(first.GetData() == nullptr);
            REQUIRE(strcmp(ref, second.GetData()) == 0);
        }
    }
}

TEST_CASE("Assignment", "[String]")
{
    SECTION("Short string")
    {
        const char ref[] = "Hello there";
        StringLocale str1(ref);
        StringLocale str2;
        str2 = str1;
        REQUIRE(strcmp(str2.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str2.GetData());
        REQUIRE(str2.GetSize() == 11);
        REQUIRE(str2.GetCapacity() == 12);
        StringLocale str3;
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
        StringLocale str1(ref);
        StringLocale str2;
        str2 = str1;
        REQUIRE(strcmp(str2.GetData(), ref) == 0);
        REQUIRE(str1.GetData() != str2.GetData());
        REQUIRE(str2.GetSize() == 574);
        REQUIRE(str2.GetCapacity() == 575);
        StringLocale str3;
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
        StringLocale str1(ref);
        StringLocale str2(ref);
        StringLocale str3("Test");
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
        StringLocale str1(ref);
        StringLocale str2(ref);
        StringLocale str3("Test");
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
            StringUtf8 str(u8"Hello there");
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
            StringUtf8 str;
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
            StringUtf8 str(u8"Goodbye and get lost");
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
            StringUtf8 str(u8"Other");
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
            StringUtf8 str(u8"Other");
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
            StringUtf8 ref(u8"Hello there");
            StringUtf8 copy;
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
            StringUtf8 ref(u8"Hello there");
            StringUtf8 copy1;
            copy1.Assign(ref, 6, 10);
            REQUIRE(copy1.GetSize() == 5);
            REQUIRE(copy1.GetCapacity() == 6);
            for (i32 i = 0; i < copy1.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8 copy2;
            copy2.Assign(ref, 6, 3);
            REQUIRE(copy2.GetSize() == 3);
            REQUIRE(copy2.GetCapacity() == 4);
            for (i32 i = 0; i < copy2.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8 copy3;
            ErrorCode err = copy3.Assign(ref, 15, 3);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(copy3.GetSize() == 0);
            REQUIRE(copy3.GetCapacity() == 0);
        }
        SECTION("Move string")
        {
            StringUtf8 ref(u8"Hello there");
            StringUtf8 copy;
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
            StringUtf8 str(ref_str);
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
            StringUtf8 str;
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
            StringUtf8 str(ref_str);
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
            StringUtf8 str(u8"Other");
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
            StringUtf8 str(u8"Other");
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
            StringUtf8 ref(ref_str);
            StringUtf8 copy;
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
            StringUtf8 ref(ref_str);
            StringUtf8 copy1;
            copy1.Assign(ref, 6, 10);
            REQUIRE(copy1.GetSize() == 10);
            REQUIRE(copy1.GetCapacity() == 11);
            for (i32 i = 0; i < copy1.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8 copy2;
            copy2.Assign(ref, 6, 3);
            REQUIRE(copy2.GetSize() == 3);
            REQUIRE(copy2.GetCapacity() == 4);
            for (i32 i = 0; i < copy2.GetSize(); i++)
            {
                REQUIRE(copy1.GetData()[i] == ref.GetData()[6 + i]);
            }
            StringUtf8 copy3;
            ErrorCode err = copy3.Assign(ref, 600, 3);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(copy3.GetSize() == 0);
            REQUIRE(copy3.GetCapacity() == 0);
        }
        SECTION("Move string")
        {
            StringUtf8 ref(ref_str);
            StringUtf8 copy;
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
            StringUtf8 str(u8"Hello there");
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
            StringUtf8 str(u8"Hello there");
            REQUIRE(!str.At(12).HasValue());
            REQUIRE(str.At(12).GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const At")
        {
            const StringUtf8 str(u8"Hello there");
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
            const StringUtf8 str(u8"Hello there");
            REQUIRE(!str.At(11).HasValue());
            REQUIRE(str.At(11).GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Array operator")
        {
            StringUtf8 str(u8"Hello there");
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Const array operator")
        {
            const StringUtf8 str(u8"Hello there");
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Front")
        {
            StringUtf8 str(u8"Hello there");
            REQUIRE(str.Front().GetValue() == 'H');
        }
        SECTION("Bad Front")
        {
            StringUtf8 str;
            REQUIRE(!str.Front().HasValue());
            REQUIRE(str.Front().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const Front")
        {
            const StringUtf8 str(u8"Hello there");
            REQUIRE(str.Front().GetValue() == 'H');
        }
        SECTION("Const bad Front")
        {
            const StringUtf8 str;
            REQUIRE(!str.Front().HasValue());
            REQUIRE(str.Front().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Back")
        {
            StringUtf8 str(u8"Hello there");
            REQUIRE(str.Back().GetValue() == 'e');
        }
        SECTION("Bad Back")
        {
            StringUtf8 str;
            REQUIRE(!str.Back().HasValue());
            REQUIRE(str.Back().GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Const Back")
        {
            const StringUtf8 str(u8"Hello there");
            REQUIRE(str.Back().GetValue() == 'e');
        }
        SECTION("Const bad Back")
        {
            const StringUtf8 str;
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
            StringUtf8 str(u8"Hello there");
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
            StringUtf8 str(u8"Hello there");
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

            StringUtf8 str(ref_str);
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
            StringUtf8 str(ref_str);
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
            StringLocale str("Hello there");
            str.Resize(0);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 0);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(std::strlen(str.GetData()) == 0);
        }
        SECTION("New size smaller then old")
        {
            StringLocale str("Hello there");
            str.Resize(5);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("New size larger then old")
        {
            StringLocale str("Hello there");
            str.Resize(20);
            REQUIRE(str.GetCapacity() == 21);
            REQUIRE(str.GetSize() == 20);
            REQUIRE(str.GetData() != nullptr);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("New size larger then old with non-zero value")
        {
            StringLocale str("Hello there");
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
            StringLocale str("Hello");
            str.Reserve(200);
            str.Append(" there");
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("String literal increase capacity")
        {
            StringLocale str("Hello");
            ErrorCode err = str.Append(" there");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("Null string literal")
        {
            StringLocale str("Hello");
            ErrorCode err = str.Append(nullptr);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Empty string literal")
        {
            StringLocale str("Hello");
            ErrorCode err = str.Append("");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string literal no change to capacity")
        {
            StringLocale str("Hello");
            str.Reserve(200);
            str.Append(" there", 5);
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Sub string literal increase capacity")
        {
            StringLocale str("Hello");
            ErrorCode err = str.Append(" there", 5);
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 11);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Null sub string literal")
        {
            StringLocale str("Hello");
            ErrorCode err = str.Append(nullptr, 5);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Count and value")
        {
            StringLocale str("Hello");
            str.Reserve(200);
            str.Append(5, 'd');
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Helloddddd") == 0);
        }
        SECTION("Zero count and value")
        {
            StringLocale str("Hello");
            str.Reserve(200);
            str.Append(0, 'd');
            REQUIRE(str.GetCapacity() == 200);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("String")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
        }
        SECTION("Empty string")
        {
            StringLocale str("Hello");
            StringLocale str2("");
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
            str.Append(str2, 0, 5);
            REQUIRE(str.GetCapacity() == 11);
            REQUIRE(str.GetSize() == 10);
            REQUIRE(strcmp(str.GetData(), "Hello ther") == 0);
        }
        SECTION("Sub string bad position")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
            ErrorCode err = str.Append(str2, 6, 5);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(strcmp(str.GetData(), "Hello") == 0);
        }
        SECTION("Sub string zero count")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
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
            StringLocale str(ref);
            str.Reserve(1000);
            str.Append(" there");
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("String literal increase capacity")
        {
            StringLocale str(ref);
            ErrorCode err = str.Append(" there");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 581);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("Null string literal")
        {
            StringLocale str(ref);
            ErrorCode err = str.Append(nullptr);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Empty string literal")
        {
            StringLocale str(ref);
            ErrorCode err = str.Append("");
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string literal no change to capacity")
        {
            StringLocale str(ref);
            str.Reserve(1000);
            str.Append(" there", 5);
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Sub string literal increase capacity")
        {
            StringLocale str(ref);
            ErrorCode err = str.Append(" there", 5);
            REQUIRE(err == ErrorCode::Success);
            REQUIRE(str.GetCapacity() == 580);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Null sub string literal")
        {
            StringLocale str(ref);
            ErrorCode err = str.Append(nullptr, 5);
            REQUIRE(err == ErrorCode::BadInput);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Count and value")
        {
            StringLocale str(ref);
            str.Reserve(1000);
            str.Append(5, 'd');
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Zero count and value")
        {
            StringLocale str(ref);
            str.Reserve(1000);
            str.Append(0, 'd');
            REQUIRE(str.GetCapacity() == 1000);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("String")
        {
            StringLocale str(ref);
            StringLocale str2(" there");
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 581);
            REQUIRE(str.GetSize() == 580);
            REQUIRE(strcmp(str.GetData(), ref_final) == 0);
        }
        SECTION("Empty string")
        {
            StringLocale str(ref);
            StringLocale str2("");
            str.Append(str2);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string")
        {
            StringLocale str(ref);
            StringLocale str2(" there");
            str.Append(str2, 0, 5);
            REQUIRE(str.GetCapacity() == 580);
            REQUIRE(str.GetSize() == 579);
        }
        SECTION("Sub string bad position")
        {
            StringLocale str(ref);
            StringLocale str2(" there");
            ErrorCode err = str.Append(str2, 6, 5);
            REQUIRE(err == ErrorCode::OutOfBounds);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
        SECTION("Sub string zero count")
        {
            StringLocale str(ref);
            StringLocale str2(" there");
            str.Append(str2, 0, 0);
            REQUIRE(str.GetCapacity() == 575);
            REQUIRE(str.GetSize() == 574);
            REQUIRE(strcmp(str.GetData(), ref) == 0);
        }
    }
}

TEST_CASE("Iterator", "[String]")
{
    using StringLocale = StringLocale;
    using ItType = StringLocale::IteratorType;
    SECTION("Difference")
    {
        StringLocale str("Hello there");
        ItType it1 = str.Begin();
        ItType it2 = str.End();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Increment")
    {
        StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        StringLocale str("Hello there");
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
        StringLocale str("Hello there");
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
        StringLocale str("Hello there");
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
        StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        ItType it2 = str.Begin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        StringLocale str("Hello there");
        ItType it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        StringLocale str("Hello there");
        ItType it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Access")
    {
        StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringLocale str("Hello there");
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
        StringLocale str("Hello there");
        StringLocale dst;
        for (ItType it = str.Begin(); it != str.End(); ++it)
        {
            dst.Append(*it);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
    }
    SECTION("Modern for loop")
    {
        StringLocale str("Hello there");
        StringLocale dst;
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
    using StringLocale = StringLocale;
    using ItType = StringLocale::ConstIteratorType;
    SECTION("Difference")
    {
        StringLocale str("Hello there");
        ItType it1 = str.ConstBegin();
        ItType it2 = str.ConstEnd();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Increment")
    {
        const StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        const StringLocale str("Hello there");
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
        StringLocale str("Hello there");
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
        StringLocale str("Hello there");
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
        const StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        ItType it2 = str.ConstBegin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringLocale str("Hello there");
        ItType it = str.ConstBegin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        const StringLocale str("Hello there");
        ItType it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        const StringLocale str("Hello there");
        ItType it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Access")
    {
        const StringLocale str("Hello there");
        ItType it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringLocale str("Hello there");
        ItType it = str.ConstBegin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringLocale str("Hello there");
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
        const StringLocale str("Hello there");
        StringLocale dst;
        for (ItType it = str.Begin(); it != str.End(); ++it)
        {
            dst.Append(*it);
        }
        REQUIRE(dst.GetSize() == 11);
        REQUIRE(strcmp(dst.GetData(), "Hello there") == 0);
    }
    SECTION("Modern for loop")
    {
        const StringLocale str("Hello there");
        StringLocale dst;
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

TEST_CASE("From locale to UTF32", "[String][FromLocale]")
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

TEST_CASE("Lexicographical compare", "[String]")
{
    SECTION("Two strings")
    {
        SECTION("Both empty")
        {
            const StringLocale a("");
            const StringLocale b("");
            REQUIRE(Compare(a, b) == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("");
            StringLocale b("aa");
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("aaa");
            StringLocale b("aaabbb");
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("aaabbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("aaa");
            StringLocale b("bbbb");
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("bbb");
            StringLocale b("aaaa");
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("bbbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b) == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("aaaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("aaa");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b) == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("aaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b) == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("ccc");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b) == 1);
        }
    }
    SECTION("First substring second string")
    {
        SECTION("Both empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, 2, 0, b).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("aa");
            StringLocale b("aa");
            REQUIRE(Compare(a, 2, 0, b).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, 1, 1, b).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("bbbaaa");
            StringLocale b("aaabbb");
            REQUIRE(Compare(a, 3, 3, b).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("cccaaabbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, 3, 6, b).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("caaa");
            StringLocale b("bbbb");
            REQUIRE(Compare(a, 1, 3, b).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("ccbbb");
            StringLocale b("aaaa");
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("ccbbbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, 2, 4, b).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("caaaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, 1, 4, b).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("ccaaa");
            StringLocale b("aaa");
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("caaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, 1, 3, b).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("aaccc");
            StringLocale b("bbb");
            REQUIRE(Compare(a, 2, 3, b).GetValue() == 1);
        }
    }
    SECTION("Both substrings")
    {
        SECTION("Both empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, 2, 0, b, 0, 0).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("aa");
            StringLocale b("aa");
            REQUIRE(Compare(a, 2, 0, b, 0, 2).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, 1, 1, b, 0, 0).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("bbbaaa");
            StringLocale b("aaabbb");
            REQUIRE(Compare(a, 3, 3, b, 0, 6).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("cccaaabbb");
            StringLocale b("caaa");
            REQUIRE(Compare(a, 3, 6, b, 1, 3).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("caaa");
            StringLocale b("abbbb");
            REQUIRE(Compare(a, 1, 3, b, 1, 4).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("ccbbb");
            StringLocale b("aaaaaa");
            REQUIRE(Compare(a, 2, 3, b, 2, 4).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("ccbbbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, 2, 4, b, 0, 3).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("caaaa");
            StringLocale b("dfdbbb");
            REQUIRE(Compare(a, 1, 4, b, 3, 3).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("ccaaa");
            StringLocale b("baaa");
            REQUIRE(Compare(a, 2, 3, b, 1, 3).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("caaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, 1, 3, b, 0, 3).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("aaccc");
            StringLocale b("abbb");
            REQUIRE(Compare(a, 2, 3, b, 1, 3).GetValue() == 1);
        }
    }
    SECTION("First substring second string literal")
    {
        SECTION("Both empty")
        {
            StringLocale a("aa");
            REQUIRE(Compare(a, 2, 0, "").GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("aa");
            REQUIRE(Compare(a, 2, 0, "aa").GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            REQUIRE(Compare(a, 1, 1, "").GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("bbbaaa");
            REQUIRE(Compare(a, 3, 3, "aaabbb").GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("cccaaabbb");
            REQUIRE(Compare(a, 3, 6, "aaa").GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("caaa");
            REQUIRE(Compare(a, 1, 3, "bbbb").GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("ccbbb");
            REQUIRE(Compare(a, 2, 3, "aaaa").GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("ccbbbb");
            REQUIRE(Compare(a, 2, 4, "aaa").GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("caaaa");
            REQUIRE(Compare(a, 1, 4, "bbb").GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("ccaaa");
            REQUIRE(Compare(a, 2, 3, "aaa").GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("caaa");
            REQUIRE(Compare(a, 1, 3, "bbb").GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("aaccc");
            REQUIRE(Compare(a, 2, 3, "bbb").GetValue() == 1);
        }
    }
    SECTION("First substring second substring literal")
    {
        SECTION("Both empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, 2, 0, "a", 0).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("aa");
            StringLocale b("aa");
            REQUIRE(Compare(a, 2, 0, "aab", 2).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            REQUIRE(Compare(a, 1, 1, "", 0).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("bbbaaa");
            REQUIRE(Compare(a, 3, 3, "aaabbba", 6).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("cccaaabbb");
            REQUIRE(Compare(a, 3, 6, "aaac", 3).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("caaa");
            REQUIRE(Compare(a, 1, 3, "bbbba", 4).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("ccbbb");
            REQUIRE(Compare(a, 2, 3, "aaaabb", 4).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("ccbbbb");
            REQUIRE(Compare(a, 2, 4, "aaa", 3).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("caaaa");
            REQUIRE(Compare(a, 1, 4, "bbbdfd", 3).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("ccaaa");
            REQUIRE(Compare(a, 2, 3, "aaaab", 3).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("caaa");
            REQUIRE(Compare(a, 1, 3, "bbb", 3).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("aaccc");
            REQUIRE(Compare(a, 2, 3, "bbb", 3).GetValue() == 1);
        }
    }
}

TEST_CASE("Sub string", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str, 6, 1);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Empty sub string")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str, 0, 0);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "");
            REQUIRE(result.GetValue().GetSize() == 0);
        }
        SECTION("Empty source string")
        {
            StringLocale str("");
            auto result = Opal::GetSubString(str, 0, 0);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == Opal::ErrorCode::OutOfBounds);
        }
        SECTION("All defaults")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "Hello");
            REQUIRE(result.GetValue().GetSize() == 5);
        }
        SECTION("Custom start pos")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "llo");
            REQUIRE(result.GetValue().GetSize() == 3);
        }
        SECTION("Custom count")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str, 2, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "ll");
            REQUIRE(result.GetValue().GetSize() == 2);
        }
        SECTION("Count over size")
        {
            StringLocale str("Hello");
            auto result = Opal::GetSubString(str, 2, 10);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "llo");
            REQUIRE(result.GetValue().GetSize() == 3);
        }
    }
    SECTION("Long string")
    {
        SECTION("Bad start position")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str, 500, 1);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Empty sub string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str, 0, 0);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "");
            REQUIRE(result.GetValue().GetSize() == 0);
        }
        SECTION("Empty source string")
        {
            StringLocale str("");
            auto result = Opal::GetSubString(str, 0, 0);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == Opal::ErrorCode::OutOfBounds);
        }
        SECTION("All defaults")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str);
            REQUIRE(result.GetValue().GetSize() == str.GetSize());
        }
        SECTION("Custom start pos")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == StringLocale(str.Begin() + 2, str.End()));
            REQUIRE(result.GetValue().GetSize() == str.GetSize() - 2);
        }
        SECTION("Custom count")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str, 2, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == StringLocale(str.Begin() + 2, str.Begin() + 4));
            REQUIRE(result.GetValue().GetSize() == 2);
        }
        SECTION("Count over size")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = Opal::GetSubString(str, 2, 1000);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == StringLocale(str.Begin() + 2, str.End()));
            REQUIRE(result.GetValue().GetSize() == str.GetSize() - 2);
        }
    }
}

TEST_CASE("Operator +=", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Append other string")
        {
            StringLocale str("Hello");
            StringLocale other(" there");
            str += other;
            REQUIRE(str == "Hello there");
        }
        SECTION("Append character")
        {
            StringLocale str("Hello");
            str += ' ';
            REQUIRE(str == "Hello ");
        }
        SECTION("Append char pointer")
        {
            StringLocale str("Hello");
            str += " there";
            REQUIRE(str == "Hello there");
        }
        SECTION("Append null")
        {
            StringLocale str("Hello");
            str += nullptr;
            REQUIRE(str == "Hello");
        }
    }
    SECTION("Long string")
    {
        SECTION("Append other string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            StringLocale other(" Hello there");
            str += other;
            REQUIRE(
                str ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. Hello there");
        }
        SECTION("Append character")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            str += ' ';
            REQUIRE(
                str ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. ");
        }
        SECTION("Append char pointer")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            str += " Hello there";
            REQUIRE(
                str ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. Hello there");
        }
        SECTION("Append null")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            str += nullptr;
            REQUIRE(
                str ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
        }
    }
}

TEST_CASE("Operator +", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Add string and string")
        {
            StringLocale str("Hello");
            StringLocale other(" there");
            auto result = str + other;
            REQUIRE(result == "Hello there");
        }
        SECTION("Add string and char")
        {
            StringLocale str("Hello");
            auto result = str + ' ';
            REQUIRE(result == "Hello ");
        }
        SECTION("Add string and char pointer")
        {
            StringLocale str("Hello");
            auto result = str + " there";
            REQUIRE(result == "Hello there");
        }
        SECTION("Add char and string")
        {
            StringLocale str("Hello");
            auto result = ' ' + str;
            REQUIRE(result == " Hello");
        }
        SECTION("Add char pointer and string")
        {
            StringLocale str("Hello");
            auto result = " there" + str;
            REQUIRE(result == " thereHello");
        }
        SECTION("Add null and string")
        {
            StringLocale str("Hello");
            auto result = nullptr + str;
            REQUIRE(result == "Hello");
        }
    }
    SECTION("Long string")
    {
        SECTION("Add string and string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            StringLocale other(" Hello there");
            auto result = str + other;
            REQUIRE(
                result ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. Hello there");
        }
        SECTION("Add string and char")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = str + ' ';
            REQUIRE(
                result ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. ");
        }
        SECTION("Add string and char pointer")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = str + " Hello there";
            REQUIRE(
                result ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book. Hello there");
        }
        SECTION("Add char and string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = ' ' + str;
            REQUIRE(
                result ==
                " Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
        }
        SECTION("Add char pointer and string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = " there" + str;
            REQUIRE(
                result ==
                " thereLorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's "
                "standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
        }
        SECTION("Add null and string")
        {
            StringLocale str(
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
            auto result = nullptr + str;
            REQUIRE(
                result ==
                "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
                "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
                "book.");
        }
    }
}

TEST_CASE("Find", "[String]")
{
    SECTION("Find with string object")
    {
        SECTION("Search string is empty and start position is valid")
        {
            const StringLocale str("Hello there");
            const StringLocale search("");
            auto result = Find(str, search);
            REQUIRE(result == 0);
        }
        SECTION("Search string is empty and start position is not valid")
        {
            const StringLocale str("Hello there");
            const StringLocale search("");
            auto result = Find(str, search, 12);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Bad start position")
        {
            const StringLocale str("Hello there");
            const StringLocale search("there");
            auto result = Find(str, search, 12);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Not enough string is left for search")
        {
            const StringLocale str("Hello there");
            const StringLocale search("there");
            auto result = Find(str, search, 8);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Main string is empty")
        {
            const StringLocale str;
            const StringLocale search("there");
            auto result = Find(str, search);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search success")
        {
            const StringLocale str("Hello there");
            const StringLocale search("there");
            auto result = Find(str, search);
            REQUIRE(result == 6);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            const StringLocale search("world");
            auto result = Find(str, search);
            REQUIRE(result == StringLocale::k_npos);
        }
    }
    SECTION("Find with char pointer")
    {
        SECTION("Search string is nullptr")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, nullptr);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search string is empty and start position is valid")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "");
            REQUIRE(result == 0);
        }
        SECTION("Search string is empty and start position is not valid")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "", 12);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Bad start position")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "there", 12);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Not enough string is left for search")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "there", 8);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Main string is empty")
        {
            const StringLocale str;
            auto result = Find(str, "there");
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search success")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "there");
            REQUIRE(result == 6);
        }
        SECTION("Search success of part of the search string")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "therearr", 0, 5);
            REQUIRE(result == 6);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "world");
            REQUIRE(result == StringLocale::k_npos);
        }
    }
    SECTION("Find a character")
    {
        SECTION("Main string is empty")
        {
            const StringLocale str;
            auto result = Find(str, 'a');
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Start position is out of bounds")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, 'a', 12);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search success")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, 't');
            REQUIRE(result == 6);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, 'w');
            REQUIRE(result == StringLocale::k_npos);
        }
    }
}

TEST_CASE("Reverse Find", "[String]")
{
    SECTION("With string object")
    {
        SECTION("Search string is empty and start position is valid")
        {
            const StringLocale str("Hello there");
            const StringLocale search("");
            auto result = ReverseFind(str, search, 5);
            REQUIRE(result == 5);
        }
        SECTION("Search string is empty and start position is not valid")
        {
            const StringLocale str("Hello there");
            const StringLocale search("");
            auto result = ReverseFind(str, search, 12);
            REQUIRE(result == 11);
        }
        SECTION("Not enough string is left for search")
        {
            const StringLocale str("Hello there");
            const StringLocale search("Hello");
            auto result = ReverseFind(str, search, 2);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search success")
        {
            const StringLocale str("Hello Hello there");
            const StringLocale search("Hello");
            auto result = ReverseFind(str, search);
            REQUIRE(result == 6);
        }
        SECTION("Partial search success")
        {
            const StringLocale str("Hello Hello there");
            const StringLocale search("Hello");
            auto result = ReverseFind(str, search, 5);
            REQUIRE(result == 0);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            const StringLocale search("world");
            auto result = ReverseFind(str, search);
            REQUIRE(result == StringLocale::k_npos);
        }
    }
    SECTION("With char pointer")
    {
        SECTION("Search string is nullptr")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, nullptr);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search string is empty and start position is valid")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, "", 5);
            REQUIRE(result == 5);
        }
        SECTION("Search string is empty and start position is not valid")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, "", 12);
            REQUIRE(result == 11);
        }
        SECTION("Not enough string is left for search")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, "Hello", 2);
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Search success")
        {
            const StringLocale str("Hello Hello there");
            auto result = ReverseFind(str, "Hello");
            REQUIRE(result == 6);
        }
        SECTION("Partial search success")
        {
            const StringLocale str("Hello Hello there");
            auto result = ReverseFind(str, "Hello", 5);
            REQUIRE(result == 0);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, "world");
            REQUIRE(result == StringLocale::k_npos);
        }
    }
    SECTION("Character")
    {
        SECTION("Main string is empty")
        {
            const StringLocale str;
            auto result = ReverseFind(str, 'a');
            REQUIRE(result == StringLocale::k_npos);
        }
        SECTION("Success search")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, 't');
            REQUIRE(result == 6);
        }
        SECTION("Search not found")
        {
            const StringLocale str("Hello there");
            auto result = ReverseFind(str, 'w');
            REQUIRE(result == StringLocale::k_npos);
        }
    }
}

TEST_CASE("Erase", "[String]")
{
    SECTION("Erase with start position and count")
    {
        SECTION("Start position out of bounds")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(30);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Count out of bounds")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(5, 30);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "Hello");
        }
        SECTION("Small count")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(5, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == "Hellohere");
        }
    }
    SECTION("Erase with single iterator")
    {
        SECTION("Iterator out of bounds")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.End());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Remove from beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(*result.GetValue() == 'e');
            REQUIRE(str.GetSize() == 10);
        }
        SECTION("Remove from middle")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin() + 5);
            REQUIRE(result.HasValue() == true);
            REQUIRE(*result.GetValue() == 't');
            REQUIRE(str.GetSize() == 10);
        }
        SECTION("Remove from end")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.End() - 1);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.End());
            REQUIRE(str.GetSize() == 10);
        }
    }
    SECTION("Erase with single const iterator")
    {
        SECTION("Iterator out of bounds")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstEnd());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Remove from beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(*result.GetValue() == 'e');
            REQUIRE(str.GetSize() == 10);
        }
        SECTION("Remove from middle")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin() + 5);
            REQUIRE(result.HasValue() == true);
            REQUIRE(*result.GetValue() == 't');
            REQUIRE(str.GetSize() == 10);
        }
        SECTION("Remove from end")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstEnd() - 1);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.End());
            REQUIRE(str.GetSize() == 10);
        }
    }
}

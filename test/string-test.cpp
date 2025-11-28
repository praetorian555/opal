#include <iostream>

#include "test-helpers.h"

#include "opal/container/string.h"
#include "opal/container/hash-map.h"
#include "opal/container/string-hash.h"

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
            MallocAllocator da;
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
            for (u64 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == 'd');
            }
        }
        SECTION("Count and value with allocator")
        {
            MallocAllocator da;
            StringUtf8 str(5, 'd', &da);
            REQUIRE(str.GetSize() == 5);
            REQUIRE(str.GetCapacity() == 6);
            REQUIRE(str.GetData() != nullptr);
            for (u64 i = 0; i < str.GetSize(); i++)
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
            MallocAllocator da;
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
            MallocAllocator da;
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
            MallocAllocator da;
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
            MallocAllocator da;
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
            MallocAllocator da1;
            MallocAllocator da2;
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
            for (u64 i = 0; i < str.GetSize(); i++)
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
            for (u64 i = 0; i < str.GetSize(); i++)
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
            MallocAllocator da;
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
            for (u64 i = 0; i < k_sub_str_size; i++)
            {
                REQUIRE(ref[i] == str.GetData()[i]);
            }
        }
        SECTION("Substring of a string literal with allocator")
        {
            MallocAllocator da;
            constexpr u64 k_sub_str_size = 75;
            StringLocale str(ref, k_sub_str_size, &da);
            REQUIRE(str.GetSize() == k_sub_str_size);
            REQUIRE(str.GetCapacity() == k_sub_str_size + 1);
            REQUIRE(str.GetData() != nullptr);
            for (u64 i = 0; i < k_sub_str_size; i++)
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
            MallocAllocator da;
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
            MallocAllocator da;
            StringLocale first(ref);
            StringLocale second(first, 6, &da);
            REQUIRE(second.GetCapacity() == 569);
            REQUIRE(second.GetSize() == 568);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (u64 i = 0; i < second.GetSize(); i++)
            {
                REQUIRE(ref[6 + i] == second.GetData()[i]);
            }
        }
        SECTION("Copy substring with allocator")
        {
            MallocAllocator da;
            StringLocale first(ref);
            StringLocale second(first, 6, &da);
            REQUIRE(second.GetCapacity() == 569);
            REQUIRE(second.GetSize() == 568);
            REQUIRE(first.GetData() != second.GetData());
            REQUIRE(&first.GetAllocator() != &second.GetAllocator());
            for (u64 i = 0; i < second.GetSize(); i++)
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
        SECTION("Copy")
        {
            SECTION("Destination string empty")
            {
                const char ref[] = "Hello there";
                StringLocale str1(ref);
                StringLocale str2;
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 11);
                REQUIRE(str2.GetCapacity() == 12);
            }
            SECTION("Destination string not empty, same allocator, not enough space in destination")
            {
                const char ref[] = "Hello there";
                StringLocale str1(ref);
                StringLocale str2(5, 'd');
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 11);
                REQUIRE(str2.GetCapacity() == 12);
            }
            SECTION("Destination string not empty, same allocator, enough space in destination")
            {
                const char ref[] = "Hello there";
                StringLocale str1(ref);
                StringLocale str2(20, 'd');
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 11);
                REQUIRE(str2.GetCapacity() == 21);
            }
            SECTION("Destination string not empty, different allocators, enough space in destination")
            {
                const char ref[] = "Hello there";
                MallocAllocator da1;
                LinearAllocator da2("Linalloc");
                StringLocale str1(ref, &da1);
                StringLocale str2(20, 'd', static_cast<AllocatorBase*>(&da2));
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 11);
                REQUIRE(str2.GetCapacity() == 12);
            }
        }
        SECTION("Move")
        {
            const char ref[] = "Hello there";
            StringLocale str1(ref);
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
    }
    SECTION("Long string")
    {
        const char ref[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy "
            "text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It "
            "has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was "
            "popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop "
            "publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        SECTION("Copy")
        {
            SECTION("Destination string empty")
            {
                StringLocale str1(ref);
                StringLocale str2;
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 574);
                REQUIRE(str2.GetCapacity() == 575);
            }
            SECTION("Destination string not empty, same allocator, not enough space in destination")
            {
                StringLocale str1(ref);
                StringLocale str2(5, 'd');
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 574);
                REQUIRE(str2.GetCapacity() == 575);
            }
            SECTION("Destination string not empty, same allocator, enough space in destination")
            {
                StringLocale str1(ref);
                StringLocale str2(600, 'd');
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 574);
                REQUIRE(str2.GetCapacity() == 601);
            }
            SECTION("Destination string not empty, different allocators, enough space in destination")
            {
                MallocAllocator da1;
                LinearAllocator da2("linalloc");
                StringLocale str1(ref, &da1);
                StringLocale str2(600, 'd', static_cast<AllocatorBase*>(&da2));
                str2 = str1;
                REQUIRE(strcmp(str2.GetData(), ref) == 0);
                REQUIRE(str1.GetData() != str2.GetData());
                REQUIRE(str2.GetSize() == 574);
                REQUIRE(str2.GetCapacity() == 575);
            }
        }
        SECTION("Move")
        {
            StringLocale str1(ref);
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

TEST_CASE("Accessor", "[String]")
{
    SECTION("Const")
    {
        const StringUtf8 str("Hello there");
        REQUIRE(strcmp(*str, "Hello world!"));
    }
    SECTION("Non-const")
    {
        StringUtf8 str("Hello there");
        REQUIRE(strcmp(*str, "Hello world!"));
    }
}

TEST_CASE("Assign", "[String]")
{
    SECTION("Short string")
    {
        SECTION("Count and value")
        {
            SECTION("Count is zero")
            {
                StringUtf8 str("Hello there");
                REQUIRE_NOTHROW(str.Assign(0, 'd'));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 0);
            }
            SECTION("Count is normal and no resize")
            {
                StringUtf8 str("Hello there");
                REQUIRE_NOTHROW(str.Assign(5, 'd'));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 5);
                for (i32 i = 0; i < 5; i++)
                {
                    REQUIRE(str.GetData()[i] == 'd');
                }
            }
            SECTION("Count is normal and triggers resize")
            {
                StringUtf8 str;
                REQUIRE_NOTHROW(str.Assign(5, 'd'));
                REQUIRE(str.GetCapacity() == 6);
                REQUIRE(str.GetSize() == 5);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == 'd');
                }
            }
            SECTION("Resize fails")
            {
                NullAllocator null_allocator;
                StringUtf8 str(&null_allocator);
                REQUIRE_THROWS_AS(str.Assign(5, 'd'), OutOfMemoryException);
            }
        }
        SECTION("Other string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other;
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 0);
            }
            SECTION("Same string")
            {
                StringUtf8 str("Hello there");
                REQUIRE_NOTHROW(str.Assign(str));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 11);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
                }
            }
            SECTION("Other string smaller then current capacity")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello");
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 5);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello"[i]);
                }
            }
            SECTION("Other string larger then current capacity")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello there and then");
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetSize() == 20);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then some"[i]);
                }
            }
        }
        SECTION("Part of the other string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other;
                const ErrorCode err = str.Assign(other, 0);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Same string")
            {
                StringUtf8 str("Hello there");
                str.Assign(str, 5, 6);
                const ErrorCode err = str.Assign(str, 5, 6);
                REQUIRE(err == ErrorCode::SelfNotAllowed);
            }
            SECTION("Bad position")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 10);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Bad count")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 0, 10);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Copy rest of the string")
            {
                const StringUtf8 ref("Hello there");
                StringUtf8 copy;
                const ErrorCode err = copy.Assign(ref, 6);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(copy.GetSize() == 5);
                REQUIRE(copy.GetCapacity() == 6);
                for (u64 i = 0; i < copy.GetSize(); i++)
                {
                    REQUIRE(copy.GetData()[i] == u8"there"[i]);
                }
            }
            SECTION("Copy with specific count")
            {
                const StringUtf8 ref("Hello there");
                StringUtf8 copy;
                const ErrorCode err = copy.Assign(ref, 6, 3);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(copy.GetSize() == 3);
                REQUIRE(copy.GetCapacity() == 4);
                for (u64 i = 0; i < copy.GetSize(); i++)
                {
                    REQUIRE(copy.GetData()[i] == u8"the"[i]);
                }
            }
            SECTION("Other string smaller then current capacity")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 0, 5);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 5);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello"[i]);
                }
            }
            SECTION("Other string larger then current capacity")
            {
                StringUtf8 str("Hello there");
                const StringUtf8 other("Hello there and then");
                const ErrorCode err = str.Assign(other, 0, 20);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 20);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then some"[i]);
                }
            }
        }
        SECTION("Move string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there");
                StringUtf8 other;
                str.Assign(Move(other));
                REQUIRE(str.GetCapacity() == 0);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(other.GetCapacity() == 0);
                REQUIRE(other.GetSize() == 0);
            }
            SECTION("Self")
            {
                StringUtf8 str("Hello there");
                str.Assign(Move(str));
                REQUIRE(str.GetCapacity() == 12);
                REQUIRE(str.GetSize() == 11);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
                }
            }
            SECTION("Non-empty string")
            {
                StringUtf8 str("Hello there");
                StringUtf8 other("Goodbye");
                str.Assign(Move(other));
                REQUIRE(str.GetCapacity() == 8);
                REQUIRE(str.GetSize() == 7);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye"[i]);
                }
                REQUIRE(other.GetCapacity() == 0);
                REQUIRE(other.GetSize() == 0);
            }
        }
        SECTION("String literal and count")
        {
            SECTION("Null pointer")
            {
                StringUtf8 str("Hello there");
                ErrorCode err = str.Assign(nullptr, 0);
                REQUIRE(err == ErrorCode::InvalidArgument);
            }
            SECTION("Count larger then string literal")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign("Hello there", 15);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Empty string literal")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign("");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(str.GetCapacity() == 12);
            }
            SECTION("Non-empty string literal")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign("Goodbye");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 7);
                REQUIRE(str.GetCapacity() == 12);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye"[i]);
                }
            }
            SECTION("Partial string literal")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign("Goodbye", 4);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 4);
                REQUIRE(str.GetCapacity() == 12);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Good"[i]);
                }
            }
            SECTION("String literal that triggers resize")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign("Goodbye and then some");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 21);
                REQUIRE(str.GetCapacity() == 22);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye and then some"[i]);
                }
            }
            SECTION("Resize failure")
            {
                NullAllocator null_allocator;
                StringUtf8 str(&null_allocator);
                REQUIRE_THROWS_AS(str.Assign("Goodbye and then some"), OutOfMemoryException);
            }
        }
        SECTION("Iterators")
        {
            SECTION("Invalid iterator")
            {
                const char* ref = "Hello there";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 3);
                REQUIRE(err == ErrorCode::InvalidArgument);
            }
            SECTION("Assign from self")
            {
                StringUtf8 str("Hello there");
                const ErrorCode err = str.Assign(str.begin(), str.end());
                REQUIRE(err == ErrorCode::SelfNotAllowed);
            }
            SECTION("Empty range")
            {
                const char* ref = "Hello there";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 5);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(str.GetCapacity() == 1);
            }
            SECTION("Normal range that triggers resize")
            {
                const char* ref = "Hello there";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 10);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 5);
                REQUIRE(str.GetCapacity() == 6);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == ref[5 + i]);
                }
            }
            SECTION("Resize fails")
            {
                NullAllocator null_allocator;
                StringUtf8 str(&null_allocator);
                const char* ref = "Hello there";
                REQUIRE_THROWS_AS(str.Assign(ref + 5, ref + 10), OutOfMemoryException);
            }
        }
    }
    SECTION("Long string")
    {
        SECTION("Count and value")
        {
            SECTION("Count is zero")
            {
                StringUtf8 str("Hello there and then some more");
                REQUIRE_NOTHROW(str.Assign(0, 'd'));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 0);
            }
            SECTION("Count is normal and no resize")
            {
                StringUtf8 str("Hello there and then some more");
                REQUIRE_NOTHROW(str.Assign(5, 'd'));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 5);
                for (i32 i = 0; i < 5; i++)
                {
                    REQUIRE(str.GetData()[i] == 'd');
                }
            }
            SECTION("Count is normal and triggers resize")
            {
                StringUtf8 str("Hello there and then some more");
                REQUIRE_NOTHROW(str.Assign(50, 'd'));
                REQUIRE(str.GetSize() == 50);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == 'd');
                }
            }
            SECTION("Resize fails")
            {
                NullAllocator null_allocator;
                StringUtf8 str( &null_allocator);
                REQUIRE_THROWS_AS(str.Assign(50, 'd'), OutOfMemoryException);
            }
        }
        SECTION("Other string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other;
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 0);
            }
            SECTION("Same string")
            {
                StringUtf8 str("Hello there and then some more");
                REQUIRE_NOTHROW(str.Assign(str));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 30);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then some more"[i]);
                }
            }
            SECTION("Other string smaller then current capacity")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello");
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 5);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello"[i]);
                }
            }
            SECTION("Other string larger then current capacity")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello there and then some more and double");
                REQUIRE_NOTHROW(str.Assign(other));
                REQUIRE(str.GetSize() == 41);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then some more and double"[i]);
                }
            }
        }
        SECTION("Part of the other string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other;
                const ErrorCode err = str.Assign(other, 0);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Same string")
            {
                StringUtf8 str("Hello there and then some more");
                str.Assign(str, 5, 6);
                const ErrorCode err = str.Assign(str, 5, 6);
                REQUIRE(err == ErrorCode::SelfNotAllowed);
            }
            SECTION("Bad position")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 40);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Bad count")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 0, 50);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Copy rest of the string")
            {
                const StringUtf8 ref("Hello there and then some more and double");
                StringUtf8 copy;
                const ErrorCode err = copy.Assign(ref, 6);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(copy.GetSize() == 35);
                for (u64 i = 0; i < copy.GetSize(); i++)
                {
                    REQUIRE(copy.GetData()[i] == u8"there and then some more and double"[i]);
                }
            }
            SECTION("Copy with specific count")
            {
                const StringUtf8 ref("Hello there and then some more and double");
                StringUtf8 copy;
                const ErrorCode err = copy.Assign(ref, 6, 28);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(copy.GetSize() == 28);
                REQUIRE(copy.GetCapacity() == 29);
                for (u64 i = 0; i < copy.GetSize(); i++)
                {
                    REQUIRE(copy.GetData()[i] == u8"there and then some more and"[i]);
                }
            }
            SECTION("Other string smaller then current capacity")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello");
                const ErrorCode err = str.Assign(other, 0, 5);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 5);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello"[i]);
                }
            }
            SECTION("Other string larger then current capacity")
            {
                StringUtf8 str("Hello there and then some more");
                const StringUtf8 other("Hello there and then");
                const ErrorCode err = str.Assign(other, 0, 20);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 20);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then"[i]);
                }
            }
        }
        SECTION("Move string")
        {
            SECTION("Empty string")
            {
                StringUtf8 str("Hello there and then some more");
                StringUtf8 other;
                str.Assign(Move(other));
                REQUIRE(str.GetCapacity() == 0);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(other.GetCapacity() == 0);
                REQUIRE(other.GetSize() == 0);
            }
            SECTION("Self")
            {
                StringUtf8 str("Hello there and then some more");
                str.Assign(Move(str));
                REQUIRE(str.GetCapacity() == 31);
                REQUIRE(str.GetSize() == 30);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Hello there and then some more"[i]);
                }
            }
            SECTION("Non-empty string")
            {
                StringUtf8 str("Hello there and then some more");
                StringUtf8 other("Goodbye");
                str.Assign(Move(other));
                REQUIRE(str.GetCapacity() == 8);
                REQUIRE(str.GetSize() == 7);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye"[i]);
                }
                REQUIRE(other.GetCapacity() == 0);
                REQUIRE(other.GetSize() == 0);
            }
        }
        SECTION("String literal and count")
        {
            SECTION("Null pointer")
            {
                StringUtf8 str("Hello there and then some more");
                ErrorCode err = str.Assign(nullptr, 0);
                REQUIRE(err == ErrorCode::InvalidArgument);
            }
            SECTION("Count larger then string literal")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign("Hello there", 50);
                REQUIRE(err == ErrorCode::OutOfBounds);
            }
            SECTION("Empty string literal")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign("");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(str.GetCapacity() == 31);
            }
            SECTION("Non-empty string literal")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign("Goodbye");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 7);
                REQUIRE(str.GetCapacity() == 31);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye"[i]);
                }
            }
            SECTION("Partial string literal")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign("Goodbye", 4);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 4);
                REQUIRE(str.GetCapacity() == 31);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Good"[i]);
                }
            }
            SECTION("String literal that triggers resize")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign("Goodbye and then some");
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 21);
                REQUIRE(str.GetCapacity() == 31);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == u8"Goodbye and then some"[i]);
                }
            }
            SECTION("Resize failure")
            {
                NullAllocator null_allocator;
                StringUtf8 str(&null_allocator);
                REQUIRE_THROWS_AS(str.Assign("Goodbye and then some"), OutOfMemoryException);
            }
        }
        SECTION("Iterators")
        {
            SECTION("Invalid iterator")
            {
                const char* ref = "Hello there and then some more";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 3);
                REQUIRE(err == ErrorCode::InvalidArgument);
            }
            SECTION("Assign from self")
            {
                StringUtf8 str("Hello there and then some more");
                const ErrorCode err = str.Assign(str.begin(), str.end());
                REQUIRE(err == ErrorCode::SelfNotAllowed);
            }
            SECTION("Empty range")
            {
                const char* ref = "Hello there and then some more";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 5);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 0);
                REQUIRE(str.GetCapacity() == 1);
            }
            SECTION("Normal range that triggers resize")
            {
                const char* ref = "Hello there and then some more";
                StringUtf8 str;
                const ErrorCode err = str.Assign(ref + 5, ref + 30);
                REQUIRE(err == ErrorCode::Success);
                REQUIRE(str.GetSize() == 25);
                REQUIRE(str.GetCapacity() == 26);
                for (u64 i = 0; i < str.GetSize(); i++)
                {
                    REQUIRE(str.GetData()[i] == ref[5 + i]);
                }
            }
            SECTION("Resize fails")
            {
                NullAllocator null_allocator;
                StringUtf8 str(&null_allocator);
                const char* ref = "Hello there";
                REQUIRE_THROWS_AS(str.Assign(ref + 5, ref + 10), OutOfMemoryException);
            }
        }
    }
}

TEST_CASE("Accessors", "[String]")
{
    SECTION("Short string")
    {
        SECTION("At")
        {
            StringUtf8 str("Hello there");
            REQUIRE(str.At(0) == 'H');
            REQUIRE(str.At(1) == 'e');
            REQUIRE(str.At(2) == 'l');
            REQUIRE(str.At(3) == 'l');
            REQUIRE(str.At(4) == 'o');
            REQUIRE(str.At(5) == ' ');
            REQUIRE(str.At(6) == 't');
            REQUIRE(str.At(7) == 'h');
            REQUIRE(str.At(8) == 'e');
            REQUIRE(str.At(9) == 'r');
            REQUIRE(str.At(10) == 'e');
        }
        SECTION("Out of bounds At")
        {
            StringUtf8 str("Hello there");
            REQUIRE_THROWS_AS(!str.At(12), OutOfBoundsException);
        }
        SECTION("Const At")
        {
            const StringUtf8 str("Hello there");
            REQUIRE(str.At(0) == 'H');
            REQUIRE(str.At(1) == 'e');
            REQUIRE(str.At(2) == 'l');
            REQUIRE(str.At(3) == 'l');
            REQUIRE(str.At(4) == 'o');
            REQUIRE(str.At(5) == ' ');
            REQUIRE(str.At(6) == 't');
            REQUIRE(str.At(7) == 'h');
            REQUIRE(str.At(8) == 'e');
            REQUIRE(str.At(9) == 'r');
            REQUIRE(str.At(10) == 'e');
        }
        SECTION("Out of bounds const At")
        {
            const StringUtf8 str("Hello there");
            REQUIRE_THROWS_AS(!str.At(11), OutOfBoundsException);
        }
        SECTION("Array operator")
        {
            StringUtf8 str("Hello there");
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Const array operator")
        {
            const StringUtf8 str("Hello there");
            REQUIRE(str[0] == 'H');
            REQUIRE(str[1] == 'e');
            REQUIRE(str[2] == 'l');
        }
        SECTION("Front")
        {
            StringUtf8 str("Hello there");
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
            const StringUtf8 str("Hello there");
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
            StringUtf8 str("Hello there");
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
            const StringUtf8 str("Hello there");
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
        SECTION("Reserve zero")
        {
            StringUtf8 str("Hello there");
            REQUIRE_THROWS_AS(str.Reserve(0), InvalidArgumentException);
        }
        SECTION("Out of memory")
        {
            NullAllocator null_allocator;
            StringUtf8 str(&null_allocator);
            REQUIRE_THROWS_AS(str.Reserve(10), OutOfMemoryException);
        }
        SECTION("Reserve larger then current capacity")
        {
            StringUtf8 str("Hello there");
            str.Reserve(20);
            REQUIRE(str.GetCapacity() == 20);
            REQUIRE(str.GetSize() == 11);
            for (u64 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
        SECTION("Reserve smaller then current capacity")
        {
            StringUtf8 str("Hello there");
            str.Reserve(5);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            for (u64 i = 0; i < str.GetSize(); i++)
            {
                REQUIRE(str.GetData()[i] == u8"Hello there"[i]);
            }
        }
    }
    SECTION("Long string")
    {
        const char8 ref_str[] =
            "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "
            "dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen "
            "book. "
            "It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. "
            "It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with "
            "desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
        SECTION("Reserve zero")
        {
            StringUtf8 str(ref_str);
            REQUIRE_THROWS_AS(str.Reserve(0), InvalidArgumentException);
        }
        SECTION("Out of memory")
        {
            NullAllocator null_allocator;
            StringUtf8 str(&null_allocator);
            REQUIRE_THROWS_AS(str.Reserve(10), OutOfMemoryException);
        }
        SECTION("Reserve larger then current capacity")
        {

            StringUtf8 str(ref_str);
            str.Reserve(600);
            REQUIRE(str.GetCapacity() == 600);
            REQUIRE(str.GetSize() == 574);
            for (u64 i = 0; i < str.GetSize(); i++)
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
            for (u64 i = 0; i < str.GetSize(); i++)
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
        SECTION("Character")
        {
            StringLocale str("Hello");
            str.Append(' ');
            REQUIRE(str.GetSize() == 6);
            REQUIRE(strcmp(str.GetData(), "Hello ") == 0);
        }
        SECTION("Character but allocation fails")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            REQUIRE_THROWS_AS(str.Append(' '), OutOfMemoryException);
        }
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
            REQUIRE(err == ErrorCode::InvalidArgument);
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
        SECTION("String literal allocation fails")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            REQUIRE_THROWS_AS(str.Append(" there"), OutOfMemoryException);
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
        SECTION("String but allocation fails")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            StringLocale str2(" there");
            REQUIRE_THROWS_AS(str.Append(str2), OutOfMemoryException);
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
        SECTION("Sub string bad count")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
            ErrorCode err = str.Append(str2, 0, 10);
            REQUIRE(err == ErrorCode::OutOfBounds);
        }
        SECTION("Sub string allocation fails")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            StringLocale str2(" there");
            REQUIRE_THROWS_AS(str.Append(str2, 0, 5), OutOfMemoryException);
        }
        SECTION("Sub string with k_npos")
        {
            StringLocale str("Hello");
            StringLocale str2(" there");
            str.Append(str2, 0, StringLocale::k_npos);
            REQUIRE(str.GetCapacity() == 12);
            REQUIRE(str.GetSize() == 11);
            REQUIRE(strcmp(str.GetData(), "Hello there") == 0);
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
            REQUIRE(err == ErrorCode::InvalidArgument);
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
            REQUIRE(err == ErrorCode::InvalidArgument);
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
    SECTION("Increment")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
        auto prev = it++;
        REQUIRE(it - prev == 1);
    }
    SECTION("Decrement")
    {
        StringLocale str("Hello there");
        auto it = str.End();
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
        auto it = str.End();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
        auto prev = it--;
        REQUIRE(prev - it == 1);
    }
    SECTION("Add")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        auto it2 = str.Begin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        StringLocale str("Hello there");
        auto it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        StringLocale str("Hello there");
        auto it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Difference")
    {
        StringLocale str("Hello there");
        auto it1 = str.Begin();
        auto it2 = str.End();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Access")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringLocale str("Hello there");
        auto it1 = str.Begin();
        auto it2 = str.Begin();
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
        for (auto it = str.Begin(); it != str.End(); ++it)
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
    SECTION("Difference")
    {
        StringLocale str("Hello there");
        auto it1 = str.ConstBegin();
        auto it2 = str.ConstEnd();
        REQUIRE(it2 - it1 == 11);
    }
    SECTION("Increment")
    {
        const StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
    }
    SECTION("Post increment")
    {
        const StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*it == 'H');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'l');
        auto prev = it++;
        REQUIRE(it - prev == 1);
    }
    SECTION("Decrement")
    {
        StringLocale str("Hello there");
        auto it = str.ConstEnd();
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
        auto it = str.ConstEnd();
        --it;
        REQUIRE(*it == 'e');
        --it;
        REQUIRE(*it == 'r');
        --it;
        REQUIRE(*it == 'e');
        auto prev = it--;
        REQUIRE(prev - it == 1);
    }
    SECTION("Add")
    {
        const StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(*(it + 0) == 'H');
        REQUIRE(*(it + 1) == 'e');
        REQUIRE(*(it + 2) == 'l');

        auto it2 = str.ConstBegin();
        REQUIRE((11 + it2) == str.End());
    }
    SECTION("Add assignment")
    {
        StringLocale str("Hello there");
        auto it = str.ConstBegin();
        REQUIRE(*(it += 0) == 'H');
        REQUIRE(*(it += 1) == 'e');
        REQUIRE(*(it += 1) == 'l');
    }
    SECTION("Subtract")
    {
        const StringLocale str("Hello there");
        auto it = str.End();
        REQUIRE((it - 0) == str.End());
        REQUIRE(*(it - 1) == 'e');
        REQUIRE(*(it - 2) == 'r');
        REQUIRE(*(it - 3) == 'e');
    }
    SECTION("Subtract assignment")
    {
        const StringLocale str("Hello there");
        auto it = str.End();
        REQUIRE((it -= 0) == str.End());
        REQUIRE(*(it -= 1) == 'e');
        REQUIRE(*(it -= 1) == 'r');
        REQUIRE(*(it -= 1) == 'e');
    }
    SECTION("Access")
    {
        const StringLocale str("Hello there");
        auto it = str.Begin();
        REQUIRE(it[0] == 'H');
        REQUIRE(it[1] == 'e');
        REQUIRE(it[2] == 'l');
    }
    SECTION("Dereference")
    {
        StringLocale str("Hello there");
        auto it = str.ConstBegin();
        REQUIRE(*it == 'H');
    }
    SECTION("Compare")
    {
        StringLocale str("Hello there");
        auto it1 = str.ConstBegin();
        auto it2 = str.ConstBegin();
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
        for (auto it = str.Begin(); it != str.End(); ++it)
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
    StringUtf8 utf8("での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf8, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to UTF8", "[String]")
{
    StringUtf8 utf8("での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf8 utf8_result;
    utf8_result.Resize(200);
    ErrorCode error = Transcode(utf32, utf8_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf8_result == utf8);
}

TEST_CASE("From native wide to UTF32", "[String]")
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide utf_wide(L"での日本語文字コードを扱うために使用されている従来の");
#elif defined(OPAL_PLATFORM_LINUX)
    const StringWide utf_wide(u"での日本語文字コードを扱うために使用されている従来の");
#endif
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(utf_wide, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to native wide", "[String]")
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide wide(L"での日本語文字コードを扱うために使用されている従来の");
#elif defined(OPAL_PLATFORM_LINUX)
    const StringWide wide(u"での日本語文字コードを扱うために使用されている従来の");
#endif
    const StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringWide wide_result;
    wide_result.Resize(200);
    ErrorCode error = Transcode(utf32, wide_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(wide_result == wide);
}

TEST_CASE("From locale to UTF32", "[String][FromLocale]")
{
    const StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");
    StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringUtf32 utf32_result;
    utf32_result.Resize(200);
    ErrorCode error = Transcode(str_locale, utf32_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(utf32_result == utf32);
}

TEST_CASE("From UTF32 to locale", "[String]")
{
    StringLocale str_locale(reinterpret_cast<const char8*>(u8"での日本語文字コードを扱うために使用されている従来の"));
    const StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");

    StringLocale locale_result;
    locale_result.Resize(300);
    ErrorCode error = Transcode(utf32, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
}

TEST_CASE("From wide to locale", "[String]")
{
#if defined(OPAL_PLATFORM_WINDOWS)
    StringWide wide(L"での日本語文字コードを扱うために使用されている従来の");
#elif defined(OPAL_PLATFORM_LINUX)
    const StringWide wide(u"での日本語文字コードを扱うために使用されている従来の");
#endif
    StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");

    StringLocale locale_result;
    locale_result.Resize(300);
    ErrorCode error = Transcode(wide, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
}

TEST_CASE("From const UTF8 to locale", "[String]")
{
    const StringUtf8 utf8("での日本語文字コードを扱うために使用されている従来の");
    StringLocale str_locale("での日本語文字コードを扱うために使用されている従来の");

    StringLocale locale_result;
    locale_result.Resize(300);
    ErrorCode error = Transcode(utf8, locale_result);
    REQUIRE(error == ErrorCode::Success);
    REQUIRE(locale_result == str_locale);
}

TEST_CASE("Transcode with empty destination", "[String]")
{
    SECTION("Output is StringUtf8")
    {
        StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");
        StringUtf8 utf8_result;
        ErrorCode error = Transcode(utf32, utf8_result);
        REQUIRE(error == ErrorCode::InsufficientSpace);
    }
    SECTION("Output is StringUtf16")
    {
        StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");
        StringUtf8 utf8_result;
        ErrorCode error = Transcode(utf32, utf8_result);
        REQUIRE(error == ErrorCode::InsufficientSpace);
    }
    SECTION("Output is StringUtf32")
    {
        StringUtf8 utf8("での日本語文字コードを扱うために使用されている従来の");
        StringUtf32 utf32_result;
        ErrorCode error = Transcode(utf8, utf32_result);
        REQUIRE(error == ErrorCode::InsufficientSpace);
    }
    SECTION("Output is StringWide")
    {
        StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");
        StringWide wide_result;
        ErrorCode error = Transcode(utf32, wide_result);
        REQUIRE(error == ErrorCode::InsufficientSpace);
    }
    SECTION("Output is StringLocale")
    {
        StringUtf32 utf32(U"での日本語文字コードを扱うために使用されている従来の");
        StringLocale locale_result;
        ErrorCode error = Transcode(utf32, locale_result);
        REQUIRE(error == ErrorCode::InsufficientSpace);
    }
}

TEST_CASE("Lexicographical compare", "[String]")
{
    SECTION("Two strings")
    {
        SECTION("Both empty")
        {
            const StringLocale a("");
            const StringLocale b("");
            REQUIRE(Compare(a, b).GetValue() == 0);
        }
        SECTION("First empty")
        {
            StringLocale a("");
            StringLocale b("aa");
            REQUIRE(Compare(a, b).GetValue() == -1);
        }
        SECTION("Second empty")
        {
            StringLocale a("aa");
            StringLocale b("");
            REQUIRE(Compare(a, b).GetValue() == 1);
        }
        SECTION("First shorter and equal")
        {
            StringLocale a("aaa");
            StringLocale b("aaabbb");
            REQUIRE(Compare(a, b).GetValue() == -1);
        }
        SECTION("Second shorter and equal")
        {
            StringLocale a("aaabbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b).GetValue() == 1);
        }
        SECTION("First shorter and smaller")
        {
            StringLocale a("aaa");
            StringLocale b("bbbb");
            REQUIRE(Compare(a, b).GetValue() == -1);
        }
        SECTION("First shorter and larger")
        {
            StringLocale a("bbb");
            StringLocale b("aaaa");
            REQUIRE(Compare(a, b).GetValue() == 1);
        }
        SECTION("Second shorter and smaller")
        {
            StringLocale a("bbbb");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b).GetValue() == 1);
        }
        SECTION("Second shorter and larger")
        {
            StringLocale a("aaaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b).GetValue() == -1);
        }
        SECTION("Equal size and same")
        {
            StringLocale a("aaa");
            StringLocale b("aaa");
            REQUIRE(Compare(a, b).GetValue() == 0);
        }
        SECTION("Equal size first smaller")
        {
            StringLocale a("aaa");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b).GetValue() == -1);
        }
        SECTION("Equal size second smaller")
        {
            StringLocale a("ccc");
            StringLocale b("bbb");
            REQUIRE(Compare(a, b).GetValue() == 1);
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
        SECTION("Search string is not empty but its not found")
        {
            const StringLocale str("Hello there");
            auto result = Find(str, "ema");
            REQUIRE(result == StringLocale::k_npos);
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
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Small count")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(5, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hellohere");
            REQUIRE(result.GetValue() == str.Begin() + 5);
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
    SECTION("Erase with a iterator range")
    {
        SECTION("Bad start iterator")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.End(), str.End());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Bad end iterator")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin(), str.Begin() - 1);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Invalid range")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin() + 5, str.Begin() + 3);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::InvalidArgument);
        }
        SECTION("Empty range")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin(), str.Begin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin());
            REQUIRE(str.GetSize() == 11);
        }
        SECTION("Remove from beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin(), str.Begin() + 5);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin());
            REQUIRE(str == " there");
            REQUIRE(str.GetSize() == 6);
        }
        SECTION("Remove from middle")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin() + 5, str.Begin() + 8);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin() + 5);
            REQUIRE(str == "Helloere");
            REQUIRE(str.GetSize() == 8);
        }
        SECTION("Remove from end")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.Begin() + 5, str.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.End());
            REQUIRE(str == "Hello");
            REQUIRE(str.GetSize() == 5);
        }
    }
    SECTION("Erase with a const iterator range")
    {
        SECTION("Bad start iterator")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstEnd(), str.ConstEnd());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Bad end iterator")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin(), str.ConstBegin() - 1);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Invalid range")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin() + 5, str.ConstBegin() + 3);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::InvalidArgument);
        }
        SECTION("Empty range")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin(), str.ConstBegin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin());
            REQUIRE(str.GetSize() == 11);
        }
        SECTION("Remove from beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin(), str.ConstBegin() + 5);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin());
            REQUIRE(str == " there");
            REQUIRE(str.GetSize() == 6);
        }
        SECTION("Remove from middle")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin() + 5, str.ConstBegin() + 8);
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.Begin() + 5);
            REQUIRE(str == "Helloere");
            REQUIRE(str.GetSize() == 8);
        }
        SECTION("Remove from end")
        {
            StringLocale str("Hello there");
            auto result = str.Erase(str.ConstBegin() + 5, str.ConstEnd());
            REQUIRE(result.HasValue() == true);
            REQUIRE(result.GetValue() == str.End());
            REQUIRE(str == "Hello");
            REQUIRE(str.GetSize() == 5);
        }
    }
}

TEST_CASE("Insert", "[String]")
{
    SECTION("With start position, count and value")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(30, 2, 'a');
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(0, 2, 'a'), OutOfMemoryException);
        }
        SECTION("Insert at beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(0, 2, 'a');
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(5, 2, 'a');
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(11, 2, 'a');
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With char pointer and count")
    {
        SECTION("Null pointer")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(0, nullptr, 2);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::InvalidArgument);
        }
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(30, "aa", 2);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(0, "aa", 2), OutOfMemoryException);
        }
        SECTION("Count is zero")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(0, "aa", 0);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert at the beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(0, "aa", 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(5, "aa", 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(11, "aa", 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With string object")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(30, in);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Bad start position of insert string")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(30, in, 10);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            StringLocale in("aa");
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(0, in), OutOfMemoryException);
        }
        SECTION("Count is zero")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(0, in, 0, 0);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert at the beginning")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(0, in);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(5, in, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(11, in, 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With single iterator")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.End() + 1, 'a', 2);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(str.Begin(), 'a', 2), OutOfMemoryException);
        }
        SECTION("Insert at beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.Begin(), 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.Begin() + 5, 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.End(), 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With single const iterator")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.ConstEnd() + 1, 'a', 2);
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(str.ConstBegin(), 'a', 2), OutOfMemoryException);
        }
        SECTION("Insert at beginning")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.ConstBegin(), 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.ConstBegin() + 5, 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            auto result = str.Insert(str.ConstEnd(), 'a', 2);
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With other iterator range")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.End() + 1, in.Begin(), in.End());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Invalid input range")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.Begin(), in.End(), in.Begin());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::InvalidArgument);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            StringLocale in("aa");
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(str.Begin(), in.Begin(), in.End()), OutOfMemoryException);
        }
        SECTION("Count is zero")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.Begin(), in.Begin(), in.Begin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert at the beginning")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.Begin(), in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.Begin() + 5, in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.End(), in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
    SECTION("With const iterator start and other iterator range")
    {
        SECTION("Bad start position")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstEnd() + 1, in.Begin(), in.End());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::OutOfBounds);
        }
        SECTION("Invalid input range")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstBegin(), in.End(), in.Begin());
            REQUIRE(result.HasValue() == false);
            REQUIRE(result.GetError() == ErrorCode::InvalidArgument);
        }
        SECTION("Memory allocation failed")
        {
            NullAllocator null_allocator;
            StringLocale str(&null_allocator);
            StringLocale in("aa");
            Expected<String<char, EncodingLocale>::iterator, ErrorCode> result;
            REQUIRE_THROWS_AS(result = str.Insert(str.ConstBegin(), in.Begin(), in.End()), OutOfMemoryException);
        }
        SECTION("Count is zero")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstBegin(), in.Begin(), in.Begin());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert at the beginning")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstBegin(), in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "aaHello there");
            REQUIRE(result.GetValue() == str.Begin());
        }
        SECTION("Insert in the middle")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstBegin() + 5, in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Helloaa there");
            REQUIRE(result.GetValue() == str.Begin() + 5);
        }
        SECTION("Insert at the end")
        {
            StringLocale str("Hello there");
            StringLocale in("aa");
            auto result = str.Insert(str.ConstEnd(), in.Begin(), in.End());
            REQUIRE(result.HasValue() == true);
            REQUIRE(str == "Hello thereaa");
            REQUIRE(result.GetValue() == str.End() - 2);
        }
    }
}

TEST_CASE("String length", "[String]")
{
    SECTION("Null string")
    {
        const char* str = nullptr;
        REQUIRE(Opal::GetStringLength(str) == 0);
    }
    SECTION("Empty string")
    {
        const char* str = "";
        REQUIRE(Opal::GetStringLength(str) == 0);
    }
    SECTION("Non-empty string")
    {
        const char* str = "Hello there";
        REQUIRE(Opal::GetStringLength(str) == 11);
    }
}

TEST_CASE("String key in map", "[String]")
{
    Opal::HashMap<Opal::StringUtf8, uint64_t, Opal::StringHash<Opal::StringUtf8>> map;
    map.insert(std::make_pair("Hello", 1));
    map.insert(std::make_pair("there", 2));

    REQUIRE(map.find("Hello")->second == 1);
    REQUIRE(map.find("there")->second == 2);
}

TEST_CASE("StartsWith", "[String]")
{
    const StringUtf8 str("Hello there");
    const StringUtf8 prefix("Hello");
    const StringUtf8 prefix_empty;
    REQUIRE(Opal::StartsWith(str, prefix));
    REQUIRE(Opal::StartsWith(str, prefix_empty));
    REQUIRE(Opal::StartsWith<StringUtf8>(str, "Hell"));
    REQUIRE(Opal::StartsWith<StringUtf8>(str, ""));
    REQUIRE(!Opal::StartsWith<StringUtf8>(str, "Hello there friends"));
}

TEST_CASE("EndsWith", "[String]")
{
    const StringUtf8 str("Hello there");
    const StringUtf8 suffix("there");
    const StringUtf8 suffix_empty;
    REQUIRE(Opal::EndsWith(str, suffix));
    REQUIRE(Opal::EndsWith(str, suffix_empty));
    REQUIRE(Opal::EndsWith<StringUtf8>(str, "re"));
    REQUIRE(Opal::EndsWith<StringUtf8>(str, ""));
    REQUIRE(!Opal::EndsWith<StringUtf8>(str, "Hello there friends"));
}

TEST_CASE("Split", "[String]")
{
    const StringUtf8 str("Hello there");
    const StringUtf8 delimiter(" ");
    StringUtf8 first;
    StringUtf8 second;
    REQUIRE(Opal::Split(str, delimiter, first, second));
    REQUIRE(first == "Hello");
    REQUIRE(second == "there");
    first = "";
    second = "";
    REQUIRE(Opal::Split<StringUtf8>(str, " ", first, second));
    REQUIRE(first == "Hello");
    REQUIRE(second == "there");
    first = "";
    second = "";
    REQUIRE(!Opal::Split<StringUtf8>(str, "m", first, second));
    REQUIRE(first == "Hello there");
    REQUIRE(second == "");
    first = "";
    second = "";
    REQUIRE(!Opal::Split<StringUtf8>("", "m", first, second));
    REQUIRE(first == "");
    REQUIRE(second == "");
    first = "";
    second = "";
    REQUIRE(!Opal::Split<StringUtf8>("", "", first, second));
    REQUIRE(first == "");
    REQUIRE(second == "");
    first = "";
    second = "";
}

TEST_CASE("Reverse", "[String]")
{
    SECTION("Normal case")
    {
        StringUtf8 str("Hello there");
        REQUIRE_NOTHROW(str.Reverse());
        REQUIRE(str == "ereht olleH");
    }
    SECTION("Empty case")
    {
        StringUtf8 empty;
        REQUIRE_NOTHROW(empty.Reverse());
        REQUIRE(empty == "");
    }
    SECTION("Reverse substring")
    {
        StringUtf8 str("Hello there");
        REQUIRE_NOTHROW(str.Reverse(str.begin() + 6, str.end()));
        REQUIRE(str == "Hello ereht");
    }
    SECTION("Reverse even number of characters")
    {
        StringUtf8 str("cube");
        REQUIRE_NOTHROW(str.Reverse());
        REQUIRE(str == "ebuc");
    }
}

TEST_CASE("Make string from i64", "[String]")
{
    SECTION("Binary")
    {
        StringUtf8 str = NumberToString(5ll, NumberSystemBase::Binary);
        REQUIRE(str == "101");
        str = NumberToString(-5ll, NumberSystemBase::Binary);
        REQUIRE(str == "1111111111111111111111111111111111111111111111111111111111111011");
        str = NumberToString(12345678ll, NumberSystemBase::Binary);
        REQUIRE(str == "101111000110000101001110");
        str = NumberToString(-12345678ll, NumberSystemBase::Binary);
        REQUIRE(str == "1111111111111111111111111111111111111111010000111001111010110010");
        str = NumberToString(std::numeric_limits<i64>::min(), NumberSystemBase::Binary);
        REQUIRE(str == "1000000000000000000000000000000000000000000000000000000000000000");
        str = NumberToString(std::numeric_limits<i64>::max(), NumberSystemBase::Binary);
        REQUIRE(str == "111111111111111111111111111111111111111111111111111111111111111");
        str = NumberToString(0ll, NumberSystemBase::Binary);
        REQUIRE(str == "0");
        str = NumberToString(-0ll, NumberSystemBase::Binary);
        REQUIRE(str == "0");
    }
    SECTION("Octal")
    {
        StringUtf8 str = NumberToString(5ll, NumberSystemBase::Octal);
        REQUIRE(str == "5");
        str = NumberToString(-5ll, NumberSystemBase::Octal);
        REQUIRE(str == "1777777777777777777773");
        str = NumberToString(12345678ll, NumberSystemBase::Octal);
        REQUIRE(str == "57060516");
        str = NumberToString(-12345678ll, NumberSystemBase::Octal);
        REQUIRE(str == "1777777777777720717262");
        str = NumberToString(std::numeric_limits<i64>::min(), NumberSystemBase::Octal);
        REQUIRE(str == "1000000000000000000000");
        str = NumberToString(std::numeric_limits<i64>::max(), NumberSystemBase::Octal);
        REQUIRE(str == "777777777777777777777");
        str = NumberToString(0ll, NumberSystemBase::Octal);
        REQUIRE(str == "0");
        str = NumberToString(-0ll, NumberSystemBase::Octal);
        REQUIRE(str == "0");
    }
    SECTION("Decimal")
    {
        StringUtf8 str = NumberToString(5ll);
        REQUIRE(str == "5");
        str = NumberToString(-5ll);
        REQUIRE(str == "-5");
        str = NumberToString(12345678ll);
        REQUIRE(str == "12345678");
        str = NumberToString(-12345678ll);
        REQUIRE(str == "-12345678");
        str = NumberToString(std::numeric_limits<i64>::min());
        REQUIRE(str == "-9223372036854775808");
        str = NumberToString(std::numeric_limits<i64>::max());
        REQUIRE(str == "9223372036854775807");
        str = NumberToString(0ll);
        REQUIRE(str == "0");
        str = NumberToString(-0ll);
        REQUIRE(str == "0");
    }
    SECTION("Hexadecimal")
    {
        StringUtf8 str = NumberToString(5ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "5");
        str = NumberToString<i64>(-5ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FFFFFFFFFFFFFFFB");
        str = NumberToString(12345678ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "BC614E");
        str = NumberToString(-12345678ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FFFFFFFFFF439EB2");
        str = NumberToString(std::numeric_limits<i64>::min(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "8000000000000000");
        str = NumberToString(std::numeric_limits<i64>::max(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "7FFFFFFFFFFFFFFF");
        str = NumberToString(0ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
        str = NumberToString(-0ll, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
    }
}

TEST_CASE("Make string from i32", "[String]")
{
    SECTION("Binary")
    {
        StringUtf8 str = NumberToString(5, NumberSystemBase::Binary);
        REQUIRE(str == "101");
        str = NumberToString(-5, NumberSystemBase::Binary);
        REQUIRE(str == "11111111111111111111111111111011");
        str = NumberToString(12345678, NumberSystemBase::Binary);
        REQUIRE(str == "101111000110000101001110");
        str = NumberToString(-12345678, NumberSystemBase::Binary);
        REQUIRE(str == "11111111010000111001111010110010");
        str = NumberToString(std::numeric_limits<i32>::min(), NumberSystemBase::Binary);
        REQUIRE(str == "10000000000000000000000000000000");
        str = NumberToString(std::numeric_limits<i32>::max(), NumberSystemBase::Binary);
        REQUIRE(str == "1111111111111111111111111111111");
        str = NumberToString(0, NumberSystemBase::Binary);
        REQUIRE(str == "0");
        str = NumberToString(-0, NumberSystemBase::Binary);
        REQUIRE(str == "0");
    }
    SECTION("Octal")
    {
        StringUtf8 str = NumberToString(5, NumberSystemBase::Octal);
        REQUIRE(str == "5");
        str = NumberToString(-5, NumberSystemBase::Octal);
        REQUIRE(str == "37777777773");
        str = NumberToString(12345678, NumberSystemBase::Octal);
        REQUIRE(str == "57060516");
        str = NumberToString(-12345678, NumberSystemBase::Octal);
        REQUIRE(str == "37720717262");
        str = NumberToString(std::numeric_limits<i32>::min(), NumberSystemBase::Octal);
        REQUIRE(str == "20000000000");
        str = NumberToString(std::numeric_limits<i32>::max(), NumberSystemBase::Octal);
        REQUIRE(str == "17777777777");
        str = NumberToString(0, NumberSystemBase::Octal);
        REQUIRE(str == "0");
        str = NumberToString(-0, NumberSystemBase::Octal);
        REQUIRE(str == "0");
    }
    SECTION("Decimal")
    {
        StringUtf8 str = NumberToString(5);
        REQUIRE(str == "5");
        str = NumberToString(-5);
        REQUIRE(str == "-5");
        str = NumberToString(12345678);
        REQUIRE(str == "12345678");
        str = NumberToString(-12345678);
        REQUIRE(str == "-12345678");
        str = NumberToString(std::numeric_limits<i32>::min());
        REQUIRE(str == "-2147483648");
        str = NumberToString(std::numeric_limits<i32>::max());
        REQUIRE(str == "2147483647");
        str = NumberToString(0);
        REQUIRE(str == "0");
        str = NumberToString(-0);
        REQUIRE(str == "0");
    }
    SECTION("Hexadecimal")
    {
        StringUtf8 str = NumberToString(5, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "5");
        str = NumberToString(-5, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FFFFFFFB");
        str = NumberToString(12345678, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "BC614E");
        str = NumberToString(-12345678, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FF439EB2");
        str = NumberToString(std::numeric_limits<i32>::min(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "80000000");
        str = NumberToString(std::numeric_limits<i32>::max(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "7FFFFFFF");
        str = NumberToString(0, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
        str = NumberToString(-0, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
    }
}

TEST_CASE("Make string from u64", "[String]")
{
    SECTION("Binary")
    {
        StringUtf8 str = NumberToString(5ull, NumberSystemBase::Binary);
        REQUIRE(str == "101");
        str = NumberToString(12345678ull, NumberSystemBase::Binary);
        REQUIRE(str == "101111000110000101001110");
        str = NumberToString(std::numeric_limits<u64>::max(), NumberSystemBase::Binary);
        REQUIRE(str == "1111111111111111111111111111111111111111111111111111111111111111");
        str = NumberToString(0ull, NumberSystemBase::Binary);
        REQUIRE(str == "0");
    }
    SECTION("Octal")
    {
        StringUtf8 str = NumberToString(5ull, NumberSystemBase::Octal);
        REQUIRE(str == "5");
        str = NumberToString(12345678ull, NumberSystemBase::Octal);
        REQUIRE(str == "57060516");
        str = NumberToString(std::numeric_limits<u64>::max(), NumberSystemBase::Octal);
        REQUIRE(str == "1777777777777777777777");
        str = NumberToString(0ull, NumberSystemBase::Octal);
        REQUIRE(str == "0");
    }
    SECTION("Decimal")
    {
        StringUtf8 str = NumberToString(5ull);
        REQUIRE(str == "5");
        str = NumberToString(12345678ull);
        REQUIRE(str == "12345678");
        str = NumberToString(std::numeric_limits<u64>::max());
        REQUIRE(str == "18446744073709551615");
        str = NumberToString(0ull);
        REQUIRE(str == "0");
    }
    SECTION("Hexadecimal")
    {
        StringUtf8 str = NumberToString(5ull, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "5");
        str = NumberToString(12345678ull, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "BC614E");
        str = NumberToString(std::numeric_limits<u64>::max(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FFFFFFFFFFFFFFFF");
        str = NumberToString(0ull, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
    }
}

TEST_CASE("Make string from u32", "[String]")
{
    SECTION("Binary")
    {
        StringUtf8 str = NumberToString(5u, NumberSystemBase::Binary);
        REQUIRE(str == "101");
        str = NumberToString(12345678u, NumberSystemBase::Binary);
        REQUIRE(str == "101111000110000101001110");
        str = NumberToString(std::numeric_limits<u32>::max(), NumberSystemBase::Binary);
        REQUIRE(str == "11111111111111111111111111111111");
        str = NumberToString(0u, NumberSystemBase::Binary);
        REQUIRE(str == "0");
    }
    SECTION("Octal")
    {
        StringUtf8 str = NumberToString(5u, NumberSystemBase::Octal);
        REQUIRE(str == "5");
        str = NumberToString(12345678u, NumberSystemBase::Octal);
        REQUIRE(str == "57060516");
        str = NumberToString(std::numeric_limits<u32>::max(), NumberSystemBase::Octal);
        REQUIRE(str == "37777777777");
        str = NumberToString(0u, NumberSystemBase::Octal);
        REQUIRE(str == "0");
    }
    SECTION("Decimal")
    {
        StringUtf8 str = NumberToString(5u);
        REQUIRE(str == "5");
        str = NumberToString(12345678u);
        REQUIRE(str == "12345678");
        str = NumberToString(std::numeric_limits<u32>::max());
        REQUIRE(str == "4294967295");
        str = NumberToString(0u);
        REQUIRE(str == "0");
    }
    SECTION("Hexadecimal")
    {
        StringUtf8 str = NumberToString(5u, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "5");
        str = NumberToString(12345678u, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "BC614E");
        str = NumberToString(std::numeric_limits<u32>::max(), NumberSystemBase::Hexadecimal);
        REQUIRE(str == "FFFFFFFF");
        str = NumberToString(0u, NumberSystemBase::Hexadecimal);
        REQUIRE(str == "0");
    }
}

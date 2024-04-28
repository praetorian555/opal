#include "catch2/catch2.hpp"

#include <iostream>

#include "opal/container/string.h"

using namespace Opal;

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

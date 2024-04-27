#include "catch2/catch2.hpp"

#include <string>
#include <iostream>

#include <Windows.h>

#include "opal/container/string.h"

using namespace Opal;

TEST_CASE("From UTF8 to UTF32", "[String]")
{
    String<c8> utf8(u8"での日本語文字コードを扱うために使用されている従来の");
    String<c32> utf32_ref(U"での日本語文字コードを扱うために使用されている従来の");

    String<c32> result = Transcode<EncodingUtf32<c32>, EncodingUtf8<c8>, String<c32>>(utf8).GetValue();
    REQUIRE(result == utf32_ref);
}

TEST_CASE("From UTF32 to UTF8", "[String]")
{
    String<c8> utf8(u8"での日本語文字コードを扱うために使用されている従来の");
    String<c32> utf32(U"での日本語文字コードを扱うために使用されている従来の");

    String<c8> result = Transcode<EncodingUtf8<c8>, EncodingUtf32<c32>, String<c8>>(utf32).GetValue();
    REQUIRE(result == utf8);
}


#include <string.h>

#include "test-helpers.h"

#include "opal/container/string-encoding.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"

using namespace Opal;

namespace
{
/** Decodes the first code point of some bytes with an encoding of 8-bit code units. */
template <typename Encoding>
ErrorCode DecodeFirst(std::initializer_list<u8> bytes, uchar32& out_code_point, u64& out_consumed)
{
    char8 units[8] = {};
    u64 count = 0;
    for (const u8 byte : bytes)
    {
        units[count++] = static_cast<char8>(byte);
    }
    ArrayView<const char8> input(units, count);
    Encoding encoding;
    const ErrorCode error = encoding.DecodeOne(input, out_code_point);
    out_consumed = count - input.GetSize();
    return error;
}

ErrorCode DecodeUtf8(std::initializer_list<u8> bytes, uchar32& out_code_point, u64& out_consumed)
{
    return DecodeFirst<EncodingUtf8<char8>>(bytes, out_code_point, out_consumed);
}

ErrorCode DecodeUtf16(std::initializer_list<u32> units, uchar32& out_code_point, u64& out_consumed)
{
    char16 buffer[4] = {};
    u64 count = 0;
    for (const u32 unit : units)
    {
        buffer[count++] = static_cast<char16>(unit);
    }
    ArrayView<const char16> input(buffer, count);
    EncodingUtf16LE<char16> encoding;
    const ErrorCode error = encoding.DecodeOne(input, out_code_point);
    out_consumed = count - input.GetSize();
    return error;
}
}  // namespace

TEST_CASE("UTF-8 decodes the shortest form of every code point at the edges of each length", "[StringEncoding]")
{
    struct Case
    {
        std::initializer_list<u8> bytes;
        uchar32 code_point;
    };
    const Case cases[] = {
        {{0x00}, 0x00},
        {{0x7F}, 0x7F},
        {{0xC2, 0x80}, 0x80},
        {{0xDF, 0xBF}, 0x7FF},
        {{0xE0, 0xA0, 0x80}, 0x800},
        {{0xED, 0x9F, 0xBF}, 0xD7FF},
        {{0xEE, 0x80, 0x80}, 0xE000},
        {{0xEF, 0xBF, 0xBD}, 0xFFFD},
        {{0xEF, 0xBF, 0xBF}, 0xFFFF},
        {{0xF0, 0x90, 0x80, 0x80}, 0x10000},
        {{0xF4, 0x8F, 0xBF, 0xBF}, 0x10FFFF},
    };
    for (const Case& test : cases)
    {
        uchar32 code_point = 0;
        u64 consumed = 0;
        REQUIRE(DecodeUtf8(test.bytes, code_point, consumed) == ErrorCode::Success);
        REQUIRE(code_point == test.code_point);
        REQUIRE(consumed == test.bytes.size());
    }
}

TEST_CASE("UTF-8 refuses what is not well formed and does not move past it", "[StringEncoding]")
{
    const std::initializer_list<u8> invalid[] = {
        {0x80},                    // A continuation byte on its own.
        {0xBF, 0x41},              // Another.
        {0xC0, 0x80},              // An overlong NUL.
        {0xC1, 0xBF},              // An overlong U+007F.
        {0xE0, 0x80, 0x80},        // An overlong three-byte form.
        {0xE0, 0x9F, 0xBF},        // An overlong U+07FF.
        {0xF0, 0x8F, 0xBF, 0xBF},  // An overlong U+FFFF.
        {0xED, 0xA0, 0x80},        // The high surrogate U+D800.
        {0xED, 0xBF, 0xBF},        // The low surrogate U+DFFF.
        {0xF4, 0x90, 0x80, 0x80},  // U+110000.
        {0xF5, 0x80, 0x80, 0x80},  // Lead bytes that start no sequence.
        {0xF8, 0x88, 0x80, 0x80},
        {0xFF},
        {0xE2, 0x28, 0xA1},  // A second byte that is not a continuation.
        {0xE2, 0x82, 0x28},  // A third.
        {0xC3, 0xC3},        // A lead byte where a continuation belongs.
    };
    for (const std::initializer_list<u8>& bytes : invalid)
    {
        uchar32 code_point = 0x1234;
        u64 consumed = 99;
        REQUIRE(DecodeUtf8(bytes, code_point, consumed) == ErrorCode::InvalidArgument);
        REQUIRE(consumed == 0);
        REQUIRE(code_point == 0x1234);
    }

    // A sequence that is well formed as far as it goes reports that it is cut short.
    uchar32 code_point = 0;
    u64 consumed = 0;
    REQUIRE(DecodeUtf8({0xE2, 0x82}, code_point, consumed) == ErrorCode::IncompleteSequence);
    REQUIRE(DecodeUtf8({0xF0, 0x9F, 0x98}, code_point, consumed) == ErrorCode::IncompleteSequence);
    REQUIRE(consumed == 0);
    REQUIRE(DecodeUtf8({}, code_point, consumed) == ErrorCode::EndOfString);
}

TEST_CASE("UTF-8 does not encode a surrogate or a code point past U+10FFFF", "[StringEncoding]")
{
    char8 buffer[4] = {};
    EncodingUtf8<char8> encoding;
    for (const uchar32 code_point : {uchar32{0xD800}, uchar32{0xDFFF}, uchar32{0x110000}})
    {
        ArrayView<char8> output(buffer, 4);
        REQUIRE(encoding.EncodeOne(code_point, output) == ErrorCode::InvalidArgument);
        REQUIRE(output.GetSize() == 4);
    }
}

TEST_CASE("UTF-16 decodes the whole Basic Multilingual Plane, surrogate pairs, and refuses a surrogate on its own", "[StringEncoding]")
{
    uchar32 code_point = 0;
    u64 consumed = 0;
    for (const u32 unit : {0x41u, 0xD7FFu, 0xE000u, 0xFEFFu, 0xFFFDu, 0xFFFFu})
    {
        REQUIRE(DecodeUtf16({unit}, code_point, consumed) == ErrorCode::Success);
        REQUIRE(code_point == unit);
        REQUIRE(consumed == 1);
    }
    REQUIRE(DecodeUtf16({0xD83D, 0xDE00}, code_point, consumed) == ErrorCode::Success);
    REQUIRE(code_point == 0x1F600);
    REQUIRE(consumed == 2);
    REQUIRE(DecodeUtf16({0xDBFF, 0xDFFF}, code_point, consumed) == ErrorCode::Success);
    REQUIRE(code_point == 0x10FFFF);

    REQUIRE(DecodeUtf16({0xDC00}, code_point, consumed) == ErrorCode::InvalidArgument);
    REQUIRE(consumed == 0);
    REQUIRE(DecodeUtf16({0xD800, 0x41}, code_point, consumed) == ErrorCode::InvalidArgument);
    REQUIRE(DecodeUtf16({0xD800, 0xD800}, code_point, consumed) == ErrorCode::InvalidArgument);
    REQUIRE(DecodeUtf16({0xD800}, code_point, consumed) == ErrorCode::IncompleteSequence);
    REQUIRE(consumed == 0);
}

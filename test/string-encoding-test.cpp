#include <string.h>

#include "test-helpers.h"

#include "opal/container/string-encoding.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"

using namespace Opal;

namespace
{
using StringViewWindows1250 = StringView<char8, EncodingWindows1250>;
using StringViewWindows1251 = StringView<char8, EncodingWindows1251>;
using StringViewWindows1252 = StringView<char8, EncodingWindows1252>;
using StringWindows1252 = String<char8, EncodingWindows1252>;

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

TEST_CASE("Windows code pages decode their letters and refuse the bytes they leave unassigned", "[StringEncoding]")
{
    struct Case
    {
        u8 byte;
        uchar32 code_point;
    };
    uchar32 code_point = 0;
    u64 consumed = 0;

    // Croatian and Serbian Latin letters, and the euro.
    const Case windows_1250[] = {{0x41, 'A'},    {0x80, 0x20AC}, {0x8A, 0x0160}, {0x8E, 0x017D}, {0x9A, 0x0161},
                                 {0x9E, 0x017E}, {0xC6, 0x0106}, {0xC8, 0x010C}, {0xD0, 0x0110}, {0xE6, 0x0107},
                                 {0xE8, 0x010D}, {0xF0, 0x0111}, {0xA0, 0x00A0}, {0xFF, 0x02D9}};
    for (const Case& test : windows_1250)
    {
        REQUIRE(DecodeFirst<EncodingWindows1250>({test.byte}, code_point, consumed) == ErrorCode::Success);
        REQUIRE(code_point == test.code_point);
        REQUIRE(consumed == 1);
    }
    // Serbian Cyrillic letters, the alphabet's ends, and the euro.
    const Case windows_1251[] = {{0x80, 0x0402}, {0x90, 0x0452}, {0x8A, 0x0409}, {0x8C, 0x040A}, {0xA3, 0x0408}, {0xBC, 0x0458},
                                 {0xC0, 0x0410}, {0xFF, 0x044F}, {0xA8, 0x0401}, {0xB8, 0x0451}, {0x88, 0x20AC}};
    for (const Case& test : windows_1251)
    {
        REQUIRE(DecodeFirst<EncodingWindows1251>({test.byte}, code_point, consumed) == ErrorCode::Success);
        REQUIRE(code_point == test.code_point);
    }
    const Case windows_1252[] = {{0x80, 0x20AC}, {0x92, 0x2019}, {0x9F, 0x0178}, {0xA0, 0x00A0}, {0xE9, 0x00E9}, {0xFF, 0x00FF}};
    for (const Case& test : windows_1252)
    {
        REQUIRE(DecodeFirst<EncodingWindows1252>({test.byte}, code_point, consumed) == ErrorCode::Success);
        REQUIRE(code_point == test.code_point);
    }

    for (const u8 byte : std::initializer_list<u8>{0x81, 0x83, 0x88, 0x90, 0x98})
    {
        REQUIRE(DecodeFirst<EncodingWindows1250>({byte}, code_point, consumed) == ErrorCode::InvalidArgument);
        REQUIRE(consumed == 0);
    }
    REQUIRE(DecodeFirst<EncodingWindows1251>({0x98}, code_point, consumed) == ErrorCode::InvalidArgument);
    for (const u8 byte : std::initializer_list<u8>{0x81, 0x8D, 0x8F, 0x90, 0x9D})
    {
        REQUIRE(DecodeFirst<EncodingWindows1252>({byte}, code_point, consumed) == ErrorCode::InvalidArgument);
    }
    REQUIRE(DecodeFirst<EncodingWindows1252>({}, code_point, consumed) == ErrorCode::EndOfString);
}

TEST_CASE("Windows code pages encode every character they decode back to its byte", "[StringEncoding]")
{
    const auto round_trip = [](auto encoding)
    {
        u64 assigned = 0;
        for (u32 byte = 0; byte < 256; ++byte)
        {
            const char8 unit = static_cast<char8>(static_cast<u8>(byte));
            ArrayView<const char8> input(&unit, 1);
            uchar32 code_point = 0;
            if (encoding.DecodeOne(input, code_point) != ErrorCode::Success)
            {
                continue;
            }
            ++assigned;
            char8 encoded = 0;
            ArrayView<char8> output(&encoded, 1);
            REQUIRE(encoding.EncodeOne(code_point, output) == ErrorCode::Success);
            REQUIRE(static_cast<u8>(encoded) == byte);
            REQUIRE(output.GetSize() == 0);
        }
        return assigned;
    };
    REQUIRE(round_trip(EncodingWindows1250()) == 251);
    REQUIRE(round_trip(EncodingWindows1251()) == 255);
    REQUIRE(round_trip(EncodingWindows1252()) == 251);

    char8 encoded = 0;
    ArrayView<char8> output(&encoded, 1);
    EncodingWindows1252 windows_1252;
    REQUIRE(windows_1252.EncodeOne(0x4E00, output) == ErrorCode::InvalidArgument);
    REQUIRE(windows_1252.EncodeOne(0x0410, output) == ErrorCode::InvalidArgument);
    ArrayView<char8> full(&encoded, 0);
    REQUIRE(windows_1252.EncodeOne('A', full) == ErrorCode::InsufficientSpace);
}

TEST_CASE("Transcode reads a code page through a view into UTF-8", "[StringEncoding]")
{
    // "Ćao, đače" in Windows-1250 and "Здраво" in Windows-1251.
    const char8 latin[] =
        "\xC6"
        "ao, \xF0"
        "a\xE8"
        "e";
    StringUtf8 utf8;
    REQUIRE(Transcode(StringViewWindows1250(latin, sizeof(latin) - 1), utf8) == ErrorCode::Success);
    REQUIRE(utf8 ==
            "\xC4\x86"
            "ao, \xC4\x91"
            "a\xC4\x8D"
            "e");

    const char8 cyrillic[] = "\xC7\xE4\xF0\xE0\xE2\xEE";
    REQUIRE(Transcode(StringViewWindows1251(cyrillic, sizeof(cyrillic) - 1), utf8) == ErrorCode::Success);
    REQUIRE(utf8 == "\xD0\x97\xD0\xB4\xD1\x80\xD0\xB0\xD0\xB2\xD0\xBE");

    StringWindows1252 western;
    REQUIRE(Transcode(StringViewUtf8("caf\xC3\xA9 \xE2\x82\xAC"), western) == ErrorCode::Success);
    REQUIRE(western == "caf\xE9 \x80");
}

TEST_CASE("Transcode fails on input that does not decode, or replaces it when asked", "[StringEncoding]")
{
    StringUtf8 utf8;
    // A stray continuation byte, an overlong form of two bytes, a surrogate of three, then a sequence cut short at the end.
    const StringViewUtf8 broken(
        "a\x80"
        "b\xC0\xAF"
        "c\xED\xA0\x80"
        "d\xF0\x9F\x98");
    REQUIRE(Transcode(broken, utf8) == ErrorCode::InvalidArgument);
    REQUIRE(Transcode(broken, utf8, InvalidSequencePolicy::Replace) == ErrorCode::Success);
    REQUIRE(utf8 ==
            "a\xEF\xBF\xBD"
            "b\xEF\xBF\xBD\xEF\xBF\xBD"
            "c\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"
            "d\xEF\xBF\xBD");

    const char8 unassigned[] = "x\x81y";
    REQUIRE(Transcode(StringViewWindows1252(unassigned, 3), utf8) == ErrorCode::InvalidArgument);
    REQUIRE(Transcode(StringViewWindows1252(unassigned, 3), utf8, InvalidSequencePolicy::Replace) == ErrorCode::Success);
    REQUIRE(utf8 == "x\xEF\xBF\xBDy");

    // A character the output cannot hold, where the output has no U+FFFD either, becomes '?'.
    StringWindows1252 western;
    REQUIRE(Transcode(StringViewUtf8("\xE6\x97\xA5"
                                     "!"),
                      western) == ErrorCode::InvalidArgument);
    REQUIRE(Transcode(StringViewUtf8("\xE6\x97\xA5"
                                     "!"),
                      western, InvalidSequencePolicy::Replace) == ErrorCode::Success);
    REQUIRE(western == "?!");
}

#pragma once

#include <uchar.h>

#include "opal/container/array-view.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{
enum class NumberSystemBase;
}
namespace Opal
{

template <typename EncodingType>
concept DecodableEncoding =
    requires(EncodingType& e, ArrayView<const typename EncodingType::CodeUnitType>& in, typename EncodingType::CodePointType& out) {
        { e.DecodeOne(in, out) } -> SameAs<ErrorCode>;
    };

template <typename EncodingType>
concept EncodableEncoding =
    requires(EncodingType& e, typename EncodingType::CodePointType& in, ArrayView<typename EncodingType::CodeUnitType>& out) {
        { e.EncodeOne(in, out) } -> SameAs<ErrorCode>;
    };

template <typename CodeUnitT>
struct EncodingUtf8
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = uchar32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span, ErrorCode::InvalidArgument if the `in_code_point` is a surrogate or above U+10FFFF.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`. Only
     * well-formed UTF-8 decodes: the shortest form of a code point up to U+10FFFF that is not a surrogate.
     * @param input Input span. Modified to point after decoded data; left as it was when decoding fails.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::IncompleteSequence if the input ends inside a
     * sequence that is well-formed so far, ErrorCode::InvalidArgument if the data is not well-formed UTF-8, ErrorCode::EndOfString if
     * there is no more data to decode.
     */
    ErrorCode DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point);
};

template <typename CodeUnitT>
struct EncodingUtf16LE
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = uchar32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span, ErrorCode::InvalidArgument if the `in_code_point` is unsupported code point.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`. The code
     * units are 16-bit values, so their byte order is the reader's concern.
     * @param input Input span. Modified to point after decoded data; left as it was when decoding fails.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::IncompleteSequence if the input ends after a high
     * surrogate, ErrorCode::InvalidArgument for a high surrogate not followed by a low one or a low surrogate on its own,
     * ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point);
};

template <typename CodeUnitT>
struct EncodingUtf32LE
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = uchar32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point);
};

struct EncodingLocale
{
    using CodeUnitType = char;
    using CodePointType = uchar32;

    EncodingLocale();

    /**
     * Encodes given code point into an output span using system encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitType>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::InvalidArgument if the decoded data does not match rules
     * for the current encoding, ErrorCode::IncompleteSequence if there is not enough input data to decode current code point,
     * ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(ArrayView<const CodeUnitType>& input, CodePointType& out_code_point);

private:
    mbstate_t m_encoding_state;
    mbstate_t m_decoding_state;
};

namespace Impl
{
/** The code points a single-byte code page gives bytes 0x80 to 0xFF, 0 where it gives none. */
using HighHalfTable = uchar32[128];

ErrorCode EncodeSingleByte(const HighHalfTable& table, uchar32 in_code_point, ArrayView<char8>& output);
ErrorCode DecodeSingleByte(const HighHalfTable& table, ArrayView<const char8>& input, uchar32& out_code_point);

extern const HighHalfTable g_windows_1250_table;
extern const HighHalfTable g_windows_1251_table;
extern const HighHalfTable g_windows_1252_table;
}  // namespace Impl

/**
 * A Windows code page of one byte a character: ASCII below 0x80, and from 0x80 the characters the code page assigns. The bytes
 * the code page leaves unassigned do not decode, and a code point it has no byte for does not encode; both report
 * ErrorCode::InvalidArgument. Decoding never reports ErrorCode::IncompleteSequence.
 */
template <const Impl::HighHalfTable& Table>
struct EncodingSingleByte
{
    using CodeUnitType = char8;
    using CodePointType = uchar32;

    /**
     * Encodes a code point as its byte.
     * @param in_code_point Input code point.
     * @param output Output span. Updated to point after the byte.
     * @return ErrorCode::Success, ErrorCode::InsufficientSpace when `output` is empty, or ErrorCode::InvalidArgument for a code
     * point the code page has no byte for.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitType>& output)
    {
        return Impl::EncodeSingleByte(Table, in_code_point, output);
    }

    /**
     * Decodes one byte.
     * @param input Input span. Updated to point after the byte; left as it was when decoding fails.
     * @param out_code_point Output code point.
     * @return ErrorCode::Success, ErrorCode::InvalidArgument for a byte the code page leaves unassigned, or ErrorCode::EndOfString
     * when `input` is empty.
     */
    ErrorCode DecodeOne(ArrayView<const CodeUnitType>& input, CodePointType& out_code_point)
    {
        return Impl::DecodeSingleByte(Table, input, out_code_point);
    }
};

/** Windows-1250: Central and Eastern European languages in the Latin script - Polish, Czech, Slovak, Hungarian, Croatian, Serbian. */
using EncodingWindows1250 = EncodingSingleByte<Impl::g_windows_1250_table>;
/** Windows-1251: languages in the Cyrillic script - Russian, Ukrainian, Bulgarian, Serbian, Macedonian. */
using EncodingWindows1251 = EncodingSingleByte<Impl::g_windows_1251_table>;
/** Windows-1252: Western European languages, a superset of ISO 8859-1's printable characters. */
using EncodingWindows1252 = EncodingSingleByte<Impl::g_windows_1252_table>;

}  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf8<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, ArrayView<CodeUnitT>& output)
{
    auto out_begin = output.begin();
    auto out_end = output.end();
    typename ArrayView<CodeUnitT>::difference_type out_size = out_end - out_begin;
    if (in_code_point <= 0x7F)
    {
        if (out_size < 1)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(in_code_point);
        output = ArrayView<CodeUnitT>(out_begin + 1, out_end);
        return ErrorCode::Success;
    }
    if (in_code_point <= 0x7FF)
    {
        if (out_size < 2)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(0xC0 | ((in_code_point >> 6) & 0x1F));
        output[1] = static_cast<CodeUnitType>(0x80 | (in_code_point & 0x3F));
        output = ArrayView<CodeUnitT>(out_begin + 2, out_end);
        return ErrorCode::Success;
    }
    if (in_code_point <= 0xFFFF)
    {
        if (in_code_point >= 0xD800 && in_code_point <= 0xDFFF)
        {
            return ErrorCode::InvalidArgument;
        }
        if (out_size < 3)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(0xE0 | ((in_code_point >> 12) & 0x0F));
        output[1] = static_cast<CodeUnitType>(0x80 | ((in_code_point >> 6) & 0x3F));
        output[2] = static_cast<CodeUnitType>(0x80 | (in_code_point & 0x3F));
        output = ArrayView<CodeUnitT>(out_begin + 3, out_end);
        return ErrorCode::Success;
    }
    if (in_code_point <= 0x10FFFF)
    {
        if (out_size < 4)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(0xF0 | ((in_code_point >> 18) & 0x07));
        output[1] = static_cast<CodeUnitType>(0x80 | ((in_code_point >> 12) & 0x3F));
        output[2] = static_cast<CodeUnitType>(0x80 | ((in_code_point >> 6) & 0x3F));
        output[3] = static_cast<CodeUnitType>(0x80 | (in_code_point & 0x3F));
        output = ArrayView<CodeUnitT>(out_begin + 4, out_end);
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point)
{
    auto in_begin = input.begin();
    auto in_end = input.end();
    auto in_size = in_end - in_begin;
    if (in_size == 0)
    {
        return ErrorCode::EndOfString;
    }
    const u8 lead = static_cast<u8>(input[0]);
    if (lead < 0x80)
    {
        out_code_point = static_cast<CodePointType>(lead);
        input = ArrayView<const CodeUnitT>(in_begin + 1, in_end);
        return ErrorCode::Success;
    }
    i64 length = 0;
    CodePointType code_point = 0;
    CodePointType least = 0;
    if (lead >= 0xC2 && lead <= 0xDF)
    {
        length = 2;
        code_point = lead & 0x1F;
        least = 0x80;
    }
    else if (lead >= 0xE0 && lead <= 0xEF)
    {
        length = 3;
        code_point = lead & 0x0F;
        least = 0x800;
    }
    else if (lead >= 0xF0 && lead <= 0xF4)
    {
        length = 4;
        code_point = lead & 0x07;
        least = 0x10000;
    }
    else
    {
        return ErrorCode::InvalidArgument;
    }
    const i64 available = in_size < length ? static_cast<i64>(in_size) : length;
    for (i64 i = 1; i < available; ++i)
    {
        const u8 next = static_cast<u8>(input[static_cast<u64>(i)]);
        if ((next & 0xC0) != 0x80)
        {
            return ErrorCode::InvalidArgument;
        }
        code_point = (code_point << 6) | (next & 0x3F);
    }
    if (available < length)
    {
        return ErrorCode::IncompleteSequence;
    }
    if (code_point < least || code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF))
    {
        return ErrorCode::InvalidArgument;
    }
    out_code_point = code_point;
    input = ArrayView<const CodeUnitT>(in_begin + length, in_end);
    return ErrorCode::Success;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf32LE<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, ArrayView<CodeUnitT>& output)
{
    if (output.GetSize() == 0)
    {
        return ErrorCode::InsufficientSpace;
    }
    output[0] = static_cast<CodeUnitT>(in_code_point);
    output = ArrayView<CodeUnitT>(output.begin() + 1, output.end());
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    out_code_point = static_cast<CodePointType>(input[0]);
    input = ArrayView<const CodeUnitT>(input.begin() + 1, input.end());
    return ErrorCode::Success;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf16LE<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, ArrayView<CodeUnitT>& output)
{
    if (output.GetSize() == 0)
    {
        return ErrorCode::InsufficientSpace;
    }
    if (in_code_point <= 0xFFFF)
    {
        if (in_code_point >= 0xD800 && in_code_point <= 0xDFFF)
        {
            // Surrogate values are not valid Unicode code points
            return ErrorCode::InvalidArgument;
        }
        output[0] = static_cast<CodeUnitT>(in_code_point);
        output = ArrayView<CodeUnitT>(output.begin() + 1, output.end());
        return ErrorCode::Success;
    }
    if (in_code_point <= 0x10FFFF)
    {
        if (output.GetSize() < 2)
        {
            return ErrorCode::InsufficientSpace;
        }
        const CodePointType code = in_code_point - 0x10000;
        const CodeUnitT high_surrogate = static_cast<CodeUnitT>((code >> 10) + 0xD800);
        const CodeUnitT low_surrogate = static_cast<CodeUnitT>((code & 0x3FF) + 0xDC00);
        output[0] = high_surrogate;
        output[1] = low_surrogate;
        output = ArrayView<CodeUnitT>(output.begin() + 2, output.end());
        return ErrorCode::Success;
    }
    return ErrorCode::InvalidArgument;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(ArrayView<const CodeUnitT>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    const CodePointType unit = static_cast<CodePointType>(input[0]) & 0xFFFF;
    if (unit < 0xD800 || unit > 0xDFFF)
    {
        out_code_point = unit;
        input = ArrayView<const CodeUnitT>(input.begin() + 1, input.end());
        return ErrorCode::Success;
    }
    if (unit <= 0xDBFF)
    {
        if (input.GetSize() < 2)
        {
            return ErrorCode::IncompleteSequence;
        }
        const CodePointType low = static_cast<CodePointType>(input[1]) & 0xFFFF;
        if (low >= 0xDC00 && low <= 0xDFFF)
        {
            out_code_point = ((unit - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
            input = ArrayView<const CodeUnitT>(input.begin() + 2, input.end());
            return ErrorCode::Success;
        }
    }
    return ErrorCode::InvalidArgument;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

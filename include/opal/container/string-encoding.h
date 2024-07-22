#pragma once

#include <cuchar>

#include "opal/export.h"
#include "opal/error-codes.h"
#include "opal/types.h"
#include "opal/container/span.h"

namespace Opal
{

template <typename CodeUnitT>
struct EncodingUtf8
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = c32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span, ErrorCode::BadInput if the `in_code_point` is unsupported code point.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, Span<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::IncompleteSequence if there is not enough input data
     * to decode current code point, ErrorCode::BadInput if the decoded data does not match rules for the current encoding,
     * ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point);
};

template <typename CodeUnitT>
struct EncodingUtf16LE
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = c32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span, ErrorCode::BadInput if the `in_code_point` is unsupported code point.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, Span<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::IncompleteSequence if there is not enough input data
     * to decode current code point, ErrorCode::BadInput if the decoded data does not match rules for the current encoding,
     * ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point);
};

template <typename CodeUnitT>
struct EncodingUtf32LE
{
    using CodeUnitType = CodeUnitT;
    using CodePointType = c32;

    /**
     * Encodes given code point into an output span using this encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, Span<CodeUnitT>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point);
};

struct OPAL_EXPORT EncodingLocale
{
    using CodeUnitType = char;
    using CodePointType = c32;

    /**
     * Encodes given code point into an output span using system encoding scheme.
     * @param in_code_point Input code point.
     * @param output Output span used to append result to. It will be update to point after the inserted data.
     * @return Returns ErrorCode::Success if encoding was successful, ErrorCode::InsufficientSpace if there is not enough space to
     * store encoding result in the `output` span.
     */
    ErrorCode EncodeOne(CodePointType in_code_point, Span<CodeUnitType>& output);

    /**
     * Decodes first sequence detected in the input span and stores the resulting code point into the `out_code_point`.
     * @param input Input span. Modified to point after decoded data.
     * @param out_code_point Output code point.
     * @return Returns ErrorCode::Success if the decoding was successful, ErrorCode::BadInput if the decoded data does not match rules
     * for the current encoding, ErrorCode::IncompleteSequence if there is not enough input data to decode current code point,
     * ErrorCode::EndOfString if there is no more data to decode.
     */
    ErrorCode DecodeOne(Span<const CodeUnitType>& input, CodePointType& out_code_point);

private:
    std::mbstate_t m_encoding_state = std::mbstate_t{};
    std::mbstate_t m_decoding_state = std::mbstate_t{};
};

}  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf8<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, Span<CodeUnitT>& output)
{
    auto out_begin = output.begin();
    auto out_end = output.end();
    typename Span<CodeUnitT>::DifferenceType out_size = out_end - out_begin;
    if (in_code_point <= 0x7F)
    {
        if (out_size < 1)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(in_code_point);
        output = Span<CodeUnitT>(out_begin + 1, out_end);
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
        output = Span<CodeUnitT>(out_begin + 2, out_end);
        return ErrorCode::Success;
    }
    if (in_code_point <= 0xFFFF)
    {
        if (out_size < 3)
        {
            return ErrorCode::InsufficientSpace;
        }
        output[0] = static_cast<CodeUnitType>(0xE0 | ((in_code_point >> 12) & 0x0F));
        output[1] = static_cast<CodeUnitType>(0x80 | ((in_code_point >> 6) & 0x3F));
        output[2] = static_cast<CodeUnitType>(0x80 | (in_code_point & 0x3F));
        output = Span<CodeUnitT>(out_begin + 3, out_end);
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
        output = Span<CodeUnitT>(out_begin + 4, out_end);
        return ErrorCode::Success;
    }
    return ErrorCode::BadInput;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point)
{
    auto in_begin = input.begin();
    auto in_end = input.end();
    auto in_size = in_end - in_begin;
    if (in_size == 0)
    {
        return ErrorCode::EndOfString;
    }
    if ((input[0] & 0x80) == 0)
    {
        out_code_point = input[0];
        input = Span<const CodeUnitT>(in_begin + 1, in_end);
        return ErrorCode::Success;
    }
    if ((input[0] & 0xF0) == 0xF0)
    {
        if (in_size < 4)
        {
            return ErrorCode::IncompleteSequence;
        }
        if ((input[1] & 0x80) != 0x80 || (input[2] & 0x80) != 0x80 || (input[3] & 0x80) != 0x80)
        {
            return ErrorCode::BadInput;
        }
        out_code_point = ((input[0] & 0x07) << 18) | ((input[1] & 0x3F) << 12) | ((input[2] & 0x3F) << 6) | (input[3] & 0x3F);
        input = Span<const CodeUnitT>(in_begin + 4, in_end);
        return ErrorCode::Success;
    }
    if ((input[0] & 0xE0) == 0xE0)
    {
        if (in_size < 3)
        {
            return ErrorCode::IncompleteSequence;
        }
        if ((input[1] & 0x80) != 0x80 || (input[2] & 0x80) != 0x80)
        {
            return ErrorCode::BadInput;
        }
        out_code_point = ((input[0] & 0x0F) << 12) | ((input[1] & 0x3F) << 6) | (input[2] & 0x3F);
        input = Span<const CodeUnitT>(in_begin + 3, in_end);
        return ErrorCode::Success;
    }
    if ((input[0] & 0xC0) == 0xC0)
    {
        if (in_size < 2)
        {
            return ErrorCode::IncompleteSequence;
        }
        if ((input[1] & 0x80) != 0x80)
        {
            return ErrorCode::BadInput;
        }
        out_code_point = ((input[0] & 0x1F) << 6) | (input[1] & 0x3F);
        input = Span<const CodeUnitT>(in_begin + 2, in_end);
        return ErrorCode::Success;
    }
    return ErrorCode::BadInput;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf32LE<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, Span<CodeUnitT>& output)
{
    if (output.GetSize() == 0)
    {
        return ErrorCode::InsufficientSpace;
    }
    output[0] = static_cast<CodeUnitT>(in_code_point);
    output = Span<CodeUnitT>(output.begin() + 1, output.end());
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    out_code_point = static_cast<CodePointType>(input[0]);
    input = Span<const CodeUnitT>(input.begin() + 1, input.end());
    return ErrorCode::Success;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename CodeUnitT>
#define CLASS_HEADER Opal::EncodingUtf16LE<CodeUnitT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::EncodeOne(typename CLASS_HEADER::CodePointType in_code_point, Span<CodeUnitT>& output)
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
            return ErrorCode::BadInput;
        }
        output[0] = static_cast<CodeUnitT>(in_code_point);
        output = Span<CodeUnitT>(output.begin() + 1, output.end());
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
        output = Span<CodeUnitT>(output.begin() + 2, output.end());
        return ErrorCode::Success;
    }
    return ErrorCode::BadInput;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::DecodeOne(Span<const CodeUnitT>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    if (input[0] < 0xD800)
    {
        out_code_point = static_cast<CodePointType>(input[0]);
        input = Span<const CodeUnitT>(input.begin() + 1, input.end());
        return ErrorCode::Success;
    }
    if (input[0] >= 0xD800 && input[0] <= 0xDBFF)
    {
        if (input.GetSize() < 2)
        {
            return ErrorCode::IncompleteSequence;
        }
        if (input[1] >= 0xDC00 && input[1] <= 0xDFFF)
        {
            out_code_point = ((input[0] - 0xD800) << 10) | (input[1] - 0xDC00);
            out_code_point += 0x10000;
            input = Span<const CodeUnitT>(input.begin() + 2, input.end());
            return ErrorCode::Success;
        }
        return ErrorCode::BadInput;
    }
    return ErrorCode::BadInput;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

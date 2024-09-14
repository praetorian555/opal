#include "opal/container/string-encoding.h"

#include <cstdlib>
#include <cuchar>
#include <cwchar>

Opal::EncodingLocale::EncodingLocale() : m_encoding_state(), m_decoding_state()
{
    OPAL_ASSERT(std::mbsinit(&m_encoding_state) != 0, "Encoding state is not initialized!");
    OPAL_ASSERT(std::mbsinit(&m_decoding_state) != 0, "Decoding state is not initialized!");
}

Opal::ErrorCode Opal::EncodingLocale::EncodeOne(CodePointType in_code_point, Span<CodeUnitType>& output)
{
    if (output.GetSize() < MB_CUR_MAX)
    {
        return ErrorCode::InsufficientSpace;
    }
    const size_t count = c32rtomb(output.GetData(), in_code_point, &m_encoding_state);
    if (count == static_cast<size_t>(-1))
    {
        return ErrorCode::BadInput;
    }
    output = Span<CodeUnitType>(output.begin() + static_cast<i64>(count), output.end());
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::EncodingLocale::DecodeOne(Span<const CodeUnitType>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    const size_t count = mbrtoc32(&out_code_point, input.GetData(), input.GetSize(), &m_decoding_state);
    if (count == 0)
    {
        // We've just written null character
        input = Span<const CodeUnitType>(input.begin() + 1, input.end());
        return ErrorCode::Success;
    }
    if (count == static_cast<size_t>(-1))
    {
        return ErrorCode::BadInput;
    }
    if (count == static_cast<size_t>(-2))
    {
        return ErrorCode::IncompleteSequence;
    }
    if (count == static_cast<size_t>(-3))
    {
        return ErrorCode::Success;
    }
    input = Span<const CodeUnitType>(input.begin() + static_cast<i64>(count), input.end());
    return ErrorCode::Success;
}

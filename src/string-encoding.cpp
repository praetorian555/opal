#include "opal/container/string-encoding.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

Opal::EncodingLocale::EncodingLocale() : m_encoding_state(), m_decoding_state()
{
    OPAL_ASSERT(mbsinit(&m_encoding_state) != 0, "Encoding state is not initialized!");
    OPAL_ASSERT(mbsinit(&m_decoding_state) != 0, "Decoding state is not initialized!");
}

Opal::ErrorCode Opal::EncodingLocale::EncodeOne(CodePointType in_code_point, ArrayView<CodeUnitType>& output)
{
    CodeUnitType buffer[MB_LEN_MAX];
    const mbstate_t saved_state = m_encoding_state;
    const size_t count = c32rtomb(buffer, in_code_point, &m_encoding_state);
    if (count == static_cast<size_t>(-1))
    {
        m_encoding_state = saved_state;
        return ErrorCode::InvalidArgument;
    }
    if (count > output.GetSize())
    {
        m_encoding_state = saved_state;
        return ErrorCode::InsufficientSpace;
    }
    if (count > 0)
    {
        memcpy(output.GetData(), buffer, count);
        output = ArrayView<CodeUnitType>(output.begin() + static_cast<i64>(count), output.end());
    }
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::EncodingLocale::DecodeOne(ArrayView<const CodeUnitType>& input, CodePointType& out_code_point)
{
    if (input.GetSize() == 0)
    {
        return ErrorCode::EndOfString;
    }
    const size_t count = mbrtoc32(&out_code_point, input.GetData(), input.GetSize(), &m_decoding_state);
    if (count == 0)
    {
        // We've just written null character
        input = ArrayView<const CodeUnitType>(input.begin() + 1, input.end());
        return ErrorCode::Success;
    }
    if (count == static_cast<size_t>(-1))
    {
        return ErrorCode::InvalidArgument;
    }
    if (count == static_cast<size_t>(-2))
    {
        return ErrorCode::IncompleteSequence;
    }
    if (count == static_cast<size_t>(-3))
    {
        return ErrorCode::Success;
    }
    input = ArrayView<const CodeUnitType>(input.begin() + static_cast<i64>(count), input.end());
    return ErrorCode::Success;
}

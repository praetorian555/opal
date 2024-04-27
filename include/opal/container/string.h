#pragma once

#include <cstring>

#include "opal/allocator.h"
#include "opal/container/span.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename CodeUnitType>
struct EncodingUtf8
{
    ErrorCode DecodeOne(Span<CodeUnitType>& input, u32& output)
    {
        if (input.GetSize() == 0)
        {
            return ErrorCode::EndOfString;
        }
        if ((input[0] & 0x80) == 0)
        {
            output = input[0];
            input = input.SubSpan(1, input.GetSize() - 1).GetValue();
            return ErrorCode::Success;
        }
        if ((input[0] & 0xF0) == 0xF0)
        {
            if (input.GetSize() < 4)
            {
                return ErrorCode::BadInput;
            }
            if ((input[1] & 0x80) != 0x80 || (input[2] & 0x80) != 0x80 || (input[3] & 0x80) != 0x80)
            {
                return ErrorCode::BadInput;
            }
            output = ((input[0] & 0x07) << 18) | ((input[1] & 0x3F) << 12) | ((input[2] & 0x3F) << 6) | (input[3] & 0x3F);
            input = input.SubSpan(4, input.GetSize() - 4).GetValue();
            return ErrorCode::Success;
        }
        if ((input[0] & 0xE0) == 0xE0)
        {
            if (input.GetSize() < 3)
            {
                return ErrorCode::BadInput;
            }
            if ((input[1] & 0x80) != 0x80 || (input[2] & 0x80) != 0x80)
            {
                return ErrorCode::BadInput;
            }
            output = ((input[0] & 0x0F) << 12) | ((input[1] & 0x3F) << 6) | (input[2] & 0x3F);
            input = input.SubSpan(3, input.GetSize() - 3).GetValue();
            return ErrorCode::Success;
        }
        if ((input[0] & 0xC0) == 0xC0)
        {
            if (input.GetSize() < 2)
            {
                return ErrorCode::BadInput;
            }
            if ((input[1] & 0x80) != 0x80)
            {
                return ErrorCode::BadInput;
            }
            output = ((input[0] & 0x1F) << 6) | (input[1] & 0x3F);
            input = input.SubSpan(2, input.GetSize() - 2).GetValue();
            return ErrorCode::Success;
        }
        return ErrorCode::BadInput;
    }

    Expected<u32, ErrorCode> EncodeOne(u32 input, Span<CodeUnitType>& output)
    {
        using ReturnType = Expected<u32, ErrorCode>;
        if (input <= 0x7F)
        {
            if (output.GetSize() < 1)
            {
                return ReturnType(ErrorCode::BadInput);
            }
            output[0] = static_cast<CodeUnitType>(input);
            return ReturnType(1);
        }
        if (input <= 0x7FF)
        {
            if (output.GetSize() < 2)
            {
                return ReturnType(ErrorCode::BadInput);
            }
            output[0] = static_cast<CodeUnitType>(0xC0 | ((input >> 6) & 0x1F));
            output[1] = static_cast<CodeUnitType>(0x80 | (input & 0x3F));
            return ReturnType(2);
        }
        if (input <= 0xFFFF)
        {
            if (output.GetSize() < 3)
            {
                return ReturnType(ErrorCode::BadInput);
            }
            output[0] = static_cast<CodeUnitType>(0xE0 | ((input >> 12) & 0x0F));
            output[1] = static_cast<CodeUnitType>(0x80 | ((input >> 6) & 0x3F));
            output[2] = static_cast<CodeUnitType>(0x80 | (input & 0x3F));
            return ReturnType(3);
        }
        if (input <= 0x10FFFF)
        {
            if (output.GetSize() < 4)
            {
                return ReturnType(ErrorCode::BadInput);
            }
            output[0] = static_cast<CodeUnitType>(0xF0 | ((input >> 18) & 0x07));
            output[1] = static_cast<CodeUnitType>(0x80 | ((input >> 12) & 0x3F));
            output[2] = static_cast<CodeUnitType>(0x80 | ((input >> 6) & 0x3F));
            output[3] = static_cast<CodeUnitType>(0x80 | (input & 0x3F));
            return ReturnType(4);
        }
        return ReturnType(ErrorCode::BadInput);
    }
};

template <typename CodeUnitType, bool k_little_endian = true>
struct EncodingUtf16
{
    ErrorCode DecodeOne(const Span<CodeUnitType> input, u32& output);
    ErrorCode EncodeOne(u32 input, Span<CodeUnitType>& output);
};

template <typename CodeUnitType, bool k_little_endian = true>
struct EncodingUtf32
{
    static_assert(sizeof(CodeUnitType) == 4, "CodeUnitType must be 4 bytes wide.");
    static_assert(k_little_endian, "Only little endian is supported.");

    ErrorCode DecodeOne(Span<CodeUnitType>& input, u32& output)
    {
        if (input.GetSize() < 1)
        {
            return ErrorCode::EndOfString;
        }
        output = static_cast<u32>(input[0]);
        input = input.SubSpan(1, input.GetSize() - 1).GetValue();
        return ErrorCode::Success;
    }

    Expected<u32, ErrorCode> EncodeOne(u32 input, Span<CodeUnitType>& output)
    {
        using ReturnType = Expected<u32, ErrorCode>;
        if (output.GetSize() < 1)
        {
            return ReturnType(ErrorCode::BadInput);
        }
        output[0] = static_cast<CodeUnitType>(input);
        return ReturnType(1);
    }
};

template <typename CodeUnitT, typename AllocatorT = DefaultAllocator>
class String
{
public:
    using CodeUnitType = CodeUnitT;
    using SizeType = u64;
    using AllocatorType = AllocatorT;

    String() = default;

    template <SizeType N>
    explicit String(const CodeUnitType (&data)[N])
    {
        Append(data, N);
    }

    bool operator==(const String& other) const
    {
        if (m_size != other.m_size)
        {
            return false;
        }
        return std::memcmp(m_data, other.m_data, m_size * sizeof(CodeUnitType)) == 0;
    }

    CodeUnitType* GetData() { return m_data; }
    [[nodiscard]] const CodeUnitType* GetData() const { return m_data; }

    SizeType GetSize() const { return m_size; }

    ErrorCode Reserve(SizeType new_capacity);
    ErrorCode Resize(SizeType new_size);

    ErrorCode Append(const CodeUnitType* data, SizeType size);

private:
    CodeUnitType* Allocate(SizeType size);
    void Deallocate(CodeUnitType* data);

    AllocatorType m_allocator;
    CodeUnitType* m_data = nullptr;
    SizeType m_size = 0;
    SizeType m_capacity = 0;
};

template <typename DstEncoding, typename SrcEncoding, typename OutputString, typename InputString>
Expected<OutputString, ErrorCode> Transcode(InputString& input);

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT, typename AllocatorT>
#define CLASS_HEADER Opal::String<CodeUnitT, AllocatorT>

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Reserve(SizeType new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return ErrorCode::Success;
    }
    CodeUnitType* new_data = Allocate(new_capacity);
    if (new_data == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    if (m_data != nullptr)
    {
        std::memcpy(new_data, m_data, m_size * sizeof(CodeUnitType));
        Deallocate(m_data);
    }
    m_data = new_data;
    m_capacity = new_capacity;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(SizeType new_size)
{
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const CodeUnitType* data, SizeType size)
{
    if (data == nullptr || size == 0)
    {
        return ErrorCode::BadInput;
    }
    if (m_size + size + 1 > m_capacity)
    {
        ErrorCode error = Reserve(m_size + size + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    std::memcpy(m_data + m_size, data, size * sizeof(CodeUnitType));
    m_size += size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
CodeUnitT* CLASS_HEADER::Allocate(SizeType size)
{
    return reinterpret_cast<CodeUnitType*>(m_allocator.Allocate(size * sizeof(CodeUnitType)));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(CodeUnitType* data)
{
    m_allocator.Deallocate(data);
}

template <typename DstEncoding, typename SrcEncoding, typename OutputString, typename InputString>
Opal::Expected<OutputString, Opal::ErrorCode> Opal::Transcode(InputString& input)
{
    using ReturnType = Expected<OutputString, ErrorCode>;
    SrcEncoding src_decoder;
    DstEncoding dst_encoder;
    typename OutputString::CodeUnitType buffer[4];
    OutputString result;
    Span<typename InputString::CodeUnitType> input_span(input.GetData(), input.GetSize());
    while (true)
    {
        u32 code_point = 0;
        ErrorCode error = src_decoder.DecodeOne(input_span, code_point);
        if (error == ErrorCode::EndOfString)
        {
            break;
        }
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
        Span<typename OutputString::CodeUnitType> output_span(buffer, 4);
        Expected<u32, ErrorCode> count = dst_encoder.EncodeOne(code_point, output_span);
        if (!count.HasValue())
        {
            return ReturnType(count.GetError());
        }
        result.Append(buffer, count.GetValue());
    }
    return ReturnType(result);
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#pragma once

#include <cstring>

#include "opal/allocator.h"
#include "opal/container/span.h"
#include "opal/error-codes.h"
#include "opal/string-encoding.h"
#include "opal/types.h"

namespace Opal
{

template <typename CodeUnitT, typename EncodingT, typename AllocatorT = DefaultAllocator>
class String
{
public:
    using CodeUnitType = CodeUnitT;
    using SizeType = u64;
    using AllocatorType = AllocatorT;
    using EncodingType = EncodingT;

    static_assert(k_is_same_value<CodeUnitType, typename EncodingType::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

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

template <typename InputString, typename OutputString>
ErrorCode Transcode(InputString& input, OutputString& output);

using StringUtf8 = String<c8, EncodingUtf8<c8>>;
using StringUtf16 = String<c16, EncodingUtf16LE<c16>>;
using StringUtf32 = String<c32, EncodingUtf32LE<c32>>;
using StringLocale = String<c, EncodingLocale>;
using StringWide = String<wc, EncodingUtf16LE<wc>>;

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT, typename EncodingT, typename AllocatorT>
#define CLASS_HEADER Opal::String<CodeUnitT, EncodingT, AllocatorT>

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
    if (new_size == m_size)
    {
        return ErrorCode::Success;
    }
    if (new_size < m_size)
    {
        m_size = new_size;
        m_data[m_size] = 0;
        return ErrorCode::Success;
    }
    if (new_size > m_capacity)
    {
        ErrorCode error = Reserve(new_size);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    for (SizeType i = m_size; i < new_size; i++)
    {
        m_data[i] = CodeUnitType();
    }
    m_size = new_size;
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
    std::memcpy(m_data + m_size * sizeof(CodeUnitType), data, size * sizeof(CodeUnitType));
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

template <typename InputString, typename OutputString>
Opal::ErrorCode Opal::Transcode(InputString& input, OutputString& output)
{
    typename InputString::EncodingType src_decoder;
    typename OutputString::EncodingType dst_encoder;
    Span<typename InputString::CodeUnitType> input_span(input.GetData(), input.GetSize());
    Span<typename OutputString::CodeUnitType> output_span(output.GetData(), output.GetSize());
    while (true)
    {
        c32 code_point = 0;
        ErrorCode error = src_decoder.DecodeOne(input_span, code_point);
        if (error == ErrorCode::EndOfString)
        {
            break;
        }
        if (error != ErrorCode::Success)
        {
            return error;
        }
        error = dst_encoder.EncodeOne(code_point, output_span);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    output.Resize(output.GetSize() - output_span.GetSize());
    return ErrorCode::Success;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

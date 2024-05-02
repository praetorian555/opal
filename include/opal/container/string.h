#pragma once

#include <cstring>

#include "opal/allocator.h"
#include "opal/container/span.h"
#include "opal/error-codes.h"
#include "opal/string-encoding.h"
#include "opal/types.h"

namespace Opal
{

template <typename CodeUnitT, typename EncodingT>
class String
{
public:
    using CodeUnitType = CodeUnitT;
    using SizeType = u64;
    using EncodingType = EncodingT;

    static_assert(k_is_same_value<CodeUnitType, typename EncodingType::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

    String(Allocator& allocator);
    String(SizeType count, CodeUnitT value, Allocator& allocator);
    String(const CodeUnitT* str, SizeType count, Allocator& allocator);
    template <SizeType N>
    String(const CodeUnitT (&str)[N], Allocator& allocator);
    String(const String& other);
    String(const String& other, Allocator& allocator);
    String(const String& other, SizeType pos, Allocator& allocator);
    String(String&& other) noexcept;

    ~String();

    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;

    bool operator==(const String& other) const;

    ErrorCode Assign(SizeType count, CodeUnitT value);
    ErrorCode Assign(const String& other);
    ErrorCode Assign(const String& other, SizeType pos, SizeType count);
    ErrorCode Assign(String&& other);
    ErrorCode Assign(const CodeUnitT* str, SizeType count);
    ErrorCode Assign(const CodeUnitT* str);

    template <typename InputIt>
    ErrorCode Assign(InputIt start, InputIt end);

    Allocator& GetAllocator() const { return *m_allocator; }

    CodeUnitType* GetData() { return m_data; }
    [[nodiscard]] const CodeUnitType* GetData() const { return m_data; }

    SizeType GetSize() const { return m_size; }
    SizeType GetCapacity() const { return m_capacity; }

    bool IsEmpty() const;

    Expected<CodeUnitT&, ErrorCode> At(SizeType pos);
    Expected<const CodeUnitT&, ErrorCode> At(SizeType pos) const;

    CodeUnitT& operator[](SizeType pos);
    const CodeUnitT& operator[](SizeType pos) const;

    Expected<CodeUnitT&, ErrorCode> Front();
    Expected<const CodeUnitT&, ErrorCode> Front() const;

    Expected<CodeUnitT&, ErrorCode> Back();
    Expected<const CodeUnitT&, ErrorCode> Back() const;

    ErrorCode Reserve(SizeType new_capacity);
    ErrorCode Resize(SizeType new_size);
    ErrorCode Resize(SizeType new_size, CodeUnitT value);

    ErrorCode Append(const CodeUnitType* str);
    ErrorCode Append(const CodeUnitType* str, SizeType size);
    ErrorCode Append(SizeType count, CodeUnitT value);
    ErrorCode Append(const String& other);
    ErrorCode Append(const String& other, SizeType pos, SizeType count);

    template <typename InputIt>
    ErrorCode AppendIt(InputIt begin, InputIt end);

private:
    inline CodeUnitType* Allocate(SizeType size);
    inline void Deallocate(CodeUnitType* data);

    Allocator* m_allocator = nullptr;
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

#define TEMPLATE_HEADER template <typename CodeUnitT, typename EncodingT>
#define CLASS_HEADER Opal::String<CodeUnitT, EncodingT>

TEMPLATE_HEADER
CLASS_HEADER::String(Allocator& allocator) : m_allocator(&allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::String(SizeType count, CodeUnitT value, Allocator& allocator) : m_allocator(&allocator), m_capacity(count), m_size(count)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = value;
    }
}

TEMPLATE_HEADER CLASS_HEADER::String(const String& other, SizeType pos, Allocator& allocator)
    : m_allocator(&allocator), m_capacity(other.GetSize() - pos)
{
    m_size = m_capacity;
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = other.m_data[pos + i];
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitT* str, SizeType count, Allocator& allocator) : m_allocator(&allocator), m_capacity(count), m_size(count)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = str[i];
    }
}

TEMPLATE_HEADER
template <CLASS_HEADER::SizeType N>
CLASS_HEADER::String(const CodeUnitT (&str)[N], Allocator& allocator) : m_allocator(&allocator), m_capacity(N), m_size(N)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = str[i];
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const String& other, Allocator& allocator)
    : m_allocator(&allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = other.GetData()[i];
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const String& other) : m_allocator(other.m_allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = other.GetData()[i];
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(String&& other) noexcept : m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
CLASS_HEADER::~String()
{
    if (m_data != nullptr)
    {
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i].~CodeUnitT();
        }
        Deallocate(m_data);
        m_data = nullptr;
    }
    m_capacity = 0;
    m_size = 0;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(const String& other)
{
    if (*this == other)
    {
        return *this;
    }

    if (m_data != nullptr)
    {
        Deallocate(m_data);
    }
    m_allocator = other.m_allocator;
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    if (m_capacity > 0)
    {
        m_data = Allocate(m_size);
        memcpy(m_data, other.m_data, m_size * sizeof(CodeUnitT));
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(String&& other) noexcept
{
    if (*this == other)
    {
        return *this;
    }

    m_allocator = other.m_allocator;
    m_data = other.m_data;
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    other.m_allocator = nullptr;
    other.m_data = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;
    return *this;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const String& other) const
{
    if (m_size != other.m_size)
    {
        return false;
    }
    return std::memcmp(m_data, other.m_data, m_size * sizeof(CodeUnitType)) == 0;
}

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
    return reinterpret_cast<CodeUnitType*>(m_allocator->Alloc(size * sizeof(CodeUnitType), 8));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(CodeUnitType* data)
{
    m_allocator->Free(data);
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

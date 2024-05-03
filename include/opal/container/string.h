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
    using EncodingType = EncodingT;

    static_assert(k_is_same_value<CodeUnitType, typename EncodingType::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

    explicit String(AllocatorT& allocator);
    String(SizeType count, CodeUnitT value, AllocatorT& allocator);
    String(const CodeUnitT* str, SizeType count, AllocatorT& allocator);
    template <u64 N>
    String(const CodeUnitT (&str)[N], AllocatorT& allocator);
    String(const String& other);
    String(const String& other, AllocatorT& allocator);
    String(const String& other, SizeType pos, AllocatorT& allocator);
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

    AllocatorT& GetAllocator() const { return *m_allocator; }

    CodeUnitType* GetData() { return m_data; }
    [[nodiscard]] const CodeUnitType* GetData() const { return m_data; }

    SizeType GetSize() const { return m_size; }
    SizeType GetCapacity() const { return m_capacity; }

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

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
    inline constexpr SizeType Min(SizeType a, SizeType b) const { return a > b ? b : a; }
    inline CodeUnitType* Allocate(SizeType size);
    inline void Deallocate(CodeUnitType* data);

    AllocatorT* m_allocator = nullptr;
    CodeUnitType* m_data = nullptr;
    SizeType m_size = 0;
    SizeType m_capacity = 0;
};

template <typename InputString, typename OutputString>
ErrorCode Transcode(InputString& input, OutputString& output);

template <typename AllocatorT>
using StringUtf8 = String<c8, EncodingUtf8<c8>, AllocatorT>;
template <typename AllocatorT>
using StringUtf16 = String<c16, EncodingUtf16LE<c16>, AllocatorT>;
template <typename AllocatorT>
using StringUtf32 = String<c32, EncodingUtf32LE<c32>, AllocatorT>;
template <typename AllocatorT>
using StringLocale = String<c, EncodingLocale, AllocatorT>;
template <typename AllocatorT>
using StringWide = String<wc, EncodingUtf16LE<wc>, AllocatorT>;

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT, typename EncodingT, typename AllocatorT>
#define CLASS_HEADER Opal::String<CodeUnitT, EncodingT, AllocatorT>

TEMPLATE_HEADER
CLASS_HEADER::String(AllocatorT& allocator) : m_allocator(&allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::String(SizeType count, CodeUnitT value, AllocatorT& allocator) : m_allocator(&allocator), m_capacity(count + 1), m_size(count)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = value;
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER CLASS_HEADER::String(const String& other, SizeType pos, AllocatorT& allocator)
    : m_allocator(&allocator)
{
    m_capacity = other.m_size - pos + 1;
    m_size = m_capacity - 1;
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = other.m_data[pos + i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitT* str, SizeType count, AllocatorT& allocator)
    : m_allocator(&allocator), m_capacity(count + 1), m_size(count)
{
    m_data = Allocate(m_capacity);
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = str[i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
template <Opal::u64 N>
CLASS_HEADER::String(const CodeUnitT (&str)[N], AllocatorT& allocator) : m_allocator(&allocator), m_capacity(N), m_size(N - 1)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = str[i];
        }
        m_data[m_size] = 0;
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const String& other, AllocatorT& allocator)
    : m_allocator(&allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = other.GetData()[i];
        }
        m_data[m_size] = 0;
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
        m_data[m_size] = 0;
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
    m_data[m_size] = 0;
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
Opal::ErrorCode CLASS_HEADER::Assign(SizeType count, CodeUnitT value)
{
    if (count + 1 > m_capacity)
    {
        if (m_data != nullptr)
        {
            Deallocate(m_data);
        }
        m_data = Allocate(count + 1);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
        m_capacity = count + 1;
    }
    for (SizeType i = 0; i < count; i++)
    {
        m_data[i] = value;
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const String& other)
{
    if (other.m_size + 1 > m_capacity)
    {
        if (m_data != nullptr)
        {
            Deallocate(m_data);
        }
        m_data = Allocate(other.m_size + 1);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
        m_capacity = other.m_size + 1;
    }
    if (other.m_size > 0)
    {
        std::memcpy(m_data, other.m_data, other.m_size * sizeof(CodeUnitT));
    }
    m_size = other.m_size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const String& other, SizeType pos, SizeType count)
{
    if (pos >= other.m_size)
    {
        return ErrorCode::OutOfBounds;
    }
    count = Min(other.GetSize() - pos, count);
    if (count + 1 > m_capacity)
    {
        if (m_data != nullptr)
        {
            Deallocate(m_data);
        }
        m_data = Allocate(count + 1);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
        m_capacity = count + 1;
    }
    if (count > 0)
    {
        std::memcpy(m_data, other.m_data + pos, count * sizeof(CodeUnitT));
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(String&& other)
{
    *this = Move(other);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const CodeUnitT* str, SizeType count)
{
    if (count + 1 > m_capacity)
    {
        if (m_data != nullptr)
        {
            Deallocate(m_data);
        }
        m_data = Allocate(count + 1);
        if (m_data == nullptr)
        {
            return ErrorCode::OutOfMemory;
        }
        m_capacity = count + 1;
    }
    for (SizeType i = 0; i < count; i++)
    {
        m_data[i] = str[i];
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const CodeUnitT* str)
{
    SizeType count = 0;
    const CodeUnitT* it = str;
    while (*it != 0)
    {
        count++;
        it++;
    }
    return Assign(str, count);
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::At(SizeType pos)
{
    using ReturnType = Expected<CodeUnitT&, ErrorCode>;
    if (pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[pos]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::At(SizeType pos) const
{
    using ReturnType = Expected<const CodeUnitT&, ErrorCode>;
    if (pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[pos]);
}

TEMPLATE_HEADER
CodeUnitT& CLASS_HEADER::operator[](SizeType pos)
{
    return m_data[pos];
}

TEMPLATE_HEADER
const CodeUnitT& CLASS_HEADER::operator[](SizeType pos) const
{
    return m_data[pos];
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::Front()
{
    using ReturnType = Expected<CodeUnitT&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    using ReturnType = Expected<const CodeUnitT&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::Back()
{
    using ReturnType = Expected<CodeUnitT&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitT&, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    using ReturnType = Expected<const CodeUnitT&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
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
        std::memcpy(new_data, m_data, (m_size + 1) * sizeof(CodeUnitType));
        Deallocate(m_data);
    }
    m_data = new_data;
    m_capacity = new_capacity;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(SizeType new_size, CodeUnitT value)
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
        ErrorCode error = Reserve(new_size + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    for (SizeType i = m_size; i < new_size; i++)
    {
        m_data[i] = value;
    }
    m_data[new_size] = 0;
    m_size = new_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(SizeType new_size)
{
    return Resize(new_size, CodeUnitT());
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

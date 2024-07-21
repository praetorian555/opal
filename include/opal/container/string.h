#pragma once

#include <cstring>

#include "opal/allocator.h"
#include "opal/container/span.h"
#include "opal/container/string-encoding.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyString>
class StringIterator
{
public:
    using ValueType = typename MyString::ValueType;
    using ReferenceType = typename MyString::ReferenceType;
    using PointerType = typename MyString::PointerType;
    using DifferenceType = typename MyString::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    StringIterator() = default;
    explicit StringIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const StringIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const StringIterator& other) const;
    bool operator>=(const StringIterator& other) const;
    bool operator<(const StringIterator& other) const;
    bool operator<=(const StringIterator& other) const;

    StringIterator& operator++();
    StringIterator operator++(int);
    StringIterator& operator--();
    StringIterator operator--(int);

    StringIterator operator+(DifferenceType n) const;
    StringIterator operator-(DifferenceType n) const;
    StringIterator& operator+=(DifferenceType n);
    StringIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const StringIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyString>
StringIterator<MyString> operator+(typename StringIterator<MyString>::DifferenceType n, const StringIterator<MyString>& it);

template <typename MyString>
class StringConstIterator
{
public:
    using ValueType = typename MyString::ValueType;
    using ReferenceType = typename MyString::ConstReferenceType;
    using PointerType = typename MyString::PointerType;
    using DifferenceType = typename MyString::DifferenceType;

    // Type aliases to be compatible with std library
    using reference = ReferenceType;
    using difference_type = DifferenceType;
    using value_type = ValueType;

    StringConstIterator() = default;
    explicit StringConstIterator(PointerType ptr) : m_ptr(ptr) {}

    bool operator==(const StringConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const StringConstIterator& other) const;
    bool operator>=(const StringConstIterator& other) const;
    bool operator<(const StringConstIterator& other) const;
    bool operator<=(const StringConstIterator& other) const;

    StringConstIterator& operator++();
    StringConstIterator operator++(int);
    StringConstIterator& operator--();
    StringConstIterator operator--(int);

    StringConstIterator operator+(DifferenceType n) const;
    StringConstIterator operator-(DifferenceType n) const;
    StringConstIterator& operator+=(DifferenceType n);
    StringConstIterator& operator-=(DifferenceType n);

    DifferenceType operator-(const StringConstIterator& other) const;

    ReferenceType operator[](DifferenceType n) const;
    ReferenceType operator*() const;
    PointerType operator->() const;

private:
    PointerType m_ptr = nullptr;
};

template <typename MyString>
StringConstIterator<MyString> operator+(typename StringConstIterator<MyString>::DifferenceType n, const StringConstIterator<MyString>& it);

template <typename CodeUnitT, typename EncodingT, typename AllocatorT = AllocatorBase>
class String
{
public:
    using ValueType = CodeUnitT;
    using ReferenceType = CodeUnitT&;
    using ConstReferenceType = const CodeUnitT&;
    using PointerType = CodeUnitT*;
    using DifferenceType = i64;
    using CodeUnitType = CodeUnitT;
    using SizeType = u64;
    using EncodingType = EncodingT;
    using IteratorType = StringIterator<String<CodeUnitT, EncodingT, AllocatorT>>;
    using ConstIteratorType = StringConstIterator<String<CodeUnitT, EncodingT, AllocatorT>>;

    static_assert(k_is_same_value<CodeUnitType, typename EncodingType::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

    /**
     * @brief Value used to represent an invalid position in the string.
     */
    static constexpr SizeType k_npos = static_cast<SizeType>(-1);

    String(AllocatorT* allocator = nullptr);
    String(SizeType count, CodeUnitT value, AllocatorT* allocator = nullptr);
    String(const CodeUnitT* str, SizeType count, AllocatorT* allocator = nullptr);
    template <u64 N>
    String(const CodeUnitT (&str)[N], AllocatorT* allocator = nullptr);
    String(const String& other, AllocatorT* allocator = nullptr);
    String(const String& other, SizeType pos, AllocatorT* allocator = nullptr);
    String(String&& other) noexcept;

    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    String(InputIt start, InputIt end, AllocatorT* allocator = nullptr);

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

    [[nodiscard]] SizeType GetSize() const { return m_size; }
    [[nodiscard]] SizeType GetCapacity() const { return m_capacity; }

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

    ErrorCode Append(const CodeUnitType& ch);
    ErrorCode Append(const CodeUnitType* str);
    ErrorCode Append(const CodeUnitType* str, SizeType size);
    ErrorCode Append(SizeType count, CodeUnitT value);
    ErrorCode Append(const String& other);
    ErrorCode Append(const String& other, SizeType pos, SizeType count);

    template <typename InputIt>
    ErrorCode AppendIt(InputIt begin, InputIt end);

    /**
     * @brief Erase a range of code units from the string.
     * @param start_pos Position in the string to start erasing from.
     * @param count Number of code units to erase. If count is equal to k_npos, the entire string starting from start_pos will be erased.
     * @return Iterator pointing to the code unit after the last erased code unit in case of a success. ErrorCode::OutOfBounds if start_pos
     * is out of bounds of the string.
     */
    Expected<IteratorType, ErrorCode> Erase(SizeType start_pos = 0, SizeType count = k_npos);

    /**
     * @brief Erase a code unit at a specific position in the string.
     * @param pos Position in the string to erase the code unit from.
     * @return Reference to the current string instance in case of a success. ErrorCode::OutOfBounds if pos is out of bounds of the string,
     */
    Expected<IteratorType, ErrorCode> Erase(IteratorType pos);
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType pos);

    /**
     * @brief Erase a range of code units from the string.
     * @param first Iterator pointing to the first code unit to erase.
     * @param last Iterator pointing to the code unit after the last code unit to erase.
     * @return Iterator pointing to the code unit after the last erased code unit in case of a success. ErrorCode::OutOfBounds if first or
     * last are out of bounds of the string. ErrorCode::BadInput if first is greater than last.
     */
    Expected<IteratorType, ErrorCode> Erase(IteratorType first, IteratorType last);
    Expected<IteratorType, ErrorCode> Erase(ConstIteratorType first, ConstIteratorType last);

    String& operator+=(const String& other);
    String& operator+=(CodeUnitType ch);
    String& operator+=(const CodeUnitType* str);

    // Iterators
    IteratorType Begin() { return IteratorType(m_data); }
    IteratorType End() { return IteratorType(m_data + m_size); }
    ConstIteratorType Begin() const { return ConstIteratorType(m_data); }
    ConstIteratorType End() const { return ConstIteratorType(m_data + m_size); }
    ConstIteratorType ConstBegin() const { return ConstIteratorType(m_data); }
    ConstIteratorType ConstEnd() const { return ConstIteratorType(m_data + m_size); }

    // Compatible with std::begin and std::end
    IteratorType begin() { return IteratorType(m_data); }
    IteratorType end() { return IteratorType(m_data + m_size); }
    ConstIteratorType begin() const { return ConstIteratorType(m_data); }
    ConstIteratorType end() const { return ConstIteratorType(m_data + m_size); }
    ConstIteratorType cbegin() const { return ConstIteratorType(m_data); }
    ConstIteratorType cend() const { return ConstIteratorType(m_data + m_size); }

    [[nodiscard]] static constexpr SizeType Min(SizeType a, SizeType b) { return a > b ? b : a; }

private:
    inline CodeUnitType* Allocate(SizeType size);
    inline void Deallocate(CodeUnitType* data);

    AllocatorT* m_allocator = nullptr;
    CodeUnitType* m_data = nullptr;
    SizeType m_size = 0;
    SizeType m_capacity = 0;
};

/**
 * @brief Compare two strings lexicographically.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param second Second string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second.
 */
template <typename MyString>
i32 Compare(const MyString& first, const MyString& second);

/**
 * @brief Compare two strings lexicographically.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If resulting range is out of bounds, it will be clamped to the size of
 * the string.
 * @param second Second string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string.
 */
template <typename MyString>
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::SizeType pos1, typename MyString::SizeType count1,
                                 const MyString& second);

/**
 * @brief Compare two strings lexicographically.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If resulting range is out of bounds, it will be clamped to the size of
 * the string.
 * @param second Second string to compare.
 * @param pos2 Position in the second string to start comparing.
 * @param count2 Number of code units to compare in the second string. If resulting range is out of bounds, it will be clamped to the size
 * of the string.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string or pos2 is greater than the size of the second string.
 */
template <typename MyString>
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::SizeType pos1, typename MyString::SizeType count1,
                                 const MyString& second, typename MyString::SizeType pos2, typename MyString::SizeType count2);

/**
 * @brief Compare a string with a null-terminated string lexicographically.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @note The null-terminated string is expected to be in the same encoding as the first string.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string.
 * @param second Null-terminated string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string. Returns ErrorCode::BadInput if second is nullptr.
 */
template <typename MyString>
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::SizeType pos1, typename MyString::SizeType count1,
                                 const typename MyString::CodeUnitType* second);

/**
 * @brief Compare a string with a null-terminated string lexicographically.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @note The null-terminated string is expected to be in the same encoding as the first string.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string.
 * @param second Null-terminated string to compare.
 * @param count2 Number of code units to compare in the second string.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string. Returns ErrorCode::BadInput if second is nullptr.
 */
template <typename MyString>
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::SizeType pos1, typename MyString::SizeType count1,
                                 const typename MyString::CodeUnitType* second, typename MyString::SizeType count2);

template <typename MyString>
MyString operator+(const MyString& lhs, const MyString& rhs);

template <typename MyString>
MyString operator+(const MyString& lhs, const typename MyString::CodeUnitType* rhs);

template <typename MyString>
MyString operator+(const MyString& lhs, typename MyString::CodeUnitType ch);

template <typename MyString>
MyString operator+(const typename MyString::CodeUnitType* lhs, const MyString& rhs);

template <typename MyString>
MyString operator+(typename MyString::CodeUnitType ch, const MyString& rhs);

/**
 * @brief Find the first occurrence of a string in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @return Position of the first occurrence of the needle in the haystack. If the needle is not found, returns MyString::k_npos. If
 * start_pos is greater than the size of the haystack, returns MyString::k_npos. If needle is empty and start_pos is less than the size of
 * the haystack, returns start_pos. If needle is empty and start_pos is greater than the size of the haystack, returns MyString::k_npos.
 */
template <typename MyString>
typename MyString::SizeType Find(const MyString& haystack, const MyString& needle, typename MyString::SizeType start_pos = 0);

/**
 * @brief Find the first occurrence of a string in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @param needle_count Number of code units to search for in the needle. Includes the null-terminator characters. If needle_count is
 * equal to MyString::k_npos, the entire needle will be searched for until the first null-terminator character.
 * @return Position of the first occurrence of the needle in the haystack. If the needle is not found, returns MyString::k_npos. If
 * start_pos is greater than the size of the haystack, returns MyString::k_npos. If needle is empty and start_pos is less than the size of
 * the haystack, returns start_pos. If needle is empty and start_pos is greater than the size of the haystack, returns MyString::k_npos.
 */
template <typename MyString>
typename MyString::SizeType Find(const MyString& haystack, const typename MyString::CodeUnitType* needle,
                                 typename MyString::SizeType start_pos = 0, typename MyString::SizeType needle_count = MyString::k_npos);

/**
 * @brief Find the first occurrence of a character in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param ch Character to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @return Position of the first occurrence of the character in the haystack. If the character is not found, returns MyString::k_npos. If
 * start_pos is greater than the size of the haystack, returns MyString::k_npos. If haystack is empty, returns MyString::k_npos.
 */
template <typename MyString>
typename MyString::SizeType Find(const MyString& haystack, const typename MyString::CodeUnitType& ch,
                                 typename MyString::SizeType start_pos = 0);

/**
 * @brief Find the last occurrence of a string in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * the size of the haystack, the entire haystack will be searched.
 * @return Position of the last occurrence of the needle in the haystack. If the needle is not found, returns MyString::k_npos. If needle is
 * empty, returns start_pos or if start_pos is larger then or equal to the size of the string the size of haystack will be returned.
 */
template <typename MyString>
typename MyString::SizeType ReverseFind(const MyString& haystack, const MyString& needle,
                                        typename MyString::SizeType start_pos = MyString::k_npos);

/**
 * @brief Find the last occurrence of a string in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * or equal to the size of the haystack, the entire haystack will be searched.
 * @param needle_count Number of code units to search for in the needle. Includes the null-terminator characters. If needle_count is
 * equal to MyString::k_npos, the entire needle will be searched for until the first null-terminator character.
 * @return Position of the last occurrence of the needle in the haystack. If the needle is not found, returns MyString::k_npos. If needle is
 * empty, returns start_pos or if start_pos is larger then or equal to the size of the string the size of haystack will be returned.
 */
template <typename MyString>
typename MyString::SizeType ReverseFind(const MyString& haystack, const typename MyString::CodeUnitType* needle,
                                        typename MyString::SizeType start_pos = MyString::k_npos,
                                        typename MyString::SizeType needle_count = MyString::k_npos);

/**
 * @brief Find the last occurrence of a character in another string.
 * @tparam MyString String type to search in.
 * @param haystack String to search in.
 * @param ch Character to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * the size of the haystack, the entire haystack will be searched.
 * @return Position of the last occurrence of the character in the haystack. If the character is not found, returns MyString::k_npos. If
 * haystack is empty, returns MyString::k_npos.
 */
template <typename MyString>
typename MyString::SizeType ReverseFind(const MyString& haystack, const typename MyString::CodeUnitType& ch,
                                        typename MyString::SizeType start_pos = MyString::k_npos);

/**
 * Transcode a string from one encoding to another.
 * @tparam InputString Type of the input string. Defines code unit type, encoding and allocator.
 * @tparam OutputString Type of the output string. Defines code unit type, encoding and allocator.
 * @param input Input string to transcode.
 * @param output Output string to store the transcoded result.
 * @return ErrorCode::Success if transcoding was successful, other error codes depend on the encoding implementation.
 */
template <typename InputString, typename OutputString>
ErrorCode Transcode(InputString& input, OutputString& output);

template <typename MyString, typename Allocator = AllocatorBase>
Expected<MyString, ErrorCode> GetSubString(const MyString& str, typename MyString::SizeType start_pos = 0,
                                           typename MyString::SizeType count = MyString::k_npos, Allocator* allocator = nullptr);

/*************************************************************************************************/
/** Most common String specializations. **********************************************************/
/*************************************************************************************************/

using StringUtf8 = String<c8, EncodingUtf8<c8>>;
using StringUtf16 = String<c16, EncodingUtf16LE<c16>>;
using StringUtf32 = String<c32, EncodingUtf32LE<c32>>;
using StringLocale = String<c, EncodingLocale>;
using StringWide = String<wc, EncodingUtf16LE<wc>>;

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitT, typename EncodingT, typename AllocatorT>
#define CLASS_HEADER Opal::String<CodeUnitT, EncodingT, AllocatorT>

TEMPLATE_HEADER
CLASS_HEADER::String(AllocatorT* allocator) : m_allocator(allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::String(SizeType count, CodeUnitT value, AllocatorT* allocator) : m_allocator(allocator), m_capacity(count + 1), m_size(count)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = value;
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER CLASS_HEADER::String(const String& other, SizeType pos, AllocatorT* allocator) : m_allocator(allocator)
{
    m_capacity = other.m_size - pos + 1;
    m_size = m_capacity - 1;
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_size = 0;
        m_capacity = 0;
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = other.m_data[pos + i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitT* str, SizeType count, AllocatorT* allocator)
    : m_allocator(allocator), m_capacity(count + 1), m_size(count)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        return;
    }
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i] = str[i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
template <Opal::u64 N>
CLASS_HEADER::String(const CodeUnitT (&str)[N], AllocatorT* allocator) : m_allocator(allocator), m_capacity(N), m_size(N - 1)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_capacity = 0;
            m_size = 0;
            return;
        }
        for (SizeType i = 0; i < m_size; i++)
        {
            m_data[i] = str[i];
        }
        m_data[m_size] = 0;
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const String& other, AllocatorT* allocator)
    : m_allocator(allocator), m_capacity(other.m_capacity), m_size(other.m_size)
{
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_capacity = 0;
            m_size = 0;
            return;
        }
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
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::String(InputIt start, InputIt end, AllocatorT* allocator) : String(&(*start), end - start, allocator)
{
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
        m_data = nullptr;
    }
    m_allocator = other.m_allocator;
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        memcpy(m_data, other.m_data, m_size * sizeof(CodeUnitT));
        m_data[m_size] = 0;
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
Opal::ErrorCode CLASS_HEADER::Append(const CodeUnitType& ch)
{
    if (m_size + 2 > m_capacity)
    {
        ErrorCode error = Reserve(m_size + 2);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    m_data[m_size] = ch;
    m_size += 1;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const CodeUnitType* str)
{
    if (str == nullptr)
    {
        return ErrorCode::BadInput;
    }
    SizeType count = 0;
    const CodeUnitT* it = str;
    while (*it != 0)
    {
        count++;
        it++;
    }
    return Append(str, count);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const CodeUnitType* str, SizeType size)
{
    if (str == nullptr)
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
    if (size > 0)
    {
        std::memcpy(m_data + m_size, str, size * sizeof(CodeUnitType));
    }
    m_size += size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(SizeType count, CodeUnitT value)
{
    if (count + 1 > m_capacity)
    {
        ErrorCode error = Reserve(count + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    for (SizeType i = m_size; i < m_size + count; i++)
    {
        m_data[i] = value;
    }
    m_size += count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const String& other)
{
    if (m_size + other.m_size + 1 > m_capacity)
    {
        ErrorCode error = Reserve(m_size + other.m_size + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    if (other.m_size > 0)
    {
        std::memcpy(m_data + m_size, other.m_data, other.m_size * sizeof(CodeUnitT));
    }
    m_size += other.m_size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const String& other, SizeType pos, SizeType count)
{
    if (pos >= other.m_size)
    {
        return ErrorCode::OutOfBounds;
    }
    count = Min(other.GetSize() - pos, count);
    if (m_size + count + 1 > m_capacity)
    {
        ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    if (count > 0)
    {
        std::memcpy(m_data + m_size, other.m_data + pos, count * sizeof(CodeUnitT));
    }
    m_size += count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(SizeType start_pos, SizeType count)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (start_pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = Min(m_size - start_pos, count);
    for (SizeType i = start_pos; i < m_size - count; i++)
    {
        m_data[i] = m_data[i + count];
    }
    m_size -= count;
    m_data[m_size] = 0;
    return ReturnType(IteratorType(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType pos)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (pos < Begin() || pos >= End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    const SizeType start_index = pos - Begin();
    for (SizeType i = start_index; i < m_size - 1; ++i)
    {
        m_data[i] = m_data[i + 1];
    }
    m_size -= 1;
    m_data[m_size] = 0;
    return ReturnType(pos);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(ConstIteratorType pos)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (pos < ConstBegin() || pos >= ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    const SizeType start_index = pos - ConstBegin();
    for (SizeType i = start_index; i < m_size - 1; ++i)
    {
        m_data[i] = m_data[i + 1];
    }
    m_size -= 1;
    m_data[m_size] = 0;
    return ReturnType(IteratorType(m_data + start_index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(IteratorType first, IteratorType last)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (first < Begin() || first >= End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (last < Begin() || last > End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return ReturnType(ErrorCode::BadInput);
    }
    if (first == last)
    {
        return ReturnType(first);
    }

    const SizeType count_to_erase = last - first;
    const SizeType start_index = first - Begin();
    for (SizeType i = start_index; i < m_size - count_to_erase; ++i)
    {
        m_data[i] = m_data[i + count_to_erase];
    }
    m_size -= count_to_erase;
    m_data[m_size] = 0;
    return ReturnType(first);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::IteratorType, Opal::ErrorCode> CLASS_HEADER::Erase(ConstIteratorType first, ConstIteratorType last)
{
    using ReturnType = Expected<IteratorType, ErrorCode>;
    if (first < ConstBegin() || first >= ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (last < ConstBegin() || last > ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return ReturnType(ErrorCode::BadInput);
    }
    if (first == last)
    {
        return ReturnType(IteratorType(m_data + (first - ConstBegin())));
    }

    const SizeType count_to_erase = last - first;
    const SizeType start_index = first - ConstBegin();
    for (SizeType i = start_index; i < m_size - count_to_erase; ++i)
    {
        m_data[i] = m_data[i + count_to_erase];
    }
    m_size -= count_to_erase;
    m_data[m_size] = 0;
    return ReturnType(IteratorType(m_data + start_index));
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const String& other)
{
    Append(other);
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(CodeUnitType ch)
{
    Append(ch);
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const CodeUnitType* str)
{
    Append(str);
    return *this;
}

TEMPLATE_HEADER
CodeUnitT* CLASS_HEADER::Allocate(SizeType size)
{
    constexpr u64 k_alignment = 8;
    const u64 size_bytes = size * sizeof(CodeUnitType);
    if (m_allocator == nullptr)
    {
        MallocAllocator allocator;
        return reinterpret_cast<CodeUnitType*>(allocator.Alloc(size_bytes, k_alignment));
    }
    return reinterpret_cast<CodeUnitType*>(m_allocator->Alloc(size_bytes, k_alignment));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(CodeUnitType* data)
{
    if (m_allocator == nullptr)
    {
        MallocAllocator allocator;
        allocator.Free(data);
    }
    else
    {
        m_allocator->Free(data);
    }
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

#define TEMPLATE_HEADER template <typename MyString>
#define CLASS_HEADER Opal::StringIterator<MyString>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const StringIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const StringIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const StringIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const StringIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    StringIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    StringIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return StringIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return StringIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const StringIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename StringIterator<MyString>::DifferenceType n, const StringIterator<MyString>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyString>
#define CLASS_HEADER Opal::StringConstIterator<MyString>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const StringConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const StringConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const StringConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const StringConstIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    StringConstIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    StringConstIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(DifferenceType n) const
{
    return StringConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(DifferenceType n) const
{
    return StringConstIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(DifferenceType n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(DifferenceType n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::DifferenceType CLASS_HEADER::operator-(const StringConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator[](DifferenceType n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::ReferenceType CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::PointerType CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename StringConstIterator<MyString>::DifferenceType n, const StringConstIterator<MyString>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

template <typename MyString>
Opal::i32 Opal::Compare(const MyString& first, const MyString& second)
{
    return Compare(first, 0, first.GetSize(), second, 0, second.GetSize()).GetValue();
}

template <typename MyString>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::SizeType pos1,
                                                         typename MyString::SizeType count1, const MyString& second)
{
    return Compare(first, pos1, count1, second, 0, second.GetSize());
}

template <typename MyString>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::SizeType pos1,
                                                         typename MyString::SizeType count1, const MyString& second,
                                                         typename MyString::SizeType pos2, typename MyString::SizeType count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using SizeType = typename MyString::SizeType;

    if (count1 != 0 && pos1 >= first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count2 != 0 && pos2 >= second.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    count1 = first.GetSize() - pos1 > count1 ? count1 : first.GetSize() - pos1;
    count2 = second.GetSize() - pos2 > count2 ? count2 : second.GetSize() - pos2;
    const SizeType count = count1 > count2 ? count2 : count1;
    for (SizeType i = 0; i < count; i++)
    {
        if (first[pos1 + i] < second[pos2 + i])
        {
            return ReturnType(-1);
        }
        if (first[pos1 + i] > second[pos2 + i])
        {
            return ReturnType(1);
        }
    }
    if (count1 < count2)
    {
        return ReturnType(-1);
    }
    if (count1 > count2)
    {
        return ReturnType(1);
    }
    return ReturnType(0);
}

template <typename MyString>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::SizeType pos1,
                                                         typename MyString::SizeType count1, const typename MyString::CodeUnitType* second)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using SizeType = typename MyString::SizeType;

    if (count1 != 0 && pos1 >= first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::BadInput);
    }

    count1 = first.GetSize() - pos1 > count1 ? count1 : first.GetSize() - pos1;
    SizeType size2 = 0;
    while (*second != 0)
    {
        if (first[pos1 + size2] < *second)
        {
            return ReturnType(-1);
        }
        if (first[pos1 + size2] > *second)
        {
            return ReturnType(1);
        }
        size2++;
        second++;
    }
    if (count1 < size2)
    {
        return ReturnType(-1);
    }
    if (count1 > size2)
    {
        return ReturnType(1);
    }
    return ReturnType(0);
}

template <typename MyString>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::SizeType pos1,
                                                         typename MyString::SizeType count1, const typename MyString::CodeUnitType* second,
                                                         typename MyString::SizeType count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using SizeType = typename MyString::SizeType;

    if (count1 != 0 && pos1 >= first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::BadInput);
    }

    count1 = first.GetSize() - pos1 > count1 ? count1 : first.GetSize() - pos1;
    SizeType size2 = 0;
    while (*second != 0 && size2 < count2)
    {
        if (first[pos1 + size2] < *second)
        {
            return ReturnType(-1);
        }
        if (first[pos1 + size2] > *second)
        {
            return ReturnType(1);
        }
        size2++;
        second++;
    }
    if (count1 < size2)
    {
        return ReturnType(-1);
    }
    if (count1 > size2)
    {
        return ReturnType(1);
    }
    return ReturnType(0);
}

template <typename MyString>
MyString Opal::operator+(const MyString& lhs, const MyString& rhs)
{
    MyString result = lhs;
    result += rhs;
    return result;
}

template <typename MyString>
MyString Opal::operator+(const MyString& lhs, const typename MyString::CodeUnitType* rhs)
{
    MyString result = lhs;
    result += rhs;
    return result;
}

template <typename MyString>
MyString Opal::operator+(const MyString& lhs, typename MyString::CodeUnitType ch)
{
    MyString result = lhs;
    result += ch;
    return result;
}

template <typename MyString>
MyString Opal::operator+(const typename MyString::CodeUnitType* lhs, const MyString& rhs)
{
    MyString result;
    result += lhs;
    result += rhs;
    return result;
}

template <typename MyString>
MyString Opal::operator+(typename MyString::CodeUnitType ch, const MyString& rhs)
{
    MyString result;
    result += ch;
    result += rhs;
    return result;
}

template <typename MyString>
typename MyString::SizeType Opal::Find(const MyString& haystack, const MyString& needle, typename MyString::SizeType start_pos)
{
    if (needle.IsEmpty())
    {
        if (start_pos >= haystack.GetSize())
        {
            return MyString::k_npos;
        }
        return start_pos;
    }
    if (start_pos >= haystack.GetSize() || needle.GetSize() > haystack.GetSize() - start_pos)
    {
        return MyString::k_npos;
    }
    for (typename MyString::SizeType haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::SizeType needle_pos = 0; needle_pos < needle.GetSize(); ++needle_pos)
        {
            if (needle[needle_pos] != haystack[haystack_pos + needle_pos])
            {
                is_found = false;
                break;
            }
        }
        if (is_found)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::SizeType Opal::Find(const MyString& haystack, const typename MyString::CodeUnitType* needle,
                                       typename MyString::SizeType start_pos, typename MyString::SizeType needle_count)
{
    if (needle == nullptr)
    {
        return MyString::k_npos;
    }
    if (needle_count == MyString::k_npos)
    {
        needle_count = 0;
        while (needle[needle_count] != 0)
        {
            ++needle_count;
        }
    }
    if (needle_count == 0)
    {
        if (start_pos >= haystack.GetSize())
        {
            return MyString::k_npos;
        }
        return start_pos;
    }
    if (start_pos >= haystack.GetSize() || needle_count > haystack.GetSize() - start_pos)
    {
        return MyString::k_npos;
    }
    for (typename MyString::SizeType haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::SizeType needle_pos = 0; needle_pos < needle_count; ++needle_pos)
        {
            if (needle[needle_pos] != haystack[haystack_pos + needle_pos])
            {
                is_found = false;
                break;
            }
        }
        if (is_found)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::SizeType Opal::Find(const MyString& haystack, const typename MyString::CodeUnitType& ch,
                                       typename MyString::SizeType start_pos)
{
    if (haystack.IsEmpty())
    {
        return MyString::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        return MyString::k_npos;
    }
    for (typename MyString::SizeType haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        if (haystack[haystack_pos] == ch)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::SizeType Opal::ReverseFind(const MyString& haystack, const MyString& needle, typename MyString::SizeType start_pos)
{
    if (needle.IsEmpty())
    {
        return start_pos >= haystack.GetSize() ? haystack.GetSize() : start_pos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize() - 1;
    }
    if (needle.GetSize() > start_pos + 1)
    {
        return MyString::k_npos;
    }
    for (typename MyString::SizeType haystack_pos = start_pos - needle.GetSize(); haystack_pos != MyString::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::SizeType needle_pos = 0; needle_pos < needle.GetSize(); ++needle_pos)
        {
            if (needle[needle_pos] != haystack[haystack_pos + needle_pos])
            {
                is_found = false;
                break;
            }
        }
        if (is_found)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::SizeType Opal::ReverseFind(const MyString& haystack, const typename MyString::CodeUnitType* needle,
                                              typename MyString::SizeType start_pos, typename MyString::SizeType needle_count)
{
    if (needle == nullptr)
    {
        return MyString::k_npos;
    }
    if (needle_count == MyString::k_npos)
    {
        needle_count = 0;
        while (needle[needle_count] != 0)
        {
            ++needle_count;
        }
    }
    if (needle_count == 0)
    {
        return start_pos >= haystack.GetSize() ? haystack.GetSize() : start_pos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize() - 1;
    }
    if (needle_count > start_pos + 1)
    {
        return MyString::k_npos;
    }
    for (typename MyString::SizeType haystack_pos = start_pos - needle_count; haystack_pos != MyString::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::SizeType needle_pos = 0; needle_pos < needle_count; ++needle_pos)
        {
            if (needle[needle_pos] != haystack[haystack_pos + needle_pos])
            {
                is_found = false;
                break;
            }
        }
        if (is_found)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::SizeType Opal::ReverseFind(const MyString& haystack, const typename MyString::CodeUnitType& ch,
                                              typename MyString::SizeType start_pos)
{
    if (haystack.IsEmpty())
    {
        return MyString::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize() - 1;
    }
    for (typename MyString::SizeType haystack_pos = start_pos; haystack_pos != MyString::k_npos; --haystack_pos)
    {
        if (haystack[haystack_pos] == ch)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString, typename Allocator>
Opal::Expected<MyString, Opal::ErrorCode> Opal::GetSubString(const MyString& str, typename MyString::SizeType start_pos,
                                                             typename MyString::SizeType count, Allocator* allocator)
{
    using ReturnType = Expected<MyString, ErrorCode>;
    if (start_pos >= str.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = MyString::Min(count, str.GetSize() - start_pos);
    return ReturnType(MyString(str.GetData() + start_pos, count, allocator));
}

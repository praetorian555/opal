#pragma once

#include <cstring>

#include "opal/allocator.h"
#include "opal/container/array-view.h"
#include "opal/container/string-encoding.h"
#include "opal/error-codes.h"
#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyString>
class StringIterator
{
public:
    using value_type = typename MyString::value_type;
    using difference_type = typename MyString::difference_type;
    using reference = typename MyString::reference;
    using pointer = typename MyString::pointer;

    StringIterator() = default;
    explicit StringIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const StringIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const StringIterator& other) const;
    bool operator>=(const StringIterator& other) const;
    bool operator<(const StringIterator& other) const;
    bool operator<=(const StringIterator& other) const;

    StringIterator& operator++();
    StringIterator operator++(int);
    StringIterator& operator--();
    StringIterator operator--(int);

    StringIterator operator+(difference_type n) const;
    StringIterator operator-(difference_type n) const;
    StringIterator& operator+=(difference_type n);
    StringIterator& operator-=(difference_type n);

    difference_type operator-(const StringIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MyString>
StringIterator<MyString> operator+(typename StringIterator<MyString>::difference_type n, const StringIterator<MyString>& it);

template <typename MyString>
class StringConstIterator
{
public:
    using value_type = typename MyString::value_type;
    using difference_type = typename MyString::difference_type;
    using const_reference = typename MyString::const_reference;
    using pointer = typename MyString::pointer;

    StringConstIterator() = default;
    explicit StringConstIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const StringConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const StringConstIterator& other) const;
    bool operator>=(const StringConstIterator& other) const;
    bool operator<(const StringConstIterator& other) const;
    bool operator<=(const StringConstIterator& other) const;

    StringConstIterator& operator++();
    StringConstIterator operator++(int);
    StringConstIterator& operator--();
    StringConstIterator operator--(int);

    StringConstIterator operator+(difference_type n) const;
    StringConstIterator operator-(difference_type n) const;
    StringConstIterator& operator+=(difference_type n);
    StringConstIterator& operator-=(difference_type n);

    difference_type operator-(const StringConstIterator& other) const;

    const_reference operator[](difference_type n) const;
    const_reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MyString>
StringConstIterator<MyString> operator+(typename StringConstIterator<MyString>::difference_type n, const StringConstIterator<MyString>& it);

/**
 * @brief String class that stores a sequence of code units.
 * @tparam CodeUnitType Type of the code unit used in the string.
 * @tparam EncodingType Type of the encoding used in the string.
 * @tparam AllocatorType Type of the allocator used for memory management. Default is AllocatorBase so that any allocator can be used.
 */
template <typename CodeUnitType, typename EncodingType, typename AllocatorType = AllocatorBase>
class String
{
public:
    using value_type = CodeUnitType;
    using allocator_type = AllocatorType;
    using size_type = u64;
    using difference_type = i64;
    using reference = CodeUnitType&;
    using const_reference = const CodeUnitType&;
    using pointer = CodeUnitType*;
    using const_pointer = CodeUnitType*;
    using encoding_type = EncodingType;
    using iterator = StringIterator<String<CodeUnitType, EncodingType, AllocatorType>>;
    using const_iterator = StringConstIterator<String<CodeUnitType, EncodingType, AllocatorType>>;

    static_assert(k_is_same_value<value_type, typename encoding_type::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

    /**
     * @brief Value used to represent an invalid position in the string.
     */
    static constexpr size_type k_npos = static_cast<size_type>(-1);

    /**
     * @brief Default constructor.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(AllocatorType* allocator = nullptr);

    /**
     * @brief Construct a string with a specific size and value.
     * @param count Number of code units to initialize the string with.
     * @param value Value of the code unit to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(size_type count, CodeUnitType value, AllocatorType* allocator = nullptr);

    /**
     * @brief Construct a string using 'count' code units from a null-terminated string.
     * @param count Number of code units to initialize the string with.
     * @param str Pointer to the null-terminated string to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(const CodeUnitType* str, size_type count, AllocatorType* allocator = nullptr);

    /**
     * @brief Construct a string using a null-terminated string.
     * @param str Pointer to the null-terminated string to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(const CodeUnitType* str, AllocatorType* allocator = nullptr);

    /**
     * Copy constructor.
     * @param other String to copy.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(const String& other, AllocatorType* allocator = nullptr);

    /**
     * @brief Construct a string using a substring of another string.
     * @param other String to copy from.
     * @param pos Position in the other string to start copying from.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(const String& other, size_type pos, AllocatorType* allocator = nullptr);
    String(String&& other) noexcept;

    /**
     * Construct a string using a range specified using random access iterators.
     * @tparam InputIt Type of the input iterator.
     * @param start Start of the range.
     * @param end End of the range.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    String(InputIt start, InputIt end, AllocatorType* allocator = nullptr);

    ~String();

    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;

    bool operator==(const String& other) const;

    /**
     * @brief Override a string with a specific number of code units.
     * @param count Number of code units to assign.
     * @param value Value of the code unit to assign.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code
     * units.
     */
    ErrorCode Assign(size_type count, CodeUnitType value);

    /**
     * Override a string with another string.
     * @param other String to assign.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code
     * units.
     */
    ErrorCode Assign(const String& other);

    /**
     * @brief Override a string with a substring of another string.
     * @param other String to assign.
     * @param pos Position in the other string to start assigning from.
     * @param count Number of code units to assign. If count is equal to k_npos, the entire string starting from pos will be assigned.
     * Default is k_npos.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfBounds if pos is out of bounds of the other string or count is
     * larger than the amount of code units in the other string starting from pos. ErrorCode::OutOfMemory if the string cannot be resized to
     * accommodate the new code units. ErrorCode::SelfNotAllowed if the other is the same as this.
     */
    ErrorCode Assign(const String& other, size_type pos, size_type count = k_npos);

    /**
     * Override a string by moving other string into it.
     * @param other String to move.
     */
    void Assign(String&& other);

    /**
     * @brief Override a string with a specific number of code units from a null-terminated string.
     * @param str Pointer to the null-terminated string to assign.
     * @param count Number of code units to assign. If count is equal to k_npos, the entire string starting from str will be assigned.
     * Default is k_npos.
     * @return ErrorCode::Success in case of a success. ErrorCode::BadInput if str is nullptr. ErrorCode::OutOfBounds if count is larger
     * then the size of the null-terminated string. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code
     * units.
     */
    ErrorCode Assign(const CodeUnitType* str, size_type count = k_npos);

    /**
     * @brief Override a string with a range specified using random access iterators.
     * @tparam InputIt Type of the input iterator. Must be a random access iterator.
     * @param start_it Start of the range.
     * @param end_it End of the range.
     * @return ErrorCode::Success in case of a success. ErrorCode::InvalidArgument if start_it is greater than end_it. ErrorCode::OutOfMemory
     * if the string cannot be resized to accommodate the new code units. ErrorCode::SelfNotAllowed if the range is the same as the current
     * string.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ErrorCode Assign(InputIt start_it, InputIt end_it);

    AllocatorType& GetAllocator() const { return *m_allocator; }

    value_type* GetData() { return m_data; }
    [[nodiscard]] const value_type* GetData() const { return m_data; }

    template <typename NewCodeUnitType>
    const NewCodeUnitType* GetDataAs() const
    {
        return reinterpret_cast<const NewCodeUnitType*>(m_data);
    }

    [[nodiscard]] size_type GetSize() const { return m_size; }
    [[nodiscard]] size_type GetCapacity() const { return m_capacity; }

    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }

    Expected<CodeUnitType&, ErrorCode> At(size_type pos);
    Expected<const CodeUnitType&, ErrorCode> At(size_type pos) const;

    CodeUnitType& operator[](size_type pos);
    const CodeUnitType& operator[](size_type pos) const;

    Expected<CodeUnitType&, ErrorCode> Front();
    Expected<const CodeUnitType&, ErrorCode> Front() const;

    Expected<CodeUnitType&, ErrorCode> Back();
    Expected<const CodeUnitType&, ErrorCode> Back() const;

    ErrorCode Reserve(size_type new_capacity);
    ErrorCode Resize(size_type new_size);
    ErrorCode Resize(size_type new_size, CodeUnitType value);

    /**
     * Shrink the size of the string to fit the data. This will only reduce the size of the string so that there is only one null-terminator
     * character at the end of the string and the size of the string will not count it.
     */
    void Trim();

    ErrorCode Append(const value_type& ch);
    ErrorCode Append(const value_type* str);
    ErrorCode Append(const value_type* str, size_type size);
    ErrorCode Append(size_type count, CodeUnitType value);
    ErrorCode Append(const String& other);
    ErrorCode Append(const String& other, size_type pos, size_type count);

    template <typename InputIt>
    ErrorCode AppendIt(InputIt begin, InputIt end);

    /**
     * @brief Insert a code unit at a specific position in the string.
     * @param start_pos Position in the string to insert the code unit at.
     * @param count Number of code units to insert.
     * @param value Value of the code unit to insert.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start_pos is out of bounds
     * of the string. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code units.
     */
    Expected<iterator, ErrorCode> Insert(size_type start_pos, size_type count, CodeUnitType value);

    /**
     * @brief Insert a string at a specific position in the string.
     * @param start_pos Position in the string to insert the string at.
     * @param str Pointer to the string to insert.
     * @param count Number of code units to insert. If count is equal to k_npos, the entire string starting from str will be inserted.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start_pos is out of bounds
     * of the string. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code units. ErrorCode::BadInput if str
     * is nullptr.
     */
    Expected<iterator, ErrorCode> Insert(size_type start_pos, const CodeUnitType* str, size_type count = k_npos);

    /**
     * @brief Insert a string at a specific position in the string.
     * @param start_pos Position in the string to insert the string at.
     * @param other String to insert.
     * @param other_start_pos Position in the other string to start inserting from.
     * @param count Number of code units to insert. If count is equal to k_npos, the entire string starting from other_start_pos will be
     * inserted.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start_pos is out of bounds
     * of the string. ErrorCode::OutOfBounds if other_start_pos is out of bounds of the other string. ErrorCode::OutOfMemory if the string
     * cannot be resized to accommodate the new code units.
     */
    Expected<iterator, ErrorCode> Insert(size_type start_pos, const String& other, size_type other_start_pos = 0, size_type count = k_npos);

    /**
     * @brief Insert a code unit at a specific position in the string.
     * @param start Iterator pointing to the position in the string to insert the code unit at.
     * @param value Value of the code unit to insert.
     * @param count Number of code units to insert. Default is 1.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start is out of bounds of
     * the string. ErrorCode::OutOfMemory if the string cannot be resized to accommodate the new code units.
     */
    Expected<iterator, ErrorCode> Insert(iterator start, CodeUnitType value, size_type count = 1);
    Expected<iterator, ErrorCode> Insert(const_iterator start, CodeUnitType value, size_type count = 1);

    /**
     * @brief Insert a string at a specific position in the string.
     * @tparam InputIt Type of the input iterator.
     * @param start Iterator pointing to the position in the string to insert the string at.
     * @param begin Iterator pointing to the first code unit in the string to insert.
     * @param end Iterator pointing to the code unit after the last code unit in the string to insert.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start is out of bounds of
     * the string. ErrorCode::BadInput if begin is greater than end. ErrorCode::OutOfMemory if the string cannot be resized to accommodate
     * the new code units.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    Expected<iterator, ErrorCode> Insert(iterator start, InputIt begin, InputIt end);
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    Expected<iterator, ErrorCode> Insert(const_iterator start, InputIt begin, InputIt end);

    /**
     * @brief Erase a range of code units from the string.
     * @param start_pos Position in the string to start erasing from.
     * @param count Number of code units to erase. If count is equal to k_npos, the entire string starting from start_pos will be erased.
     * @return Iterator pointing to the code unit after the last erased code unit in case of a success. ErrorCode::OutOfBounds if start_pos
     * is out of bounds of the string.
     */
    Expected<iterator, ErrorCode> Erase(size_type start_pos = 0, size_type count = k_npos);

    /**
     * @brief Erase a code unit at a specific position in the string.
     * @param pos Position in the string to erase the code unit from.
     * @return Reference to the current string instance in case of a success. ErrorCode::OutOfBounds if pos is out of bounds of the string,
     */
    Expected<iterator, ErrorCode> Erase(iterator pos);
    Expected<iterator, ErrorCode> Erase(const_iterator pos);

    /**
     * @brief Erase a range of code units from the string.
     * @param first Iterator pointing to the first code unit to erase.
     * @param last Iterator pointing to the code unit after the last code unit to erase.
     * @return Iterator pointing to the code unit after the last erased code unit in case of a success. ErrorCode::OutOfBounds if first or
     * last are out of bounds of the string. ErrorCode::BadInput if first is greater than last.
     */
    Expected<iterator, ErrorCode> Erase(iterator first, iterator last);
    Expected<iterator, ErrorCode> Erase(const_iterator first, const_iterator last);

    String& operator+=(const String& other);
    String& operator+=(value_type ch);
    String& operator+=(const value_type* str);

    // Iterators
    iterator Begin() { return iterator(m_data); }
    iterator End() { return iterator(m_data + m_size); }
    const_iterator Begin() const { return const_iterator(m_data); }
    const_iterator End() const { return const_iterator(m_data + m_size); }
    const_iterator ConstBegin() const { return const_iterator(m_data); }
    const_iterator ConstEnd() const { return const_iterator(m_data + m_size); }

    // Compatible with std::begin and std::end
    iterator begin() { return iterator(m_data); }
    iterator end() { return iterator(m_data + m_size); }
    const_iterator begin() const { return const_iterator(m_data); }
    const_iterator end() const { return const_iterator(m_data + m_size); }
    const_iterator cbegin() const { return const_iterator(m_data); }
    const_iterator cend() const { return const_iterator(m_data + m_size); }

    [[nodiscard]] static constexpr size_type Min(size_type a, size_type b) { return a > b ? b : a; }

private:
    inline value_type* Allocate(size_type size);
    inline void Deallocate(value_type* data);

    AllocatorType* m_allocator = nullptr;
    value_type* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
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
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::size_type pos1, typename MyString::size_type count1,
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
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::size_type pos1, typename MyString::size_type count1,
                                 const MyString& second, typename MyString::size_type pos2, typename MyString::size_type count2);

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
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::size_type pos1, typename MyString::size_type count1,
                                 const typename MyString::value_type* second);

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
Expected<i32, ErrorCode> Compare(const MyString& first, typename MyString::size_type pos1, typename MyString::size_type count1,
                                 const typename MyString::value_type* second, typename MyString::size_type count2);

template <typename MyString>
MyString operator+(const MyString& lhs, const MyString& rhs);

template <typename MyString>
MyString operator+(const MyString& lhs, const typename MyString::value_type* rhs);

template <typename MyString>
MyString operator+(const MyString& lhs, typename MyString::value_type ch);

template <typename MyString>
MyString operator+(const typename MyString::value_type* lhs, const MyString& rhs);

template <typename MyString>
MyString operator+(typename MyString::value_type ch, const MyString& rhs);

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
typename MyString::size_type Find(const MyString& haystack, const MyString& needle, typename MyString::size_type start_pos = 0);

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
typename MyString::size_type Find(const MyString& haystack, const typename MyString::value_type* needle,
                                  typename MyString::size_type start_pos = 0, typename MyString::size_type needle_count = MyString::k_npos);

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
typename MyString::size_type Find(const MyString& haystack, const typename MyString::value_type& ch,
                                  typename MyString::size_type start_pos = 0);

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
typename MyString::size_type ReverseFind(const MyString& haystack, const MyString& needle,
                                         typename MyString::size_type start_pos = MyString::k_npos);

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
typename MyString::size_type ReverseFind(const MyString& haystack, const typename MyString::value_type* needle,
                                         typename MyString::size_type start_pos = MyString::k_npos,
                                         typename MyString::size_type needle_count = MyString::k_npos);

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
typename MyString::size_type ReverseFind(const MyString& haystack, const typename MyString::value_type& ch,
                                         typename MyString::size_type start_pos = MyString::k_npos);

/**
 * Transcode a string from one encoding to another.
 * @tparam InputString Type of the input string. Defines code unit type, encoding and allocator.
 * @tparam OutputString Type of the output string. Defines code unit type, encoding and allocator.
 * @param input Input string to transcode.
 * @param output Output string to store the transcoded result.
 * @return ErrorCode::Success if transcoding was successful, other error codes depend on the encoding implementation.
 */
template <typename InputString, typename OutputString>
ErrorCode Transcode(const InputString& input, OutputString& output);

/**
 * @brief Get a substring from a string.
 * @tparam MyString Type of the string used. Defines code unit type, encoding and allocator.
 * @tparam Allocator Type of the allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param str String to get the substring from.
 * @param start_pos Position in the string to start the substring from. Default is 0.
 * @param count Number of code units to include in the substring. If count is equal to MyString::k_npos, the entire string starting from
 * start_pos will be included.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @return Substring in case of a success. ErrorCode::OutOfBounds if start_pos is greater than the size of the string.
 */
template <typename MyString, typename Allocator = AllocatorBase>
Expected<MyString, ErrorCode> GetSubString(const MyString& str, typename MyString::size_type start_pos = 0,
                                           typename MyString::size_type count = MyString::k_npos, Allocator* allocator = nullptr);

/**
 * @brief Get the length of a null-terminated string.
 * @tparam CodeUnitType Type of the code unit in the string.
 * @param str Null-terminated string to get the length of.
 * @return Length of the string. If str is nullptr, returns 0.
 */
template <typename CodeUnitType>
u64 StringLength(const CodeUnitType* str);

#define _OPAL_UTF8(text) u8##text

/**
 * @brief Macro to convert a string literal to UTF-8 encoding.
 * @param text String literal to convert.
 */
#define OPAL_UTF8(text) _OPAL_UTF8(text)

/*************************************************************************************************/
/** Most common String specializations. **********************************************************/
/*************************************************************************************************/

using StringUtf8 = String<char8, EncodingUtf8<char8>>;
using StringUtf32 = String<uchar32, EncodingUtf32LE<uchar32>>;
using StringLocale = String<char8, EncodingLocale>;
using StringWide = String<char16, EncodingUtf16LE<char16>>;

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitType, typename EncodingType, typename AllocatorType>
#define CLASS_HEADER Opal::String<CodeUnitType, EncodingType, AllocatorType>

TEMPLATE_HEADER
CLASS_HEADER::String(AllocatorType* allocator) : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::String(size_type count, CodeUnitType value, AllocatorType* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_size(count), m_capacity(count + 1)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i] = value;
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER CLASS_HEADER::String(const String& other, size_type pos, AllocatorType* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
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
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i] = other.m_data[pos + i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitType* str, size_type count, AllocatorType* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_size(count), m_capacity(count + 1)
{
    m_data = Allocate(m_capacity);
    if (m_data == nullptr)
    {
        m_capacity = 0;
        m_size = 0;
        return;
    }
    for (size_type i = 0; i < m_size; i++)
    {
        m_data[i] = str[i];
    }
    m_data[m_size] = 0;
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitType* str, AllocatorType* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    m_size = StringLength(str);
    m_capacity = m_size + 1;
    if (m_capacity > 0)
    {
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_capacity = 0;
            m_size = 0;
            return;
        }
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i] = str[i];
        }
        m_data[m_size] = 0;
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const String& other, AllocatorType* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator), m_size(other.m_size), m_capacity(other.m_capacity)
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
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i] = other.GetData()[i];
        }
        m_data[m_size] = 0;
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(String&& other) noexcept
    : m_allocator(other.m_allocator), m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
{
    other.m_allocator = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::String(InputIt start, InputIt end, AllocatorType* allocator) : String(&(*start), end - start, allocator)
{
}

TEMPLATE_HEADER
CLASS_HEADER::String::~String()
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
    // If the strings are the same, return early
    if (this == &other)
    {
        return *this;
    }

    if (m_allocator != other.m_allocator)
    {
        Deallocate(m_data);
        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
        m_allocator = other.m_allocator;
    }
    if (m_capacity < other.m_size + 1)
    {
        Deallocate(m_data);
        m_capacity = other.m_capacity;
        m_data = Allocate(m_capacity);
        if (m_data == nullptr)
        {
            m_capacity = 0;
            m_size = 0;
            return *this;
        }
    }
    m_size = other.m_size;
    memcpy(m_data, other.m_data, m_size * sizeof(CodeUnitType));
    m_data[m_size] = 0;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(String&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    Deallocate(m_data);
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
    if (m_size == 0)
    {
        return true;
    }
    OPAL_ASSERT(m_data != nullptr, "String data is null");
    OPAL_ASSERT(other.m_data != nullptr, "Other string data is null");
    return std::memcmp(m_data, other.m_data, m_size * sizeof(value_type)) == 0;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(size_type count, CodeUnitType value)
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
    for (size_type i = 0; i < count; i++)
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
    if (this == &other)
    {
        return ErrorCode::Success;
    }
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
        std::memcpy(m_data, other.m_data, other.m_size * sizeof(CodeUnitType));
    }
    m_size = other.m_size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const String& other, size_type pos, size_type count)
{
    if (this == &other)
    {
        return ErrorCode::SelfNotAllowed;
    }
    if (pos >= other.m_size)
    {
        return ErrorCode::OutOfBounds;
    }
    if (count == k_npos)
    {
        count = other.m_size - pos;
    }
    if (count > other.m_size - pos)
    {
        return ErrorCode::OutOfBounds;
    }
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
        std::memcpy(m_data, other.m_data + pos, count * sizeof(CodeUnitType));
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::Assign(String&& other)
{
    *this = Move(other);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const CodeUnitType* str, size_type count)
{
    if (str == nullptr)
    {
        return ErrorCode::InvalidArgument;
    }
    u64 str_size = StringLength(str);
    if (count == k_npos)
    {
        count = str_size;
    }
    if (count > str_size)
    {
        return ErrorCode::OutOfBounds;
    }
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
    for (size_type i = 0; i < count; i++)
    {
        m_data[i] = str[i];
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Assign(InputIt start_it, InputIt end_it)
{
    if (start_it > end_it)
    {
        return ErrorCode::InvalidArgument;
    }
    if (&(*start_it) >= m_data && &(*start_it) < m_data + m_size)
    {
        return ErrorCode::SelfNotAllowed;
    }
    u64 count = static_cast<u64>(end_it - start_it);
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
    for (size_type i = 0; i < count; i++)
    {
        m_data[i] = *start_it;
        start_it++;
    }
    m_size = count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::At(size_type pos)
{
    using ReturnType = Expected<CodeUnitType&, ErrorCode>;
    if (pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[pos]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::At(size_type pos) const
{
    using ReturnType = Expected<const CodeUnitType&, ErrorCode>;
    if (pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[pos]);
}

TEMPLATE_HEADER
CodeUnitType& CLASS_HEADER::operator[](size_type pos)
{
    return m_data[pos];
}

TEMPLATE_HEADER
const CodeUnitType& CLASS_HEADER::operator[](size_type pos) const
{
    return m_data[pos];
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Front()
{
    using ReturnType = Expected<CodeUnitType&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    using ReturnType = Expected<const CodeUnitType&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[0]);
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Back()
{
    using ReturnType = Expected<CodeUnitType&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    using ReturnType = Expected<const CodeUnitType&, ErrorCode>;
    if (m_size == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(m_data[m_size - 1]);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Reserve(size_type new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return ErrorCode::Success;
    }
    value_type* new_data = Allocate(new_capacity);
    if (new_data == nullptr)
    {
        return ErrorCode::OutOfMemory;
    }
    if (m_data != nullptr)
    {
        std::memcpy(new_data, m_data, (m_size + 1) * sizeof(value_type));
        Deallocate(m_data);
    }
    m_data = new_data;
    m_capacity = new_capacity;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(size_type new_size, CodeUnitType value)
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
    for (size_type i = m_size; i < new_size; i++)
    {
        m_data[i] = value;
    }
    m_data[new_size] = 0;
    m_size = new_size;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(size_type new_size)
{
    return Resize(new_size, CodeUnitType());
}

TEMPLATE_HEADER
void CLASS_HEADER::Trim()
{
    const size_type real_size = StringLength(m_data);
    if (m_size != 0 && m_size > real_size)
    {
        m_size = real_size;
        m_data[m_size] = 0;
    }
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const value_type& ch)
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
Opal::ErrorCode CLASS_HEADER::Append(const value_type* str)
{
    if (str == nullptr)
    {
        return ErrorCode::InvalidArgument;
    }
    size_type count = 0;
    const CodeUnitType* it = str;
    while (*it != 0)
    {
        count++;
        it++;
    }
    return Append(str, count);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const value_type* str, size_type size)
{
    if (str == nullptr)
    {
        return ErrorCode::InvalidArgument;
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
        std::memcpy(m_data + m_size, str, size * sizeof(value_type));
    }
    m_size += size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(size_type count, CodeUnitType value)
{
    if (count + 1 > m_capacity)
    {
        ErrorCode error = Reserve(count + 1);
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    for (size_type i = m_size; i < m_size + count; i++)
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
        std::memcpy(m_data + m_size, other.m_data, other.m_size * sizeof(CodeUnitType));
    }
    m_size += other.m_size;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const String& other, size_type pos, size_type count)
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
        std::memcpy(m_data + m_size, other.m_data + pos, count * sizeof(CodeUnitType));
    }
    m_size += count;
    m_data[m_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, size_type count,
                                                                                      CodeUnitType value)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start_pos > m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return ReturnType(iterator(m_data + start_pos));
    }
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = value;
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, const CodeUnitType* str,
                                                                                      size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start_pos > m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (str == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    if (count == k_npos)
    {
        count = 0;
        while (str[count] != 0)
        {
            ++count;
        }
    }
    if (count == 0)
    {
        return ReturnType(iterator(m_data + start_pos));
    }
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = str[i - start_pos];
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, const String& other,
                                                                                      size_type other_start_pos, size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start_pos > m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (other_start_pos > other.m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == k_npos)
    {
        count = other.m_size;
    }
    if (count == 0)
    {
        return ReturnType(iterator(m_data + start_pos));
    }
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = other[i - start_pos];
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(iterator start, CodeUnitType value, size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start < Begin() || start > End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return ReturnType(start);
    }
    const size_type start_pos = start - Begin();
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = value;
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(const_iterator start, CodeUnitType value,
                                                                                      size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start < ConstBegin() || start > ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return ReturnType(iterator(m_data + (start - ConstBegin())));
    }
    const size_type start_pos = start - ConstBegin();
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = value;
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(iterator start, InputIt begin, InputIt end)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start < Begin() || start > End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (begin > end)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    const size_type count = end - begin;
    if (count == 0)
    {
        return ReturnType(start);
    }
    const size_type start_pos = start - Begin();
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = *(begin + i - start_pos);
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(const_iterator start, InputIt begin, InputIt end)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start < ConstBegin() || start > ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (begin > end)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    const size_type count = end - begin;
    if (count == 0)
    {
        return ReturnType(iterator(m_data + (start - ConstBegin())));
    }
    const size_type start_pos = start - ConstBegin();
    if (m_size + count + 1 > m_capacity)
    {
        const ErrorCode error = Reserve(m_size + count + 1);
        if (error != ErrorCode::Success)
        {
            return ReturnType(error);
        }
    }
    for (size_type i = m_size - 1; i >= start_pos && i != k_npos; --i)
    {
        m_data[i + count] = m_data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        m_data[i] = *(begin + static_cast<difference_type>(i - start_pos));
    }
    m_size += count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(size_type start_pos, size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (start_pos >= m_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = Min(m_size - start_pos, count);
    for (size_type i = start_pos; i < m_size - count; i++)
    {
        m_data[i] = m_data[i + count];
    }
    m_size -= count;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator pos)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (pos < Begin() || pos >= End())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    const size_type start_index = static_cast<size_type>(pos - Begin());
    for (size_type i = start_index; i < m_size - 1; ++i)
    {
        m_data[i] = m_data[i + 1];
    }
    m_size -= 1;
    m_data[m_size] = 0;
    return ReturnType(pos);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(const_iterator pos)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (pos < ConstBegin() || pos >= ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    const size_type start_index = pos - ConstBegin();
    for (size_type i = start_index; i < m_size - 1; ++i)
    {
        m_data[i] = m_data[i + 1];
    }
    m_size -= 1;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator first, iterator last)
{
    using ReturnType = Expected<iterator, ErrorCode>;
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
        return ReturnType(ErrorCode::InvalidArgument);
    }
    if (first == last)
    {
        return ReturnType(first);
    }

    const size_type count_to_erase = last - first;
    const size_type start_index = first - Begin();
    for (size_type i = start_index; i < m_size - count_to_erase; ++i)
    {
        m_data[i] = m_data[i + count_to_erase];
    }
    m_size -= count_to_erase;
    m_data[m_size] = 0;
    return ReturnType(first);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(const_iterator first, const_iterator last)
{
    using ReturnType = Expected<iterator, ErrorCode>;
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
        return ReturnType(ErrorCode::InvalidArgument);
    }
    if (first == last)
    {
        return ReturnType(iterator(m_data + (first - ConstBegin())));
    }

    const size_type count_to_erase = last - first;
    const size_type start_index = first - ConstBegin();
    for (size_type i = start_index; i < m_size - count_to_erase; ++i)
    {
        m_data[i] = m_data[i + count_to_erase];
    }
    m_size -= count_to_erase;
    m_data[m_size] = 0;
    return ReturnType(iterator(m_data + start_index));
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const String& other)
{
    Append(other);
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(value_type ch)
{
    Append(ch);
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const value_type* str)
{
    Append(str);
    return *this;
}

TEMPLATE_HEADER
CodeUnitType* CLASS_HEADER::Allocate(size_type size)
{
    OPAL_ASSERT(m_allocator != nullptr, "Allocator should never be null!");
    constexpr u64 k_alignment = 8;
    const u64 size_bytes = size * sizeof(value_type);
    return reinterpret_cast<value_type*>(m_allocator->Alloc(size_bytes, k_alignment));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(value_type* data)
{
    OPAL_ASSERT(m_allocator != nullptr, "Allocator should never be null!");
    m_allocator->Free(data);
}

template <typename InputString, typename OutputString>
Opal::ErrorCode Opal::Transcode(const InputString& input, OutputString& output)
{
    typename InputString::encoding_type src_decoder;
    typename OutputString::encoding_type dst_encoder;
    ArrayView<const typename InputString::value_type> input_span(input.GetData(), input.GetSize());
    ArrayView<typename OutputString::value_type> output_span(output.GetData(), output.GetSize());
    while (true)
    {
        uchar32 code_point = 0;
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
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return StringIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return StringIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const StringIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename StringIterator<MyString>::difference_type n, const StringIterator<MyString>& it)
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
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return StringConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return StringConstIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const StringConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename StringConstIterator<MyString>::difference_type n, const StringConstIterator<MyString>& it)
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
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::size_type pos1,
                                                         typename MyString::size_type count1, const MyString& second)
{
    return Compare(first, pos1, count1, second, 0, second.GetSize());
}

template <typename MyString>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::size_type pos1,
                                                         typename MyString::size_type count1, const MyString& second,
                                                         typename MyString::size_type pos2, typename MyString::size_type count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename MyString::size_type;

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
    const size_type count = count1 > count2 ? count2 : count1;
    for (size_type i = 0; i < count; i++)
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
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::size_type pos1,
                                                         typename MyString::size_type count1, const typename MyString::value_type* second)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename MyString::size_type;

    if (count1 != 0 && pos1 >= first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }

    count1 = first.GetSize() - pos1 > count1 ? count1 : first.GetSize() - pos1;
    size_type size2 = 0;
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
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const MyString& first, typename MyString::size_type pos1,
                                                         typename MyString::size_type count1, const typename MyString::value_type* second,
                                                         typename MyString::size_type count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename MyString::size_type;

    if (count1 != 0 && pos1 >= first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }

    count1 = first.GetSize() - pos1 > count1 ? count1 : first.GetSize() - pos1;
    size_type size2 = 0;
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
MyString Opal::operator+(const MyString& lhs, const typename MyString::value_type* rhs)
{
    MyString result = lhs;
    result += rhs;
    return result;
}

template <typename MyString>
MyString Opal::operator+(const MyString& lhs, typename MyString::value_type ch)
{
    MyString result = lhs;
    result += ch;
    return result;
}

template <typename MyString>
MyString Opal::operator+(const typename MyString::value_type* lhs, const MyString& rhs)
{
    MyString result;
    result += lhs;
    result += rhs;
    return result;
}

template <typename MyString>
MyString Opal::operator+(typename MyString::value_type ch, const MyString& rhs)
{
    MyString result;
    result += ch;
    result += rhs;
    return result;
}

template <typename MyString>
typename MyString::size_type Opal::Find(const MyString& haystack, const MyString& needle, typename MyString::size_type start_pos)
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
    for (typename MyString::size_type haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::size_type needle_pos = 0; needle_pos < needle.GetSize(); ++needle_pos)
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
typename MyString::size_type Opal::Find(const MyString& haystack, const typename MyString::value_type* needle,
                                        typename MyString::size_type start_pos, typename MyString::size_type needle_count)
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
    for (typename MyString::size_type haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::size_type needle_pos = 0; needle_pos < needle_count; ++needle_pos)
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
typename MyString::size_type Opal::Find(const MyString& haystack, const typename MyString::value_type& ch,
                                        typename MyString::size_type start_pos)
{
    if (haystack.IsEmpty())
    {
        return MyString::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        return MyString::k_npos;
    }
    for (typename MyString::size_type haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        if (haystack[haystack_pos] == ch)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString>
typename MyString::size_type Opal::ReverseFind(const MyString& haystack, const MyString& needle, typename MyString::size_type start_pos)
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
    for (typename MyString::size_type haystack_pos = start_pos - needle.GetSize(); haystack_pos != MyString::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::size_type needle_pos = 0; needle_pos < needle.GetSize(); ++needle_pos)
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
typename MyString::size_type Opal::ReverseFind(const MyString& haystack, const typename MyString::value_type* needle,
                                               typename MyString::size_type start_pos, typename MyString::size_type needle_count)
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
    for (typename MyString::size_type haystack_pos = start_pos - needle_count; haystack_pos != MyString::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename MyString::size_type needle_pos = 0; needle_pos < needle_count; ++needle_pos)
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
typename MyString::size_type Opal::ReverseFind(const MyString& haystack, const typename MyString::value_type& ch,
                                               typename MyString::size_type start_pos)
{
    if (haystack.IsEmpty())
    {
        return MyString::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize() - 1;
    }
    for (typename MyString::size_type haystack_pos = start_pos; haystack_pos != MyString::k_npos; --haystack_pos)
    {
        if (haystack[haystack_pos] == ch)
        {
            return haystack_pos;
        }
    }
    return MyString::k_npos;
}

template <typename MyString, typename Allocator>
Opal::Expected<MyString, Opal::ErrorCode> Opal::GetSubString(const MyString& str, typename MyString::size_type start_pos,
                                                             typename MyString::size_type count, Allocator* allocator)
{
    using ReturnType = Expected<MyString, ErrorCode>;
    if (start_pos >= str.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = MyString::Min(count, str.GetSize() - start_pos);
    return ReturnType(MyString(str.GetData() + start_pos, count, allocator));
}

template <typename CodeUnitType>
Opal::u64 Opal::StringLength(const CodeUnitType* str)
{
    if (str == nullptr)
    {
        return 0;
    }
    u64 length = 0;
    while (*str != 0)
    {
        ++str;
        ++length;
    }
    return length;
}

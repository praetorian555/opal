#pragma once

#include <cinttypes>
#include <compare>
#include <initializer_list>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "opal/allocator.h"
#include "opal/container/array-view.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/string-encoding.h"
#include "opal/error-codes.h"
#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

enum class NumberSystemBase
{
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hexadecimal = 16
};

template <typename StringClass>
class StringIterator
{
public:
    using value_type = typename StringClass::value_type;
    using difference_type = typename StringClass::difference_type;
    using reference = typename StringClass::reference;
    using pointer = typename StringClass::pointer;

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

template <typename StringClass>
StringIterator<StringClass> operator+(typename StringIterator<StringClass>::difference_type n, const StringIterator<StringClass>& it);

template <typename StringClass>
class StringConstIterator
{
public:
    using value_type = typename StringClass::value_type;
    using difference_type = typename StringClass::difference_type;
    using reference = typename StringClass::const_reference;
    using const_reference = typename StringClass::const_reference;
    using pointer = typename StringClass::pointer;

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

template <typename StringClass>
StringConstIterator<StringClass> operator+(typename StringConstIterator<StringClass>::difference_type n,
                                           const StringConstIterator<StringClass>& it);

/**
 * @brief Iterator that walks a string backwards. Refers to the code unit before the forward iterator it holds.
 * @tparam StringClass Type of the string being iterated.
 * @tparam BaseIterator Forward iterator this one is built on.
 */
template <typename StringClass, typename BaseIterator>
class StringReverseIteratorBase
{
public:
    using value_type = typename BaseIterator::value_type;
    using difference_type = typename BaseIterator::difference_type;
    using reference = typename BaseIterator::reference;
    using pointer = typename BaseIterator::pointer;

    StringReverseIteratorBase() = default;
    explicit StringReverseIteratorBase(BaseIterator base) : m_base(base) {}

    /** Get the underlying forward iterator, one past the code unit this refers to. */
    BaseIterator GetBase() const { return m_base; }

    bool operator==(const StringReverseIteratorBase& other) const { return m_base == other.m_base; }
    bool operator>(const StringReverseIteratorBase& other) const;
    bool operator>=(const StringReverseIteratorBase& other) const;
    bool operator<(const StringReverseIteratorBase& other) const;
    bool operator<=(const StringReverseIteratorBase& other) const;

    StringReverseIteratorBase& operator++();
    StringReverseIteratorBase operator++(int);
    StringReverseIteratorBase& operator--();
    StringReverseIteratorBase operator--(int);

    StringReverseIteratorBase operator+(difference_type n) const;
    StringReverseIteratorBase operator-(difference_type n) const;
    StringReverseIteratorBase& operator+=(difference_type n);
    StringReverseIteratorBase& operator-=(difference_type n);

    difference_type operator-(const StringReverseIteratorBase& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    BaseIterator m_base = {};
};

template <typename StringClass, typename BaseIterator>
StringReverseIteratorBase<StringClass, BaseIterator> operator+(
    typename StringReverseIteratorBase<StringClass, BaseIterator>::difference_type n,
    const StringReverseIteratorBase<StringClass, BaseIterator>& it);

/**
 * @brief String class that stores a sequence of code units.
 * @tparam CodeUnitType Type of the code unit used in the string.
 * @tparam EncodingType Type of the encoding used in the string.
 */
template <typename CodeUnitType, typename EncodingType>
class String
{
public:
    using value_type = CodeUnitType;
    using allocator_type = AllocatorBase;
    using size_type = u64;
    using difference_type = i64;
    using reference = CodeUnitType&;
    using const_reference = const CodeUnitType&;
    using pointer = CodeUnitType*;
    using const_pointer = const CodeUnitType*;
    using encoding_type = EncodingType;
    using iterator = StringIterator<String>;
    using const_iterator = StringConstIterator<String>;
    using reverse_iterator = StringReverseIteratorBase<String, iterator>;
    using const_reverse_iterator = StringReverseIteratorBase<String, const_iterator>;

    static_assert(k_is_same_value<value_type, typename encoding_type::CodeUnitType>,
                  "Encoding code unit type needs to match string code unit type");

    /**
     * @brief Value used to represent an invalid position in the string.
     */
    static constexpr size_type k_npos = static_cast<size_type>(-1);

    /**
     * @brief Largest number of code units a string can hold, not counting the null terminator.
     */
    static constexpr size_type k_max_size = (k_npos / sizeof(CodeUnitType)) - 1;

    /**
     * @brief Default constructor.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     */
    String(allocator_type* allocator = nullptr);

    /**
     * @brief Construct a string with a specific size and value.
     * @param count Number of code units to initialize the string with.
     * @param value Value of the code unit to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw OutOfMemoryException if count is larger than k_max_size or the allocator runs out of memory.
     */
    String(size_type count, CodeUnitType value, allocator_type* allocator = nullptr);

    /**
     * @brief Construct a string using 'count' code units read from str. str does not need to be null-terminated.
     * @param count Number of code units to initialize the string with.
     * @param str Pointer to the code units to initialize the string with. May be nullptr only when count is 0.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw InvalidArgumentException if str is nullptr and count is not 0.
     * @throw OutOfMemoryException if count is larger than k_max_size or the allocator runs out of memory.
     */
    String(const CodeUnitType* str, size_type count, allocator_type* allocator = nullptr);

    /**
     * @brief Construct a string using a null-terminated string.
     * @param str Pointer to the null-terminated string to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw OutOfMemoryException if allocator runs out of memory.
     */
    String(const CodeUnitType* str, allocator_type* allocator = nullptr);

    /**
     * @brief Construct a string using a substring of another string.
     * @param other String to copy from.
     * @param pos Position in the other string to start copying from. May be equal to the size of other, which yields an empty string.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw OutOfBoundsException if pos is greater than the size of other.
     * @throw OutOfMemoryException if allocator runs out of memory.
     */
    String(const String& other, size_type pos, allocator_type* allocator = nullptr);
    String(String&& other) noexcept;

    /**
     * Construct a string using a range specified using random access iterators.
     * @tparam InputIt Type of the input iterator.
     * @param start Start of the range.
     * @param end End of the range.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw OutOfMemoryException if allocator runs out of memory.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    String(InputIt start, InputIt end, allocator_type* allocator = nullptr);

    /**
     * Construct a string from a list of code units.
     * @param init_list Code units to initialize the string with.
     * @param allocator Allocator to use for memory management. If nullptr, the default allocator will be used.
     * @throw OutOfMemoryException if allocator runs out of memory.
     */
    String(std::initializer_list<CodeUnitType> init_list, allocator_type* allocator = nullptr);

    /** The string does not copy. Use Clone to make an owning copy, or move it. */
    /**
     * Build a string the way the matching constructor does, reporting a failed allocation instead of throwing it. Use these when
     * the allocator is budgeted and running out is an outcome to branch on.
     * @return The string, or ErrorCode::OutOfMemory. The (str, count) overload reports ErrorCode::InvalidArgument for a null str
     *         with a non-zero count.
     */
    [[nodiscard]] static Expected<String, ErrorCode> Create(size_type count, CodeUnitType value, allocator_type* allocator = nullptr);
    [[nodiscard]] static Expected<String, ErrorCode> Create(const CodeUnitType* str, size_type count, allocator_type* allocator = nullptr);
    [[nodiscard]] static Expected<String, ErrorCode> Create(const CodeUnitType* str, allocator_type* allocator = nullptr);
    [[nodiscard]] static Expected<String, ErrorCode> Create(std::initializer_list<CodeUnitType> init_list,
                                                            allocator_type* allocator = nullptr);

    /**
     * Create a deep copy of this string, reporting a failed allocation instead of throwing it.
     * @param allocator Allocator to be used for the copy. If nullptr, the source string's allocator will be used.
     * @return The copy, or ErrorCode::OutOfMemory.
     */
    [[nodiscard]] Expected<String, ErrorCode> TryClone(AllocatorBase* allocator = nullptr) const;

    String(const String& other) = delete;
    String& operator=(const String& other) = delete;

    String Clone(AllocatorBase* allocator = nullptr) const;

    ~String();

    /**
     * Move assignment. Always uses allocator from the source string.
     * @param other Source string.
     * @return Reference to this string.
     */
    String& operator=(String&& other) noexcept;

    bool operator==(const String& other) const;

    /**
     * @brief Compare with a null-terminated string. A nullptr compares equal to an empty string.
     * @param other Null-terminated string to compare with.
     * @return True when both hold the same code units.
     */
    bool operator==(const CodeUnitType* other) const;

    /**
     * @brief Order two strings by comparing code units, the shorter string first when one is a prefix of the other. Code units are ordered
     * the way the code unit type orders them, which is signed for char8, matching the free Compare function.
     * @param other String to compare with.
     * @return How this string orders against other.
     */
    std::strong_ordering operator<=>(const String& other) const;

    /**
     * @brief Order against a null-terminated string. A nullptr orders as an empty string.
     * @param other Null-terminated string to compare with.
     * @return How this string orders against other.
     */
    std::strong_ordering operator<=>(const CodeUnitType* other) const;

    [[nodiscard]] allocator_type& GetAllocator() const { return *GetAllocatorPtr(); }

    value_type* GetData() { return IsSmall() ? GetSmallData() : m_storage.large.data; }
    [[nodiscard]] const value_type* GetData() const { return IsSmall() ? GetSmallData() : m_storage.large.data; }
    value_type* operator*() { return GetData(); }
    const value_type* operator*() const { return GetData(); }

    [[nodiscard]] size_type GetSize() const { return IsSmall() ? GetSmallSize() : m_storage.large.size; }

    /**
     * @brief Number of code units the string can hold, the null terminator included. A string of GetCapacity() - 1 code units is full.
     * Note that this counts differently from DynamicArray, which has no terminator to account for.
     */
    [[nodiscard]] size_type GetCapacity() const { return IsSmall() ? k_sso_capacity : m_storage.large.capacity; }

    [[nodiscard]] bool IsEmpty() const { return GetSize() == 0; }
    [[nodiscard]] bool empty() const { return GetSize() == 0; }

    /**
     * @brief Override a string with a specific number of code units.
     * @param count Number of code units to assign.
     * @param value Value of the code unit to assign.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Assign(size_type count, CodeUnitType value);

    /**
     * Override a string with another string. Assigning a string to itself is allowed and does nothing.
     * @param other String to assign.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Assign(const String& other);

    /**
     * @brief Override a string with a substring of another string.
     * @param other String to assign.
     * @param pos Position in the other string to start assigning from. May be equal to the size of the other string, which assigns an
     * empty string.
     * @param count Number of code units to assign. If count is equal to k_npos, the entire string starting from pos will be assigned.
     * Default is k_npos.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfBounds if pos is out of bounds of the other string or count is
     * larger than the amount of code units in the other string starting from pos. ErrorCode::SelfNotAllowed if the other is the same as
     * this. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Assign(const String& other, size_type pos, size_type count = k_npos);

    /**
     * Override a string by moving other string into it.
     * @param other String to move.
     */
    void Assign(String&& other);

    /**
     * Override a string with a list of code units.
     * @param init_list Code units to assign.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Assign(std::initializer_list<CodeUnitType> init_list);

    /**
     * Override a string with a list of code units. Keeps the current allocator.
     * @param init_list Code units to assign.
     * @return Reference to this string.
     * @throw OutOfMemoryException if allocator runs out of memory.
     */
    String& operator=(std::initializer_list<CodeUnitType> init_list);

    /**
     * @brief Override a string with a specific number of code units from a null-terminated string.
     * @param str Pointer to the null-terminated string to assign.
     * @param count Number of code units to assign. If count is equal to k_npos, the entire string starting from str will be assigned.
     * Default is k_npos.
     * @return ErrorCode::Success in case of a success. ErrorCode::InvalidArgument if str is nullptr. ErrorCode::OutOfBounds if count is larger
     * then the size of the null-terminated string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Assign(const CodeUnitType* str, size_type count = k_npos);

    /**
     * @brief Override a string with a range specified using random access iterators.
     * @tparam InputIt Type of the input iterator. Must be a random access iterator.
     * @param start_it Start of the range.
     * @param end_it End of the range.
     * @return ErrorCode::Success in case of a success. ErrorCode::InvalidArgument if start_it is greater than end_it.
     * ErrorCode::SelfNotAllowed if the range is the same as the current string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ErrorCode Assign(InputIt start_it, InputIt end_it);

    /**
     * @brief Get the code unit at a specific position in the string. Bounds checking only in debug mode.
     * @param pos Position in the string to get the code unit from.
     * @return Reference to the code unit.
     */
    CodeUnitType& At(size_type pos);
    [[nodiscard]] const CodeUnitType& At(size_type pos) const;

    /**
     * @brief Get the code unit at a specific position in the string. Bounds-checked.
     * @param pos Position in the string to get the code unit from.
     * @return Reference to the code unit.
     * @throw OutOfBoundsException if pos is out of bounds.
     */
    CodeUnitType& operator[](size_type pos);
    const CodeUnitType& operator[](size_type pos) const;

    /**
     * @brief Get the first code unit in the string.
     * @return Reference to the first code unit in case of a success. ErrorCode::OutOfBounds if the string is empty.
     */
    Expected<CodeUnitType&, ErrorCode> Front();
    [[nodiscard]] Expected<const CodeUnitType&, ErrorCode> Front() const;

    /**
     * @brief Get the last code unit in the string.
     * @return Reference to the last code unit in case of a success. ErrorCode::OutOfBounds if the string is empty.
     */
    Expected<CodeUnitType&, ErrorCode> Back();
    [[nodiscard]] Expected<const CodeUnitType&, ErrorCode> Back() const;

    /**
     * Reserve memory for a specific number of code units, the null terminator included. Reserve(n) therefore leaves room for n - 1 code
     * units of text, matching GetCapacity. Does nothing when the requested capacity is not larger than the current one.
     * @param new_capacity Number of code units to reserve memory for, terminator included.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown, in which case the string is
     * left as it was.
     */
    ErrorCode Reserve(size_type new_capacity);

    /**
     * Resize the string to a specific size. If new code units are added they will be initialized with the default value of the code unit.
     * @param new_size New size of the string.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Resize(size_type new_size);

    /**
     * Resize the string to a specific size. If new code units are added they will be initialized with the provided value.
     * @param new_size New size of the string.
     * @param value Value of the code unit to initialize the new code units with.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Resize(size_type new_size, CodeUnitType value);

    /**
     * Set the size of the string to 0. Does not deallocate memory.
     */
    void Clear();

    /**
     * Remove the last code unit from the string. Does nothing when the string is empty.
     */
    void PopBack();

    /**
     * Give up any capacity beyond what the current contents need. Does nothing when the string is small enough to live inline. Invalidates
     * every iterator and pointer into the string when it moves the code units.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the smaller buffer could not be taken, in which case the
     * string keeps the one it has.
     */
    ErrorCode ShrinkToFit();

    /**
     * Shrink the size of the string to fit the data. This will only reduce the size of the string so that there is only one null-terminator
     * character at the end of the string and the size of the string will not count it.
     */
    void Trim();

    /**
     * Remove leading and trailing whitespace characters from the string. Characters removed are: space, tab, newline, and carriage return.
     */
    void Strip();

    /**
     * Replace a range of the string with another string. The range may be empty, in which case this inserts, and the replacement may be
     * empty, in which case this erases.
     * @param start_pos Position where the replaced range starts. May be equal to the size of the string.
     * @param count Number of code units to replace. Clamped to what is left in the string.
     * @param other String to put in place of the range. May be this string.
     * @return Iterator pointing to the first code unit of the replacement in case of a success. ErrorCode::OutOfBounds if start_pos is
     * greater than the size of the string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    Expected<iterator, ErrorCode> Replace(size_type start_pos, size_type count, const String& other);

    /**
     * Replace a range of the string with a null-terminated string.
     * @param start_pos Position where the replaced range starts. May be equal to the size of the string.
     * @param count Number of code units to replace. Clamped to what is left in the string.
     * @param other Code units to put in place of the range. May point into this string.
     * @param other_count Number of code units to read from other. If equal to k_npos, everything up to the null terminator is used.
     * @return Iterator pointing to the first code unit of the replacement in case of a success. ErrorCode::OutOfBounds if start_pos is
     * greater than the size of the string. ErrorCode::InvalidArgument if other is nullptr. ErrorCode::OutOfMemory if the string could not be
     * grown.
     */
    Expected<iterator, ErrorCode> Replace(size_type start_pos, size_type count, const CodeUnitType* other,
                                          size_type other_count = k_npos);

    /**
     * Replace a range of the string with another string.
     * @param first Iterator pointing to the first code unit to replace.
     * @param last Iterator pointing one past the last code unit to replace.
     * @param other String to put in place of the range. May be this string.
     * @return Iterator pointing to the first code unit of the replacement in case of a success. ErrorCode::OutOfBounds if first or last are
     * out of bounds of the string. ErrorCode::InvalidArgument if first is greater than last. ErrorCode::OutOfMemory if the string could not
     * be grown.
     */
    Expected<iterator, ErrorCode> Replace(const_iterator first, const_iterator last, const String& other);

    /**
     * Replace every occurrence of a sub-string. Scanning continues after the replacement, so a replacement that contains the needle does
     * not feed back into the search.
     * @param needle Sub-string to look for. An empty needle matches nothing.
     * @param replacement String to put in place of each occurrence.
     * @return How many occurrences were replaced. ErrorCode::OutOfMemory if the string could not be grown, in which case the replacements
     * made up to that point stand.
     */
    Expected<size_type, ErrorCode> ReplaceAll(const String& needle, const String& replacement);

    /**
     * Replace the first occurrence of a sub-string.
     * @param needle Sub-string to look for. An empty needle matches nothing.
     * @param replacement String to put in place of the occurrence.
     * @return True when an occurrence was found and replaced. ErrorCode::OutOfMemory if the string could not be grown.
     */
    Expected<bool, ErrorCode> ReplaceFirst(const String& needle, const String& replacement);

    /**
     * Get a sub-string of this string.
     * @param start_pos Position to start the sub-string from. May be equal to the size of the string, which yields an empty sub-string.
     * @param count Number of code units to include. If count is equal to k_npos, or reaches past the end, the rest of the string is used.
     * @param allocator Allocator to use for the result. If nullptr, the default allocator will be used.
     * @return Sub-string in case of a success. ErrorCode::OutOfBounds if start_pos is greater than the size of the string.
     * @throw OutOfMemoryException when there is no more memory.
     */
    [[nodiscard]] Expected<String, ErrorCode> GetSubString(size_type start_pos = 0, size_type count = k_npos,
                                                           allocator_type* allocator = nullptr) const;

    /**
     * Check whether the string contains a sub-string. An empty needle is always contained.
     * @param needle Sub-string to look for.
     * @return True when the needle occurs in the string.
     */
    [[nodiscard]] bool Contains(const String& needle) const;

    /**
     * Check whether the string contains a sub-string.
     * @param needle Null-terminated sub-string to look for. A nullptr is never contained.
     * @return True when the needle occurs in the string.
     */
    [[nodiscard]] bool Contains(const CodeUnitType* needle) const;

    /**
     * Check whether the string contains a code unit.
     * @param ch Code unit to look for.
     * @return True when the code unit occurs in the string.
     */
    [[nodiscard]] bool Contains(CodeUnitType ch) const;

    /**
     * Append a code unit to the end of the string.
     * @param ch Code unit to append.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Append(const value_type& ch);

    /**
     * Append a specific number of code units to the end of the string.
     * @param str Pointer to the code units to append.
     * @param size Number of code units to append. If size is equal to k_npos, the entire string starting from str will be appended.
     * @return ErrorCode::Success in case of a success. ErrorCode::InvalidArgument if str is nullptr. ErrorCode::OutOfMemory if the string
     * could not be grown.
     */
    ErrorCode Append(const value_type* str, size_type size = k_npos);

    /**
     * Append a specific number of code units to the end of the string.
     * @param count Number of code units to append.
     * @param value Value of the code unit to append.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Append(size_type count, CodeUnitType value);

    /**
     * Append a string to the end of the string.
     * @param other String to append.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Append(const String& other);

    /**
     * Append a substring of another string to the end of the string.
     * @param other String to append.
     * @param pos Position in the other string to start appending from.
     * @param count Number of code units to append. If count is equal to k_npos, the entire string starting from pos
     * will be appended.
     * @return ErrorCode::Success in case of a success. ErrorCode::OutOfBounds if pos is out of bounds of the other
     * string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    ErrorCode Append(const String& other, size_type pos, size_type count = k_npos);

    /**
     * Append a range specified using random access iterators to the end of the string.
     * @tparam InputIt Type of the input iterator. Must be a random access iterator.
     * @param begin_it Iterator pointing to the first code unit in the string to append.
     * @param end_it Iterator pointing to the code unit after the last code unit in the string to append.
     * @return ErrorCode::Success in case of a success. ErrorCode::InvalidArgument if begin is greater than end.
     * ErrorCode::SelfNotAllowed if the range is the same as the current string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ErrorCode Append(InputIt begin_it, InputIt end_it);

    /**
     * @brief Insert a code unit at a specific position in the string.
     * @param start_pos Position in the string to insert the code unit at.
     * @param count Number of code units to insert.
     * @param value Value of the code unit to insert.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start_pos is out of bounds
     * of the string. ErrorCode::OutOfMemory if the string could not be grown.
     */
    Expected<iterator, ErrorCode> Insert(size_type start_pos, size_type count, CodeUnitType value);

    /**
     * @brief Insert a string at a specific position in the string.
     * @param start_pos Position in the string to insert the string at.
     * @param str Pointer to the string to insert.
     * @param count Number of code units to insert. If count is equal to k_npos, the entire string starting from str will be inserted.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start_pos is out of bounds
     * of the string. ErrorCode::InvalidArgument if str is nullptr. ErrorCode::OutOfMemory if the string could not be grown.
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
     * of the string, if other_start_pos is out of bounds of the other string, or if count exceeds what is left in the other string after
     * other_start_pos. Inserting a string into itself is allowed. ErrorCode::OutOfMemory if the string could not be grown.
     */
    Expected<iterator, ErrorCode> Insert(size_type start_pos, const String& other, size_type other_start_pos = 0, size_type count = k_npos);

    /**
     * @brief Insert a code unit at a specific position in the string.
     * @param start Iterator pointing to the position in the string to insert the code unit at.
     * @param value Value of the code unit to insert.
     * @param count Number of code units to insert. Default is 1.
     * @return Iterator pointing to the first inserted code unit in case of a success. ErrorCode::OutOfBounds if start is out of bounds of
     * the string. ErrorCode::OutOfMemory if the string could not be grown.
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
     * the string. ErrorCode::InvalidArgument if begin is greater than end. ErrorCode::SelfNotAllowed if begin points into this string,
     * matching Append and Assign. ErrorCode::OutOfMemory if the string could not be grown.
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
     * A start_pos equal to the size of the string erases nothing.
     * @return Iterator pointing to the code unit after the last erased code unit in case of a success. ErrorCode::OutOfBounds if start_pos
     * is greater than the size of the string.
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
     * last are out of bounds of the string. ErrorCode::InvalidArgument if first is greater than last. An empty range at the end of the
     * string erases nothing.
     */
    Expected<iterator, ErrorCode> Erase(iterator first, iterator last);
    Expected<iterator, ErrorCode> Erase(const_iterator first, const_iterator last);

    void Reverse();
    void Reverse(iterator start_it, iterator end_it);

    /**
     * @brief Append to the end of the string. An operator has nothing to return a code through, so use Append when the failure matters.
     * @throw OutOfMemoryException when there is no more memory.
     */
    String& operator+=(const String& other);
    String& operator+=(value_type ch);
    String& operator+=(const value_type* str);

    // Iterators
    iterator Begin() { return iterator(GetData()); }
    iterator End() { return iterator(GetData() + GetSize()); }
    [[nodiscard]] const_iterator Begin() const { return const_iterator(MutableData()); }
    [[nodiscard]] const_iterator End() const { return const_iterator(MutableData() + GetSize()); }
    [[nodiscard]] const_iterator ConstBegin() const { return const_iterator(MutableData()); }
    [[nodiscard]] const_iterator ConstEnd() const { return const_iterator(MutableData() + GetSize()); }

    // Compatible with std::begin and std::end
    iterator begin() { return iterator(GetData()); }
    iterator end() { return iterator(GetData() + GetSize()); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(MutableData()); }
    [[nodiscard]] const_iterator end() const { return const_iterator(MutableData() + GetSize()); }
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(MutableData()); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(MutableData() + GetSize()); }

    /**
     * @brief Get a reverse iterator pointing to the last code unit. Advancing it steps towards the front.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator rbegin() const { return const_reverse_iterator(cend()); }
    [[nodiscard]] const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    /**
     * @brief Get a reverse iterator pointing one before the first code unit.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator rend() const { return const_reverse_iterator(cbegin()); }
    [[nodiscard]] const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    [[nodiscard]] static constexpr size_type Min(size_type a, size_type b) { return a > b ? b : a; }

    /**
     * @brief Maximum number of code units (including null terminator) that fit in the SSO buffer.
     */
    static constexpr size_type k_sso_buf_size = 24;
    static constexpr size_type k_sso_capacity = k_sso_buf_size / sizeof(value_type);

private:
    static_assert(sizeof(void*) == 8, "SSO pointer tagging requires a 64-bit platform");

    static constexpr uintptr_t k_sso_flag_bit = uintptr_t(1) << 63;
    static constexpr int k_sso_size_shift = 57;
    static constexpr uintptr_t k_sso_size_mask = uintptr_t(0x3F) << k_sso_size_shift;
    static constexpr uintptr_t k_ptr_mask = (uintptr_t(1) << 57) - 1;

    [[nodiscard]] bool IsSmall() const { return (m_allocator_and_tag & k_sso_flag_bit) != 0; }

    [[nodiscard]] allocator_type* GetAllocatorPtr() const
    {
        return reinterpret_cast<allocator_type*>(m_allocator_and_tag & k_ptr_mask);
    }

    [[nodiscard]] size_type GetSmallSize() const
    {
        return static_cast<size_type>((m_allocator_and_tag & k_sso_size_mask) >> k_sso_size_shift);
    }

    void SetSmallSize(size_type new_size)
    {
        m_allocator_and_tag = (m_allocator_and_tag & k_ptr_mask)
                            | k_sso_flag_bit
                            | (static_cast<uintptr_t>(new_size) << k_sso_size_shift);
    }

    void InitSmall(allocator_type* alloc, size_type size)
    {
        m_allocator_and_tag = reinterpret_cast<uintptr_t>(alloc)
                            | k_sso_flag_bit
                            | (static_cast<uintptr_t>(size) << k_sso_size_shift);
    }

    void InitLarge(allocator_type* alloc)
    {
        m_allocator_and_tag = reinterpret_cast<uintptr_t>(alloc);
    }

    void SetSize(size_type new_size)
    {
        if (IsSmall())
            SetSmallSize(new_size);
        else
            m_storage.large.size = new_size;
    }

    value_type* GetSmallData() { return reinterpret_cast<value_type*>(m_storage.raw); }
    [[nodiscard]] const value_type* GetSmallData() const { return reinterpret_cast<const value_type*>(m_storage.raw); }

    // Returns a mutable pointer to the data, even from const context.
    // Used internally for iterator construction (StringConstIterator takes a non-const pointer).
    value_type* MutableData() const
    {
        if (IsSmall())
            return const_cast<value_type*>(reinterpret_cast<const value_type*>(m_storage.raw));
        return m_storage.large.data;
    }

    // Ensures the buffer holds at least required_capacity code units, terminator included. Capacity
    // grows by half on every reallocation so that repeated appends stay amortized constant time.
    ErrorCode Grow(size_type required_capacity)
    {
        const size_type current_capacity = GetCapacity();
        if (required_capacity <= current_capacity)
        {
            return ErrorCode::Success;
        }
        size_type new_capacity = required_capacity;
        if (current_capacity <= k_max_size / 2)
        {
            const size_type scaled = current_capacity + (current_capacity / 2) + 1;
            new_capacity = scaled > new_capacity ? scaled : new_capacity;
        }
        return Reserve(new_capacity);
    }

    // Ensures room for current_size + added code units plus a terminator.
    ErrorCode GrowForAppend(size_type current_size, size_type added)
    {
        if (added > k_max_size - current_size)
        {
            return ErrorCode::OutOfMemory;
        }
        return Grow(current_size + added + 1);
    }

    // The allocating half of each constructor, returning a code instead of throwing. The constructors throw over these, and the
    // Create factories branch on them, so the two never drift apart. Each expects an empty, freshly built string.
    ErrorCode Construct(allocator_type* alloc, size_type count, CodeUnitType value);
    ErrorCode Construct(allocator_type* alloc, const CodeUnitType* str, size_type count);
    ErrorCode Construct(allocator_type* alloc, std::initializer_list<CodeUnitType> init_list);

    // Builds the initial storage for a string of exactly count code units and writes the terminator.
    // The code units themselves are left uninitialized. Only valid on a freshly default-initialized
    // object, since it assumes m_storage is still zeroed. Leaves an empty string behind on failure.
    ErrorCode InitStorage(allocator_type* alloc, size_type count)
    {
        if (!IsValidSize(count))
        {
            InitSmall(alloc, 0);
            m_storage.raw[0] = 0;
            return ErrorCode::OutOfMemory;
        }
        if (count + 1 <= k_sso_capacity)
        {
            InitSmall(alloc, count);
            GetSmallData()[count] = 0;
            return ErrorCode::Success;
        }
        // Allocate reads the allocator back out of the tag, so it has to be stored first. Start from
        // the empty small state, which is what a failed allocation should leave behind anyway.
        InitSmall(alloc, 0);
        m_storage.raw[0] = 0;
        value_type* new_data = Allocate(count + 1);
        if (new_data == nullptr) [[unlikely]]
        {
            return ErrorCode::OutOfMemory;
        }
        InitLarge(alloc);
        m_storage.large.data = new_data;
        m_storage.large.size = count;
        m_storage.large.capacity = count + 1;
        new_data[count] = 0;
        return ErrorCode::Success;
    }

    // Makes room for new_size code units plus a terminator, discarding the current contents. The new
    // buffer is allocated before the old one is released so that a failed allocation leaves the
    // string unchanged rather than holding a dangling pointer.
    ErrorCode PrepareForOverwrite(size_type new_size)
    {
        if (!IsValidSize(new_size))
        {
            return ErrorCode::OutOfMemory;
        }
        if (new_size + 1 <= GetCapacity())
        {
            return ErrorCode::Success;
        }
        value_type* new_data = Allocate(new_size + 1);
        if (new_data == nullptr) [[unlikely]]
        {
            return ErrorCode::OutOfMemory;
        }
        if (!IsSmall() && m_storage.large.data != nullptr)
        {
            Deallocate(m_storage.large.data);
        }
        allocator_type* alloc = GetAllocatorPtr();
        InitLarge(alloc);
        m_storage.large.data = new_data;
        m_storage.large.size = 0;
        m_storage.large.capacity = new_size + 1;
        return ErrorCode::Success;
    }

    // Returns the offset of str into this string's own buffer, or k_npos when str points somewhere
    // else. Used to re-derive caller pointers that would dangle once the buffer is reallocated.
    [[nodiscard]] size_type GetInternalOffset(const value_type* str) const
    {
        const value_type* data = GetData();
        const uintptr_t address = reinterpret_cast<uintptr_t>(str);
        const uintptr_t buffer_begin = reinterpret_cast<uintptr_t>(data);
        const uintptr_t buffer_end = buffer_begin + GetCapacity() * sizeof(value_type);
        if (address < buffer_begin || address >= buffer_end)
        {
            return k_npos;
        }
        return static_cast<size_type>(str - data);
    }

    // False for sizes that would overflow the byte count or the null-terminator slot.
    static bool IsValidSize(size_type new_size) { return new_size <= k_max_size; }

    // Returns nullptr when the request cannot be served, which is an ordinary result rather than an error.
    inline value_type* Allocate(size_type size);
    inline void Deallocate(value_type* data);

    uintptr_t m_allocator_and_tag = 0;

    union Storage
    {
        struct
        {
            value_type* data;
            size_type size;
            size_type capacity;
        } large;
        u8 raw[k_sso_buf_size];
    } m_storage = {};
};

extern template class OPAL_EXPORT String<char8, EncodingUtf8<char8>>;

// Forward declare StringView so the StringLike concept can reference it.
template <typename CodeUnitType, typename EncodingType>
class StringView;

template <typename T>
inline constexpr bool k_is_string_value = false;

template <typename CodeUnitType, typename EncodingType>
inline constexpr bool k_is_string_value<String<CodeUnitType, EncodingType>> = true;

template <typename T>
inline constexpr bool k_is_string_view_value = false;

template <typename CodeUnitType, typename EncodingType>
inline constexpr bool k_is_string_view_value<StringView<CodeUnitType, EncodingType>> = true;

/**
 * @brief Concept that checks if a type is an instantiation of String or StringView.
 * @tparam T The type to be evaluated.
 */
template <typename T>
concept StringLike = k_is_string_value<T> || k_is_string_view_value<T>;

/**
 * @brief Concept that checks if a type is a string that owns its code units, so it can be grown, cloned and allocated into.
 * @tparam T The type to be evaluated.
 */
template <typename T>
concept OwningStringLike = k_is_string_value<T>;

/**
 * @brief Compare two strings lexicographically.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param second Second string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second.
 */
template <StringLike StringClass>
Expected<i32, ErrorCode> Compare(const StringClass& first, const StringClass& second);

/**
 * @brief Compare two strings lexicographically.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If you want to compare from pos1 to the end of the string, set
 * count1 to k_npos.
 * @param second Second string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string.
 */
template <StringLike StringClass>
Expected<i32, ErrorCode> Compare(const StringClass& first, typename StringClass::size_type pos1, typename StringClass::size_type count1,
                                 const StringClass& second);

/**
 * @brief Compare two strings lexicographically.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If you want to compare from pos1 to the end of the string, set
 * count1 to k_npos.
 * @param second Second string to compare.
 * @param pos2 Position in the second string to start comparing.
 * @param count2 Number of code units to compare in the second string. If you want to compare from pos2 to the end of the string, set
 * count2 to k_npos.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string or pos2 is greater than the size of the second string.
 */
template <StringLike StringClass>
Expected<i32, ErrorCode> Compare(const StringClass& first, typename StringClass::size_type pos1, typename StringClass::size_type count1,
                                 const StringClass& second, typename StringClass::size_type pos2, typename StringClass::size_type count2);

/**
 * @brief Compare a string with a null-terminated string lexicographically.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @note The null-terminated string is expected to be in the same encoding as the first string.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If count1 is equal to k_npos, the entire string starting from pos1
 * will be compared.
 * @param second Null-terminated string to compare.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string. Returns ErrorCode::InvalidArgument if second is nullptr.
 */
template <StringLike StringClass>
Expected<i32, ErrorCode> Compare(const StringClass& first, typename StringClass::size_type pos1, typename StringClass::size_type count1,
                                 const typename StringClass::value_type* second);

/**
 * @brief Compare a string with a null-terminated string lexicographically.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @note The null-terminated string is expected to be in the same encoding as the first string.
 * @param first First string to compare.
 * @param pos1 Position in the first string to start comparing.
 * @param count1 Number of code units to compare in the first string. If count1 is equal to k_npos, the entire string starting from pos1
 * will be compared.
 * @param second Null-terminated string to compare.
 * @param count2 Number of code units to compare in the second string. If count2 is equal to k_npos, the entire string starting from
 * second will be compared.
 * @return 0 if strings are equal, negative value if first is less than second, positive value if first is greater than second. Returns
 * ErrorCode::OutOfBounds if pos1 is greater than the size of the first string. Returns ErrorCode::InvalidArgument if second is nullptr.
 */
template <StringLike StringClass>
Expected<i32, ErrorCode> Compare(const StringClass& first, typename StringClass::size_type pos1, typename StringClass::size_type count1,
                                 const typename StringClass::value_type* second, typename StringClass::size_type count2);

template <OwningStringLike StringClass>
StringClass operator+(const StringClass& lhs, const StringClass& rhs);

template <OwningStringLike StringClass>
StringClass operator+(const StringClass& lhs, const typename StringClass::value_type* rhs);

template <OwningStringLike StringClass>
StringClass operator+(const StringClass& lhs, typename StringClass::value_type ch);

template <OwningStringLike StringClass>
StringClass operator+(const typename StringClass::value_type* lhs, const StringClass& rhs);

template <OwningStringLike StringClass>
StringClass operator+(typename StringClass::value_type ch, const StringClass& rhs);

/**
 * @brief Find the first occurrence of a string in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @return Position of the first occurrence of the needle in the haystack. If the needle is not found, returns StringClass::k_npos. If
 * start_pos is greater than the size of the haystack, returns StringClass::k_npos. If needle is empty and start_pos is not greater than
 * the size of the haystack, returns start_pos, so an empty needle matches at the end of the haystack.
 */
template <StringLike StringClass>
typename StringClass::size_type Find(const StringClass& haystack, const StringClass& needle, typename StringClass::size_type start_pos = 0);

/**
 * @brief Find the first occurrence of a string in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @param needle_count Number of code units to search for in the needle. Includes the null-terminator characters. If needle_count is
 * equal to StringClass::k_npos, the entire needle will be searched for until the first null-terminator character.
 * @return Position of the first occurrence of the needle in the haystack. If the needle is not found, returns StringClass::k_npos. If
 * start_pos is greater than the size of the haystack, returns StringClass::k_npos. If needle is empty and start_pos is not greater than
 * the size of the haystack, returns start_pos, so an empty needle matches at the end of the haystack.
 */
template <StringLike StringClass>
typename StringClass::size_type Find(const StringClass& haystack, const typename StringClass::value_type* needle,
                                     typename StringClass::size_type start_pos = 0,
                                     typename StringClass::size_type needle_count = StringClass::k_npos);

/**
 * @brief Find the first occurrence of a character in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param ch Character to search for.
 * @param start_pos Position in the haystack to start searching from.
 * @return Position of the first occurrence of the character in the haystack. If the character is not found, returns StringClass::k_npos. If
 * start_pos is greater than the size of the haystack, returns StringClass::k_npos. If haystack is empty, returns StringClass::k_npos.
 */
template <StringLike StringClass>
typename StringClass::size_type Find(const StringClass& haystack, const typename StringClass::value_type& ch,
                                     typename StringClass::size_type start_pos = 0);

/**
 * @brief Find the last occurrence of a string in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * the size of the haystack, the entire haystack will be searched.
 * @return Position of the last occurrence of the needle in the haystack. If the needle is not found, returns StringClass::k_npos. If needle
 * is empty, returns start_pos or if start_pos is larger than or equal to the size of the string the size of haystack will be returned.
 */
template <StringLike StringClass>
typename StringClass::size_type ReverseFind(const StringClass& haystack, const StringClass& needle,
                                            typename StringClass::size_type start_pos = StringClass::k_npos);

/**
 * @brief Find the last occurrence of a string in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param needle String to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * or equal to the size of the haystack, the entire haystack will be searched.
 * @param needle_count Number of code units to search for in the needle. Includes the null-terminator characters. If needle_count is
 * equal to StringClass::k_npos, the entire needle will be searched for until the first null-terminator character.
 * @return Position of the last occurrence of the needle in the haystack. If the needle is not found, returns StringClass::k_npos. If needle
 * is empty, returns start_pos or if start_pos is larger than or equal to the size of the string the size of haystack will be returned.
 */
template <StringLike StringClass>
typename StringClass::size_type ReverseFind(const StringClass& haystack, const typename StringClass::value_type* needle,
                                            typename StringClass::size_type start_pos = StringClass::k_npos,
                                            typename StringClass::size_type needle_count = StringClass::k_npos);

/**
 * @brief Find the last occurrence of a character in another string.
 * @tparam StringClass String type to search in.
 * @param haystack String to search in.
 * @param ch Character to search for.
 * @param start_pos Position in the haystack to start searching from. Search will start from right to left. If start_pos is greater than
 * the size of the haystack, the entire haystack will be searched.
 * @return Position of the last occurrence of the character in the haystack. If the character is not found, returns StringClass::k_npos. If
 * haystack is empty, returns StringClass::k_npos.
 */
template <StringLike StringClass>
typename StringClass::size_type ReverseFind(const StringClass& haystack, const typename StringClass::value_type& ch,
                                            typename StringClass::size_type start_pos = StringClass::k_npos);

/**
 * Transcode a string from one encoding to another.
 * @tparam InputStringClass Type of the input string. Defines code unit type and encoding. Needs to be DecodableEncoding.
 * @tparam InputStringClass Type of the output string. Defines code unit type and encoding. Needs to be EncodableEncoding.
 * @param input Input string to transcode.
 * @param output Output string to store the transcoded result. Grown as needed and resized to the length of the result, so it does not
 * need to be sized by the caller. Its contents are unspecified if transcoding fails.
 * @return ErrorCode::Success if transcoding was successful. ErrorCode::OutOfMemory when the output string cannot be grown, other error
 * codes depend on the encoding implementation.
 */
template <typename InputStringClass, typename OutputStringClass>
    requires Opal::DecodableEncoding<typename InputStringClass::encoding_type> &&
             Opal::EncodableEncoding<typename OutputStringClass::encoding_type>
ErrorCode Transcode(const InputStringClass& input, OutputStringClass& output);

/**
 * @brief Get a substring from a string.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @tparam Allocator Type of the allocator to use for allocating the result. If nullptr, the default allocator will be used.
 * @param str String to get the substring from.
 * @param start_pos Position in the string to start the substring from. Default is 0. May be equal to the size of the string, which yields
 * an empty substring.
 * @param count Number of code units to include in the substring. If count is equal to StringClass::k_npos, the entire string starting from
 * start_pos will be included.
 * @param allocator Allocator to use for allocating the result. If nullptr, the default allocator will be used. Ignored when StringClass
 * is a view, in which case the result is a view into the same buffer and nothing is allocated.
 * @return Substring in case of a success. ErrorCode::OutOfBounds if start_pos is greater than the size of the string.
 */
template <StringLike StringClass, typename Allocator = AllocatorBase>
Expected<StringClass, ErrorCode> GetSubString(const StringClass& str, typename StringClass::size_type start_pos = 0,
                                              typename StringClass::size_type count = StringClass::k_npos, Allocator* allocator = nullptr);

/**
 * @brief Get the length of a null-terminated string.
 * @tparam CodeUnitType Type of the code unit in the string.
 * @param str Null-terminated string to get the length of.
 * @return Length of the string. If str is nullptr, returns 0.
 */
template <typename CodeUnitType>
u64 GetStringLength(const CodeUnitType* str);

/**
 * @brief Check if a given string starts with a specified prefix.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param str String which to check.
 * @param prefix Prefix which to use.
 * @return Returns true if str starts with prefix, false otherwise.
 */
template <StringLike StringClass>
bool StartsWith(const StringClass& str, const StringClass& prefix);

/**
 * @brief Check if a given string ends with a specified suffix.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param str String which to check.
 * @param suffix Suffix which to use.
 * @return Returns true if str ends with suffix, false otherwise.
 */
template <StringLike StringClass>
bool EndsWith(const StringClass& str, const StringClass& suffix);

/**
 * @brief Split string into two parts around the specified delimiter.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param str String to split.
 * @param delimiter Pattern to find for the split.
 * @note When StringClass is a view, both parts are views into the buffer of str.
 * @param first String contents before the delimiter.
 * @param second String contents after the delimiter.
 * @return Returns true if delimiter is found and there are no errors extracting two parts of the string,
 * false otherwise. An empty delimiter never matches, in which case first receives the whole string.
 */
template <StringLike StringClass>
bool Split(const StringClass& str, const StringClass& delimiter, StringClass& first, StringClass& second);

/**
 * @brief Split string into multiple parts around the delimiter. Useful when delimiter occurs multiple times in the
 * input string.
 * @tparam StringClass Type of the string used. Defines code unit type, encoding and allocator.
 * @param str String to split.
 * @param delimiter Pattern to find for the split.
 * @note When StringClass is a view, the parts are views into the buffer of str.
 * @param result Array of string parts after the splitting.
 * @return Returns true if delimiter is found at least once and there are no errors extracting two parts
 * of the string, false otherwise. An empty delimiter never matches, in which case result receives the
 * whole string as a single part.
 */
template <StringLike StringClass>
bool SplitToArray(const StringClass& str, const StringClass& delimiter, DynamicArray<StringClass>& result);

/*************************************************************************************************/
/** Most common String specializations. **********************************************************/
/*************************************************************************************************/

using StringUtf8 = String<char8, EncodingUtf8<char8>>;
using StringUtf32 = String<uchar32, EncodingUtf32LE<uchar32>>;
using StringLocale = String<char8, EncodingLocale>;
using StringWide = String<char16, EncodingUtf16LE<char16>>;

/*************************************************************************************************/
/** Useful functions not implemented for all string classes. *************************************/
/*************************************************************************************************/

/**
 * Converts number to a string.
 * @param base Number base system used.
 * @param add_leading_zeros If true add leading zeros in binary, octal and hexadecimal bases for positive numbers up to the bit size of
 * the type.
 * @return Number as a string.
 * @throw OutOfMemoryException when default allocator runs out of memory.
 */
template <Integral T>
StringUtf8 NumberToString(T value, NumberSystemBase base = NumberSystemBase::Decimal, bool add_leading_zeros = false);

template <FloatingPoint T>
StringUtf8 NumberToString(T value, i32 decimal_points = -1);

/**
 * Converts a string to an integral type.
 * @tparam T Integral type to convert to.
 * @tparam StringClass String-like type that supports operator* returning a const char pointer (e.g. StringUtf8, StringViewUtf8).
 *         The string does not need to be null-terminated, only the code units it reports are read.
 * @param str String to convert.
 * @param base Number base system used. If 0, the base is auto-detected from the string prefix (0x for hex, 0 for octal, etc.).
 * @return Converted value.
 */
template <Integral T, StringLike StringClass>
T StringToNumber(const StringClass& str, i32 base = 0);
template <Integral T>
T StringToNumber(const char8* str, i32 base = 0);

};  // namespace Opal

#define TEMPLATE_HEADER template <typename CodeUnitType, typename EncodingType>
#define CLASS_HEADER Opal::String<CodeUnitType, EncodingType>

TEMPLATE_HEADER
CLASS_HEADER::String(allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    InitSmall(allocator, 0);
    m_storage.raw[0] = 0;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Construct(allocator_type* alloc, size_type count, CodeUnitType value)
{
    const ErrorCode error = InitStorage(alloc, count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* buf = GetData();
    for (size_type i = 0; i < count; i++)
    {
        buf[i] = value;
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Construct(allocator_type* alloc, const CodeUnitType* str, size_type count)
{
    if (str == nullptr && count > 0)
    {
        return ErrorCode::InvalidArgument;
    }
    const ErrorCode error = InitStorage(alloc, count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* buf = GetData();
    for (size_type i = 0; i < count; i++)
    {
        buf[i] = str[i];
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Construct(allocator_type* alloc, std::initializer_list<CodeUnitType> init_list)
{
    const size_type count = init_list.size();
    const ErrorCode error = InitStorage(alloc, count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* buf = GetData();
    for (size_type i = 0; i < count; i++)
    {
        buf[i] = *(init_list.begin() + i);
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
CLASS_HEADER::String(size_type count, CodeUnitType value, allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    if (Construct(allocator, count, value) != ErrorCode::Success) [[unlikely]]
    {
        // A constructor has no way to hand back a code, so allocation failure stays an exception here.
        throw OutOfMemoryException(allocator->GetName(), count);
    }
}

TEMPLATE_HEADER CLASS_HEADER::String(const String& other, size_type pos, allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    const size_type other_size = other.GetSize();
    if (pos > other_size)
    {
        throw OutOfBoundsException(pos, 0, other_size);
    }
    const size_type count = other_size - pos;
    const value_type* other_data = other.GetData();
    if (InitStorage(allocator, count) != ErrorCode::Success) [[unlikely]]
    {
        // A constructor has no way to hand back a code, so allocation failure stays an exception here.
        throw OutOfMemoryException(allocator->GetName(), count);
    }
    value_type* buf = GetData();
    for (size_type i = 0; i < count; i++)
    {
        buf[i] = other_data[pos + i];
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitType* str, size_type count, allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    const ErrorCode error = Construct(allocator, str, count);
    if (error == ErrorCode::InvalidArgument)
    {
        throw InvalidArgumentException(__FUNCTION__, "str", count);
    }
    if (error != ErrorCode::Success) [[unlikely]]
    {
        // A constructor has no way to hand back a code, so allocation failure stays an exception here.
        throw OutOfMemoryException(allocator->GetName(), count);
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(std::initializer_list<CodeUnitType> init_list, allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    if (Construct(allocator, init_list) != ErrorCode::Success) [[unlikely]]
    {
        // A constructor has no way to hand back a code, so allocation failure stays an exception here.
        throw OutOfMemoryException(allocator->GetName(), init_list.size());
    }
}

TEMPLATE_HEADER
CLASS_HEADER::String(const CodeUnitType* str, allocator_type* allocator)
{
    allocator = allocator == nullptr ? GetDefaultAllocator() : allocator;
    const size_type count = GetStringLength(str);
    if (Construct(allocator, str, count) != ErrorCode::Success) [[unlikely]]
    {
        // A constructor has no way to hand back a code, so allocation failure stays an exception here.
        throw OutOfMemoryException(allocator->GetName(), count);
    }
}

// TEMPLATE_HEADER
// CLASS_HEADER::String(const String& other, allocator_type* allocator)
//     : m_allocator(allocator == nullptr ? other.m_allocator : allocator), m_size(other.m_size), m_capacity(other.m_capacity)
// {
//     if (other.m_capacity > 0)
//     {
//         m_data = Allocate(other.m_capacity);
//         m_size = other.m_size;
//         m_capacity = other.m_capacity;
//         for (size_type i = 0; i < m_size; i++)
//         {
//             m_data[i] = other.GetData()[i];
//         }
//         m_data[m_size] = 0;
//     }
// }

TEMPLATE_HEADER
CLASS_HEADER::String(String&& other) noexcept
    : m_allocator_and_tag(other.m_allocator_and_tag)
{
    std::memcpy(&m_storage, &other.m_storage, k_sso_buf_size);
    allocator_type* alloc = other.GetAllocatorPtr();
    other.InitSmall(alloc, 0);
    other.m_storage.raw[0] = 0;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
CLASS_HEADER::String(InputIt start, InputIt end, allocator_type* allocator) : String(&(*start), Narrow<size_type>(end - start), allocator)
{
}

TEMPLATE_HEADER
Opal::String<CodeUnitType, EncodingType> CLASS_HEADER::Clone(AllocatorBase* allocator) const
{
    allocator = allocator == nullptr ? GetAllocatorPtr() : allocator;
    String out(GetData(), GetSize(), allocator);
    return out;
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::TryClone(AllocatorBase* allocator) const
{
    return Create(GetData(), GetSize(), allocator == nullptr ? GetAllocatorPtr() : allocator);
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::Create(size_type count, CodeUnitType value, allocator_type* allocator)
{
    using Result = Expected<String, ErrorCode>;
    String out(allocator);
    const ErrorCode error = out.Construct(out.GetAllocatorPtr(), count, value);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Result(error);
    }
    return Result(Move(out));
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::Create(const CodeUnitType* str, size_type count, allocator_type* allocator)
{
    using Result = Expected<String, ErrorCode>;
    String out(allocator);
    const ErrorCode error = out.Construct(out.GetAllocatorPtr(), str, count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Result(error);
    }
    return Result(Move(out));
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::Create(const CodeUnitType* str, allocator_type* allocator)
{
    return Create(str, GetStringLength(str), allocator);
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::Create(std::initializer_list<CodeUnitType> init_list,
                                                                    allocator_type* allocator)
{
    using Result = Expected<String, ErrorCode>;
    String out(allocator);
    const ErrorCode error = out.Construct(out.GetAllocatorPtr(), init_list);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Result(error);
    }
    return Result(Move(out));
}

TEMPLATE_HEADER
CLASS_HEADER::String::~String()
{
    if (!IsSmall() && m_storage.large.data != nullptr)
    {
        Deallocate(m_storage.large.data);
    }
}

// TEMPLATE_HEADER
// CLASS_HEADER& CLASS_HEADER::operator=(const String& other)
// {
//     // If the strings are the same, return early
//     if (this == &other)
//     {
//         return *this;
//     }
//
//     if (m_allocator != other.m_allocator)
//     {
//         if (m_allocator != nullptr)
//         {
//             Deallocate(m_data);
//         }
//         m_data = nullptr;
//         m_capacity = 0;
//         m_size = 0;
//         m_allocator = other.m_allocator;
//     }
//     if (m_capacity < other.m_size + 1)
//     {
//         Deallocate(m_data);
//         m_capacity = other.m_capacity;
//         m_data = Allocate(m_capacity);
//     }
//     m_size = other.m_size;
//     memcpy(m_data, other.m_data, m_size * sizeof(CodeUnitType));
//     m_data[m_size] = 0;
//     return *this;
// }

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(String&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (!IsSmall() && m_storage.large.data != nullptr)
    {
        Deallocate(m_storage.large.data);
    }
    m_allocator_and_tag = other.m_allocator_and_tag;
    std::memcpy(&m_storage, &other.m_storage, k_sso_buf_size);
    allocator_type* alloc = other.GetAllocatorPtr();
    other.InitSmall(alloc, 0);
    other.m_storage.raw[0] = 0;
    return *this;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const String& other) const
{
    const size_type my_size = GetSize();
    const size_type other_size = other.GetSize();
    if (my_size != other_size)
    {
        return false;
    }
    if (my_size == 0)
    {
        return true;
    }
    return std::memcmp(GetData(), other.GetData(), my_size * sizeof(value_type)) == 0;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const CodeUnitType* other) const
{
    const size_type my_size = GetSize();
    const size_type other_size = GetStringLength(other);
    if (my_size != other_size)
    {
        return false;
    }
    if (my_size == 0)
    {
        return true;
    }
    return std::memcmp(GetData(), other, my_size * sizeof(value_type)) == 0;
}

TEMPLATE_HEADER
std::strong_ordering CLASS_HEADER::operator<=>(const String& other) const
{
    const size_type my_size = GetSize();
    const size_type other_size = other.GetSize();
    const size_type count = my_size < other_size ? my_size : other_size;
    const value_type* my_data = GetData();
    const value_type* other_data = other.GetData();
    for (size_type i = 0; i < count; ++i)
    {
        if (my_data[i] != other_data[i])
        {
            return my_data[i] < other_data[i] ? std::strong_ordering::less : std::strong_ordering::greater;
        }
    }
    return my_size <=> other_size;
}

TEMPLATE_HEADER
std::strong_ordering CLASS_HEADER::operator<=>(const CodeUnitType* other) const
{
    const size_type my_size = GetSize();
    const size_type other_size = GetStringLength(other);
    const size_type count = my_size < other_size ? my_size : other_size;
    const value_type* my_data = GetData();
    for (size_type i = 0; i < count; ++i)
    {
        if (my_data[i] != other[i])
        {
            return my_data[i] < other[i] ? std::strong_ordering::less : std::strong_ordering::greater;
        }
    }
    return my_size <=> other_size;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(size_type count, CodeUnitType value)
{
    const ErrorCode error = PrepareForOverwrite(count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* data = GetData();
    for (size_type i = 0; i < count; i++)
    {
        data[i] = value;
    }
    SetSize(count);
    data[count] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const String& other)
{
    if (this == &other)
    {
        return ErrorCode::Success;
    }
    const size_type other_size = other.GetSize();
    const ErrorCode error = PrepareForOverwrite(other_size);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* data = GetData();
    if (other_size > 0)
    {
        std::memcpy(data, other.GetData(), other_size * sizeof(CodeUnitType));
    }
    SetSize(other_size);
    data[other_size] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const String& other, size_type pos, size_type count)
{
    if (this == &other)
    {
        return ErrorCode::SelfNotAllowed;
    }
    const size_type other_size = other.GetSize();
    if (pos > other_size)
    {
        return ErrorCode::OutOfBounds;
    }
    if (count == k_npos)
    {
        count = other_size - pos;
    }
    if (count > other_size - pos)
    {
        return ErrorCode::OutOfBounds;
    }
    const ErrorCode error = PrepareForOverwrite(count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* data = GetData();
    if (count > 0)
    {
        std::memcpy(data, other.GetData() + pos, count * sizeof(CodeUnitType));
    }
    SetSize(count);
    data[count] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::Assign(String&& other)
{
    *this = Move(other);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(std::initializer_list<CodeUnitType> init_list)
{
    const size_type count = init_list.size();
    const ErrorCode error = PrepareForOverwrite(count);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* data = GetData();
    for (size_type i = 0; i < count; i++)
    {
        data[i] = *(init_list.begin() + i);
    }
    SetSize(count);
    data[count] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(std::initializer_list<CodeUnitType> init_list)
{
    // An assignment operator has no way to hand back a code, so allocation failure stays an exception here.
    if (Assign(init_list) != ErrorCode::Success) [[unlikely]]
    {
        throw OutOfMemoryException(GetAllocator().GetName(), init_list.size());
    }
    return *this;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Assign(const CodeUnitType* str, size_type count)
{
    if (str == nullptr)
    {
        return ErrorCode::InvalidArgument;
    }
    u64 str_size = GetStringLength(str);
    if (count == k_npos)
    {
        count = str_size;
    }
    if (count > str_size)
    {
        return ErrorCode::OutOfBounds;
    }
    PrepareForOverwrite(count);
    value_type* data = GetData();
    for (size_type i = 0; i < count; i++)
    {
        data[i] = str[i];
    }
    SetSize(count);
    data[count] = 0;
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
    const value_type* my_data = GetData();
    const size_type my_size = GetSize();
    if (&(*start_it) >= my_data && &(*start_it) < my_data + my_size)
    {
        return ErrorCode::SelfNotAllowed;
    }
    u64 count = static_cast<u64>(end_it - start_it);
    PrepareForOverwrite(count);
    value_type* data = GetData();
    for (size_type i = 0; i < count; i++)
    {
        data[i] = *start_it;
        start_it++;
    }
    SetSize(count);
    data[count] = 0;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
CodeUnitType& CLASS_HEADER::At(size_type pos)
{
    OPAL_ASSERT(pos < GetSize(), "Index out of bounds");
    return GetData()[pos];
}

TEMPLATE_HEADER
const CodeUnitType& CLASS_HEADER::At(size_type pos) const
{
    OPAL_ASSERT(pos < GetSize(), "Index out of bounds");
    return GetData()[pos];
}

TEMPLATE_HEADER
CodeUnitType& CLASS_HEADER::operator[](size_type pos)
{
    const size_type sz = GetSize();
    if (pos >= sz)
    {
        throw OutOfBoundsException(pos, 0, sz == 0 ? 0 : sz - 1);
    }
    return GetData()[pos];
}

TEMPLATE_HEADER
const CodeUnitType& CLASS_HEADER::operator[](size_type pos) const
{
    const size_type sz = GetSize();
    if (pos >= sz)
    {
        throw OutOfBoundsException(pos, 0, sz == 0 ? 0 : sz - 1);
    }
    return GetData()[pos];
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Front()
{
    using ReturnType = Expected<CodeUnitType&, ErrorCode>;
    if (GetSize() == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(GetData()[0]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Front() const
{
    using ReturnType = Expected<const CodeUnitType&, ErrorCode>;
    if (GetSize() == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(GetData()[0]);
}

TEMPLATE_HEADER
Opal::Expected<CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Back()
{
    using ReturnType = Expected<CodeUnitType&, ErrorCode>;
    const size_type sz = GetSize();
    if (sz == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(GetData()[sz - 1]);
}

TEMPLATE_HEADER
Opal::Expected<const CodeUnitType&, Opal::ErrorCode> CLASS_HEADER::Back() const
{
    using ReturnType = Expected<const CodeUnitType&, ErrorCode>;
    const size_type sz = GetSize();
    if (sz == 0)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    return ReturnType(GetData()[sz - 1]);
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Reserve(size_type new_capacity)
{
    if (new_capacity <= GetCapacity())
    {
        return ErrorCode::Success;
    }
    value_type* new_data = Allocate(new_capacity);
    if (new_data == nullptr) [[unlikely]]
    {
        return ErrorCode::OutOfMemory;
    }
    if (IsSmall())
    {
        // Transition from small to large. Save inline data before overwriting the union.
        const size_type old_size = GetSmallSize();
        if (old_size > 0)
        {
            std::memcpy(new_data, GetSmallData(), (old_size + 1) * sizeof(value_type));
        }
        else
        {
            new_data[0] = 0;
        }
        allocator_type* alloc = GetAllocatorPtr();
        InitLarge(alloc);
        m_storage.large.data = new_data;
        m_storage.large.size = old_size;
        m_storage.large.capacity = new_capacity;
    }
    else
    {
        if (m_storage.large.data != nullptr)
        {
            std::memcpy(new_data, m_storage.large.data, (m_storage.large.size + 1) * sizeof(value_type));
            Deallocate(m_storage.large.data);
        }
        m_storage.large.data = new_data;
        m_storage.large.capacity = new_capacity;
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(size_type new_size, CodeUnitType value)
{
    const size_type old_size = GetSize();
    if (new_size == old_size)
    {
        return ErrorCode::Success;
    }
    if (new_size < old_size)
    {
        SetSize(new_size);
        GetData()[new_size] = 0;
        return ErrorCode::Success;
    }
    if (!IsValidSize(new_size))
    {
        return ErrorCode::OutOfMemory;
    }
    const ErrorCode error = Grow(new_size + 1);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return error;
    }
    value_type* data = GetData();
    for (size_type i = old_size; i < new_size; i++)
    {
        data[i] = value;
    }
    data[new_size] = 0;
    SetSize(new_size);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Resize(size_type new_size)
{
    return Resize(new_size, CodeUnitType());
}

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    SetSize(0);
    GetData()[0] = 0;
}

TEMPLATE_HEADER
void CLASS_HEADER::PopBack()
{
    const size_type sz = GetSize();
    if (sz == 0)
    {
        return;
    }
    SetSize(sz - 1);
    GetData()[sz - 1] = 0;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::ShrinkToFit()
{
    if (IsSmall())
    {
        return ErrorCode::Success;
    }
    const size_type sz = GetSize();
    if (sz + 1 == m_storage.large.capacity)
    {
        return ErrorCode::Success;
    }
    value_type* old_data = m_storage.large.data;
    if (sz + 1 <= k_sso_capacity)
    {
        allocator_type* alloc = GetAllocatorPtr();
        InitSmall(alloc, sz);
        value_type* buf = GetSmallData();
        std::memcpy(buf, old_data, (sz + 1) * sizeof(value_type));
        Deallocate(old_data);
        return ErrorCode::Success;
    }
    value_type* new_data = Allocate(sz + 1);
    if (new_data == nullptr) [[unlikely]]
    {
        return ErrorCode::OutOfMemory;
    }
    std::memcpy(new_data, old_data, (sz + 1) * sizeof(value_type));
    Deallocate(old_data);
    m_storage.large.data = new_data;
    m_storage.large.capacity = sz + 1;
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Replace(size_type start_pos, size_type count,
                                                                                       const CodeUnitType* other, size_type other_count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    const size_type sz = GetSize();
    if (start_pos > sz)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (other == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    if (other_count == k_npos)
    {
        other_count = GetStringLength(other);
    }
    count = Min(count, sz - start_pos);
    if (GetInternalOffset(other) != k_npos)
    {
        String source(GetAllocatorPtr());
        const ErrorCode error = source.Append(other, other_count);
        if (error != ErrorCode::Success) [[unlikely]]
        {
            return ReturnType(error);
        }
        return Replace(start_pos, count, source.GetData(), other_count);
    }
    const size_type tail = sz - start_pos - count;
    if (other_count > count)
    {
        const ErrorCode error = GrowForAppend(sz, other_count - count);
        if (error != ErrorCode::Success) [[unlikely]]
        {
            return ReturnType(error);
        }
        value_type* data = GetData();
        const size_type shift = other_count - count;
        for (size_type i = 0; i < tail; ++i)
        {
            data[sz + shift - 1 - i] = data[sz - 1 - i];
        }
    }
    else if (other_count < count)
    {
        value_type* data = GetData();
        for (size_type i = 0; i < tail; ++i)
        {
            data[start_pos + other_count + i] = data[start_pos + count + i];
        }
    }
    value_type* data = GetData();
    for (size_type i = 0; i < other_count; ++i)
    {
        data[start_pos + i] = other[i];
    }
    const size_type new_size = sz - count + other_count;
    data[new_size] = 0;
    SetSize(new_size);
    return ReturnType(iterator(data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Replace(size_type start_pos, size_type count,
                                                                                       const String& other)
{
    return Replace(start_pos, count, other.GetData(), other.GetSize());
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Replace(const_iterator first, const_iterator last,
                                                                                       const String& other)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (first < ConstBegin() || first > ConstEnd() || last < ConstBegin() || last > ConstEnd())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (first > last)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    const size_type start_pos = Narrow<size_type>(first - ConstBegin());
    const size_type count = Narrow<size_type>(last - first);
    return Replace(start_pos, count, other);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::size_type, Opal::ErrorCode> CLASS_HEADER::ReplaceAll(const String& needle, const String& replacement)
{
    using ReturnType = Expected<size_type, ErrorCode>;
    if (needle.IsEmpty())
    {
        return ReturnType(size_type(0));
    }
    if (&needle == this || &replacement == this)
    {
        String needle_copy(GetAllocatorPtr());
        String replacement_copy(GetAllocatorPtr());
        if (needle_copy.Append(needle) != ErrorCode::Success || replacement_copy.Append(replacement) != ErrorCode::Success) [[unlikely]]
        {
            return ReturnType(ErrorCode::OutOfMemory);
        }
        return ReplaceAll(needle_copy, replacement_copy);
    }
    const size_type needle_size = needle.GetSize();
    const size_type replacement_size = replacement.GetSize();
    size_type replaced = 0;
    size_type start_pos = 0;
    while (true)
    {
        const size_type pos = Find(*this, needle, start_pos);
        if (pos == k_npos)
        {
            return ReturnType(replaced);
        }
        const auto result = Replace(pos, needle_size, replacement);
        if (!result.HasValue()) [[unlikely]]
        {
            return ReturnType(result.GetError());
        }
        ++replaced;
        start_pos = pos + replacement_size;
    }
}

TEMPLATE_HEADER
Opal::Expected<bool, Opal::ErrorCode> CLASS_HEADER::ReplaceFirst(const String& needle, const String& replacement)
{
    using ReturnType = Expected<bool, ErrorCode>;
    if (needle.IsEmpty())
    {
        return ReturnType(false);
    }
    if (&needle == this || &replacement == this)
    {
        String needle_copy(GetAllocatorPtr());
        String replacement_copy(GetAllocatorPtr());
        if (needle_copy.Append(needle) != ErrorCode::Success || replacement_copy.Append(replacement) != ErrorCode::Success) [[unlikely]]
        {
            return ReturnType(ErrorCode::OutOfMemory);
        }
        return ReplaceFirst(needle_copy, replacement_copy);
    }
    const size_type pos = Find(*this, needle);
    if (pos == k_npos)
    {
        return ReturnType(false);
    }
    const auto result = Replace(pos, needle.GetSize(), replacement);
    if (!result.HasValue()) [[unlikely]]
    {
        return ReturnType(result.GetError());
    }
    return ReturnType(true);
}

TEMPLATE_HEADER
Opal::Expected<CLASS_HEADER, Opal::ErrorCode> CLASS_HEADER::GetSubString(size_type start_pos, size_type count,
                                                                        allocator_type* allocator) const
{
    return Opal::GetSubString(*this, start_pos, count, allocator);
}

TEMPLATE_HEADER
bool CLASS_HEADER::Contains(const String& needle) const
{
    return Find(*this, needle) != k_npos;
}

TEMPLATE_HEADER
bool CLASS_HEADER::Contains(const CodeUnitType* needle) const
{
    return Find(*this, needle) != k_npos;
}

TEMPLATE_HEADER
bool CLASS_HEADER::Contains(CodeUnitType ch) const
{
    return Find(*this, ch) != k_npos;
}

TEMPLATE_HEADER
void CLASS_HEADER::Trim()
{
    const size_type my_size = GetSize();
    const size_type real_size = GetStringLength(GetData());
    if (my_size != 0 && my_size > real_size)
    {
        SetSize(real_size);
        GetData()[real_size] = 0;
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::Strip()
{
    const size_type my_size = GetSize();
    if (my_size == 0)
    {
        return;
    }
    auto is_whitespace = [](CodeUnitType ch) -> bool
    { return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'; };
    size_type start = 0;
    while (start < my_size && is_whitespace(GetData()[start]))
    {
        ++start;
    }
    size_type end = my_size;
    while (end > start && is_whitespace(GetData()[end - 1]))
    {
        --end;
    }
    const size_type new_size = end - start;
    if (start > 0 && new_size > 0)
    {
        for (size_type i = 0; i < new_size; ++i)
        {
            GetData()[i] = GetData()[start + i];
        }
    }
    SetSize(new_size);
    GetData()[new_size] = 0;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const value_type& ch)
{
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, 1);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    data[sz] = ch;
    sz += 1;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const value_type* str, size_type size)
{
    if (str == nullptr)
    {
        return ErrorCode::InvalidArgument;
    }
    if (size == k_npos)
    {
        size = GetStringLength(str);
    }
    size_type sz = GetSize();
    const size_type alias_offset = GetInternalOffset(str);
    const ErrorCode grow_error = GrowForAppend(sz, size);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    if (size > 0)
    {
        const value_type* source = alias_offset == k_npos ? str : data + alias_offset;
        std::memmove(data + sz, source, size * sizeof(value_type));
    }
    sz += size;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(size_type count, CodeUnitType value)
{
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    for (size_type i = sz; i < sz + count; i++)
    {
        data[i] = value;
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const String& other)
{
    size_type sz = GetSize();
    const size_type other_size = other.GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, other_size);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    if (other_size > 0)
    {
        std::memcpy(data + sz, other.GetData(), other_size * sizeof(CodeUnitType));
    }
    sz += other_size;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::ErrorCode CLASS_HEADER::Append(const String& other, size_type pos, size_type count)
{
    const size_type other_size = other.GetSize();
    if (pos >= other_size)
    {
        return ErrorCode::OutOfBounds;
    }
    if (count == k_npos)
    {
        count = other_size - pos;
    }
    if (count > other_size - pos)
    {
        return ErrorCode::OutOfBounds;
    }
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    if (count > 0)
    {
        std::memcpy(data + sz, other.GetData() + pos, count * sizeof(CodeUnitType));
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Append(InputIt begin_it, InputIt end_it)
{
    if (begin_it > end_it)
    {
        return ErrorCode::InvalidArgument;
    }
    const value_type* my_data = GetData();
    const size_type my_size = GetSize();
    if (&(*begin_it) >= my_data && &(*begin_it) < my_data + my_size)
    {
        return ErrorCode::SelfNotAllowed;
    }
    u64 count = static_cast<u64>(end_it - begin_it);
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return grow_error;
    }
    value_type* data = GetData();
    for (size_type i = 0; i < count; i++)
    {
        data[sz + i] = *begin_it;
        begin_it++;
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ErrorCode::Success;
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, size_type count,
                                                                                      CodeUnitType value)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    size_type sz = GetSize();
    if (start_pos > sz)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return ReturnType(iterator(GetData() + start_pos));
    }
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = value;
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, const CodeUnitType* str,
                                                                                      size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    size_type sz = GetSize();
    if (start_pos > sz)
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
        return ReturnType(iterator(GetData() + start_pos));
    }
    if (GetInternalOffset(str) != k_npos)
    {
        String source(GetAllocatorPtr());
        const ErrorCode error = source.Append(str, count);
        if (error != ErrorCode::Success) [[unlikely]]
        {
            return ReturnType(error);
        }
        return Insert(start_pos, source.GetData(), count);
    }
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = str[i - start_pos];
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Insert(size_type start_pos, const String& other,
                                                                                      size_type other_start_pos, size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    size_type sz = GetSize();
    if (start_pos > sz)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    const size_type other_size = other.GetSize();
    if (other_start_pos > other_size)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == k_npos)
    {
        count = other_size - other_start_pos;
    }
    if (count > other_size - other_start_pos)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count == 0)
    {
        return ReturnType(iterator(GetData() + start_pos));
    }
    if (&other == this)
    {
        return Insert(start_pos, other.GetData() + other_start_pos, count);
    }
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = other.At(other_start_pos + i - start_pos);
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
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
    const size_type start_pos = Narrow<size_type>(start - Begin());
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = value;
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
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
        return ReturnType(iterator(GetData() + (start - ConstBegin())));
    }
    const size_type start_pos = Narrow<size_type>(start - ConstBegin());
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = value;
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
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
    const size_type count = Narrow<size_type>(end - begin);
    if (count == 0)
    {
        return ReturnType(start);
    }
    const value_type* my_data = GetData();
    if (&(*begin) >= my_data && &(*begin) < my_data + GetSize())
    {
        return ReturnType(ErrorCode::SelfNotAllowed);
    }
    const size_type start_pos = Narrow<size_type>(start - Begin());
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = *(begin + Narrow<difference_type>(i - start_pos));
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
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
    const size_type count = Narrow<size_type>(end - begin);
    if (count == 0)
    {
        return ReturnType(iterator(GetData() + (start - ConstBegin())));
    }
    const value_type* my_data = GetData();
    if (&(*begin) >= my_data && &(*begin) < my_data + GetSize())
    {
        return ReturnType(ErrorCode::SelfNotAllowed);
    }
    const size_type start_pos = Narrow<size_type>(start - ConstBegin());
    size_type sz = GetSize();
    const ErrorCode grow_error = GrowForAppend(sz, count);
    if (grow_error != ErrorCode::Success) [[unlikely]]
    {
        return ReturnType(grow_error);
    }
    value_type* data = GetData();
    for (size_type i = sz - 1; i >= start_pos && i != k_npos; --i)
    {
        data[i + count] = data[i];
    }
    for (size_type i = start_pos; i < start_pos + count; ++i)
    {
        data[i] = *(begin + static_cast<difference_type>(i - start_pos));
    }
    sz += count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(size_type start_pos, size_type count)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    size_type sz = GetSize();
    if (start_pos > sz)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = Min(sz - start_pos, count);
    value_type* data = GetData();
    for (size_type i = start_pos; i < sz - count; i++)
    {
        data[i] = data[i + count];
    }
    sz -= count;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_pos));
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
    size_type sz = GetSize();
    value_type* data = GetData();
    for (size_type i = start_index; i < sz - 1; ++i)
    {
        data[i] = data[i + 1];
    }
    sz -= 1;
    data[sz] = 0;
    SetSize(sz);
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

    const size_type start_index = Narrow<size_type>(pos - ConstBegin());
    size_type sz = GetSize();
    value_type* data = GetData();
    for (size_type i = start_index; i < sz - 1; ++i)
    {
        data[i] = data[i + 1];
    }
    sz -= 1;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_index));
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(iterator first, iterator last)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (first < Begin() || first > End())
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

    const size_type count_to_erase = Narrow<size_type>(last - first);
    const size_type start_index = Narrow<size_type>(first - Begin());
    size_type sz = GetSize();
    value_type* data = GetData();
    for (size_type i = start_index; i < sz - count_to_erase; ++i)
    {
        data[i] = data[i + count_to_erase];
    }
    sz -= count_to_erase;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(first);
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(const_iterator first, const_iterator last)
{
    using ReturnType = Expected<iterator, ErrorCode>;
    if (first < ConstBegin() || first > ConstEnd())
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
        return ReturnType(iterator(GetData() + Narrow<size_type>(first - ConstBegin())));
    }

    const size_type count_to_erase = Narrow<size_type>(last - first);
    const size_type start_index = Narrow<size_type>(first - ConstBegin());
    size_type sz = GetSize();
    value_type* data = GetData();
    for (size_type i = start_index; i < sz - count_to_erase; ++i)
    {
        data[i] = data[i + count_to_erase];
    }
    sz -= count_to_erase;
    data[sz] = 0;
    SetSize(sz);
    return ReturnType(iterator(data + start_index));
}

template <typename CodeUnitType, typename EncodingType>
void Opal::String<CodeUnitType, EncodingType>::Reverse()
{
    Reverse(begin(), end());
}

template <typename CodeUnitType, typename EncodingType>
void Opal::String<CodeUnitType, EncodingType>::Reverse(iterator start_it, iterator end_it)
{
    if (start_it == end_it || start_it == end_it - 1)
    {
        return;
    }
    if (start_it > end_it) [[unlikely]]
    {
        throw InvalidArgumentException(__FUNCTION__, "end_it - start_it", end_it - start_it);
    }

    iterator last_it = end_it - 1;
    while (start_it < last_it)
    {
        CodeUnitType c = *start_it;
        *start_it = *last_it;
        *last_it = c;
        ++start_it;
        --last_it;
    }
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const String& other)
{
    // These operators have no way to hand back a code, so allocation failure stays an exception here.
    if (Append(other) != ErrorCode::Success) [[unlikely]]
    {
        throw OutOfMemoryException(GetAllocator().GetName(), GetSize() + other.GetSize());
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(value_type ch)
{
    if (Append(ch) != ErrorCode::Success) [[unlikely]]
    {
        throw OutOfMemoryException(GetAllocator().GetName(), GetSize() + 1);
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(const value_type* str)
{
    // A null str is reported as InvalidArgument, which this operator has always ignored.
    if (Append(str) == ErrorCode::OutOfMemory) [[unlikely]]
    {
        throw OutOfMemoryException(GetAllocator().GetName(), GetSize() + GetStringLength(str));
    }
    return *this;
}

TEMPLATE_HEADER
CodeUnitType* CLASS_HEADER::Allocate(size_type size)
{
    allocator_type* alloc = GetAllocatorPtr();
    if (alloc == nullptr)
    {
        return nullptr;
    }
    constexpr u64 k_alignment = alignof(CodeUnitType);
    if (size > k_max_size + 1)
    {
        return nullptr;
    }
    const u64 size_bytes = size * sizeof(value_type);
    return reinterpret_cast<value_type*>(alloc->Alloc(size_bytes, k_alignment));
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(value_type* data)
{
    // Every constructor stores an allocator, so a null one cannot happen. The destructor reaches this,
    // and throwing from there terminates, so the impossible case is a no-op rather than an exception.
    allocator_type* alloc = GetAllocatorPtr();
    if (alloc == nullptr)
    {
        return;
    }
    alloc->Free(data);
}

template <typename InputStringClass, typename OutputStringClass>
    requires Opal::DecodableEncoding<typename InputStringClass::encoding_type> &&
             Opal::EncodableEncoding<typename OutputStringClass::encoding_type>
Opal::ErrorCode Opal::Transcode(const InputStringClass& input, OutputStringClass& output)
{
    using output_value_type = typename OutputStringClass::value_type;
    typename InputStringClass::encoding_type src_decoder;
    typename OutputStringClass::encoding_type dst_encoder;
    ArrayView<const typename InputStringClass::value_type> input_span(input.GetData(), input.GetSize());
    if (output.GetSize() < input.GetSize())
    {
        const ErrorCode resize_error = output.Resize(input.GetSize());
        if (resize_error != ErrorCode::Success) [[unlikely]]
        {
            return resize_error;
        }
    }
    ArrayView<output_value_type> output_span(output.GetData(), output.GetSize());
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
        if (error == ErrorCode::InsufficientSpace)
        {
            const typename OutputStringClass::size_type written = output.GetSize() - output_span.GetSize();
            const ErrorCode resize_error = output.Resize(output.GetSize() * 2 + 8);
            if (resize_error != ErrorCode::Success) [[unlikely]]
            {
                return resize_error;
            }
            output_span = ArrayView<output_value_type>(output.GetData() + written, output.GetData() + output.GetSize());
            error = dst_encoder.EncodeOne(code_point, output_span);
        }
        if (error != ErrorCode::Success)
        {
            return error;
        }
    }
    return output.Resize(output.GetSize() - output_span.GetSize());
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename StringClass>
#define CLASS_HEADER Opal::StringIterator<StringClass>

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
    ++m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    StringIterator temp = *this;
    ++m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    StringIterator temp = *this;
    --m_ptr;
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
CLASS_HEADER Opal::operator+(typename StringIterator<StringClass>::difference_type n, const StringIterator<StringClass>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename StringClass>
#define CLASS_HEADER Opal::StringConstIterator<StringClass>

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
    ++m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    StringConstIterator temp = *this;
    ++m_ptr;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    --m_ptr;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    StringConstIterator temp = *this;
    --m_ptr;
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
CLASS_HEADER Opal::operator+(typename StringConstIterator<StringClass>::difference_type n, const StringConstIterator<StringClass>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename StringClass, typename BaseIterator>
#define CLASS_HEADER Opal::StringReverseIteratorBase<StringClass, BaseIterator>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const StringReverseIteratorBase& other) const
{
    return m_base < other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const StringReverseIteratorBase& other) const
{
    return m_base <= other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const StringReverseIteratorBase& other) const
{
    return m_base > other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const StringReverseIteratorBase& other) const
{
    return m_base >= other.m_base;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    --m_base;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    StringReverseIteratorBase temp = *this;
    --m_base;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    ++m_base;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    StringReverseIteratorBase temp = *this;
    ++m_base;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return StringReverseIteratorBase(m_base - n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return StringReverseIteratorBase(m_base + n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_base -= n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_base += n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const StringReverseIteratorBase& other) const
{
    return other.m_base - m_base;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(*this + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    BaseIterator element = m_base;
    --element;
    return *element;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    BaseIterator element = m_base;
    --element;
    return element.operator->();
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename StringReverseIteratorBase<StringClass, BaseIterator>::difference_type n,
                             const StringReverseIteratorBase<StringClass, BaseIterator>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

template <Opal::StringLike StringClass>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const StringClass& first, const StringClass& second)
{
    return Compare(first, 0, first.GetSize(), second, 0, second.GetSize());
}

template <Opal::StringLike StringClass>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const StringClass& first, typename StringClass::size_type pos1,
                                                         typename StringClass::size_type count1, const StringClass& second)
{
    return Compare(first, pos1, count1, second, 0, second.GetSize());
}

template <Opal::StringLike StringClass>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const StringClass& first, typename StringClass::size_type pos1,
                                                         typename StringClass::size_type count1, const StringClass& second,
                                                         typename StringClass::size_type pos2, typename StringClass::size_type count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename StringClass::size_type;

    if (pos1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (pos2 > second.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count1 == StringClass::k_npos)
    {
        count1 = first.GetSize() - pos1;
    }
    if (count2 == StringClass::k_npos)
    {
        count2 = second.GetSize() - pos2;
    }
    if (pos1 + count1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (pos2 + count2 > second.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

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

template <Opal::StringLike StringClass>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const StringClass& first, typename StringClass::size_type pos1,
                                                         typename StringClass::size_type count1,
                                                         const typename StringClass::value_type* second)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename StringClass::size_type;

    if (pos1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count1 == StringClass::k_npos)
    {
        count1 = first.GetSize() - pos1;
    }
    if (pos1 + count1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    size_type count2 = GetStringLength(second);

    size_type count = count2 > count1 ? count1 : count2;
    for (size_type i = 0; i < count; i++)
    {
        if (first[pos1 + i] < second[i])
        {
            return ReturnType(-1);
        }
        if (first[pos1 + i] > second[i])
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

template <Opal::StringLike StringClass>
Opal::Expected<Opal::i32, Opal::ErrorCode> Opal::Compare(const StringClass& first, typename StringClass::size_type pos1,
                                                         typename StringClass::size_type count1,
                                                         const typename StringClass::value_type* second,
                                                         typename StringClass::size_type count2)
{
    using ReturnType = Expected<i32, ErrorCode>;
    using size_type = typename StringClass::size_type;

    if (pos1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (count1 == StringClass::k_npos)
    {
        count1 = first.GetSize() - pos1;
    }
    if (pos1 + count1 > first.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    if (second == nullptr)
    {
        return ReturnType(ErrorCode::InvalidArgument);
    }
    size_type str_length = GetStringLength(second);
    if (count2 == StringClass::k_npos)
    {
        count2 = str_length;
    }
    if (count2 > str_length)
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }

    size_type count = count2 > count1 ? count1 : count2;
    for (size_type i = 0; i < count; ++i)
    {
        if (first[pos1 + i] < second[i])
        {
            return ReturnType(-1);
        }
        if (first[pos1 + i] > second[i])
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

template <Opal::OwningStringLike StringClass>
StringClass Opal::operator+(const StringClass& lhs, const StringClass& rhs)
{
    StringClass result = lhs.Clone();
    result += rhs;
    return result;
}

template <Opal::OwningStringLike StringClass>
StringClass Opal::operator+(const StringClass& lhs, const typename StringClass::value_type* rhs)
{
    StringClass result = lhs.Clone();
    result += rhs;
    return result;
}

template <Opal::OwningStringLike StringClass>
StringClass Opal::operator+(const StringClass& lhs, typename StringClass::value_type ch)
{
    StringClass result = lhs.Clone();
    result += ch;
    return result;
}

template <Opal::OwningStringLike StringClass>
StringClass Opal::operator+(const typename StringClass::value_type* lhs, const StringClass& rhs)
{
    StringClass result;
    result += lhs;
    result += rhs;
    return result;
}

template <Opal::OwningStringLike StringClass>
StringClass Opal::operator+(typename StringClass::value_type ch, const StringClass& rhs)
{
    StringClass result;
    result += ch;
    result += rhs;
    return result;
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::Find(const StringClass& haystack, const StringClass& needle,
                                           typename StringClass::size_type start_pos)
{
    if (needle.IsEmpty())
    {
        if (start_pos > haystack.GetSize())
        {
            return StringClass::k_npos;
        }
        return start_pos;
    }
    return Find(haystack, needle.GetData(), start_pos, needle.GetSize());
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::Find(const StringClass& haystack, const typename StringClass::value_type* needle,
                                           typename StringClass::size_type start_pos, typename StringClass::size_type needle_count)
{
    if (needle == nullptr)
    {
        return StringClass::k_npos;
    }
    if (needle_count == StringClass::k_npos)
    {
        needle_count = 0;
        while (needle[needle_count] != 0)
        {
            ++needle_count;
        }
    }
    if (needle_count == 0)
    {
        if (start_pos > haystack.GetSize())
        {
            return StringClass::k_npos;
        }
        return start_pos;
    }
    if (start_pos >= haystack.GetSize() || needle_count > haystack.GetSize() - start_pos)
    {
        return StringClass::k_npos;
    }
    for (typename StringClass::size_type haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        bool is_found = false;
        for (typename StringClass::size_type needle_pos = 0; needle_pos < needle_count; ++needle_pos)
        {
            // We found the symbol only if there is enough of a haystack to find it
            is_found = haystack_pos + needle_pos < haystack.GetSize() && needle[needle_pos] == haystack.At(haystack_pos + needle_pos);
            if (!is_found)
            {
                break;
            }
        }
        if (is_found)
        {
            return haystack_pos;
        }
    }
    return StringClass::k_npos;
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::Find(const StringClass& haystack, const typename StringClass::value_type& ch,
                                           typename StringClass::size_type start_pos)
{
    if (haystack.IsEmpty())
    {
        return StringClass::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        return StringClass::k_npos;
    }
    for (typename StringClass::size_type haystack_pos = start_pos; haystack_pos < haystack.GetSize(); ++haystack_pos)
    {
        if (haystack.At(haystack_pos) == ch)
        {
            return haystack_pos;
        }
    }
    return StringClass::k_npos;
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::ReverseFind(const StringClass& haystack, const StringClass& needle,
                                                  typename StringClass::size_type start_pos)
{
    if (needle.IsEmpty())
    {
        return start_pos >= haystack.GetSize() ? haystack.GetSize() : start_pos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize();
    }
    if (needle.GetSize() > start_pos + 1)
    {
        return StringClass::k_npos;
    }
    for (typename StringClass::size_type haystack_pos = start_pos - needle.GetSize(); haystack_pos != StringClass::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename StringClass::size_type needle_pos = 0; needle_pos < needle.GetSize(); ++needle_pos)
        {
            if (needle[needle_pos] != haystack.At(haystack_pos + needle_pos))
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
    return StringClass::k_npos;
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::ReverseFind(const StringClass& haystack, const typename StringClass::value_type* needle,
                                                  typename StringClass::size_type start_pos, typename StringClass::size_type needle_count)
{
    if (needle == nullptr)
    {
        return StringClass::k_npos;
    }
    if (needle_count == StringClass::k_npos)
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
        start_pos = haystack.GetSize();
    }
    if (needle_count > start_pos + 1)
    {
        return StringClass::k_npos;
    }
    for (typename StringClass::size_type haystack_pos = start_pos - needle_count; haystack_pos != StringClass::k_npos; --haystack_pos)
    {
        bool is_found = true;
        for (typename StringClass::size_type needle_pos = 0; needle_pos < needle_count; ++needle_pos)
        {
            if (needle[needle_pos] != haystack.At(haystack_pos + needle_pos))
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
    return StringClass::k_npos;
}

template <Opal::StringLike StringClass>
typename StringClass::size_type Opal::ReverseFind(const StringClass& haystack, const typename StringClass::value_type& ch,
                                                  typename StringClass::size_type start_pos)
{
    if (haystack.IsEmpty())
    {
        return StringClass::k_npos;
    }
    if (start_pos >= haystack.GetSize())
    {
        start_pos = haystack.GetSize() - 1;
    }
    for (typename StringClass::size_type haystack_pos = start_pos; haystack_pos != StringClass::k_npos; --haystack_pos)
    {
        if (haystack.At(haystack_pos) == ch)
        {
            return haystack_pos;
        }
    }
    return StringClass::k_npos;
}

template <Opal::StringLike StringClass, typename Allocator>
Opal::Expected<StringClass, Opal::ErrorCode> Opal::GetSubString(const StringClass& str, typename StringClass::size_type start_pos,
                                                                typename StringClass::size_type count, Allocator* allocator)
{
    using ReturnType = Expected<StringClass, ErrorCode>;
    static_cast<void>(allocator);
    if (start_pos > str.GetSize())
    {
        return ReturnType(ErrorCode::OutOfBounds);
    }
    count = StringClass::Min(count, str.GetSize() - start_pos);
    if constexpr (k_is_string_view_value<StringClass>)
    {
        return ReturnType(StringClass(str.GetData() + start_pos, count));
    }
    else
    {
        return ReturnType(StringClass(str.GetData() + start_pos, count, allocator));
    }
}

template <typename CodeUnitType>
Opal::u64 Opal::GetStringLength(const CodeUnitType* str)
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

template <Opal::StringLike StringClass>
bool Opal::StartsWith(const StringClass& str, const StringClass& prefix)
{
    if (prefix.GetSize() > str.GetSize())
    {
        return false;
    }
    for (typename StringClass::size_type i = 0; i < prefix.GetSize(); ++i)
    {
        if (prefix.At(i) != str.At(i))
        {
            return false;
        }
    }
    return true;
}

template <Opal::StringLike StringClass>
bool Opal::EndsWith(const StringClass& str, const StringClass& suffix)
{
    if (suffix.GetSize() > str.GetSize())
    {
        return false;
    }
    for (typename StringClass::size_type i = 0; i < suffix.GetSize(); ++i)
    {
        if (suffix.At(i) != str.At(str.GetSize() - suffix.GetSize() + i))
        {
            return false;
        }
    }
    return true;
}

template <Opal::StringLike StringClass>
bool Opal::Split(const StringClass& str, const StringClass& delimiter, StringClass& first, StringClass& second)
{
    typename StringClass::size_type pos = delimiter.IsEmpty() ? StringClass::k_npos : Opal::Find(str, delimiter);
    if (pos == StringClass::k_npos)
    {
        if constexpr (k_is_string_view_value<StringClass>)
        {
            first = str;
        }
        else
        {
            first = str.Clone();
        }
        return false;
    }
    auto first_it = Opal::GetSubString(str, 0, pos);
    if (!first_it.HasValue())
    {
        return false;
    }
    first = std::move(first_it.GetValue());
    auto second_it = Opal::GetSubString(str, pos + delimiter.GetSize(), StringClass::k_npos);
    if (!second_it.HasValue())
    {
        return false;
    }
    second = std::move(second_it.GetValue());
    return true;
}

template <Opal::StringLike StringClass>
bool Opal::SplitToArray(const StringClass& str, const StringClass& delimiter, DynamicArray<StringClass>& result)
{
    typename StringClass::size_type start_pos = 0;
    while (true)
    {
        typename StringClass::size_type pos = delimiter.IsEmpty() ? StringClass::k_npos : Find(str, delimiter, start_pos);
        if (pos == StringClass::k_npos)
        {
            auto it = GetSubString(str, start_pos, StringClass::k_npos);
            if (!it.HasValue())
            {
                return false;
            }
            result.PushBack(std::move(it.GetValue()));
            return start_pos != 0;
        }
        auto it = GetSubString(str, start_pos, pos - start_pos);
        if (!it.HasValue())
        {
            return false;
        }
        result.PushBack(std::move(it.GetValue()));
        start_pos = pos + delimiter.GetSize();
    }
}

namespace Opal::Impl
{

template <typename T>
void ToBinary(Opal::StringUtf8& out_str, T number, bool leading_zeros)
{
    constexpr Opal::i32 k_max_digit_count = sizeof(T) * 8;
    size_t count = 0;
    while (number != 0 && count < k_max_digit_count)
    {
        char digit = number & 0x01 ? '1' : '0';
        out_str.At(count++) = digit;
        number >>= 1;
    }
    while (count < k_max_digit_count && leading_zeros)
    {
        out_str.At(count++) = '0';
    }
    if (count == 0)
    {
        out_str.At(0) = '0';
    }
    out_str.Trim();
    out_str.Reverse();
}

template <typename T>
Opal::StringUtf8 GetFormat(Opal::NumberSystemBase number_system_base, bool add_leading_zeros)
{
    constexpr size_t k_type_size = sizeof(T);
    StringUtf8 out_str("%");
    if (add_leading_zeros && number_system_base != NumberSystemBase::Decimal)
    {
        const i32 width = number_system_base == NumberSystemBase::Hexadecimal ? static_cast<i32>(k_type_size * 2)
                                                                             : static_cast<i32>((k_type_size * 8 + 2) / 3);
        out_str += "0";
        out_str += NumberToString(width);
    }
    if constexpr (k_type_size == 8)
    {
        switch (number_system_base)
        {
            case NumberSystemBase::Octal:
                out_str += PRIo64;
                break;
            case NumberSystemBase::Hexadecimal:
                out_str += PRIX64;
                break;
            case NumberSystemBase::Decimal:
            {
                if constexpr (Opal::SignedIntegral<T>)
                {
                    out_str += PRId64;
                    break;
                }
                else
                {
                    out_str += PRIu64;
                    break;
                }
            }
            default:
                // NumberSystemBase::Binary is the only other value, and NumberToString handles it without a printf format.
                OPAL_ASSERT(false, "No printf format for this number system base");
                break;
        }
    }
    else if constexpr (k_type_size == 4)
    {
        switch (number_system_base)
        {
            case NumberSystemBase::Octal:
                out_str += PRIo32;
                break;
            case NumberSystemBase::Hexadecimal:
                out_str += PRIX32;
                break;
            case NumberSystemBase::Decimal:
            {
                if constexpr (Opal::SignedIntegral<T>)
                {
                    out_str += PRId32;
                    break;
                }
                else
                {
                    out_str += PRIu32;
                    break;
                }
            }
            default:
                // NumberSystemBase::Binary is the only other value, and NumberToString handles it without a printf format.
                OPAL_ASSERT(false, "No printf format for this number system base");
                break;
        }
    }
    else if constexpr (k_type_size == 2)
    {
        switch (number_system_base)
        {
            case NumberSystemBase::Octal:
                out_str += PRIo16;
                break;
            case NumberSystemBase::Hexadecimal:
                out_str += PRIX16;
                break;
            case NumberSystemBase::Decimal:
            {
                if constexpr (Opal::SignedIntegral<T>)
                {
                    out_str += PRId16;
                    break;
                }
                else
                {
                    out_str += PRIu16;
                    break;
                }
            }
            default:
                // NumberSystemBase::Binary is the only other value, and NumberToString handles it without a printf format.
                OPAL_ASSERT(false, "No printf format for this number system base");
                break;
        }
    }
    else if constexpr (k_type_size == 1)
    {
        switch (number_system_base)
        {
            case NumberSystemBase::Octal:
                out_str += PRIo8;
                break;
            case NumberSystemBase::Hexadecimal:
                out_str += PRIX8;
                break;
            case NumberSystemBase::Decimal:
            {
                if constexpr (Opal::SignedIntegral<T>)
                {
                    out_str += PRId8;
                    break;
                }
                else
                {
                    out_str += PRIu8;
                    break;
                }
            }
            default:
                // NumberSystemBase::Binary is the only other value, and NumberToString handles it without a printf format.
                OPAL_ASSERT(false, "No printf format for this number system base");
                break;
        }
    }
    return out_str;
}
}  // namespace Opal::Impl

template <Opal::Integral T>
Opal::StringUtf8 Opal::NumberToString(T value, NumberSystemBase base, bool add_leading_zeros)
{
    StringUtf8 str(256, 0);
    if (base == NumberSystemBase::Binary)
    {
        Impl::ToBinary(str, value, add_leading_zeros);
    }
    else
    {
        StringUtf8 format = Impl::GetFormat<T>(base, add_leading_zeros);
#if defined(OPAL_COMPILER_MSVC)
        sprintf_s(*str, str.GetSize(), *format, value);
#else
        OPAL_START_DISABLE_WARNINGS
        OPAL_DISABLE_WARNING("-Wformat-nonliteral")
        sprintf(*str, *format, value);
        OPAL_END_DISABLE_WARNINGS
#endif
    }
    str.Trim();
    return str;
}

template <Opal::FloatingPoint T>
Opal::StringUtf8 Opal::NumberToString(T value, i32 decimal_points)
{
    StringUtf8 str(256, 0);
    StringUtf8 format("%");
    format += decimal_points == -1 ? "" : "." + NumberToString(decimal_points);
    if constexpr (sizeof(T) == 8)
    {
        format += "lf";
    }
    else
    {
        format += "f";
    }
#if defined(OPAL_COMPILER_MSVC)
    sprintf_s(*str, str.GetSize(), *format, value);
#else
    OPAL_START_DISABLE_WARNINGS
    OPAL_DISABLE_WARNING("-Wformat-nonliteral")
    OPAL_DISABLE_WARNING("-Wdouble-promotion")
    sprintf(*str, *format, value);
    OPAL_END_DISABLE_WARNINGS
#endif
    str.Trim();
    return str;
}

template <Opal::Integral T, Opal::StringLike StringClass>
T Opal::StringToNumber(const StringClass& str, i32 base)
{
    static_assert(k_is_same_value<typename StringClass::value_type, char8>, "StringToNumber only supports 8-bit code units");
    constexpr u64 k_stack_buffer_size = 64;
    const u64 size = str.GetSize();
    char8 stack_buffer[k_stack_buffer_size];
    StringUtf8 heap_buffer;
    const char8* terminated = stack_buffer;
    if (size < k_stack_buffer_size)
    {
        if (size > 0)
        {
            std::memcpy(stack_buffer, *str, size);
        }
        stack_buffer[size] = 0;
    }
    else
    {
        heap_buffer = StringUtf8(*str, size);
        terminated = *heap_buffer;
    }
    char* end = nullptr;
    if constexpr (SignedIntegral<T>)
    {
        return Narrow<T>(strtoll(terminated, &end, base));
    }
    else
    {
        return Narrow<T>(strtoull(terminated, &end, base));
    }
}

template <Opal::Integral T>
T Opal::StringToNumber(const char8* str, i32 base)
{
    char* end = nullptr;
    if constexpr (SignedIntegral<T>)
    {
        return Narrow<T>(strtoll(str, &end, base));
    }
    else
    {
        return Narrow<T>(strtoull(str, &end, base));
    }
}

#pragma once

#include "opal/container/array-view.h"
#include "opal/container/expected.h"
#include "opal/container/string-encoding.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/exceptions.h"
#include "opal/types.h"

namespace Opal
{

/**
 * @brief Lightweight, non-owning, read-only view into a string's code units.
 * @tparam CodeUnitType Type of the code unit used in the string.
 * @tparam EncodingType Type of the encoding used in the string.
 */
template <typename CodeUnitType, typename EncodingType>
class StringView
{
public:
    using value_type = CodeUnitType;
    using size_type = u64;
    using difference_type = i64;
    using reference = const CodeUnitType&;
    using const_reference = const CodeUnitType&;
    using pointer = const CodeUnitType*;
    using const_pointer = const CodeUnitType*;
    using encoding_type = EncodingType;
    using const_iterator = StringConstIterator<StringView>;

    static constexpr size_type k_npos = static_cast<size_type>(-1);

    /** @brief Default constructor. Creates an empty view. */
    StringView() = default;

    /**
     * @brief Construct a view from a pointer and size.
     * @param str Pointer to the first code unit.
     * @param count Number of code units.
     */
    StringView(const CodeUnitType* str, size_type count) : m_data(str), m_size(count) {}

    /**
     * @brief Construct a view from a null-terminated string.
     * @param str Pointer to the null-terminated string.
     */
    StringView(const CodeUnitType* str) : m_data(str), m_size(str ? GetStringLength(str) : 0) {}

    /**
     * @brief Implicit conversion from String.
     * @param str String to create a view of.
     */
    StringView(const String<CodeUnitType, EncodingType>& str) : m_data(str.GetData()), m_size(str.GetSize()) {}

    /**
     * @brief Construct from an ArrayView of const code units.
     * @param view ArrayView to create a string view from.
     */
    StringView(const ArrayView<const CodeUnitType>& view) : m_data(view.GetData()), m_size(view.GetSize()) {}

    StringView(const StringView&) = default;
    StringView& operator=(const StringView&) = default;

    ~StringView() = default;

    /** @brief Get the underlying data pointer. */
    [[nodiscard]] const CodeUnitType* GetData() const { return m_data; }

    /** @brief Dereference operator for consistency with String. */
    const CodeUnitType* operator*() const { return m_data; }

    /** @brief Get the number of code units. */
    [[nodiscard]] size_type GetSize() const { return m_size; }

    /** @brief Check if the view is empty. */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * @brief Get the code unit at a specific position. Bounds-checked.
     * @param pos Position in the view.
     * @return Reference to the code unit.
     * @throw OutOfBoundsException if pos is out of bounds.
     */
    const CodeUnitType& operator[](size_type pos) const
    {
        if (pos >= m_size)
        {
            throw OutOfBoundsException(pos, 0, m_size - 1);
        }
        return m_data[pos];
    }

    /**
     * @brief Get the code unit at a specific position. Unchecked.
     * @param pos Position in the view.
     * @return Reference to the code unit.
     */
    const CodeUnitType& At(size_type pos) const { return m_data[pos]; }

    /**
     * @brief Get the first code unit.
     * @return Reference to the first code unit, or ErrorCode::OutOfBounds if the view is empty.
     */
    [[nodiscard]] Expected<const CodeUnitType&, ErrorCode> Front() const
    {
        if (m_size == 0)
        {
            return Expected<const CodeUnitType&, ErrorCode>(ErrorCode::OutOfBounds);
        }
        return Expected<const CodeUnitType&, ErrorCode>(m_data[0]);
    }

    /**
     * @brief Get the last code unit.
     * @return Reference to the last code unit, or ErrorCode::OutOfBounds if the view is empty.
     */
    [[nodiscard]] Expected<const CodeUnitType&, ErrorCode> Back() const
    {
        if (m_size == 0)
        {
            return Expected<const CodeUnitType&, ErrorCode>(ErrorCode::OutOfBounds);
        }
        return Expected<const CodeUnitType&, ErrorCode>(m_data[m_size - 1]);
    }

    /**
     * @brief Advance the start of the view by n code units.
     * @param n Number of code units to remove from the front.
     */
    void RemovePrefix(size_type n)
    {
        m_data += n;
        m_size -= n;
    }

    /**
     * @brief Shrink the view by n code units from the end.
     * @param n Number of code units to remove from the back.
     */
    void RemoveSuffix(size_type n) { m_size -= n; }

    /**
     * @brief Get a sub-view.
     * @param pos Start position.
     * @param count Number of code units. Defaults to k_npos (rest of the view).
     * @return Sub-view, or ErrorCode::OutOfBounds if pos is out of range.
     */
    [[nodiscard]] Expected<StringView, ErrorCode> SubView(size_type pos, size_type count = k_npos) const
    {
        if (pos > m_size)
        {
            return Expected<StringView, ErrorCode>(ErrorCode::OutOfBounds);
        }
        size_type actual_count = count;
        if (count == k_npos || pos + count > m_size)
        {
            actual_count = m_size - pos;
        }
        return Expected<StringView, ErrorCode>(StringView(m_data + pos, actual_count));
    }

    /**
     * @brief Compare two views for equality.
     * @param other View to compare with.
     * @return True if the views contain identical code units.
     */
    bool operator==(const StringView& other) const
    {
        if (m_size != other.m_size)
        {
            return false;
        }
        if (m_data == other.m_data)
        {
            return true;
        }
        for (size_type i = 0; i < m_size; ++i)
        {
            if (m_data[i] != other.m_data[i])
            {
                return false;
            }
        }
        return true;
    }

    // Iterators

    const_iterator Begin() const { return const_iterator(m_data); }
    const_iterator End() const { return const_iterator(m_data + m_size); }
    const_iterator ConstBegin() const { return const_iterator(m_data); }
    const_iterator ConstEnd() const { return const_iterator(m_data + m_size); }

    const_iterator begin() const { return const_iterator(m_data); }
    const_iterator end() const { return const_iterator(m_data + m_size); }
    const_iterator cbegin() const { return const_iterator(m_data); }
    const_iterator cend() const { return const_iterator(m_data + m_size); }

private:
    const CodeUnitType* m_data = nullptr;
    size_type m_size = 0;
};

/*************************************************************************************************/
/** Most common StringView specializations. ******************************************************/
/*************************************************************************************************/

using StringViewUtf8 = StringView<char8, EncodingUtf8<char8>>;
using StringViewUtf32 = StringView<uchar32, EncodingUtf32LE<uchar32>>;
using StringViewLocale = StringView<char8, EncodingLocale>;
using StringViewWide = StringView<char16, EncodingUtf16LE<char16>>;

}  // namespace Opal
#pragma once

#include <format>

#include "opal/container/string-view.h"
#include "opal/container/string.h"

namespace Opal
{

namespace Impl
{

/**
 * Output iterator adapter that appends characters to a StringUtf8. Used internally by Format and AppendFormat to bridge
 * std::vformat_to with Opal's string type.
 */
struct StringFormatIterator
{
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    StringUtf8* m_output;

    explicit StringFormatIterator(StringUtf8& output) : m_output(&output) {}

    StringFormatIterator& operator=(char c)
    {
        // An output iterator has nowhere to put a code, so a failed append stays an exception here.
        if (m_output->Append(static_cast<char8>(c)) != ErrorCode::Success) [[unlikely]]
        {
            throw OutOfMemoryException(m_output->GetAllocator().GetName(), m_output->GetSize() + 1);
        }
        return *this;
    }

    StringFormatIterator& operator*() { return *this; }
    StringFormatIterator& operator++() { return *this; }
    StringFormatIterator operator++(int) { return *this; }
};

}  // namespace Impl

/**
 * Append formatted text to an existing string using std::format syntax.
 *
 * When called with no variadic arguments, the format string is appended directly without parsing. Otherwise, arguments
 * are formatted via std::vformat_to.
 *
 * @param output  String to append to.
 * @param fmt     Format string using std::format syntax (e.g., "{}", "{:.2f}", "{:#x}").
 * @param args    Values to format into the string.
 * @throw OutOfMemoryException when the output string cannot be grown.
 */
template <typename... Args>
void AppendFormat(StringUtf8& output, StringViewUtf8 fmt, Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        if (output.Append(fmt.GetData(), fmt.GetSize()) != ErrorCode::Success) [[unlikely]]
        {
            throw OutOfMemoryException(output.GetAllocator().GetName(), output.GetSize() + fmt.GetSize());
        }
    }
    else
    {
        Impl::StringFormatIterator out(output);
        std::vformat_to(std::move(out), std::string_view(fmt.GetData(), fmt.GetSize()), std::make_format_args(args...));
    }
}

/**
 * Create a new formatted string using std::format syntax.
 *
 * @param fmt     Format string using std::format syntax (e.g., "{}", "{:.2f}", "{:#x}").
 * @param args    Values to format into the string.
 * @return        A new StringUtf8 containing the formatted result.
 * @throw OutOfMemoryException when the result string cannot be grown.
 */
template <typename... Args>
StringUtf8 Format(StringViewUtf8 fmt, Args&&... args)
{
    StringUtf8 result;
    AppendFormat(result, fmt, std::forward<Args>(args)...);
    return result;
}

}  // namespace Opal

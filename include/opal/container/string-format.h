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
        m_output->Append(static_cast<char8>(c));
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
 */
template <typename... Args>
void AppendFormat(StringUtf8& output, StringViewUtf8 fmt, Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        output.Append(fmt.GetData(), fmt.GetSize());
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
 */
template <typename... Args>
StringUtf8 Format(StringViewUtf8 fmt, Args&&... args)
{
    StringUtf8 result;
    AppendFormat(result, fmt, std::forward<Args>(args)...);
    return result;
}

}  // namespace Opal

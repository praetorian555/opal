#pragma once

#include <format>

#include "opal/container/expected.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"

namespace Opal
{

namespace Impl
{

/**
 * Output iterator adapter that appends characters to a StringUtf8. Used internally by Format and AppendFormat to bridge
 * std::vformat_to with Opal's string type.
 *
 * An output iterator has no return value, so the first failed append is recorded in the referenced code and every
 * later write is dropped. The caller reads the code once formatting is done.
 */
struct StringFormatIterator
{
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    StringUtf8* m_output;
    ErrorCode* m_error;

    StringFormatIterator(StringUtf8& output, ErrorCode& error) : m_output(&output), m_error(&error) {}

    StringFormatIterator& operator=(char c)
    {
        if (*m_error != ErrorCode::Success) [[unlikely]]
        {
            return *this;
        }
        const ErrorCode error = m_output->Append(static_cast<char8>(c));
        if (error != ErrorCode::Success) [[unlikely]]
        {
            *m_error = error;
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
 * @return ErrorCode::Success, or ErrorCode::OutOfMemory when the output string could not be grown. On failure the
 *         output holds however much was written before the string ran out of room.
 */
template <typename... Args>
[[nodiscard]] ErrorCode AppendFormat(StringUtf8& output, StringViewUtf8 fmt, Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        return output.Append(fmt.GetData(), fmt.GetSize());
    }
    else
    {
        ErrorCode error = ErrorCode::Success;
        Impl::StringFormatIterator out(output, error);
        std::vformat_to(std::move(out), std::string_view(fmt.GetData(), fmt.GetSize()), std::make_format_args(args...));
        return error;
    }
}

/**
 * Create a new formatted string using std::format syntax.
 *
 * @param fmt     Format string using std::format syntax (e.g., "{}", "{:.2f}", "{:#x}").
 * @param args    Values to format into the string.
 * @return A new StringUtf8 containing the formatted result, or ErrorCode::OutOfMemory when it could not be grown.
 */
template <typename... Args>
Expected<StringUtf8, ErrorCode> Format(StringViewUtf8 fmt, Args&&... args)
{
    StringUtf8 result;
    const ErrorCode error = AppendFormat(result, fmt, std::forward<Args>(args)...);
    if (error != ErrorCode::Success) [[unlikely]]
    {
        return Expected<StringUtf8, ErrorCode>(error);
    }
    return Expected<StringUtf8, ErrorCode>(Move(result));
}

}  // namespace Opal

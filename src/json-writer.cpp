#include "opal/container/json-writer.h"

#include <math.h>
#include <string.h>

#include "opal/container/dynamic-array.h"
#include "opal/container/string-format.h"
#include "opal/sort/quick-sort.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Serializer.
// ------------------------------------------------------------------------------------------------

namespace
{

struct Frame
{
    const JsonValue* container;
    JsonValue::ObjectIterator obj_it;
    JsonValue::ObjectIterator obj_end;
    u64 size;
    u64 index;
    bool is_object;
    /** Where an object written in key order keeps its entries in JsonSerializer::m_sorted; k_unsorted otherwise. */
    u64 sorted_begin;
};

constexpr u64 k_unsorted = ~0ull;

struct SortedEntry
{
    StringViewUtf8 key;
    const JsonValue* value = nullptr;

    bool operator<(const SortedEntry& other) const
    {
        const u64 common = key.GetSize() < other.key.GetSize() ? key.GetSize() : other.key.GetSize();
        const int order = memcmp(key.GetData(), other.key.GetData(), common);
        return order != 0 ? order < 0 : key.GetSize() < other.key.GetSize();
    }
};

class JsonSerializer
{
public:
    JsonSerializer(StringUtf8& output, const JsonWriteOptions& options) : m_output(output), m_options(options) {}

    /**
     * Walk the value and write it out.
     * @return The first failure hit, or ErrorCode::Success. Once one is recorded every later write is skipped, so the
     *         walk stops rather than piling more work onto a string that is already short.
     */
    ErrorCode Serialize(const JsonValue& root)
    {
        WriteValue(root);
        while (!m_stack.IsEmpty() && m_error == ErrorCode::Success)
        {
            Frame& frame = m_stack.Back();
            if (frame.index >= frame.size)
            {
                const bool is_object = frame.is_object;
                if (frame.sorted_begin != k_unsorted)
                {
                    (void)m_sorted.Resize(frame.sorted_begin);
                }
                m_stack.PopBack();
                CloseContainer(is_object);
                continue;
            }
            if (frame.index > 0)
            {
                WriteSeparator();
            }

            // Advance the frame before WriteValue, since WriteValue may push a new frame onto the
            // stack causing a reallocation that invalidates the frame reference.
            if (frame.is_object)
            {
                StringViewUtf8 key;
                const JsonValue* val = nullptr;
                if (frame.sorted_begin != k_unsorted)
                {
                    const SortedEntry& entry = m_sorted[frame.sorted_begin + frame.index];
                    key = entry.key;
                    val = entry.value;
                }
                else
                {
                    auto [entry_key, entry_value] = *frame.obj_it;
                    key = entry_key;
                    val = &entry_value.Get();
                    ++frame.obj_it;
                }
                ++frame.index;
                WriteString(key);
                Append(':');
                if (m_options->pretty)
                {
                    Append(' ');
                }
                WriteValue(*val);
            }
            else
            {
                const JsonValue& child = (*frame.container)[frame.index];
                ++frame.index;
                WriteValue(child);
            }
        }
        return m_error;
    }

private:
    // Every write goes through one of these three, so recording the first failure here is enough to stop the walk.
    void Append(char8 ch)
    {
        if (m_error != ErrorCode::Success) [[unlikely]]
        {
            return;
        }
        m_error = m_output->Append(ch);
    }

    void Append(const char8* str, u64 size)
    {
        if (m_error != ErrorCode::Success) [[unlikely]]
        {
            return;
        }
        m_error = m_output->Append(str, size);
    }

    template <typename... Args>
    void AppendFormatted(StringViewUtf8 fmt, Args&&... args)
    {
        if (m_error != ErrorCode::Success) [[unlikely]]
        {
            return;
        }
        m_error = AppendFormat(*m_output, fmt, std::forward<Args>(args)...);
    }

    void WriteValue(const JsonValue& value)
    {
        switch (value.GetType())
        {
            case JsonType::Null:
                WriteNull();
                break;
            case JsonType::Bool:
                WriteBool(value.GetBool());
                break;
            case JsonType::Number:
                if (value.IsIntegerNumber())
                {
                    WriteInteger(value.GetIntegerNumber());
                }
                else
                {
                    WriteNumber(value.GetNumber());
                }
                break;
            case JsonType::String:
                WriteString(value.GetString());
                break;
            case JsonType::Array:
                OpenArray(value);
                break;
            case JsonType::Object:
                OpenObject(value);
                break;
        }
    }

    void WriteNull() { Append("null", 4); }

    void WriteBool(bool value)
    {
        if (value)
        {
            Append("true", 4);
        }
        else
        {
            Append("false", 5);
        }
    }

    void WriteNumber(f64 value)
    {
        // JSON has no spelling for either, and a JsonValue can hold one because a f64 can.
        if (isnan(value) || isinf(value))
        {
            m_error = ErrorCode::InvalidArgument;
            return;
        }
        AppendFormatted("{:.17g}", value);
    }

    void WriteInteger(i64 value) { AppendFormatted("{}", value); }

    void WriteString(StringViewUtf8 value)
    {
        Append('"');
        WriteEscapedString(value);
        Append('"');
    }

    void WriteEscapedString(StringViewUtf8 value)
    {
        const char8* data = value.GetData();
        const u64 size = value.GetSize();
        u64 flush_start = 0;

        for (u64 i = 0; i < size; ++i)
        {
            const auto c = static_cast<u8>(data[i]);
            const char* escape = nullptr;
            u64 escape_len = 0;

            switch (c)
            {
                case '"':
                    escape = "\\\"";
                    escape_len = 2;
                    break;
                case '\\':
                    escape = "\\\\";
                    escape_len = 2;
                    break;
                case '\b':
                    escape = "\\b";
                    escape_len = 2;
                    break;
                case '\f':
                    escape = "\\f";
                    escape_len = 2;
                    break;
                case '\n':
                    escape = "\\n";
                    escape_len = 2;
                    break;
                case '\r':
                    escape = "\\r";
                    escape_len = 2;
                    break;
                case '\t':
                    escape = "\\t";
                    escape_len = 2;
                    break;
                default:
                    if (c < 0x20)
                    {
                        // Flush safe run before this character.
                        if (i > flush_start)
                        {
                            Append(data + flush_start, i - flush_start);
                        }
                        AppendFormatted("\\u{:04x}", c);
                        flush_start = i + 1;
                    }
                    continue;
            }

            // Flush safe run before escape.
            if (i > flush_start)
            {
                Append(data + flush_start, i - flush_start);
            }
            Append(escape, escape_len);
            flush_start = i + 1;
        }

        // Flush remaining safe bytes.
        if (flush_start < size)
        {
            Append(data + flush_start, size - flush_start);
        }
    }

    void OpenArray(const JsonValue& value)
    {
        const u64 size = value.GetSize();
        if (size == 0)
        {
            Append("[]", 2);
            return;
        }

        Append('[');
        if (m_options->pretty)
        {
            ++m_depth;
            WriteNewline();
            WriteIndent();
        }

        using ObjIt = JsonValue::ObjectIterator;
        using Inner = ObjIt::InnerIterator;
        if (!(m_stack.EmplaceBack(&value, ObjIt(Inner{}), ObjIt(Inner{}), size, 0ull, false, k_unsorted)).HasValue()) [[unlikely]]
        {
            m_error = ErrorCode::OutOfMemory;
        }
    }

    void OpenObject(const JsonValue& value)
    {
        const u64 size = value.GetSize();
        if (size == 0)
        {
            Append("{}", 2);
            return;
        }

        Append('{');
        if (m_options->pretty)
        {
            ++m_depth;
            WriteNewline();
            WriteIndent();
        }

        const auto range = value.Items();
        u64 sorted_begin = k_unsorted;
        if (m_options->sort_keys)
        {
            sorted_begin = m_sorted.GetSize();
            for (auto [key, val] : range)
            {
                if (m_sorted.PushBack(SortedEntry{key, &val.Get()}) != ErrorCode::Success) [[unlikely]]
                {
                    m_error = ErrorCode::OutOfMemory;
                    return;
                }
            }
            ArrayView<SortedEntry> entries(m_sorted.GetData() + sorted_begin, size);
            QuickSortImproved(entries);
        }
        if (!(m_stack.EmplaceBack(&value, range.begin(), range.end(), size, 0ull, true, sorted_begin)).HasValue()) [[unlikely]]
        {
            m_error = ErrorCode::OutOfMemory;
        }
    }

    void CloseContainer(bool is_object)
    {
        if (m_options->pretty)
        {
            --m_depth;
            WriteNewline();
            WriteIndent();
        }
        Append(is_object ? '}' : ']');
    }

    void WriteSeparator()
    {
        Append(',');
        if (m_options->pretty)
        {
            WriteNewline();
            WriteIndent();
        }
    }

    void WriteNewline() { Append('\n'); }

    void WriteIndent()
    {
        if (m_options->use_tabs)
        {
            for (u32 i = 0; i < m_depth; ++i)
            {
                Append('\t');
            }
        }
        else
        {
            const u32 count = m_depth * m_options->indent_width;
            for (u32 i = 0; i < count; ++i)
            {
                Append(' ');
            }
        }
    }

    Ref<StringUtf8> m_output;
    Ref<const JsonWriteOptions> m_options;
    DynamicArray<Frame> m_stack;
    /** The entries of every open object written in key order, each object's sorted, innermost last. */
    DynamicArray<SortedEntry> m_sorted;
    u32 m_depth = 0;
    ErrorCode m_error = ErrorCode::Success;
};

}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonWriter.
// ------------------------------------------------------------------------------------------------

Expected<StringUtf8, ErrorCode> JsonWriter::Serialize(const JsonValue& value, AllocatorBase* allocator)
{
    return Serialize(value, JsonWriteOptions{}, allocator);
}

Expected<StringUtf8, ErrorCode> JsonWriter::Serialize(const JsonValue& value, const JsonWriteOptions& options, AllocatorBase* allocator)
{
    using Result = Expected<StringUtf8, ErrorCode>;
    StringUtf8 output(allocator);
    JsonSerializer serializer(output, options);
    const ErrorCode error = serializer.Serialize(value);
    if (error != ErrorCode::Success)
    {
        return Result(error);
    }
    return Result(Move(output));
}

}  // namespace Opal

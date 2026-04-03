#include "opal/container/json-writer.h"

#include <cmath>

#include "opal/container/dynamic-array.h"
#include "opal/container/string-format.h"

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
};

class JsonSerializer
{
public:
    JsonSerializer(StringUtf8& output, const JsonWriteOptions& options) : m_output(output), m_options(options) {}

    void Serialize(const JsonValue& root)
    {
        WriteValue(root);
        while (!m_stack.IsEmpty())
        {
            Frame& frame = m_stack.Back();
            if (frame.index >= frame.size)
            {
                const bool is_object = frame.is_object;
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
                auto [key, val] = *frame.obj_it;
                ++frame.obj_it;
                ++frame.index;
                WriteString(key);
                m_output->Append(':');
                if (m_options->pretty)
                {
                    m_output->Append(' ');
                }
                WriteValue(val);
            }
            else
            {
                const JsonValue& child = (*frame.container)[frame.index];
                ++frame.index;
                WriteValue(child);
            }
        }
    }

private:
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

    void WriteNull() const { m_output->Append("null", 4); }

    void WriteBool(bool value) const
    {
        if (value)
        {
            m_output->Append("true", 4);
        }
        else
        {
            m_output->Append("false", 5);
        }
    }

    void WriteNumber(f64 value) const
    {
        if (std::isnan(value))
        {
            throw JsonSerializeException("NaN is not a valid JSON number");
        }
        if (std::isinf(value))
        {
            throw JsonSerializeException("Infinity is not a valid JSON number");
        }
        AppendFormat(m_output, "{:.17g}", value);
    }

    void WriteInteger(i64 value) const { AppendFormat(m_output, "{}", value); }

    void WriteString(StringViewUtf8 value) const
    {
        m_output->Append('"');
        WriteEscapedString(value);
        m_output->Append('"');
    }

    void WriteEscapedString(StringViewUtf8 value) const
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
                            m_output->Append(data + flush_start, i - flush_start);
                        }
                        AppendFormat(m_output, "\\u{:04x}", c);
                        flush_start = i + 1;
                    }
                    continue;
            }

            // Flush safe run before escape.
            if (i > flush_start)
            {
                m_output->Append(data + flush_start, i - flush_start);
            }
            m_output->Append(escape, escape_len);
            flush_start = i + 1;
        }

        // Flush remaining safe bytes.
        if (flush_start < size)
        {
            m_output->Append(data + flush_start, size - flush_start);
        }
    }

    void OpenArray(const JsonValue& value)
    {
        const u64 size = value.GetSize();
        if (size == 0)
        {
            m_output->Append("[]", 2);
            return;
        }

        m_output->Append('[');
        if (m_options->pretty)
        {
            ++m_depth;
            WriteNewline();
            WriteIndent();
        }

        using ObjIt = JsonValue::ObjectIterator;
        using Inner = ObjIt::InnerIterator;
        m_stack.EmplaceBack(&value, ObjIt(Inner{}), ObjIt(Inner{}), size, 0, false);
    }

    void OpenObject(const JsonValue& value)
    {
        const u64 size = value.GetSize();
        if (size == 0)
        {
            m_output->Append("{}", 2);
            return;
        }

        m_output->Append('{');
        if (m_options->pretty)
        {
            ++m_depth;
            WriteNewline();
            WriteIndent();
        }

        const auto range = value.Items();
        m_stack.EmplaceBack(&value, range.begin(), range.end(), size, 0, true);
    }

    void CloseContainer(bool is_object)
    {
        if (m_options->pretty)
        {
            --m_depth;
            WriteNewline();
            WriteIndent();
        }
        m_output->Append(is_object ? '}' : ']');
    }

    void WriteSeparator() const
    {
        m_output->Append(',');
        if (m_options->pretty)
        {
            WriteNewline();
            WriteIndent();
        }
    }

    void WriteNewline() const { m_output->Append('\n'); }

    void WriteIndent() const
    {
        if (m_options->use_tabs)
        {
            for (u32 i = 0; i < m_depth; ++i)
            {
                m_output->Append('\t');
            }
        }
        else
        {
            const u32 count = m_depth * m_options->indent_width;
            for (u32 i = 0; i < count; ++i)
            {
                m_output->Append(' ');
            }
        }
    }

    Ref<StringUtf8> m_output;
    Ref<const JsonWriteOptions> m_options;
    DynamicArray<Frame> m_stack;
    u32 m_depth = 0;
};

}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonWriter.
// ------------------------------------------------------------------------------------------------

StringUtf8 JsonWriter::Serialize(const JsonValue& value, AllocatorBase* allocator)
{
    return Serialize(value, JsonWriteOptions{}, allocator);
}

StringUtf8 JsonWriter::Serialize(const JsonValue& value, const JsonWriteOptions& options, AllocatorBase* allocator)
{
    StringUtf8 output(allocator);
    JsonSerializer serializer(output, options);
    serializer.Serialize(value);
    return output;
}

}  // namespace Opal

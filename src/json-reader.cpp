#include "opal/container/json-reader.h"

#include "opal/container/in-place-array.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Helper.
// ------------------------------------------------------------------------------------------------

namespace
{
const char* JsonTypeToString(JsonType type)
{
    switch (type)
    {
        case JsonType::Null:
            return "null";
        case JsonType::Bool:
            return "bool";
        case JsonType::Number:
            return "number";
        case JsonType::String:
            return "string";
        case JsonType::Array:
            return "array";
        case JsonType::Object:
            return "object";
    }
    return "unknown";
}

void ThrowTypeMismatch(const char* expected, JsonType actual)
{
    throw JsonTypeMismatchException(expected, JsonTypeToString(actual));
}
}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonValue constructors.
// ------------------------------------------------------------------------------------------------

JsonValue::JsonValue() : m_data(JsonNull{}) {}
JsonValue::JsonValue(bool value) : m_data(value) {}
JsonValue::JsonValue(f64 value) : m_data(value) {}
JsonValue::JsonValue(StringViewUtf8 value) : m_data(value) {}
JsonValue::JsonValue(JsonArray value) : m_data(std::move(value)) {}
JsonValue::JsonValue(JsonObject value) : m_data(std::move(value)) {}

// ------------------------------------------------------------------------------------------------
// JsonValue clone.
// ------------------------------------------------------------------------------------------------

JsonValue JsonValue::Clone(AllocatorBase* allocator) const
{
    JsonValue result;
    result.m_data = m_data.Clone(allocator);
    return result;
}

// ------------------------------------------------------------------------------------------------
// JsonValue type queries.
// ------------------------------------------------------------------------------------------------

JsonType JsonValue::GetType() const
{
    return static_cast<JsonType>(m_data.GetIndex());
}

bool JsonValue::IsNull() const
{
    return m_data.IsActive<JsonNull>();
}
bool JsonValue::IsBool() const
{
    return m_data.IsActive<bool>();
}
bool JsonValue::IsNumber() const
{
    return m_data.IsActive<f64>();
}
bool JsonValue::IsString() const
{
    return m_data.IsActive<StringViewUtf8>();
}
bool JsonValue::IsArray() const
{
    return m_data.IsActive<JsonArray>();
}
bool JsonValue::IsObject() const
{
    return m_data.IsActive<JsonObject>();
}

// ------------------------------------------------------------------------------------------------
// JsonValue value extraction.
// ------------------------------------------------------------------------------------------------

bool JsonValue::GetBool() const
{
    if (!IsBool())
    {
        ThrowTypeMismatch("bool", GetType());
    }
    return m_data.Get<bool>();
}

f64 JsonValue::GetNumber() const
{
    if (!IsNumber())
    {
        ThrowTypeMismatch("number", GetType());
    }
    return m_data.Get<f64>();
}

StringViewUtf8 JsonValue::GetString() const
{
    if (!IsString())
    {
        ThrowTypeMismatch("string", GetType());
    }
    return m_data.Get<StringViewUtf8>();
}

// ------------------------------------------------------------------------------------------------
// JsonValue element access.
// ------------------------------------------------------------------------------------------------

const JsonValue& JsonValue::operator[](u64 index) const
{
    if (!IsArray())
    {
        ThrowTypeMismatch("array", GetType());
    }
    const auto& arr = m_data.Get<JsonArray>();
    if (index >= arr->GetSize())
    {
        throw OutOfBoundsException(index, 0, arr->GetSize() - 1);
    }
    return arr->operator[](index);
}

const JsonValue& JsonValue::operator[](StringViewUtf8 key) const
{
    if (!IsObject())
    {
        ThrowTypeMismatch("object", GetType());
    }
    const auto& obj = m_data.Get<JsonObject>();
    const auto it = obj->Find(key);
    if (it == obj->cend())
    {
        throw InvalidArgumentException("JsonValue::operator[]", "Key not found");
    }
    return it.GetValue();
}

const JsonValue& JsonValue::GetPath(StringViewUtf8 path) const
{
    const JsonValue* current = this;
    const char8* data = path.GetData();
    const u64 size = path.GetSize();
    u64 start = 0;

    for (u64 i = 0; i <= size; ++i)
    {
        if (i == size || data[i] == '.')
        {
            const StringViewUtf8 segment(data + start, i - start);
            if (segment.GetSize() == 0)
            {
                start = i + 1;
                continue;
            }

            // Try to parse as an integer index.
            bool is_index = true;
            u64 index = 0;
            for (u64 j = 0; j < segment.GetSize(); ++j)
            {
                const char8 c = segment.GetData()[j];
                if (c < '0' || c > '9')
                {
                    is_index = false;
                    break;
                }
                index = index * 10 + static_cast<u64>(c - '0');
            }

            if (is_index && current->IsArray())
            {
                current = &(*current)[index];
            }
            else
            {
                current = &(*current)[segment];
            }

            start = i + 1;
        }
    }

    return *current;
}

// ------------------------------------------------------------------------------------------------
// JsonValue container queries.
// ------------------------------------------------------------------------------------------------

u64 JsonValue::GetSize() const
{
    if (IsArray())
    {
        return m_data.Get<JsonArray>()->GetSize();
    }
    if (IsObject())
    {
        return m_data.Get<JsonObject>()->GetSize();
    }
    ThrowTypeMismatch("array or object", GetType());
#if defined(OPAL_DEBUG)
    return 0;
#endif
}

// ------------------------------------------------------------------------------------------------
// JsonValue::ArrayIterator.
// ------------------------------------------------------------------------------------------------

JsonValue::ArrayIterator::ArrayIterator(const JsonValue* data, u64 index) : m_data(data), m_index(index) {}

const JsonValue& JsonValue::ArrayIterator::operator*() const
{
    return m_data[m_index];
}

JsonValue::ArrayIterator& JsonValue::ArrayIterator::operator++()
{
    ++m_index;
    return *this;
}

bool JsonValue::ArrayIterator::operator==(const ArrayIterator& other) const
{
    return m_data == other.m_data && m_index == other.m_index;
}

bool JsonValue::ArrayIterator::operator!=(const ArrayIterator& other) const
{
    return !(*this == other);
}

JsonValue::ArrayIterator JsonValue::begin() const
{
    if (!IsArray())
    {
        ThrowTypeMismatch("array", GetType());
    }
    const auto& arr = m_data.Get<JsonArray>();
    return {arr->GetData(), 0};
}

JsonValue::ArrayIterator JsonValue::end() const
{
    if (!IsArray())
    {
        ThrowTypeMismatch("array", GetType());
    }
    const auto& arr = m_data.Get<JsonArray>();
    return {arr->GetData(), arr->GetSize()};
}

// ------------------------------------------------------------------------------------------------
// JsonValue::ObjectIterator.
// ------------------------------------------------------------------------------------------------

JsonValue::ObjectIterator::ObjectIterator(InnerIterator it) : m_it(it) {}

JsonValue::ObjectEntry JsonValue::ObjectIterator::operator*() const
{
    const auto& pair = *m_it;
    return ObjectEntry{.key = pair.key, .value = pair.value};
}

JsonValue::ObjectIterator& JsonValue::ObjectIterator::operator++()
{
    ++m_it;
    return *this;
}

bool JsonValue::ObjectIterator::operator==(const ObjectIterator& other) const
{
    return m_it == other.m_it;
}

bool JsonValue::ObjectIterator::operator!=(const ObjectIterator& other) const
{
    return !(*this == other);
}

// ------------------------------------------------------------------------------------------------
// JsonValue::ObjectRange.
// ------------------------------------------------------------------------------------------------

JsonValue::ObjectRange::ObjectRange(ObjectIterator begin_it, ObjectIterator end_it) : m_begin(begin_it), m_end(end_it) {}

JsonValue::ObjectIterator JsonValue::ObjectRange::begin() const
{
    return m_begin;
}
JsonValue::ObjectIterator JsonValue::ObjectRange::end() const
{
    return m_end;
}

JsonValue::ObjectRange JsonValue::Items() const
{
    if (!IsObject())
    {
        ThrowTypeMismatch("object", GetType());
    }
    const auto& obj = m_data.Get<JsonObject>();
    return {ObjectIterator(obj->begin()), ObjectIterator(obj->end())};
}

// ------------------------------------------------------------------------------------------------
// Parser.
// ------------------------------------------------------------------------------------------------

namespace
{

class JsonParser
{
public:
    JsonParser(StringViewUtf8 input, AllocatorBase* allocator, DynamicArray<StringUtf8>& escaped_strings)
        : m_input(input.GetData()), m_size(input.GetSize()), m_allocator(allocator), m_escaped_strings(escaped_strings)
    {
    }

    JsonValue Parse()
    {
        SkipWhitespace();
        JsonValue result = ParseValue();
        SkipWhitespace();
        if (m_pos < m_size)
        {
            ThrowError("Unexpected trailing content");
        }
        return result;
    }

private:
    JsonValue ParseValue()
    {
        if (m_pos >= m_size)
        {
            ThrowError("Unexpected end of input");
        }

        const char8 c = m_input[m_pos];
        switch (c)
        {
            case '"':
                return JsonValue(ParseString());
            case '{':
                return ParseObject();
            case '[':
                return ParseArray();
            case 't':
            case 'f':
                return JsonValue(ParseBool());
            case 'n':
                return ParseNull();
            default:
                if (c == '-' || (c >= '0' && c <= '9'))
                {
                    return JsonValue(ParseNumber());
                }
                ThrowError("Unexpected character");
        }
    }

    // -- Null --

    JsonValue ParseNull()
    {
        Expect('n');
        Expect('u');
        Expect('l');
        Expect('l');
        return {};
    }

    // -- Bool --

    bool ParseBool()
    {
        if (m_input[m_pos] == 't')
        {
            Expect('t');
            Expect('r');
            Expect('u');
            Expect('e');
            return true;
        }
        Expect('f');
        Expect('a');
        Expect('l');
        Expect('s');
        Expect('e');
        return false;
    }

    // -- Number --

    f64 ParseNumber()
    {
        const u64 start = m_pos;

        if (m_pos < m_size && m_input[m_pos] == '-')
        {
            ++m_pos;
        }

        if (m_pos >= m_size || m_input[m_pos] < '0' || m_input[m_pos] > '9')
        {
            ThrowError("Invalid number");
        }

        if (m_input[m_pos] == '0')
        {
            ++m_pos;
        }
        else
        {
            while (m_pos < m_size && m_input[m_pos] >= '0' && m_input[m_pos] <= '9')
            {
                ++m_pos;
            }
        }

        if (m_pos < m_size && m_input[m_pos] == '.')
        {
            ++m_pos;
            if (m_pos >= m_size || m_input[m_pos] < '0' || m_input[m_pos] > '9')
            {
                ThrowError("Invalid number: expected digit after decimal point");
            }
            while (m_pos < m_size && m_input[m_pos] >= '0' && m_input[m_pos] <= '9')
            {
                ++m_pos;
            }
        }

        if (m_pos < m_size && (m_input[m_pos] == 'e' || m_input[m_pos] == 'E'))
        {
            ++m_pos;
            if (m_pos < m_size && (m_input[m_pos] == '+' || m_input[m_pos] == '-'))
            {
                ++m_pos;
            }
            if (m_pos >= m_size || m_input[m_pos] < '0' || m_input[m_pos] > '9')
            {
                ThrowError("Invalid number: expected digit in exponent");
            }
            while (m_pos < m_size && m_input[m_pos] >= '0' && m_input[m_pos] <= '9')
            {
                ++m_pos;
            }
        }

        // Convert the number substring to f64. We need a null-terminated string for strtod.
        constexpr u64 k_buf_size = 64;
        InPlaceArray<char, k_buf_size> buf;
        const u64 len = m_pos - start;
        if (len >= k_buf_size)
        {
            ThrowError("Number literal too long");
        }
        for (u64 i = 0; i < len; ++i)
        {
            buf[i] = static_cast<char>(m_input[start + i]);
        }
        buf[len] = '\0';

        char* end_ptr = nullptr;
        const f64 result = strtod(buf.GetData(), &end_ptr);
        if (end_ptr != buf.GetData() + len)
        {
            ThrowError("Invalid number");
        }
        return result;
    }

    // -- String --

    StringViewUtf8 ParseString()
    {
        Expect('"');
        const u64 start = m_pos;
        bool has_escapes = false;

        while (m_pos < m_size && m_input[m_pos] != '"')
        {
            if (m_input[m_pos] == '\\')
            {
                has_escapes = true;
                ++m_pos;
                if (m_pos >= m_size)
                {
                    ThrowError("Unterminated string escape");
                }
            }
            ++m_pos;
        }

        if (m_pos >= m_size)
        {
            ThrowError("Unterminated string");
        }

        if (!has_escapes)
        {
            const StringViewUtf8 result(m_input + start, m_pos - start);
            ++m_pos;  // skip closing quote
            return result;
        }

        // String has escapes, need to unescape into a buffer.
        StringUtf8 unescaped(m_allocator);
        const u64 end = m_pos;
        u64 i = start;
        while (i < end)
        {
            if (m_input[i] == '\\')
            {
                ++i;
                switch (m_input[i])
                {
                    case '"':
                        unescaped.Append('"');
                        break;
                    case '\\':
                        unescaped.Append('\\');
                        break;
                    case '/':
                        unescaped.Append('/');
                        break;
                    case 'b':
                        unescaped.Append('\b');
                        break;
                    case 'f':
                        unescaped.Append('\f');
                        break;
                    case 'n':
                        unescaped.Append('\n');
                        break;
                    case 'r':
                        unescaped.Append('\r');
                        break;
                    case 't':
                        unescaped.Append('\t');
                        break;
                    case 'u':
                        UnescapeUnicode(unescaped, i);
                        break;
                    default:
                        ThrowError("Invalid escape sequence");
                }
                ++i;
            }
            else
            {
                unescaped.Append(m_input[i]);
                ++i;
            }
        }

        ++m_pos;  // skip closing quote

        m_escaped_strings.PushBack(std::move(unescaped));
        const StringUtf8& stored = m_escaped_strings[m_escaped_strings.GetSize() - 1];
        return {stored};
    }

    void UnescapeUnicode(StringUtf8& out, u64& i) const
    {
        // i points to 'u', the 4 hex digits follow.
        ++i;
        if (i + 4 > m_size)
        {
            ThrowError("Invalid unicode escape");
        }
        u32 codepoint = ParseHex4(i);
        i += 3;  // Advance 3 more (the outer loop will ++i for the 4th).

        // Handle surrogate pairs.
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
        {
            // High surrogate, expect low surrogate.
            const u64 next = i + 1;
            if (next + 5 < m_size && m_input[next] == '\\' && m_input[next + 1] == 'u')
            {
                const u32 low = ParseHex4(next + 2);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    i = next + 5;
                }
                else
                {
                    ThrowError("Invalid surrogate pair");
                }
            }
            else
            {
                ThrowError("Expected low surrogate");
            }
        }

        // Encode codepoint as UTF-8.
        EncodeUtf8(out, codepoint);
    }

    [[nodiscard]] u32 ParseHex4(u64 pos) const
    {
        u32 result = 0;
        for (u64 j = 0; j < 4; ++j)
        {
            const char8 c = m_input[pos + j];
            u32 digit = 0;
            if (c >= '0' && c <= '9')
            {
                digit = static_cast<u32>(c - '0');
            }
            else if (c >= 'a' && c <= 'f')
            {
                digit = static_cast<u32>(c - 'a') + 10;
            }
            else if (c >= 'A' && c <= 'F')
            {
                digit = static_cast<u32>(c - 'A') + 10;
            }
            else
            {
                ThrowError("Invalid hex digit in unicode escape");
            }
            result = (result << 4) | digit;
        }
        return result;
    }

    static void EncodeUtf8(StringUtf8& out, u32 codepoint)
    {
        if (codepoint <= 0x7F)
        {
            out.Append(static_cast<char8>(codepoint));
        }
        else if (codepoint <= 0x7FF)
        {
            out.Append(static_cast<char8>(0xC0 | (codepoint >> 6)));
            out.Append(static_cast<char8>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0xFFFF)
        {
            out.Append(static_cast<char8>(0xE0 | (codepoint >> 12)));
            out.Append(static_cast<char8>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.Append(static_cast<char8>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0x10FFFF)
        {
            out.Append(static_cast<char8>(0xF0 | (codepoint >> 18)));
            out.Append(static_cast<char8>(0x80 | ((codepoint >> 12) & 0x3F)));
            out.Append(static_cast<char8>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.Append(static_cast<char8>(0x80 | (codepoint & 0x3F)));
        }
    }

    // -- Array --

    JsonValue ParseArray()
    {
        Expect('[');
        SkipWhitespace();

        JsonArray arr(m_allocator);

        if (m_pos < m_size && m_input[m_pos] == ']')
        {
            ++m_pos;
            return JsonValue(std::move(arr));
        }

        while (true)
        {
            SkipWhitespace();
            arr->PushBack(ParseValue());
            SkipWhitespace();

            if (m_pos >= m_size)
            {
                ThrowError("Unterminated array");
            }

            if (m_input[m_pos] == ']')
            {
                ++m_pos;
                return JsonValue(std::move(arr));
            }

            if (m_input[m_pos] != ',')
            {
                ThrowError("Expected ',' or ']' in array");
            }
            ++m_pos;
        }
    }

    // -- Object --

    JsonValue ParseObject()
    {
        Expect('{');
        SkipWhitespace();

        JsonObject obj(m_allocator);

        if (m_pos < m_size && m_input[m_pos] == '}')
        {
            ++m_pos;
            return JsonValue(std::move(obj));
        }

        while (true)
        {
            SkipWhitespace();

            if (m_pos >= m_size || m_input[m_pos] != '"')
            {
                ThrowError("Expected string key in object");
            }

            const StringViewUtf8 key = ParseString();

            SkipWhitespace();
            Expect(':');
            SkipWhitespace();

            JsonValue value = ParseValue();
            obj->Insert(key, std::move(value));

            SkipWhitespace();

            if (m_pos >= m_size)
            {
                ThrowError("Unterminated object");
            }

            if (m_input[m_pos] == '}')
            {
                ++m_pos;
                return JsonValue(std::move(obj));
            }

            if (m_input[m_pos] != ',')
            {
                ThrowError("Expected ',' or '}' in object");
            }
            ++m_pos;
        }
    }

    // -- Utilities --

    void SkipWhitespace()
    {
        while (m_pos < m_size)
        {
            const char8 c = m_input[m_pos];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                if (c == '\n')
                {
                    ++m_line;
                    m_column = 1;
                }
                else
                {
                    ++m_column;
                }
                ++m_pos;
            }
            else
            {
                break;
            }
        }
    }

    void Expect(char8 expected)
    {
        if (m_pos >= m_size || m_input[m_pos] != expected)
        {
            InPlaceArray<char, 64> msg;
            snprintf(msg.GetData(), msg.GetSize(), "Expected '%c'", static_cast<char>(expected));
            ThrowError(msg.GetData());
        }
        ++m_pos;
        ++m_column;
    }

    [[noreturn]] void ThrowError(const char* message) const { throw JsonParseException(m_line, m_column, m_pos, message); }

    const char8* m_input = nullptr;
    u64 m_size = 0;
    u64 m_pos = 0;
    u64 m_line = 1;
    u64 m_column = 1;
    AllocatorBase* m_allocator = nullptr;
    DynamicArray<StringUtf8>& m_escaped_strings;
};

}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonReader.
// ------------------------------------------------------------------------------------------------

JsonReader JsonReader::Parse(const StringUtf8& input, AllocatorBase* allocator)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    JsonReader reader;
    reader.m_allocator = allocator;
    reader.m_escaped_strings = DynamicArray<StringUtf8>(allocator);

    JsonParser parser(input, allocator, reader.m_escaped_strings);
    reader.m_root = parser.Parse();
    return reader;
}

JsonReader JsonReader::Parse(StringUtf8&& input, AllocatorBase* allocator)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    JsonReader reader;
    reader.m_allocator = allocator;
    reader.m_owned_input = std::move(input);
    reader.m_escaped_strings = DynamicArray<StringUtf8>(allocator);

    const StringViewUtf8 view(reader.m_owned_input);
    JsonParser parser(view, allocator, reader.m_escaped_strings);
    reader.m_root = parser.Parse();
    return reader;
}

const JsonValue& JsonReader::GetRoot() const
{
    return m_root;
}

}  // namespace Opal

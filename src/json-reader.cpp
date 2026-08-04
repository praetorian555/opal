#include "opal/container/json-reader.h"

#include <cerrno>
#include <cstdlib>

#include "opal/container/dynamic-array.h"
#include "opal/container/in-place-array.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Helper.
// ------------------------------------------------------------------------------------------------

namespace
{
/**
 * Unwinds the parser out of however deep it got.
 *
 * The parser is a recursive descent over an explicit stack that fails from twenty-odd places, most of them with no
 * useful value to return. Threading a code back through every one of them would obscure the grammar, so the abort
 * stays a non-local jump. It is caught in Parse and handed back as a JsonParseError, so it never escapes the library.
 */
struct ParseAbort
{
    JsonParseError error;
};

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
    OPAL_RAISE(JsonTypeMismatchException(expected, JsonTypeToString(actual)));
}
}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonValue constructors.
// ------------------------------------------------------------------------------------------------

JsonValue::JsonValue() : m_data(JsonNull{}) {}
JsonValue::JsonValue(bool value) : m_data(value) {}
JsonValue::JsonValue(f64 value) : m_data(value) {}
JsonValue::JsonValue(i64 value) : m_data(value) {}
JsonValue::JsonValue(StringViewUtf8 value) : m_data(value) {}
JsonValue::JsonValue(JsonArray value) : m_data(std::move(value)) {}
JsonValue::JsonValue(JsonObject value) : m_data(std::move(value)) {}

// ------------------------------------------------------------------------------------------------
// JsonValue factory methods.
// ------------------------------------------------------------------------------------------------

JsonValue JsonValue::MakeNull()
{
    return {};
}

JsonValue JsonValue::MakeBool(bool value)
{
    return JsonValue(value);
}

JsonValue JsonValue::MakeNumber(f64 value)
{
    return JsonValue(value);
}

JsonValue JsonValue::MakeNumber(i64 value)
{
    return JsonValue(value);
}

JsonValue JsonValue::MakeString(StringViewUtf8 value)
{
    return JsonValue(value);
}

JsonValue JsonValue::MakeArray(AllocatorBase* allocator)
{
    return JsonValue(JsonArray(allocator));
}

JsonValue JsonValue::MakeObject(AllocatorBase* allocator)
{
    return JsonValue(JsonObject(allocator));
}

// ------------------------------------------------------------------------------------------------
// JsonValue mutation.
// ------------------------------------------------------------------------------------------------

void JsonValue::PushBack(JsonValue&& value)
{
    if (!IsArray())
    {
        ThrowTypeMismatch("array", GetType());
    }
    m_data.Get<JsonArray>()->PushBack(std::move(value));
}

void JsonValue::Insert(StringViewUtf8 key, JsonValue&& value)
{
    if (!IsObject())
    {
        ThrowTypeMismatch("object", GetType());
    }
    m_data.Get<JsonObject>()->Insert(key, std::move(value));
}

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
    const auto index = m_data.GetIndex();
    // Variant order: JsonNull(0), bool(1), f64(2), i64(3), StringViewUtf8(4), ...
    // JsonType enum: Null(0), Bool(1), Number(2), String(3), ...
    // i64 at index 3 also maps to Number, so indices > 2 need adjustment.
    if (index <= 2)
    {
        return static_cast<JsonType>(index);
    }
    // index 3 (i64) -> Number (2), index 4+ shifted down by 1.
    return static_cast<JsonType>(index - 1);
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
    return m_data.IsActive<f64>() || m_data.IsActive<i64>();
}
bool JsonValue::IsIntegerNumber() const
{
    return m_data.IsActive<i64>();
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

Expected<bool, ErrorCode> JsonValue::TryGetBool() const
{
    using Result = Expected<bool, ErrorCode>;
    if (!IsBool())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    return Result(m_data.Get<bool>());
}

Expected<f64, ErrorCode> JsonValue::TryGetNumber() const
{
    using Result = Expected<f64, ErrorCode>;
    if (!IsNumber())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    if (m_data.IsActive<i64>())
    {
        return Result(static_cast<f64>(m_data.Get<i64>()));
    }
    return Result(m_data.Get<f64>());
}

Expected<i64, ErrorCode> JsonValue::TryGetIntegerNumber() const
{
    using Result = Expected<i64, ErrorCode>;
    if (!IsIntegerNumber())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    return Result(m_data.Get<i64>());
}

Expected<StringViewUtf8, ErrorCode> JsonValue::TryGetString() const
{
    using Result = Expected<StringViewUtf8, ErrorCode>;
    if (!IsString())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    return Result(m_data.Get<StringViewUtf8>());
}

bool JsonValue::GetBool() const
{
    const Expected<bool, ErrorCode> value = TryGetBool();
    if (!value.HasValue())
    {
        ThrowTypeMismatch("bool", GetType());
    }
    return value.GetValue();
}

f64 JsonValue::GetNumber() const
{
    const Expected<f64, ErrorCode> value = TryGetNumber();
    if (!value.HasValue())
    {
        ThrowTypeMismatch("number", GetType());
    }
    return value.GetValue();
}

i64 JsonValue::GetIntegerNumber() const
{
    const Expected<i64, ErrorCode> value = TryGetIntegerNumber();
    if (!value.HasValue())
    {
        ThrowTypeMismatch("integer number", GetType());
    }
    return value.GetValue();
}

StringViewUtf8 JsonValue::GetString() const
{
    const Expected<StringViewUtf8, ErrorCode> value = TryGetString();
    if (!value.HasValue())
    {
        ThrowTypeMismatch("string", GetType());
    }
    return value.GetValue();
}

// ------------------------------------------------------------------------------------------------
// JsonValue element access.
// ------------------------------------------------------------------------------------------------

Expected<const JsonValue&, ErrorCode> JsonValue::TryAt(u64 index) const
{
    using Result = Expected<const JsonValue&, ErrorCode>;
    if (!IsArray())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    const auto& arr = m_data.Get<JsonArray>();
    if (index >= arr->GetSize())
    {
        return Result(ErrorCode::OutOfBounds);
    }
    return Result(arr->operator[](index));
}

Expected<const JsonValue&, ErrorCode> JsonValue::TryFind(StringViewUtf8 key) const
{
    using Result = Expected<const JsonValue&, ErrorCode>;
    if (!IsObject())
    {
        return Result(ErrorCode::TypeMismatch);
    }
    const auto& obj = m_data.Get<JsonObject>();
    const auto it = obj->Find(key);
    if (it == obj->cend())
    {
        return Result(ErrorCode::KeyNotFound);
    }
    return Result(it.GetValue());
}

const JsonValue& JsonValue::operator[](u64 index) const
{
    if (!IsArray())
    {
        ThrowTypeMismatch("array", GetType());
    }
    const Expected<const JsonValue&, ErrorCode> value = TryAt(index);
    if (!value.HasValue())
    {
        OPAL_RAISE(OutOfBoundsException(index, 0, m_data.Get<JsonArray>()->GetSize() - 1));
    }
    return value.GetValue();
}

const JsonValue& JsonValue::operator[](StringViewUtf8 key) const
{
    if (!IsObject())
    {
        ThrowTypeMismatch("object", GetType());
    }
    const Expected<const JsonValue&, ErrorCode> value = TryFind(key);
    if (!value.HasValue())
    {
        OPAL_RAISE(InvalidArgumentException("JsonValue::operator[]", "Key not found"));
    }
    return value.GetValue();
}

const JsonValue& JsonValue::GetPath(StringViewUtf8 path) const
{
    const Expected<const JsonValue&, ErrorCode> value = TryGetPath(path);
    if (!value.HasValue())
    {
        // Walk it again through the throwing accessors so the failing step reports itself the way it always has.
        return GetPathThrowing(path);
    }
    return value.GetValue();
}

Expected<const JsonValue&, ErrorCode> JsonValue::TryGetPath(StringViewUtf8 path) const
{
    using Result = Expected<const JsonValue&, ErrorCode>;
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
                index = (index * 10) + static_cast<u64>(c - '0');
            }

            Expected<const JsonValue&, ErrorCode> next =
                (is_index && current->IsArray()) ? current->TryAt(index) : current->TryFind(segment);
            if (!next.HasValue())
            {
                return Result(next.GetError());
            }
            current = &next.GetValue();

            start = i + 1;
        }
    }

    return Result(*current);
}

const JsonValue& JsonValue::GetPathThrowing(StringViewUtf8 path) const
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
                index = (index * 10) + static_cast<u64>(c - '0');
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
#ifdef OPAL_DEBUG
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

struct ParseFrame
{
    JsonArray arr;
    JsonObject obj;
    StringViewUtf8 key;
    bool is_object;
};

// The parser unwinds out of a recursive descent with a non-local jump, so it needs exceptions. Everything else in this file, the
// JsonValue accessors included, works without them.
#if defined(OPAL_EXCEPTIONS)

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
        if (m_pos >= m_size)
        {
            ThrowError("Unexpected end of input");
        }

        JsonValue value = ParseLeafOrOpenContainer();

        // Deliver completed values up the stack.
        for (;;)
        {
            if (m_stack.IsEmpty())
            {
                SkipWhitespace();
                if (m_pos < m_size)
                {
                    ThrowError("Unexpected trailing content");
                }
                return value;
            }

            // Deliver value to the current container frame.
            ParseFrame& frame = m_stack.Back();
            if (frame.is_object)
            {
                frame.obj->Insert(frame.key, std::move(value));
            }
            else
            {
                frame.arr->PushBack(std::move(value));
            }

            SkipWhitespace();
            if (m_pos >= m_size)
            {
                ThrowError(frame.is_object ? "Unterminated object" : "Unterminated array");
            }

            const char8 c = m_input[m_pos];
            if (frame.is_object)
            {
                if (c == '}')
                {
                    ++m_pos;
                    ++m_column;
                    value = JsonValue(std::move(m_stack.Back().obj));
                    m_stack.PopBack();
                    continue;
                }
                if (c != ',')
                {
                    ThrowError("Expected ',' or '}' in object");
                }
                ++m_pos;
                ++m_column;

                // Parse the next key.
                SkipWhitespace();
                if (m_pos >= m_size || m_input[m_pos] != '"')
                {
                    ThrowError("Expected string key in object");
                }
                m_stack.Back().key = ParseString();
                SkipWhitespace();
                Expect(':');
            }
            else
            {
                if (c == ']')
                {
                    ++m_pos;
                    ++m_column;
                    value = JsonValue(std::move(m_stack.Back().arr));
                    m_stack.PopBack();
                    continue;
                }
                if (c != ',')
                {
                    ThrowError("Expected ',' or ']' in array");
                }
                ++m_pos;
                ++m_column;
            }

            // Parse the next element value.
            SkipWhitespace();
            if (m_pos >= m_size)
            {
                ThrowError("Unexpected end of input");
            }
            value = ParseLeafOrOpenContainer();
        }
    }

private:
    // -- Value dispatch --

    JsonValue ParseLeafOrOpenContainer()
    {
        const char8 c = m_input[m_pos];
        switch (c)
        {
            case '"':
                return JsonValue(ParseString());
            case 't':
            case 'f':
                return JsonValue(ParseBool());
            case 'n':
                return ParseNull();
            case '[':
            {
                Expect('[');
                SkipWhitespace();
                if (m_pos < m_size && m_input[m_pos] == ']')
                {
                    ++m_pos;
                    ++m_column;
                    return JsonValue(MakeArray());
                }
                if (!(m_stack.EmplaceBack(ParseFrame{MakeArray(), JsonObject{}, StringViewUtf8{}, false})).HasValue()) [[unlikely]]
                {
                    AbortOutOfMemory();
                }
                return ParseLeafOrOpenContainer();
            }
            case '{':
            {
                Expect('{');
                SkipWhitespace();
                if (m_pos < m_size && m_input[m_pos] == '}')
                {
                    ++m_pos;
                    ++m_column;
                    return JsonValue(MakeObject());
                }
                if (m_pos >= m_size || m_input[m_pos] != '"')
                {
                    ThrowError("Expected string key in object");
                }
                const StringViewUtf8 key = ParseString();
                SkipWhitespace();
                Expect(':');
                SkipWhitespace();
                if (!(m_stack.EmplaceBack(JsonArray{}, MakeObject(), key, true)).HasValue()) [[unlikely]]
                {
                    AbortOutOfMemory();
                }
                return ParseLeafOrOpenContainer();
            }
            default:
            {
                if (c == '-' || (c >= '0' && c <= '9'))
                {
                    return ParseNumber();
                }
                ThrowError("Unexpected character");
            }
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

    JsonValue ParseNumber()
    {
        const u64 start = m_pos;
        bool is_float = false;

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
            is_float = true;
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
            is_float = true;
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

        // Copy number substring into a null-terminated buffer.
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

        // Integer literals (no '.' or 'e'/'E') are stored as i64 when they fit.
        if (!is_float)
        {
            char* end_ptr = nullptr;
            errno = 0;
            const i64 result = strtoll(buf.GetData(), &end_ptr, 10);
            if (end_ptr == buf.GetData() + len && errno != ERANGE)
            {
                return JsonValue(result);
            }
        }

        char* end_ptr = nullptr;
        const f64 result = strtod(buf.GetData(), &end_ptr);
        if (end_ptr != buf.GetData() + len)
        {
            ThrowError("Invalid number");
        }
        return JsonValue(result);
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
                        AppendOrAbort(unescaped, '"');
                        break;
                    case '\\':
                        AppendOrAbort(unescaped, '\\');
                        break;
                    case '/':
                        AppendOrAbort(unescaped, '/');
                        break;
                    case 'b':
                        AppendOrAbort(unescaped, '\b');
                        break;
                    case 'f':
                        AppendOrAbort(unescaped, '\f');
                        break;
                    case 'n':
                        AppendOrAbort(unescaped, '\n');
                        break;
                    case 'r':
                        AppendOrAbort(unescaped, '\r');
                        break;
                    case 't':
                        AppendOrAbort(unescaped, '\t');
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
                AppendOrAbort(unescaped, m_input[i]);
                ++i;
            }
        }

        ++m_pos;  // skip closing quote

        m_escaped_strings->PushBack(std::move(unescaped));
        const StringUtf8& stored = m_escaped_strings.Get()[m_escaped_strings->GetSize() - 1];
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

    void AppendOrAbort(StringUtf8& out, char8 ch) const
    {
        if (out.Append(ch) != ErrorCode::Success) [[unlikely]]
        {
            AbortOutOfMemory();
        }
    }

    void EncodeUtf8(StringUtf8& out, u32 codepoint) const
    {
        if (codepoint <= 0x7F)
        {
            AppendOrAbort(out, static_cast<char8>(codepoint));
        }
        else if (codepoint <= 0x7FF)
        {
            AppendOrAbort(out, static_cast<char8>(0xC0 | (codepoint >> 6)));
            AppendOrAbort(out, static_cast<char8>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0xFFFF)
        {
            AppendOrAbort(out, static_cast<char8>(0xE0 | (codepoint >> 12)));
            AppendOrAbort(out, static_cast<char8>(0x80 | ((codepoint >> 6) & 0x3F)));
            AppendOrAbort(out, static_cast<char8>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0x10FFFF)
        {
            AppendOrAbort(out, static_cast<char8>(0xF0 | (codepoint >> 18)));
            AppendOrAbort(out, static_cast<char8>(0x80 | ((codepoint >> 12) & 0x3F)));
            AppendOrAbort(out, static_cast<char8>(0x80 | ((codepoint >> 6) & 0x3F)));
            AppendOrAbort(out, static_cast<char8>(0x80 | (codepoint & 0x3F)));
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
            memset(msg.GetData(), 0, 64);
            snprintf(msg.GetData(), msg.GetSize(), "Expected '%c'", static_cast<char>(expected));
            ThrowError(msg.GetData());
        }
        ++m_pos;
        ++m_column;
    }

    [[noreturn]] void ThrowError(const char* message) const
    {
        throw ParseAbort{JsonParseError(ErrorCode::InvalidArgument, m_line, m_column, m_pos, message)};
    }

    [[noreturn]] void AbortOutOfMemory() const
    {
        throw ParseAbort{JsonParseError(ErrorCode::OutOfMemory, m_line, m_column, m_pos, "Out of memory")};
    }

    // The containers are reference counted, so building one is an allocation that a budgeted allocator can refuse.
    JsonArray MakeArray() const
    {
        Expected<JsonArray, ErrorCode> array = JsonArray::Create(m_allocator);
        if (!array.HasValue()) [[unlikely]]
        {
            AbortOutOfMemory();
        }
        return std::move(array).GetValue();
    }

    JsonObject MakeObject() const
    {
        Expected<JsonObject, ErrorCode> object = JsonObject::Create(m_allocator);
        if (!object.HasValue()) [[unlikely]]
        {
            AbortOutOfMemory();
        }
        return std::move(object).GetValue();
    }

    const char8* m_input = nullptr;
    u64 m_size = 0;
    u64 m_pos = 0;
    u64 m_line = 1;
    u64 m_column = 1;
    AllocatorBase* m_allocator = nullptr;
    Ref<DynamicArray<StringUtf8>> m_escaped_strings;
    DynamicArray<ParseFrame> m_stack;
};

#endif  // OPAL_EXCEPTIONS

}  // namespace

// ------------------------------------------------------------------------------------------------
// JsonReader.
// ------------------------------------------------------------------------------------------------

#if defined(OPAL_EXCEPTIONS)

Expected<JsonReader, JsonParseError> JsonReader::Parse(const StringUtf8& input, AllocatorBase* allocator)
{
    using Result = Expected<JsonReader, JsonParseError>;
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    JsonReader reader;
    reader.m_allocator = allocator;
    reader.m_escaped_strings = DynamicArray<StringUtf8>(allocator);

    JsonParser parser(input, allocator, reader.m_escaped_strings);
    try
    {
        reader.m_root = parser.Parse();
    } catch (const ParseAbort& abort)
    {
        return Result(abort.error);
    }
    return Result(Move(reader));
}

Expected<JsonReader, JsonParseError> JsonReader::Parse(StringUtf8&& input, AllocatorBase* allocator)
{
    using Result = Expected<JsonReader, JsonParseError>;
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    JsonReader reader;
    reader.m_allocator = allocator;
    reader.m_owned_input = MakeScoped<StringUtf8>(allocator, std::move(input));
    if (!reader.m_owned_input.IsValid()) [[unlikely]]
    {
        return Result(JsonParseError(ErrorCode::OutOfMemory, 0, 0, 0, "Out of memory"));
    }
    reader.m_escaped_strings = DynamicArray<StringUtf8>(allocator);

    const StringViewUtf8 view(*reader.m_owned_input.Get());
    JsonParser parser(view, allocator, reader.m_escaped_strings);
    try
    {
        reader.m_root = parser.Parse();
    } catch (const ParseAbort& abort)
    {
        return Result(abort.error);
    }
    return Result(Move(reader));
}

#endif  // OPAL_EXCEPTIONS

const JsonValue& JsonReader::GetRoot() const
{
    return m_root;
}

}  // namespace Opal

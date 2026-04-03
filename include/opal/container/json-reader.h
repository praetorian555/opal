#pragma once

#include "opal/allocator.h"
#include "opal/casts.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/hash-map.h"
#include "opal/container/shared-ptr.h"
#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/exceptions.h"
#include "opal/variant.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Exceptions.
// ------------------------------------------------------------------------------------------------

struct JsonParseException : Exception
{
    JsonParseException(u64 in_line, u64 in_column, u64 in_byte_offset, const char* message)
        : Exception(StringEx("JSON parse error at ") + in_line + ":" + in_column + ": " + message),
          line(in_line),
          column(in_column),
          byte_offset(in_byte_offset)
    {
    }

    u64 line = 0;
    u64 column = 0;
    u64 byte_offset = 0;
};

struct JsonTypeMismatchException : Exception
{
    JsonTypeMismatchException(const char* expected, const char* actual)
        : Exception(StringEx("JSON type mismatch: expected ") + expected + ", got " + actual)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Forward declarations and type aliases.
// ------------------------------------------------------------------------------------------------

struct JsonNull
{
};

enum class JsonType : u8
{
    Null,
    Bool,
    Number,
    String,
    Array,
    Object,
};

class JsonValue;

using JsonArray = SharedPtr<DynamicArray<JsonValue>, ThreadingPolicy::SingleThread>;
using JsonObject = SharedPtr<HashMap<StringViewUtf8, JsonValue>, ThreadingPolicy::SingleThread>;

// ------------------------------------------------------------------------------------------------
// JsonValue.
// ------------------------------------------------------------------------------------------------

class JsonValue
{
public:
    using VariantType = Variant<JsonNull, bool, f64, i64, StringViewUtf8, JsonArray, JsonObject>;

    // -- Constructors --

    JsonValue();
    explicit JsonValue(bool value);
    explicit JsonValue(f64 value);
    explicit JsonValue(i64 value);
    explicit JsonValue(StringViewUtf8 value);
    explicit JsonValue(JsonArray value);
    explicit JsonValue(JsonObject value);

    JsonValue(const JsonValue&) = delete;
    JsonValue& operator=(const JsonValue&) = delete;

    JsonValue(JsonValue&& other) noexcept = default;
    JsonValue& operator=(JsonValue&& other) noexcept = default;

    ~JsonValue() = default;

    // -- Factory methods --

    [[nodiscard]] static JsonValue MakeNull();
    [[nodiscard]] static JsonValue MakeBool(bool value);
    [[nodiscard]] static JsonValue MakeNumber(f64 value);
    [[nodiscard]] static JsonValue MakeNumber(i64 value);

    template <typename T>
        requires IntegralOrFloatingPoint<T>
    [[nodiscard]] static JsonValue MakeNumber(T value)
    {
        if constexpr (Integral<T>)
        {
            return JsonValue(static_cast<i64>(value));
        }
        else
        {
            return JsonValue(static_cast<f64>(value));
        }
    }

    [[nodiscard]] static JsonValue MakeString(StringViewUtf8 value);
    [[nodiscard]] static JsonValue MakeArray(AllocatorBase* allocator = nullptr);
    [[nodiscard]] static JsonValue MakeObject(AllocatorBase* allocator = nullptr);

    // -- Mutation --

    /**
     * Appends a value to the end of an array.
     * @throws JsonTypeMismatchException if not an array.
     */
    void PushBack(JsonValue&& value);

    /**
     * Inserts a key-value pair into an object.
     * @throws JsonTypeMismatchException if not an object.
     */
    void Insert(StringViewUtf8 key, JsonValue&& value);

    // -- Clone --

    [[nodiscard]] JsonValue Clone(AllocatorBase* allocator = nullptr) const;

    // -- Type queries --

    [[nodiscard]] JsonType GetType() const;

    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsBool() const;
    [[nodiscard]] bool IsNumber() const;
    [[nodiscard]] bool IsIntegerNumber() const;
    [[nodiscard]] bool IsString() const;
    [[nodiscard]] bool IsArray() const;
    [[nodiscard]] bool IsObject() const;

    // -- Value extraction (throw JsonTypeMismatchException on type mismatch) --

    [[nodiscard]] bool GetBool() const;
    [[nodiscard]] f64 GetNumber() const;
    [[nodiscard]] i64 GetIntegerNumber() const;
    [[nodiscard]] StringViewUtf8 GetString() const;

    template <typename T>
        requires IntegralOrFloatingPoint<T>
    [[nodiscard]] T GetNumberAs() const
    {
        if (m_data.IsActive<i64>())
        {
            return Narrow<T>(m_data.Get<i64>());
        }
        return Narrow<T>(GetNumber());
    }

    // -- Element access (throw on type mismatch or missing key/index) --

    [[nodiscard]] const JsonValue& operator[](u64 index) const;
    [[nodiscard]] const JsonValue& operator[](StringViewUtf8 key) const;

    [[nodiscard]] const JsonValue& GetPath(StringViewUtf8 path) const;

    // -- Container queries --

    [[nodiscard]] u64 GetSize() const;

    // -- Array iteration (range-for support) --

    class ArrayIterator
    {
    public:
        ArrayIterator(const JsonValue* data, u64 index);

        const JsonValue& operator*() const;
        ArrayIterator& operator++();
        bool operator==(const ArrayIterator& other) const;
        bool operator!=(const ArrayIterator& other) const;

    private:
        const JsonValue* m_data = nullptr;
        u64 m_index = 0;
    };

    /**
     * Range-for support for arrays.
     * Usage: for (const JsonValue& elem : json_array) { ... }
     * @throws JsonTypeMismatchException if not an array.
     */
    [[nodiscard]] ArrayIterator begin() const;
    [[nodiscard]] ArrayIterator end() const;

    // -- Object iteration --

    struct ObjectEntry
    {
        StringViewUtf8 key;
        Ref<const JsonValue> value;
    };

    class ObjectIterator
    {
    public:
        using InnerIterator = HashMapConstIterator<HashMap<StringViewUtf8, JsonValue>>;

        explicit ObjectIterator(InnerIterator it);

        ObjectEntry operator*() const;
        ObjectIterator& operator++();
        bool operator==(const ObjectIterator& other) const;
        bool operator!=(const ObjectIterator& other) const;

    private:
        InnerIterator m_it;
    };

    class ObjectRange
    {
    public:
        ObjectRange(ObjectIterator begin_it, ObjectIterator end_it);

        [[nodiscard]] ObjectIterator begin() const;
        [[nodiscard]] ObjectIterator end() const;

    private:
        ObjectIterator m_begin;
        ObjectIterator m_end;
    };

    /**
     * Returns an iterable range over the key-value pairs of an object.
     * Usage: for (auto [key, val] : json_object.Items()) { ... }
     * @throws JsonTypeMismatchException if not an object.
     */
    [[nodiscard]] ObjectRange Items() const;

    // -- Pattern matching --

    template <typename Visitor>
    auto Visit(Visitor&& visitor) const
    {
        return m_data.Visit(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void VisitPartial(Visitor&& visitor) const
    {
        m_data.VisitPartial(std::forward<Visitor>(visitor));
    }

private:
    VariantType m_data;
};

// ------------------------------------------------------------------------------------------------
// JsonReader.
// ------------------------------------------------------------------------------------------------

class JsonReader
{
public:
    /**
     * Parse JSON from a borrowed string view. Caller must keep the input alive
     * as long as this JsonReader exists.
     * @param input JSON text. Caller retains ownership.
     * @param allocator Allocator for all internal allocations. If nullptr, the default allocator is used.
     * @throws JsonParseException on malformed JSON input.
     */
    static JsonReader Parse(const StringUtf8& input, AllocatorBase* allocator = nullptr);

    /**
     * Parse JSON and take ownership of the input string.
     * @param input JSON text. Ownership is transferred to JsonReader via move.
     * @param allocator Allocator for all internal allocations. If nullptr, the default allocator is used.
     * @throws JsonParseException on malformed JSON input.
     */
    static JsonReader Parse(StringUtf8&& input, AllocatorBase* allocator = nullptr);

    JsonReader(const JsonReader&) = delete;
    JsonReader& operator=(const JsonReader&) = delete;

    JsonReader(JsonReader&& other) noexcept = default;
    JsonReader& operator=(JsonReader&& other) noexcept = default;

    ~JsonReader() = default;

    [[nodiscard]] const JsonValue& GetRoot() const;

private:
    JsonReader() = default;

    JsonValue m_root;
    StringUtf8 m_owned_input;
    DynamicArray<StringUtf8> m_escaped_strings;
    AllocatorBase* m_allocator = nullptr;
};

}  // namespace Opal

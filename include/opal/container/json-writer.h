#pragma once

#include "opal/container/expected.h"
#include "opal/container/json-reader.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/export.h"

namespace Opal
{

// ------------------------------------------------------------------------------------------------
// Options.
// ------------------------------------------------------------------------------------------------

struct JsonWriteOptions
{
    bool pretty = false;
    u32 indent_width = 4;
    bool use_tabs = false;
};

// ------------------------------------------------------------------------------------------------
// JsonWriter.
// ------------------------------------------------------------------------------------------------

class OPAL_EXPORT JsonWriter
{
public:
    /**
     * Serialize a JsonValue to a compact JSON string.
     * @param value    The JSON value to serialize.
     * @param allocator Allocator for the output string. If nullptr, the default allocator is used.
     * @return The JSON text, ErrorCode::InvalidArgument when the value holds a NaN or an infinity, which JSON cannot
     *         spell, or ErrorCode::OutOfMemory when the output could not be grown.
     */
    [[nodiscard]] static Expected<StringUtf8, ErrorCode> Serialize(const JsonValue& value, AllocatorBase* allocator = nullptr);

    /**
     * Serialize a JsonValue to a JSON string with formatting options.
     * @param value    The JSON value to serialize.
     * @param options  Formatting options (pretty print, indent width, tabs vs spaces).
     * @param allocator Allocator for the output string. If nullptr, the default allocator is used.
     * @return The JSON text, ErrorCode::InvalidArgument when the value holds a NaN or an infinity, which JSON cannot
     *         spell, or ErrorCode::OutOfMemory when the output could not be grown.
     */
    [[nodiscard]] static Expected<StringUtf8, ErrorCode> Serialize(const JsonValue& value, const JsonWriteOptions& options,
                                                                   AllocatorBase* allocator = nullptr);

    JsonWriter(const JsonWriter&) = delete;
    JsonWriter& operator=(const JsonWriter&) = delete;
    JsonWriter(JsonWriter&&) = delete;
    JsonWriter& operator=(JsonWriter&&) = delete;

private:
    JsonWriter() = default;
};

}  // namespace Opal

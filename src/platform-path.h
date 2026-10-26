#pragma once

#include "opal/container/string-view.h"
#include "opal/container/string.h"
#include "opal/defines.h"
#include "opal/error-codes.h"

#if defined(OPAL_PLATFORM_POSIX)
#include <limits.h>
#include <string.h>
#endif

// Turns a path view into the form the operating system's file calls take. Private to the library sources that make those calls.

namespace Opal::Impl
{

#if defined(OPAL_PLATFORM_WINDOWS)

/**
 * Transcodes a path to UTF-16 for the wide Win32 calls.
 * @param path Path to transcode.
 * @param out_path Receives the null-terminated UTF-16 path.
 * @return ErrorCode::Success, ErrorCode::OutOfMemory, or what Transcode reported for a path that is not valid UTF-8.
 */
inline ErrorCode ToWidePath(StringViewUtf8 path, StringWide& out_path)
{
    return Transcode(path, out_path);
}

#elif defined(OPAL_PLATFORM_POSIX)

/** A path with a null terminator, for the POSIX calls that need one. Held on the stack, so making one does not allocate. */
struct NativePath
{
    char data[PATH_MAX];
};

/**
 * Copies a path into a NativePath.
 * @param path Path to copy.
 * @param out_path Receives the null-terminated path.
 * @return ErrorCode::Success, or ErrorCode::InvalidArgument for a path of PATH_MAX bytes or more, which the kernel would reject too.
 */
inline ErrorCode ToNativePath(StringViewUtf8 path, NativePath& out_path)
{
    if (path.GetSize() >= PATH_MAX)
    {
        return ErrorCode::InvalidArgument;
    }
    memcpy(out_path.data, path.GetData(), path.GetSize());
    out_path.data[path.GetSize()] = '\0';
    return ErrorCode::Success;
}

#endif

}  // namespace Opal::Impl

#pragma once

#if _WIN32
#define OPAL_PLATFORM_WINDOWS
#endif

#if !defined(OPAL_PLATFORM_WINDOWS)
#error "Only Windows is supported at the moment"
#endif

#if _MSC_VER
#define OPAL_COMPILER_MSVC
#endif

#if !defined(OPAL_COMPILER_MSVC)
#error "Only MSVC is supported at the moment"
#endif

#define OPAL_VERSION_MAJOR 0
#define OPAL_VERSION_MINOR 1

#define OPAL_VERSION_STRING "0.1"

#if defined(OPAL_COMPILER_MSVC)
#define OPAL_DISABLE_OPTIMIZATION __pragma(optimize("", off))
#define OPAL_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#endif

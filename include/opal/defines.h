#pragma once

#if _WIN32
#define OPAL_PLATFORM_WINDOWS
#elif __linux__
#define OPAL_PLATFORM_LINUX
#endif

#if _MSC_VER
#define OPAL_COMPILER_MSVC
#elif __clang__
#define OPAL_COMPILER_CLANG
#elif __GNUC__
#define OPAL_COMPILER_GCC
#endif

#define OPAL_STRINGIFY(x) #x
#define OPAL_TOSTRING(x) OPAL_STRINGIFY(x)

#if defined(OPAL_COMPILER_MSVC)
#define OPAL_START_DISABLE_WARNINGS __pragma(warning(push))
#define OPAL_DISABLE_WARNING(warningName)
#define OPAL_END_DISABLE_WARNINGS __pragma(warning(pop))
#elif defined(OPAL_COMPILER_GCC)
#define OPAL_START_DISABLE_WARNINGS _Pragma("GCC diagnostic push")
#define OPAL_DISABLE_WARNING(warningName) _Pragma(OPAL_TOSTRING(GCC diagnostic ignored #warningName))
#define OPAL_END_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")
#elif defined(OPAL_COMPILER_CLANG)
#define OPAL_START_DISABLE_WARNINGS _Pragma("clang diagnostic push")
#define OPAL_DISABLE_WARNING(warningName) _Pragma(OPAL_TOSTRING(clang diagnostic ignored #warningName))
#define OPAL_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
#endif

#define OPAL_VERSION_MAJOR 0
#define OPAL_VERSION_MINOR 1

#define OPAL_VERSION_STRING "0.1"

#if defined(OPAL_COMPILER_MSVC)
#define OPAL_DISABLE_OPTIMIZATION __pragma(optimize("", off))
#define OPAL_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#elif defined(OPAL_COMPILER_GCC)
#define OPAL_DISABLE_OPTIMIZATION _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#define OPAL_ENABLE_OPTIMIZATION _Pragma("GCC pop_options")
#elif defined(OPAL_COMPILER_CLANG)
#define OPAL_DISABLE_OPTIMIZATION _Pragma("clang optimize off")
#define OPAL_ENABLE_OPTIMIZATION _Pragma("clang optimize on")
#endif

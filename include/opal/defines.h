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
#define OPAL_DISABLE_WARNING(warning_name)
#define OPAL_DISABLE_MSVC_WARNING(warning_name) __pragma(warning(disable: warning_name))
#define OPAL_END_DISABLE_WARNINGS __pragma(warning(pop))
#elif defined(OPAL_COMPILER_GCC)
#define OPAL_START_DISABLE_WARNINGS _Pragma("GCC diagnostic push")
#define OPAL_DISABLE_WARNING(warning_name) _Pragma(OPAL_TOSTRING(GCC diagnostic ignored warning_name))
#define OPAL_DISABLE_MSVC_WARNING(warning_name)
#define OPAL_END_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")
#elif defined(OPAL_COMPILER_CLANG)
#define OPAL_START_DISABLE_WARNINGS _Pragma("clang diagnostic push")
#define OPAL_DISABLE_WARNING(warning_name)  _Pragma(OPAL_TOSTRING(clang diagnostic ignored warning_name))
#define OPAL_DISABLE_MSVC_WARNING(warning_name)
#define OPAL_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
#endif

// Keep in step with the version passed to project() in CMakeLists.txt.
#define OPAL_VERSION_MAJOR 0
#define OPAL_VERSION_MINOR 5
#define OPAL_VERSION_PATCH 0

#define OPAL_VERSION_STRING "0.5.0"

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

#if defined(NDEBUG)
#define OPAL_RELEASE
#else
#define OPAL_DEBUG
#endif

// Whether the library may throw. Off when the compiler was told there are no exceptions, so that a build with -fno-exceptions or
// /EHsc- gets the same answer without having to be told twice. Define OPAL_NO_EXCEPTIONS to turn them off while the compiler still
// allows them.
#if !defined(OPAL_NO_EXCEPTIONS)
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#define OPAL_EXCEPTIONS
#endif
#endif

#define OPAL_CACHE_LINE_SIZE (64)


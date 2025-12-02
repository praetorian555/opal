#pragma once

#include "opal/defines.h"

namespace Opal
{

using i8 = signed char;
using i16 = short;
using i32 = int;
using i64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

using char8 = char;
using uchar32 = char32_t;

#if defined(OPAL_PLATFORM_WINDOWS)
using char16 = wchar_t;
#elif defined(OPAL_PLATFORM_LINUX)
using char16 = char16_t;
#endif

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

static_assert(sizeof(char8) == 1);
static_assert(sizeof(char16) == 2);

//static_assert(sizeof(c8) == 1);
//static_assert(sizeof(c16) == 2);
//static_assert(sizeof(c32) == 4);

#define OPAL_KB(x) (x * 1024ull)
#define OPAL_MB(x) (x * 1024ull * 1024ull)
#define OPAL_GB(x) (x * 1024ull * 1024ull * 1024ull)
#define OPAL_TB(x) (x * 1024ull * 1024ull * 1024ull * 1024ull)

}  // namespace Opal

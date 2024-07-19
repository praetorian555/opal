#pragma once

#include <array>

#include "opal/types.h"

namespace Opal
{

template <typename T, u64 N>
using StackArray = std::array<T, N>;

} // namespace Opal

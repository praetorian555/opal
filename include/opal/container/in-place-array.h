#pragma once

#include <array>

#include "opal/types.h"

namespace Opal
{

template <typename T, u64 N>
using InPlaceArray = std::array<T, N>;

} // namespace Opal

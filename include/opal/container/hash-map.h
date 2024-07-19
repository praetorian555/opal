#pragma once

#include <unordered_map>

namespace Opal
{

template <typename Key, typename Value, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, Value, Hash>;

} // namespace Opal

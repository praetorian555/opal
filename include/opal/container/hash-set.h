#pragma once

#include <unordered_set>

namespace Opal
{

template <typename Key, typename Hash = std::hash<Key>>
using HashSet = std::unordered_set<Key, Hash>;

} // namespace Opal

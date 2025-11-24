#include "opal/hash.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING("-Wold-style-cast")
OPAL_DISABLE_WARNING("-Wconversion")
#include "wyhash/wyhash.h"
OPAL_END_DISABLE_WARNINGS

Opal::u64 Opal::Hash::CalcRawArray(const Opal::u8* data, Opal::u64 size, Opal::u64 seed)
{
    return wyhash(data, size, seed, _wyp);
}


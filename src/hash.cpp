#include "opal/hash.h"

#include "wyhash/wyhash.h"

Opal::u64 Opal::Hash::CalcRawArray(const Opal::u8* data, Opal::u64 size, Opal::u64 seed)
{
    return wyhash(data, size, seed, _wyp);
}


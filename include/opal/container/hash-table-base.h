#pragma once

#include <emmintrin.h>

#include "opal/bit.h"
#include "opal/types.h"

// Shared machinery behind HashSet and HashMap. Both store their elements in a flat array of slots paired with an array of one byte
// control values, one per slot, and probe the control array a group at a time. Nothing here depends on the element type, so it lives
// outside of both templates.

namespace Opal::Impl
{

/** Number of control bytes one probe step examines at once. */
inline constexpr u64 k_group_width = 16;

/** Control value for a slot that has never been occupied. Probing stops when it meets one. */
inline constexpr i8 k_control_empty = -128;  // 0b10000000

/** Control value for a slot whose element was erased. Probing passes over it and keeps going. */
inline constexpr i8 k_control_deleted = -2;  // 0b11111110

/** Control value marking the end of the control bytes. Never matches a lookup. */
inline constexpr i8 k_control_sentinel = -1;  // 0b11111111

/** Smallest number of elements a table is built for. */
inline constexpr u64 k_default_capacity = 4;

/** @return True when the control value belongs to a slot holding an element. */
[[nodiscard]] inline bool IsControlFull(i8 control)
{
    return control >= 0;
}

/** @return The smallest 2^n - 1 that is at least `value`, and 1 for a `value` of 0. Doubles as the mask that wraps an index. */
[[nodiscard]] inline u64 GetNextPowerOf2MinusOne(u64 value)
{
    return value != 0 ? ~u64{} >> CountLeadingZeros(value) : 1;
}

/** @return How many elements a table of `capacity` slots takes before it has to grow. */
[[nodiscard]] inline u64 GetGrowthThreshold(u64 capacity)
{
    return (capacity * 7) / 8;
}

/** @return The part of the hash that picks the group to start probing from. `seed` varies the result per table. */
[[nodiscard]] inline u64 GetHash1(u64 hash, const void* seed)
{
    return (hash >> 7) ^ (reinterpret_cast<u64>(seed) >> 12);
}

/** @return The seven bits of the hash kept in the control byte. Always non-negative, so it never collides with a special value. */
[[nodiscard]] inline i8 GetHash2(u64 hash)
{
    return static_cast<i8>(hash & 0x7f);
}

/**
 * Finds the slots in a group whose control byte equals `pattern`.
 * @param group First of k_group_width control bytes. Does not have to be aligned.
 * @param pattern Control value to look for.
 * @return Mask with a bit set per matching slot, in group order.
 */
[[nodiscard]] inline BitMask<u32> GetGroupMatch(const i8* group, i8 pattern)
{
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    const __m128i match = _mm_set1_epi8(pattern);
    // Compare byte for byte, then collect the top bit of each result byte into the low 16 bits of a word.
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl))));
}

/**
 * Finds the slots in a group that were never occupied.
 * @param group First of k_group_width control bytes. Does not have to be aligned.
 * @return Mask with a bit set per empty slot, in group order.
 */
[[nodiscard]] inline BitMask<u32> GetGroupMatchEmpty(const i8* group)
{
    return GetGroupMatch(group, k_control_empty);
}

/**
 * Finds the slots in a group that hold no element, whether they were erased or never used.
 * @param group First of k_group_width control bytes. Does not have to be aligned.
 * @return Mask with a bit set per available slot, in group order.
 */
[[nodiscard]] inline BitMask<u32> GetGroupNotFull(const i8* group)
{
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    const __m128i special = _mm_set1_epi8(k_control_sentinel);
    // Empty and deleted both order below the sentinel, every occupied value orders above it.
    return BitMask<u32>(static_cast<u32>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, ctrl))));
}

/**
 * Finds the slots in a group that hold an element.
 * @param group First of k_group_width control bytes. Does not have to be aligned.
 * @return Mask with a bit set per occupied slot, in group order.
 */
[[nodiscard]] inline u32 GetGroupFullMask(const i8* group)
{
    const __m128i ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(group));
    // Every special value has its top bit set and a hash fragment does not, so the occupied slots are the bits movemask leaves clear.
    return static_cast<u32>(_mm_movemask_epi8(ctrl)) ^ 0xFFFFu;
}

/**
 * Reads the occupied slots of the group holding `start`, dropping the ones before `start`, and walks on to the next groups while that
 * comes up empty. Groups are k_group_width slots wide and aligned to their width.
 * @param control_bytes First control byte of the table.
 * @param start Slot to begin at.
 * @param capacity Number of slots in the table, which is also the index of the sentinel.
 * @param base Receives the first slot of the group the mask describes.
 * @return Mask with a bit set per occupied slot of the group at `base`, or 0 when no slot from `start` on holds an element.
 */
[[nodiscard]] inline u32 ReadIterationGroup(const i8* control_bytes, u64 start, u64 capacity, u64& base)
{
    constexpr u64 k_group_mask = k_group_width - 1;
    if (start >= capacity)
    {
        return 0;
    }
    base = start & ~k_group_mask;
    u32 keep = 0xFFFFu << (start - base);
    while (base < capacity)
    {
        u32 mask = GetGroupFullMask(control_bytes + base) & keep;
        // The last group runs past the sentinel into the cloned bytes, which mirror the start of the table.
        if (capacity - base < k_group_width)
        {
            mask &= (1u << (capacity - base)) - 1;
        }
        if (mask != 0)
        {
            return mask;
        }
        base += k_group_width;
        keep = 0xFFFFu;
    }
    return 0;
}

/**
 * Finds the next occupied slot for an iteration. Each group of control bytes is read once per iteration, when the mask runs out.
 * Forced inline because MSVC otherwise leaves it out of line in the iterator, and a call per element costs more than the search.
 * @param control_bytes First control byte of the table.
 * @param index Slot the iteration is on. The search starts at the slot after it, so ~u64{} starts it at the first slot.
 * @param mask Occupied slots after `index` in its group, one bit per slot of the group, or 0 when the group has not been read yet.
 * Comes back describing the slots after the one returned.
 * @param capacity Number of slots in the table, which is also the index of the sentinel.
 * @return Index of the next occupied slot, or ~u64{} when there is none.
 */
[[nodiscard]] OPAL_FORCE_INLINE u64 FindNextFull(const i8* control_bytes, u64 index, u32& mask, u64 capacity)
{
    u64 base = index & ~(k_group_width - 1);
    while (true)
    {
        if (mask == 0)
        {
            // index + 1 wraps to 0 when starting from ~u64{}.
            mask = ReadIterationGroup(control_bytes, index + 1, capacity, base);
            if (mask == 0)
            {
                return ~u64{};
            }
        }
        const u64 found = base + CountTrailingZeros(mask);
        mask &= mask - 1;
        // The slot can have been erased since its group was read.
        if (IsControlFull(control_bytes[found]))
        {
            return found;
        }
        index = found;
    }
}

/**
 * Writes a control byte, and its copy in the run repeated after the end of the table so that a group read never runs off it.
 * @param index Slot the control byte belongs to.
 * @param control Value to write.
 * @param control_bytes First control byte of the table.
 * @param capacity Number of slots in the table, which is also the index mask.
 */
inline void SetControlByte(u64 index, i8 control, i8* control_bytes, u64 capacity)
{
    constexpr u64 k_cloned_bytes_count = k_group_width - 1;
    control_bytes[index] = control;
    // For an index inside the first k_cloned_bytes_count the subtraction wraps, which lands on the copy at the end. For any other index
    // it lands back on the byte just written.
    control_bytes[((index - k_cloned_bytes_count) & capacity) + (k_cloned_bytes_count & capacity)] = control;
}

}  // namespace Opal::Impl

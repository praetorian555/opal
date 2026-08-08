#pragma once

#include <atomic>

#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

/**
 * How an atomic operation orders the memory accesses around it.
 *
 * Relaxed orders nothing but the operation itself. Acquire on a read makes every write the matching release was ordered
 * before visible to this thread. Release on a write publishes everything this thread did before it. AcqRel is both, for
 * read-modify-write operations. SeqCst adds a single total order across all SeqCst operations in the program.
 */
enum class MemoryOrder : u8
{
    Relaxed,
    Acquire,
    Release,
    AcqRel,
    SeqCst
};

namespace Impl
{

constexpr std::memory_order ToStdMemoryOrder(MemoryOrder order)
{
    switch (order)
    {
        case MemoryOrder::Relaxed:
            return std::memory_order_relaxed;
        case MemoryOrder::Acquire:
            return std::memory_order_acquire;
        case MemoryOrder::Release:
            return std::memory_order_release;
        case MemoryOrder::AcqRel:
            return std::memory_order_acq_rel;
        case MemoryOrder::SeqCst:
            break;
    }
    return std::memory_order_seq_cst;
}

}  // namespace Impl

/**
 * A value that several threads may read and write at once without tearing or a data race.
 *
 * The memory order is a template argument rather than a function argument so that it is a constant in every build,
 * including an unoptimised one.
 * @tparam T Type of the value. Must be trivially copyable and no wider than a machine word.
 */
template <typename T>
    requires IsPOD<T> && (sizeof(T) <= 8)
class Atomic
{
public:
    using value_type = T;

    /** True if operations on this type are lock free on every platform the library supports. */
    static constexpr bool k_is_always_lock_free = std::atomic<T>::is_always_lock_free;

    Atomic() = default;
    constexpr Atomic(T value) : m_value(value) {}

    Atomic(const Atomic&) = delete;
    Atomic& operator=(const Atomic&) = delete;
    Atomic(Atomic&&) = delete;
    Atomic& operator=(Atomic&&) = delete;

    ~Atomic() = default;

    /**
     * Read the value.
     * @tparam k_order Order to read with. Release and AcqRel do not apply to a read.
     * @return The value at the moment of the read.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    T Load() const
    {
        static_assert(k_order != MemoryOrder::Release && k_order != MemoryOrder::AcqRel, "A load cannot release");
        return m_value.load(Impl::ToStdMemoryOrder(k_order));
    }

    /**
     * Write the value.
     * @tparam k_order Order to write with. Acquire and AcqRel do not apply to a write.
     * @param value The value to write.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    void Store(T value)
    {
        static_assert(k_order != MemoryOrder::Acquire && k_order != MemoryOrder::AcqRel, "A store cannot acquire");
        m_value.store(value, Impl::ToStdMemoryOrder(k_order));
    }

    /**
     * Write the value and read what was there before, in one step.
     * @param value The value to write.
     * @return The value that was there before.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    T Exchange(T value)
    {
        return m_value.exchange(value, Impl::ToStdMemoryOrder(k_order));
    }

    /**
     * Write desired only if the current value is expected, in one step.
     * @tparam k_success Order to apply when the write happens.
     * @tparam k_failure Order to apply when it does not. Cannot release.
     * @param in_out_expected The value to compare against. On failure it is overwritten with what was actually there.
     * @param desired The value to write.
     * @return True if the write happened.
     */
    template <MemoryOrder k_success = MemoryOrder::SeqCst, MemoryOrder k_failure = MemoryOrder::SeqCst>
    bool CompareExchangeStrong(T& in_out_expected, T desired)
    {
        static_assert(k_failure != MemoryOrder::Release && k_failure != MemoryOrder::AcqRel, "A failed compare exchange cannot release");
        return m_value.compare_exchange_strong(in_out_expected, desired, Impl::ToStdMemoryOrder(k_success), Impl::ToStdMemoryOrder(k_failure));
    }

    /**
     * Write desired only if the current value is expected, in one step. May fail even when the values match, so it is
     * only useful inside a loop, where it is cheaper than CompareExchangeStrong on some platforms.
     * @tparam k_success Order to apply when the write happens.
     * @tparam k_failure Order to apply when it does not. Cannot release.
     * @param in_out_expected The value to compare against. On failure it is overwritten with what was actually there.
     * @param desired The value to write.
     * @return True if the write happened.
     */
    template <MemoryOrder k_success = MemoryOrder::SeqCst, MemoryOrder k_failure = MemoryOrder::SeqCst>
    bool CompareExchangeWeak(T& in_out_expected, T desired)
    {
        static_assert(k_failure != MemoryOrder::Release && k_failure != MemoryOrder::AcqRel, "A failed compare exchange cannot release");
        return m_value.compare_exchange_weak(in_out_expected, desired, Impl::ToStdMemoryOrder(k_success), Impl::ToStdMemoryOrder(k_failure));
    }

    /**
     * Add to the value and read what was there before, in one step.
     * @param operand The amount to add.
     * @return The value that was there before.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    T FetchAdd(T operand)
        requires Integral<T>
    {
        return m_value.fetch_add(operand, Impl::ToStdMemoryOrder(k_order));
    }

    /**
     * Subtract from the value and read what was there before, in one step.
     * @param operand The amount to subtract.
     * @return The value that was there before.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    T FetchSub(T operand)
        requires Integral<T>
    {
        return m_value.fetch_sub(operand, Impl::ToStdMemoryOrder(k_order));
    }

    /**
     * Block until the value differs from old_value, then return. May also return without the value having changed, so
     * callers must re-check the condition in a loop.
     * @tparam k_order Order to read with. Release and AcqRel do not apply to a read.
     * @param old_value The value to wait away from.
     */
    template <MemoryOrder k_order = MemoryOrder::SeqCst>
    void Wait(T old_value) const
    {
        static_assert(k_order != MemoryOrder::Release && k_order != MemoryOrder::AcqRel, "A wait cannot release");
        m_value.wait(old_value, Impl::ToStdMemoryOrder(k_order));
    }

    /** Wake one thread waiting on this value. */
    void NotifyOne() { m_value.notify_one(); }

    /** Wake every thread waiting on this value. */
    void NotifyAll() { m_value.notify_all(); }

    /**
     * Address of the underlying value, for handing to a platform wait primitive such as futex or WaitOnAddress.
     * @return Address of the value.
     * @note Only for that purpose. Reading or writing through this pointer is a data race, since it bypasses every
     *       guarantee this type exists to provide.
     */
    T* GetStorageAddress()
    {
        static_assert(sizeof(std::atomic<T>) == sizeof(T), "Atomic storage is not the value it holds");
        static_assert(alignof(std::atomic<T>) >= alignof(T), "Atomic storage is underaligned for the value it holds");
        return reinterpret_cast<T*>(&m_value);
    }

private:
    std::atomic<T> m_value;
};

}  // namespace Opal

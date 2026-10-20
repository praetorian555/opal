#include "opal/threading/signal.h"

#include <cmath>

#include "opal/time.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <errno.h>
#include <limits.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

Opal::Signal::Signal() = default;

Opal::Signal::~Signal() = default;

Opal::u32 Opal::Signal::GetState() const
{
    return m_state.Load<MemoryOrder::Acquire>();
}

void Opal::Signal::Wait(u32 expected_state)
{
    while (m_state.Load<MemoryOrder::Acquire>() == expected_state)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        u32 compare_value = expected_state;
        WaitOnAddress(m_state.GetStorageAddress(), &compare_value, sizeof(u32), INFINITE);
#elif defined(OPAL_PLATFORM_LINUX)
        syscall(SYS_futex, m_state.GetStorageAddress(), FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected_state, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
    }
}

bool Opal::Signal::WaitFor(u32 expected_state, u64 timeout_ms)
{
    const f64 deadline_ms = GetMilliSeconds() + static_cast<f64>(timeout_ms);
    while (m_state.Load<MemoryOrder::Acquire>() == expected_state)
    {
        const f64 remaining_ms = deadline_ms - GetMilliSeconds();
        if (remaining_ms <= 0)
        {
            return false;
        }
#if defined(OPAL_PLATFORM_WINDOWS)
        constexpr f64 k_longest_wait_ms = static_cast<f64>(INFINITE - 1);
        const f64 wait_ms = std::ceil(remaining_ms < k_longest_wait_ms ? remaining_ms : k_longest_wait_ms);
        u32 compare_value = expected_state;
        WaitOnAddress(m_state.GetStorageAddress(), &compare_value, sizeof(u32), static_cast<DWORD>(wait_ms));
#elif defined(OPAL_PLATFORM_LINUX)
        const f64 remaining_ns = remaining_ms * 1'000'000.0;
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(remaining_ns / 1'000'000'000.0);
        ts.tv_nsec = static_cast<long>(remaining_ns - static_cast<f64>(ts.tv_sec) * 1'000'000'000.0);
        syscall(SYS_futex, m_state.GetStorageAddress(), FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected_state, &ts, nullptr, 0);
#else
#error "Platform not supported"
#endif
    }
    return true;
}

void Opal::Signal::NotifyOne()
{
    m_state.FetchAdd<MemoryOrder::Release>(1);
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeByAddressSingle(m_state.GetStorageAddress());
#elif defined(OPAL_PLATFORM_LINUX)
    syscall(SYS_futex, m_state.GetStorageAddress(), FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
}

void Opal::Signal::NotifyAll()
{
    m_state.FetchAdd<MemoryOrder::Release>(1);
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeByAddressAll(m_state.GetStorageAddress());
#elif defined(OPAL_PLATFORM_LINUX)
    syscall(SYS_futex, m_state.GetStorageAddress(), FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
}
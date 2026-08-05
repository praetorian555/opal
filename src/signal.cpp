#include "opal/threading/signal.h"

#include "opal/time.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cerrno>
#include <climits>
#endif

Opal::Signal::Signal() = default;

Opal::Signal::~Signal() = default;

Opal::u32 Opal::Signal::GetState() const
{
    return m_state.load(std::memory_order_acquire);
}

void Opal::Signal::Wait(u32 expected_state)
{
    while (m_state.load(std::memory_order_acquire) == expected_state)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        u32 compare_value = expected_state;
        WaitOnAddress(&m_state, &compare_value, sizeof(u32), INFINITE);
#elif defined(OPAL_PLATFORM_LINUX)
        syscall(SYS_futex, &m_state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected_state, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
    }
}

bool Opal::Signal::WaitFor(u32 expected_state, u64 timeout_ms)
{
    const f64 deadline_ms = GetMilliSeconds() + static_cast<f64>(timeout_ms);
    while (m_state.load(std::memory_order_acquire) == expected_state)
    {
        const f64 remaining_ms = deadline_ms - GetMilliSeconds();
        if (remaining_ms <= 0)
        {
            return false;
        }
#if defined(OPAL_PLATFORM_WINDOWS)
        u32 compare_value = expected_state;
        WaitOnAddress(&m_state, &compare_value, sizeof(u32), static_cast<DWORD>(remaining_ms));
#elif defined(OPAL_PLATFORM_LINUX)
        const u64 remaining_ms_int = static_cast<u64>(remaining_ms);
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(remaining_ms_int / 1000);
        ts.tv_nsec = static_cast<long>((remaining_ms_int % 1000) * 1000000);
        syscall(SYS_futex, &m_state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected_state, &ts, nullptr, 0);
#else
#error "Platform not supported"
#endif
    }
    return true;
}

void Opal::Signal::NotifyOne()
{
    m_state.fetch_add(1, std::memory_order_release);
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeByAddressSingle(&m_state);
#elif defined(OPAL_PLATFORM_LINUX)
    syscall(SYS_futex, &m_state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
}

void Opal::Signal::NotifyAll()
{
    m_state.fetch_add(1, std::memory_order_release);
#if defined(OPAL_PLATFORM_WINDOWS)
    WakeByAddressAll(&m_state);
#elif defined(OPAL_PLATFORM_LINUX)
    syscall(SYS_futex, &m_state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, nullptr, nullptr, 0);
#else
#error "Platform not supported"
#endif
}
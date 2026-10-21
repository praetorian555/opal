#include "opal/threading/atomic.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <linux/membarrier.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

Opal::Atomic<bool> Opal::Impl::g_asymmetric_fence_ready = false;

namespace
{
// Every light fence reads the flag, so it is written once rather than on every heavy fence, which would keep pulling its cache
// line away from them.
void MarkReady()
{
    if (!Opal::Impl::g_asymmetric_fence_ready.Load<Opal::MemoryOrder::Relaxed>())
    {
        Opal::Impl::g_asymmetric_fence_ready.Store<Opal::MemoryOrder::Relaxed>(true);
    }
}
}  // namespace

#if defined(OPAL_PLATFORM_WINDOWS)

void Opal::AsymmetricThreadFenceHeavy()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    FlushProcessWriteBuffers();
    MarkReady();
}

#elif defined(OPAL_PLATFORM_LINUX)

namespace
{
long Membarrier(int command)
{
    return syscall(__NR_membarrier, command, 0, 0);
}

// The expedited command needs the process registered once. A kernel or sandbox without it leaves the global command, which is
// slower but needs no registration, and failing that, nothing: the light half then keeps its hardware fence.
enum class Barrier
{
    PrivateExpedited,
    Global,
    None,
};

Barrier ChooseBarrier()
{
    if (Membarrier(MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED) == 0)
    {
        return Barrier::PrivateExpedited;
    }
    const long supported = Membarrier(MEMBARRIER_CMD_QUERY);
    if (supported > 0 && (supported & MEMBARRIER_CMD_GLOBAL) != 0)
    {
        return Barrier::Global;
    }
    return Barrier::None;
}
}  // namespace

void Opal::AsymmetricThreadFenceHeavy()
{
    static const Barrier s_barrier = ChooseBarrier();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    switch (s_barrier)
    {
        case Barrier::PrivateExpedited:
            if (Membarrier(MEMBARRIER_CMD_PRIVATE_EXPEDITED) == 0)
            {
                MarkReady();
            }
            break;
        case Barrier::Global:
            if (Membarrier(MEMBARRIER_CMD_GLOBAL) == 0)
            {
                MarkReady();
            }
            break;
        case Barrier::None:
            break;
    }
}

#else
#error "Platform not supported"
#endif

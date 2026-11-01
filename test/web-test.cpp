#include "test-helpers.h"

#if defined(OPAL_PLATFORM_WEB)

#include "opal/assert.h"
#include "opal/paths.h"
#include "opal/threading/condition-variable.h"
#include "opal/threading/mutex.h"
#include "opal/threading/signal.h"
#include "opal/threading/thread-pool.h"
#include "opal/threading/thread.h"
#include "opal/time.h"

using namespace Opal;

// What the web does in place of what thread-test.cpp checks, which is left out of this build: there is one thread, so nothing can start
// and a wait that would block could never end.

namespace
{
constexpr u64 k_long_timeout_ms = 5000;
constexpr f64 k_at_once_ms = 1000;

void ThrowViolation(const char*)
{
    throw 1;
}

struct ScopedThrowingHandler
{
    ScopedThrowingHandler() { SetContractViolationHandler(&ThrowViolation); }
    ~ScopedThrowingHandler() { SetContractViolationHandler(nullptr); }
};
}  // namespace

TEST_CASE("No thread can start", "[Web]")
{
    Expected<ThreadHandle, ErrorCode> thread = CreateThread([]() {});
    REQUIRE(thread.GetError() == ErrorCode::OSFailure);

    Expected<ScopePtr<ThreadPool>, ErrorCode> pool = ThreadPool::Create(2);
    REQUIRE(pool.GetError() == ErrorCode::OSFailure);
}

TEST_CASE("The page runs on one core", "[Web]")
{
    Expected<CpuInfo, ErrorCode> info = GetCpuInfo();
    REQUIRE(info.HasValue());
    REQUIRE(info.GetValue().logical_cores_count == 1);
    REQUIRE(info.GetValue().physical_processors.GetSize() == 1);
    REQUIRE(PrintCpuInfo() == ErrorCode::Success);
}

TEST_CASE("A signal never blocks", "[Web]")
{
    Signal signal;
    const u32 state = signal.GetState();

    const f64 start = GetMilliSeconds();
    REQUIRE_FALSE(signal.WaitFor(state, k_long_timeout_ms));
    REQUIRE(GetMilliSeconds() - start < k_at_once_ms);

    signal.NotifyOne();
    REQUIRE(signal.WaitFor(state, k_long_timeout_ms));
    signal.Wait(state);
    signal.NotifyAll();
    REQUIRE(signal.GetState() != state);

    const ScopedThrowingHandler handler;
    REQUIRE_THROWS_AS(signal.Wait(signal.GetState()), int);
}

TEST_CASE("A condition variable never blocks", "[Web]")
{
    Mutex<bool> mutex(true);
    ConditionVariable condition;
    condition.NotifyOne();
    condition.NotifyAll();

    auto guard = mutex.Lock();
    const f64 start = GetMilliSeconds();
    REQUIRE_FALSE(condition.WaitFor(guard, k_long_timeout_ms));
    REQUIRE(GetMilliSeconds() - start < k_at_once_ms);

    // A predicate that already holds never reaches the wait.
    REQUIRE(*condition.Wait(guard, [](const bool value) { return value; }));

    const ScopedThrowingHandler handler;
    REQUIRE_THROWS_AS(condition.Wait(guard), int);
}

TEST_CASE("A page has no executable", "[Web]")
{
    REQUIRE(Paths::GetExecutablePath().GetError() == ErrorCode::NotImplemented);
}

#endif

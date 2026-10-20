#pragma once

#include <new>
#include <type_traits>
#include <utility>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/container/expected.h"
#include "opal/container/shared-ptr.h"
#include "opal/defines.h"
#include "opal/error-codes.h"
#include "opal/threading/atomic.h"
#include "opal/threading/executor.h"
#include "opal/threading/signal.h"
#include "opal/time.h"

namespace Opal
{

template <typename T>
class Future;

template <typename T>
class Promise;

/**
 * What a Future<T> yields: an Expected<T, ErrorCode>, or for Future<void> an ErrorCode that is ErrorCode::Success when the promise
 * was kept.
 */
template <typename T>
using FutureResult = std::conditional_t<std::is_void_v<T>, ErrorCode, Expected<T, ErrorCode>>;

namespace Impl
{

template <typename T>
struct IsExpectedOfErrorCode : std::false_type
{
};

template <typename T>
struct IsExpectedOfErrorCode<Expected<T, ErrorCode>> : std::true_type
{
    using type = T;
};

/** Value type of the future that a callable returning R fills: R itself, or T when R is Expected<T, ErrorCode>. */
template <typename R>
using FutureValueOf = typename std::conditional_t<IsExpectedOfErrorCode<R>::value, IsExpectedOfErrorCode<R>, std::type_identity<R>>::type;

template <typename T>
FutureResult<T> MakeFutureError(ErrorCode error)
{
    if constexpr (std::is_void_v<T>)
    {
        return error;
    }
    else
    {
        return Expected<T, ErrorCode>(error);
    }
}

template <typename T>
struct FutureContinuationBase
{
    virtual ~FutureContinuationBase() = default;
    virtual void Run(FutureResult<T>&& result) = 0;
};

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_MSVC_WARNING(4324)
/**
 * State shared by a Promise and its Future: the result once there is one, and what to run when it arrives. It is a Runnable so that
 * a task can be the state of its own future; running a plain promise's state does nothing.
 */
template <typename T>
class FutureState : public Runnable
{
public:
    using ResultType = FutureResult<T>;

    explicit FutureState(AllocatorBase* allocator) : m_allocator(allocator) {}

    ~FutureState() override
    {
        if ((m_flags.Load<MemoryOrder::Acquire>() & k_ready) != 0)
        {
            GetResult().~ResultType();
        }
        Delete(m_allocator, m_continuation);
    }

    FutureState(const FutureState&) = delete;
    FutureState& operator=(const FutureState&) = delete;
    FutureState(FutureState&&) = delete;
    FutureState& operator=(FutureState&&) = delete;

    void Run() override {}

    void Abandon() override
    {
        if (!IsSet())
        {
            SetResult(MakeFutureError<T>(ErrorCode::BrokenPromise));
        }
    }

    /** Store the result, wake waiters and run the continuation if there is one. Returns false if a result was already set. */
    bool SetResult(ResultType&& result)
    {
        if (m_is_claimed.Exchange<MemoryOrder::AcqRel>(true))
        {
            return false;
        }
        new (m_storage) ResultType(std::move(result));
        const u32 previous = m_flags.FetchOr<MemoryOrder::AcqRel>(k_ready);
        m_ready_signal.NotifyAll();
        if ((previous & k_has_continuation) != 0)
        {
            RunContinuation();
        }
        return true;
    }

    /** Take ownership of continuation and run it once the result is set, now if it already is. */
    void SetContinuation(FutureContinuationBase<T>* continuation)
    {
        m_continuation = continuation;
        const u32 previous = m_flags.FetchOr<MemoryOrder::AcqRel>(k_has_continuation);
        if ((previous & k_ready) != 0)
        {
            RunContinuation();
        }
    }

    [[nodiscard]] bool IsReady() const { return (m_flags.Load<MemoryOrder::Acquire>() & k_ready) != 0; }

    [[nodiscard]] bool IsSet() const { return m_is_claimed.Load<MemoryOrder::Acquire>(); }

    void Wait()
    {
        while (!IsReady())
        {
            const u32 signal_state = m_ready_signal.GetState();
            if (IsReady())
            {
                return;
            }
            m_ready_signal.Wait(signal_state);
        }
    }

    bool WaitFor(u64 timeout_ms)
    {
        const f64 deadline_ms = GetMilliSeconds() + static_cast<f64>(timeout_ms);
        while (!IsReady())
        {
            const u32 signal_state = m_ready_signal.GetState();
            if (IsReady())
            {
                return true;
            }
            const f64 remaining_ms = deadline_ms - GetMilliSeconds();
            if (remaining_ms <= 0)
            {
                return false;
            }
            m_ready_signal.WaitFor(signal_state, static_cast<u64>(remaining_ms) + 1);
        }
        return true;
    }

    /** Move the result out. Only once the state is ready. */
    ResultType TakeResult() { return std::move(GetResult()); }

    [[nodiscard]] AllocatorBase* GetAllocator() const { return m_allocator; }

private:
    static constexpr u32 k_ready = 1;
    static constexpr u32 k_has_continuation = 2;

    ResultType& GetResult() { return *std::launder(reinterpret_cast<ResultType*>(m_storage)); }

    void RunContinuation()
    {
        FutureContinuationBase<T>* continuation = m_continuation;
        m_continuation = nullptr;
        continuation->Run(TakeResult());
        Delete(m_allocator, continuation);
    }

    AllocatorBase* m_allocator = nullptr;
    FutureContinuationBase<T>* m_continuation = nullptr;
    Atomic<bool> m_is_claimed = false;
    Atomic<u32> m_flags = 0;
    Signal m_ready_signal;
    alignas(ResultType) u8 m_storage[sizeof(ResultType)];
};
OPAL_END_DISABLE_WARNINGS

/** Call func with args and turn what it returns into a FutureResult<U>: success for void, the Expected as is, or a plain value. */
template <typename U, typename Func, typename... Args>
FutureResult<U> InvokeForResult(Func& func, Args&&... args)
{
    using R = std::invoke_result_t<Func&, Args...>;
    if constexpr (std::is_void_v<R>)
    {
        func(std::forward<Args>(args)...);
        return ErrorCode::Success;
    }
    else if constexpr (IsExpectedOfErrorCode<R>::value)
    {
        return func(std::forward<Args>(args)...);
    }
    else
    {
        return Expected<U, ErrorCode>(func(std::forward<Args>(args)...));
    }
}

/** A callable and the state of the future for what it returns, in one object. Running it calls the callable and sets the result. */
template <typename Func, typename T>
class TaskState final : public FutureState<T>
{
public:
    TaskState(AllocatorBase* allocator, Func&& func) : FutureState<T>(allocator), m_func(std::move(func)) {}

    void Run() override { this->SetResult(InvokeForResult<T>(m_func)); }

private:
    Func m_func;
};

/** Reaches the parts of Future and Promise that only the threading code may use. */
struct FutureAccess
{
    template <typename T>
    static Future<T> MakeFuture(SharedPtr<FutureState<T>>&& state)
    {
        return Future<T>(std::move(state));
    }

    template <typename T>
    static bool SetResult(Promise<T>& promise, FutureResult<T>&& result)
    {
        return promise.Set(std::move(result));
    }
};

template <typename T, typename Func, typename U>
struct ThenContinuation final : FutureContinuationBase<T>
{
    ThenContinuation(Func&& func, Promise<U>&& promise) : m_func(std::move(func)), m_promise(std::move(promise)) {}

    void Run(FutureResult<T>&& result) override { FutureAccess::SetResult(m_promise, InvokeForResult<U>(m_func, std::move(result))); }

    Func m_func;
    Promise<U> m_promise;
};

/** A continuation's callable, its promise and, once it arrives, its input, waiting to be run on an executor. */
template <typename T, typename Func, typename U>
class PostedContinuation final : public Runnable
{
public:
    using InputType = FutureResult<T>;

    PostedContinuation(Func&& func, Promise<U>&& promise) : m_func(std::move(func)), m_promise(std::move(promise)) {}

    ~PostedContinuation() override
    {
        if (m_has_input)
        {
            GetInput().~InputType();
        }
    }

    PostedContinuation(const PostedContinuation&) = delete;
    PostedContinuation& operator=(const PostedContinuation&) = delete;
    PostedContinuation(PostedContinuation&&) = delete;
    PostedContinuation& operator=(PostedContinuation&&) = delete;

    void SetInput(InputType&& input)
    {
        new (m_input) InputType(std::move(input));
        m_has_input = true;
    }

    void Run() override { FutureAccess::SetResult(m_promise, InvokeForResult<U>(m_func, std::move(GetInput()))); }

    void Abandon() override { m_promise.SetError(ErrorCode::BrokenPromise); }

    void Fail(ErrorCode error) { m_promise.SetError(error); }

private:
    InputType& GetInput() { return *std::launder(reinterpret_cast<InputType*>(m_input)); }

    Func m_func;
    Promise<U> m_promise;
    alignas(InputType) u8 m_input[sizeof(InputType)];
    bool m_has_input = false;
};

/** Hands the result to a PostedContinuation and posts it, so the callable runs on the executor rather than here. */
template <typename T, typename Func, typename U>
struct PostContinuation final : FutureContinuationBase<T>
{
    PostContinuation(Executor& executor, SharedPtr<PostedContinuation<T, Func, U>>&& job) : m_executor(&executor), m_job(std::move(job)) {}

    void Run(FutureResult<T>&& result) override
    {
        m_job->SetInput(std::move(result));
        const ErrorCode posted = m_executor->Post(SharedPtr<Runnable>(m_job.Clone()));
        if (posted != ErrorCode::Success)
        {
            m_job->Fail(posted);
        }
    }

    Executor* m_executor;
    SharedPtr<PostedContinuation<T, Func, U>> m_job;
};

}  // namespace Impl

/**
 * The receiving end of a value that is produced later, possibly on another thread. Filled by the matching Promise.
 *
 * A future has one consumer: it can be moved but not copied, and Get, a successful TryGet and Then each consume it, leaving it
 * invalid.
 * @tparam T Type of the value. Can be void, for work that produces nothing but can still fail. Cannot be ErrorCode.
 */
template <typename T>
class Future
{
public:
    static_assert(!std::is_same_v<T, ErrorCode>, "A Future<ErrorCode> could not tell its value from its error");
    static_assert(!std::is_reference_v<T>, "A Future cannot hold a reference");

    using ResultType = FutureResult<T>;

    /** Creates an invalid future, attached to no promise. */
    Future() = default;
    ~Future() = default;

    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;

    Future(Future&&) noexcept = default;
    Future& operator=(Future&&) noexcept = default;

    /** @return True if this future is attached to a promise and has not been consumed. */
    [[nodiscard]] bool IsValid() const { return m_state.IsValid(); }

    /** @return True if the result has arrived. False for an invalid future. */
    [[nodiscard]] bool IsReady() const { return m_state.IsValid() && m_state->IsReady(); }

    /** Block until the result has arrived. Returns at once for an invalid future. */
    void Wait()
    {
        if (m_state.IsValid())
        {
            m_state->Wait();
        }
    }

    /**
     * Block until the result has arrived or the timeout runs out.
     * @param timeout_ms Longest time to wait, in milliseconds.
     * @return True if the result has arrived. False on timeout, or for an invalid future.
     */
    bool WaitFor(u64 timeout_ms) { return m_state.IsValid() && m_state->WaitFor(timeout_ms); }

    /**
     * Block until the result has arrived and take it. The future is invalid afterwards.
     * @return The value or the error the promise was given, ErrorCode::BrokenPromise if the promise was destroyed without either,
     *         or ErrorCode::InvalidArgument for an invalid future.
     */
    ResultType Get()
    {
        if (!m_state.IsValid())
        {
            return Impl::MakeFutureError<T>(ErrorCode::InvalidArgument);
        }
        m_state->Wait();
        return Take();
    }

    /**
     * Take the result if it has arrived, without blocking. The future is invalid afterwards, unless the result had not arrived.
     * @return What Get would, or ErrorCode::WouldBlock if the result has not arrived yet.
     */
    ResultType TryGet()
    {
        if (!m_state.IsValid())
        {
            return Impl::MakeFutureError<T>(ErrorCode::InvalidArgument);
        }
        if (!m_state->IsReady())
        {
            return Impl::MakeFutureError<T>(ErrorCode::WouldBlock);
        }
        return Take();
    }

    /**
     * Run func on the result once it arrives, and get a future for what func returns.
     *
     * func runs on the thread that sets the result, or on the calling thread if the result has already arrived. It takes the
     * ResultType by rvalue, errors included, and returns a plain value, void, or an Expected<U, ErrorCode>, giving a Future of the
     * value, a Future<void>, or a Future<U>.
     * @param func Callable to run on the result.
     * @return Future for what func returns, and this future is invalid afterwards. An invalid future, with this one left as it was,
     *         if this future is invalid or the continuation could not be allocated.
     */
    template <typename Func>
    auto Then(Func&& func)
    {
        using FuncType = std::decay_t<Func>;
        using R = std::invoke_result_t<FuncType&, ResultType&&>;
        using U = Impl::FutureValueOf<R>;
        using Continuation = Impl::ThenContinuation<T, FuncType, U>;

        if (!m_state.IsValid())
        {
            return Future<U>{};
        }
        AllocatorBase* allocator = m_state->GetAllocator();
        Expected<Promise<U>, ErrorCode> promise = Promise<U>::Create(allocator);
        if (!promise.HasValue())
        {
            return Future<U>{};
        }
        Future<U> next = promise.GetValue().GetFuture();
        Continuation* continuation = New<Continuation>(allocator, FuncType(std::forward<Func>(func)), std::move(promise.GetValue()));
        if (continuation == nullptr)
        {
            return Future<U>{};
        }
        SharedPtr<Impl::FutureState<T>> state = std::move(m_state);
        state->SetContinuation(continuation);
        return next;
    }

    /**
     * Run func on the result once it arrives, on the given executor, and get a future for what func returns.
     *
     * Like Then(func), except that func never runs on the thread that sets the result: it is posted to executor, even when the
     * result has already arrived. Use it to finish work on a TaskQueue drained by a UI thread.
     * @param executor Where func runs. Must outlive the result's arrival.
     * @param func Callable to run on the result, as for Then(func).
     * @return Future for what func returns, and this future is invalid afterwards. It reports what Post reported if the executor
     *         did not accept the work. An invalid future, with this one left as it was, if this future is invalid or the
     *         continuation could not be allocated.
     */
    template <typename Func>
    auto Then(Executor& executor, Func&& func)
    {
        using FuncType = std::decay_t<Func>;
        using R = std::invoke_result_t<FuncType&, ResultType&&>;
        using U = Impl::FutureValueOf<R>;
        using Job = Impl::PostedContinuation<T, FuncType, U>;
        using Continuation = Impl::PostContinuation<T, FuncType, U>;

        if (!m_state.IsValid())
        {
            return Future<U>{};
        }
        AllocatorBase* allocator = m_state->GetAllocator();
        Expected<Promise<U>, ErrorCode> promise = Promise<U>::Create(allocator);
        if (!promise.HasValue())
        {
            return Future<U>{};
        }
        Future<U> next = promise.GetValue().GetFuture();
        SharedPtr<Job> job(allocator, FuncType(std::forward<Func>(func)), std::move(promise.GetValue()));
        if (!job.IsValid())
        {
            return Future<U>{};
        }
        Continuation* continuation = New<Continuation>(allocator, executor, std::move(job));
        if (continuation == nullptr)
        {
            return Future<U>{};
        }
        SharedPtr<Impl::FutureState<T>> state = std::move(m_state);
        state->SetContinuation(continuation);
        return next;
    }

private:
    friend class Promise<T>;
    friend struct Impl::FutureAccess;

    explicit Future(SharedPtr<Impl::FutureState<T>>&& state) : m_state(std::move(state)) {}

    ResultType Take()
    {
        SharedPtr<Impl::FutureState<T>> state = std::move(m_state);
        return state->TakeResult();
    }

    SharedPtr<Impl::FutureState<T>> m_state;
};

/**
 * The producing end of a Future: set its value or its error once, from any thread. A promise destroyed without either sets
 * ErrorCode::BrokenPromise, so its future never waits forever.
 * @tparam T Type of the value. Can be void. Cannot be ErrorCode.
 */
template <typename T>
class Promise
{
public:
    static_assert(!std::is_same_v<T, ErrorCode>, "A Promise<ErrorCode> could not tell its value from its error");

    /** Creates an invalid promise, with no future. */
    Promise() = default;

    /**
     * Create a promise and the state it shares with its future.
     * @param allocator Allocator for the shared state and for continuations. Must be thread safe. If null, uses the default allocator.
     * @return The promise, ErrorCode::InvalidArgument if the allocator is not thread safe, or ErrorCode::OutOfMemory.
     */
    static Expected<Promise, ErrorCode> Create(AllocatorBase* allocator = nullptr)
    {
        if (allocator == nullptr)
        {
            allocator = GetDefaultAllocator();
        }
        if (!allocator->IsThreadSafe())
        {
            return Expected<Promise, ErrorCode>(ErrorCode::InvalidArgument);
        }
        SharedPtr<Impl::FutureState<T>> state(allocator, allocator);
        if (!state.IsValid())
        {
            return Expected<Promise, ErrorCode>(ErrorCode::OutOfMemory);
        }
        return Expected<Promise, ErrorCode>(Promise(std::move(state)));
    }

    ~Promise() { Break(); }

    Promise(const Promise&) = delete;
    Promise& operator=(const Promise&) = delete;

    Promise(Promise&& other) noexcept : m_state(std::move(other.m_state)), m_is_future_taken(other.m_is_future_taken) {}
    Promise& operator=(Promise&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        Break();
        m_state = std::move(other.m_state);
        m_is_future_taken = other.m_is_future_taken;
        return *this;
    }

    [[nodiscard]] bool IsValid() const { return m_state.IsValid(); }

    /**
     * Get the future this promise fills. Only once.
     * @return The future, or an invalid future if this promise is invalid or its future was already taken.
     */
    Future<T> GetFuture()
    {
        OPAL_ASSERT(!m_is_future_taken, "The future of a promise can be taken only once");
        if (!m_state.IsValid() || m_is_future_taken)
        {
            return Future<T>{};
        }
        m_is_future_taken = true;
        return Future<T>(m_state.Clone());
    }

    /**
     * Set the value.
     * @return True if this set the result. False if a result was already set or this promise is invalid.
     */
    template <typename U = T>
        requires(!std::is_void_v<U>)
    bool SetValue(std::type_identity_t<U> value)
    {
        return Set(Expected<T, ErrorCode>(std::move(value)));
    }

    /**
     * Mark the work done, for a Promise<void>.
     * @return True if this set the result. False if a result was already set or this promise is invalid.
     */
    template <typename U = T>
        requires std::is_void_v<U>
    bool SetValue()
    {
        return Set(ErrorCode::Success);
    }

    /**
     * Set the value or the error that result holds.
     * @return True if this set the result. False if a result was already set or this promise is invalid.
     */
    template <typename U = T>
        requires(!std::is_void_v<U>)
    bool SetResult(std::type_identity_t<Expected<U, ErrorCode>> result)
    {
        return Set(std::move(result));
    }

    /**
     * Set an error instead of a value. ErrorCode::Success is a value for a Promise<void>, and not allowed otherwise.
     * @return True if this set the result. False if a result was already set or this promise is invalid.
     */
    bool SetError(ErrorCode error)
    {
        OPAL_ASSERT(std::is_void_v<T> || error != ErrorCode::Success, "ErrorCode::Success is not an error");
        return Set(Impl::MakeFutureError<T>(error));
    }

private:
    friend struct Impl::FutureAccess;

    explicit Promise(SharedPtr<Impl::FutureState<T>>&& state) : m_state(std::move(state)) {}

    bool Set(FutureResult<T>&& result) { return m_state.IsValid() && m_state->SetResult(std::move(result)); }

    void Break()
    {
        if (m_state.IsValid() && !m_state->IsSet())
        {
            m_state->SetResult(Impl::MakeFutureError<T>(ErrorCode::BrokenPromise));
        }
        m_state.Reset();
    }

    SharedPtr<Impl::FutureState<T>> m_state;
    bool m_is_future_taken = false;
};

}  // namespace Opal

template <typename Function>
auto Opal::Executor::Submit(Function function)
{
    using ValueType = Impl::FutureValueOf<std::invoke_result_t<Function&>>;
    using ReturnType = Expected<Future<ValueType>, ErrorCode>;
    using State = Impl::TaskState<Function, ValueType>;

    AllocatorBase* allocator = GetAllocator();
    SharedPtr<State> state(allocator, allocator, std::move(function));
    if (!state.IsValid())
    {
        return ReturnType(ErrorCode::OutOfMemory);
    }
    Future<ValueType> future = Impl::FutureAccess::MakeFuture(SharedPtr<Impl::FutureState<ValueType>>(state.Clone()));
    const ErrorCode posted = Post(SharedPtr<Runnable>(std::move(state)));
    if (posted != ErrorCode::Success)
    {
        return ReturnType(posted);
    }
    return ReturnType(std::move(future));
}

#pragma once

#include "opal/allocator.h"
#include "opal/container/shared-ptr.h"
#include "opal/error-codes.h"

namespace Opal
{

/** A piece of work an Executor can run. */
class Runnable
{
public:
    virtual ~Runnable() = default;

    /** Do the work. */
    virtual void Run() = 0;

    /**
     * Called instead of Run by an executor that drops this work without running it, so that whoever waits on it hears so. Work
     * submitted through Executor::Submit reports ErrorCode::BrokenPromise to its future.
     */
    virtual void Abandon() {}
};

/**
 * Something that runs work handed to it: a ThreadPool on its workers, a TaskQueue on the thread that drains it, or a custom
 * implementation. Submit hands over a callable and returns a Future for what it returns; Future::Then can name an executor to run
 * a continuation on.
 */
class Executor
{
public:
    virtual ~Executor() = default;

    /**
     * Queue work to run. Safe to call from any thread. Work accepted here must later be either run or, if the executor drops it,
     * abandoned with Runnable::Abandon.
     * @param work The work. Left untouched when it is not accepted.
     * @return ErrorCode::Success, ErrorCode::ChannelClosed if the executor no longer accepts work, ErrorCode::InvalidArgument for
     *         an invalid pointer, or another code an implementation documents.
     */
    virtual ErrorCode Post(SharedPtr<Runnable>&& work) = 0;

    /** @return Thread-safe allocator for the work and futures that Submit creates. */
    [[nodiscard]] virtual AllocatorBase* GetAllocator() const = 0;

    /**
     * Run a callable on this executor and get a future for what it returns.
     * @param function Callable taking no arguments. It may return nothing, giving a Future<void>; a plain value of type R, giving a
     *                 Future<R>; or an Expected<R, ErrorCode>, giving a Future<R> that carries the error too.
     * @return The future, ErrorCode::InvalidArgument if GetAllocator is not thread safe, ErrorCode::OutOfMemory if the task could
     *         not be allocated, or what Post reported.
     */
    template <typename Function>
    auto Submit(Function function);
};

}  // namespace Opal

// Submit is defined with Future, which needs the declarations above.
#include "opal/threading/future.h"

#pragma once

#include <new>

#include "opal/assert.h"
#include "opal/type-traits.h"
#include "opal/types.h"

namespace Opal
{

namespace Impl
{

template <typename ReturnType, typename... Args>
struct FunctionVTable
{
    ReturnType (*invoke)(void* storage, Args... args) = nullptr;
    void (*destroy)(void* storage) = nullptr;
    void (*move_construct)(void* destination, void* source) = nullptr;
    void (*copy_construct)(void* destination, const void* source) = nullptr;
};

template <typename Callable, typename ReturnType, typename... Args>
ReturnType FunctionInvoke(void* storage, Args... args)
{
    return (*static_cast<Callable*>(storage))(Forward<Args>(args)...);
}

template <typename Callable>
void FunctionDestroy(void* storage)
{
    static_cast<Callable*>(storage)->~Callable();
}

template <typename Callable>
void FunctionMoveConstruct(void* destination, void* source)
{
    new (destination) Callable(Move(*static_cast<Callable*>(source)));
}

template <typename Callable>
void FunctionCopyConstruct(void* destination, const void* source)
{
    new (destination) Callable(*static_cast<const Callable*>(source));
}

template <typename Callable, typename ReturnType, typename... Args>
constexpr FunctionVTable<ReturnType, Args...> MakeFunctionVTable()
{
    FunctionVTable<ReturnType, Args...> table;
    table.invoke = &FunctionInvoke<Callable, ReturnType, Args...>;
    table.destroy = &FunctionDestroy<Callable>;
    table.move_construct = &FunctionMoveConstruct<Callable>;
    if constexpr (CopyConstructable<Callable>)
    {
        table.copy_construct = &FunctionCopyConstruct<Callable>;
    }
    return table;
}

template <typename Callable, typename ReturnType, typename... Args>
inline constexpr FunctionVTable<ReturnType, Args...> k_function_vtable = MakeFunctionVTable<Callable, ReturnType, Args...>();

/** Callable that calls k_method on the instance it holds. */
template <auto k_method, typename Instance, typename ReturnType, typename... Args>
struct MethodCallable
{
    Instance* instance = nullptr;

    ReturnType operator()(Args... args) const { return (instance->*k_method)(Forward<Args>(args)...); }
};

}  // namespace Impl

/** Number of bytes a Function reserves for the callable it holds, when the caller does not pick a size. */
inline constexpr u64 k_default_function_capacity = 32;

template <typename Signature, u64 k_capacity = k_default_function_capacity>
class Function;

/**
 * Holds any callable with signature ReturnType(Args...) in storage of its own, so calling one costs no allocation and
 * no indirection through the heap.
 *
 * The callable must fit in k_capacity bytes. One that does not is a compile error, not a hidden allocation: raise
 * k_capacity, or hold a pointer to an object the caller owns.
 * @tparam Signature Signature of the callable, written as ReturnType(Args...).
 * @tparam k_capacity Number of bytes reserved for the callable.
 */
template <typename ReturnType, typename... Args, u64 k_capacity>
class Function<ReturnType(Args...), k_capacity>
{
public:
    using VTable = Impl::FunctionVTable<ReturnType, Args...>;

    /** Number of bytes available to the callable. */
    static constexpr u64 k_storage_size = k_capacity;

    /** Strictest alignment a callable may ask for. */
    static constexpr u64 k_storage_alignment = alignof(void*) * 2;

    /** Construct an unbound Function. Calling it is a contract violation. */
    Function() = default;

    /**
     * Construct a Function holding the given callable.
     * @tparam Callable Type of the callable. Must fit in k_capacity bytes and need no more than k_storage_alignment.
     * @param callable The callable to hold.
     */
    template <typename Callable>
        requires(!SameAs<typename Decay<Callable>::Type, Function>)
    Function(Callable&& callable)
    {
        using Decayed = typename Decay<Callable>::Type;
        static_assert(sizeof(Decayed) <= k_capacity,
                      "Callable does not fit in Function storage. Raise k_capacity, or hold a pointer to an object you own.");
        static_assert(alignof(Decayed) <= k_storage_alignment, "Callable needs stricter alignment than Function storage provides.");

        new (m_storage) Decayed(Forward<Callable>(callable));
        m_vtable = &Impl::k_function_vtable<Decayed, ReturnType, Args...>;
    }

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    Function(Function&& other) noexcept;
    Function& operator=(Function&& other) noexcept;

    ~Function();

    /**
     * Copy this Function and the callable it holds.
     * @return A Function holding a copy of the callable.
     * @note A callable that is not copy constructible cannot be cloned, and asking for it ends the program through the
     *       contract violation handler. Cloning an unbound Function yields an unbound Function.
     */
    [[nodiscard]] Function Clone() const;

    /**
     * Build a Function that calls k_method on the given instance. The instance is not owned and must outlive the
     * Function.
     * @tparam k_method Pointer to the member function to call.
     * @param instance The object to call it on.
     * @return A Function bound to that method and instance.
     */
    template <auto k_method, typename Instance>
    [[nodiscard]] static Function FromMethod(Instance* instance);

    /**
     * Call the held callable.
     * @param args Arguments to pass on.
     * @return Whatever the callable returns.
     * @note Calling an unbound Function is a caller mistake, not a runtime outcome: the check runs in every build and
     *       ends the program through the contract violation handler. Check IsBound first.
     */
    ReturnType operator()(Args... args);

    /** @return True if this Function holds a callable. */
    [[nodiscard]] bool IsBound() const { return m_vtable != nullptr; }

    /** Destroy the held callable, leaving the Function unbound. */
    void Reset();

private:
    alignas(k_storage_alignment) u8 m_storage[k_capacity];
    const VTable* m_vtable = nullptr;
};

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

#define TEMPLATE_HEADER template <typename ReturnType, typename... Args, Opal::u64 k_capacity>
#define CLASS_HEADER Opal::Function<ReturnType(Args...), k_capacity>

TEMPLATE_HEADER
CLASS_HEADER::Function(Function&& other) noexcept
{
    if (other.m_vtable != nullptr)
    {
        other.m_vtable->move_construct(m_storage, other.m_storage);
        m_vtable = other.m_vtable;
        other.Reset();
    }
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(Function&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    Reset();
    if (other.m_vtable != nullptr)
    {
        other.m_vtable->move_construct(m_storage, other.m_storage);
        m_vtable = other.m_vtable;
        other.Reset();
    }
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER::~Function()
{
    Reset();
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::Clone() const
{
    Function result;
    if (m_vtable == nullptr)
    {
        return result;
    }
    OPAL_VERIFY(m_vtable->copy_construct != nullptr, "Held callable is not copy constructible");
    m_vtable->copy_construct(result.m_storage, m_storage);
    result.m_vtable = m_vtable;
    return result;
}

TEMPLATE_HEADER
template <auto k_method, typename Instance>
CLASS_HEADER CLASS_HEADER::FromMethod(Instance* instance)
{
    return Function(Impl::MethodCallable<k_method, Instance, ReturnType, Args...>{instance});
}

TEMPLATE_HEADER
ReturnType CLASS_HEADER::operator()(Args... args)
{
    OPAL_VERIFY(m_vtable != nullptr, "Calling an unbound Function");
    return m_vtable->invoke(m_storage, Forward<Args>(args)...);
}

TEMPLATE_HEADER
void CLASS_HEADER::Reset()
{
    if (m_vtable != nullptr)
    {
        m_vtable->destroy(m_storage);
        m_vtable = nullptr;
    }
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

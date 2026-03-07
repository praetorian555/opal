#pragma once

#include <cstddef>

#include "common.h"
#include "exceptions.h"
#include "type-traits.h"

namespace Opal
{

namespace Impl
{

/** Recursive union that provides raw, uninitialized storage for each alternative type. */
template <typename... T>
union VariantStorage
{
};

template <typename T, typename... Rest>
union VariantStorage<T, Rest...>
{
    T head;
    VariantStorage<Rest...> tail;

    VariantStorage() {}
    ~VariantStorage() {}
};

/**
 * Compile-time accessor that retrieves a reference to the value stored at a given
 * index within a VariantStorage.
 */
template <std::size_t Index, typename... Ts>
struct VariantAccessor;

/** Base case: Index == 0, return head. */
template <typename T, typename... Rest>
struct VariantAccessor<0, T, Rest...>
{
    static T& Get(VariantStorage<T, Rest...>& storage)
    {
        return storage.head;
    }
    static const T& Get(const VariantStorage<T, Rest...>& storage)
    {
        return storage.head;
    }
};

/** Recursive case: skip head, decrement index, recurse into tail. */
template <std::size_t Index, typename T, typename... Rest>
struct VariantAccessor<Index, T, Rest...>
{
    static auto& Get(VariantStorage<T, Rest...>& storage)
    {
        return VariantAccessor<Index - 1, Rest...>::Get(storage.tail);
    }
    static const auto& Get(const VariantStorage<T, Rest...>& storage)
    {
        return VariantAccessor<Index - 1, Rest...>::Get(storage.tail);
    }
};

/** Maps a type T to its zero-based index in a type list. Fails to compile if T is not found. */
template <typename T, typename... Rest>
struct TypeIndex
{
    static constexpr std::size_t k_index = 0;
};

template <typename T, typename First, typename... Rest>
struct TypeIndex<T, First, Rest...>
{
    static constexpr std::size_t k_index = Opal::SameAs<T, First> ? 0 : 1 + TypeIndex<T, Rest...>::k_index;
};

}  // namespace Impl

/**
 * A type-safe tagged union that holds exactly one value from the alternative types Ts.
 *
 * The default constructor initializes the first alternative. Copy is deleted;
 * only move semantics are supported. Throws InvalidArgumentException on
 * type-mismatched access.
 *
 * @tparam Ts The alternative types this variant can hold.
 */
template <typename... Ts>
class Variant
{
public:
    /** Default-constructs the first alternative type. */
    Variant()
    {
        auto& slot = Impl::VariantAccessor<0, Ts...>::Get(m_storage);
        using Type = Decay<decltype(slot)>::Type;
        new (&slot) Type();
        m_index = 0;
    }

    /**
     * Constructs the variant holding a value of type T.
     * T (after stripping references and cv-qualifiers) must be one of the alternative types in Ts.
     */
    template <typename T>
    Variant(T&& value)
    {
        using DecayT = Decay<T>::Type;
        constexpr std::size_t k_idx = Impl::TypeIndex<DecayT, Ts...>::k_index;
        auto& slot = Impl::VariantAccessor<k_idx, Ts...>::Get(m_storage);
        new (&slot) DecayT(std::forward<T>(value));
        m_index = k_idx;
    }

    Variant(const Variant& other) = delete;
    Variant& operator=(const Variant& other) = delete;

    /** Move-constructs from another variant, leaving the source in a moved-from state. */
    Variant(Variant&& other) noexcept
    {
        MoveFrom(other);
    }

    /** Move-assigns from another variant, destroying the current value first. */
    Variant& operator=(Variant&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        Destroy();
        MoveFrom(other);
        return *this;
    }

    ~Variant()
    {
        Destroy();
    }

    /** Returns true if the variant currently holds a value of type T. */
    template <typename T>
    [[nodiscard]] bool IsActive() const
    {
        return m_index == Impl::TypeIndex<T, Ts...>::k_index;
    }

    /**
     * Returns a reference to the stored value of type T.
     * @throws InvalidArgumentException if T is not the active alternative.
     */
    template <typename T>
    auto& Get()
    {
        using DecayT = Decay<T>::Type;
        constexpr std::size_t k_idx = Impl::TypeIndex<DecayT, Ts...>::k_index;
        if (k_idx != m_index)
        {
            throw InvalidArgumentException(__FUNCTION__, "Index", static_cast<u64>(k_idx));
        }
        return Impl::VariantAccessor<k_idx, Ts...>::Get(m_storage);
    }

    /**
     * Returns a const reference to the stored value of type T.
     * @throws InvalidArgumentException if T is not the active alternative.
     */
    template <typename T>
    const auto& Get() const
    {
        using DecayT = Decay<T>::Type;
        constexpr std::size_t k_idx = Impl::TypeIndex<DecayT, Ts...>::k_index;
        if (k_idx != m_index)
        {
            throw InvalidArgumentException(__FUNCTION__, "Index", static_cast<u64>(k_idx));
        }
        return Impl::VariantAccessor<k_idx, Ts...>::Get(m_storage);
    }

    /**
     * Returns a reference to the stored value at the compile-time Index.
     * @throws InvalidArgumentException if Index is not the active alternative.
     */
    template <std::size_t Index>
    auto& Get()
    {
        if (Index != m_index)
        {
            throw InvalidArgumentException(__FUNCTION__, "Index", static_cast<u64>(Index));
        }
        return Impl::VariantAccessor<Index, Ts...>::Get(m_storage);
    }

    /**
     * Returns a const reference to the stored value at the compile-time Index.
     * @throws InvalidArgumentException if Index is not the active alternative.
     */
    template <std::size_t Index>
    const auto& Get() const
    {
        
        if (Index != m_index)
        {
            throw InvalidArgumentException(__FUNCTION__, "Index", static_cast<u64>(Index));
        }
        return Impl::VariantAccessor<Index, Ts...>::Get(m_storage);
    }

    /** Returns the zero-based index of the currently active alternative. */
    [[nodiscard]] std::size_t GetIndex() const
    {
        return m_index;
    }

    /**
     * Creates a deep clone of this variant. POD alternatives are copied directly,
     * non-POD alternatives are cloned via their own Clone() method.
     * @param allocator Optional allocator to pass to the Clone() call of non-POD alternatives.
     * @return A new Variant holding a clone of the currently active alternative.
     */
    /**
     * @throws InvalidArgumentException if the variant is in a moved-from state.
     */
    Variant Clone(AllocatorBase* allocator = nullptr) const
    {
        ThrowIfEmpty();
        Variant result;
        result.Destroy();
        result.CloneFrom(*this, allocator);
        return result;
    }

    /**
     * Calls the visitor with the active alternative's value.
     * The visitor must be callable with every alternative type, and all
     * overloads must return the same type.
     * @param visitor A callable (or Overloaded set of callables) to invoke.
     * @return The value returned by the visitor.
     */
    /**
     * @throws InvalidArgumentException if the variant is in a moved-from state.
     */
    template <typename Visitor>
    auto Visit(Visitor&& visitor)
    {
        ThrowIfEmpty();
        return VisitDispatch(std::forward<Visitor>(visitor), std::make_index_sequence<sizeof...(Ts)>{});
    }

    /**
     * @throws InvalidArgumentException if the variant is in a moved-from state.
     */
    template <typename Visitor>
    auto Visit(Visitor&& visitor) const
    {
        ThrowIfEmpty();
        return VisitDispatch(std::forward<Visitor>(visitor), std::make_index_sequence<sizeof...(Ts)>{});
    }

private:
    using Storage = Impl::VariantStorage<Ts...>;

    // Per-alternative operation implementations, instantiated once per index.

    template <std::size_t I>
    static void DestroyAt(Storage& storage)
    {
        auto& slot = Impl::VariantAccessor<I, Ts...>::Get(storage);
        using Type = Decay<decltype(slot)>::Type;
        slot.~Type();
    }

    template <std::size_t I>
    static void MoveAt(Storage& dst, Storage& src)
    {
        auto& src_slot = Impl::VariantAccessor<I, Ts...>::Get(src);
        auto& dst_slot = Impl::VariantAccessor<I, Ts...>::Get(dst);
        using Type = Decay<decltype(dst_slot)>::Type;
        new (&dst_slot) Type(Move(src_slot));
        src_slot.~Type();
    }

    template <std::size_t I>
    static void CloneAt(Storage& dst, const Storage& src, AllocatorBase* allocator)
    {
        const auto& src_slot = Impl::VariantAccessor<I, Ts...>::Get(src);
        auto& dst_slot = Impl::VariantAccessor<I, Ts...>::Get(dst);
        using Type = Decay<decltype(dst_slot)>::Type;
        new (&dst_slot) Type(Opal::Clone(src_slot, allocator));
    }

    // Function pointer tables, built at compile time via index_sequence pack expansion.

    using DestroyFn = void (*)(Storage&);
    using MoveFn = void (*)(Storage&, Storage&);
    using CloneFn = void (*)(Storage&, const Storage&, AllocatorBase*);

    template <typename>
    struct FnTables;

    template <std::size_t... Is>
    struct FnTables<std::index_sequence<Is...>>
    {
        static constexpr DestroyFn k_destroy[] = {&DestroyAt<Is>...};
        static constexpr MoveFn k_move[] = {&MoveAt<Is>...};
        static constexpr CloneFn k_clone[] = {&CloneAt<Is>...};
    };

    using Tables = FnTables<std::make_index_sequence<sizeof...(Ts)>>;

    void Destroy()
    {
        if (m_index < sizeof...(Ts))
        {
            Tables::k_destroy[m_index](m_storage);
            m_index = static_cast<std::size_t>(-1);
        }
    }

    void MoveFrom(Variant& other)
    {
        if (other.m_index < sizeof...(Ts))
        {
            Tables::k_move[other.m_index](m_storage, other.m_storage);
            m_index = other.m_index;
            other.m_index = static_cast<std::size_t>(-1);
        }
    }

    void CloneFrom(const Variant& other, AllocatorBase* allocator)
    {
        if (other.m_index < sizeof...(Ts))
        {
            Tables::k_clone[other.m_index](m_storage, other.m_storage, allocator);
            m_index = other.m_index;
        }
    }

    // Per-alternative visit implementations.

    template <std::size_t Index, typename ReturnType, typename Visitor>
    static ReturnType VisitAt(Visitor&& visitor, Storage& storage)
    {
        return std::forward<Visitor>(visitor)(Impl::VariantAccessor<Index, Ts...>::Get(storage));
    }

    template <std::size_t Index, typename ReturnType, typename Visitor>
    static ReturnType ConstVisitAt(Visitor&& visitor, const Storage& storage)
    {
        return std::forward<Visitor>(visitor)(Impl::VariantAccessor<Index, Ts...>::Get(storage));
    }

    template <typename Visitor, std::size_t... Indices>
    auto VisitDispatch(Visitor&& visitor, std::index_sequence<Indices...>)
    {
        using ReturnType = decltype(std::forward<Visitor>(visitor)(Impl::VariantAccessor<0, Ts...>::Get(m_storage)));
        using VisitFn = ReturnType (*)(Visitor&&, Storage&);
        const VisitFn table[] = {&VisitAt<Indices, ReturnType, Visitor>...};
        return table[m_index](std::forward<Visitor>(visitor), m_storage);
    }

    template <typename Visitor, std::size_t... Indices>
    auto VisitDispatch(Visitor&& visitor, std::index_sequence<Indices...>) const
    {
        using ReturnType = decltype(std::forward<Visitor>(visitor)(Impl::VariantAccessor<0, Ts...>::Get(m_storage)));
        using VisitConstFn = ReturnType (*)(Visitor&&, const Storage&);
        const VisitConstFn table[] = {&ConstVisitAt<Indices, ReturnType, Visitor>...};
        return table[m_index](std::forward<Visitor>(visitor), m_storage);
    }

    void ThrowIfEmpty() const
    {
        if (m_index >= sizeof...(Ts))
        {
            throw InvalidArgumentException(__FUNCTION__, "Variant is in a moved-from state");
        }
    }

    Storage m_storage;
    std::size_t m_index = static_cast<std::size_t>(-1);
};

/**
 * Helper for creating a visitor from multiple lambdas via aggregate initialization.
 * Usage: v.Visit(Overloaded{[](i32 x) { ... }, [](f32 x) { ... }});
 */
template <typename... Visitors>
struct Overloaded : Visitors...
{
    using Visitors::operator()...;
};

template <typename... Visitors>
Overloaded(Visitors...) -> Overloaded<Visitors...>;

}  // namespace Opal

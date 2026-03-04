#pragma once

#include <tuple>

#include "opal/allocator.h"
#include "opal/common.h"

/**
 * Helper macro that generates const and non-const GetFields() methods returning a tuple of references
 * to the listed fields. Used together with ClonableBase to enable automatic deep cloning of a struct or class.
 *
 * Usage:
 * @code
 *   struct MyData : ClonableBase<MyData>
 *   {
 *       StringUtf8 name;
 *       i32 value = 0;
 *
 *       OPAL_CLONE_FIELDS(name, value);
 *   };
 * @endcode
 *
 * @param ... Comma-separated list of member fields to include in the clone operation. Each field type must
 *            either be a POD type or provide its own Clone(AllocatorBase*) method.
 */
#define OPAL_CLONE_FIELDS(...)        \
    auto GetFields() const            \
    {                                 \
        return std::tie(__VA_ARGS__); \
    }                                 \
    auto GetFields()                  \
    {                                 \
        return std::tie(__VA_ARGS__); \
    }

namespace Opal
{

/**
 * CRTP base class that provides automatic deep-clone semantics to derived types while
 * disabling implicit copy construction and copy assignment. Derived classes must use the
 * OPAL_CLONE_FIELDS macro (or manually define GetFields() methods) to list the fields
 * that should be cloned.
 *
 * The derived type must be default-constructible. Clone() works by default-constructing
 * a new instance and then move-assigning each cloned field into it. This means derived
 * classes are free to define their own constructors.
 *
 * The Clone() method iterates over the fields returned by GetFields() and calls
 * Opal::Clone on each one. POD fields are copied directly, while non-POD fields are
 * cloned via their own Clone() method, allowing deep copies of complex object graphs.
 *
 * Usage:
 * @code
 *   // Simple aggregate style.
 *   struct Config : ClonableBase<Config>
 *   {
 *       StringUtf8 name;
 *       i32 id = 0;
 *
 *       OPAL_CLONE_FIELDS(name, id);
 *   };
 *
 *   Config original;
 *   original.name = "hello";
 *   original.id = 42;
 *   Config copy = original.Clone();          // Deep clone with default allocator
 *   Config copy2 = original.Clone(&myAlloc); // Deep clone with custom allocator
 *
 *   // With user-defined constructors.
 *   struct Player : ClonableBase<Player>
 *   {
 *       StringUtf8 name;
 *       i32 score = 0;
 *
 *       Player() = default;
 *       Player(StringUtf8 in_name, i32 in_score) : name(std::move(in_name)), score(in_score) {}
 *
 *       OPAL_CLONE_FIELDS(name, score);
 *   };
 *
 *   Player p("Alice", 100);
 *   Player p2 = p.Clone();
 * @endcode
 *
 * @tparam Derived The concrete type that inherits from this base (CRTP pattern).
 */
template <typename Derived>
class ClonableBase
{
public:
    ClonableBase() = default;
    ClonableBase(const ClonableBase& other) = delete;
    ClonableBase& operator=(const ClonableBase& other) = delete;
    ClonableBase(ClonableBase&& other) noexcept = default;
    ClonableBase& operator=(ClonableBase&& other) noexcept = default;
    ~ClonableBase() = default;

    /**
     * Creates a deep clone of the derived object. A new instance is default-constructed and then
     * each field listed in OPAL_CLONE_FIELDS is cloned and move-assigned into it. POD types are
     * copied directly, and non-POD types are cloned via their own Clone() method.
     * @param allocator Optional allocator to pass to the Clone() call of each non-POD field.
     *                  When nullptr, the default allocator is used.
     * @return A new instance of Derived with all listed fields deep-cloned.
     */
    Derived Clone(AllocatorBase* allocator = nullptr) const
        requires requires(const Derived& d) { d.GetFields(); }
    {
        Derived result;
        auto src = static_cast<const Derived*>(this)->GetFields();
        auto dst = result.GetFields();
        CloneFields(src, dst, allocator, std::make_index_sequence<std::tuple_size_v<decltype(src)>>{});
        return result;
    }

private:
    template <typename SrcTuple, typename DstTuple, size_t... Is>
    static void CloneFields(const SrcTuple& src, DstTuple& dst, AllocatorBase* allocator, std::index_sequence<Is...>)
    {
        ((std::get<Is>(dst) = Opal::Clone(std::get<Is>(src), allocator)), ...);
    }
};

}  // namespace Opal
#pragma once

#include <tuple>

#include "opal/allocator.h"
#include "opal/common.h"

/**
 * Helper macro that generates a GetFields() method returning a tuple of references to the listed fields.
 * Used together with ClonableBase to enable automatic deep cloning of a struct or class.
 *
 * Usage:
 * @code
 *   struct MyData : ClonableBase<MyData>
 *   {
 *       StringUtf8 name;
 *       i32 value;
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
    }

namespace Opal
{

/**
 * CRTP base class that provides automatic deep-clone semantics to derived types while
 * disabling implicit copy construction and copy assignment. Derived classes must use the
 * OPAL_CLONE_FIELDS macro (or manually define a GetFields() method) to list the fields
 * that should be cloned.
 *
 * The Clone() method iterates over the fields returned by GetFields() and calls
 * Opal::Clone on each one. POD fields are copied directly, while non-POD fields are
 * cloned via their own Clone() method, allowing deep copies of complex object graphs.
 *
 * Usage:
 * @code
 *   struct Config : ClonableBase<Config>
 *   {
 *       StringUtf8 name;
 *       i32 id;
 *
 *       Config(StringUtf8 n, i32 i) : name(std::move(n)), id(i) {}
 *
 *       OPAL_CLONE_FIELDS(name, id);
 *   };
 *
 *   Config original("hello", 42);
 *   Config copy = original.Clone();          // Deep clone with default allocator
 *   Config copy2 = original.Clone(&myAlloc); // Deep clone with custom allocator
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
     * Creates a deep clone of the derived object. Each field listed in OPAL_CLONE_FIELDS is cloned
     * individually: POD types are copied, and non-POD types are cloned via their own Clone() method.
     * @param allocator Optional allocator to pass to the Clone() call of each non-POD field.
     *                  When nullptr, the default allocator is used.
     * @return A new instance of Derived with all listed fields deep-cloned.
     */
    Derived Clone(AllocatorBase* allocator = nullptr) const
        requires requires(const Derived& d) { d.GetFields(); }
    {
        return std::apply([allocator](const auto&... fields) { return Derived{Opal::Clone(fields, allocator)...}; },
                          static_cast<const Derived*>(this)->GetFields());
    }
};

}  // namespace Opal
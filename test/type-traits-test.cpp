#include "test-helpers.h"

#include "opal/clonable-base.h"
#include "opal/container/dynamic-array.h"
#include "opal/container/in-place-array.h"
#include "opal/type-traits.h"

using namespace Opal;

namespace
{

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) noexcept = default;
    NonCopyable& operator=(NonCopyable&&) noexcept = default;
    ~NonCopyable() = default;
};

struct NonMovable
{
    NonMovable() = default;
    NonMovable(const NonMovable&) = default;
    NonMovable& operator=(const NonMovable&) = default;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
    ~NonMovable() = default;
};

struct NonDefaultConstructible
{
    explicit NonDefaultConstructible(i32) {}
};

struct NonDestructible
{
    ~NonDestructible() = delete;
};

struct ClonableType : ClonableBase<ClonableType>
{
    i32 value = 0;
    OPAL_CLONE_FIELDS(value);
};

struct NonClonableType
{
    i32 value = 0;
};

struct HasDiffType
{
    using difference_type = i64;
};

struct HasRefType
{
    using reference = i32&;
};

struct HasValType
{
    using value_type = i32;
};

struct NoSubTypes
{
};

struct Comparable
{
    i32 value = 0;
    bool operator==(const Comparable& other) const { return value == other.value; }
    bool operator!=(const Comparable& other) const { return value != other.value; }
};

struct NonComparable
{
    i32 value = 0;
};

struct TrivialType
{
    i32 x;
    f32 y;
};

struct NonTrivialType
{
    NonTrivialType(const NonTrivialType& t) : x(t.x) {}
    i32 x;
};

}  // namespace

// ------------------------------------------------------------------------------------------------
// Reference and const removal utilities.
// ------------------------------------------------------------------------------------------------

TEST_CASE("RemoveReference", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<RemoveReferenceType<i32>, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveReferenceType<i32&>, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveReferenceType<i32&&>, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveReferenceType<const i32&>, const i32>);
}

TEST_CASE("RemoveConstVolatile", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<RemoveConstVolatile<i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveConstVolatile<const i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveConstVolatile<volatile i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveConstVolatile<const volatile i32>::Type, i32>);
}

TEST_CASE("Move", "[TypeTraits]")
{
    i32 value = 42;
    i32&& result = Move(value);
    REQUIRE(result == 42);
}

// ------------------------------------------------------------------------------------------------
// Raw array utilities.
// ------------------------------------------------------------------------------------------------

TEST_CASE("k_is_raw_array", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_raw_array<i32[3]>);
    STATIC_REQUIRE(k_is_raw_array<i32[]>);
    STATIC_REQUIRE_FALSE(k_is_raw_array<i32>);
    STATIC_REQUIRE_FALSE(k_is_raw_array<i32*>);
}

TEST_CASE("RawArray concept", "[TypeTraits]")
{
    STATIC_REQUIRE(RawArray<i32[3]>);
    STATIC_REQUIRE(RawArray<i32[]>);
    STATIC_REQUIRE_FALSE(RawArray<i32>);
    STATIC_REQUIRE_FALSE(RawArray<i32*>);
}

TEST_CASE("GetRawArraySize", "[TypeTraits]")
{
    i32 arr[5] = {};
    REQUIRE(GetRawArraySize(arr) == 5);

    f64 arr2[2] = {};
    REQUIRE(GetRawArraySize(arr2) == 2);
}

TEST_CASE("RemoveExtent", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<RemoveExtent<i32[3]>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<RemoveExtent<i32[]>::Type, i32>);
}

TEST_CASE("Decay", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<Decay<const i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<Decay<volatile i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<Decay<const volatile i32>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<Decay<i32[3]>::Type, i32*>);
    STATIC_REQUIRE(k_is_same_value<Decay<i32[]>::Type, i32*>);
}

// ------------------------------------------------------------------------------------------------
// Compile-time type property checks.
// ------------------------------------------------------------------------------------------------

TEST_CASE("k_is_void_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_void_value<void>);
    STATIC_REQUIRE_FALSE(k_is_void_value<i32>);
    STATIC_REQUIRE_FALSE(k_is_void_value<void*>);
}

TEST_CASE("k_is_pointer_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_pointer_value<i32*>);
    STATIC_REQUIRE(k_is_pointer_value<const i32*>);
    STATIC_REQUIRE(k_is_pointer_value<void*>);
    STATIC_REQUIRE_FALSE(k_is_pointer_value<i32>);
    STATIC_REQUIRE_FALSE(k_is_pointer_value<i32&>);
}

TEST_CASE("k_is_reference_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_reference_value<i32&>);
    STATIC_REQUIRE(k_is_reference_value<i32&&>);
    STATIC_REQUIRE(k_is_reference_value<const i32&>);
    STATIC_REQUIRE_FALSE(k_is_reference_value<i32>);
    STATIC_REQUIRE_FALSE(k_is_reference_value<i32*>);
}

TEST_CASE("k_is_const_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_const_value<const i32>);
    STATIC_REQUIRE(k_is_const_value<const f32>);
    STATIC_REQUIRE_FALSE(k_is_const_value<i32>);
    STATIC_REQUIRE_FALSE(k_is_const_value<const i32*>);  // Pointer to const, not a const type itself.
    STATIC_REQUIRE_FALSE(k_is_const_value<const i32&>);  // Reference to const, not a const type itself.
}

TEST_CASE("k_is_array_literal_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_array_literal_value<i32[3]>);
    STATIC_REQUIRE(k_is_array_literal_value<f32[1]>);
    STATIC_REQUIRE_FALSE(k_is_array_literal_value<i32>);
    STATIC_REQUIRE_FALSE(k_is_array_literal_value<i32*>);
}

TEST_CASE("k_is_convertible_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_convertible_value<i32, f64>);
    STATIC_REQUIRE(k_is_convertible_value<f32, f64>);
    STATIC_REQUIRE(k_is_convertible_value<i32, i64>);
    STATIC_REQUIRE_FALSE(k_is_convertible_value<i32*, i32>);
}

TEST_CASE("k_is_constructible_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_constructible_value<i32, f64>);
    STATIC_REQUIRE(k_is_constructible_value<i32, i32>);
}

// ------------------------------------------------------------------------------------------------
// Convertible and Constructible concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Convertible concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Convertible<i32, f64>);
    STATIC_REQUIRE(Convertible<i32, i64>);
    STATIC_REQUIRE_FALSE(Convertible<i32*, i32>);
}

TEST_CASE("Constructible concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Constructible<i32, i32>);
    STATIC_REQUIRE(Constructible<i32, f64>);
}

// ------------------------------------------------------------------------------------------------
// Conditional type selection.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Conditional", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<Conditional<true, i32, f64>::Type, i32>);
    STATIC_REQUIRE(k_is_same_value<Conditional<false, i32, f64>::Type, f64>);
}

TEST_CASE("ConditionalType", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<ConditionalType<true, i32, f64>, i32>);
    STATIC_REQUIRE(k_is_same_value<ConditionalType<false, i32, f64>, f64>);
}

// ------------------------------------------------------------------------------------------------
// Type identity check.
// ------------------------------------------------------------------------------------------------

TEST_CASE("k_is_same_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<i32, i32>);
    STATIC_REQUIRE(k_is_same_value<const i32, const i32>);
    STATIC_REQUIRE_FALSE(k_is_same_value<i32, f32>);
    STATIC_REQUIRE_FALSE(k_is_same_value<i32, const i32>);
    STATIC_REQUIRE_FALSE(k_is_same_value<i32, i32&>);
}

// ------------------------------------------------------------------------------------------------
// Basic type concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Pointer concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Pointer<i32*>);
    STATIC_REQUIRE(Pointer<void*>);
    STATIC_REQUIRE_FALSE(Pointer<i32>);
    STATIC_REQUIRE_FALSE(Pointer<i32&>);
}

TEST_CASE("Reference concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Reference<i32&>);
    STATIC_REQUIRE(Reference<i32&&>);
    STATIC_REQUIRE_FALSE(Reference<i32>);
    STATIC_REQUIRE_FALSE(Reference<i32*>);
}

TEST_CASE("ArrayLiteral concept", "[TypeTraits]")
{
    STATIC_REQUIRE(ArrayLiteral<i32[3]>);
    STATIC_REQUIRE_FALSE(ArrayLiteral<i32>);
    STATIC_REQUIRE_FALSE(ArrayLiteral<i32*>);
}

TEST_CASE("SameAs concept", "[TypeTraits]")
{
    STATIC_REQUIRE(SameAs<i32, i32>);
    STATIC_REQUIRE_FALSE(SameAs<i32, f32>);
    STATIC_REQUIRE_FALSE(SameAs<i32, const i32>);
}

TEST_CASE("EqualityComparable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(EqualityComparable<i32>);
    STATIC_REQUIRE(EqualityComparable<Comparable>);
    STATIC_REQUIRE_FALSE(EqualityComparable<NonComparable>);
}

// ------------------------------------------------------------------------------------------------
// Construction, assignment, and lifetime concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("DefaultConstructable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(DefaultConstructable<i32>);
    STATIC_REQUIRE(DefaultConstructable<TrivialType>);
    STATIC_REQUIRE_FALSE(DefaultConstructable<NonDefaultConstructible>);
}

TEST_CASE("CopyConstructable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(CopyConstructable<i32>);
    STATIC_REQUIRE(CopyConstructable<TrivialType>);
    STATIC_REQUIRE_FALSE(CopyConstructable<NonCopyable>);
}

TEST_CASE("MoveConstructable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(MoveConstructable<i32>);
    STATIC_REQUIRE(MoveConstructable<NonCopyable>);
    STATIC_REQUIRE_FALSE(MoveConstructable<NonMovable>);
}

TEST_CASE("CopyAssignable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(CopyAssignable<i32>);
    STATIC_REQUIRE(CopyAssignable<TrivialType>);
    STATIC_REQUIRE_FALSE(CopyAssignable<NonCopyable>);
}

TEST_CASE("MoveAssignable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(MoveAssignable<i32>);
    STATIC_REQUIRE(MoveAssignable<NonCopyable>);
    STATIC_REQUIRE_FALSE(MoveAssignable<NonMovable>);
}

TEST_CASE("Clonable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Clonable<ClonableType>);
    STATIC_REQUIRE_FALSE(Clonable<NonClonableType>);
    STATIC_REQUIRE_FALSE(Clonable<i32>);
}

TEST_CASE("Destructible concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Destructible<i32>);
    STATIC_REQUIRE(Destructible<TrivialType>);
    STATIC_REQUIRE(Destructible<NonCopyable>);
    STATIC_REQUIRE_FALSE(Destructible<NonDestructible>);
}

// ------------------------------------------------------------------------------------------------
// Sub-type getters for iterator support.
// ------------------------------------------------------------------------------------------------

TEST_CASE("HasDifferenceSubType concept", "[TypeTraits]")
{
    STATIC_REQUIRE(HasDifferenceSubType<HasDiffType>);
    STATIC_REQUIRE_FALSE(HasDifferenceSubType<NoSubTypes>);
}

TEST_CASE("DifferenceTypeGetter", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<DifferenceTypeGetter<NoSubTypes>::Type, void>);
    STATIC_REQUIRE(k_is_same_value<DifferenceTypeGetter<HasDiffType>::Type, i64>);
    STATIC_REQUIRE(k_is_same_value<DifferenceTypeGetter<i32*>::Type, i64>);
}

TEST_CASE("HasReferenceSubType concept", "[TypeTraits]")
{
    STATIC_REQUIRE(HasReferenceSubType<HasRefType>);
    STATIC_REQUIRE_FALSE(HasReferenceSubType<NoSubTypes>);
}

TEST_CASE("ReferenceTypeGetter", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<ReferenceTypeGetter<NoSubTypes>::Type, void>);
    STATIC_REQUIRE(k_is_same_value<ReferenceTypeGetter<HasRefType>::Type, i32&>);
    STATIC_REQUIRE(k_is_same_value<ReferenceTypeGetter<i32*>::Type, i32&>);
}

TEST_CASE("HasValueSubType concept", "[TypeTraits]")
{
    STATIC_REQUIRE(HasValueSubType<HasValType>);
    STATIC_REQUIRE_FALSE(HasValueSubType<NoSubTypes>);
}

TEST_CASE("ValueTypeGetter", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_same_value<ValueTypeGetter<NoSubTypes>::Type, void>);
    STATIC_REQUIRE(k_is_same_value<ValueTypeGetter<HasValType>::Type, i32>);
}

// ------------------------------------------------------------------------------------------------
// Iterator concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Iterator concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Iterator<i32*>);
    STATIC_REQUIRE(Iterator<DynamicArray<i32>::iterator>);
    STATIC_REQUIRE_FALSE(Iterator<i32>);
}

TEST_CASE("InputIterator concept", "[TypeTraits]")
{
    STATIC_REQUIRE(InputIterator<i32*>);
    STATIC_REQUIRE(InputIterator<DynamicArray<i32>::iterator>);
    STATIC_REQUIRE_FALSE(InputIterator<i32>);
}

TEST_CASE("ForwardIterator concept", "[TypeTraits]")
{
    STATIC_REQUIRE(ForwardIterator<DynamicArray<i32>::iterator>);
    STATIC_REQUIRE_FALSE(ForwardIterator<i32>);
}

TEST_CASE("BidirectionalIterator concept", "[TypeTraits]")
{
    STATIC_REQUIRE(BidirectionalIterator<DynamicArray<i32>::iterator>);
    STATIC_REQUIRE_FALSE(BidirectionalIterator<i32>);
}

TEST_CASE("RandomAccessIterator concept", "[TypeTraits]")
{
    STATIC_REQUIRE(RandomAccessIterator<DynamicArray<i32>::iterator>);
    STATIC_REQUIRE(RandomAccessIterator<InPlaceArray<i32, 3>::iterator>);
    STATIC_REQUIRE_FALSE(RandomAccessIterator<i32>);
}

// ------------------------------------------------------------------------------------------------
// Range concept.
// ------------------------------------------------------------------------------------------------

TEST_CASE("Range concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Range<DynamicArray<i32>>);
    STATIC_REQUIRE(Range<InPlaceArray<i32, 3>>);
    STATIC_REQUIRE_FALSE(Range<i32>);
}

// ------------------------------------------------------------------------------------------------
// Type matching utilities.
// ------------------------------------------------------------------------------------------------

TEST_CASE("k_is_any_value", "[TypeTraits]")
{
    STATIC_REQUIRE(k_is_any_value<i32, i32, f32, f64>);
    STATIC_REQUIRE(k_is_any_value<f64, i32, f32, f64>);
    STATIC_REQUIRE_FALSE(k_is_any_value<u8, i32, f32, f64>);
}

TEST_CASE("AnyOf concept", "[TypeTraits]")
{
    STATIC_REQUIRE(AnyOf<i32, i32, f32, f64>);
    STATIC_REQUIRE(AnyOf<f64, i32, f32, f64>);
    STATIC_REQUIRE_FALSE(AnyOf<u8, i32, f32, f64>);
}

// ------------------------------------------------------------------------------------------------
// Numeric type concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("FloatingPoint concept", "[TypeTraits]")
{
    STATIC_REQUIRE(FloatingPoint<f32>);
    STATIC_REQUIRE(FloatingPoint<f64>);
    STATIC_REQUIRE_FALSE(FloatingPoint<i32>);
    STATIC_REQUIRE_FALSE(FloatingPoint<bool>);
}

TEST_CASE("Integral concept", "[TypeTraits]")
{
    STATIC_REQUIRE(Integral<i8>);
    STATIC_REQUIRE(Integral<i16>);
    STATIC_REQUIRE(Integral<i32>);
    STATIC_REQUIRE(Integral<i64>);
    STATIC_REQUIRE(Integral<u8>);
    STATIC_REQUIRE(Integral<u16>);
    STATIC_REQUIRE(Integral<u32>);
    STATIC_REQUIRE(Integral<u64>);
    STATIC_REQUIRE(Integral<char8>);
    STATIC_REQUIRE(Integral<char16>);
    STATIC_REQUIRE(Integral<uchar32>);
    STATIC_REQUIRE_FALSE(Integral<f32>);
    STATIC_REQUIRE_FALSE(Integral<f64>);
    STATIC_REQUIRE_FALSE(Integral<bool>);
}

TEST_CASE("SignedIntegral concept", "[TypeTraits]")
{
    STATIC_REQUIRE(SignedIntegral<i8>);
    STATIC_REQUIRE(SignedIntegral<i16>);
    STATIC_REQUIRE(SignedIntegral<i32>);
    STATIC_REQUIRE(SignedIntegral<i64>);
    STATIC_REQUIRE(SignedIntegral<char8>);
    STATIC_REQUIRE(SignedIntegral<char16>);
    STATIC_REQUIRE_FALSE(SignedIntegral<u8>);
    STATIC_REQUIRE_FALSE(SignedIntegral<u32>);
    STATIC_REQUIRE_FALSE(SignedIntegral<f32>);
}

TEST_CASE("UnsignedIntegral concept", "[TypeTraits]")
{
    STATIC_REQUIRE(UnsignedIntegral<u8>);
    STATIC_REQUIRE(UnsignedIntegral<u16>);
    STATIC_REQUIRE(UnsignedIntegral<u32>);
    STATIC_REQUIRE(UnsignedIntegral<u64>);
    STATIC_REQUIRE(UnsignedIntegral<uchar32>);
    STATIC_REQUIRE_FALSE(UnsignedIntegral<i32>);
    STATIC_REQUIRE_FALSE(UnsignedIntegral<f32>);
}

TEST_CASE("IntegralOrFloatingPoint concept", "[TypeTraits]")
{
    STATIC_REQUIRE(IntegralOrFloatingPoint<i32>);
    STATIC_REQUIRE(IntegralOrFloatingPoint<f64>);
    STATIC_REQUIRE(IntegralOrFloatingPoint<u8>);
    STATIC_REQUIRE_FALSE(IntegralOrFloatingPoint<bool>);
    STATIC_REQUIRE_FALSE(IntegralOrFloatingPoint<void*>);
}

// ------------------------------------------------------------------------------------------------
// Triviality and layout concepts.
// ------------------------------------------------------------------------------------------------

TEST_CASE("IsTriviallyCopyable concept", "[TypeTraits]")
{
    STATIC_REQUIRE(IsTriviallyCopyable<i32>);
    STATIC_REQUIRE(IsTriviallyCopyable<TrivialType>);
    STATIC_REQUIRE_FALSE(IsTriviallyCopyable<NonTrivialType>);
}

TEST_CASE("IsStandardLayout concept", "[TypeTraits]")
{
    STATIC_REQUIRE(IsStandardLayout<i32>);
    STATIC_REQUIRE(IsStandardLayout<TrivialType>);
    STATIC_REQUIRE(IsStandardLayout<NonTrivialType>);
}

TEST_CASE("IsPOD concept", "[TypeTraits]")
{
    STATIC_REQUIRE(IsPOD<i32>);
    STATIC_REQUIRE(IsPOD<TrivialType>);
    STATIC_REQUIRE_FALSE(IsPOD<NonTrivialType>);
}

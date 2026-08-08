#include "test-helpers.h"

#include "opal/container/dynamic-array.h"
#include "opal/rng.h"
#include "opal/sort/heap-sort.h"
#include "opal/sort/insertion-sort.h"
#include "opal/sort/key-indexed-counting.h"
#include "opal/sort/merge-sort.h"
#include "opal/sort/quick-sort.h"

using namespace Opal;

namespace
{

struct Record
{
    i32 key = 0;
    i32 tag = 0;

    bool operator<(const Record& other) const { return key < other.key; }
};

bool IsSortedAscending(const DynamicArray<i32>& values)
{
    for (u64 i = 1; i < values.GetSize(); ++i)
    {
        if (values.At(i) < values.At(i - 1))
        {
            return false;
        }
    }
    return true;
}

DynamicArray<i32> MakeRandom(u64 count, u64 seed)
{
    RNG rng(seed);
    DynamicArray<i32> values(count);
    for (u64 i = 0; i < count; ++i)
    {
        values.At(i) = rng.RandomI32(-1000, 1000);
    }
    return values;
}

template <typename SortFunction>
void CheckSortsIntegers(SortFunction sort)
{
    {
        DynamicArray<i32> values;
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values.GetSize() == 0);
    }
    {
        DynamicArray<i32> values = {7};
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values == DynamicArray<i32>({7}));
    }
    {
        DynamicArray<i32> values = {1, 2, 3, 4, 5, 6, 7, 8};
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values == DynamicArray<i32>({1, 2, 3, 4, 5, 6, 7, 8}));
    }
    {
        DynamicArray<i32> values = {8, 7, 6, 5, 4, 3, 2, 1};
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values == DynamicArray<i32>({1, 2, 3, 4, 5, 6, 7, 8}));
    }
    {
        DynamicArray<i32> values = {4, 4, 4, 4, 4, 4};
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values == DynamicArray<i32>({4, 4, 4, 4, 4, 4}));
    }
    {
        DynamicArray<i32> values = {3, 1, 2, 3, 1, 2, 3, 1, 2};
        ArrayView<i32> view(values);
        sort(view);
        CHECK(values == DynamicArray<i32>({1, 1, 1, 2, 2, 2, 3, 3, 3}));
    }
    // Larger than the small range cutoffs of the improved variants, so the recursive path is exercised.
    for (const u64 count : {17u, 33u, 64u, 500u})
    {
        DynamicArray<i32> values = MakeRandom(count, count * 7 + 1);
        DynamicArray<i32> expected = values.Clone();
        ArrayView<i32> expected_view(expected);
        HeapSort(expected_view);

        ArrayView<i32> view(values);
        sort(view);
        CHECK(IsSortedAscending(values));
        CHECK(values == expected);
    }
}

template <typename SortFunction>
void CheckSortIsStable(SortFunction sort)
{
    DynamicArray<Record> values = {{2, 0}, {1, 1}, {2, 2}, {1, 3}, {3, 4}, {2, 5}, {1, 6}, {3, 7}, {2, 8}};
    ArrayView<Record> view(values);
    sort(view);

    const i32 expected_tags[] = {1, 3, 6, 0, 2, 5, 8, 4, 7};
    REQUIRE(values.GetSize() == 9);
    for (u64 i = 0; i < values.GetSize(); ++i)
    {
        CHECK(values.At(i).tag == expected_tags[i]);
    }
    for (u64 i = 1; i < values.GetSize(); ++i)
    {
        CHECK(!(values.At(i) < values.At(i - 1)));
    }
}

}  // namespace

TEST_CASE("KeyIndexedCounting", "[sort]")
{
    SECTION("Non-empty array")
    {
        DynamicArray<u64> keys = {2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};
        DynamicArray<u64> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(values == DynamicArray<u64>({2, 5, 8, 0, 3, 6, 9, 1, 4, 7, 10}));
    }
    SECTION("Empty array")
    {
        DynamicArray<u64> keys;
        DynamicArray<u64> values;
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::Success);
        REQUIRE(values == DynamicArray<u64>());
    }
    SECTION("Bad input")
    {
        DynamicArray<u64> keys = {2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};
        DynamicArray<u64> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ArrayView<u64> keys_span(keys);
        ArrayView<u64> values_span(values);

        ErrorCode err = KeyIndexedCounting<4>(keys_span, values_span);
        REQUIRE(err == ErrorCode::InvalidArgument);
    }
}

TEST_CASE("InsertionSort", "[sort]")
{
    SECTION("Sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { InsertionSort(view); }); }
    SECTION("Is stable") { CheckSortIsStable([](ArrayView<Record>& view) { InsertionSort(view); }); }
    SECTION("Improved sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { InsertionSortImproved(view); }); }
}

TEST_CASE("HeapSort", "[sort]")
{
    SECTION("Sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { HeapSort(view); }); }
}

TEST_CASE("QuickSort", "[sort]")
{
    SECTION("Sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { QuickSort(view); }); }
    SECTION("Improved sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { QuickSortImproved(view); }); }
    SECTION("Three way sorts") { CheckSortsIntegers([](ArrayView<i32>& view) { QuickSortThreeWay(view); }); }
    SECTION("Uses the caller's generator")
    {
        RNG rng(42);
        DynamicArray<i32> values = MakeRandom(200, 3);
        ArrayView<i32> view(values);
        QuickSort(view, &rng);
        CHECK(IsSortedAscending(values));
    }
}

TEST_CASE("MergeSort", "[sort]")
{
    SECTION("Sorts")
    {
        CheckSortsIntegers([](ArrayView<i32>& view) { REQUIRE(MergeSort(view) == ErrorCode::Success); });
    }
    SECTION("Is stable")
    {
        CheckSortIsStable([](ArrayView<Record>& view) { REQUIRE(MergeSort(view) == ErrorCode::Success); });
    }
    SECTION("Bottom up sorts")
    {
        CheckSortsIntegers([](ArrayView<i32>& view) { REQUIRE(MergeSortBottomUp(view) == ErrorCode::Success); });
    }
    SECTION("Bottom up is stable")
    {
        CheckSortIsStable([](ArrayView<Record>& view) { REQUIRE(MergeSortBottomUp(view) == ErrorCode::Success); });
    }
    SECTION("Improved sorts")
    {
        CheckSortsIntegers([](ArrayView<i32>& view) { REQUIRE(MergeSortImproved(view) == ErrorCode::Success); });
    }
    SECTION("Reports a failed allocation")
    {
        NullAllocator allocator;
        DynamicArray<i32> values = {5, 4, 3, 2, 1};
        ArrayView<i32> view(values);
        CHECK(MergeSort(view, &allocator) == ErrorCode::OutOfMemory);
        CHECK(MergeSortBottomUp(view, &allocator) == ErrorCode::OutOfMemory);
        CHECK(MergeSortImproved(view, &allocator) == ErrorCode::OutOfMemory);
    }
}

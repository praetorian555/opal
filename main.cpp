#include <cassert>

#include "helpers.h"
#include "insertion-sort.h"
#include "merge-sort.h"
#include "quick-sort.h"

int main()
{
	constexpr int k_times_to_measure = 50;
	std::vector<int> arr = CreateRandomArray<int>(10'000, 5);
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "insertion sort", [&s]()
			{
				InsertionSort(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "insertion sort with improvements", [&s]()
			{
				InsertionSortImproved(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "top down merge sort", [&s]()
			{
				MergeSort(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "bottom up merge sort", [&s]()
			{
				MergeSortBottomUp(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "top down merge sort with improvements", [&s]()
			{
				MergeSortImproved(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "quick sort", [&s]()
			{
				QuickSort(s);
				assert(IsSorted(s));
			});
	}

	return 0;
}

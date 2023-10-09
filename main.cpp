#include <cassert>

#include "helpers.h"
#include "insertion-sort.h"
#include "merge-sort.h"
#include "quick-sort.h"

namespace Test
{
}

void SortRunTimes();

int main()
{
	//{
	//	std::vector<int> arr = CreateRandomArray<int>(10'000, 5);
	//	std::span<int> s{ arr };
	//	Test::InsertionSort(s);
	//	assert(IsSorted(s));
	//}
	//{
	//	std::vector<int> arr = CreateRandomArray<int>(1'000'000, 5);
	//	std::span<int> s{ arr };
	//	Test::MergeSort(s);
	//	assert(IsSorted(s));
	//}

	SortRunTimes();
	return 0;
}

void SortRunTimes()
{
	constexpr int k_times_to_measure = 50;
	const std::vector<int> arr = CreateRandomArray<int>(10'000, 5);
	const std::vector<int> arr_eq = CreateRandomArray<int>(10'000, 5, 1, 3);
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
		Measure(k_times_to_measure, "std::sort", [&s]()
			{
				std::sort(s.begin(), s.end());
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
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "quick sort with improvements", [&s]()
			{
				QuickSortImproved(s);
				assert(IsSorted(s));
			});
	}
	std::cout << "\n";
	{
		std::vector<int> arr_cpy = arr_eq;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "std::sort many equal values", [&s]()
			{
				std::sort(s.begin(), s.end());
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr_eq;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "improved quick sort many equal values", [&s]()
			{
				QuickSortImproved(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr_eq;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "quick sort three way many equal values", [&s]()
			{
				QuickSortThreeWay(s);
				assert(IsSorted(s));
			});
	}
	{
		std::vector<int> arr_cpy = arr_eq;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "top down merge sort with improvements many equal values", [&s]()
			{
				MergeSortImproved(s);
				assert(IsSorted(s));
			});
	}
}

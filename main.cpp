#include <cassert>

#include "helpers.h"
#include "insertion-sort.h"
#include "merge-sort.h"
#include "quick-sort.h"
#include "queue.h"

namespace Test
{
	void Merge(std::span<int>& arr, std::span<int>& aux, int lo, int mid, int hi)
	{
		std::copy(arr.begin() + lo, arr.begin() + hi + 1, aux.begin() + lo);

		int i = lo;
		int j = mid + 1;
		for (int k = lo; k <= hi; k++)
		{
			if (i > mid)
			{
				arr[k] = aux[j++];
			}
			else if (j > hi)
			{
				arr[k] = aux[i++];
			}
			else if (aux[i] < aux[j])
			{
				arr[k] = aux[i++];
			}
			else
			{
				arr[k] = aux[j++];
			}
		}
	}

	void MergeSortBottomUp(std::span<int>& arr)
	{
		std::vector<int> aux(arr.begin(), arr.end());
		std::span<int> aux_s(aux.begin(), aux.end());
		for (int sz = 1; sz < arr.size(); sz *= 2)
		{
			for (int i = 0; i < arr.size(); i += 2 * sz)
			{
				int hi = std::min(i + sz + sz - 1, (int)arr.size() - 1);
				Merge(arr, aux_s, i, i + sz - 1, hi);
			}
		}
	}
}

void SortRunTimes();
void TestQueue();

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
	//{
	//	std::vector<int> arr = CreateRandomArray<int>(1'000'000, 5);
	//	std::span<int> s{ arr };
	//	Test::QuickSort(s);
	//	assert(IsSorted(s));
	//}
	//{
	//	std::vector<int> arr = CreateRandomArray<int>(1024 * 1024, 5);
	//	std::span<int> s{ arr };
	//	Test::MergeSortBottomUp(s);
	//	assert(IsSorted(s));
	//}

	SortRunTimes();

	TestQueue();

	return 0;
}

void SortRunTimes()
{
	constexpr int k_times_to_measure = 50;
	UniformRandomIntGenerator gen(5);
	const std::vector<int> arr = gen.GetRandomIntArray(10'000);
	const std::vector<int> arr_eq = gen.GetRandomIntArray(10'000, 1, 3);
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

void TestQueue()
{
	UniformRandomIntGenerator gen(5);
	constexpr int k_arr_size = 10'000;
	const std::vector<int> arr = gen.GetRandomIntArray(k_arr_size);
	Queue<int> q;
	for (int i = 0; i < arr.size(); i++)
	{
		q.Enqueue(arr[i]);
	}
	for (int i = 0; i < arr.size(); i++)
	{
		assert(arr[i] == q.Dequeue());
	}

	int insert_count = 0;
	int fetch_count = 0;
	while (insert_count != k_arr_size || fetch_count != k_arr_size)
	{
		int random_insert_count = gen.GetRandomInt(10, 20);
		random_insert_count = std::min(random_insert_count, k_arr_size - insert_count);
		for (int i = 0; i < random_insert_count; i++)
		{
			q.Enqueue(arr[insert_count + i]);
		}
		insert_count += random_insert_count;

		int random_fetch_count = gen.GetRandomInt(2, 8);
		random_fetch_count = std::min(random_fetch_count, k_arr_size - fetch_count);
		for (int i = 0; i < random_fetch_count; i++)
		{
			assert(arr[fetch_count + i] == q.Dequeue());
		}
		fetch_count += random_fetch_count;
	}
}

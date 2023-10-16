#include <cassert>

#include "heap-sort.h"
#include "helpers.h"
#include "insertion-sort.h"
#include "merge-sort.h"
#include "quick-sort.h"
#include "heap-sort.h"
#include "queue.h"
#include "stack.h"
#include "priority-queue.h"

namespace Test
{
}

void SortRunTimes();
void TestQueue();
void TestStack();
void TestPQ();

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
	TestStack();

	TestPQ();

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
	{
		std::vector<int> arr_cpy = arr;
		std::span<int> s{ arr_cpy };
		Measure(k_times_to_measure, "heap sort", [&s]()
			{
				HeapSort(s);
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

void TestStack()
{
	UniformRandomIntGenerator gen(5);
	constexpr int k_arr_size = 10'000;
	const std::vector<int> arr = gen.GetRandomIntArray(k_arr_size);
	Stack<int> s;
	for (int i = 0; i < arr.size(); i++)
	{
		s.Push(arr[i]);
	}
	for (int i = arr.size() - 1; i >= 0; i--)
	{
		assert(s.Pop() == arr[i]);
	}
}

void TestPQ()
{
	{
		UniformRandomIntGenerator gen(5);
		constexpr int k_arr_size = 10'000;
		std::vector<int> arr = gen.GetRandomIntArray(k_arr_size);
		PriorityQueue<int> pq;
		for (int i = 0; i < arr.size(); i++)
		{
			pq.Enqueue(arr[i]);
		}
		auto comp = std::greater<int>();
		std::sort(arr.begin(), arr.end(), comp);
		for (int i = 0; i < arr.size(); i++)
		{
			assert(arr[i] == pq.GetTop());
			assert(arr[i] == pq.PopTop());
		}
	}
	{
		UniformRandomIntGenerator gen(5);
		constexpr int k_arr_size = 10'000;
		std::vector<int> arr = gen.GetRandomIntArray(k_arr_size);
		IndexPriorityQueue<int> pq(std::less<int>{}, k_arr_size + 1);
		for (int i = 0; i < arr.size(); i++)
		{
			pq.Enqueue(i, arr[i]);
		}
		auto comp = std::greater<int>();
		std::sort(arr.begin(), arr.end(), comp);
		for (int i = 0; i < arr.size(); i++)
		{
			assert(arr[i] == pq.GetTop());
			assert(arr[i] == pq.PopTop());
		}
	}
	{
		UniformRandomIntGenerator gen(5);
		constexpr int k_arr_size = 10'000;
		std::vector<int> arr = gen.GetRandomIntArray(k_arr_size);
		IndexPriorityQueue<int> pq(std::less<int>{}, k_arr_size + 1);
		for (int i = 0; i < arr.size(); i++)
		{
			pq.Enqueue(i, arr[i]);
		}
		for (int i = 0; i < arr.size(); i++)
		{
			assert(pq.Contains(i));
			assert(arr[i] == pq.GetValueOf(i));
		}
		pq.DeleteValue(10);
		assert(!pq.Contains(10));

		const int old_value = pq.GetValueOf(15);
		pq.ChangeValue(15, 10);
		assert(old_value != pq.GetValueOf(15));
		assert(10 == pq.GetValueOf(15));
	}
}

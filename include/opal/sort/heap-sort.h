#pragma once

#include <span>

namespace HeapSortPrivate
{
	template <typename T>
	bool Compare(std::span<T>& arr, int i, int j)
	{
		return arr[i - 1] < arr[j - 1];
	}

	template <typename T>
	void Exchange(std::span<T>& arr, int i, int j)
	{
		std::swap(arr[i - 1], arr[j - 1]);
	}

	template <typename T>
	void Sink(std::span<T>& arr, int k, int size)
	{
		while (2 * k <= size)
		{
			int j = 2 * k;
			if (j < size && Compare(arr, j, j + 1))
			{
				j++;
			}
			if (!Compare(arr, k, j))
			{
				break;
			}
			Exchange(arr, k, j);
			k = j;
		}
	}
}

template <typename T>
void HeapSort(std::span<T>& arr)
{
	// Building the heap
	int size = arr.size();
	for (int k = size / 2; k >= 1; --k)
	{
		HeapSortPrivate::Sink(arr, k, size);
	}
	// Sort down phase
	int k = size;
	while (k > 1)
	{
		HeapSortPrivate::Exchange(arr, 1, k--);
		HeapSortPrivate::Sink(arr, 1, k);
	}
}

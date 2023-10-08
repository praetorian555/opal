#pragma once

#include <span>

#include "helpers.h"

namespace PrivateQuickSort
{
	template <typename T>
	int Partition(std::span<T>& arr, int lo, int hi)
	{
		int i = lo;
		int j = hi + 1;
		T v = arr[lo];
		while (true)
		{
			while (arr[++i] < v && i != hi) {}
			while (v < arr[--j] && j != lo) {}
			if (i >= j)
			{
				break;
			}
			std::swap(arr[i], arr[j]);
		}
		std::swap(arr[lo], arr[j]);
		return j;
	}

	template <typename T>
	void QuickSort(std::span<T>& arr, int lo, int hi)
	{
		if (hi <= lo)
		{
			return;
		}
		const int j = Partition(arr, lo, hi);
		QuickSort(arr, lo, j - 1);
		QuickSort(arr, j + 1, hi);
	}
}

template <typename T>
void QuickSort(std::span<T>& arr)
{
	Shuffle(arr);
	PrivateQuickSort::QuickSort(arr, 0, arr.size() - 1);
}
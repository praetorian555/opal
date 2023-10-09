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

		template <typename T>
	int PartitionImproved(std::span<T>& arr, int lo, int hi)
	{
		const int mid = lo + (hi - lo) / 2;
		if (arr[hi] < arr[lo])
		{
			std::swap(arr[hi], arr[lo]);
		}
		if (arr[mid] < arr[lo])
		{
			std::swap(arr[mid], arr[lo]);
		}
		if (arr[hi] < arr[mid])
		{
			std::swap(arr[mid], arr[hi]);
		}

		std::swap(arr[mid], arr[lo]);
		int i = lo;
		int j = hi + 1;
		T v = arr[lo];
		while (true)
		{
			while (arr[++i] < v) {}
			while (v < arr[--j]) {}
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
	void QuickSortImproved(std::span<T>& arr, int lo, int hi)
	{
		constexpr int k_small_array_cutoff = 32;
		if (hi - lo <= k_small_array_cutoff)
		{
			std::span<T> s(arr.begin() + lo, arr.begin() + hi + 1);
			InsertionSortImproved(s);
			return;
		}
		const int j = PartitionImproved(arr, lo, hi);
		QuickSortImproved(arr, lo, j - 1);
		QuickSortImproved(arr, j + 1, hi);
	}
}

template <typename T>
void QuickSort(std::span<T>& arr)
{
	Shuffle(arr);
	PrivateQuickSort::QuickSort(arr, 0, arr.size() - 1);
}

template <typename T>
void QuickSortImproved(std::span<T>& arr)
{
	Shuffle(arr);
	PrivateQuickSort::QuickSortImproved(arr, 0, arr.size() - 1);
}

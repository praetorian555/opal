#pragma once

#include <span>
#include <vector>

#include "insertion-sort.h"

namespace PrivateMergeSort
{
	template <typename T>
	void Merge(std::span<T>& dst, std::span<T>& src, int lo, int mid, int hi)
	{
		int i = lo, j = mid + 1;

		// Copy from arr to aux since arr contains sorted values
		for (int k = lo; k <= hi; k++)
		{
			src[k] = dst[k];
		}

		for (int k = lo; k <= hi; k++)
		{
			if (i > mid)
			{
				dst[k] = src[j++];
			}
			else if (j > hi)
			{
				dst[k] = src[i++];
			}
			else if (src[i] < src[j])
			{
				dst[k] = src[i++];
			}
			else
			{
				dst[k] = src[j++];
			}
		}
	}

	template <typename T>
	void MergeSort(std::span<T>& arr, std::span<T>& aux, int lo, int hi)
	{
		if (hi <= lo) return;
		const int mid = lo + (hi - lo) / 2;
		MergeSort(arr, aux, lo, mid);
		MergeSort(arr, aux, mid + 1, hi);
		Merge(arr, aux, lo, mid, hi);
	}

	template <typename T>
	void MergeImproved(std::span<T>& dst, std::span<T>& src, int lo, int mid, int hi)
	{
		int i = lo, j = mid + 1;

		for (int k = lo; k <= hi; k++)
		{
			if (i > mid)
			{
				dst[k] = src[j++];
			}
			else if (j > hi)
			{
				dst[k] = src[i++];
			}
			else if (src[i] < src[j])
			{
				dst[k] = src[i++];
			}
			else
			{
				dst[k] = src[j++];
			}
		}
	}

	template <typename T>
	void MergeSortImproved(std::span<T>& arr, std::span<T>& aux, int lo, int hi)
	{
		constexpr int k_small_array_cutoff = 15;
		if (hi - lo <= k_small_array_cutoff)
		{
			std::span<T> s(arr.begin() + lo, arr.begin() + hi + 1);
			InsertionSortImproved(s);
			return;
		}
		const int mid = lo + (hi - lo) / 2;
		MergeSortImproved(aux, arr, lo, mid);
		MergeSortImproved(aux, arr, mid + 1, hi);
		if (arr[mid] > arr[mid + 1])
		{
			MergeImproved(arr, aux, lo, mid, hi);
		}
	}
}

template <typename T>
void MergeSort(std::span<T>& arr)
{
	std::vector<T> aux(arr.size());
	std::span<T> aux_s(aux);
	PrivateMergeSort::MergeSort(arr, aux_s, 0, arr.size() - 1);
}

template <typename T>
void MergeSortBottomUp(std::span<T>& arr)
{
	std::vector<T> aux(arr.size());
	std::span<T> aux_s(aux);
	for (int sz = 1; sz < arr.size(); sz *= 2)
	{
		for (int lo = 0; lo < arr.size() - sz; lo += sz + sz)
		{
			PrivateMergeSort::Merge(arr, aux_s, lo, lo + sz - 1, std::min(lo + sz + sz - 1, static_cast<int>(arr.size() - 1)));
		}
	}
}

// Improved version of merge sort which:
// 1. Uses insertion sort for small arrays
// 2. Eliminates the copy from arr to aux in the merge function
// 3. Eliminates the merge if the mid element is smaller then mid + 1 element in the array
template <typename T>
void MergeSortImproved(std::span<T>& arr)
{
	std::vector<T> aux(arr.size());
	std::span<T> aux_s(aux);
	PrivateMergeSort::MergeSortImproved(arr, aux_s, 0, arr.size() - 1);
}

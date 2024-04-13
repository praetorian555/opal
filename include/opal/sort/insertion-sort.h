#pragma once

#include <vector>
#include <span>

template <typename T>
void InsertionSort(std::span<T>& arr)
{
    for (int i = 1; i < arr.size(); i++)
    {
        for (int j = i; j > 0 && arr[j - 1] > arr[j]; j--)
        {
            std::swap(arr[j - 1], arr[j]);
        }
    }
}

// This version contains sentinel improvement to eliminate the j > 0 check in
// the inner loop and exchange optimization which caches the current value and
// just moves elements to the right when needed, not doing an exchange every
// time.
template <typename T>
void InsertionSortImproved(std::span<T>& arr)
{
    int min_idx = 0;
    for (int i = 0; i < arr.size(); i++)
    {
        if (arr[i] < arr[min_idx])
        {
            min_idx = i;
        }
    }
    std::swap(arr[min_idx], arr[0]);
    for (int i = 1; i < arr.size(); i++)
    {
        T val = arr[i];
        int j = i;
        for (; arr[j - 1] > val; j--)
        {
            arr[j] = arr[j - 1];
        }
        arr[j] = val;
    }
}

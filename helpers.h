#pragma once

#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <span>

template <typename T>
bool IsSorted(const std::span<T>& arr)
{
	for (int i = 1; i < arr.size(); i++)
	{
		if (arr[i] < arr[i - 1])
		{
			return false;
		}
	}
	return true;
}

template <typename T>
std::vector<T> CreateRandomArray(int size, int seed)
{
	std::default_random_engine e(seed);
	std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min());
	std::vector<T> ret(size);
	for (int i = 0; i < size; i++)
	{
		ret[i] = dist(e);
	}
	return ret;
}

class Stopwatch
{
public:
	void Start()
	{
		m_start = std::chrono::steady_clock::now();
	}

	double End()
	{
		auto end = std::chrono::steady_clock::now();
		const std::chrono::duration<double> elapsed_seconds{ end - m_start };
		const double duration_ms = elapsed_seconds.count() * 1'000;
		return duration_ms;
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> m_start;
};

template <typename Function>
void Measure(int times_to_run, const std::string& tag, Function func)
{
	double measures = 0.0;
	for (int i = 0; i < times_to_run; i++)
	{
		Stopwatch sw;
		sw.Start();
		func();
		measures += sw.End();
	}
	measures /= static_cast<double>(times_to_run);
	std::cout << "Measuring " << tag << ", averaged over " << times_to_run << " iterations: " << std::fixed << std::setprecision(3) << measures << " ms\n";
}

template <typename T>
void Shuffle(std::span<T>& arr)
{
	std::mt19937_64 gen;
	for (int i = 0; i < arr.size(); i++)
	{
		std::uniform_int_distribution<T> dist(0, arr.size() - i - 1);
		const int r = i + dist(gen);
		std::swap(arr[i], arr[r]);
	}
}

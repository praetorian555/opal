#pragma once

#include <cstdint>
#include <cassert>
#include <utility>

/**
 * Priority queue data structure. New element is added in log2N time and top is acquired in log2N time.
 * @tparam T Data type.
 * @tparam CompareFunction function that has the following signature: bool (const T& a, const T& b).
 * If it returns true a goes to the bottom and b goes to the top. For example if we use std::less, larger
 * values will be on top.
 */
template <typename T, typename CompareFunction = std::less<T>>
class PriorityQueue
{
private:
	static constexpr int32_t k_default_capacity = 32;
public:
	PriorityQueue(const CompareFunction& compare = CompareFunction{}, int32_t capacity = k_default_capacity) : m_capacity(capacity), m_compare(compare)
	{
		m_data = new T[m_capacity];
	}

	void Enqueue(const T& value)
	{
		TryResize();
		m_data[++m_size] = value;
		Swim(m_size);
	}

	T PopTop()
	{
		assert(m_size > 0);
		T value = m_data[1];
		Exchange(1, m_size--);
		Sink(1);
		return value;
	}

	bool IsEmpty() const
	{
		return m_size == 0;
	}

	int32_t GetSize() const
	{
		return m_size;
	}

private:
	bool ShouldExpand() const
	{
		return m_size + 1 == m_capacity;
	}

	bool ShouldShrink() const
	{
		return m_size + 1 <= m_capacity / 4 && m_size + 1 > k_default_capacity;
	}

	void TryResize()
	{
		if (ShouldExpand())
		{
			Resize(true);
		}
		else if (ShouldShrink())
		{
			Resize(false);
		}
	}

	void Resize(bool should_expand)
	{
		const int32_t new_capacity = should_expand ? m_capacity * 2 : m_capacity / 2;
		T* new_data = new T[new_capacity];
		for (int i = 1; i <= m_size; i++)
		{
			new_data[i] = std::move(m_data[i]);
		}
		delete[] m_data;
		m_data = new_data;
		m_capacity = new_capacity;
	}

	void Swim(int k)
	{
		while (k > 1 && Compare(k / 2, k))
		{
			Exchange(k / 2, k);
			k /= 2;
		}
	}

	void Sink(int k)
	{
		while (2 * k <= m_size)
		{
			int j = 2 * k;
			if (j < m_size && Compare(j, j + 1)) j++;
			if (!Compare(k, j))
			{
				break;
			}
			Exchange(k, j);
			k = j;
		}
	}

	void Exchange(int i, int j)
	{
		std::swap(m_data[i], m_data[j]);
	}

	bool Compare(int i, int j)
	{
		return m_compare(m_data[i], m_data[j]);
	}

	T* m_data = nullptr;
	int32_t m_size = 0;
	int32_t m_capacity = 0;
	CompareFunction m_compare;
};

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

	T& GetTop()
	{
		assert(m_size > 0);
		return m_data[1];
	}

	const T& GetTop() const
	{
		assert(m_size > 0);
		return m_data[1];
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

/**
 * Improvement on regular priority queue where we associate each inserted value with an
 * index so we can access it later.
 */
template <typename T, typename CompareFunction = std::less<T>>
class IndexPriorityQueue
{
private:
	static constexpr int32_t k_default_capacity = 32;
public:
	IndexPriorityQueue(const CompareFunction& compare = CompareFunction(), int32_t capacity = k_default_capacity) : m_compare(compare), m_capacity(capacity)
	{
		assert(m_capacity > 0);
		m_values = new T[m_capacity];
		m_heap = new int32_t[m_capacity];
		m_index_to_heap_index = new int32_t[m_capacity];
		for (int i = 0; i < m_capacity; i++)
		{
			m_index_to_heap_index[i] = -1;
		}
	}

	void Enqueue(int32_t index, const T& value)
	{
		assert(!Contains(index));
		assert(m_size - 1 < m_capacity);
		m_values[index] = value;
		m_heap[++m_size] = index;
		m_index_to_heap_index[index] = m_size;
		Swim(m_size);
	}

	void ChangeValue(int32_t index, const T& new_value)
	{
		assert(Contains(index));
		m_values[index] = new_value;
		Swim(m_index_to_heap_index[index]);
		Sink(m_index_to_heap_index[index]);
	}

	bool Contains(int32_t index) const
	{
		if (index < 0 || index >= m_capacity)
		{
			return false;
		}
		return m_index_to_heap_index[index] != -1;
	}

	void DeleteValue(int32_t index)
	{
		assert(Contains(index));
		const int heap_index = m_index_to_heap_index[index];
		Exchange(heap_index, m_size--);
		Swim(heap_index);
		Sink(heap_index);
		m_values[index].~T();
		m_index_to_heap_index[index] = -1;
	}

	T& GetTop()
	{
		assert(!IsEmpty());
		return m_values[m_heap[1]];
	}

	const T& GetTop() const
	{
		assert(!IsEmpty());
		return m_values[m_heap[1]];
	}

	int32_t GetTopIndex() const
	{
		assert(!IsEmpty());
		return m_heap[1];
	}

	T PopTop()
	{
		T value = std::move(m_values[m_heap[1]]);
		const int32_t index = m_heap[1];
		Exchange(1, m_size--);
		Sink(1);
		m_index_to_heap_index[index] = -1;
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

	T& GetValueOf(int32_t index)
	{
		assert(Contains(index));
		return m_values[index];
	}

	const T& GetValueOf(int32_t index) const
	{
		assert(Contains(index));
		return m_values[index];
	}

private:
	bool Compare(int i, int j)
	{
		return m_compare(m_values[m_heap[i]], m_values[m_heap[j]]);
	}

	void Exchange(int i, int j) const
	{
		std::swap(m_heap[i], m_heap[j]);
		m_index_to_heap_index[m_heap[i]] = i;
		m_index_to_heap_index[m_heap[j]] = j;
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
			if (j < m_size && Compare(j, j + 1))
			{
				j++;
			}
			if (!Compare(k, j))
			{
				break;
			}
			Exchange(k, j);
			k = j;
		}
	}

	// Storing values such that their index in the array corresponds to the index given by the user
	T* m_values = nullptr;
	// Main heap structure, here we are using user provided indices to represent values
	int32_t* m_heap = nullptr;
	// Input is user provided index and output is index of it in the m_heap
	int32_t* m_index_to_heap_index = nullptr;
	int32_t m_size = 0;
	int32_t m_capacity = 0;
	CompareFunction m_compare;
};
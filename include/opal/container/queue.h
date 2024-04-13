#pragma once

#include <cstdint>
#include <utility>
#include <cassert>

template <typename T>
class Queue
{
private:
	static constexpr int32_t k_default_capacity = 32;

public:
	Queue()
	{
		m_data = new T[k_default_capacity];
		m_capacity = k_default_capacity;
	}

	bool IsEmpty() const
	{
		return m_size == 0;
	}

	int32_t GetSize() const
	{
		return m_size;
	}

	void Enqueue(const T& value)
	{
		TryResize();
		m_data[m_write_pos] = value;
		ModInc(m_write_pos);
		m_size++;
	}

	T Dequeue()
	{
		assert(m_size != 0);
		T value = m_data[m_read_pos];
		ModInc(m_read_pos);
		m_size--;
		TryResize();
		return value;
	}

private:
	void ModInc(T& val)
	{
		++val;
		val %= m_capacity;
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

	bool ShouldExpand() const
	{
		return m_size == m_capacity;
	}

	bool ShouldShrink() const
	{
		return m_size <= m_capacity / 4 && m_size > k_default_capacity;
	}

	void Resize(bool should_expand)
	{
		const int32_t new_capacity = should_expand ? 2 * m_capacity : m_capacity / 2;
		T* new_data = new T[new_capacity];
		for (int i = 0; i < m_size; i++)
		{
			new_data[i] = std::move(m_data[m_read_pos]);
			ModInc(m_read_pos);
		}
		m_read_pos = 0;
		m_write_pos = m_size;
		m_capacity = new_capacity;
		delete[] m_data;
		m_data = new_data;
	}

	T* m_data = nullptr;
	int32_t m_size = 0;
	int32_t m_capacity = 0;
	int32_t m_write_pos = 0;
	int32_t m_read_pos = 0;
};

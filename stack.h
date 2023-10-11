#pragma once

#include <cstdint>
#include <cassert>
#include <utility>

template <typename T>
class Stack
{
private:
	static constexpr int32_t k_default_capacity = 32;
public:
	Stack(int32_t capacity = k_default_capacity) : m_capacity(capacity)
	{
		assert(m_capacity > 0);
		m_data = new T[m_capacity];
	}

	void Push(const T& value)
	{
		TryResize();
		m_data[m_size++] = value;
	}

	T Pop()
	{
		T value = m_data[--m_size];
		TryResize();
		return value;
	}

private:
	bool ShouldExpand() const
	{
		return m_size == m_capacity;
	}

	bool ShouldShrink() const
	{
		return m_size <= m_capacity / 4 && m_size > k_default_capacity;
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
		for (int i = 0; i < m_size; i++)
		{
			new_data[i] = std::move(m_data[i]);
		}
		delete[] m_data;
		m_data = new_data;
		m_capacity = new_capacity;
	}

	T* m_data = nullptr;
	int32_t m_size = 0;
	int32_t m_capacity = 0;
};
#pragma once

#include <cstdint>
#include <cstdlib>

namespace Opal
{

/**
 * Alternative to std::vector.
 */
template <typename T>
class DynamicArray
{
public:
    using ValueType = T;
    using SizeType = uint32_t;

    DynamicArray() = default;
    explicit DynamicArray(SizeType count);
    DynamicArray(SizeType count, const T& default_value);
    // TODO: Implement constructors that take iterators as input
    // TODO: Implement constructors that take std::initializer_list as input
    DynamicArray(const DynamicArray& other);
    DynamicArray(DynamicArray&& other) noexcept;

    ~DynamicArray();

    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other) noexcept;

    [[nodiscard]] SizeType GetCapacity() const;
    [[nodiscard]] SizeType GetSize() const;
    T* GetData();
    const T* GetData() const;

private:
    T* Allocate(SizeType byte_count);
    void Deallocate(T* data);

    SizeType m_capacity = 4;
    SizeType m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/***************************************** Implementation *****************************************/

template <typename T>
Opal::DynamicArray<T>::DynamicArray(SizeType count)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count * sizeof(T));
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

template <typename T>
Opal::DynamicArray<T>::DynamicArray(SizeType count, const T& default_value)
{
    if (count > m_capacity)
    {
        m_capacity = count;
    }
    m_data = Allocate(count * sizeof(T));
    m_size = count;
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(default_value);  // Invokes copy constructor on allocated memory
    }
}

template <typename T>
Opal::DynamicArray<T>::DynamicArray(const DynamicArray& other) : m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = Allocate(m_capacity * sizeof(T));
    for (SizeType i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(other.m_data[i]);  // Invokes copy constructor on allocated memory
    }
}

template <typename T>
Opal::DynamicArray<T>::DynamicArray(DynamicArray&& other) noexcept
    : m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

template <typename T>
Opal::DynamicArray<T>::~DynamicArray()
{
    for (SizeType i = 0; i < m_size; i++)
    {
        m_data[i].~T();  // Invokes destructor on allocated memory
    }
    Deallocate(m_data);
}

template <typename T>
inline Opal::DynamicArray<T>::SizeType Opal::DynamicArray<T>::GetCapacity() const
{
    return m_capacity;
}

template <typename T>
inline Opal::DynamicArray<T>::SizeType Opal::DynamicArray<T>::GetSize() const
{
    return m_size;
}

template <typename T>
inline T* Opal::DynamicArray<T>::GetData()
{
    return m_data;
}

template <typename T>
inline const T* Opal::DynamicArray<T>::GetData() const
{
    return m_data;
}

template <typename T>
T* Opal::DynamicArray<T>::Allocate(SizeType byte_count)
{
    return static_cast<T*>(malloc(byte_count));
}

template <typename T>
void Opal::DynamicArray<T>::Deallocate(T* data)
{
    free(data);
}

#pragma once

#include <cstring>
#include <initializer_list>
#include <new>
#include <utility>

#include "opal/allocator.h"
#include "opal/assert.h"
#include "opal/casts.h"
#include "opal/common.h"
#include "opal/container/expected.h"
#include "opal/container/iterator.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

template <typename MyArray>
class DynamicArrayConstIterator;

/*************************************************************************************************/
/** Iterator API *********************************************************************************/
/*************************************************************************************************/

template <typename MyArray>
class DynamicArrayIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::reference;
    using pointer = typename MyArray::pointer;

    DynamicArrayIterator() = default;
    explicit DynamicArrayIterator(pointer ptr) : m_ptr(ptr) {}

    bool operator==(const DynamicArrayIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayIterator& other) const;
    bool operator>=(const DynamicArrayIterator& other) const;
    bool operator<(const DynamicArrayIterator& other) const;
    bool operator<=(const DynamicArrayIterator& other) const;

    DynamicArrayIterator& operator++();
    DynamicArrayIterator operator++(int);
    DynamicArrayIterator& operator--();
    DynamicArrayIterator operator--(int);

    DynamicArrayIterator operator+(difference_type n) const;
    DynamicArrayIterator operator-(difference_type n) const;
    DynamicArrayIterator& operator+=(difference_type n);
    DynamicArrayIterator& operator-=(difference_type n);

    difference_type operator-(const DynamicArrayIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    friend class DynamicArrayConstIterator<MyArray>;

    pointer m_ptr = nullptr;
};

template <typename MyArray>
DynamicArrayIterator<MyArray> operator+(typename DynamicArrayIterator<MyArray>::difference_type n, const DynamicArrayIterator<MyArray>& it);

/*************************************************************************************************/
/** Const Iterator API ***************************************************************************/
/*************************************************************************************************/

template <typename MyArray>
class DynamicArrayConstIterator
{
public:
    using value_type = typename MyArray::value_type;
    using difference_type = typename MyArray::difference_type;
    using reference = typename MyArray::const_reference;
    using pointer = typename MyArray::const_pointer;

    DynamicArrayConstIterator() = default;
    explicit DynamicArrayConstIterator(pointer ptr) : m_ptr(ptr) {}

    // A mutable iterator converts to a const one, the way it does for the standard containers, so
    // methods that only read through the position take a single const_iterator parameter.
    DynamicArrayConstIterator(const DynamicArrayIterator<MyArray>& other) : m_ptr(other.m_ptr) {}

    bool operator==(const DynamicArrayConstIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator>(const DynamicArrayConstIterator& other) const;
    bool operator>=(const DynamicArrayConstIterator& other) const;
    bool operator<(const DynamicArrayConstIterator& other) const;
    bool operator<=(const DynamicArrayConstIterator& other) const;

    DynamicArrayConstIterator& operator++();
    DynamicArrayConstIterator operator++(int);
    DynamicArrayConstIterator& operator--();
    DynamicArrayConstIterator operator--(int);

    DynamicArrayConstIterator operator+(difference_type n) const;
    DynamicArrayConstIterator operator-(difference_type n) const;
    DynamicArrayConstIterator& operator+=(difference_type n);
    DynamicArrayConstIterator& operator-=(difference_type n);

    difference_type operator-(const DynamicArrayConstIterator& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    pointer m_ptr = nullptr;
};

template <typename MyArray>
DynamicArrayConstIterator<MyArray> operator+(typename DynamicArrayConstIterator<MyArray>::difference_type n,
                                             const DynamicArrayConstIterator<MyArray>& it);

/*************************************************************************************************/
/** Reverse Iterator API *************************************************************************/
/*************************************************************************************************/

/**
 * Walks a DynamicArray from the last element to the first. Advancing it steps backwards through
 * the array, so the usual begin-to-end loop visits the elements in reverse.
 *
 * Like the standard reverse iterators, it holds the position one past the element it refers to, so
 * that the reverse end can be built from the array's begin without pointing before the storage.
 */
template <typename MyArray, typename BaseIterator>
class DynamicArrayReverseIteratorBase
{
public:
    using value_type = typename BaseIterator::value_type;
    using difference_type = typename BaseIterator::difference_type;
    using reference = typename BaseIterator::reference;
    using pointer = typename BaseIterator::pointer;

    DynamicArrayReverseIteratorBase() = default;
    explicit DynamicArrayReverseIteratorBase(BaseIterator base) : m_base(base) {}

    /** Get the underlying forward iterator, one past the element this refers to. */
    BaseIterator GetBase() const { return m_base; }

    bool operator==(const DynamicArrayReverseIteratorBase& other) const { return m_base == other.m_base; }
    bool operator>(const DynamicArrayReverseIteratorBase& other) const;
    bool operator>=(const DynamicArrayReverseIteratorBase& other) const;
    bool operator<(const DynamicArrayReverseIteratorBase& other) const;
    bool operator<=(const DynamicArrayReverseIteratorBase& other) const;

    DynamicArrayReverseIteratorBase& operator++();
    DynamicArrayReverseIteratorBase operator++(int);
    DynamicArrayReverseIteratorBase& operator--();
    DynamicArrayReverseIteratorBase operator--(int);

    DynamicArrayReverseIteratorBase operator+(difference_type n) const;
    DynamicArrayReverseIteratorBase operator-(difference_type n) const;
    DynamicArrayReverseIteratorBase& operator+=(difference_type n);
    DynamicArrayReverseIteratorBase& operator-=(difference_type n);

    difference_type operator-(const DynamicArrayReverseIteratorBase& other) const;

    reference operator[](difference_type n) const;
    reference operator*() const;
    pointer operator->() const;

private:
    BaseIterator m_base = {};
};

template <typename MyArray, typename BaseIterator>
DynamicArrayReverseIteratorBase<MyArray, BaseIterator> operator+(
    typename DynamicArrayReverseIteratorBase<MyArray, BaseIterator>::difference_type n,
    const DynamicArrayReverseIteratorBase<MyArray, BaseIterator>& it);

/**
 * Represents continuous memory storage on the heap that can dynamically grow in size. Similar to std::vector.
 *
 * The array does not copy. It moves, or it clones through Opal::Clone, so a non-POD element type
 * has to provide Clone(AllocatorBase*) to be stored by value rather than moved in.
 *
 * A value or range handed to Assign, Insert, Resize, PushBack or Append may name elements of the
 * array it is being given to. std::vector leaves the range case undefined; this does not.
 *
 * Growing invalidates every iterator, pointer and reference into the array. So does erasing, from
 * the erased position onwards.
 */
template <typename T>
class DynamicArray
{
public:
    using value_type = T;
    using allocator_type = AllocatorBase;
    using size_type = u64;
    using difference_type = i64;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = DynamicArrayIterator<DynamicArray>;
    using const_iterator = DynamicArrayConstIterator<DynamicArray>;
    using reverse_iterator = DynamicArrayReverseIteratorBase<DynamicArray, iterator>;
    using const_reverse_iterator = DynamicArrayReverseIteratorBase<DynamicArray, const_iterator>;

    static_assert(!k_is_reference_value<value_type>, "Value type must not be a reference");
    static_assert(!k_is_const_value<value_type>, "Value type must not be const");

    /**
     * Default constructor.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` default constructed elements.
     * @param count Number of elements to construct.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    explicit DynamicArray(size_type count, allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` elements with value `default_value`.
     * @param count Number of elements to construct.
     * @param default_value Value of the elements.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(size_type count, const T& default_value, allocator_type* allocator = nullptr);

    /**
     * Construct an array with `count` elements copied from `data`.
     * @param data Source data.
     * @param count Number of elements to copy.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(const T* data, size_type count, allocator_type* allocator = nullptr);

    /**
     * The array does not copy. Use @ref Clone for a deep copy, or move it.
     */
    DynamicArray(const DynamicArray& other) = delete;
    DynamicArray& operator=(const DynamicArray& other) = delete;

    /**
     * Move constructor.
     * @param other Source array.
     */
    DynamicArray(DynamicArray&& other) noexcept;

    /**
     * Construct an array with elements from the initializer list.
     * @param init_list Initializer list.
     * @param allocator Allocator to be used for memory allocation. If nullptr, the default allocator will be used.
     */
    DynamicArray(std::initializer_list<T> init_list, allocator_type* allocator = nullptr);

    /**
     * Create a deep copy of this array. Unlike the copy constructor, Clone allows specifying a different allocator for
     * the new array. The cloned array will have the same capacity and elements as the source array.
     * @param allocator Allocator to be used for the cloned array. If nullptr, the source array's allocator will be used.
     * @return A new DynamicArray that is a deep copy of this array.
     */
    DynamicArray Clone(AllocatorBase* allocator = nullptr) const;

    ~DynamicArray();

    /**
     * Copy and move assignments. Always uses allocator from the source array.
     * @param other Source array.
     * @return Reference to this array.
     */
    DynamicArray& operator=(DynamicArray&& other) noexcept;

    /**
     * Compare element by element. Two arrays are equal when they have the same size and every
     * element compares equal to the one at the same index.
     * @param other Array to compare against.
     * @return True when the arrays are equal.
     */
    bool operator==(const DynamicArray& other) const;

    /**
     * Clears the array and adds `count` new elements with value `value`. `value` may be an element
     * of this array.
     * @param count How many new elements to add.
     * @param value Value of the new elements.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Assign(size_type count, const T& value);

    /**
     * Clears the array and adds new elements based on the input iterator range. The range may read
     * from this array.
     * @tparam InputIt Input iterator type.
     * @param start Start of the range, inclusive.
     * @param end end of the range, exclusive.
     * @return ErrorCode::Success, or ErrorCode::InvalidArgument if @p start is greater than @p end.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    ErrorCode Assign(InputIt start, InputIt end);

    /**
     * Clears the array and adds the elements of the initializer list.
     * @param init_list Initializer list.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Assign(std::initializer_list<T> init_list);

    /**
     * Clears the array and adds the elements of the initializer list. Keeps the current allocator.
     * @param init_list Initializer list.
     * @return Reference to this array.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    DynamicArray& operator=(std::initializer_list<T> init_list);

    /**
     * Get a reference to the element at specified index.
     * @param index Index of the element in the array.
     * @return Returns a reference to the element in the array at the given index.
     * @throw OutOfBoundsException when index is out of bounds.
     */
    reference At(size_type index);
    const_reference At(size_type index) const;

    /**
     * Get a reference to the element at specified index. No index bounds checking.
     * @param index Index of the element in the array.
     * @return Reference to the element.
     */
    reference operator[](size_type index);
    const_reference operator[](size_type index) const;

    /**
     * Get a reference to the first element in the array.
     * @return Reference to the first element.
     * @throw OutOfBoundsException when array is empty.
     */
    reference Front();
    const_reference Front() const;

    /**
     * Get a reference to the last element in the array.
     * @return Reference to the last element.
     * @throw OutOfBoundsException when array is empty.
     */
    reference Back();
    const_reference Back() const;

    /**
     * Get a pointer to the first element. The elements are contiguous, so the pointer addresses
     * all of them. Null until the array first takes memory, which reserving does even though no
     * element exists yet.
     * @return Pointer to the storage.
     */
    T* GetData();
    const T* GetData() const;

    /**
     * Get how many elements the array can hold before it has to take more memory.
     * @return Current capacity, in elements.
     */
    [[nodiscard]] size_type GetCapacity() const;

    /**
     * Get how many elements the array holds.
     * @return Current size, in elements.
     */
    [[nodiscard]] size_type GetSize() const;

    /**
     * Get the allocator the array takes its memory from. Never null.
     * @return The allocator in use.
     */
    allocator_type* GetAllocator() const { return m_allocator; }

    /**
     * Move the elements to memory from `allocator` and use it from then on. Does nothing when the
     * array already uses that allocator. Capacity comes down to the current size, so any spare
     * room the array was holding is given up.
     * @param allocator Allocator to move to. If nullptr, the default allocator is used.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void SetAllocator(allocator_type* allocator);

    /**
     * Check if the array is empty.
     * @return True if the array is empty, false otherwise.
     */
    [[nodiscard]] bool IsEmpty() const { return m_size == 0; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    /**
     * Increase the capacity of the array to a value `new_capacity` if its greater then current capacity, otherwise do nothing.
     * @param new_capacity New capacity of the array.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Reserve(size_type new_capacity);

    /**
     * Give up any capacity beyond the current size. Releases the storage outright when the array is
     * empty. Does nothing when there is no spare room. Invalidates every iterator, pointer and
     * reference into the array when it moves the elements.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void ShrinkToFit();

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are default constructed.
     * @param new_size New size of the array.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Resize(size_type new_size);

    /**
     * Change the size of the array to `new_size`. If `new_size` is greater than current size, new elements are copy constructed from
     * `default_value`, which may be an element of this array.
     * @param new_size New size of the array.
     * @param default_value Value to copy construct new elements from.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void Resize(size_type new_size, const T& default_value);

    /**
     * Clear the array and set its size to 0. Does not deallocate memory.
     */
    void Clear();

    /**
     * Add a new element to the end of the array. If the array is full, it will be resized. `value`
     * may be an element of this array.
     *
     * Only a POD element type can be pushed by copy. Anything else has to be moved in, or cloned
     * by the caller and the clone moved in, which keeps the copying explicit.
     * @param value Value of the new element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    void PushBack(const T& value)
        requires IsPOD<T>;
    void PushBack(T&& value);

    /**
     * Construct a new element in-place at the end of the array. If the array is full, it will be resized.
     * @tparam Args Types of the arguments to forward to the constructor.
     * @param args Arguments to forward to the constructor.
     * @return Reference to the newly constructed element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    template <typename... Args>
    reference EmplaceBack(Args&&... args);

    /**
     * Add every element of `container` to the end of the array, in order. The container is left
     * unchanged; its elements are cloned.
     * @param container Source range.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    template <typename ContainerClass>
        requires Range<ContainerClass>
    void Append(const ContainerClass& container);

    /**
     * Add every element of `container` to the end of the array, in order, moving them out of the
     * container. Only selected for an rvalue; an lvalue container is appended by the overload
     * above and left intact.
     * @param container Source range, emptied of its element values.
     * @throw OutOfMemoryException when allocator runs out of memory.
     */
    template <typename ContainerClass>
        requires NonReferenceRange<ContainerClass>
    void Append(ContainerClass&& container);

    /**
     * Remove the last element from the array.
     */
    void PopBack();

    /**
     * Insert a new element at the specified position. `value` may be an element of this array.
     * @param position Iterator pointing to the position where the new element should be inserted. Can be @ref cend to insert at the
     * end.
     * @param value Value of the new element.
     * @return Iterator pointing to the newly inserted element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     */
    iterator Insert(const_iterator position, const T& value);
    iterator Insert(const_iterator position, T&& value);

    /**
     * Insert `count` new elements with value `value` at the specified position. `value` may be an
     * element of this array.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be cend() to insert at the
     * end.
     * @param count How many new elements to insert.
     * @param value Value of the new elements.
     * @return Iterator pointing to the first newly inserted element or @p position if no element is inserted.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     */
    iterator Insert(const_iterator position, size_type count, const T& value);

    /**
     * Insert new elements from the range [@p start_it, @p end_it) at the specified position. The
     * range may read from this array, which std::vector does not allow.
     * @tparam InputIt Input iterator type.
     * @param position Iterator pointing to the position where the new elements should be inserted. Can be @ref cend to insert at the
     * end.
     * @param start_it Start of the range, inclusive.
     * @param end_it end of the range, exclusive.
     * @return Iterator pointing to the first newly inserted element or @p position if no element is inserted.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     * @throw InvalidArgumentException if @p start_it is greater than @p end_it.
     */
    template <typename InputIt>
        requires RandomAccessIterator<InputIt>
    iterator Insert(const_iterator position, InputIt start_it, InputIt end_it);

    /**
     * Construct a new element in-place at the specified position. The arguments may name elements
     * of this array.
     * @tparam Args Types of the arguments to forward to the constructor.
     * @param position Iterator pointing to the position where the new element should be constructed. Can be @ref cend to construct at
     * the end.
     * @param args Arguments to forward to the constructor.
     * @return Iterator pointing to the newly constructed element.
     * @throw OutOfMemoryException when allocator runs out of memory.
     * @throw OutOfBoundsException when position is out of bounds.
     */
    template <typename... Args>
    iterator Emplace(const_iterator position, Args&&... args);

    /**
     * Erase the element at the specified position. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the element following the erased element. Returns @ref end() if @p position is out of bounds.
     */
    iterator Erase(const_iterator position);

    /**
     * Erase the element at the specified position by swapping it with the last element. Does not deallocate memory.
     * @param position Iterator pointing to the element to erase.
     * @return Iterator pointing to the new element at the @pos position, or @ref end() if @p position is out of bounds.
     */
    iterator EraseWithSwap(const_iterator position);

    /**
     * Erase elements in the range [start, end). Does not deallocate memory.
     * @param start_it Iterator pointing to the first element to erase.
     * @param end_it Iterator pointing to the element following the last element to erase.
     * @return Iterator pointing to the element following the last erased element, ErrorCode::InvalidArgument if @p start_it is
     * greater than @p end_it, or ErrorCode::OutOfBounds if either is out of bounds.
     */
    Expected<iterator, ErrorCode> Erase(const_iterator start_it, const_iterator end_it);

    /**
     * Find the first element equal to `value`.
     * @param value Value to look for. Uses equality operator of the type T.
     * @return Iterator to the first match, or @ref end() when nothing matches.
     */
    iterator Find(const T& value);
    const_iterator Find(const T& value) const;

    /**
     * Find the first element the predicate accepts.
     * @tparam Predicate Callable taking a `const T&` and returning bool.
     * @param predicate Test to apply to each element in order.
     * @return Iterator to the first match, or @ref end() when nothing matches.
     */
    template <typename Predicate>
    iterator FindIf(Predicate predicate);
    template <typename Predicate>
    const_iterator FindIf(Predicate predicate) const;

    /**
     * Check whether any element equals `value`.
     * @param value Value to look for. Uses equality operator of the type T.
     * @return True when at least one element matches.
     */
    bool Contains(const T& value) const;

    /**
     * Remove every element the predicate accepts. The order of the remaining elements stays the
     * same. Does not deallocate memory.
     * @tparam Predicate Callable taking a `const T&` and returning bool.
     * @param predicate Test to apply to each element.
     * @return How many elements were removed.
     */
    template <typename Predicate>
    size_type RemoveIf(Predicate predicate);

    /**
     * Remove the first element matching the value argument. Do nothing if no element matches. Later
     * matches are left in place. The order of the elements stays the same.
     * @param value Value to find. Uses equality operator of the type T.
     */
    void Remove(const T& value);

    /**
     * Remove the first element matching the value argument, by moving the last element into its
     * place. Do nothing if no element matches. Later matches are left in place. The order of the
     * elements does not stay the same.
     * @param value Value to find. Uses equality operator of the type T.
     */
    void RemoveWithSwap(const T& value);

    /** Iterator API - Compatible with standard library. */

    /**
     * Get an iterator pointing to the first element in the array.
     * @return Iterator pointing to the first element.
     */
    iterator begin() { return iterator(m_data); }

    /**
     * Get a const iterator pointing to the first element in the array.
     * @return Const iterator pointing to the first element.
     */
    const_iterator begin() const { return const_iterator(m_data); }

    /**
     * Get a const iterator pointing to the first element in the array.
     * @return Const iterator pointing to the first element.
     */
    const_iterator cbegin() const { return const_iterator(m_data); }

    /**
     * Get an iterator pointing to the element following the last element in the array.
     * @return Iterator pointing to the element following the last element.
     */
    iterator end() { return iterator(m_data + m_size); }

    /**
     * Get a const iterator pointing to the element following the last element in the array.
     * @return Const iterator pointing to the element following the last element.
     */
    const_iterator end() const { return const_iterator(m_data + m_size); }

    /**
     * Get a const iterator pointing to the element following the last element in the array.
     * @return Const iterator pointing to the element following the last element.
     */
    const_iterator cend() const { return const_iterator(m_data + m_size); }

    /**
     * Get a reverse iterator pointing to the last element in the array. Advancing it steps towards
     * the front.
     * @return Reverse iterator pointing to the last element.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    /**
     * Get a reverse iterator pointing one before the first element in the array.
     * @return Reverse iterator marking the end of a backwards walk.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(cbegin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

private:
    T* Allocate(size_type count);
    void Deallocate(T* ptr);

    size_type GetNextCapacity(size_type current_capacity) const;

    // Open a gap of `count` slots at `pos_offset` for an insert, moving the elements at and
    // after it up. The slots that land past the old end are raw storage and are
    // move-constructed into; the rest hold live objects and are move-assigned. Leaves m_size
    // alone: the caller fills the gap — assigning to the slots below the old size,
    // constructing into those at or above it — and then adds `count` to it. Capacity for
    // m_size + count must already be reserved.
    void MakeInsertGap(difference_type pos_offset, size_type count);

    // Open a one-slot gap at `pos_offset` and put `value` in it, returning an iterator to it.
    // Capacity for m_size + 1 must already be reserved, and `value` must not name an element of
    // this array.
    template <typename U>
    iterator InsertOneAt(difference_type pos_offset, U&& value);

    // Open a gap of `count` slots at `pos_offset` and fill it with copies of `value`, returning
    // an iterator to the first. Capacity for m_size + count must already be reserved, and `value`
    // must not name an element of this array.
    iterator InsertCountAt(difference_type pos_offset, size_type count, const T& value);

    // Grow into a fresh buffer with a gap of `count` slots at `pos_offset`, and return an iterator
    // to the first of them. `construct_gap(T* gap)` fills the gap by constructing into it, and is
    // called while the old elements are still alive and in place, so a source that reads from them
    // stays valid. Only for the case where the array has to grow.
    template <typename ConstructGap>
    iterator GrowAndInsert(difference_type pos_offset, size_type count, ConstructGap&& construct_gap);

    // True when the range reads out of this array's own storage. Such a range cannot survive the
    // elements being shifted or released, so the methods that would do either build their result
    // in fresh storage instead.
    template <typename InputIt>
    bool RangeReadsOwnStorage(InputIt start_it, InputIt end_it) const;

    // True when `value` is one of this array's own elements. Methods that release or overwrite the
    // elements take a copy of it first when so, and pay nothing for the check otherwise.
    bool ValueReadsOwnStorage(const T& value) const;

    // Make room for `count` more elements in one go, keeping the geometric growth step so that
    // repeated appends do not degrade into one allocation per call.
    void ReserveForAppend(size_type count);

    // Move every element into fresh storage of `new_capacity` taken from `allocator`, destroy what
    // is left behind, release the old buffer and adopt the allocator. `new_capacity` has to be at
    // least the current size; zero releases the storage outright.
    void Rehome(allocator_type* allocator, size_type new_capacity);

    static constexpr f64 k_resize_factor = 1.5;

    allocator_type* m_allocator = nullptr;
    size_type m_capacity = 0;
    size_type m_size = 0;
    T* m_data = nullptr;
};

}  // namespace Opal

/*************************************************************************************************/
/***************************************** Implementation ****************************************/
/*************************************************************************************************/

#define TEMPLATE_HEADER template <typename T>
#define CLASS_HEADER Opal::DynamicArray<T>

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(allocator_type* allocator) : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator) {}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T();  // Invokes default constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(size_type count, const T& default_value, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    for (size_type i = 0; i < m_size; i++)
    {
        new (&m_data[i]) T(Opal::Clone(default_value));  // Invokes copy constructor on allocated memory
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(const T* data, size_type count, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, data, count * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(data[i]);  // Invokes copy constructor on allocated memory
        }
    }
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(DynamicArray&& other) noexcept
    : m_allocator(Move(other.m_allocator)), m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data)
{
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray(std::initializer_list<T> init_list, allocator_type* allocator)
    : m_allocator(allocator == nullptr ? GetDefaultAllocator() : allocator)
{
    size_type count = init_list.size();
    if (count == 0)
    {
        return;
    }
    m_data = Allocate(count);
    m_capacity = count;
    m_size = count;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, init_list.begin(), count * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            new (&m_data[i]) T(Opal::Clone(*(init_list.begin() + i)));  // Invokes copy constructor on allocated memory
        }
    }
}

template <typename T>
Opal::DynamicArray<T> Opal::DynamicArray<T>::Clone(AllocatorBase* allocator) const
{
    allocator = allocator == nullptr ? m_allocator : allocator;
    DynamicArray clone(allocator);
    clone.Reserve(m_capacity);
    clone.Append(*this);
    return clone;
}

TEMPLATE_HEADER
CLASS_HEADER::DynamicArray::~DynamicArray()
{
    if (m_data != nullptr)
    {
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        Deallocate(m_data);
        m_data = nullptr;
    }
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(DynamicArray&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (m_data != nullptr)
    {
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        Deallocate(m_data);
        m_data = nullptr;
    }
    m_allocator = other.m_allocator;
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    m_data = other.m_data;
    other.m_capacity = 0;
    other.m_size = 0;
    other.m_data = nullptr;
    return *this;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator==(const DynamicArray& other) const
{
    if (m_size != other.m_size)
    {
        return false;
    }
    if (m_size == 0)
    {
        return true;
    }
    if constexpr (IsPOD<T> && HasUniqueObjectRepresentations<T>)
    {
        return memcmp(m_data, other.m_data, m_size * sizeof(T)) == 0;
    }
    else
    {
        for (size_type i = 0; i < m_size; i++)
        {
            if (m_data[i] != other.m_data[i])
            {
                return false;
            }
        }
        return true;
    }
}

TEMPLATE_HEADER
inline CLASS_HEADER::size_type CLASS_HEADER::GetCapacity() const
{
    return m_capacity;
}

TEMPLATE_HEADER
inline CLASS_HEADER::size_type CLASS_HEADER::GetSize() const
{
    return m_size;
}

TEMPLATE_HEADER
void CLASS_HEADER::SetAllocator(allocator_type* allocator)
{
    if (allocator == nullptr)
    {
        allocator = GetDefaultAllocator();
    }
    if (m_allocator == allocator)
    {
        return;
    }
    Rehome(allocator, m_size);
}

TEMPLATE_HEADER
void CLASS_HEADER::Assign(size_type count, const T& value)
{
    if (ValueReadsOwnStorage(value))
    {
        // `value` is one of the elements about to be destroyed. Re-enter with a copy of it that
        // lives on the stack, which the check above then passes over.
        T value_copy(Opal::Clone(value));
        Assign(count, value_copy);
        return;
    }
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    // The old elements are gone. Nothing below may leave them looking alive, since anything below
    // can throw and the destructor would then run over them a second time.
    m_size = 0;
    if (count > m_capacity)
    {
        // Take the new storage before releasing the old, so a failed allocation leaves the array
        // holding a buffer it still owns rather than a freed one.
        T* new_data = Allocate(count);
        Deallocate(m_data);
        m_data = new_data;
        m_capacity = count;
    }
    for (size_type i = 0; i < count; i++)
    {
        new (&m_data[i]) T(Opal::Clone(value));  // Invokes copy constructor on allocated memory
        m_size++;
    }
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
Opal::ErrorCode CLASS_HEADER::Assign(InputIt start, InputIt end)
{
    if (start > end) [[unlikely]]
    {
        return ErrorCode::InvalidArgument;
    }
    size_type count = static_cast<size_type>(end - start);
    if (RangeReadsOwnStorage(start, end))
    {
        // The range reads the very elements this would otherwise destroy first, so the new
        // contents are built in fresh storage and the old buffer is only released afterwards.
        T* new_data = Allocate(count);
        for (size_type i = 0; i < count; ++i)
        {
            new (&new_data[i]) T(*(start + Narrow<difference_type>(i)));  // Invokes copy constructor on allocated memory
        }
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = 0; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        Deallocate(m_data);
        m_data = new_data;
        m_capacity = count;
        m_size = count;
        return ErrorCode::Success;
    }
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    // The old elements are gone. Nothing below may leave them looking alive, since anything below
    // can throw and the destructor would then run over them a second time.
    m_size = 0;
    if (count == 0)
    {
        return ErrorCode::Success;
    }
    if (count > m_capacity)
    {
        // Take the new storage before releasing the old, so a failed allocation leaves the array
        // holding a buffer it still owns rather than a freed one.
        T* new_data = Allocate(count);
        Deallocate(m_data);
        m_data = new_data;
        m_capacity = count;
    }
    InputIt current = start;
    if constexpr (IsPOD<T>)
    {
        memcpy(m_data, &(*current), count * sizeof(T));
        m_size = count;
    }
    else
    {
        for (size_type i = 0; i < count; ++i)
        {
            new (&m_data[i]) T(*(current + Narrow<difference_type>(i)));  // Invokes copy constructor on allocated memory
            m_size++;
        }
    }
    return ErrorCode::Success;
}

TEMPLATE_HEADER
void CLASS_HEADER::Assign(std::initializer_list<T> init_list)
{
    // The list is its own storage, never ours, so nothing here can alias.
    Clear();
    const size_type count = init_list.size();
    if (count > m_capacity)
    {
        T* new_data = Allocate(count);
        Deallocate(m_data);
        m_data = new_data;
        m_capacity = count;
    }
    for (size_type i = 0; i < count; i++)
    {
        new (&m_data[i]) T(Opal::Clone(*(init_list.begin() + i)));  // Invokes copy constructor on allocated memory
        m_size++;
    }
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator=(std::initializer_list<T> init_list)
{
    Assign(init_list);
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::At(size_type index)
{
    if (index >= m_size) [[unlikely]]
    {
        if (m_size == 0)
        {
            throw OutOfBoundsException("The array is empty!");
        }
        throw OutOfBoundsException(index, u64{0}, m_size - 1);
    }
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::At(size_type index) const
{
    if (index >= m_size) [[unlikely]]
    {
        if (m_size == 0)
        {
            throw OutOfBoundsException("The array is empty!");
        }
        throw OutOfBoundsException(index, u64{0}, m_size - 1);
    }
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](DynamicArray::size_type index)
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::operator[](DynamicArray::size_type index) const
{
    OPAL_ASSERT(index < m_size, "Index out of bounds");
    return m_data[index];
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::Front()
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[0];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::Front() const
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[0];
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::Back()
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[m_size - 1];
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_reference CLASS_HEADER::Back() const
{
    if (m_size == 0) [[unlikely]]
    {
        throw OutOfBoundsException("The array is empty!");
    }
    return m_data[m_size - 1];
}

TEMPLATE_HEADER
inline T* CLASS_HEADER::GetData()
{
    return m_data;
}

TEMPLATE_HEADER
inline const T* CLASS_HEADER::GetData() const
{
    return m_data;
}

TEMPLATE_HEADER
void CLASS_HEADER::Reserve(DynamicArray::size_type new_capacity)
{
    if (new_capacity <= m_capacity)
    {
        return;
    }
    Rehome(m_allocator, new_capacity);
}

TEMPLATE_HEADER
void CLASS_HEADER::ShrinkToFit()
{
    if (m_capacity == m_size)
    {
        return;
    }
    Rehome(m_allocator, m_size);
}

TEMPLATE_HEADER
void CLASS_HEADER::Resize(DynamicArray::size_type new_size)
{
    Resize(new_size, T());
}

TEMPLATE_HEADER
void CLASS_HEADER::Resize(DynamicArray::size_type new_size, const T& default_value)
{
    if (new_size == m_size)
    {
        return;
    }
    if (new_size < m_size)
    {
        if constexpr (!IsPOD<T>)
        {
            for (size_type i = new_size; i < m_size; i++)
            {
                m_data[i].~T();  // Invokes destructor on allocated memory
            }
        }
        m_size = new_size;
    }
    else if (new_size > m_capacity)
    {
        if (ValueReadsOwnStorage(default_value))
        {
            // Growing frees the storage `default_value` lives in. Re-enter with a copy of it that
            // lives on the stack, which the check then passes over.
            T value_copy(Opal::Clone(default_value));
            Resize(new_size, value_copy);
            return;
        }
        Reserve(new_size);
        for (size_type i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(Opal::Clone(default_value));  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
    else
    {
        for (size_type i = m_size; i < new_size; i++)
        {
            new (&m_data[i]) T(Opal::Clone(default_value));  // Invokes copy constructor on allocated memory
        }
        m_size = new_size;
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::Clear()
{
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = 0; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    m_size = 0;
}

TEMPLATE_HEADER
void CLASS_HEADER::PushBack(const T& value)
    requires IsPOD<T>
{
    if (m_size == m_capacity)
    {
        if (ValueReadsOwnStorage(value))
        {
            // Growing frees the storage `value` lives in. Re-enter with a copy of it that lives on
            // the stack, which the check then passes over.
            T value_copy(value);
            PushBack(value_copy);
            return;
        }
        Reserve(GetNextCapacity(m_capacity));
    }
    new (&m_data[m_size]) T(value);  // Invokes copy constructor on allocated memory
    m_size++;
}

TEMPLATE_HEADER
void CLASS_HEADER::PushBack(T&& value)
{
    if (m_size == m_capacity)
    {
        if (ValueReadsOwnStorage(value))
        {
            T value_copy(Move(value));
            PushBack(Move(value_copy));
            return;
        }
        Reserve(GetNextCapacity(m_capacity));
    }
    new (&m_data[m_size]) T(Move(value));  // Invokes move constructor on allocated memory
    m_size++;
}

TEMPLATE_HEADER
template <typename... Args>
typename CLASS_HEADER::reference CLASS_HEADER::EmplaceBack(Args&&... args)
{
    if (m_size == m_capacity)
    {
        // The arguments are allowed to name elements of this array, whose storage growing frees,
        // so the element is built before the buffer goes away and then moved into place.
        T value(std::forward<Args>(args)...);
        Reserve(GetNextCapacity(m_capacity));
        new (&m_data[m_size]) T(Move(value));  // Invokes move constructor on allocated memory
        m_size++;
        return m_data[m_size - 1];
    }
    new (&m_data[m_size]) T(std::forward<Args>(args)...);
    m_size++;
    return m_data[m_size - 1];
}

TEMPLATE_HEADER
template <typename ContainerClass>
    requires Opal::Range<ContainerClass>
void CLASS_HEADER::Append(const ContainerClass& container)
{
    // Take the room up front. The loop below reads `container` from beginning to end, and if a
    // PushBack were left to grow part way through, it would reallocate out from under a container
    // that is this array itself.
    ReserveForAppend(static_cast<size_type>(Opal::end(container) - Opal::begin(container)));
    for (const auto& element : container)
    {
        PushBack(Opal::Clone(element));
    }
}

TEMPLATE_HEADER
template <typename ContainerClass>
    requires Opal::NonReferenceRange<ContainerClass>
void CLASS_HEADER::Append(ContainerClass&& container)
{
    ReserveForAppend(static_cast<size_type>(Opal::end(container) - Opal::begin(container)));
    for (auto& element : container)
    {
        PushBack(std::move(element));
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::PopBack()
{
    if (m_size == 0)
    {
        return;
    }
    if constexpr (!IsPOD<T>)
    {
        m_data[m_size - 1].~T();  // Invokes destructor on allocated memory
    }
    m_size--;
}

TEMPLATE_HEADER
void CLASS_HEADER::MakeInsertGap(difference_type pos_offset, size_type count)
{
    const size_type pos = static_cast<size_type>(pos_offset);
    const size_type after = m_size - pos;  // elements at and after the position
    // The last `to_construct` of them land past the old end, in storage no object lives in
    // yet: those must be move-constructed, since assigning to raw memory would run the
    // element's assignment (and its cleanup) on an object that was never constructed.
    const size_type to_construct = after < count ? after : count;
    for (size_type i = 0; i < to_construct; ++i)
    {
        const size_type source = m_size - to_construct + i;
        new (&m_data[source + count]) T(Move(m_data[source]));  // Invokes move constructor on allocated memory
    }
    // The survivors below them move onto live objects, back to front so none is overwritten
    // before it has been read.
    for (size_type i = m_size - to_construct; i > pos; --i)
    {
        m_data[i - 1 + count] = Move(m_data[i - 1]);
    }
}

TEMPLATE_HEADER
template <typename U>
typename CLASS_HEADER::iterator CLASS_HEADER::InsertOneAt(difference_type pos_offset, U&& value)
{
    const size_type old_size = m_size;
    MakeInsertGap(pos_offset, 1);
    iterator mut_position = begin() + pos_offset;
    if (static_cast<size_type>(pos_offset) < old_size)
    {
        *mut_position = std::forward<U>(value);
    }
    else
    {
        new (&(*mut_position)) T(std::forward<U>(value));  // Invokes copy or move constructor on allocated memory
    }
    m_size++;
    return mut_position;
}

TEMPLATE_HEADER
template <typename InputIt>
bool CLASS_HEADER::RangeReadsOwnStorage(InputIt start_it, InputIt end_it) const
{
    if (m_data == nullptr || start_it == end_it)
    {
        return false;
    }
    const u64 first = reinterpret_cast<u64>(&(*start_it));
    const u64 storage_begin = reinterpret_cast<u64>(m_data);
    const u64 storage_end = reinterpret_cast<u64>(m_data + m_capacity);
    return first >= storage_begin && first < storage_end;
}

TEMPLATE_HEADER
bool CLASS_HEADER::ValueReadsOwnStorage(const T& value) const
{
    if (m_data == nullptr)
    {
        return false;
    }
    const u64 address = reinterpret_cast<u64>(&value);
    return address >= reinterpret_cast<u64>(m_data) && address < reinterpret_cast<u64>(m_data + m_capacity);
}

TEMPLATE_HEADER
void CLASS_HEADER::Rehome(allocator_type* allocator, size_type new_capacity)
{
    OPAL_ASSERT(new_capacity >= m_size, "New storage has to hold what the array already has");
    if (new_capacity == 0)
    {
        Deallocate(m_data);
        m_data = nullptr;
        m_capacity = 0;
        m_allocator = allocator;
        return;
    }
    const size_type bytes_to_allocate = new_capacity * sizeof(T);
    T* new_data = static_cast<T*>(allocator->Alloc(bytes_to_allocate, alignof(T)));
    if (new_data == nullptr) [[unlikely]]
    {
        throw OutOfMemoryException(allocator->GetName(), bytes_to_allocate);
    }
    if constexpr (IsPOD<T>)
    {
        if (m_size > 0)
        {
            memcpy(new_data, m_data, m_size * sizeof(T));
        }
    }
    else
    {
        // A moved-from element is still a live object, so it is owed a destructor before its
        // storage goes back to the allocator.
        for (size_type i = 0; i < m_size; i++)
        {
            new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
            m_data[i].~T();                         // Invokes destructor on allocated memory
        }
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
    m_allocator = allocator;
}

TEMPLATE_HEADER
void CLASS_HEADER::ReserveForAppend(size_type count)
{
    const size_type needed = m_size + count;
    if (needed <= m_capacity)
    {
        return;
    }
    const size_type geometric = GetNextCapacity(m_capacity);
    Reserve(needed > geometric ? needed : geometric);
}

TEMPLATE_HEADER
template <typename ConstructGap>
typename CLASS_HEADER::iterator CLASS_HEADER::GrowAndInsert(difference_type pos_offset, size_type count, ConstructGap&& construct_gap)
{
    size_type new_capacity = GetNextCapacity(m_capacity);
    if (m_size + count > new_capacity)
    {
        new_capacity = m_size + count;
    }
    const size_type pos = static_cast<size_type>(pos_offset);
    T* new_data = Allocate(new_capacity);
    // Fill the gap before touching the old elements. Whatever the caller reads from is still
    // where it was, which is what lets a source range point into this array.
    construct_gap(new_data + pos);
    if constexpr (IsPOD<T>)
    {
        if (pos > 0)
        {
            memcpy(new_data, m_data, pos * sizeof(T));
        }
        if (m_size > pos)
        {
            memcpy(new_data + pos + count, m_data + pos, (m_size - pos) * sizeof(T));
        }
    }
    else
    {
        for (size_type i = 0; i < pos; i++)
        {
            new (&new_data[i]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
            m_data[i].~T();                         // Invokes destructor on allocated memory
        }
        for (size_type i = pos; i < m_size; i++)
        {
            new (&new_data[i + count]) T(Move(m_data[i]));  // Invokes move constructor on allocated memory
            m_data[i].~T();                                 // Invokes destructor on allocated memory
        }
    }
    Deallocate(m_data);
    m_data = new_data;
    m_capacity = new_capacity;
    m_size += count;
    return begin() + pos_offset;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::InsertCountAt(difference_type pos_offset, size_type count, const T& value)
{
    const size_type old_size = m_size;
    MakeInsertGap(pos_offset, count);
    iterator return_it = begin() + pos_offset;
    for (size_type i = 0; i < count; i++)
    {
        const size_type destination = static_cast<size_type>(pos_offset) + i;
        if (destination < old_size)
        {
            m_data[destination] = value;
        }
        else
        {
            new (&m_data[destination]) T(value);  // Invokes copy constructor on allocated memory
        }
    }
    m_size += count;
    return return_it;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, const T& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), i64{0}, cend() - cbegin());
    }
    if (ValueReadsOwnStorage(value))
    {
        // Opening the gap moves `value` out from under itself, and growing frees the storage it
        // lives in. Re-enter with a copy that lives on the stack, which the check passes over.
        T value_copy(value);
        return Insert(position, value_copy);
    }
    difference_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        Reserve(GetNextCapacity(m_capacity));
    }
    return InsertOneAt(pos_offset, value);
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(DynamicArray::const_iterator position, T&& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), i64{0}, cend() - cbegin());
    }
    if (ValueReadsOwnStorage(value))
    {
        T value_copy(Move(value));
        return Insert(position, Move(value_copy));
    }
    difference_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        Reserve(GetNextCapacity(m_capacity));
    }
    return InsertOneAt(pos_offset, Move(value));
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, size_type count, const T& value)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), i64{0}, cend() - cbegin());
    }
    if (count == 0)
    {
        return begin() + (position - cbegin());
    }
    if (ValueReadsOwnStorage(value))
    {
        // Opening the gap moves `value` out from under itself, and growing frees the storage it
        // lives in. Re-enter with a copy that lives on the stack, which the check passes over.
        T value_copy(value);
        return Insert(position, count, value_copy);
    }
    difference_type pos_offset = position - cbegin();
    if (m_size + count > m_capacity)
    {
        size_type new_capacity = GetNextCapacity(m_capacity);
        new_capacity = m_size + count > new_capacity ? m_size + count : new_capacity;
        Reserve(new_capacity);
    }
    return InsertCountAt(pos_offset, count, value);
}

TEMPLATE_HEADER
template <typename InputIt>
    requires Opal::RandomAccessIterator<InputIt>
typename CLASS_HEADER::iterator CLASS_HEADER::Insert(const_iterator position, InputIt start_it, InputIt end_it)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), i64{0}, cend() - cbegin());
    }
    if (start_it > end_it)
    {
        throw InvalidArgumentException(__FUNCTION__, "end_it - start_it", static_cast<i64>(end_it - start_it));
    }
    size_type count = static_cast<size_type>(end_it - start_it);
    if (count == 0)
    {
        return begin() + (position - cbegin());
    }
    difference_type pos_offset = position - cbegin();
    // A range that reads out of this array cannot survive the elements being shifted, so it takes
    // the growing path even when there is room, which builds the result somewhere else entirely.
    if (m_size + count > m_capacity || RangeReadsOwnStorage(start_it, end_it))
    {
        // The gap is filled from the old elements before they are moved out of the way.
        return GrowAndInsert(pos_offset, count,
                             [start_it, end_it](T* gap)
                             {
                                 size_type offset = 0;
                                 for (InputIt current = start_it; current < end_it; ++current, ++offset)
                                 {
                                     new (&gap[offset]) T(*current);  // Invokes copy constructor on allocated memory
                                 }
                             });
    }
    const size_type old_size = m_size;
    MakeInsertGap(pos_offset, count);
    iterator return_it = begin() + pos_offset;
    size_type offset = 0;
    for (InputIt current = start_it; current < end_it; ++current, ++offset)
    {
        const size_type destination = static_cast<size_type>(pos_offset) + offset;
        if (destination < old_size)
        {
            m_data[destination] = *current;
        }
        else
        {
            new (&m_data[destination]) T(*current);  // Invokes copy constructor on allocated memory
        }
    }
    m_size += count;
    return return_it;
}

TEMPLATE_HEADER
template <typename... Args>
typename CLASS_HEADER::iterator CLASS_HEADER::Emplace(const_iterator position, Args&&... args)
{
    if (position < cbegin() || position > cend()) [[unlikely]]
    {
        throw OutOfBoundsException(position - cbegin(), i64{0}, cend() - cbegin());
    }
    // The arguments are allowed to name elements of this array, and both opening the gap and
    // growing move those out from under them, so the element is built before either happens.
    T value(std::forward<Args>(args)...);
    difference_type pos_offset = position - cbegin();
    if (m_size == m_capacity)
    {
        Reserve(GetNextCapacity(m_capacity));
    }
    return InsertOneAt(pos_offset, Move(value));
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Erase(const_iterator position)
{
    if (position < cbegin() || position >= cend())
    {
        return end();
    }
    difference_type pos_offset = position - cbegin();
    iterator mut_position = begin() + pos_offset;
    // Shift the tail down onto live elements first, then destroy the vacated last slot.
    // Destroying up front and assigning into the destroyed slot would run the element's
    // assignment on an object whose lifetime has ended, which for an owning type (a
    // ScopePtr, say) frees a pointer that was already freed.
    while (mut_position < end() - 1)
    {
        *mut_position = Move(*(mut_position + 1));
        ++mut_position;
    }
    (*(end() - 1)).~T();  // Invokes destructor on allocated memory
    m_size--;
    return begin() + pos_offset;
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::EraseWithSwap(DynamicArray::const_iterator position)
{
    if (position < cbegin() || position >= cend())
    {
        return end();
    }
    iterator mut_position = begin() + (position - cbegin());
    // As in Erase: overwrite the element with the last one while both are alive, and destroy
    // the slot the last one vacated.
    if (mut_position != end() - 1)
    {
        *mut_position = Move(*(end() - 1));
        (*(end() - 1)).~T();  // Invokes destructor on allocated memory
        m_size--;
        return mut_position;
    }
    (*mut_position).~T();  // Invokes destructor on allocated memory
    m_size--;
    return end();
}

TEMPLATE_HEADER
Opal::Expected<typename CLASS_HEADER::iterator, Opal::ErrorCode> CLASS_HEADER::Erase(DynamicArray::const_iterator start_it,
                                                                                     DynamicArray::const_iterator end_it)
{
    if (start_it > end_it)
    {
        return Expected<iterator, ErrorCode>(ErrorCode::InvalidArgument);
    }
    if (start_it < cbegin() || end_it > cend())
    {
        return Expected<iterator, ErrorCode>(ErrorCode::OutOfBounds);
    }
    if (start_it == end_it)
    {
        return Expected<iterator, ErrorCode>(begin() + (start_it - cbegin()));
    }
    const difference_type start_offset = start_it - cbegin();
    const difference_type end_offset = end_it - cbegin();
    // Shift the survivors down over the erased range while every element is still alive, then
    // destroy the slots left vacated at the back. Destroying first and assigning into the
    // destroyed slots would run the element's assignment after its lifetime ended, which for
    // an owning type frees an already-freed pointer.
    iterator mut_start = begin() + start_offset;
    iterator mut_end = begin() + end_offset;
    while (mut_end < end())
    {
        *mut_start = Move(*mut_end);
        ++mut_start;
        ++mut_end;
    }
    for (iterator it = mut_start; it < end(); ++it)
    {
        (*it).~T();  // Invokes destructor on allocated memory
    }
    m_size -= Narrow<size_type>(end_offset - start_offset);
    using ReturnType = Expected<iterator, ErrorCode>;
    return ReturnType{begin() + start_offset};
}

TEMPLATE_HEADER
typename CLASS_HEADER::iterator CLASS_HEADER::Find(const T& value)
{
    return FindIf([&value](const T& element) { return element == value; });
}

TEMPLATE_HEADER
typename CLASS_HEADER::const_iterator CLASS_HEADER::Find(const T& value) const
{
    return FindIf([&value](const T& element) { return element == value; });
}

TEMPLATE_HEADER
template <typename Predicate>
typename CLASS_HEADER::iterator CLASS_HEADER::FindIf(Predicate predicate)
{
    for (size_type i = 0; i < m_size; i++)
    {
        if (predicate(m_data[i]))
        {
            return begin() + Narrow<difference_type>(i);
        }
    }
    return end();
}

TEMPLATE_HEADER
template <typename Predicate>
typename CLASS_HEADER::const_iterator CLASS_HEADER::FindIf(Predicate predicate) const
{
    for (size_type i = 0; i < m_size; i++)
    {
        if (predicate(m_data[i]))
        {
            return cbegin() + Narrow<difference_type>(i);
        }
    }
    return cend();
}

TEMPLATE_HEADER
bool CLASS_HEADER::Contains(const T& value) const
{
    return Find(value) != cend();
}

TEMPLATE_HEADER
template <typename Predicate>
typename CLASS_HEADER::size_type CLASS_HEADER::RemoveIf(Predicate predicate)
{
    // Shift the survivors down over the removed ones while every element is still alive, then
    // destroy the slots left vacated at the back, as the range Erase does.
    size_type kept = 0;
    for (size_type i = 0; i < m_size; i++)
    {
        if (predicate(m_data[i]))
        {
            continue;
        }
        if (kept != i)
        {
            m_data[kept] = Move(m_data[i]);
        }
        kept++;
    }
    const size_type removed = m_size - kept;
    if constexpr (!IsPOD<T>)
    {
        for (size_type i = kept; i < m_size; i++)
        {
            m_data[i].~T();  // Invokes destructor on allocated memory
        }
    }
    m_size = kept;
    return removed;
}

TEMPLATE_HEADER
void CLASS_HEADER::Remove(const T& value)
{
    iterator it = begin();
    while (it != end())
    {
        if (*it == value)
        {
            Erase(it);
            return;
        }
        ++it;
    }
}

TEMPLATE_HEADER
void CLASS_HEADER::RemoveWithSwap(const T& value)
{
    iterator it = begin();
    while (it != end())
    {
        if (*it == value)
        {
            EraseWithSwap(it);
            return;
        }
        ++it;
    }
}

TEMPLATE_HEADER
T* CLASS_HEADER::Allocate(size_type count)
{
    OPAL_ASSERT(m_allocator, "Allocator should never be null!");
    constexpr u64 k_alignment = alignof(T);
    const size_type bytes_to_allocate = count * sizeof(T);
    T* memory = static_cast<T*>(m_allocator->Alloc(bytes_to_allocate, k_alignment));
    if (memory == nullptr) [[unlikely]]
    {
        throw OutOfMemoryException(m_allocator->GetName(), bytes_to_allocate);
    }
    return memory;
}

TEMPLATE_HEADER
void CLASS_HEADER::Deallocate(T* ptr)
{
    if (m_allocator)
    {
        m_allocator->Free(ptr);
    }
}

TEMPLATE_HEADER
typename CLASS_HEADER::size_type CLASS_HEADER::GetNextCapacity(size_type current_capacity) const
{
    if (current_capacity == 0)
    {
        return 1;
    }
    return static_cast<size_type>((Narrow<f64>(current_capacity) * k_resize_factor) + 1.0);
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray>
#define CLASS_HEADER Opal::DynamicArrayIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DynamicArrayIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DynamicArrayIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DynamicArrayIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DynamicArrayIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    DynamicArrayIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    DynamicArrayIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return DynamicArrayIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return DynamicArrayIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const DynamicArrayIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    OPAL_ASSERT(m_ptr, "Dereferencing null pointer");
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename DynamicArrayIterator<MyArray>::difference_type n, const DynamicArrayIterator<MyArray>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray, typename BaseIterator>
#define CLASS_HEADER Opal::DynamicArrayReverseIteratorBase<MyArray, BaseIterator>

// The base sits one past the element referred to, so every comparison is the reverse of the base's
// and every step goes the other way.

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DynamicArrayReverseIteratorBase& other) const
{
    return m_base < other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DynamicArrayReverseIteratorBase& other) const
{
    return m_base <= other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DynamicArrayReverseIteratorBase& other) const
{
    return m_base > other.m_base;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DynamicArrayReverseIteratorBase& other) const
{
    return m_base >= other.m_base;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    --m_base;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    DynamicArrayReverseIteratorBase temp = *this;
    --m_base;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    ++m_base;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    DynamicArrayReverseIteratorBase temp = *this;
    ++m_base;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return DynamicArrayReverseIteratorBase(m_base - n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return DynamicArrayReverseIteratorBase(m_base + n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_base -= n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_base += n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const DynamicArrayReverseIteratorBase& other) const
{
    return other.m_base - m_base;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(*this + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    BaseIterator element = m_base;
    --element;
    return *element;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    BaseIterator element = m_base;
    --element;
    return element.operator->();
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename DynamicArrayReverseIteratorBase<MyArray, BaseIterator>::difference_type n,
                             const DynamicArrayReverseIteratorBase<MyArray, BaseIterator>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

#define TEMPLATE_HEADER template <typename MyArray>
#define CLASS_HEADER Opal::DynamicArrayConstIterator<MyArray>

TEMPLATE_HEADER
bool CLASS_HEADER::operator>(const DynamicArrayConstIterator& other) const
{
    return m_ptr > other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator>=(const DynamicArrayConstIterator& other) const
{
    return m_ptr >= other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<(const DynamicArrayConstIterator& other) const
{
    return m_ptr < other.m_ptr;
}

TEMPLATE_HEADER
bool CLASS_HEADER::operator<=(const DynamicArrayConstIterator& other) const
{
    return m_ptr <= other.m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator++()
{
    m_ptr++;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator++(int)
{
    DynamicArrayConstIterator temp = *this;
    m_ptr++;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator--()
{
    m_ptr--;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator--(int)
{
    DynamicArrayConstIterator temp = *this;
    m_ptr--;
    return temp;
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator+(difference_type n) const
{
    return DynamicArrayConstIterator(m_ptr + n);
}

TEMPLATE_HEADER
CLASS_HEADER CLASS_HEADER::operator-(difference_type n) const
{
    return DynamicArrayConstIterator(m_ptr - n);
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator+=(difference_type n)
{
    m_ptr += n;
    return *this;
}

TEMPLATE_HEADER
CLASS_HEADER& CLASS_HEADER::operator-=(difference_type n)
{
    m_ptr -= n;
    return *this;
}

TEMPLATE_HEADER
typename CLASS_HEADER::difference_type CLASS_HEADER::operator-(const DynamicArrayConstIterator& other) const
{
    return m_ptr - other.m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator[](difference_type n) const
{
    return *(m_ptr + n);
}

TEMPLATE_HEADER
typename CLASS_HEADER::reference CLASS_HEADER::operator*() const
{
    return *m_ptr;
}

TEMPLATE_HEADER
typename CLASS_HEADER::pointer CLASS_HEADER::operator->() const
{
    return m_ptr;
}

TEMPLATE_HEADER
CLASS_HEADER Opal::operator+(typename DynamicArrayConstIterator<MyArray>::difference_type n, const DynamicArrayConstIterator<MyArray>& it)
{
    return it + n;
}

#undef TEMPLATE_HEADER
#undef CLASS_HEADER

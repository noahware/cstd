#pragma once
#include "types.hpp"
#include "utility.hpp"

namespace cstd
{
	// Non-modifying sequence operations over raw-pointer ranges [first, last).
	// `find`-family returns `last` on miss (std semantics), so the results compose
	// with the erase-remove idiom and with each other.

	template <class T>
	[[nodiscard]] constexpr T* find(T* first, T* const last, const T& value)
	{
		for (; first != last; ++first)
		{
			if (*first == value)
			{
				return first;
			}
		}

		return last;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* find_if(T* first, T* const last, UnaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			if (predicate(*first))
			{
				return first;
			}
		}

		return last;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* find_if_not(T* first, T* const last, UnaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			if (!predicate(*first))
			{
				return first;
			}
		}

		return last;
	}

	template <class T>
	[[nodiscard]] constexpr T* find_first_of(T* first, T* const last, const T* s_first, const T* const s_last)
	{
		for (; first != last; ++first)
		{
			for (const T* s = s_first; s != s_last; ++s)
			{
				if (*first == *s)
				{
					return first;
				}
			}
		}

		return last;
	}

	template <class T, class BinaryPredicate>
	[[nodiscard]] constexpr T* find_first_of(T* first, T* const last, const T* s_first, const T* const s_last, BinaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			for (const T* s = s_first; s != s_last; ++s)
			{
				if (predicate(*first, *s))
				{
					return first;
				}
			}
		}

		return last;
	}

	template <class T>
	[[nodiscard]] constexpr T* adjacent_find(T* first, T* const last)
	{
		if (first == last)
		{
			return last;
		}

		for (T* next = first + 1; next != last; ++first, ++next)
		{
			if (*first == *next)
			{
				return first;
			}
		}

		return last;
	}

	template <class T, class BinaryPredicate>
	[[nodiscard]] constexpr T* adjacent_find(T* first, T* const last, BinaryPredicate predicate)
	{
		if (first == last)
		{
			return last;
		}

		for (T* next = first + 1; next != last; ++first, ++next)
		{
			if (predicate(*first, *next))
			{
				return first;
			}
		}

		return last;
	}

	template <class T>
	[[nodiscard]] constexpr size_t count(const T* first, const T* const last, const T& value)
	{
		size_t total = 0;

		for (; first != last; ++first)
		{
			if (*first == value)
			{
				++total;
			}
		}

		return total;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr size_t count_if(const T* first, const T* const last, UnaryPredicate predicate)
	{
		size_t total = 0;

		for (; first != last; ++first)
		{
			if (predicate(*first))
			{
				++total;
			}
		}

		return total;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool all_of(const T* first, const T* const last, UnaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			if (!predicate(*first))
			{
				return false;
			}
		}

		return true;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool any_of(const T* first, const T* const last, UnaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			if (predicate(*first))
			{
				return true;
			}
		}

		return false;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool none_of(const T* first, const T* const last, UnaryPredicate predicate)
	{
		return !any_of(first, last, predicate);
	}

	template <class T, class UnaryFunction>
	constexpr UnaryFunction for_each(T* first, T* const last, UnaryFunction function)
	{
		for (; first != last; ++first)
		{
			function(*first);
		}

		return function;
	}

	template <class T>
	[[nodiscard]] constexpr bool equal(const T* first1, const T* const last1, const T* first2)
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			if (!(*first1 == *first2))
			{
				return false;
			}
		}

		return true;
	}

	template <class T, class BinaryPredicate>
	[[nodiscard]] constexpr bool equal(const T* first1, const T* const last1, const T* first2, BinaryPredicate predicate)
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			if (!predicate(*first1, *first2))
			{
				return false;
			}
		}

		return true;
	}

	template <class T>
	[[nodiscard]] constexpr T* min_element(T* first, T* const last)
	{
		if (first == last)
		{
			return last;
		}

		T* smallest = first;

		for (++first; first != last; ++first)
		{
			if (*first < *smallest)
			{
				smallest = first;
			}
		}

		return smallest;
	}

	template <class T, class Compare>
	[[nodiscard]] constexpr T* min_element(T* first, T* const last, Compare compare)
	{
		if (first == last)
		{
			return last;
		}

		T* smallest = first;

		for (++first; first != last; ++first)
		{
			if (compare(*first, *smallest))
			{
				smallest = first;
			}
		}

		return smallest;
	}

	template <class T>
	[[nodiscard]] constexpr T* max_element(T* first, T* const last)
	{
		if (first == last)
		{
			return last;
		}

		T* largest = first;

		for (++first; first != last; ++first)
		{
			if (*largest < *first)
			{
				largest = first;
			}
		}

		return largest;
	}

	template <class T, class Compare>
	[[nodiscard]] constexpr T* max_element(T* first, T* const last, Compare compare)
	{
		if (first == last)
		{
			return last;
		}

		T* largest = first;

		for (++first; first != last; ++first)
		{
			if (compare(*largest, *first))
			{
				largest = first;
			}
		}

		return largest;
	}

	// Modifying sequence operations. Element-wise assignment / swap (never memcpy), so
	// they stay correct for non-trivially-copyable T. copy/transform take an output
	// pointer and return one-past-the-last written; the caller guarantees its capacity.

	template <class T>
	constexpr T* copy(const T* first, const T* const last, T* destination)
	{
		for (; first != last; ++first, ++destination)
		{
			*destination = *first;
		}

		return destination;
	}

	template <class T>
	constexpr void fill(T* first, T* const last, const T& value)
	{
		for (; first != last; ++first)
		{
			*first = value;
		}
	}

	template <class T, class U, class UnaryOperation>
	constexpr U* transform(const T* first, const T* const last, U* destination, UnaryOperation operation)
	{
		for (; first != last; ++first, ++destination)
		{
			*destination = operation(*first);
		}

		return destination;
	}

	template <class T, class U, class V, class BinaryOperation>
	constexpr V* transform(const T* first1, const T* const last1, const U* first2, V* destination, BinaryOperation operation)
	{
		for (; first1 != last1; ++first1, ++first2, ++destination)
		{
			*destination = operation(*first1, *first2);
		}

		return destination;
	}

	template <class T>
	constexpr void reverse(T* first, T* last)
	{
		while (first < last)
		{
			cstd::swap(*first, *--last);

			++first;
		}
	}

	// Stable in-place compaction: keeps the elements that do not match, returns the new
	// logical end. The physical tail is left intact (use erase / pop_back to drop it).

	template <class T>
	[[nodiscard]] constexpr T* remove(T* first, T* const last, const T& value)
	{
		T* new_end = first;

		for (; first != last; ++first)
		{
			if (!(*first == value))
			{
				if (new_end != first)
				{
					*new_end = move(*first);
				}

				++new_end;
			}
		}

		return new_end;
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* remove_if(T* first, T* const last, UnaryPredicate predicate)
	{
		T* new_end = first;

		for (; first != last; ++first)
		{
			if (!predicate(*first))
			{
				if (new_end != first)
				{
					*new_end = move(*first);
				}

				++new_end;
			}
		}

		return new_end;
	}

	template <class T, class UnaryPredicate>
	constexpr T* copy_if(const T* first, const T* const last, T* destination, UnaryPredicate predicate)
	{
		for (; first != last; ++first)
		{
			if (predicate(*first))
			{
				*destination = *first;

				++destination;
			}
		}

		return destination;
	}

	template <class T>
	constexpr T* fill_n(T* first, const size_t count, const T& value)
	{
		for (size_t i = 0; i < count; ++i, ++first)
		{
			*first = value;
		}

		return first;
	}

	template <class T>
	constexpr void replace(T* first, T* const last, const T& old_value, const T& new_value)
	{
		for (; first != last; ++first)
		{
			if (*first == old_value)
			{
				*first = new_value;
			}
		}
	}

	template <class T, class UnaryPredicate>
	constexpr void replace_if(T* first, T* const last, UnaryPredicate predicate, const T& new_value)
	{
		for (; first != last; ++first)
		{
			if (predicate(*first))
			{
				*first = new_value;
			}
		}
	}

	// Stable in-place dedup of consecutive equal runs: keeps the first of each run and
	// returns the new logical end (physical tail left intact, like remove).

	template <class T>
	[[nodiscard]] constexpr T* unique(T* first, T* const last)
	{
		if (first == last)
		{
			return last;
		}

		T* result = first;

		while (++first != last)
		{
			if (!(*result == *first))
			{
				++result;

				if (result != first)
				{
					*result = move(*first);
				}
			}
		}

		return result + 1;
	}

	template <class T, class BinaryPredicate>
	[[nodiscard]] constexpr T* unique(T* first, T* const last, BinaryPredicate predicate)
	{
		if (first == last)
		{
			return last;
		}

		T* result = first;

		while (++first != last)
		{
			if (!predicate(*result, *first))
			{
				++result;

				if (result != first)
				{
					*result = move(*first);
				}
			}
		}

		return result + 1;
	}

	// Default ordering functors (no <functional>). Heterogeneous operands so a single
	// comparator can compare, e.g., a flat_map entry against a bare key.

	struct less
	{
		template <class A, class B>
		[[nodiscard]] constexpr bool operator()(const A& left, const B& right) const
		{
			return left < right;
		}
	};

	struct greater
	{
		template <class A, class B>
		[[nodiscard]] constexpr bool operator()(const A& left, const B& right) const
		{
			return right < left;
		}
	};

	// Binary search over a sorted range. lower_bound: first position not ordered before
	// value; upper_bound: first position value is ordered before. Iterative — no recursion.

	template <class T, class V, class Compare>
	[[nodiscard]] constexpr T* lower_bound(T* first, T* const last, const V& value, Compare compare)
	{
		size_t count = static_cast<size_t>(last - first);

		while (count > 0)
		{
			const size_t half = count / 2;
			T* const middle = first + half;

			if (compare(*middle, value))
			{
				first = middle + 1;
				count -= half + 1;
			}
			else
			{
				count = half;
			}
		}

		return first;
	}

	template <class T, class V>
	[[nodiscard]] constexpr T* lower_bound(T* first, T* const last, const V& value)
	{
		return lower_bound(first, last, value, less{});
	}

	template <class T, class V, class Compare>
	[[nodiscard]] constexpr T* upper_bound(T* first, T* const last, const V& value, Compare compare)
	{
		size_t count = static_cast<size_t>(last - first);

		while (count > 0)
		{
			const size_t half = count / 2;
			T* const middle = first + half;

			if (compare(value, *middle))
			{
				count = half;
			}
			else
			{
				first = middle + 1;
				count -= half + 1;
			}
		}

		return first;
	}

	template <class T, class V>
	[[nodiscard]] constexpr T* upper_bound(T* first, T* const last, const V& value)
	{
		return upper_bound(first, last, value, less{});
	}

	template <class T, class V, class Compare>
	[[nodiscard]] constexpr bool binary_search(T* first, T* const last, const V& value, Compare compare)
	{
		T* const found = lower_bound(first, last, value, compare);

		return found != last && !compare(value, *found);
	}

	template <class T, class V>
	[[nodiscard]] constexpr bool binary_search(T* first, T* const last, const V& value)
	{
		return binary_search(first, last, value, less{});
	}

	namespace detail
	{
		template <class T, class Compare>
		constexpr void insertion_sort(T* const first, T* const last, Compare compare)
		{
			for (T* current = first + 1; current < last; ++current)
			{
				T key = move(*current);
				T* hole = current;

				while (hole > first && compare(key, *(hole - 1)))
				{
					*hole = move(*(hole - 1));
					--hole;
				}

				*hole = move(key);
			}
		}

		template <class T, class Compare>
		constexpr void sift_down(T* const first, const size_t start, const size_t count, Compare compare)
		{
			size_t root = start;

			for (;;)
			{
				size_t child = root * 2 + 1;

				if (child >= count)
				{
					break;
				}

				if (child + 1 < count && compare(first[child], first[child + 1]))
				{
					++child;
				}

				if (compare(first[root], first[child]))
				{
					cstd::swap(first[root], first[child]);

					root = child;
				}
				else
				{
					break;
				}
			}
		}

		template <class T, class Compare>
		constexpr void heap_sort(T* const first, T* const last, Compare compare)
		{
			const size_t count = static_cast<size_t>(last - first);

			for (size_t node = count / 2; node-- > 0; )
			{
				sift_down(first, node, count, compare);
			}

			for (size_t end = count; end-- > 1; )
			{
				cstd::swap(first[0], first[end]);

				sift_down(first, 0, end, compare);
			}
		}
	}

	// Unstable sort: insertion sort for small ranges, heapsort otherwise. Heapsort is
	// iterative with O(1) auxiliary space and guaranteed O(n log n) — no recursion, no
	// allocation, no worst-case blow-up.
	template <class T, class Compare>
	constexpr void sort(T* first, T* last, Compare compare)
	{
		const size_t count = static_cast<size_t>(last - first);

		if (count < 2)
		{
			return;
		}

		if (count <= 16)
		{
			detail::insertion_sort(first, last, compare);
		}
		else
		{
			detail::heap_sort(first, last, compare);
		}
	}

	template <class T>
	constexpr void sort(T* first, T* last)
	{
		sort(first, last, less{});
	}

	template <class T, class Compare>
	[[nodiscard]] constexpr bool is_sorted(T* first, T* const last, Compare compare)
	{
		if (first == last)
		{
			return true;
		}

		for (T* next = first + 1; next != last; ++next, ++first)
		{
			if (compare(*next, *first))
			{
				return false;
			}
		}

		return true;
	}

	template <class T>
	[[nodiscard]] constexpr bool is_sorted(T* first, T* const last)
	{
		return is_sorted(first, last, less{});
	}
}

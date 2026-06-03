#pragma once
#include "span.hpp"
#include "algorithm.hpp"

namespace cstd::ranges
{
	// Legacy linear search kept for backward compatibility: returns nullptr on miss.
	// New code should prefer ranges::find, which returns end (std semantics).
	template <class T>
	T* search(T* const begin, T* const end, const T& target)
	{
		T* current = begin;

		while (current < end)
		{
			if (*current == target)
			{
				return current;
			}

			++current;
		}

		return nullptr;
	}

	template <class T>
	T* search(const cstd::span<T> span, const T& target)
	{
		return search(span.begin(), span.end(), target);
	}

	// Span wrappers over the iterator-pair core in namespace cstd. Calls are fully
	// qualified with cstd:: so they reach the core rather than recursing here.

	template <class T>
	[[nodiscard]] constexpr T* find(const cstd::span<T> range, const T& value)
	{
		return cstd::find(range.begin(), range.end(), value);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* find_if(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::find_if(range.begin(), range.end(), predicate);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* find_if_not(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::find_if_not(range.begin(), range.end(), predicate);
	}

	template <class T>
	[[nodiscard]] constexpr size_t count(const cstd::span<T> range, const T& value)
	{
		return cstd::count(range.begin(), range.end(), value);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr size_t count_if(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::count_if(range.begin(), range.end(), predicate);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool all_of(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::all_of(range.begin(), range.end(), predicate);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool any_of(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::any_of(range.begin(), range.end(), predicate);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr bool none_of(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::none_of(range.begin(), range.end(), predicate);
	}

	template <class T, class UnaryFunction>
	constexpr UnaryFunction for_each(const cstd::span<T> range, UnaryFunction function)
	{
		return cstd::for_each(range.begin(), range.end(), function);
	}

	template <class T>
	[[nodiscard]] constexpr bool equal(const cstd::span<T> left, const cstd::span<T> right)
	{
		if (left.size() != right.size())
		{
			return false;
		}

		return cstd::equal(left.begin(), left.end(), right.begin());
	}

	template <class T>
	[[nodiscard]] constexpr T* min_element(const cstd::span<T> range)
	{
		return cstd::min_element(range.begin(), range.end());
	}

	template <class T, class Compare>
	[[nodiscard]] constexpr T* min_element(const cstd::span<T> range, Compare compare)
	{
		return cstd::min_element(range.begin(), range.end(), compare);
	}

	template <class T>
	[[nodiscard]] constexpr T* max_element(const cstd::span<T> range)
	{
		return cstd::max_element(range.begin(), range.end());
	}

	template <class T, class Compare>
	[[nodiscard]] constexpr T* max_element(const cstd::span<T> range, Compare compare)
	{
		return cstd::max_element(range.begin(), range.end(), compare);
	}

	template <class T>
	constexpr void fill(const cstd::span<T> range, const T& value)
	{
		cstd::fill(range.begin(), range.end(), value);
	}

	template <class T>
	constexpr void reverse(const cstd::span<T> range)
	{
		cstd::reverse(range.begin(), range.end());
	}

	template <class T>
	[[nodiscard]] constexpr T* remove(const cstd::span<T> range, const T& value)
	{
		return cstd::remove(range.begin(), range.end(), value);
	}

	template <class T, class UnaryPredicate>
	[[nodiscard]] constexpr T* remove_if(const cstd::span<T> range, UnaryPredicate predicate)
	{
		return cstd::remove_if(range.begin(), range.end(), predicate);
	}

	template <class T>
	constexpr void sort(const cstd::span<T> range)
	{
		cstd::sort(range.begin(), range.end());
	}

	template <class T, class Compare>
	constexpr void sort(const cstd::span<T> range, Compare compare)
	{
		cstd::sort(range.begin(), range.end(), compare);
	}

	template <class T>
	[[nodiscard]] constexpr T* lower_bound(const cstd::span<T> range, const T& value)
	{
		return cstd::lower_bound(range.begin(), range.end(), value);
	}

	template <class T>
	[[nodiscard]] constexpr T* upper_bound(const cstd::span<T> range, const T& value)
	{
		return cstd::upper_bound(range.begin(), range.end(), value);
	}

	template <class T>
	[[nodiscard]] constexpr bool binary_search(const cstd::span<T> range, const T& value)
	{
		return cstd::binary_search(range.begin(), range.end(), value);
	}

	template <class T>
	[[nodiscard]] constexpr bool is_sorted(const cstd::span<T> range)
	{
		return cstd::is_sorted(range.begin(), range.end());
	}
}

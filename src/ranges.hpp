#pragma once
#include "span.hpp"

namespace cstd::ranges
{
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
}

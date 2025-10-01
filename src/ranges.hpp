#pragma once

namespace cstl::ranges
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
}

#pragma once
#include "exception.hpp"
#include "crt.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T, size_t Count>
	class array
	{
	public:
		using size_type = size_t;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		[[nodiscard]] pointer begin() noexcept
		{
			return elements;
		}

		[[nodiscard]] pointer end() noexcept
		{
			return elements + Count;
		}

		[[nodiscard]] const_pointer begin() const noexcept
		{
			return elements;
		}

		[[nodiscard]] const_pointer end() const noexcept
		{
			return elements + Count;
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < Count, "[array] attempted to access outside of range of elements");

			return elements[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < Count, "[array] attempted to access outside of range of elements");

			return elements[index];
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return Count;
		}

		[[nodiscard]] pointer data() noexcept
		{
			return elements;
		}

		[[nodiscard]] const_pointer data() const noexcept
		{
			return elements;
		}

		value_type elements[Count];
	};
}

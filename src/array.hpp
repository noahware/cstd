#pragma once
#include "exception.hpp"
#include "crt.hpp"
#include "types.hpp"

namespace cstd
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

		[[nodiscard]] constexpr pointer begin() noexcept
		{
			return elements;
		}

		[[nodiscard]] constexpr pointer end() noexcept
		{
			return elements + Count;
		}

		[[nodiscard]] constexpr const_pointer begin() const noexcept
		{
			return elements;
		}

		[[nodiscard]] constexpr const_pointer end() const noexcept
		{
			return elements + Count;
		}

		[[nodiscard]] constexpr reference operator[](const size_type index)
		{
			CSTD_ASSERT(index < Count, "[array] attempted to access outside of range of elements");

			return elements[index];
		}

		[[nodiscard]] constexpr const_reference operator[](const size_type index) const
		{
			CSTD_ASSERT(index < Count, "[array] attempted to access outside of range of elements");

			return elements[index];
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return Count;
		}

		[[nodiscard]] constexpr pointer data() noexcept
		{
			return elements;
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept
		{
			return elements;
		}

		value_type elements[Count];
	};
}

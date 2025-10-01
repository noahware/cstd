#pragma once
#include "exception.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T, uint64_t Count>
	class array_t
	{
	public:
		using size_type = uint64_t;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		array_t() = default;

		explicit array_t(const value_type& default_value)
		{
			for (auto& element : elements_)
			{
				element = default_value;
			}
		}

		[[nodiscard]] pointer begin() noexcept
		{
			return elements_;
		}

		[[nodiscard]] pointer end() noexcept
		{
			return elements_ + Count;
		}

		[[nodiscard]] const_pointer begin() const noexcept
		{
			return elements_;
		}

		[[nodiscard]] const_pointer end() const noexcept
		{
			return elements_ + Count;
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < Count, "[array_t] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < Count, "[array_t] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] constexpr size_type size() noexcept
		{
			return Count;
		}

	protected:
		value_type elements_[Count];
	};
}

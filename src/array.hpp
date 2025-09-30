#pragma once
#include "types.hpp"

namespace cstl
{
	template <class T, uint64_t Count>
	class array_t
	{
	protected:
		T elements_[Count];

	public:
		using size_type = uint64_t;
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

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
			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			return elements_[index];
		}

		[[nodiscard]] constexpr size_type size() noexcept
		{
			return Count;
		}
	};
}

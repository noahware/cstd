#pragma once
#include "exception.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T>
	class span
	{
	public:
		using size_type = size_t;
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;

		constexpr span() noexcept = default;

		constexpr span(const pointer elements, const size_type count) noexcept
				:	elements_(elements),
					count_(count) { }

		constexpr span(const pointer elements_begin, const pointer elements_end) noexcept
				:	elements_(elements_begin),
					count_(static_cast<size_type>(elements_end - elements_begin)) { }

		constexpr span(vector<T>& elements) noexcept
				:	elements_(elements.begin()),
					count_(elements.size()) { }

		template <class U, size_type Count>
		constexpr span(array<U, Count>& array) noexcept
				:	elements_(array.data()),
					count_(Count) { }

		[[nodiscard]] constexpr pointer begin() const noexcept
		{
			return elements_;
		}

		[[nodiscard]] constexpr pointer end() const noexcept
		{
			return elements_ + count_;
		}

		[[nodiscard]] constexpr reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < count_, "[span] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return count_;
		}

		[[nodiscard]] constexpr pointer data() const noexcept
		{
			return elements_;
		}

	protected:
		pointer elements_ = nullptr;
		size_type count_ = 0;
	};
}

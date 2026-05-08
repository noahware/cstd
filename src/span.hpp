#pragma once
#include "exception.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T>
	class span_t
	{
	public:
		using size_type = uint64_t;
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;

		span_t() noexcept = default;

		explicit span_t(const pointer elements, const size_type count) noexcept
				:	elements_(elements),
					count_(count) { }

		explicit span_t(const pointer elements_begin, const pointer elements_end) noexcept
				:	elements_(elements_begin),
					count_(static_cast<size_type>(elements_end - elements_begin)) { }

		[[nodiscard]] pointer begin() const noexcept
		{
			return elements_;
		}

		[[nodiscard]] pointer end() const noexcept
		{
			return elements_ + count_;
		}

		[[nodiscard]] reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < count_, "[span_t] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] size_type size() const noexcept
		{
			return count_;
		}

		[[nodiscard]] pointer data() const noexcept
		{
			return elements_;
		}

	protected:
		pointer elements_ = nullptr;
		size_type count_ = 0;
	};
}

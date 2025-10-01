#pragma once
#include "exception.hpp"
#include "memory.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T>
	class vector_t
	{
	public:
		using size_type = uint64_t;
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

		vector_t() = default;

		explicit vector_t(const size_type size)
		{
			reserve(size);

			size_ = size;
		}

		[[nodiscard]] pointer begin() noexcept
		{
			return elements_;
		}

		[[nodiscard]] pointer end() noexcept
		{
			return elements_ + size_;
		}

		[[nodiscard]] const_pointer begin() const noexcept
		{
			return elements_;
		}

		[[nodiscard]] const_pointer end() const noexcept
		{
			return elements_ + size_;
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < size_, "[vector_t] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[vector_t] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] size_type size() const
		{
			return size_;
		}

		[[nodiscard]] size_type max_size() const
		{
			return max_size_;
		}

		void push_back(const_reference value)
		{
			if (size_ < max_size_)
			{
				elements_[size_++] = value;
			}
			else
			{
				grow_elements();

				push_back(value);
			}
		}

		void pop_back()
		{
			CSTL_ASSERT(size_ != 0, "[vector_t] attempted to pop back empty vector");

			size_--;
		}

		void reserve(const size_type size)
		{
			if (0 < size && max_size_ < size)
			{
				pointer const new_elements = allocate_elements(size);

				CSTL_ASSERT(new_elements != nullptr, "[vector_t] unable to reserve elements");

				copy_elements(new_elements, size_);
				discard_elements();

				elements_ = new_elements;
				max_size_ = size;
			}
		}

		void resize(const size_type new_size)
		{
			if (new_size < size_)
			{
				pointer const start = begin() + new_size;
				pointer const end = begin() + size_;

				destroy_elements(start, end);
			}
			else
			{
				if (max_size_ < new_size)
				{
					reserve(new_size);
				}

				// todo: construct new elements
			}

			size_ = new_size;
		}

	protected:

		constexpr static size_type growth_factor = 2;

		void grow_elements()
		{
			const size_type new_size = max_size_ * growth_factor;

			pointer const new_elements = allocate_elements(new_size);

			CSTL_ASSERT(new_elements != nullptr, "[vector_t] unable to grow elements");

			copy_elements(new_elements, size_);
			discard_elements();

			elements_ = new_elements;
			max_size_ = new_size;
		}

		void destroy_elements(pointer const start, pointer const end)
		{
			pointer current = start;

			while (current < end)
			{
				// todo: implement destroying of non-trivial types

				++current;
			}
		}

		void copy_elements(pointer const new_elements, const size_type copy_size)
		{
			if (elements_)
			{
				// todo: support for non-trivial types

				memory::copy(new_elements, elements_, copy_size * sizeof(T));
			}
		}

		void discard_elements()
		{
			if (elements_)
			{
				memory::free(elements_);

				elements_ = nullptr;
			}
		}

		static pointer allocate_elements(const size_type new_size)
		{
			const size_type new_elements_size = new_size * sizeof(T);

			return static_cast<T*>(memory::allocate(new_elements_size));
		}

		pointer elements_ = nullptr;

		size_type size_ = 0;
		size_type max_size_ = 0;
	};
}

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
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		vector_t() = default;

		explicit vector_t(const size_type size)
		{
			reserve(size);

			size_ = size;
		}

		~vector_t()
		{
			discard_elements();

			size_ = 0;
			max_size_ = 0;
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

		[[nodiscard]] pointer front()
		{
			CSTL_ASSERT(!empty(), "[vector_t] front() called on an empty vector");

			return begin();
		}

		[[nodiscard]] const_pointer front() const
		{
			CSTL_ASSERT(!empty(), "[vector_t] front() called on an empty vector");

			return begin();
		}

		[[nodiscard]] pointer back()
		{
			CSTL_ASSERT(!empty(), "[vector_t] back() called on an empty vector");

			return end() - 1;
		}

		[[nodiscard]] const_pointer back() const
		{
			CSTL_ASSERT(!empty(), "[vector_t] back() called on an empty vector");

			return end() - 1;
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

		[[nodiscard]] pointer data() const
		{
			return elements_;
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

				deconstruct_elements(start, end);
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

		void clear()
		{
			size_ = 0;
		}

		[[nodiscard]] bool empty() const
		{
			return size_ == 0;
		}

	protected:

		constexpr static size_type growth_factor = 2;

		void grow_elements()
		{
			const size_type new_size = max_size_ ? max_size_ * growth_factor : 1;

			pointer const new_elements = allocate_elements(new_size);

			CSTL_ASSERT(new_elements != nullptr, "[vector_t] unable to grow elements");

			copy_elements(new_elements, size_);
			discard_elements();

			elements_ = new_elements;
			max_size_ = new_size;
		}

		void deconstruct_elements(pointer const start, pointer const end)
		{
			pointer current = start;

			while (current < end)
			{
				// todo: implement deconstructing of non-trivial types

				++current;
			}
		}

		void copy_elements(pointer const new_elements, const size_type copy_size)
		{
			if (elements_)
			{
				// todo: support for non-trivial types

				memory::copy(new_elements, elements_, copy_size * sizeof(value_type));
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
			if (new_size == 0)
			{
				return nullptr;
			}

			const size_type new_elements_size = new_size * sizeof(value_type);

			return static_cast<pointer>(memory::allocate(new_elements_size));
		}

		pointer elements_ = nullptr;

		size_type size_ = 0;
		size_type max_size_ = 0;
	};
}

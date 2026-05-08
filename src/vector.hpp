#pragma once
#include "exception.hpp"
#include "initializer_list.hpp"
#include "type_traits.hpp"
#include "crt.hpp"
#include "types.hpp"

namespace cstl
{
	template <class T>
	class vector
	{
	public:
		using size_type = size_t;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using rvalue_reference = value_type&&;

		vector() = default;

		explicit vector(const size_type size, const_reference default_value = T())
		{
			reserve(size);

			for (size_type i = 0; i < size; i++)
			{
				push_back(default_value);
			}
		}

		template <class Y>
		vector(const initializer_list<Y> list)
		{
			reserve(list.size());

			for (const Y& entry : list)
			{
				push_back(static_cast<value_type>(entry));
			}
		}

		vector(const vector&) = delete;
		vector& operator=(const vector&) = delete;

		vector(vector&& right) noexcept
				:	elements_(right.elements_),
					size_(right.size_),
					max_size_(right.max_size_)
		{
			right.elements_ = nullptr;
			right.size_ = 0;
			right.max_size_ = 0;
		}

		vector& operator=(vector&& right) noexcept
		{
			if (this != &right)
			{
				discard_elements();

				elements_ = right.elements_;
				size_ = right.size_;
				max_size_ = right.max_size_;

				right.elements_ = nullptr;
				right.size_ = 0;
				right.max_size_ = 0;
			}

			return *this;
		}

		~vector()
		{
			discard_elements();
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

		[[nodiscard]] reference front()
		{
			CSTL_ASSERT(!empty(), "[vector] front() called on an empty vector");

			return *begin();
		}

		[[nodiscard]] const_reference front() const
		{
			CSTL_ASSERT(!empty(), "[vector] front() called on an empty vector");

			return *begin();
		}

		[[nodiscard]] reference back()
		{
			CSTL_ASSERT(!empty(), "[vector] back() called on an empty vector");

			return *(end() - 1);
		}

		[[nodiscard]] const_reference back() const
		{
			CSTL_ASSERT(!empty(), "[vector] back() called on an empty vector");

			return *(end() - 1);
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < size_, "[vector] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[vector] attempted to access outside of range of elements");

			return elements_[index];
		}

		[[nodiscard]] size_type size() const noexcept
		{
			return size_;
		}

		[[nodiscard]] size_type max_size() const noexcept
		{
			return max_size_;
		}

		[[nodiscard]] pointer data() noexcept
		{
			return elements_;
		}

		[[nodiscard]] const_pointer data() const noexcept
		{
			return elements_;
		}

		void push_back(const_reference value)
		{
			if (max_size_ <= size_)
			{
				grow_elements();
			}

			new (elements_ + size_) T(value);

			++size_;
		}

		void push_back(rvalue_reference value)
		{
			if (max_size_ <= size_)
			{
				grow_elements();
			}

			new (elements_ + size_) T(static_cast<rvalue_reference>(value));

			++size_;
		}

		void pop_back()
		{
			CSTL_ASSERT(size_ != 0, "[vector] attempted to pop back empty vector");

			size_--;

			const auto current = elements_ + size_;

			deconstruct_elements(current, current + 1);
		}

		void reserve(const size_type size)
		{
			if (0 < size && max_size_ < size)
			{
				pointer const new_elements = allocate_elements(size);

				CSTL_ASSERT(new_elements != nullptr, "[vector] unable to reserve elements");

				move_elements(new_elements);
				free_elements();

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

				for (size_type i = size_; i < new_size; ++i)
				{
					new (elements_ + i) T();
				}
			}

			size_ = new_size;
		}

		void clear() noexcept
		{
			deconstruct_elements(begin(), end());

			size_ = 0;
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return size_ == 0;
		}

	protected:
		constexpr static size_type growth_factor = 2;

		void grow_elements()
		{
			const size_type new_size = max_size_ ? max_size_ * growth_factor : 1;

			pointer const new_elements = allocate_elements(new_size);

			CSTL_ASSERT(new_elements != nullptr, "[vector] unable to grow elements");

			move_elements(new_elements);
			free_elements();

			elements_ = new_elements;
			max_size_ = new_size;
		}

		void deconstruct_elements(pointer const start, pointer const end)
		{
			if constexpr (!is_trivially_destructible_v<T>)
			{
				for (pointer current = start; current < end; ++current)
				{
					current->~T();
				}
			}
		}

		void move_elements(pointer const new_elements)
		{
			if (!elements_)
			{
				return;
			}

			if constexpr (is_trivially_copyable_v<T>)
			{
				crt::memcpy(new_elements, elements_, size_ * sizeof(value_type));
			}
			else
			{
				for (size_type i = 0; i < size_; ++i)
				{
					new (new_elements + i) T(static_cast<rvalue_reference>(elements_[i]));

					elements_[i].~T();
				}
			}
		}

		void free_elements()
		{
			if (elements_)
			{
				crt::free(elements_);

				elements_ = nullptr;
			}
		}

		void discard_elements()
		{
			deconstruct_elements(begin(), end());
			free_elements();

			size_ = 0;
			max_size_ = 0;
		}

		static pointer allocate_elements(const size_type new_size)
		{
			if (new_size == 0)
			{
				return nullptr;
			}

			const size_type new_elements_size = new_size * sizeof(value_type);

			return static_cast<pointer>(crt::malloc(new_elements_size));
		}

		pointer elements_ = nullptr;

		size_type size_ = 0;
		size_type max_size_ = 0;
	};
}

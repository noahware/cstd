#pragma once
#include "types.hpp"

namespace cstl
{
	template <class T>
	class base_ptr_t
	{
	public:
		using size_type = size_t;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		base_ptr_t() noexcept = default;

		explicit base_ptr_t(const pointer object)
				:	object_(object) { }

		[[nodiscard]] pointer get() const noexcept
		{
			return object_;
		}

		[[nodiscard]] reference operator*() const noexcept
		{
			return *object_;
		}

		[[nodiscard]] pointer operator->() const noexcept
		{
			return object_;
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return object_ != nullptr;
		}

	protected:
		pointer object_ = nullptr;
	};

	template <class T>
	class unique_ptr_t : public base_ptr_t<T>
	{
	public:
		using pointer = typename base_ptr_t<T>::pointer;

		unique_ptr_t() noexcept = default;

		explicit unique_ptr_t(const pointer object)
				:	base_ptr_t<T>(object) { }

		unique_ptr_t(const unique_ptr_t&) = delete;
		unique_ptr_t& operator=(const unique_ptr_t&) = delete;

		unique_ptr_t(unique_ptr_t&& right) noexcept
				:	base_ptr_t<T>(right.object_)
		{
			right.object_ = nullptr;
		}

		unique_ptr_t& operator=(unique_ptr_t&& right) noexcept
		{
			if (this != &right)
			{
				reset();

				this->object_ = right.object_;

				right.object_ = nullptr;
			}

			return *this;
		}

		~unique_ptr_t() noexcept
		{
			reset();
		}

		void reset() noexcept
		{
			if (this->object_)
			{
				delete this->object_;

				this->object_ = nullptr;
			}
		}
	};

	template <class T>
	class shared_ptr_t : public base_ptr_t<T>
	{
	public:
		using size_type = typename base_ptr_t<T>::size_type;
		using pointer = typename base_ptr_t<T>::pointer;

		shared_ptr_t() noexcept = default;
		
		explicit shared_ptr_t(const pointer object, size_type* const reference_count)
				:	base_ptr_t<T>(object),
					reference_count_(reference_count) { }

		shared_ptr_t(const shared_ptr_t& right)
				:	base_ptr_t<T>(right.object_),
					reference_count_(right.reference_count_)
		{
			++(*reference_count_);
		}

		shared_ptr_t& operator=(const shared_ptr_t& right)
		{
			if (this != &right)
			{
				reset();

				this->object_ = right.object_;
				reference_count_ = right.reference_count_;

				++(*reference_count_);
			}

			return *this;
		}

		shared_ptr_t(shared_ptr_t&& right) noexcept
				:	base_ptr_t<T>(right.object_),
					reference_count_(right.reference_count_)
		{
			right.object_ = nullptr;
			right.reference_count_ = nullptr;
		}

		shared_ptr_t& operator=(shared_ptr_t&& right) noexcept
		{
			if (this != &right)
			{
				reset();

				this->object_ = right.object_;
				reference_count_ = right.reference_count_;

				right.object_ = nullptr;
				right.reference_count_ = nullptr;
			}

			return *this;
		}

		~shared_ptr_t() noexcept
		{
			reset();
		}

		void reset() noexcept
		{
			if (reference_count_)
			{
				if (--(*reference_count_) == 0)
				{
					delete this->object_;
					delete reference_count_;
				}

				this->object_ = nullptr;
				reference_count_ = nullptr;
			}
		}

	protected:
		// todo: make cstl::atomic<size_type>*
		size_type* reference_count_ = nullptr;
	};

	template <class T, class... Args>
	unique_ptr_t<T> make_unique(Args... args)
	{
		T* object = new T(args...);

		return unique_ptr_t<T>(object);
	}

	template <class T, class... Args>
	shared_ptr_t<T> make_shared(Args... args)
	{
		T* object = new T(args...);
		auto* count = new typename shared_ptr_t<T>::size_type(1);

		return shared_ptr_t<T>(object, count);
	}
}

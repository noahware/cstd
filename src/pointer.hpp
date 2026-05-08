#pragma once
#include "types.hpp"

namespace cstl
{
	template <class T>
	class base_ptr
	{
	public:
		using size_type = size_t;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		base_ptr() noexcept = default;

		explicit base_ptr(const pointer object)
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
	class unique_ptr : public base_ptr<T>
	{
	public:
		using pointer = typename base_ptr<T>::pointer;

		unique_ptr() noexcept = default;

		explicit unique_ptr(const pointer object)
				:	base_ptr<T>(object) { }

		// add derived class support to cast
		template <class U> friend class unique_ptr;

		template <class U>
		unique_ptr(unique_ptr<U>&& right) noexcept
				:	base_ptr<T>(right.object_)
		{
			right.object_ = nullptr;
		}

		unique_ptr(const unique_ptr&) = delete;
		unique_ptr& operator=(const unique_ptr&) = delete;

		unique_ptr(unique_ptr&& right) noexcept
				:	base_ptr<T>(right.object_)
		{
			right.object_ = nullptr;
		}

		unique_ptr& operator=(unique_ptr&& right) noexcept
		{
			if (this != &right)
			{
				reset();

				this->object_ = right.object_;

				right.object_ = nullptr;
			}

			return *this;
		}

		~unique_ptr() noexcept
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
	class shared_ptr : public base_ptr<T>
	{
	public:
		using size_type = typename base_ptr<T>::size_type;
		using pointer = typename base_ptr<T>::pointer;

		shared_ptr() noexcept = default;
		
		explicit shared_ptr(const pointer object, size_type* const reference_count)
				:	base_ptr<T>(object),
					reference_count_(reference_count) { }

		shared_ptr(const shared_ptr& right)
				:	base_ptr<T>(right.object_),
					reference_count_(right.reference_count_)
		{
			++(*reference_count_);
		}

		// add derived class support to cast
		template <class U> friend class shared_ptr;

		template <class U>
		shared_ptr(const shared_ptr<U>& right) noexcept
				:	shared_ptr(right.object_, right.reference_count_) { }

		template <class U>
		shared_ptr(shared_ptr<U>&& right) noexcept
				:	base_ptr<T>(right.object_),
					reference_count_(right.reference_count_)
		{
			right.object_ = nullptr;
			right.reference_count_ = nullptr;
		}

		shared_ptr& operator=(const shared_ptr& right)
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

		shared_ptr(shared_ptr&& right) noexcept
				:	base_ptr<T>(right.object_),
					reference_count_(right.reference_count_)
		{
			right.object_ = nullptr;
			right.reference_count_ = nullptr;
		}

		shared_ptr& operator=(shared_ptr&& right) noexcept
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

		~shared_ptr() noexcept
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
	unique_ptr<T> make_unique(Args... args)
	{
		T* object = new T(args...);

		return unique_ptr<T>(object);
	}

	template <class T, class... Args>
	shared_ptr<T> make_shared(Args... args)
	{
		T* object = new T(args...);
		auto* count = new typename shared_ptr<T>::size_type(1);

		return shared_ptr<T>(object, count);
	}
}

#pragma once
#include "exception.hpp"
#include "utility.hpp"

namespace cstd
{
	struct nullopt_type
	{
		struct _null { };

		explicit constexpr nullopt_type(_null) {}
	};

	inline constexpr nullopt_type nullopt{nullopt_type::_null{}};

	template <class T>
	class optional
	{
	public:
		using value_type = T;

		// The payload lives in an anonymous union, so a disengaged optional holds no
		// constructed T. value_ is brought to life with placement-new only while
		// engaged and explicitly destroyed on reset — never default-constructed.
		constexpr optional() noexcept {}
		constexpr optional(nullopt_type) noexcept {}

		constexpr optional(T value) noexcept
				:	has_value_(true),
					value_(move(value)) { }

		optional(const optional& right)
		{
			if (right.has_value_)
			{
				new (&value_) value_type(right.value_);

				has_value_ = true;
			}
		}

		optional(optional&& right) noexcept
		{
			if (right.has_value_)
			{
				new (&value_) value_type(move(right.value_));

				has_value_ = true;

				right.reset();
			}
		}

		optional& operator=(const optional& right)
		{
			if (this != &right)
			{
				if (right.has_value_)
				{
					assign(right.value_);
				}
				else
				{
					reset();
				}
			}

			return *this;
		}

		optional& operator=(optional&& right) noexcept
		{
			if (this != &right)
			{
				if (right.has_value_)
				{
					assign(move(right.value_));

					right.reset();
				}
				else
				{
					reset();
				}
			}

			return *this;
		}

		optional& operator=(T value)
		{
			assign(move(value));

			return *this;
		}

		optional& operator=(nullopt_type)
		{
			reset();

			return *this;
		}

		constexpr ~optional()
		{
			reset();
		}

		constexpr void reset()
		{
			if (has_value_)
			{
				value_.~value_type();

				has_value_ = false;
			}
		}

		template <class... Args>
		value_type& emplace(Args&&... args)
		{
			reset();

			new (&value_) value_type(forward<Args>(args)...);

			has_value_ = true;

			return value_;
		}

		[[nodiscard]] value_type* operator->() noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return &value_;
		}

		[[nodiscard]] const value_type* operator->() const noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return &value_;
		}

		[[nodiscard]] constexpr value_type& operator*() noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return value_;
		}

		[[nodiscard]] constexpr const value_type& operator*() const noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return value_;
		}

		[[nodiscard]] constexpr value_type& value() noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return value_;
		}

		[[nodiscard]] constexpr const value_type& value() const noexcept
		{
			CSTD_ASSERT(has_value_, "[optional] accessing non populated value");

			return value_;
		}

		[[nodiscard]] constexpr bool has_value() const noexcept
		{
			return has_value_;
		}

		[[nodiscard]] explicit constexpr operator bool() const noexcept
		{
			return has_value_;
		}

		[[nodiscard]] constexpr value_type value_or(value_type default_value) const&
		{
			return has_value_ ? value_ : move(default_value);
		}

		[[nodiscard]] constexpr value_type value_or(value_type default_value) &&
		{
			return has_value_ ? move(value_) : move(default_value);
		}

	protected:
		void assign(const value_type& incoming)
		{
			if (has_value_)
			{
				value_ = incoming;
			}
			else
			{
				new (&value_) value_type(incoming);

				has_value_ = true;
			}
		}

		void assign(value_type&& incoming)
		{
			if (has_value_)
			{
				value_ = move(incoming);
			}
			else
			{
				new (&value_) value_type(move(incoming));

				has_value_ = true;
			}
		}

		bool has_value_ = false;

		union
		{
			value_type value_;
		};
	};
}

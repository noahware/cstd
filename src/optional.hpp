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

		constexpr optional() noexcept = default;
		constexpr optional(nullopt_type) noexcept {}

		constexpr optional(T value) noexcept
				:	has_value_(true),
					value_(move(value)) { }

		constexpr optional(const optional& right) noexcept
				:	has_value_(right.has_value_),
					value_(right.value_) { }

		constexpr optional& operator=(T value) noexcept
		{
			value_ = move(value);
			has_value_ = true;

			return *this;
		}

		constexpr optional& operator=(nullopt_type) noexcept
		{
			reset();

			return *this;
		}

		constexpr optional& operator=(const optional& right) noexcept
		{
			has_value_ = right.has_value_;
			value_ = right.value_;

			return *this;
		}

		constexpr optional(optional&& right) noexcept
				:	has_value_(right.has_value_),
					value_(move(right.value_))
		{
			right.reset();
		}

		constexpr optional& operator=(optional&& right) noexcept
		{
			if (this != &right)
			{
				value_ = move(right.value_);
				has_value_ = right.has_value_;

				right.reset();
			}

			return *this;
		}

		constexpr void reset()
		{
			value_ = { };
			has_value_ = false;
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

	protected:
		bool has_value_ = false;
		value_type value_ = { };
	};
}

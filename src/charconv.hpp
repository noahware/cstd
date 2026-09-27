#pragma once
#include "types.hpp"
#include "type_traits.hpp"

namespace cstd
{
	struct from_chars_result
	{
		const char* ptr;
		int ec;		// 0 on success, non-zero on failure
	};

	namespace detail
	{
		[[nodiscard]] constexpr int digit_value(const char ch, const int base) noexcept
		{
			int value = -1;

			if (ch >= '0' && ch <= '9')
			{
				value = ch - '0';
			}
			else if (ch >= 'a' && ch <= 'z')
			{
				value = ch - 'a' + 10;
			}
			else if (ch >= 'A' && ch <= 'Z')
			{
				value = ch - 'A' + 10;
			}

			if (value >= base)
			{
				return -1;
			}

			return value;
		}

		// Maximum value of an unsigned type, computed without pulling in <limits>.
		template <class T>
		inline constexpr T unsigned_max = static_cast<T>(~T(0));
	}

	// Parses an unsigned or signed integer from [first, last) in the given base (2–36).
	// Returns a from_chars_result whose .ptr points one past the last consumed character.
	// If no digits are consumed, .ptr == first and .ec != 0.
	// Overflow sets .ec != 0 and .ptr to the point of overflow.

	template <class T>
	[[nodiscard]] constexpr from_chars_result from_chars(const char* first, const char* last, T& value, const int base = 10) noexcept
	{
		static_assert(is_integral_v<T>, "from_chars: T must be an integral type");

		if (first == last)
		{
			return { first, 1 };
		}

		const char* pos = first;

		// Handle sign for signed types.
		bool negative = false;

		if constexpr (is_signed_v<T>)
		{
			if (*pos == '-')
			{
				negative = true;
				++pos;

				if (pos == last)
				{
					return { first, 1 };
				}
			}
		}

		// We accumulate in an unsigned type to avoid signed overflow UB.
		using unsigned_t = conditional_t<is_signed_v<T>,
			conditional_t<sizeof(T) == 1, uint8_t,
			conditional_t<sizeof(T) == 2, uint16_t,
			conditional_t<sizeof(T) == 4, uint32_t, uint64_t>>>,
			T>;

		const unsigned_t ubase = static_cast<unsigned_t>(base);
		unsigned_t result = 0;
		bool any_digits = false;

		while (pos != last)
		{
			const int digit = detail::digit_value(*pos, base);

			if (digit < 0)
			{
				break;
			}

			const unsigned_t next = static_cast<unsigned_t>(result * ubase + static_cast<unsigned_t>(digit));

			// Overflow check: the new value must be >= the old (wraps on overflow).
			if (any_digits && next / ubase != result)
			{
				return { pos, 1 };
			}

			result = next;
			any_digits = true;
			++pos;
		}

		if (!any_digits)
		{
			return { first, 1 };
		}

		if constexpr (is_signed_v<T>)
		{
			if (negative)
			{
				// For signed types the minimum negative magnitude is one more than
				// the maximum positive, so we check the unsigned result against the
				// limit before negating.
				constexpr unsigned_t magnitude_limit = static_cast<unsigned_t>(
					detail::unsigned_max<unsigned_t> / 2 + 1);

				if (result > magnitude_limit)
				{
					return { pos, 1 };
				}

				value = static_cast<T>(-static_cast<T>(result));
			}
			else
			{
				constexpr unsigned_t positive_limit = static_cast<unsigned_t>(
					detail::unsigned_max<unsigned_t> / 2);

				if (result > positive_limit)
				{
					return { pos, 1 };
				}

				value = static_cast<T>(result);
			}
		}
		else
		{
			value = result;
		}

		return { pos, 0 };
	}
}

#pragma once
#include "types.hpp"

namespace cstd
{
	template <class T>
	struct numeric_limits
	{
		[[nodiscard]] static constexpr T min() noexcept { return T(); }
		[[nodiscard]] static constexpr T max() noexcept { return T(); }

		static constexpr bool is_signed = false;
		static constexpr int digits = 0;
	};

	namespace detail
	{
		// Ranges are computed from the type's own width (types.hpp asserts 8-bit
		// bytes), so the fixed-width aliases resolve correctly on any platform.
		template <class T>
		struct unsigned_limits
		{
			[[nodiscard]] static constexpr T min() noexcept { return static_cast<T>(0); }
			[[nodiscard]] static constexpr T max() noexcept { return static_cast<T>(~static_cast<T>(0)); }

			static constexpr bool is_signed = false;
			static constexpr int digits = static_cast<int>(sizeof(T) * 8);
		};

		template <class T>
		struct signed_limits
		{
			[[nodiscard]] static constexpr T max() noexcept
			{
				return static_cast<T>((static_cast<unsigned long long>(1) << (sizeof(T) * 8 - 1)) - 1);
			}

			[[nodiscard]] static constexpr T min() noexcept
			{
				return static_cast<T>(-max() - 1);
			}

			static constexpr bool is_signed = true;
			static constexpr int digits = static_cast<int>(sizeof(T) * 8 - 1);
		};
	}

	template <>
	struct numeric_limits<bool>
	{
		[[nodiscard]] static constexpr bool min() noexcept { return false; }
		[[nodiscard]] static constexpr bool max() noexcept { return true; }

		static constexpr bool is_signed = false;
		static constexpr int digits = 1;
	};

	template <> struct numeric_limits<signed char>        : detail::signed_limits<signed char> { };
	template <> struct numeric_limits<short>              : detail::signed_limits<short> { };
	template <> struct numeric_limits<int>                : detail::signed_limits<int> { };
	template <> struct numeric_limits<long>               : detail::signed_limits<long> { };
	template <> struct numeric_limits<long long>          : detail::signed_limits<long long> { };

	template <> struct numeric_limits<unsigned char>      : detail::unsigned_limits<unsigned char> { };
	template <> struct numeric_limits<unsigned short>     : detail::unsigned_limits<unsigned short> { };
	template <> struct numeric_limits<unsigned int>       : detail::unsigned_limits<unsigned int> { };
	template <> struct numeric_limits<unsigned long>      : detail::unsigned_limits<unsigned long> { };
	template <> struct numeric_limits<unsigned long long> : detail::unsigned_limits<unsigned long long> { };
}

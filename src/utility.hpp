#pragma once
#include "type_traits.hpp"

namespace cstl
{
	template <class T>
	struct remove_reference
	{
		using type = T;
	};

	template <class T>
	struct remove_reference<T&>
	{
		using type = T;
	};

	template <class T>
	struct remove_reference<T&&>
	{
		using type = T;
	};

	template <typename T>
	using remove_reference_t = typename remove_reference<T>::type;

	template <class T, class Y>
	[[nodiscard]] constexpr T bit_cast(const Y& object) noexcept
	{
		static_assert(sizeof(T) == sizeof(Y), "bit_cast types differ in size");
		static_assert(is_trivially_copyable_v<T> && is_trivially_copyable_v<Y>,
			"bit_cast requires types to be trivially copyable");

		return __builtin_bit_cast(T, object);
	}

	template <typename T>
	constexpr remove_reference_t<T>&& move(T&& object) noexcept
	{
		return static_cast<remove_reference_t<T>&&>(object);
	}
}

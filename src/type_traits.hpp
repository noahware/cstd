#pragma once

namespace cstl
{
    template <class T>
    inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(T);

    template <class T>
    inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(T);

    template <class T, class U>
    struct is_same
	{
        static constexpr bool value = false;
    };

    template <class T>
    struct is_same<T, T>
	{
        static constexpr bool value = true;
    };

    template <class T, class U>
    inline constexpr bool is_same_v = is_same<T, U>::value;
}

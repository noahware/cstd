#pragma once

namespace cstl
{
    template <class T>
    inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(T);

    template <class T>
    inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(T);

    template <class T>
    struct is_integral
    {
        static constexpr bool value = false;
    };

    template <> struct is_integral<bool>               { static constexpr bool value = true; };
    template <> struct is_integral<char>               { static constexpr bool value = true; };
    template <> struct is_integral<signed char>        { static constexpr bool value = true; };
    template <> struct is_integral<unsigned char>      { static constexpr bool value = true; };
    template <> struct is_integral<char8_t>            { static constexpr bool value = true; };
    template <> struct is_integral<char16_t>           { static constexpr bool value = true; };
    template <> struct is_integral<char32_t>           { static constexpr bool value = true; };
    template <> struct is_integral<wchar_t>            { static constexpr bool value = true; };
    template <> struct is_integral<short>              { static constexpr bool value = true; };
    template <> struct is_integral<unsigned short>     { static constexpr bool value = true; };
    template <> struct is_integral<int>                { static constexpr bool value = true; };
    template <> struct is_integral<unsigned int>       { static constexpr bool value = true; };
    template <> struct is_integral<long>               { static constexpr bool value = true; };
    template <> struct is_integral<unsigned long>      { static constexpr bool value = true; };
    template <> struct is_integral<long long>          { static constexpr bool value = true; };
    template <> struct is_integral<unsigned long long> { static constexpr bool value = true; };

    template <class T>
    inline constexpr bool is_integral_v = is_integral<T>::value;

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

    template <bool Condition, class T, class F>
    struct conditional
    {
        using type = T;
    };

    template <class T, class F>
    struct conditional<false, T, F>
    {
        using type = F;
    };

    template <bool Condition, class T, class F>
    using conditional_t = typename conditional<Condition, T, F>::type;
}

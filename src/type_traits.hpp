#pragma once

namespace cstd
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

    template <class T, T Value>
    struct integral_constant
    {
        using value_type = T;
        static constexpr T value = Value;
    };

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template <bool Condition, class T = void>
    struct enable_if { };

    template <class T>
    struct enable_if<true, T>
    {
        using type = T;
    };

    template <bool Condition, class T = void>
    using enable_if_t = typename enable_if<Condition, T>::type;

    template <class...>
    using void_t = void;

    template <class T> struct remove_const { using type = T; };
    template <class T> struct remove_const<const T> { using type = T; };
    template <class T> using remove_const_t = typename remove_const<T>::type;

    template <class T> struct remove_volatile { using type = T; };
    template <class T> struct remove_volatile<volatile T> { using type = T; };
    template <class T> using remove_volatile_t = typename remove_volatile<T>::type;

    template <class T> struct remove_cv { using type = remove_volatile_t<remove_const_t<T>>; };
    template <class T> using remove_cv_t = typename remove_cv<T>::type;

    template <class T> struct remove_pointer { using type = T; };
    template <class T> struct remove_pointer<T*> { using type = T; };
    template <class T> struct remove_pointer<T* const> { using type = T; };
    template <class T> struct remove_pointer<T* volatile> { using type = T; };
    template <class T> struct remove_pointer<T* const volatile> { using type = T; };
    template <class T> using remove_pointer_t = typename remove_pointer<T>::type;

    template <class T> struct is_lvalue_reference : false_type { };
    template <class T> struct is_lvalue_reference<T&> : true_type { };
    template <class T> inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

    template <class T> struct is_rvalue_reference : false_type { };
    template <class T> struct is_rvalue_reference<T&&> : true_type { };
    template <class T> inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

    template <class T> struct is_reference : false_type { };
    template <class T> struct is_reference<T&> : true_type { };
    template <class T> struct is_reference<T&&> : true_type { };
    template <class T> inline constexpr bool is_reference_v = is_reference<T>::value;

    template <class T> struct is_const : false_type { };
    template <class T> struct is_const<const T> : true_type { };
    template <class T> inline constexpr bool is_const_v = is_const<T>::value;

    template <class T> struct is_void : integral_constant<bool, is_same_v<remove_cv_t<T>, void>> { };
    template <class T> inline constexpr bool is_void_v = is_void<T>::value;

    template <class T> struct is_pointer : false_type { };
    template <class T> struct is_pointer<T*> : true_type { };
    template <class T> struct is_pointer<T* const> : true_type { };
    template <class T> struct is_pointer<T* volatile> : true_type { };
    template <class T> struct is_pointer<T* const volatile> : true_type { };
    template <class T> inline constexpr bool is_pointer_v = is_pointer<T>::value;

    namespace detail
    {
        template <class T> struct type_identity { using type = T; };

        template <class T> type_identity<T&>  add_lvalue_ref(int);
        template <class T> type_identity<T>   add_lvalue_ref(long);

        template <class T> type_identity<T&&> add_rvalue_ref(int);
        template <class T> type_identity<T>   add_rvalue_ref(long);
    }

    template <class T> struct add_lvalue_reference : decltype(detail::add_lvalue_ref<T>(0)) { };
    template <class T> using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    template <class T> struct add_rvalue_reference : decltype(detail::add_rvalue_ref<T>(0)) { };
    template <class T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    template <class T>
    add_rvalue_reference_t<T> declval() noexcept;

    template <class T> struct decay { using type = remove_cv_t<T>; };
    template <class T> struct decay<T&> { using type = remove_cv_t<T>; };
    template <class T> struct decay<T&&> { using type = remove_cv_t<T>; };
    template <class T> using decay_t = typename decay<T>::type;

    template <class Base, class Derived>
    inline constexpr bool is_base_of_v = __is_base_of(Base, Derived);

    template <class Base, class Derived>
    struct is_base_of : integral_constant<bool, __is_base_of(Base, Derived)> { };

    template <class T, class... Args>
    inline constexpr bool is_constructible_v = __is_constructible(T, Args...);

    template <class T, class... Args>
    struct is_constructible : integral_constant<bool, __is_constructible(T, Args...)> { };

    template <class T> inline constexpr bool is_default_constructible_v = __is_constructible(T);
    template <class T> inline constexpr bool is_copy_constructible_v = __is_constructible(T, add_lvalue_reference_t<const T>);
    template <class T> inline constexpr bool is_move_constructible_v = __is_constructible(T, add_rvalue_reference_t<T>);

    template <class To, class From>
    inline constexpr bool is_assignable_v = __is_assignable(To, From);

    template <class T> inline constexpr bool is_copy_assignable_v = __is_assignable(add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>);
    template <class T> inline constexpr bool is_move_assignable_v = __is_assignable(add_lvalue_reference_t<T>, add_rvalue_reference_t<T>);

    namespace detail
    {
        template <class To> void convertible_sink(To) noexcept;

        template <class From, class To, class = void>
        struct is_convertible_impl : false_type { };

        template <class From, class To>
        struct is_convertible_impl<From, To, void_t<decltype(convertible_sink<To>(declval<From>()))>> : true_type { };
    }

    template <class From, class To>
    inline constexpr bool is_convertible_v = detail::is_convertible_impl<From, To>::value;

    template <class From, class To>
    struct is_convertible : integral_constant<bool, is_convertible_v<From, To>> { };
}

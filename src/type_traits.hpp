#pragma once

namespace cstl
{
    template <class T>
    inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(T);

    template <class T>
    inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(T);
}

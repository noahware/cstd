#include "array.hpp"
#include "flat_map.hpp"
#include "vector.hpp"

cstl::int32_t main()
{
	constexpr cstl::array_t<cstl::int32_t, 4> x = { 0, 1, 2, 3 };

	(void)x;

	cstl::vector_t<cstl::uint64_t> y = cstl::initializer_list_t({ 1, 2, 3 });

	y.push_back(4);

	const cstl::flat_map_t<cstl::int32_t, bool> z = cstl::initializer_list_t({ cstl::pair_t(1, true), cstl::pair_t(2, false), cstl::pair_t(3, true) });

	(void)z;

	return 0;
}

#include "ranges.hpp"
#include "vector.hpp"

cstl::int32_t main()
{
	cstl::vector_t<cstl::int32_t> x(3);

	x[0] = 1;
	x[1] = 2;
	x[2] = 4;

	const auto it = cstl::ranges::search(x.begin(), x.end(), 4);

	(void)it;

	return 0;
}

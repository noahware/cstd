#include "vector.hpp"

cstl::int32_t main()
{
	cstl::vector_t<cstl::int32_t> x(3);

	x[3] = 0; // causes assertion

	x.push_back(0);

	x[3] = 0; // does not cause assertion

	cstl::vector_t<cstl::uint64_t> y;

	y.reserve(5);

	for (cstl::uint64_t i = 0; i < 8; i++)
	{
		y.push_back(i);
	}

	y.pop_back();

	y.resize(2);

	return 0;
}

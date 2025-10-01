#include "flat_map.hpp"
#include "vector.hpp"

cstl::int32_t main()
{
	cstl::flat_map_t<cstl::uint32_t, const char*> x;

	x[5] = "hello";
	x[12] = "world";

	const char* output = x[5];

	(void)output;
		
	return 0;
}

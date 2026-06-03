#include "optional.hpp"
#include "types.hpp"

cstl::int32_t main()
{
	cstl::optional<cstl::int32_t> x(5);

	x = cstl::nullopt;

	if (x)
	{
		return *x * 2;
	}

	x = 9;

	return *x;
}

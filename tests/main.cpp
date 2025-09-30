#include "array.hpp"
#include "memory.hpp"

cstl::int32_t main()
{
	cstl::array_t<int, 3> integer_array;

	for (int& x : integer_array)
	{
		x = 3;
	}

	integer_array[1] = 0;

	const cstl::array_t<float, 3> float_array(5.f);

	for (const float& x : float_array)
	{
		(void)x;
	}

	const float first_float = *float_array.begin();

	(void)first_float;

	return 0;
}

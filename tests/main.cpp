#include "memory.hpp"

cstl::int32_t main()
{
	void* const buffer = cstl::memory::allocate(0x1000);

	cstl::memory::free(buffer);

	return 0;
}

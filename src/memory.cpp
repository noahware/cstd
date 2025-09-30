#include "memory.hpp"
#include <cstdlib>

void* cstl::memory::allocate(const uint64_t size)
{
	return ::malloc(size);
}

void cstl::memory::free(void* const buffer)
{
	return ::free(buffer);
}

#include "memory.hpp"
#include <cstdlib>
#include <intrin.h>

void* cstl::memory::allocate(const uint64_t size)
{
	return ::malloc(size);
}

void cstl::memory::free(void* const buffer)
{
	return ::free(buffer);
}

void cstl::memory::copy(void* const destination, const void* const source, const uint64_t size)
{
	__movsb(static_cast<uint8_t*>(destination), static_cast<const uint8_t*>(source), size);
}

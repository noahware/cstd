#include "crt.hpp"
#include <cstdlib>
#include <cstring>
#include <intrin.h>

void* cstl::mem::allocate(const uint64_t size)
{
	return ::malloc(size);
}

void cstl::mem::free(void* const buffer)
{
	return ::free(buffer);
}

void cstl::mem::copy(void* const destination, const void* const source, const uint64_t size)
{
	__movsb(static_cast<uint8_t*>(destination), static_cast<const uint8_t*>(source), size);
}

cstl::uint64_t cstl::str::length(const char* const str)
{
	return strlen(str);
}

cstl::uint64_t cstl::str::wlength(const wchar_t* const str)
{
	return wcslen(str);
}

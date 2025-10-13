#include "crt.hpp"
#include <Windows.h>

void* cstl::crt::malloc(const uint64_t size)
{
	return ::malloc(size);
}

void cstl::crt::free(void* const buffer)
{
	::free(buffer);
}

void cstl::crt::memcpy(void* const destination, const void* const source, const uint64_t size)
{
	__movsb(static_cast<uint8_t*>(destination), static_cast<const uint8_t*>(source), size);
}

cstl::uint64_t cstl::crt::strlen(const char* const str)
{
	return ::strlen(str);
}

cstl::uint64_t cstl::crt::wcslen(const wchar_t* const str)
{
	return ::wcslen(str);
}

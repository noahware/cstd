#include "crt.hpp"
#include <intrin.h>
#include <cstring>

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

cstl::int32_t cstl::crt::strcmp(const char* first_str, const char* second_str)
{
	return ::strcmp(first_str, second_str);
}

cstl::int32_t cstl::crt::wcscmp(const wchar_t* first_str, const wchar_t* second_str)
{
	return ::wcscmp(first_str, second_str);
}

cstl::int32_t cstl::crt::strncmp(const char* first_str, const char* second_str, uint64_t size)
{
	return ::strncmp(first_str, second_str, size);
}

cstl::int32_t cstl::crt::wcsncmp(const wchar_t* first_str, const wchar_t* second_str, uint64_t size)
{
	return ::wcsncmp(first_str, second_str, size);
}

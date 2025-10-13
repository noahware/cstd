#pragma once
#include "types.hpp"

namespace cstl::crt
{
	void* malloc(uint64_t size);
	void free(void* buffer);
	void memcpy(void* destination, const void* source, uint64_t size);

	uint64_t strlen(const char* str);
	uint64_t wcslen(const wchar_t* str);
}

#pragma once
#include "types.hpp"

namespace cstl::crt
{
	void* malloc(uint64_t size);
	void free(void* buffer);
	void memcpy(void* destination, const void* source, uint64_t size);

	uint64_t strlen(const char* str);
	uint64_t wcslen(const wchar_t* str);

	int32_t strcmp(const char* first_str, const char* second_str);
	int32_t wcscmp(const wchar_t* first_str, const wchar_t* second_str);

	int32_t strncmp(const char* first_str, const char* second_str, uint64_t size);
	int32_t wcsncmp(const wchar_t* first_str, const wchar_t* second_str, uint64_t size);
}

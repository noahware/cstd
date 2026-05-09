#pragma once
#include "types.hpp"

namespace cstl::crt
{
	void* malloc(size_t size);
	void free(void* buffer);
	void memcpy(void* destination, const void* source, size_t size);

	size_t strlen(const char* str);
	size_t wcslen(const wchar_t* str);

	int32_t strcmp(const char* first_str, const char* second_str);
	int32_t wcscmp(const wchar_t* first_str, const wchar_t* second_str);

	int32_t strncmp(const char* first_str, const char* second_str, size_t size);
	int32_t wcsncmp(const wchar_t* first_str, const wchar_t* second_str, size_t size);

	const char* strchr(const char* str, char target);
	const wchar_t* wcschr(const wchar_t* str, wchar_t target);

	const char* strstr(const char* str, const char* substr);
	const wchar_t* wcsstr(const wchar_t* str, const wchar_t* second_str);
}

#pragma once
#include "types.hpp"

void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new(size_t size, void* buffer) noexcept;
void* operator new[](size_t size, void* buffer) noexcept;
void operator delete(void* buffer) noexcept;
void operator delete(void* buffer, size_t size) noexcept;

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

	const char* strchr(const char* str, char target);
	const wchar_t* wcschr(const wchar_t* str, wchar_t target);

	const char* strstr(const char* str, const char* substr);
	const wchar_t* wcsstr(const wchar_t* str, const wchar_t* second_str);
}

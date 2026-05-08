#include "crt.hpp"
#include <Windows.h>

void* operator new(const size_t size)
{
	void* const buffer = cstl::crt::malloc(size);

	return buffer;
}

void* operator new[](const size_t size)
{
	void* const buffer = cstl::crt::malloc(size);

	return buffer;
}

void* operator new([[maybe_unused]] const size_t size, void* const buffer) noexcept
{
	return buffer;
}

void* operator new[]([[maybe_unused]] const size_t size, void* const buffer) noexcept
{
	return buffer;
}

void operator delete(void* buffer) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstl::crt::free(buffer);
}

void operator delete(void* buffer, [[maybe_unused]] const size_t size) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstl::crt::free(buffer);
}

void* cstl::crt::malloc(const size_t size)
{
	return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void cstl::crt::free(void* const buffer)
{
	VirtualFree(buffer, 0, MEM_RELEASE);
}

void cstl::crt::memcpy(void* const destination, const void* const source, const size_t size)
{
	__movsb(static_cast<uint8_t*>(destination), static_cast<const uint8_t*>(source), size);
}

cstl::size_t cstl::crt::strlen(const char* const str)
{
	const char* current = str;

	while (*current)
	{
		current++;
	}

	return current - str;
}

cstl::size_t cstl::crt::wcslen(const wchar_t* const str)
{
	const wchar_t* current = str;

	while (*current)
	{
		current++;
	}

	return current - str;
}

cstl::int32_t cstl::crt::strcmp(const char* first_str, const char* second_str)
{
	while (*first_str != 0 && *first_str == *second_str)
	{
		first_str++;
		second_str++;
	}

	return static_cast<cstl::uint8_t>(*first_str) - static_cast<cstl::uint8_t>(*second_str);
}

cstl::int32_t cstl::crt::wcscmp(const wchar_t* first_str, const wchar_t* second_str)
{
	while (*first_str != 0 && *first_str == *second_str)
	{
		first_str++;
		second_str++;
	}

	return static_cast<cstl::uint32_t>(*first_str) - static_cast<cstl::uint32_t>(*second_str);
}

cstl::int32_t cstl::crt::strncmp(const char* first_str, const char* second_str, size_t size)
{
	while (0 < size--)
	{
		const cstl::uint8_t first = static_cast<cstl::uint8_t>(*first_str++);
		const cstl::uint8_t second = static_cast<cstl::uint8_t>(*second_str++);

		if (first != second)
		{
			return first - second;
		}

		if (first == '\0')
		{
			return 0;
		}
	}

	return 0;
}

cstl::int32_t cstl::crt::wcsncmp(const wchar_t* first_str, const wchar_t* second_str, size_t size)
{
	while (0 < size--)
	{
		const cstl::uint32_t first = static_cast<cstl::uint32_t>(*first_str++);
		const cstl::uint32_t second = static_cast<cstl::uint32_t>(*second_str++);

		if (first != second)
		{
			return first - second;
		}

		if (first == '\0')
		{
			return 0;
		}
	}

	return 0;
}

const char* cstl::crt::strchr(const char* str, const char target)
{
	do
	{
		if (*str == target)
		{
			return str;
		}
	} while (*str++);

	return nullptr;
}

const wchar_t* cstl::crt::wcschr(const wchar_t* str, const wchar_t target)
{
	do
	{
		if (*str == target)
		{
			return str;
		}
	} while (*str++);

	return nullptr;
}

const char* cstl::crt::strstr(const char* const str, const char* const substr)
{
	const size_t length = strlen(substr);

	if (!length)
	{
		return str;
	}

	for (const char* i = str; (i = strchr(i, *substr)) != nullptr; i++)
	{
		if (!strncmp(i, substr, length))
		{
			return i;
		}
	}

	return nullptr;
}

const wchar_t* cstl::crt::wcsstr(const wchar_t* const str, const wchar_t* const substr)
{
	const size_t length = wcslen(substr);

	if (!length)
	{
		return str;
	}

	for (const wchar_t* i = str; (i = wcschr(i, *substr)) != nullptr; i++)
	{
		if (!wcsncmp(i, substr, length))
		{
			return i;
		}
	}

	return nullptr;
}

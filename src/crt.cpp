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

void operator delete[](void* buffer) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstl::crt::free(buffer);
}

void operator delete[](void* buffer, [[maybe_unused]] const size_t size) noexcept
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

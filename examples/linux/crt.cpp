#include "crt.hpp"
#include <sys/mman.h>
#include <string.h>

namespace
{
	struct alloc_header
	{
		cstd::size_t size;
	};
}

void* operator new(const size_t size)
{
	return cstd::crt::malloc(size);
}

void* operator new[](const size_t size)
{
	return cstd::crt::malloc(size);
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

	cstd::crt::free(buffer);
}

void operator delete(void* buffer, [[maybe_unused]] const size_t size) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstd::crt::free(buffer);
}

void operator delete[](void* buffer) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstd::crt::free(buffer);
}

void operator delete[](void* buffer, [[maybe_unused]] const size_t size) noexcept
{
	if (!buffer)
	{
		return;
	}

	cstd::crt::free(buffer);
}

void* cstd::crt::malloc(const size_t size)
{
	const size_t total = size + sizeof(alloc_header);
	void* const ptr = mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (ptr == MAP_FAILED)
	{
		return nullptr;
	}

	auto* const header = static_cast<alloc_header*>(ptr);
	header->size = total;

	return header + 1;
}

void cstd::crt::free(void* const buffer)
{
	auto* const header = static_cast<alloc_header*>(buffer) - 1;
	munmap(header, header->size);
}

void cstd::crt::memcpy(void* const destination, const void* const source, const size_t size)
{
	::memcpy(destination, source, size);
}

void cstd::crt::memset(void* const destination, const int32_t value, const size_t size)
{
	::memset(destination, value, size);
}

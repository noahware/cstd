#pragma once
#include "types.hpp"

namespace cstl
{
	namespace mem
	{
		void* allocate(uint64_t size);
		void free(void* buffer);

		void copy(void* destination, const void* source, uint64_t size);
	}

	namespace str
	{
		uint64_t length(const char* str);
		uint64_t wlength(const wchar_t* str);
	}
}

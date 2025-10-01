#pragma once
#include "types.hpp"

namespace cstl
{
	namespace memory
	{
		void* allocate(uint64_t size);

		void free(void* buffer);

		void copy(void* destination, const void* source, uint64_t size);
	}
}

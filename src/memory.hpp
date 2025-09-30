#pragma once
#include "types.hpp"

namespace cstl
{
	namespace memory
	{
		void* allocate(uint64_t size);

		void free(void* buffer);
	}
}

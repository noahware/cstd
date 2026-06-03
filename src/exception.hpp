#pragma once

#if defined(_MSC_VER)
	#define CSTL_TRAP() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
	#define CSTL_TRAP() __builtin_trap()
#else
	#define CSTL_TRAP() (*static_cast<volatile int*>(nullptr) = 0)
#endif

#define CSTL_ASSERT(condition, info_str) do { if (!(condition)) { CSTL_TRAP(); } } while (0)

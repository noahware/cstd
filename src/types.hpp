#pragma once
#include <cstdint>
#include <cstddef>

namespace cstl
{
	using std::int8_t;
	using std::int16_t;
	using std::int32_t;
	using std::int64_t;
	using std::uint8_t;
	using std::uint16_t;
	using std::uint32_t;
	using std::uint64_t;

	using std::size_t;

	// std::byte is a C++17 enum class for raw memory, not an arithmetic type, so
	// keep our own arithmetic byte.
	typedef unsigned char byte;

	// <cstdint> already guarantees the exact-width integers are exactly that wide
	// (or absent, which is a compile error). The one assumption it does not cover:
	// the containers do pointer arithmetic through size_t, so it must be pointer-
	// width.
	static_assert(sizeof(size_t) == sizeof(void*), "cstl: size_t must match pointer width");
}

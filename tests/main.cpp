#include "string.hpp"
#include "hash.hpp"
#include "types.hpp"

cstl::int32_t main()
{
	constexpr cstl::string_view value = "nah";
	constexpr cstl::string_view value2 = "nah";

	constexpr auto hash = cstl::hash<cstl::string_view>();
	constexpr auto result = hash(value2);

	return hash(value) == result;
}

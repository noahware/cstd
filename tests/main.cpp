#include "array.hpp"
#include "string_view.hpp"

cstl::int32_t main()
{
	const cstl::string_view_t string_view = "hello world!";

	const char* raw_string = string_view.data();
	const char raw_char = raw_string[0];

	(void)raw_char;

	return 0;
}

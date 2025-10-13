#include "string.hpp"

cstl::int32_t main()
{
	const cstl::string_t string = "hello world!";

	const char* raw_string = string.data();
	const char raw_char = string[3];

	(void)raw_string;
	(void)raw_char;

	return string == "hello world!";
}

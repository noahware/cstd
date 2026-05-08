#include "pointer.hpp"
#include "string.hpp"

cstl::int32_t main()
{
	cstl::vector<cstl::unique_ptr<int>> pointers;

	pointers.push_back(cstl::make_unique<int>(7)); // will become 8 after the += 1

	for (const auto& element : pointers)
	{
		*element += 1;
	}

	pointers.push_back(cstl::make_unique<int>(6));

	return *(pointers[0]) + *(pointers[1]); // 8 + 6 = 14
}

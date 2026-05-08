#pragma once

namespace cstl
{
	template <class First, class Second>
	class pair
	{
	public:
		using first_type = First;
		using second_type = Second;

		pair() = default;

		explicit pair(const first_type& first_in, const second_type& second_in)
			:	first(first_in),
				second(second_in) {}

		first_type first;
		second_type second;
	};
}

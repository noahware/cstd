#pragma once
#include "exception.hpp"
#include "crt.hpp"

namespace cstl
{
	template <class T>
	class base_string_view_t
	{
	public:
		using size_type = uint64_t;
		using character_type = T;
		using pointer = character_type*;
		using const_pointer = const character_type*;
		using reference = character_type&;
		using const_reference = const character_type&;

		base_string_view_t() = default;

		explicit base_string_view_t(const_pointer const data, const size_type size)
				:	data_(data),
					size_(size) { }

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < size_, "[base_string_view_t] attempted to access outside of range of data");

			return data_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[base_string_view_t] attempted to access outside of range of data");

			return data_[index];
		}

		[[nodiscard]] const_pointer data() const
		{
			return data_;
		}

		[[nodiscard]] size_type size() const
		{
			return size_;
		}

		[[nodiscard]] bool empty() const
		{
			return size_ == 0;
		}

	private:
		const_pointer data_ = nullptr;
		size_type size_ = 0;
	};

	class string_view_t : public base_string_view_t<char>
	{
	public:
		using character_type = char;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		string_view_t() = default;

		string_view_t(const_pointer const data)
				:	base_string_view_t(data, str::length(data)) { }
	};

	class wstring_view_t : base_string_view_t<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		wstring_view_t() = default;

		explicit wstring_view_t(const_pointer const data)
			: base_string_view_t(data, str::wlength(data)) {}
	};
}

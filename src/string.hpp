#pragma once
#include "crt.hpp"
#include "exception.hpp"

namespace cstl
{
	template <class T>
	class base_string_t
	{
	public:
		using size_type = uint64_t;
		using character_type = T;
		using pointer = character_type*;
		using const_pointer = const character_type*;
		using reference = character_type&;
		using const_reference = const character_type&;

		base_string_t() = default;

		explicit base_string_t(const_pointer const data, const size_type size)
				:	data_(static_cast<pointer>(mem::allocate((size + 1) * sizeof(T)))),
					size_(size)
		{
			CSTL_ASSERT(data_ != nullptr, "[base_string_t] unable to allocate memory for string");

			mem::copy(data_, data, (size + 1) * sizeof(T));
		}

		[[nodiscard]] pointer begin() noexcept
		{
			return data_;
		}

		[[nodiscard]] pointer end() noexcept
		{
			return data_ + size_;
		}

		[[nodiscard]] const_pointer begin() const noexcept
		{
			return data_;
		}

		[[nodiscard]] const_pointer end() const noexcept
		{
			return data_ + size_;
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < size_, "[base_string_t] attempted to access outside of range of data");

			return data_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[base_string_t] attempted to access outside of range of data");

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
		pointer data_ = nullptr;
		size_type size_ = 0;
	};

	class string_t : public base_string_t<char>
	{
	public:
		using character_type = char;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		string_t() = default;

		string_t(const_pointer const data)
				:	base_string_t(data, str::length(data)) {}
	};

	class wstring_t : public base_string_t<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		wstring_t() = default;

		wstring_t(const_pointer const data)
				:	base_string_t(data, str::wlength(data)) {}
	};
}

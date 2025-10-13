#pragma once
#include "crt.hpp"
#include "vector.hpp"
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
				:	elements_(size + 1)
		{
			crt::memcpy(elements_.data(), data, size * sizeof(T));

			elements_[size] = T('\0');
		}

		[[nodiscard]] pointer begin() noexcept
		{
			return elements_.begin();
		}

		[[nodiscard]] pointer end() noexcept
		{
			return elements_.begin() + size();
		}

		[[nodiscard]] const_pointer begin() const noexcept
		{
			return elements_.begin();
		}

		[[nodiscard]] const_pointer end() const noexcept
		{
			return elements_.begin() + size();
		}

		[[nodiscard]] reference operator[](const size_type index)
		{
			CSTL_ASSERT(index < size(), "[base_string_t] attempted to access outside of range of data");

			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size(), "[base_string_t] attempted to access outside of range of data");

			return elements_[index];
		}

		[[nodiscard]] pointer data() noexcept
		{
			return elements_.data();
		}

		[[nodiscard]] const_pointer data() const noexcept
		{
			return elements_.data();
		}

		[[nodiscard]] size_type size() const
		{
			return elements_.size() - 1;
		}

		[[nodiscard]] bool empty() const
		{
			return size() == 0;
		}

	private:
		vector_t<character_type> elements_;
	};

	class string_t : public base_string_t<char>
	{
	public:
		using character_type = char;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		string_t() = default;

		string_t(const_pointer const data)
				:	base_string_t(data, crt::strlen(data)) {}

		string_t(const_pointer const data, const size_type size)
				:	base_string_t(data, size) {}
	};

	class wstring_t : public base_string_t<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		wstring_t() = default;

		wstring_t(const_pointer const data)
				:	base_string_t(data, crt::wcslen(data)) {}

		wstring_t(const_pointer const data, const size_type size)
				:	base_string_t(data, size) {}
	};
}

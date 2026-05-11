#pragma once
#include "crt.hpp"
#include "vector.hpp"
#include "exception.hpp"
#include "string_view.hpp"

namespace cstl
{
	template <class T>
	class base_string
	{
	public:
		using size_type = size_t;
		using character_type = T;
		using pointer = character_type*;
		using const_pointer = const character_type*;
		using reference = character_type&;
		using const_reference = const character_type&;

		constexpr static size_type npos = static_cast<size_type>(-1);

		base_string()  noexcept = default;

		explicit base_string(const_pointer const data, const size_type size) noexcept
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
			CSTL_ASSERT(index < size(), "[base_string] attempted to access outside of range of data");

			return elements_[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size(), "[base_string] attempted to access outside of range of data");

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

	protected:
		vector<character_type> elements_;
	};

	class string : public base_string<char>
	{
	public:
		using character_type = char;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		string() noexcept = default;

		string(const_pointer const data) noexcept
				:	base_string(data, crt::strlen(data)) {}

		string(const_pointer const data, const size_type size) noexcept
				:	base_string(data, size) {}

		[[nodiscard]] size_type find(const string_view& substr, const size_type offset = 0) const
		{
			const const_pointer found = crt::strstr(data() + offset, substr.data());

			if (found == nullptr)
			{
				return npos;
			}

			return found - data();
		}

		[[nodiscard]] bool contains(const string_view& substr) const
		{
			return find(substr) != npos;
		}

		bool operator==(const string_view& right) const
		{
			return crt::strcmp(data(), right.data()) == 0;
		}
	};

	class wstring : public base_string<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		wstring() noexcept = default;

		wstring(const_pointer const data) noexcept
				:	base_string(data, crt::wcslen(data)) {}

		wstring(const_pointer const data, const size_type size) noexcept
				:	base_string(data, size) {}

		[[nodiscard]] size_type find(const wstring_view& substr, const size_type offset = 0) const
		{
			const const_pointer found = crt::wcsstr(data() + offset, substr.data());

			if (found == nullptr)
			{
				return npos;
			}

			return found - data();
		}

		[[nodiscard]] bool contains(const wstring_view& substr) const
		{
			return find(substr) != npos;
		}

		bool operator==(const wstring_view& right) const
		{
			return crt::wcscmp(data(), right.data()) == 0;
		}
	};
}

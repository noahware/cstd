#pragma once
#include "crt.hpp"
#include "exception.hpp"

namespace cstl
{
	template <class T>
	class base_string_view
	{
	public:
		using size_type = size_t;
		using character_type = T;
		using pointer = character_type*;
		using const_pointer = const character_type*;
		using reference = character_type&;
		using const_reference = const character_type&;

		constexpr static size_type npos = static_cast<size_type>(-1);

		constexpr base_string_view() noexcept = default;

		constexpr explicit base_string_view(const_pointer const data, const size_type size) noexcept
				:	data_(data),
					size_(size) {}

		[[nodiscard]] constexpr const_pointer begin() const noexcept
		{
			return data_;
		}

		[[nodiscard]] constexpr const_pointer end() const noexcept
		{
			return data_ + size_;
		}

		[[nodiscard]] constexpr const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[base_string_view] attempted to access outside of range of data");

			return data_[index];
		}

		[[nodiscard]] constexpr const_pointer data() const
		{
			return data_;
		}

		[[nodiscard]] constexpr size_type size() const
		{
			return size_;
		}

		[[nodiscard]] constexpr bool empty() const
		{
			return size_ == 0;
		}

	protected:
		const_pointer data_ = nullptr;
		size_type size_ = 0;
	};

	class string_view : public base_string_view<char>
	{
	public:
		using character_type = char;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		constexpr string_view() noexcept = default;

		constexpr string_view(const_pointer const data) noexcept
				:	base_string_view(data, crt::strlen(data)) {}

		constexpr string_view(const_pointer const data, const size_type size) noexcept
				:	base_string_view(data, size) {}

		[[nodiscard]] constexpr size_type find(const string_view& substr, const size_type offset = 0) const
		{
			const const_pointer found = crt::strstr(data() + offset, substr.data());

			if (found == nullptr)
			{
				return npos;
			}

			return found - data();
		}

		[[nodiscard]] constexpr bool contains(const string_view& substr) const
		{
			return find(substr) != npos;
		}

		[[nodiscard]] constexpr bool operator==(const string_view& right) const
		{
			return crt::strcmp(data(), right.data()) == 0;
		}
	};

	class wstring_view : public base_string_view<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		constexpr wstring_view() noexcept = default;

		constexpr wstring_view(const_pointer const data) noexcept
				:	base_string_view(data, crt::wcslen(data)) {}

		constexpr wstring_view(const_pointer const data, const size_type size) noexcept
				:	base_string_view(data, size) {}

		[[nodiscard]] constexpr size_type find(const wstring_view& substr, const size_type offset = 0) const
		{
			const const_pointer found = crt::wcsstr(data() + offset, substr.data());

			if (found == nullptr)
			{
				return npos;
			}

			return found - data();
		}

		[[nodiscard]] constexpr bool contains(const wstring_view& substr) const
		{
			return find(substr) != npos;
		}

		[[nodiscard]] constexpr bool operator==(const wstring_view& right) const
		{
			return crt::wcscmp(data(), right.data()) == 0;
		}
	};
}

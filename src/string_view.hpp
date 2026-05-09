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

		base_string_view() = default;

		explicit base_string_view(const_pointer const data, const size_type size)
				:	data_(data),
					size_(size) {}

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
			CSTL_ASSERT(index < size_, "[base_string_view] attempted to access outside of range of data");

			return const_cast<pointer>(data_)[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const
		{
			CSTL_ASSERT(index < size_, "[base_string_view] attempted to access outside of range of data");

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

		string_view() = default;

		string_view(const_pointer const data)
				:	base_string_view(data, crt::strlen(data)) {}

		string_view(const_pointer const data, const size_type size)
				:	base_string_view(data, size) {}

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

	class wstring_view : public base_string_view<wchar_t>
	{
	public:
		using character_type = wchar_t;
		using pointer = character_type*;
		using const_pointer = const character_type*;

		wstring_view() = default;

		wstring_view(const_pointer const data)
				:	base_string_view(data, crt::wcslen(data)) {}

		wstring_view(const_pointer const data, const size_type size)
				:	base_string_view(data, size) {}

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

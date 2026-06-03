#pragma once
#include "crt.hpp"
#include "exception.hpp"

namespace cstd
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
			CSTD_ASSERT(index < size_, "[base_string_view] attempted to access outside of range of data");

			return data_[index];
		}

		[[nodiscard]] constexpr const_reference front() const
		{
			CSTD_ASSERT(size_ != 0, "[base_string_view] front() called on an empty view");

			return data_[0];
		}

		[[nodiscard]] constexpr const_reference back() const
		{
			CSTD_ASSERT(size_ != 0, "[base_string_view] back() called on an empty view");

			return data_[size_ - 1];
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

		constexpr void remove_prefix(const size_type count)
		{
			CSTD_ASSERT(count <= size_, "[base_string_view] remove_prefix count exceeds size");

			data_ += count;
			size_ -= count;
		}

		constexpr void remove_suffix(const size_type count)
		{
			CSTD_ASSERT(count <= size_, "[base_string_view] remove_suffix count exceeds size");

			size_ -= count;
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

		[[nodiscard]] constexpr size_type rfind(const string_view& needle, const size_type offset = npos) const
		{
			if (needle.size() > size())
			{
				return npos;
			}

			size_type last_start = size() - needle.size();

			if (offset < last_start)
			{
				last_start = offset;
			}

			for (size_type i = last_start + 1; i-- > 0; )
			{
				if (crt::strncmp(data() + i, needle.data(), needle.size()) == 0)
				{
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_type find_first_of(const string_view& set, const size_type offset = 0) const
		{
			for (size_type i = offset; i < size(); ++i)
			{
				for (size_type j = 0; j < set.size(); ++j)
				{
					if ((*this)[i] == set[j])
					{
						return i;
					}
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr string_view substr(const size_type position = 0, size_type count = npos) const
		{
			CSTD_ASSERT(position <= size(), "[string_view] substr position out of range");

			const size_type available = size() - position;

			if (count > available)
			{
				count = available;
			}

			return string_view(data() + position, count);
		}

		[[nodiscard]] constexpr bool contains(const string_view& substr) const
		{
			return find(substr) != npos;
		}

		[[nodiscard]] constexpr bool starts_with(const string_view& prefix) const
		{
			return prefix.size() <= size() && crt::strncmp(data(), prefix.data(), prefix.size()) == 0;
		}

		[[nodiscard]] constexpr bool starts_with(const character_type character) const
		{
			return !empty() && front() == character;
		}

		[[nodiscard]] constexpr bool ends_with(const string_view& suffix) const
		{
			return suffix.size() <= size()
				&& crt::strncmp(data() + (size() - suffix.size()), suffix.data(), suffix.size()) == 0;
		}

		[[nodiscard]] constexpr bool ends_with(const character_type character) const
		{
			return !empty() && back() == character;
		}

		[[nodiscard]] constexpr bool operator==(const string_view& right) const
		{
			return size() == right.size() && crt::strncmp(data(), right.data(), size()) == 0;
		}

		[[nodiscard]] constexpr bool operator!=(const string_view& right) const
		{
			return !(*this == right);
		}

		[[nodiscard]] constexpr bool operator<(const string_view& right) const
		{
			const size_type min_size = size() < right.size() ? size() : right.size();
			const int32_t result = crt::strncmp(data(), right.data(), min_size);

			if (result != 0)
			{
				return result < 0;
			}

			return size() < right.size();
		}

		[[nodiscard]] constexpr bool operator>(const string_view& right) const
		{
			return right < *this;
		}

		[[nodiscard]] constexpr bool operator<=(const string_view& right) const
		{
			return !(right < *this);
		}

		[[nodiscard]] constexpr bool operator>=(const string_view& right) const
		{
			return !(*this < right);
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

		[[nodiscard]] constexpr size_type rfind(const wstring_view& needle, const size_type offset = npos) const
		{
			if (needle.size() > size())
			{
				return npos;
			}

			size_type last_start = size() - needle.size();

			if (offset < last_start)
			{
				last_start = offset;
			}

			for (size_type i = last_start + 1; i-- > 0; )
			{
				if (crt::wcsncmp(data() + i, needle.data(), needle.size()) == 0)
				{
					return i;
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr size_type find_first_of(const wstring_view& set, const size_type offset = 0) const
		{
			for (size_type i = offset; i < size(); ++i)
			{
				for (size_type j = 0; j < set.size(); ++j)
				{
					if ((*this)[i] == set[j])
					{
						return i;
					}
				}
			}

			return npos;
		}

		[[nodiscard]] constexpr wstring_view substr(const size_type position = 0, size_type count = npos) const
		{
			CSTD_ASSERT(position <= size(), "[wstring_view] substr position out of range");

			const size_type available = size() - position;

			if (count > available)
			{
				count = available;
			}

			return wstring_view(data() + position, count);
		}

		[[nodiscard]] constexpr bool contains(const wstring_view& substr) const
		{
			return find(substr) != npos;
		}

		[[nodiscard]] constexpr bool starts_with(const wstring_view& prefix) const
		{
			return prefix.size() <= size() && crt::wcsncmp(data(), prefix.data(), prefix.size()) == 0;
		}

		[[nodiscard]] constexpr bool starts_with(const character_type character) const
		{
			return !empty() && front() == character;
		}

		[[nodiscard]] constexpr bool ends_with(const wstring_view& suffix) const
		{
			return suffix.size() <= size()
				&& crt::wcsncmp(data() + (size() - suffix.size()), suffix.data(), suffix.size()) == 0;
		}

		[[nodiscard]] constexpr bool ends_with(const character_type character) const
		{
			return !empty() && back() == character;
		}

		[[nodiscard]] constexpr bool operator==(const wstring_view& right) const
		{
			return size() == right.size() && crt::wcsncmp(data(), right.data(), size()) == 0;
		}

		[[nodiscard]] constexpr bool operator!=(const wstring_view& right) const
		{
			return !(*this == right);
		}

		[[nodiscard]] constexpr bool operator<(const wstring_view& right) const
		{
			const size_type min_size = size() < right.size() ? size() : right.size();
			const int32_t result = crt::wcsncmp(data(), right.data(), min_size);

			if (result != 0)
			{
				return result < 0;
			}

			return size() < right.size();
		}

		[[nodiscard]] constexpr bool operator>(const wstring_view& right) const
		{
			return right < *this;
		}

		[[nodiscard]] constexpr bool operator<=(const wstring_view& right) const
		{
			return !(right < *this);
		}

		[[nodiscard]] constexpr bool operator>=(const wstring_view& right) const
		{
			return !(*this < right);
		}
	};
}

#pragma once
#include "pair.hpp"
#include "types.hpp"
#include "vector.hpp"

namespace cstd
{
	template <class Key, class T>
	class flat_map
	{
	public:
		using size_type = size_t;
		using key_type = Key;
		using mapped_type = T;
		using mapped_reference = mapped_type&;
		using const_mapped_reference = const mapped_type&;
		using value_type = pair<key_type, mapped_type>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		flat_map() = default;

		template <class Y>
		flat_map(const initializer_list<Y> list)
		{
			for (const Y& entry : list)
			{
				elements_.push_back(static_cast<value_type>(entry));
			}
		}

		[[nodiscard]] pointer begin() noexcept
		{
			return elements_.data();
		}

		[[nodiscard]] pointer end() noexcept
		{
			return elements_.data() + elements_.size();
		}

		[[nodiscard]] pointer front()
		{
			CSTD_ASSERT(!empty(), "[flat_map] front() called on an empty flat map");

			return begin();
		}

		[[nodiscard]] const_pointer front() const
		{
			CSTD_ASSERT(!empty(), "[flat_map] front() called on an empty flat map");

			return begin();
		}

		[[nodiscard]] pointer back()
		{
			CSTD_ASSERT(!empty(), "[flat_map] back() called on an empty flat map");

			return end() - 1;
		}

		[[nodiscard]] const_pointer back() const
		{
			CSTD_ASSERT(!empty(), "[flat_map] back() called on an empty flat map");

			return end() - 1;
		}

		[[nodiscard]] mapped_reference operator[](const key_type& key)
		{
			pointer entry = find(key);

			if (!entry)
			{
				elements_.push_back(value_type(key, {}));

				entry = &elements_.back();
			}

			return entry->second;
		}

		[[nodiscard]] const_mapped_reference operator[](const key_type& key) const
		{
			const_pointer entry = find(key);

			CSTD_ASSERT(entry != nullptr, "[flat_map] unable to find key");

			return entry->second;
		}

		[[nodiscard]] pointer find(const key_type key)
		{
			for (size_type i = 0; i < elements_.size(); i++)
			{
				pointer const entry = &elements_[i];

				if (key == entry->first)
				{
					return entry;
				}
			}

			return nullptr;
		}

		[[nodiscard]] const_pointer find(const key_type key) const
		{
			for (size_type i = 0; i < elements_.size(); i++)
			{
				const_pointer const entry = &elements_[i];

				if (key == entry->first)
				{
					return entry;
				}
			}

			return nullptr;
		}

		void clear()
		{
			elements_.clear();
		}

		[[nodiscard]] bool empty() const
		{
			return elements_.empty();
		}

	protected:
		vector<value_type> elements_;
	};
}

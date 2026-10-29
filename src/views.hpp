#pragma once
#include "types.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace cstd::views
{
	// ---- iota_view ----
	// Lazy sequence of incrementing values starting from a given value.
	// Bounded (finite) or unbounded (sentinel = unreachable_sentinel_t).

	struct unreachable_sentinel_t
	{
		template <class T>
		[[nodiscard]] constexpr friend bool operator==(const T&, unreachable_sentinel_t) noexcept { return false; }

		template <class T>
		[[nodiscard]] constexpr friend bool operator!=(const T&, unreachable_sentinel_t) noexcept { return true; }

		template <class T>
		[[nodiscard]] constexpr friend bool operator==(unreachable_sentinel_t, const T&) noexcept { return false; }

		template <class T>
		[[nodiscard]] constexpr friend bool operator!=(unreachable_sentinel_t, const T&) noexcept { return true; }
	};

	template <class T, class Bound = unreachable_sentinel_t>
	class iota_view
	{
	public:
		class iterator
		{
		public:
			constexpr explicit iterator(const T value) noexcept : value_(value) { }

			[[nodiscard]] constexpr T operator*() const noexcept { return value_; }

			constexpr iterator& operator++() noexcept { ++value_; return *this; }

			constexpr iterator operator++(int) noexcept
			{
				iterator copy = *this;
				++value_;
				return copy;
			}

			[[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept { return value_ == other.value_; }
			[[nodiscard]] constexpr bool operator!=(const iterator& other) const noexcept { return value_ != other.value_; }

			[[nodiscard]] constexpr bool operator==(unreachable_sentinel_t) const noexcept { return false; }
			[[nodiscard]] constexpr bool operator!=(unreachable_sentinel_t) const noexcept { return true; }

		private:
			T value_;
		};

		constexpr explicit iota_view(const T start) noexcept : start_(start), bound_() { }
		constexpr iota_view(const T start, const Bound bound) noexcept : start_(start), bound_(bound) { }

		[[nodiscard]] constexpr iterator begin() const noexcept { return iterator(start_); }

		[[nodiscard]] constexpr auto end() const noexcept
		{
			if constexpr (is_same_v<Bound, unreachable_sentinel_t>)
			{
				return unreachable_sentinel_t{};
			}
			else
			{
				return iterator(static_cast<T>(bound_));
			}
		}

	private:
		T start_;
		Bound bound_;
	};

	// iota(start) -> unbounded, iota(start, bound) -> bounded
	template <class T>
	[[nodiscard]] constexpr iota_view<T> iota(const T start) noexcept
	{
		return iota_view<T>(start);
	}

	template <class T, class Bound>
	[[nodiscard]] constexpr iota_view<T, Bound> iota(const T start, const Bound bound) noexcept
	{
		return iota_view<T, Bound>(start, bound);
	}

	// ---- pipe infrastructure ----
	// Views are composed with operator|.  Each adaptor closure stores its arguments and
	// produces the composed view when piped a range.

	namespace detail
	{
		// Tag base so operator| can be constrained to adaptors only.
		struct adaptor_closure_base { };

		template <class Derived>
		struct adaptor_closure : adaptor_closure_base { };

		// Composed closure: left then right.
		template <class Left, class Right>
		struct composed_closure : adaptor_closure<composed_closure<Left, Right>>
		{
			Left left_;
			Right right_;

			constexpr composed_closure(Left left, Right right)
				: left_(cstd::move(left)), right_(cstd::move(right)) { }

			template <class Range>
			[[nodiscard]] constexpr auto operator()(Range&& range) const
			{
				return right_(left_(static_cast<Range&&>(range)));
			}
		};

		// Pipe between two closures: returns a new composed closure.
		template <class Left, class Right,
			class = enable_if_t<is_base_of_v<adaptor_closure_base, decay_t<Left>>>,
			class = enable_if_t<is_base_of_v<adaptor_closure_base, decay_t<Right>>>>
		[[nodiscard]] constexpr auto operator|(Left left, Right right)
		{
			return composed_closure<Left, Right>(cstd::move(left), cstd::move(right));
		}

		// Pipe a range into a closure: range | closure -> closure(range).  This lives in `detail`
		// alongside the closure types so argument-dependent lookup finds it from the closure operand
		// whatever namespace the range comes from.
		template <class Range, class Closure,
			class = enable_if_t<!is_base_of_v<adaptor_closure_base, decay_t<Range>>>,
			class = enable_if_t<is_base_of_v<adaptor_closure_base, decay_t<Closure>>>>
		[[nodiscard]] constexpr auto operator|(Range&& range, Closure&& closure)
		{
			return closure(static_cast<Range&&>(range));
		}
	}

	// ---- transform_view ----

	template <class Range, class Func>
	class transform_view
	{
		Range range_;
		Func func_;

	public:
		class iterator
		{
			using inner_iter = decltype(cstd::declval<const Range&>().begin());

			inner_iter it_;
			const Func* func_;

		public:
			constexpr iterator(inner_iter it, const Func* func) noexcept : it_(it), func_(func) { }

			[[nodiscard]] constexpr auto operator*() const { return (*func_)(*it_); }

			constexpr iterator& operator++() { ++it_; return *this; }

			constexpr iterator operator++(int)
			{
				iterator copy = *this;
				++it_;
				return copy;
			}

			[[nodiscard]] constexpr bool operator==(const iterator& other) const { return it_ == other.it_; }
			[[nodiscard]] constexpr bool operator!=(const iterator& other) const { return it_ != other.it_; }

			// Support comparison against a sentinel type (e.g. unreachable_sentinel_t or a
			// different end iterator type from the underlying range).
			template <class Sentinel>
			[[nodiscard]] constexpr bool operator==(const Sentinel& s) const { return it_ == s; }

			template <class Sentinel>
			[[nodiscard]] constexpr bool operator!=(const Sentinel& s) const { return it_ != s; }
		};

		class sentinel
		{
			using inner_sentinel = decltype(cstd::declval<const Range&>().end());

			inner_sentinel sent_;

		public:
			constexpr explicit sentinel(inner_sentinel sent) noexcept : sent_(sent) { }

			[[nodiscard]] constexpr friend bool operator==(const iterator& it, const sentinel& s) { return it == s.sent_; }
			[[nodiscard]] constexpr friend bool operator!=(const iterator& it, const sentinel& s) { return it != s.sent_; }
			[[nodiscard]] constexpr friend bool operator==(const sentinel& s, const iterator& it) { return it == s.sent_; }
			[[nodiscard]] constexpr friend bool operator!=(const sentinel& s, const iterator& it) { return it != s.sent_; }
		};

		constexpr transform_view(Range range, Func func)
			: range_(cstd::move(range)), func_(cstd::move(func)) { }

		[[nodiscard]] constexpr iterator begin() const { return iterator(range_.begin(), &func_); }
		[[nodiscard]] constexpr sentinel end() const { return sentinel(range_.end()); }
	};

	namespace detail
	{
		template <class Func>
		struct transform_closure : adaptor_closure<transform_closure<Func>>
		{
			Func func_;

			constexpr explicit transform_closure(Func func) : func_(cstd::move(func)) { }

			template <class Range>
			[[nodiscard]] constexpr auto operator()(Range&& range) const
			{
				return transform_view<decay_t<Range>, Func>(static_cast<Range&&>(range), func_);
			}
		};
	}

	template <class Func>
	[[nodiscard]] constexpr auto transform(Func func)
	{
		return detail::transform_closure<Func>(cstd::move(func));
	}

	// ---- filter_view ----

	template <class Range, class Pred>
	class filter_view
	{
		Range range_;
		Pred pred_;

	public:
		class iterator
		{
			using inner_iter = decltype(cstd::declval<const Range&>().begin());
			using inner_sentinel = decltype(cstd::declval<const Range&>().end());

			inner_iter it_;
			inner_sentinel end_;
			const Pred* pred_;

			constexpr void advance_to_match()
			{
				while (it_ != end_ && !(*pred_)(*it_))
				{
					++it_;
				}
			}

		public:
			constexpr iterator(inner_iter it, inner_sentinel end, const Pred* pred)
				: it_(it), end_(end), pred_(pred)
			{
				advance_to_match();
			}

			[[nodiscard]] constexpr decltype(auto) operator*() const { return *it_; }

			constexpr iterator& operator++()
			{
				++it_;
				advance_to_match();
				return *this;
			}

			constexpr iterator operator++(int)
			{
				iterator copy = *this;
				++(*this);
				return copy;
			}

			[[nodiscard]] constexpr bool operator==(const iterator& other) const { return it_ == other.it_; }
			[[nodiscard]] constexpr bool operator!=(const iterator& other) const { return it_ != other.it_; }

			template <class Sentinel>
			[[nodiscard]] constexpr bool operator==(const Sentinel& s) const { return it_ == s; }

			template <class Sentinel>
			[[nodiscard]] constexpr bool operator!=(const Sentinel& s) const { return it_ != s; }
		};

		class sentinel
		{
			using inner_sentinel = decltype(cstd::declval<const Range&>().end());

			inner_sentinel sent_;

		public:
			constexpr explicit sentinel(inner_sentinel sent) noexcept : sent_(sent) { }

			[[nodiscard]] constexpr friend bool operator==(const iterator& it, const sentinel& s) { return it == s.sent_; }
			[[nodiscard]] constexpr friend bool operator!=(const iterator& it, const sentinel& s) { return it != s.sent_; }
			[[nodiscard]] constexpr friend bool operator==(const sentinel& s, const iterator& it) { return it == s.sent_; }
			[[nodiscard]] constexpr friend bool operator!=(const sentinel& s, const iterator& it) { return it != s.sent_; }
		};

		constexpr filter_view(Range range, Pred pred)
			: range_(cstd::move(range)), pred_(cstd::move(pred)) { }

		[[nodiscard]] constexpr iterator begin() const { return iterator(range_.begin(), range_.end(), &pred_); }
		[[nodiscard]] constexpr sentinel end() const { return sentinel(range_.end()); }
	};

	namespace detail
	{
		template <class Pred>
		struct filter_closure : adaptor_closure<filter_closure<Pred>>
		{
			Pred pred_;

			constexpr explicit filter_closure(Pred pred) : pred_(cstd::move(pred)) { }

			template <class Range>
			[[nodiscard]] constexpr auto operator()(Range&& range) const
			{
				return filter_view<decay_t<Range>, Pred>(static_cast<Range&&>(range), pred_);
			}
		};
	}

	template <class Pred>
	[[nodiscard]] constexpr auto filter(Pred pred)
	{
		return detail::filter_closure<Pred>(cstd::move(pred));
	}

	// ---- take_while_view ----

	template <class Range, class Pred>
	class take_while_view
	{
		Range range_;
		Pred pred_;

	public:
		class sentinel;

		class iterator
		{
			using inner_iter = decltype(cstd::declval<const Range&>().begin());

			inner_iter it_;
			const Pred* pred_;

		public:
			constexpr iterator(inner_iter it, const Pred* pred) noexcept : it_(it), pred_(pred) { }

			[[nodiscard]] constexpr decltype(auto) operator*() const { return *it_; }

			constexpr iterator& operator++() { ++it_; return *this; }

			constexpr iterator operator++(int)
			{
				iterator copy = *this;
				++it_;
				return copy;
			}

			[[nodiscard]] constexpr bool operator==(const iterator& other) const { return it_ == other.it_; }
			[[nodiscard]] constexpr bool operator!=(const iterator& other) const { return it_ != other.it_; }

			template <class Sentinel>
			[[nodiscard]] constexpr bool operator==(const Sentinel& s) const { return it_ == s; }

			template <class Sentinel>
			[[nodiscard]] constexpr bool operator!=(const Sentinel& s) const { return it_ != s; }

			// The inner iterator, exposed for sentinel comparison.
			[[nodiscard]] constexpr const inner_iter& base() const noexcept { return it_; }
			[[nodiscard]] constexpr const Pred& predicate() const noexcept { return *pred_; }
		};

		class sentinel
		{
			using inner_sentinel = decltype(cstd::declval<const Range&>().end());

			inner_sentinel sent_;

		public:
			constexpr explicit sentinel(inner_sentinel sent) noexcept : sent_(sent) { }

			[[nodiscard]] constexpr friend bool operator==(const iterator& it, const sentinel& s)
			{
				return it.base() == s.sent_ || !it.predicate()(*it);
			}

			[[nodiscard]] constexpr friend bool operator!=(const iterator& it, const sentinel& s)
			{
				return !(it == s);
			}

			[[nodiscard]] constexpr friend bool operator==(const sentinel& s, const iterator& it) { return it == s; }
			[[nodiscard]] constexpr friend bool operator!=(const sentinel& s, const iterator& it) { return it != s; }
		};

		constexpr take_while_view(Range range, Pred pred)
			: range_(cstd::move(range)), pred_(cstd::move(pred)) { }

		[[nodiscard]] constexpr iterator begin() const { return iterator(range_.begin(), &pred_); }
		[[nodiscard]] constexpr sentinel end() const { return sentinel(range_.end()); }
	};

	namespace detail
	{
		template <class Pred>
		struct take_while_closure : adaptor_closure<take_while_closure<Pred>>
		{
			Pred pred_;

			constexpr explicit take_while_closure(Pred pred) : pred_(cstd::move(pred)) { }

			template <class Range>
			[[nodiscard]] constexpr auto operator()(Range&& range) const
			{
				return take_while_view<decay_t<Range>, Pred>(static_cast<Range&&>(range), pred_);
			}
		};
	}

	template <class Pred>
	[[nodiscard]] constexpr auto take_while(Pred pred)
	{
		return detail::take_while_closure<Pred>(cstd::move(pred));
	}

	// ---- join_view ----
	// Flattens a range of ranges into a single range.

	template <class Range>
	class join_view
	{
		Range range_;

		using outer_iter = decltype(cstd::declval<const Range&>().begin());
		using outer_sentinel = decltype(cstd::declval<const Range&>().end());

		// The inner range type is whatever dereferencing an outer iterator yields.
		using inner_range = remove_reference_t<decltype(*cstd::declval<outer_iter&>())>;
		using inner_iter = decltype(cstd::declval<inner_range&>().begin());

	public:
		class iterator
		{
			// Mutable outer iterator stored by value (we advance through it).
			outer_iter outer_;
			outer_sentinel outer_end_;

			// The current inner range and position within it.
			// Managed as raw storage so the inner range need not be default-constructible.
			alignas(inner_range) unsigned char inner_storage_[sizeof(inner_range)];
			alignas(inner_iter) unsigned char iter_storage_[sizeof(inner_iter)];
			bool has_inner_ = false;

			constexpr inner_range& inner_range_ref() { return *reinterpret_cast<inner_range*>(inner_storage_); }
			constexpr const inner_range& inner_range_ref() const { return *reinterpret_cast<const inner_range*>(inner_storage_); }

			constexpr inner_iter& inner_ref() { return *reinterpret_cast<inner_iter*>(iter_storage_); }
			constexpr const inner_iter& inner_ref() const { return *reinterpret_cast<const inner_iter*>(iter_storage_); }

			constexpr void emplace_inner()
			{
				new (inner_storage_) inner_range(*outer_);
				new (iter_storage_) inner_iter(inner_range_ref().begin());
				has_inner_ = true;
			}

			constexpr void settle()
			{
				while (outer_ != outer_end_)
				{
					emplace_inner();

					if (inner_ref() != inner_range_ref().end())
					{
						return;
					}

					has_inner_ = false;
					++outer_;
				}
			}

		public:
			constexpr iterator(outer_iter outer, outer_sentinel outer_end)
				: outer_(outer), outer_end_(outer_end)
			{
				settle();
			}

			[[nodiscard]] constexpr decltype(auto) operator*() const { return *inner_ref(); }

			constexpr iterator& operator++()
			{
				++inner_ref();

				if (inner_ref() == inner_range_ref().end())
				{
					has_inner_ = false;
					++outer_;
					settle();
				}

				return *this;
			}

			constexpr iterator operator++(int)
			{
				iterator copy = *this;
				++(*this);
				return copy;
			}

			[[nodiscard]] constexpr bool operator==(const iterator& other) const
			{
				const bool this_end = (outer_ == outer_end_);
				const bool other_end = (other.outer_ == other.outer_end_);

				if (this_end || other_end)
				{
					return this_end == other_end;
				}

				return inner_ref() == other.inner_ref();
			}

			[[nodiscard]] constexpr bool operator!=(const iterator& other) const { return !(*this == other); }

			[[nodiscard]] constexpr bool is_end() const noexcept { return outer_ == outer_end_; }
		};

		struct sentinel
		{
			[[nodiscard]] constexpr friend bool operator==(const iterator& it, sentinel) { return it.is_end(); }
			[[nodiscard]] constexpr friend bool operator!=(const iterator& it, sentinel) { return !it.is_end(); }
			[[nodiscard]] constexpr friend bool operator==(sentinel, const iterator& it) { return it.is_end(); }
			[[nodiscard]] constexpr friend bool operator!=(sentinel, const iterator& it) { return !it.is_end(); }
		};

		constexpr explicit join_view(Range range) : range_(cstd::move(range)) { }

		[[nodiscard]] constexpr iterator begin() const { return iterator(range_.begin(), range_.end()); }
		[[nodiscard]] constexpr sentinel end() const { return sentinel{}; }
	};

	namespace detail
	{
		struct join_closure : adaptor_closure<join_closure>
		{
			template <class Range>
			[[nodiscard]] constexpr auto operator()(Range&& range) const
			{
				return join_view<decay_t<Range>>(static_cast<Range&&>(range));
			}
		};
	}

	inline constexpr detail::join_closure join{};
}

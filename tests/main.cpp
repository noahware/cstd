#include "atomic.hpp"
#include "pointer.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "flat_map.hpp"
#include "string.hpp"
#include "string_view.hpp"
#include "span.hpp"
#include "hash.hpp"
#include "limits.hpp"
#include "utility.hpp"
#include "type_traits.hpp"
#include "exception.hpp"
#include "types.hpp"

struct point
{
	cstd::int16_t x;
	cstd::int16_t y;
};

cstd::int32_t main()
{
	// atomic<size_t> exercises the 8-byte integral path.
	cstd::atomic<cstd::size_t> counter(0);

	if (++counter != 1)
	{
		return 1;
	}

	if (counter.fetch_add(4) != 1)
	{
		return 2;
	}

	if (counter.load() != 5)
	{
		return 3;
	}

	if (counter.fetch_sub(2) != 5)
	{
		return 4;
	}

	if (counter.load() != 3)
	{
		return 5;
	}

	CSTD_ASSERT(static_cast<cstd::size_t>(counter) == 3, "[atomic] conversion mismatch");

	// atomic<uint32_t> exercises the 4-byte integral path.
	cstd::atomic<cstd::uint32_t> small(10);

	if (--small != 9)
	{
		return 6;
	}

	if (small.fetch_sub(9) != 9)
	{
		return 7;
	}

	if (small.load() != 0)
	{
		return 8;
	}

	// atomic<uint16_t> exercises the 2-byte integral path.
	cstd::atomic<cstd::uint16_t> tiny(0);

	if (tiny.fetch_add(300) != 0)
	{
		return 9;
	}

	if (tiny.load() != 300)
	{
		return 10;
	}

	// atomic<point> proves any trivially-copyable type works through the
	// bit-pattern path (no arithmetic ops), not just scalars.
	cstd::atomic<point> location(point{ 1, 2 });

	const point start = location.load();

	if (start.x != 1 || start.y != 2)
	{
		return 11;
	}

	const point previous = location.exchange(point{ 3, 4 });

	if (previous.x != 1 || previous.y != 2)
	{
		return 12;
	}

	point expected = point{ 3, 4 };

	if (!location.compare_exchange_strong(expected, point{ 5, 6 }))
	{
		return 13;
	}

	const point current = location.load();

	if (current.x != 5 || current.y != 6)
	{
		return 14;
	}

	point wrong = point{ 0, 0 };

	if (location.compare_exchange_strong(wrong, point{ 7, 8 }))
	{
		return 15;
	}

	if (wrong.x != 5 || wrong.y != 6)
	{
		return 16;
	}

	// shared_ptr now carries an atomic reference count.
	cstd::shared_ptr<cstd::int32_t> owner = cstd::make_shared<cstd::int32_t>(42);

	if (!owner)
	{
		return 17;
	}

	if (*owner != 42)
	{
		return 18;
	}

	{
		cstd::shared_ptr<cstd::int32_t> copy = owner;
		cstd::shared_ptr<cstd::int32_t> another(copy);

		if (*copy != 42 || *another != 42)
		{
			return 19;
		}
	}

	// The copies released without freeing the object while owner is still alive.
	if (*owner != 42)
	{
		return 20;
	}

	// ---- bug-fix verification: each scenario instantiates a fixed template path ----
	{
		// Bug 1: vector copy-assignment must return *this and copy the elements.
		cstd::vector<cstd::int32_t> source;
		source.push_back(1);
		source.push_back(2);

		cstd::vector<cstd::int32_t> destination;
		cstd::vector<cstd::int32_t>& assigned = (destination = source);

		CSTD_ASSERT(&assigned == &destination, "[test] vector copy-assignment must return *this");
		CSTD_ASSERT(destination.size() == 2 && destination[0] == 1 && destination[1] == 2,
			"[test] vector copy-assignment must copy the elements");

		// insert: middle (lvalue -> copy overload), front and end (rvalue -> move overload).
		cstd::vector<cstd::int32_t> numbers;
		numbers.push_back(10);
		numbers.push_back(30);

		const cstd::int32_t middle = 20;
		cstd::vector<cstd::int32_t>::pointer inserted = numbers.insert(numbers.begin() + 1, middle);

		CSTD_ASSERT(inserted == numbers.begin() + 1 && *inserted == 20, "[test] insert returns the new element");
		CSTD_ASSERT(numbers.size() == 3 && numbers[0] == 10 && numbers[1] == 20 && numbers[2] == 30,
			"[test] insert shifts elements right");

		numbers.insert(numbers.begin(), 5);
		numbers.insert(numbers.end(), 40);

		CSTD_ASSERT(numbers.size() == 5 && numbers[0] == 5 && numbers[4] == 40,
			"[test] insert at front and end");

		// Bugs 2 & 3: flat_map operator[] missing-key insert and existing-key update.
		cstd::flat_map<cstd::int32_t, cstd::int32_t> map;
		map[1] = 10;
		map[2] = 20;
		map[1] = 11;

		CSTD_ASSERT(map[1] == 11, "[test] flat_map existing-key update");
		CSTD_ASSERT(map[2] == 20, "[test] flat_map missing-key insert");

		// Bugs A & B: default string is empty and NUL-terminated; push_back keeps the trailing NUL.
		cstd::string empty;
		CSTD_ASSERT(empty.size() == 0 && empty.empty() && empty.data()[0] == '\0',
			"[test] default string must be empty and NUL-terminated");

		cstd::string text;
		text.push_back('a');
		text.push_back('b');
		text.push_back('c');

		CSTD_ASSERT(text.size() == 3, "[test] string push_back must grow size");
		CSTD_ASSERT(text.data()[0] == 'a' && text.data()[1] == 'b' && text.data()[2] == 'c'
			&& text.data()[3] == '\0', "[test] string push_back must keep content and trailing NUL");

		// Bug D: span must construct from a non-const vector and array.
		cstd::span<cstd::int32_t> from_vector(source);
		CSTD_ASSERT(from_vector.size() == 2 && from_vector[1] == 2, "[test] span from vector");

		cstd::array<cstd::int32_t, 3> fixed{ 5, 6, 7 };
		cstd::span<cstd::int32_t> from_array(fixed);
		CSTD_ASSERT(from_array.size() == 3 && from_array[2] == 7, "[test] span from array");

		// string_view is a read-only view: begin()/end() yield const_pointer.
		char buffer[] = "hi";
		cstd::string_view view(buffer);
		cstd::string_view::const_pointer first = view.begin();
		CSTD_ASSERT(view.end() - first == 2 && *first == 'h', "[test] string_view begin/end span the data");

		// Bug F: generic hash<T> must compile and be deterministic.
		cstd::hash<cstd::int32_t> hasher;
		CSTD_ASSERT(hasher(42) == hasher(42), "[test] hash must be deterministic");
	}

	// Bug C: shared_ptr cross-type (converting) copy must increment the refcount.
	{
		struct shape { cstd::int32_t sides_ = 0; };
		struct square : shape { };

		cstd::shared_ptr<square> sq = cstd::make_shared<square>();
		sq->sides_ = 4;

		{
			cstd::shared_ptr<shape> sh = sq;   // converting copy: must share ownership
			CSTD_ASSERT(sh->sides_ == 4, "[test] shared_ptr converting copy must share the object");
		}

		CSTD_ASSERT(sq->sides_ == 4, "[test] shared_ptr object must outlive a dropped converting copy");
	}

	// ---- foundational utilities ----
	{
		// Records how it was constructed, to prove perfect forwarding moves rather than copies.
		struct probe
		{
			cstd::int32_t copies_ = 0;
			cstd::int32_t moves_ = 0;

			probe() = default;
			probe(const probe& other) noexcept : copies_(other.copies_ + 1), moves_(other.moves_) { }
			probe(probe&& other) noexcept : copies_(other.copies_), moves_(other.moves_ + 1) { }
		};

		// forward: a pure rvalue must be moved into make_unique, never copied.
		cstd::unique_ptr<probe> forwarded = cstd::make_unique<probe>(probe{});
		CSTD_ASSERT(forwarded->copies_ == 0, "[test] forward moves an rvalue (no copy)");
		CSTD_ASSERT(forwarded->moves_ == 1, "[test] forward move-constructs exactly once");

		// swap
		cstd::vector<cstd::int32_t> left;
		left.push_back(1);
		cstd::vector<cstd::int32_t> right;
		right.push_back(9);
		right.push_back(8);
		cstd::swap(left, right);
		CSTD_ASSERT(left.size() == 2 && left[0] == 9 && right.size() == 1 && right[0] == 1, "[test] swap exchanges contents");

		// exchange
		cstd::int32_t slot = 3;
		const cstd::int32_t previous = cstd::exchange(slot, 7);
		CSTD_ASSERT(previous == 3 && slot == 7, "[test] exchange returns old and stores new");

		// min / max / clamp
		CSTD_ASSERT(cstd::min(2, 5) == 2 && cstd::max(2, 5) == 5, "[test] min/max");
		CSTD_ASSERT(cstd::clamp(10, 0, 5) == 5 && cstd::clamp(-1, 0, 5) == 0 && cstd::clamp(3, 0, 5) == 3, "[test] clamp");

		// numeric_limits
		CSTD_ASSERT(cstd::numeric_limits<cstd::uint8_t>::max() == 255, "[test] uint8 max");
		CSTD_ASSERT(cstd::numeric_limits<cstd::uint8_t>::min() == 0, "[test] uint8 min");
		CSTD_ASSERT(cstd::numeric_limits<cstd::int32_t>::max() == 2147483647, "[test] int32 max");
		CSTD_ASSERT(cstd::numeric_limits<cstd::int32_t>::is_signed, "[test] int32 is signed");
		CSTD_ASSERT(!cstd::numeric_limits<cstd::uint32_t>::is_signed, "[test] uint32 is unsigned");

		// compile-time trait checks
		struct trait_base { };
		struct trait_derived : trait_base { };

		static_assert(cstd::is_lvalue_reference_v<int&>, "is_lvalue_reference");
		static_assert(!cstd::is_lvalue_reference_v<int>, "is_lvalue_reference negative");
		static_assert(cstd::is_rvalue_reference_v<int&&>, "is_rvalue_reference");
		static_assert(cstd::is_pointer_v<int*>, "is_pointer");
		static_assert(cstd::is_void_v<const void>, "is_void");
		static_assert(cstd::is_same_v<cstd::decay_t<const int&>, int>, "decay");
		static_assert(cstd::is_same_v<cstd::remove_pointer_t<int* const>, int>, "remove_pointer");
		static_assert(cstd::is_base_of_v<trait_base, trait_derived>, "is_base_of");
		static_assert(!cstd::is_base_of_v<trait_derived, trait_base>, "is_base_of negative");
		static_assert(cstd::is_convertible_v<trait_derived*, trait_base*>, "is_convertible");
		static_assert(!cstd::is_convertible_v<trait_base*, trait_derived*>, "is_convertible negative");
	}

	*owner = 7;

	return *owner - 7;
}

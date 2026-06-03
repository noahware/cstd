#include "atomic.hpp"
#include "pointer.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "flat_map.hpp"
#include "string.hpp"
#include "string_view.hpp"
#include "span.hpp"
#include "hash.hpp"
#include "exception.hpp"
#include "types.hpp"

struct point
{
	cstl::int16_t x;
	cstl::int16_t y;
};

cstl::int32_t main()
{
	// atomic<size_t> exercises the 8-byte integral path.
	cstl::atomic<cstl::size_t> counter(0);

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

	CSTL_ASSERT(static_cast<cstl::size_t>(counter) == 3, "[atomic] conversion mismatch");

	// atomic<uint32_t> exercises the 4-byte integral path.
	cstl::atomic<cstl::uint32_t> small(10);

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
	cstl::atomic<cstl::uint16_t> tiny(0);

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
	cstl::atomic<point> location(point{ 1, 2 });

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
	cstl::shared_ptr<cstl::int32_t> owner = cstl::make_shared<cstl::int32_t>(42);

	if (!owner)
	{
		return 17;
	}

	if (*owner != 42)
	{
		return 18;
	}

	{
		cstl::shared_ptr<cstl::int32_t> copy = owner;
		cstl::shared_ptr<cstl::int32_t> another(copy);

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
		cstl::vector<cstl::int32_t> source;
		source.push_back(1);
		source.push_back(2);

		cstl::vector<cstl::int32_t> destination;
		cstl::vector<cstl::int32_t>& assigned = (destination = source);

		CSTL_ASSERT(&assigned == &destination, "[test] vector copy-assignment must return *this");
		CSTL_ASSERT(destination.size() == 2 && destination[0] == 1 && destination[1] == 2,
			"[test] vector copy-assignment must copy the elements");

		// insert: middle (lvalue -> copy overload), front and end (rvalue -> move overload).
		cstl::vector<cstl::int32_t> numbers;
		numbers.push_back(10);
		numbers.push_back(30);

		const cstl::int32_t middle = 20;
		cstl::vector<cstl::int32_t>::pointer inserted = numbers.insert(numbers.begin() + 1, middle);

		CSTL_ASSERT(inserted == numbers.begin() + 1 && *inserted == 20, "[test] insert returns the new element");
		CSTL_ASSERT(numbers.size() == 3 && numbers[0] == 10 && numbers[1] == 20 && numbers[2] == 30,
			"[test] insert shifts elements right");

		numbers.insert(numbers.begin(), 5);
		numbers.insert(numbers.end(), 40);

		CSTL_ASSERT(numbers.size() == 5 && numbers[0] == 5 && numbers[4] == 40,
			"[test] insert at front and end");

		// Bugs 2 & 3: flat_map operator[] missing-key insert and existing-key update.
		cstl::flat_map<cstl::int32_t, cstl::int32_t> map;
		map[1] = 10;
		map[2] = 20;
		map[1] = 11;

		CSTL_ASSERT(map[1] == 11, "[test] flat_map existing-key update");
		CSTL_ASSERT(map[2] == 20, "[test] flat_map missing-key insert");

		// Bugs A & B: default string is empty and NUL-terminated; push_back keeps the trailing NUL.
		cstl::string empty;
		CSTL_ASSERT(empty.size() == 0 && empty.empty() && empty.data()[0] == '\0',
			"[test] default string must be empty and NUL-terminated");

		cstl::string text;
		text.push_back('a');
		text.push_back('b');
		text.push_back('c');

		CSTL_ASSERT(text.size() == 3, "[test] string push_back must grow size");
		CSTL_ASSERT(text.data()[0] == 'a' && text.data()[1] == 'b' && text.data()[2] == 'c'
			&& text.data()[3] == '\0', "[test] string push_back must keep content and trailing NUL");

		// Bug D: span must construct from a non-const vector and array.
		cstl::span<cstl::int32_t> from_vector(source);
		CSTL_ASSERT(from_vector.size() == 2 && from_vector[1] == 2, "[test] span from vector");

		cstl::array<cstl::int32_t, 3> fixed{ 5, 6, 7 };
		cstl::span<cstl::int32_t> from_array(fixed);
		CSTL_ASSERT(from_array.size() == 3 && from_array[2] == 7, "[test] span from array");

		// string_view is a read-only view: begin()/end() yield const_pointer.
		char buffer[] = "hi";
		cstl::string_view view(buffer);
		cstl::string_view::const_pointer first = view.begin();
		CSTL_ASSERT(view.end() - first == 2 && *first == 'h', "[test] string_view begin/end span the data");

		// Bug F: generic hash<T> must compile and be deterministic.
		cstl::hash<cstl::int32_t> hasher;
		CSTL_ASSERT(hasher(42) == hasher(42), "[test] hash must be deterministic");
	}

	// Bug C: shared_ptr cross-type (converting) copy must increment the refcount.
	{
		struct shape { cstl::int32_t sides_ = 0; };
		struct square : shape { };

		cstl::shared_ptr<square> sq = cstl::make_shared<square>();
		sq->sides_ = 4;

		{
			cstl::shared_ptr<shape> sh = sq;   // converting copy: must share ownership
			CSTL_ASSERT(sh->sides_ == 4, "[test] shared_ptr converting copy must share the object");
		}

		CSTL_ASSERT(sq->sides_ == 4, "[test] shared_ptr object must outlive a dropped converting copy");
	}

	*owner = 7;

	return *owner - 7;
}

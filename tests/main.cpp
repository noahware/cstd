#include "atomic.hpp"
#include "pointer.hpp"
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

	*owner = 7;

	return *owner - 7;
}

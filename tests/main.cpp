#include "atomic.hpp"
#include "pointer.hpp"
#include "mutex.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "pair.hpp"
#include "optional.hpp"
#include "expected.hpp"
#include "algorithm.hpp"
#include "ranges.hpp"
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

// Tracks live instance count + copy/move counts to prove containers run
// destructors and construct elements in place rather than copying.
struct lifetime_probe
{
	static cstd::int32_t live;
	static cstd::int32_t copies;
	static cstd::int32_t moves;

	cstd::int32_t value = 0;

	lifetime_probe() { ++live; }
	explicit lifetime_probe(cstd::int32_t initial) : value(initial) { ++live; }
	lifetime_probe(const lifetime_probe& other) : value(other.value) { ++live; ++copies; }
	lifetime_probe(lifetime_probe&& other) noexcept : value(other.value) { ++live; ++moves; }
	lifetime_probe& operator=(const lifetime_probe& other) { value = other.value; return *this; }
	lifetime_probe& operator=(lifetime_probe&& other) noexcept { value = other.value; return *this; }
	~lifetime_probe() { --live; }
};

cstd::int32_t lifetime_probe::live = 0;
cstd::int32_t lifetime_probe::copies = 0;
cstd::int32_t lifetime_probe::moves = 0;

// Each test scenario lives in its own function so no single stack frame grows
// large enough to need __chkstk (unavailable under /NODEFAULTLIB). A failed
// CSTD_ASSERT traps; reaching the end of main means every scenario passed.

static void test_bug_fixes()
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

static void test_shared_ptr_conversion()
{
	// Bug C: shared_ptr cross-type (converting) copy must increment the refcount.
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

static void test_foundational_utilities()
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

static void test_vector_emplace_erase()
{
	// emplace_back constructs in place (no copy/move) and returns a reference to back().
	cstd::vector<lifetime_probe> e;
	e.reserve(4);
	lifetime_probe::copies = 0;
	lifetime_probe::moves = 0;

	cstd::vector<lifetime_probe>::reference ref = e.emplace_back(8);
	CSTD_ASSERT(lifetime_probe::copies == 0 && lifetime_probe::moves == 0, "[test] emplace_back constructs in place");
	CSTD_ASSERT(&ref == &e.back() && ref.value == 8, "[test] emplace_back returns back reference");

	// erase must run destructors and shift survivors left.
	{
		CSTD_ASSERT(lifetime_probe::live == 1, "[test] one probe live before erase scope");

		cstd::vector<lifetime_probe> v;
		v.emplace_back(1);
		v.emplace_back(2);
		v.emplace_back(3);
		v.emplace_back(4);
		CSTD_ASSERT(v.size() == 4 && lifetime_probe::live == 5, "[test] emplace_back grows live count");

		cstd::vector<lifetime_probe>::pointer next = v.erase(v.begin() + 1);   // removes value 2
		CSTD_ASSERT(v.size() == 3 && lifetime_probe::live == 4, "[test] erase one destroys one");
		CSTD_ASSERT(v[0].value == 1 && v[1].value == 3 && v[2].value == 4, "[test] erase shifts survivors");
		CSTD_ASSERT(next == v.begin() + 1 && next->value == 3, "[test] erase returns iterator to next element");

		v.erase(v.begin(), v.begin() + 2);   // removes 1 and 3
		CSTD_ASSERT(v.size() == 1 && v[0].value == 4 && lifetime_probe::live == 2, "[test] erase range destroys range");
	}

	CSTD_ASSERT(lifetime_probe::live == 1, "[test] vector destructor runs all element destructors");
}

static void test_vector_assign_insert()
{
	// assign (count+value / range), range insert, reserve/shrink_to_fit, at, comparisons.
	cstd::vector<cstd::int32_t> v;
	v.assign(3, 7);
	CSTD_ASSERT(v.size() == 3 && v[0] == 7 && v[2] == 7, "[test] vector assign(count, value)");

	cstd::int32_t source[] = { 1, 2, 3, 4 };
	v.assign(source, source + 4);
	CSTD_ASSERT(v.size() == 4 && v[0] == 1 && v[3] == 4, "[test] vector assign(range)");

	cstd::int32_t more[] = { 8, 9 };
	v.insert(v.begin() + 1, more, more + 2);   // { 1, 8, 9, 2, 3, 4 }
	CSTD_ASSERT(v.size() == 6 && v[1] == 8 && v[2] == 9 && v[3] == 2 && v[5] == 4, "[test] vector range insert");
	CSTD_ASSERT(v.at(0) == 1, "[test] vector at()");

	v.reserve(100);
	CSTD_ASSERT(v.max_size() >= 100, "[test] reserve grows capacity");
	v.shrink_to_fit();
	CSTD_ASSERT(v.max_size() == v.size() && v.size() == 6 && v[5] == 4, "[test] shrink_to_fit keeps contents");

	cstd::vector<cstd::int32_t> x;
	x.assign(source, source + 4);   // { 1, 2, 3, 4 }
	cstd::vector<cstd::int32_t> y;
	y.assign(source, source + 3);   // { 1, 2, 3 }
	CSTD_ASSERT(x != y && y < x, "[test] vector comparison: prefix is less");
	y.push_back(4);
	CSTD_ASSERT(x == y, "[test] vector equality");
	y.push_back(5);
	CSTD_ASSERT(x < y && y > x, "[test] vector less by length");
}

static void test_leaf_types()
{
	// pair: make_pair decays, comparisons, swap, converting copy.
	static_assert(cstd::is_same_v<decltype(cstd::make_pair(1, 'a').first), cstd::int32_t>, "make_pair decays first");
	static_assert(cstd::is_same_v<decltype(cstd::make_pair(1, 'a').second), char>, "make_pair decays second");

	cstd::pair<cstd::int32_t, cstd::int32_t> p1(1, 2);
	cstd::pair<cstd::int32_t, cstd::int32_t> p2(1, 3);
	CSTD_ASSERT(p1 == p1 && p1 != p2 && p1 < p2 && p2 > p1 && p1 <= p1 && p1 >= p1, "[test] pair comparisons");

	p1.swap(p2);
	CSTD_ASSERT(p1.second == 3 && p2.second == 2, "[test] pair swap");

	cstd::pair<cstd::int64_t, cstd::int64_t> widened = p1;   // converting copy
	CSTD_ASSERT(widened.first == 1 && widened.second == 3, "[test] pair converting copy");

	// array: accessors, equality, fill, ordering, swap, constexpr.
	cstd::array<cstd::int32_t, 3> a{ 1, 2, 3 };
	CSTD_ASSERT(a.at(0) == 1 && a.front() == 1 && a.back() == 3 && !a.empty(), "[test] array accessors");

	cstd::array<cstd::int32_t, 3> b{ 1, 2, 3 };
	CSTD_ASSERT(a == b, "[test] array equality");

	b.fill(9);
	CSTD_ASSERT(b[0] == 9 && b[1] == 9 && b[2] == 9 && a < b, "[test] array fill + ordering");

	a.swap(b);
	CSTD_ASSERT(a[0] == 9 && b[0] == 1, "[test] array swap");

	static_assert([]() constexpr { cstd::array<cstd::int32_t, 2> c{ 0, 0 }; c.fill(4); return c[0] == 4 && c[1] == 4; }(),
		"array constexpr fill");

	// span: front/back/empty/first/last/subspan.
	cstd::array<cstd::int32_t, 5> data{ 10, 20, 30, 40, 50 };
	cstd::span<cstd::int32_t> whole(data);
	CSTD_ASSERT(!whole.empty() && whole.front() == 10 && whole.back() == 50, "[test] span front/back/empty");

	cstd::span<cstd::int32_t> head = whole.first(2);
	cstd::span<cstd::int32_t> tail = whole.last(2);
	cstd::span<cstd::int32_t> mid = whole.subspan(1, 2);
	cstd::span<cstd::int32_t> rest = whole.subspan(3);
	CSTD_ASSERT(head.size() == 2 && head[0] == 10 && head[1] == 20, "[test] span first");
	CSTD_ASSERT(tail.size() == 2 && tail[0] == 40 && tail[1] == 50, "[test] span last");
	CSTD_ASSERT(mid.size() == 2 && mid[0] == 20 && mid[1] == 30, "[test] span subspan(offset,count)");
	CSTD_ASSERT(rest.size() == 2 && rest[0] == 40 && rest[1] == 50, "[test] span subspan(offset)");

	// string_view: front/back, starts_with/ends_with, substr, rfind, find_first_of, comparisons, remove_*.
	cstd::string_view sv("hello world");
	CSTD_ASSERT(sv.front() == 'h' && sv.back() == 'd', "[test] string_view front/back");
	CSTD_ASSERT(sv.starts_with(cstd::string_view("hello")) && sv.ends_with(cstd::string_view("world")),
		"[test] string_view starts/ends_with");
	CSTD_ASSERT(sv.starts_with('h') && sv.ends_with('d'), "[test] string_view starts/ends_with char");
	CSTD_ASSERT(!sv.starts_with(cstd::string_view("world")), "[test] string_view starts_with negative");

	CSTD_ASSERT(sv.substr(6, 5) == cstd::string_view("world"), "[test] string_view substr(pos,count)");
	CSTD_ASSERT(sv.substr(6) == cstd::string_view("world"), "[test] string_view substr(pos)");
	CSTD_ASSERT(sv.rfind(cstd::string_view("o")) == 7, "[test] string_view rfind");
	CSTD_ASSERT(sv.find_first_of(cstd::string_view("ol")) == 2, "[test] string_view find_first_of");

	cstd::string_view abc("abc");
	cstd::string_view abd("abd");
	cstd::string_view ab("ab");
	CSTD_ASSERT(abc < abd && abd > abc && abc != abd && ab < abc, "[test] string_view ordering");

	sv.remove_prefix(6);   // "world"
	CSTD_ASSERT(sv == cstd::string_view("world"), "[test] string_view remove_prefix");
	sv.remove_suffix(3);   // "wo"
	CSTD_ASSERT(sv == cstd::string_view("wo"), "[test] string_view remove_suffix");

	static_assert(cstd::string_view("abcdef").substr(2, 3) == cstd::string_view("cde"), "string_view constexpr substr");
	static_assert(cstd::string_view("prefix-suffix").starts_with(cstd::string_view("prefix")),
		"string_view constexpr starts_with");
}

static void test_string()
{
	// append / += (char, C-string, string_view) with value semantics and intact NUL.
	cstd::string s;
	s += 'a';
	s += "bc";
	s.append(cstd::string_view("de"));
	CSTD_ASSERT(s.size() == 5 && s == cstd::string_view("abcde"), "[test] string append/+=");
	CSTD_ASSERT(s.data()[5] == '\0', "[test] string keeps trailing NUL");

	cstd::string a("foo");
	cstd::string b("bar");
	cstd::string concatenated = a + b;
	CSTD_ASSERT(concatenated == cstd::string_view("foobar"), "[test] operator+ (string + string)");
	CSTD_ASSERT(a == cstd::string_view("foo") && b == cstd::string_view("bar"), "[test] operator+ leaves operands unchanged");
	CSTD_ASSERT((a + "baz") == cstd::string_view("foobaz"), "[test] operator+ (string + cstr)");
	CSTD_ASSERT(("pre" + a) == cstd::string_view("prefoo"), "[test] operator+ (cstr + string)");
	CSTD_ASSERT((a + '!') == cstd::string_view("foo!"), "[test] operator+ (string + char)");

	// substr (owning), c_str.
	cstd::string hello("hello");
	cstd::string sub = hello.substr(1, 3);
	CSTD_ASSERT(sub == cstd::string_view("ell") && sub.c_str()[3] == '\0', "[test] string substr is owning + NUL");
	CSTD_ASSERT(hello.substr(2) == cstd::string_view("llo"), "[test] string substr to end");

	// clear.
	cstd::string cleared("xyz");
	cleared.clear();
	CSTD_ASSERT(cleared.empty() && cleared.size() == 0 && cleared.data()[0] == '\0', "[test] string clear");

	// resize: shrink, then grow with fill.
	cstd::string shrunk("hello");
	shrunk.resize(2);
	CSTD_ASSERT(shrunk == cstd::string_view("he") && shrunk.data()[2] == '\0', "[test] string resize shrink");

	cstd::string grown("ab");
	grown.resize(5, 'x');
	CSTD_ASSERT(grown == cstd::string_view("abxxx") && grown.data()[5] == '\0', "[test] string resize grow with fill");

	// starts_with / ends_with / ordering (string converts to string_view).
	CSTD_ASSERT(hello.starts_with(cstd::string_view("he")) && hello.ends_with(cstd::string_view("lo")),
		"[test] string starts/ends_with");
	CSTD_ASSERT(hello.starts_with('h') && hello.ends_with('o'), "[test] string starts/ends_with char");

	cstd::string abc("abc");
	cstd::string abd("abd");
	CSTD_ASSERT(abc < abd && abd > abc && abc != abd && abc == abc, "[test] string comparisons");
}

static void test_optional()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] probe baseline before optional");

	// default / nullopt construct no T.
	cstd::optional<lifetime_probe> empty1;
	cstd::optional<lifetime_probe> empty2(cstd::nullopt);
	CSTD_ASSERT(!empty1.has_value() && !empty2 && lifetime_probe::live == 0, "[test] disengaged optional holds no T");

	// assigning a value engages and holds exactly one; reset destroys exactly one.
	{
		cstd::optional<lifetime_probe> o;
		o = lifetime_probe(42);
		CSTD_ASSERT(o.has_value() && o->value == 42 && lifetime_probe::live == 1, "[test] optional assign engages one T");

		o.reset();
		CSTD_ASSERT(!o.has_value() && lifetime_probe::live == 0, "[test] optional reset destroys the payload");
	}

	// copy of engaged copy-constructs payload once; copy of disengaged copies nothing.
	{
		cstd::optional<lifetime_probe> engaged(lifetime_probe(7));
		lifetime_probe::copies = 0;
		cstd::optional<lifetime_probe> copy_engaged = engaged;
		CSTD_ASSERT(copy_engaged.has_value() && copy_engaged->value == 7 && lifetime_probe::copies == 1,
			"[test] copy of engaged copies payload once");

		cstd::optional<lifetime_probe> disengaged;
		lifetime_probe::copies = 0;
		cstd::optional<lifetime_probe> copy_disengaged = disengaged;
		CSTD_ASSERT(!copy_disengaged.has_value() && lifetime_probe::copies == 0, "[test] copy of disengaged copies nothing");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] optional destructors balanced after copies");

	// move disengages the source.
	{
		cstd::optional<lifetime_probe> src(lifetime_probe(9));
		cstd::optional<lifetime_probe> dst = cstd::move(src);
		CSTD_ASSERT(dst.has_value() && dst->value == 9 && !src.has_value(), "[test] optional move disengages source");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] optional destructors balanced after move");

	// emplace destroys old payload, constructs new in place, returns a reference.
	{
		cstd::optional<lifetime_probe> o(lifetime_probe(1));
		lifetime_probe& r = o.emplace(2);
		CSTD_ASSERT(o.has_value() && o->value == 2 && &r == &o.value() && lifetime_probe::live == 1,
			"[test] optional emplace replaces in place");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] optional destructors balanced after emplace");

	// value_or: payload when engaged, default when disengaged.
	cstd::optional<cstd::int32_t> some(5);
	cstd::optional<cstd::int32_t> none;
	CSTD_ASSERT(some.value_or(0) == 5 && none.value_or(99) == 99, "[test] optional value_or");

	// constexpr engaged-from-value read path.
	constexpr cstd::optional<cstd::int32_t> compile_time(5);
	static_assert(compile_time.has_value() && *compile_time == 5, "optional constexpr value");
}

static void test_expected()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] probe baseline before expected");

	cstd::expected<cstd::int32_t, cstd::int32_t> ok(5);
	CSTD_ASSERT(ok.has_value() && static_cast<bool>(ok), "[test] expected value is engaged");
	CSTD_ASSERT(*ok == 5 && ok.value() == 5, "[test] expected value access");
	CSTD_ASSERT(ok.value_or(0) == 5, "[test] expected value_or returns the value");

	cstd::expected<point, cstd::int32_t> pt(point{ 3, 4 });
	CSTD_ASSERT(pt->x == 3 && pt->y == 4, "[test] expected operator->");

	cstd::expected<cstd::int32_t, cstd::int32_t> err(cstd::unexpected<cstd::int32_t>(42));
	CSTD_ASSERT(!err.has_value() && !err, "[test] expected error is disengaged");
	CSTD_ASSERT(err.error() == 42, "[test] expected error access");
	CSTD_ASSERT(err.value_or(99) == 99, "[test] expected value_or returns the default on error");

	cstd::expected<point, cstd::int32_t> in_place_value(cstd::in_place, point{ 1, 2 });
	CSTD_ASSERT(in_place_value.has_value() && in_place_value->x == 1 && in_place_value->y == 2,
		"[test] expected in_place construction");

	cstd::expected<point, cstd::int32_t> unexpect_error(cstd::unexpect, 7);
	CSTD_ASSERT(!unexpect_error.has_value() && unexpect_error.error() == 7,
		"[test] expected unexpect construction");

	{
		cstd::expected<lifetime_probe, cstd::int32_t> value_state(cstd::in_place, 3);
		CSTD_ASSERT(value_state.has_value() && value_state->value == 3 && lifetime_probe::live == 1,
			"[test] expected holds one live value");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] expected destroys the value member");

	{
		cstd::expected<cstd::int32_t, lifetime_probe> error_state(cstd::unexpect, 4);
		CSTD_ASSERT(!error_state.has_value() && error_state.error().value == 4 && lifetime_probe::live == 1,
			"[test] expected holds one live error");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] expected destroys the error member");

	{
		cstd::expected<cstd::int32_t, lifetime_probe> source(cstd::unexpect, 8);
		lifetime_probe::copies = 0;
		cstd::expected<cstd::int32_t, lifetime_probe> copy = source;
		CSTD_ASSERT(!copy.has_value() && copy.error().value == 8 && lifetime_probe::copies == 1,
			"[test] expected copy duplicates the active error once");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] expected copies balanced");

	cstd::expected<void, cstd::int32_t> good;
	good.value();
	CSTD_ASSERT(good.has_value() && static_cast<bool>(good), "[test] expected<void> success");

	cstd::expected<void, cstd::int32_t> bad(cstd::unexpect, 9);
	CSTD_ASSERT(!bad.has_value() && bad.error() == 9, "[test] expected<void> error");

	cstd::expected<cstd::int32_t, cstd::int32_t> ok_twin(5);
	CSTD_ASSERT(ok == ok_twin, "[test] expected equality");
	CSTD_ASSERT(err == cstd::unexpected<cstd::int32_t>(42), "[test] expected == unexpected");
	CSTD_ASSERT(ok != err, "[test] expected value != error");

	constexpr cstd::expected<cstd::int32_t, cstd::int32_t> compile_value(5);
	static_assert(compile_value.has_value() && *compile_value == 5, "expected constexpr value");
	constexpr cstd::expected<cstd::int32_t, cstd::int32_t> compile_error(cstd::unexpect, 1);
	static_assert(!compile_error.has_value() && compile_error.error() == 1, "expected constexpr error");

	CSTD_ASSERT(lifetime_probe::live == 0, "[test] expected probe balanced at end");
}

static void test_algorithm_nonmodifying()
{
	cstd::int32_t values[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
	cstd::int32_t* const first = values;
	cstd::int32_t* const last = values + 8;

	// find: present returns the slot; absent returns `last` (std semantics, not nullptr).
	CSTD_ASSERT(cstd::find(first, last, 4) == values + 2, "[test] find present");
	CSTD_ASSERT(cstd::find(first, last, 7) == last, "[test] find absent returns last");

	auto is_even = [](cstd::int32_t x) { return x % 2 == 0; };
	CSTD_ASSERT(cstd::find_if(first, last, is_even) == values + 2, "[test] find_if");
	CSTD_ASSERT(cstd::find_if_not(first, last, is_even) == values + 0, "[test] find_if_not");

	CSTD_ASSERT(cstd::count(first, last, 1) == 2, "[test] count");
	CSTD_ASSERT(cstd::count_if(first, last, is_even) == 3, "[test] count_if");

	CSTD_ASSERT(cstd::any_of(first, last, is_even) && !cstd::all_of(first, last, is_even)
		&& !cstd::none_of(first, last, is_even), "[test] any/all/none_of");

	// empty range: all_of/none_of are vacuously true, any_of false.
	CSTD_ASSERT(cstd::all_of(first, first, is_even) && cstd::none_of(first, first, is_even)
		&& !cstd::any_of(first, first, is_even), "[test] quantifiers on empty range");

	struct adder { cstd::int32_t sum = 0; void operator()(cstd::int32_t x) { sum += x; } };
	adder total = cstd::for_each(first, last, adder{});
	CSTD_ASSERT(total.sum == 31, "[test] for_each returns accumulated functor");

	cstd::int32_t same[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
	CSTD_ASSERT(cstd::equal(first, last, same), "[test] equal");
	same[0] = 99;
	CSTD_ASSERT(!cstd::equal(first, last, same), "[test] equal negative");

	auto same_parity = [](cstd::int32_t a, cstd::int32_t b) { return (a % 2) == (b % 2); };
	cstd::int32_t parity[] = { 1, 3, 2, 3, 7, 1, 4, 8 };
	CSTD_ASSERT(cstd::equal(first, last, parity, same_parity), "[test] equal with predicate");

	// first-of-equal tie-breaking matches the standard library.
	cstd::int32_t ties[] = { 3, 1, 1, 4, 4, 2 };
	CSTD_ASSERT(cstd::min_element(ties, ties + 6) == ties + 1, "[test] min_element first of equal");
	CSTD_ASSERT(cstd::max_element(ties, ties + 6) == ties + 3, "[test] max_element first of equal");
	CSTD_ASSERT(*cstd::min_element(first, last) == 1 && *cstd::max_element(first, last) == 9,
		"[test] min/max_element values");
	CSTD_ASSERT(cstd::min_element(first, first) == first, "[test] min_element empty returns last");
}

static void test_ranges()
{
	cstd::array<cstd::int32_t, 6> data{ 5, 3, 8, 1, 8, 2 };
	cstd::span<cstd::int32_t> range(data);

	CSTD_ASSERT(cstd::ranges::find(range, 8) == range.begin() + 2, "[test] ranges::find present");
	CSTD_ASSERT(cstd::ranges::find(range, 99) == range.end(), "[test] ranges::find miss returns end");
	CSTD_ASSERT(cstd::ranges::search(range, 99) == nullptr, "[test] ranges::search miss returns nullptr (legacy)");

	auto is_even = [](cstd::int32_t x) { return x % 2 == 0; };
	CSTD_ASSERT(cstd::ranges::count_if(range, is_even) == 3, "[test] ranges::count_if");
	CSTD_ASSERT(*cstd::ranges::min_element(range) == 1 && *cstd::ranges::max_element(range) == 8,
		"[test] ranges min/max_element");

	cstd::array<cstd::int32_t, 6> twin{ 5, 3, 8, 1, 8, 2 };
	cstd::span<cstd::int32_t> twin_range(twin);
	CSTD_ASSERT(cstd::ranges::equal(range, twin_range), "[test] ranges::equal");
}

static void test_algorithm_modifying()
{
	cstd::int32_t src[] = { 1, 2, 3, 4 };
	cstd::int32_t destination[4] = { 0, 0, 0, 0 };
	cstd::int32_t* copy_end = cstd::copy(src, src + 4, destination);
	CSTD_ASSERT(copy_end == destination + 4 && destination[0] == 1 && destination[3] == 4, "[test] copy");

	cstd::int32_t buffer[3] = { 0, 0, 0 };
	cstd::fill(buffer, buffer + 3, 9);
	CSTD_ASSERT(buffer[0] == 9 && buffer[1] == 9 && buffer[2] == 9, "[test] fill");

	cstd::int32_t squares[4] = {};
	cstd::int32_t* transform_end = cstd::transform(src, src + 4, squares, [](cstd::int32_t x) { return x * x; });
	CSTD_ASSERT(transform_end == squares + 4 && squares[0] == 1 && squares[3] == 16, "[test] transform unary");

	cstd::int32_t left[] = { 1, 2, 3 };
	cstd::int32_t right[] = { 10, 20, 30 };
	cstd::int32_t sums[3] = {};
	cstd::transform(left, left + 3, right, sums, [](cstd::int32_t x, cstd::int32_t y) { return x + y; });
	CSTD_ASSERT(sums[0] == 11 && sums[1] == 22 && sums[2] == 33, "[test] transform binary");

	cstd::int32_t even[] = { 1, 2, 3, 4 };
	cstd::reverse(even, even + 4);
	CSTD_ASSERT(even[0] == 4 && even[1] == 3 && even[2] == 2 && even[3] == 1, "[test] reverse even length");

	cstd::int32_t odd[] = { 1, 2, 3 };
	cstd::reverse(odd, odd + 3);
	CSTD_ASSERT(odd[0] == 3 && odd[1] == 2 && odd[2] == 1, "[test] reverse odd length");

	// remove_if + erase-remove idiom (stable survivors).
	cstd::vector<cstd::int32_t> v;
	for (cstd::int32_t i = 1; i <= 6; ++i)
	{
		v.push_back(i);
	}

	cstd::int32_t* new_end = cstd::remove_if(v.begin(), v.end(), [](cstd::int32_t x) { return x % 2 == 0; });
	CSTD_ASSERT(new_end == v.begin() + 3, "[test] remove_if returns new logical end");

	while (v.end() != new_end)
	{
		v.pop_back();
	}

	CSTD_ASSERT(v.size() == 3 && v[0] == 1 && v[1] == 3 && v[2] == 5, "[test] erase-remove keeps stable survivors");

	// remove all / remove none.
	cstd::int32_t all_twos[] = { 2, 2, 2 };
	CSTD_ASSERT(cstd::remove(all_twos, all_twos + 3, 2) == all_twos, "[test] remove all collapses to begin");
	cstd::int32_t no_match[] = { 1, 2, 3 };
	CSTD_ASSERT(cstd::remove(no_match, no_match + 3, 9) == no_match + 3, "[test] remove none keeps end");

	// ranges modifying wrappers.
	cstd::array<cstd::int32_t, 4> arr{ 1, 2, 3, 4 };
	cstd::span<cstd::int32_t> arr_span(arr);
	cstd::ranges::reverse(arr_span);
	CSTD_ASSERT(arr[0] == 4 && arr[3] == 1, "[test] ranges::reverse");
	cstd::ranges::fill(arr_span, 7);
	CSTD_ASSERT(arr[0] == 7 && arr[3] == 7, "[test] ranges::fill");
}

static void test_algorithm_sorted()
{
	// lower_bound / upper_bound / binary_search on sorted data with duplicates.
	cstd::int32_t sorted[] = { 1, 2, 2, 2, 3 };
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 2) == sorted + 1, "[test] lower_bound first match");
	CSTD_ASSERT(cstd::upper_bound(sorted, sorted + 5, 2) == sorted + 4, "[test] upper_bound past last match");
	CSTD_ASSERT(cstd::upper_bound(sorted, sorted + 5, 2) - cstd::lower_bound(sorted, sorted + 5, 2) == 3,
		"[test] bound multiplicity");
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 0) == sorted, "[test] lower_bound below all");
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 9) == sorted + 5, "[test] lower_bound above all");
	CSTD_ASSERT(cstd::binary_search(sorted, sorted + 5, 3) && !cstd::binary_search(sorted, sorted + 5, 4),
		"[test] binary_search present/absent");

	// sort: small range exercises the insertion-sort path (count <= 16).
	cstd::int32_t small[] = { 5, 2, 8, 1, 9, 3, 7, 4 };
	cstd::sort(small, small + 8);
	CSTD_ASSERT(cstd::is_sorted(small, small + 8) && small[0] == 1 && small[7] == 9, "[test] sort small (insertion)");
	CSTD_ASSERT(cstd::count(small, small + 8, 5) == 1, "[test] sort preserves elements");

	// sort: large range exercises heapsort; adversarial inputs that would break naive quicksort.
	cstd::int32_t big[64];
	for (cstd::int32_t i = 0; i < 64; ++i)
	{
		big[i] = 64 - i;
	}
	cstd::sort(big, big + 64);
	CSTD_ASSERT(cstd::is_sorted(big, big + 64) && big[0] == 1 && big[63] == 64, "[test] sort reverse-sorted (heap)");

	for (cstd::int32_t i = 0; i < 64; ++i)
	{
		big[i] = 7;
	}
	cstd::sort(big, big + 64);
	CSTD_ASSERT(cstd::is_sorted(big, big + 64) && big[0] == 7 && big[63] == 7, "[test] sort all-equal");

	for (cstd::int32_t i = 0; i < 64; ++i)
	{
		big[i] = i;
	}
	cstd::sort(big, big + 64);
	CSTD_ASSERT(cstd::is_sorted(big, big + 64) && big[0] == 0 && big[63] == 63, "[test] sort already-sorted");

	// custom comparator: descending order.
	cstd::int32_t descending[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
	cstd::sort(descending, descending + 8, cstd::greater{});
	CSTD_ASSERT(descending[0] == 9 && descending[7] == 1 && cstd::is_sorted(descending, descending + 8, cstd::greater{}),
		"[test] sort descending with comparator");

	// is_sorted edges.
	cstd::int32_t one[] = { 42 };
	CSTD_ASSERT(cstd::is_sorted(one, one + 1) && cstd::is_sorted(one, one), "[test] is_sorted single/empty");
	cstd::int32_t disordered[] = { 1, 3, 2 };
	CSTD_ASSERT(!cstd::is_sorted(disordered, disordered + 3), "[test] is_sorted detects disorder");

	// ranges sort + binary_search.
	cstd::array<cstd::int32_t, 5> ra{ 4, 2, 5, 1, 3 };
	cstd::span<cstd::int32_t> ra_span(ra);
	cstd::ranges::sort(ra_span);
	CSTD_ASSERT(cstd::ranges::is_sorted(ra_span) && cstd::ranges::binary_search(ra_span, 3)
		&& !cstd::ranges::binary_search(ra_span, 9), "[test] ranges sort + binary_search");
}

static void test_flat_map()
{
	// keys stay sorted regardless of insertion order.
	cstd::flat_map<cstd::int32_t, cstd::int32_t> m;
	m[3] = 30;
	m[1] = 10;
	m[2] = 20;
	CSTD_ASSERT(m.size() == 3, "[test] flat_map size");
	CSTD_ASSERT(m.begin()[0].first == 1 && m.begin()[1].first == 2 && m.begin()[2].first == 3,
		"[test] flat_map iterates in sorted key order");

	CSTD_ASSERT(m.contains(2) && !m.contains(9), "[test] flat_map contains");
	CSTD_ASSERT(m.count(2) == 1 && m.count(9) == 0, "[test] flat_map count");
	CSTD_ASSERT(m.at(1) == 10 && m.at(3) == 30, "[test] flat_map at");

	CSTD_ASSERT(m.erase(2) == 1 && m.erase(9) == 0, "[test] flat_map erase return value");
	CSTD_ASSERT(m.size() == 2 && !m.contains(2) && m.begin()[0].first == 1 && m.begin()[1].first == 3,
		"[test] flat_map erase keeps sorted order");

	// initializer-list with unsorted literals yields sorted iteration.
	using kv = cstd::pair<cstd::int32_t, cstd::int32_t>;
	kv entries[] = { { 5, 50 }, { 1, 10 }, { 3, 30 } };
	cstd::initializer_list<kv> entry_list(entries);
	cstd::flat_map<cstd::int32_t, cstd::int32_t> from_list(entry_list);
	CSTD_ASSERT(from_list.begin()[0].first == 1 && from_list.begin()[1].first == 3 && from_list.begin()[2].first == 5,
		"[test] flat_map initializer_list is sorted");
}

static void test_mutex()
{
	cstd::mutex m;
	CSTD_ASSERT(m.try_lock(), "[mutex] fresh try_lock acquires");
	CSTD_ASSERT(!m.try_lock(), "[mutex] try_lock fails while held");
	m.unlock();
	CSTD_ASSERT(m.try_lock(), "[mutex] try_lock succeeds after unlock");
	m.unlock();

	m.lock();
	CSTD_ASSERT(!m.try_lock(), "[mutex] lock() holds the mutex");
	m.unlock();
	CSTD_ASSERT(m.try_lock(), "[mutex] unlock() releases the mutex");
	m.unlock();

	// lock_guard locks on construction and releases at scope exit.
	cstd::mutex guarded;
	{
		cstd::lock_guard<cstd::mutex> lock(guarded);
		CSTD_ASSERT(!guarded.try_lock(), "[mutex] lock_guard holds during scope");
	}
	CSTD_ASSERT(guarded.try_lock(), "[mutex] lock_guard releases at scope exit");
	guarded.unlock();

	// adopt_lock: caller already holds the mutex; guard adopts and unlocks on exit.
	guarded.lock();
	{
		cstd::lock_guard<cstd::mutex> adopted(guarded, cstd::adopt_lock);
		CSTD_ASSERT(!guarded.try_lock(), "[mutex] adopted guard keeps the lock held");
	}
	CSTD_ASSERT(guarded.try_lock(), "[mutex] adopted guard releases at scope exit");
	guarded.unlock();

	static_assert(cstd::is_same_v<cstd::lock_guard<cstd::mutex>::mutex_type, cstd::mutex>,
		"lock_guard exposes mutex_type");
}

static void test_unique_lock()
{
	cstd::mutex m;

	// Default-constructed: no associated mutex, not owning.
	cstd::unique_lock<cstd::mutex> empty;
	CSTD_ASSERT(!empty.owns_lock(), "[unique_lock] default does not own");
	CSTD_ASSERT(empty.mutex() == nullptr, "[unique_lock] default has no mutex");
	CSTD_ASSERT(!static_cast<bool>(empty), "[unique_lock] default is falsy");

	// Locking constructor takes ownership; destructor releases it.
	{
		cstd::unique_lock<cstd::mutex> lock(m);
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] ctor locks");
		CSTD_ASSERT(lock.mutex() == &m, "[unique_lock] ctor records mutex");
		CSTD_ASSERT(static_cast<bool>(lock), "[unique_lock] owning lock is truthy");
		CSTD_ASSERT(!m.try_lock(), "[unique_lock] mutex held during scope");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] dtor unlocks");
	m.unlock();

	// defer_lock: associate without locking; lock()/unlock() drive ownership.
	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::defer_lock);
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] defer does not lock");
		lock.lock();
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] lock() acquires");
		lock.unlock();
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] unlock() releases");
	}

	// try_to_lock: acquires a free mutex, fails on a held one.
	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::try_to_lock);
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] try_to_lock acquires free mutex");
	}
	m.lock();
	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::try_to_lock);
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] try_to_lock fails on held mutex");
	}
	m.unlock();

	// adopt_lock: caller already holds it; guard adopts and unlocks on exit.
	m.lock();
	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::adopt_lock);
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] adopt owns");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] adopt releases at scope exit");
	m.unlock();

	// Move construction transfers ownership; the source goes empty.
	{
		cstd::unique_lock<cstd::mutex> source(m);
		cstd::unique_lock<cstd::mutex> sink(cstd::move(source));
		CSTD_ASSERT(!source.owns_lock() && source.mutex() == nullptr, "[unique_lock] move empties source");
		CSTD_ASSERT(sink.owns_lock() && sink.mutex() == &m, "[unique_lock] move transfers ownership");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] moved-to lock unlocks at scope exit");
	m.unlock();

	// Move assignment unlocks the prior lock, then adopts the source.
	{
		cstd::unique_lock<cstd::mutex> a(m);
		cstd::unique_lock<cstd::mutex> b;
		b = cstd::move(a);
		CSTD_ASSERT(b.owns_lock() && !a.owns_lock(), "[unique_lock] move-assign transfers");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] move-assign target unlocks at scope exit");
	m.unlock();

	// release() detaches the association WITHOUT unlocking: the mutex stays held.
	{
		cstd::unique_lock<cstd::mutex> lock(m);
		cstd::mutex* raw = lock.release();
		CSTD_ASSERT(raw == &m, "[unique_lock] release returns the mutex");
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] release drops ownership");
		CSTD_ASSERT(!m.try_lock(), "[unique_lock] release does NOT unlock");
		raw->unlock();
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] mutex usable after manual unlock");
	m.unlock();

	// swap exchanges ownership and association both ways.
	{
		cstd::unique_lock<cstd::mutex> held(m);
		cstd::unique_lock<cstd::mutex> other;
		held.swap(other);
		CSTD_ASSERT(other.owns_lock() && !held.owns_lock(), "[unique_lock] member swap moves ownership");
		cstd::swap(held, other);
		CSTD_ASSERT(held.owns_lock() && !other.owns_lock(), "[unique_lock] free swap moves it back");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] swapped lock unlocks at scope exit");
	m.unlock();
}

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

	// Each scenario traps via CSTD_ASSERT on failure; returning 0 means all passed.
	test_bug_fixes();
	test_shared_ptr_conversion();
	test_foundational_utilities();
	test_vector_emplace_erase();
	test_vector_assign_insert();
	test_leaf_types();
	test_string();
	test_optional();
	test_expected();
	test_algorithm_nonmodifying();
	test_ranges();
	test_algorithm_modifying();
	test_algorithm_sorted();
	test_flat_map();
	test_mutex();
	test_unique_lock();

	*owner = 7;

	return *owner - 7;
}

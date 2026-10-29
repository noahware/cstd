#include "atomic.hpp"
#include "pointer.hpp"
#include "mutex.hpp"
#include "function.hpp"
#include "list.hpp"
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
#include "ring_buffer.hpp"
#include "static_ring_buffer.hpp"
#include "unordered_map.hpp"
#include "bitset.hpp"
#include "charconv.hpp"
#include "views.hpp"

struct point
{
	cstd::int16_t x;
	cstd::int16_t y;
};

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

static void test_bug_fixes()
{
	cstd::vector<cstd::int32_t> source;
	source.push_back(1);
	source.push_back(2);

	cstd::vector<cstd::int32_t> destination;
	cstd::vector<cstd::int32_t>& assigned = (destination = source);

	CSTD_ASSERT(&assigned == &destination, "[test] vector copy-assignment must return *this");
	CSTD_ASSERT(destination.size() == 2 && destination[0] == 1 && destination[1] == 2,
		"[test] vector copy-assignment must copy the elements");

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

	cstd::flat_map<cstd::int32_t, cstd::int32_t> map;
	map[1] = 10;
	map[2] = 20;
	map[1] = 11;

	CSTD_ASSERT(map[1] == 11, "[test] flat_map existing-key update");
	CSTD_ASSERT(map[2] == 20, "[test] flat_map missing-key insert");

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

	cstd::span<cstd::int32_t> from_vector(source);
	CSTD_ASSERT(from_vector.size() == 2 && from_vector[1] == 2, "[test] span from vector");

	cstd::array<cstd::int32_t, 3> fixed{ 5, 6, 7 };
	cstd::span<cstd::int32_t> from_array(fixed);
	CSTD_ASSERT(from_array.size() == 3 && from_array[2] == 7, "[test] span from array");

	char buffer[] = "hi";
	cstd::string_view view(buffer);
	cstd::string_view::const_pointer first = view.begin();
	CSTD_ASSERT(view.end() - first == 2 && *first == 'h', "[test] string_view begin/end span the data");

	cstd::hash<cstd::int32_t> hasher;
	CSTD_ASSERT(hasher(42) == hasher(42), "[test] hash must be deterministic");
}

static void test_shared_ptr_conversion()
{
	struct shape { cstd::int32_t sides_ = 0; };
	struct square : shape { };

	cstd::shared_ptr<square> sq = cstd::make_shared<square>();
	sq->sides_ = 4;

	{
		cstd::shared_ptr<shape> sh = sq;
		CSTD_ASSERT(sh->sides_ == 4, "[test] shared_ptr converting copy must share the object");
	}

	CSTD_ASSERT(sq->sides_ == 4, "[test] shared_ptr object must outlive a dropped converting copy");
}

static void test_pointer_casts_shared()
{
	struct base { cstd::int32_t tag_ = 0; };
	struct derived : base { cstd::int32_t extra_ = 99; };

	cstd::shared_ptr<derived> d = cstd::make_shared<derived>();
	d->tag_ = 42;

	cstd::shared_ptr<base> b = cstd::static_pointer_cast<base>(d);
	CSTD_ASSERT(b->tag_ == 42, "[static_pointer_cast] shared upcast");

	cstd::shared_ptr<derived> back = cstd::static_pointer_cast<derived>(b);
	CSTD_ASSERT(back->extra_ == 99, "[static_pointer_cast] shared downcast");

	back->tag_ = 7;
	CSTD_ASSERT(d->tag_ == 7, "[static_pointer_cast] shared ownership");

	cstd::shared_ptr<base> empty;
	cstd::shared_ptr<derived> empty_cast = cstd::static_pointer_cast<derived>(empty);
	CSTD_ASSERT(!empty_cast, "[static_pointer_cast] shared null");

	cstd::shared_ptr<const int> ci = cstd::make_shared<int>(10);
	cstd::shared_ptr<int> mi = cstd::const_pointer_cast<int>(ci);
	CSTD_ASSERT(*mi == 10, "[const_pointer_cast] shared value");
	*mi = 20;
	CSTD_ASSERT(*ci == 20, "[const_pointer_cast] shared ownership");

	cstd::shared_ptr<const int> empty_const;
	CSTD_ASSERT(!cstd::const_pointer_cast<int>(empty_const), "[const_pointer_cast] shared null");

	cstd::shared_ptr<int> ri = cstd::make_shared<int>(55);
	cstd::shared_ptr<int> ri2 = cstd::reinterpret_pointer_cast<int>(ri);
	CSTD_ASSERT(*ri2 == 55, "[reinterpret_pointer_cast] shared identity");
}

static void test_pointer_casts_unique()
{
	struct base { cstd::int32_t tag_ = 0; };
	struct derived : base { cstd::int32_t extra_ = 77; };

	cstd::unique_ptr<derived> d = cstd::make_unique<derived>();
	d->tag_ = 42;
	d->extra_ = 77;

	cstd::unique_ptr<base> b = cstd::static_pointer_cast<base>(cstd::move(d));
	CSTD_ASSERT(!d, "[static_pointer_cast] unique source emptied");
	CSTD_ASSERT(b->tag_ == 42, "[static_pointer_cast] unique upcast");

	cstd::unique_ptr<derived> back = cstd::static_pointer_cast<derived>(cstd::move(b));
	CSTD_ASSERT(!b, "[static_pointer_cast] unique source emptied 2");
	CSTD_ASSERT(back->extra_ == 77, "[static_pointer_cast] unique downcast");

	cstd::unique_ptr<derived> empty;
	cstd::unique_ptr<base> empty_cast = cstd::static_pointer_cast<base>(cstd::move(empty));
	CSTD_ASSERT(!empty_cast, "[static_pointer_cast] unique null");

	cstd::unique_ptr<int> mi = cstd::make_unique<int>(10);
	cstd::unique_ptr<const int> ci = cstd::const_pointer_cast<const int>(cstd::move(mi));
	CSTD_ASSERT(!mi, "[const_pointer_cast] unique source emptied");
	CSTD_ASSERT(*ci == 10, "[const_pointer_cast] unique value");

	cstd::unique_ptr<int> ri = cstd::make_unique<int>(33);
	cstd::unique_ptr<int> ri2 = cstd::reinterpret_pointer_cast<int>(cstd::move(ri));
	CSTD_ASSERT(!ri, "[reinterpret_pointer_cast] unique source emptied");
	CSTD_ASSERT(*ri2 == 33, "[reinterpret_pointer_cast] unique identity");
}

static void test_foundational_utilities()
{
	struct probe
	{
		cstd::int32_t copies_ = 0;
		cstd::int32_t moves_ = 0;

		probe() = default;
		probe(const probe& other) noexcept : copies_(other.copies_ + 1), moves_(other.moves_) { }
		probe(probe&& other) noexcept : copies_(other.copies_), moves_(other.moves_ + 1) { }
	};

	cstd::unique_ptr<probe> forwarded = cstd::make_unique<probe>(probe{});
	CSTD_ASSERT(forwarded->copies_ == 0, "[test] forward moves an rvalue (no copy)");
	CSTD_ASSERT(forwarded->moves_ == 1, "[test] forward move-constructs exactly once");

	cstd::vector<cstd::int32_t> left;
	left.push_back(1);
	cstd::vector<cstd::int32_t> right;
	right.push_back(9);
	right.push_back(8);
	cstd::swap(left, right);
	CSTD_ASSERT(left.size() == 2 && left[0] == 9 && right.size() == 1 && right[0] == 1, "[test] swap exchanges contents");

	cstd::int32_t slot = 3;
	const cstd::int32_t previous = cstd::exchange(slot, 7);
	CSTD_ASSERT(previous == 3 && slot == 7, "[test] exchange returns old and stores new");

	CSTD_ASSERT(cstd::min(2, 5) == 2 && cstd::max(2, 5) == 5, "[test] min/max");
	CSTD_ASSERT(cstd::clamp(10, 0, 5) == 5 && cstd::clamp(-1, 0, 5) == 0 && cstd::clamp(3, 0, 5) == 3, "[test] clamp");

	CSTD_ASSERT(cstd::numeric_limits<cstd::uint8_t>::max() == 255, "[test] uint8 max");
	CSTD_ASSERT(cstd::numeric_limits<cstd::uint8_t>::min() == 0, "[test] uint8 min");
	CSTD_ASSERT(cstd::numeric_limits<cstd::int32_t>::max() == 2147483647, "[test] int32 max");
	CSTD_ASSERT(cstd::numeric_limits<cstd::int32_t>::is_signed, "[test] int32 is signed");
	CSTD_ASSERT(!cstd::numeric_limits<cstd::uint32_t>::is_signed, "[test] uint32 is unsigned");

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

static void test_numeric_limits()
{
	using il = cstd::numeric_limits<cstd::int32_t>;
	CSTD_ASSERT(il::is_specialized && il::is_integer && il::is_exact && il::is_bounded, "[test] int32 integer flags");
	CSTD_ASSERT(il::is_signed && !il::is_modulo && il::radix == 2, "[test] int32 signed/modulo/radix");
	CSTD_ASSERT(il::digits == 31 && il::digits10 == 9, "[test] int32 digits/digits10");
	CSTD_ASSERT(il::lowest() == il::min() && il::min() == -2147483647 - 1, "[test] int32 lowest equals min");
	CSTD_ASSERT(!il::has_infinity && !il::is_iec559 && il::round_style == cstd::round_toward_zero, "[test] int32 float traits off");

	using ul = cstd::numeric_limits<cstd::uint64_t>;
	CSTD_ASSERT(!ul::is_signed && ul::is_modulo, "[test] uint64 unsigned is_modulo");
	CSTD_ASSERT(ul::digits == 64 && ul::digits10 == 19, "[test] uint64 digits");
	CSTD_ASSERT(ul::max() == 18446744073709551615ull && ul::min() == 0 && ul::lowest() == 0, "[test] uint64 max/min/lowest");

	using bl = cstd::numeric_limits<bool>;
	CSTD_ASSERT(bl::is_specialized && bl::is_integer && bl::digits == 1, "[test] bool integer flags");
	CSTD_ASSERT(bl::max() == true && bl::min() == false && !bl::is_modulo, "[test] bool min/max");

	using cl = cstd::numeric_limits<char>;
	CSTD_ASSERT(cl::is_specialized && cl::is_integer, "[test] char specialized");
	CSTD_ASSERT(cl::digits == (cl::is_signed ? 7 : 8), "[test] char digits track signedness");
	CSTD_ASSERT(cl::max() > 0 && (cl::is_signed ? cl::min() < 0 : cl::min() == 0), "[test] char min/max track signedness");

	using u16 = cstd::numeric_limits<char16_t>;
	CSTD_ASSERT(!u16::is_signed && u16::digits == 16 && u16::max() == 65535, "[test] char16_t limits");

	struct opaque { };
	CSTD_ASSERT(!cstd::numeric_limits<opaque>::is_specialized && !cstd::numeric_limits<opaque>::is_integer, "[test] primary template unspecialized");
}

static void test_type_traits()
{
	enum unscoped_enum { enum_value };
	enum class scoped_enum { value };
	struct plain { cstd::int32_t x; };
	struct empty_type { };
	struct polymorphic_type { virtual ~polymorphic_type() = default; };
	struct abstract_type { virtual void f() = 0; };
	struct final_type final { };
	union simple_union { cstd::int32_t a; cstd::int64_t b; };

	static_assert(cstd::is_integral_v<int> && cstd::is_integral_v<const volatile int>, "is_integral strips cv");
	static_assert(!cstd::is_integral_v<plain> && !cstd::is_integral_v<int*>, "is_integral negative");
	static_assert(cstd::is_floating_point_v<double> && cstd::is_floating_point_v<const float> && !cstd::is_floating_point_v<int>, "is_floating_point");
	static_assert(cstd::is_arithmetic_v<int> && cstd::is_arithmetic_v<char> && !cstd::is_arithmetic_v<int*>, "is_arithmetic");
	static_assert(cstd::is_array_v<int[4]> && cstd::is_array_v<int[]> && !cstd::is_array_v<int>, "is_array");
	static_assert(cstd::is_enum_v<unscoped_enum> && cstd::is_enum_v<scoped_enum> && !cstd::is_enum_v<int>, "is_enum");
	static_assert(cstd::is_class_v<plain> && !cstd::is_class_v<simple_union> && !cstd::is_class_v<int>, "is_class");
	static_assert(cstd::is_union_v<simple_union> && !cstd::is_union_v<plain>, "is_union");
	static_assert(cstd::is_function_v<void(int)> && !cstd::is_function_v<int> && !cstd::is_function_v<void(*)(int)>, "is_function");
	static_assert(cstd::is_null_pointer_v<decltype(nullptr)> && !cstd::is_null_pointer_v<int*>, "is_null_pointer");

	static_assert(cstd::is_member_pointer_v<cstd::int32_t plain::*>, "is_member_pointer");
	static_assert(cstd::is_member_object_pointer_v<cstd::int32_t plain::*> && !cstd::is_member_function_pointer_v<cstd::int32_t plain::*>, "is_member_object_pointer");

	static_assert(cstd::is_fundamental_v<int> && cstd::is_fundamental_v<void> && !cstd::is_fundamental_v<plain>, "is_fundamental");
	static_assert(cstd::is_scalar_v<int> && cstd::is_scalar_v<int*> && cstd::is_scalar_v<unscoped_enum> && !cstd::is_scalar_v<plain>, "is_scalar");
	static_assert(cstd::is_object_v<plain> && cstd::is_object_v<int> && !cstd::is_object_v<int&> && !cstd::is_object_v<void>, "is_object");
	static_assert(cstd::is_compound_v<plain> && cstd::is_compound_v<int*> && !cstd::is_compound_v<int>, "is_compound");

	static_assert(cstd::is_const_v<const int> && !cstd::is_const_v<int>, "is_const");
	static_assert(cstd::is_volatile_v<volatile int> && !cstd::is_volatile_v<int>, "is_volatile");
	static_assert(cstd::is_signed_v<int> && !cstd::is_signed_v<unsigned int> && !cstd::is_signed_v<bool>, "is_signed");
	static_assert(cstd::is_unsigned_v<unsigned int> && cstd::is_unsigned_v<bool> && !cstd::is_unsigned_v<int>, "is_unsigned");
	static_assert(cstd::is_empty_v<empty_type> && !cstd::is_empty_v<plain>, "is_empty");
	static_assert(cstd::is_polymorphic_v<polymorphic_type> && !cstd::is_polymorphic_v<plain> && cstd::has_virtual_destructor_v<polymorphic_type>, "is_polymorphic/has_virtual_destructor");
	static_assert(cstd::is_abstract_v<abstract_type> && !cstd::is_abstract_v<plain>, "is_abstract");
	static_assert(cstd::is_final_v<final_type> && !cstd::is_final_v<plain> && cstd::is_aggregate_v<plain>, "is_final/is_aggregate");
	static_assert(cstd::is_standard_layout_v<plain> && cstd::is_trivial_v<plain>, "is_standard_layout/is_trivial");
	static_assert(cstd::is_destructible_v<plain> && cstd::is_destructible_v<int>, "is_destructible");
}

static void test_vector_emplace_erase()
{
	cstd::vector<lifetime_probe> e;
	e.reserve(4);
	lifetime_probe::copies = 0;
	lifetime_probe::moves = 0;

	cstd::vector<lifetime_probe>::reference ref = e.emplace_back(8);
	CSTD_ASSERT(lifetime_probe::copies == 0 && lifetime_probe::moves == 0, "[test] emplace_back constructs in place");
	CSTD_ASSERT(&ref == &e.back() && ref.value == 8, "[test] emplace_back returns back reference");

	{
		CSTD_ASSERT(lifetime_probe::live == 1, "[test] one probe live before erase scope");

		cstd::vector<lifetime_probe> v;
		v.emplace_back(1);
		v.emplace_back(2);
		v.emplace_back(3);
		v.emplace_back(4);
		CSTD_ASSERT(v.size() == 4 && lifetime_probe::live == 5, "[test] emplace_back grows live count");

		cstd::vector<lifetime_probe>::pointer next = v.erase(v.begin() + 1);
		CSTD_ASSERT(v.size() == 3 && lifetime_probe::live == 4, "[test] erase one destroys one");
		CSTD_ASSERT(v[0].value == 1 && v[1].value == 3 && v[2].value == 4, "[test] erase shifts survivors");
		CSTD_ASSERT(next == v.begin() + 1 && next->value == 3, "[test] erase returns iterator to next element");

		v.erase(v.begin(), v.begin() + 2);
		CSTD_ASSERT(v.size() == 1 && v[0].value == 4 && lifetime_probe::live == 2, "[test] erase range destroys range");
	}

	CSTD_ASSERT(lifetime_probe::live == 1, "[test] vector destructor runs all element destructors");
}

static void test_vector_assign_insert()
{
	cstd::vector<cstd::int32_t> v;
	v.assign(3, 7);
	CSTD_ASSERT(v.size() == 3 && v[0] == 7 && v[2] == 7, "[test] vector assign(count, value)");

	cstd::int32_t source[] = { 1, 2, 3, 4 };
	v.assign(source, source + 4);
	CSTD_ASSERT(v.size() == 4 && v[0] == 1 && v[3] == 4, "[test] vector assign(range)");

	cstd::int32_t more[] = { 8, 9 };
	v.insert(v.begin() + 1, more, more + 2);
	CSTD_ASSERT(v.size() == 6 && v[1] == 8 && v[2] == 9 && v[3] == 2 && v[5] == 4, "[test] vector range insert");
	CSTD_ASSERT(v.at(0) == 1, "[test] vector at()");

	v.reserve(100);
	CSTD_ASSERT(v.max_size() >= 100, "[test] reserve grows capacity");
	v.shrink_to_fit();
	CSTD_ASSERT(v.max_size() == v.size() && v.size() == 6 && v[5] == 4, "[test] shrink_to_fit keeps contents");

	cstd::vector<cstd::int32_t> x;
	x.assign(source, source + 4);
	cstd::vector<cstd::int32_t> y;
	y.assign(source, source + 3);
	CSTD_ASSERT(x != y && y < x, "[test] vector comparison: prefix is less");
	y.push_back(4);
	CSTD_ASSERT(x == y, "[test] vector equality");
	y.push_back(5);
	CSTD_ASSERT(x < y && y > x, "[test] vector less by length");
}

static void test_vector_capacity_swap()
{
	cstd::vector<cstd::int32_t> v;
	v.reserve(10);
	CSTD_ASSERT(v.capacity() == 10 && v.capacity() == v.max_size(), "[test] vector capacity()");

	v.resize(3, 5);
	CSTD_ASSERT(v.size() == 3 && v[0] == 5 && v[2] == 5, "[test] vector resize(n, value) fill");
	v.resize(5, 9);
	CSTD_ASSERT(v.size() == 5 && v[2] == 5 && v[3] == 9 && v[4] == 9, "[test] vector resize grow keeps prefix");
	v.resize(1, 0);
	CSTD_ASSERT(v.size() == 1 && v[0] == 5, "[test] vector resize shrink");

	cstd::vector<cstd::int32_t> a;
	a.push_back(1);
	a.push_back(2);
	cstd::vector<cstd::int32_t> b;
	b.push_back(9);

	a.swap(b);
	CSTD_ASSERT(a.size() == 1 && a[0] == 9 && b.size() == 2 && b[0] == 1 && b[1] == 2, "[test] vector member swap");

	cstd::swap(a, b);
	CSTD_ASSERT(a.size() == 2 && a[0] == 1 && b.size() == 1 && b[0] == 9, "[test] vector free swap");
}

static void test_leaf_types()
{
	static_assert(cstd::is_same_v<decltype(cstd::make_pair(1, 'a').first), cstd::int32_t>, "make_pair decays first");
	static_assert(cstd::is_same_v<decltype(cstd::make_pair(1, 'a').second), char>, "make_pair decays second");

	cstd::pair<cstd::int32_t, cstd::int32_t> p1(1, 2);
	cstd::pair<cstd::int32_t, cstd::int32_t> p2(1, 3);
	CSTD_ASSERT(p1 == p1 && p1 != p2 && p1 < p2 && p2 > p1 && p1 <= p1 && p1 >= p1, "[test] pair comparisons");

	p1.swap(p2);
	CSTD_ASSERT(p1.second == 3 && p2.second == 2, "[test] pair swap");

	cstd::pair<cstd::int64_t, cstd::int64_t> widened = p1;
	CSTD_ASSERT(widened.first == 1 && widened.second == 3, "[test] pair converting copy");

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

	sv.remove_prefix(6);
	CSTD_ASSERT(sv == cstd::string_view("world"), "[test] string_view remove_prefix");
	sv.remove_suffix(3);
	CSTD_ASSERT(sv == cstd::string_view("wo"), "[test] string_view remove_suffix");

	static_assert(cstd::string_view("abcdef").substr(2, 3) == cstd::string_view("cde"), "string_view constexpr substr");
	static_assert(cstd::string_view("prefix-suffix").starts_with(cstd::string_view("prefix")),
		"string_view constexpr starts_with");
}

static void test_string()
{
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

	cstd::string hello("hello");
	cstd::string sub = hello.substr(1, 3);
	CSTD_ASSERT(sub == cstd::string_view("ell") && sub.c_str()[3] == '\0', "[test] string substr is owning + NUL");
	CSTD_ASSERT(hello.substr(2) == cstd::string_view("llo"), "[test] string substr to end");

	cstd::string cleared("xyz");
	cleared.clear();
	CSTD_ASSERT(cleared.empty() && cleared.size() == 0 && cleared.data()[0] == '\0', "[test] string clear");

	cstd::string shrunk("hello");
	shrunk.resize(2);
	CSTD_ASSERT(shrunk == cstd::string_view("he") && shrunk.data()[2] == '\0', "[test] string resize shrink");

	cstd::string grown("ab");
	grown.resize(5, 'x');
	CSTD_ASSERT(grown == cstd::string_view("abxxx") && grown.data()[5] == '\0', "[test] string resize grow with fill");

	CSTD_ASSERT(hello.starts_with(cstd::string_view("he")) && hello.ends_with(cstd::string_view("lo")),
		"[test] string starts/ends_with");
	CSTD_ASSERT(hello.starts_with('h') && hello.ends_with('o'), "[test] string starts/ends_with char");

	cstd::string abc("abc");
	cstd::string abd("abd");
	CSTD_ASSERT(abc < abd && abd > abc && abc != abd && abc == abc, "[test] string comparisons");
}

static void test_string_conversion()
{
	cstd::wstring_view wide_view(L"Hello");
	cstd::string narrowed = cstd::to_string(wide_view);
	CSTD_ASSERT(narrowed.size() == 5 && narrowed == cstd::string_view("Hello"), "[test] to_string from wstring_view");
	CSTD_ASSERT(narrowed.data()[5] == '\0', "[test] to_string trailing NUL");

	cstd::string_view narrow_view("World");
	cstd::wstring widened = cstd::to_wstring(narrow_view);
	CSTD_ASSERT(widened.size() == 5 && widened == cstd::wstring_view(L"World"), "[test] to_wstring from string_view");
	CSTD_ASSERT(widened.data()[5] == L'\0', "[test] to_wstring trailing NUL");

	cstd::string empty_narrow = cstd::to_string(cstd::wstring_view(L""));
	CSTD_ASSERT(empty_narrow.empty(), "[test] to_string empty");

	cstd::wstring empty_wide = cstd::to_wstring(cstd::string_view(""));
	CSTD_ASSERT(empty_wide.empty(), "[test] to_wstring empty");
}

static void test_string_view_search()
{
	cstd::string_view text("abracadabra");

	CSTD_ASSERT(text.find(cstd::string_view("cad")) == 4, "[test] string_view find substring");
	CSTD_ASSERT(text.find("ra") == 2 && text.find("ra", 3) == 9, "[test] string_view find with offset");
	CSTD_ASSERT(text.find("xyz") == cstd::string_view::npos, "[test] string_view find miss returns npos");
	CSTD_ASSERT(text.find('a') == 0 && text.find('a', 1) == 3, "[test] string_view find char");
	CSTD_ASSERT(text.find('z') == cstd::string_view::npos, "[test] string_view find char miss");

	CSTD_ASSERT(text.rfind("abra") == 7 && text.rfind("abra", 6) == 0, "[test] string_view rfind substring");
	CSTD_ASSERT(text.rfind('a') == 10 && text.rfind('b', 5) == 1, "[test] string_view rfind char");

	CSTD_ASSERT(text.find_first_of("cd") == 4 && text.find_last_of("cd") == 6, "[test] string_view find_first/last_of");
	CSTD_ASSERT(text.find_first_of("xyz") == cstd::string_view::npos, "[test] string_view find_first_of miss");
	CSTD_ASSERT(text.find_first_not_of("abr") == 4 && text.find_last_not_of("abr") == 6, "[test] string_view find_first/last_not_of");
	CSTD_ASSERT(cstd::string_view("aaa").find_first_not_of('a') == cstd::string_view::npos, "[test] string_view find_first_not_of char all-match");

	CSTD_ASSERT(text.contains(cstd::string_view("cada")) && text.contains('d'), "[test] string_view contains");
	CSTD_ASSERT(!text.contains("xyz") && !text.contains('z'), "[test] string_view contains miss");

	cstd::string_view prefix = text.substr(0, 5);
	CSTD_ASSERT(prefix.find('c') == 4, "[test] string_view find within bounded view");
	CSTD_ASSERT(prefix.find("ad") == cstd::string_view::npos, "[test] string_view find does not read past bounded view");
	CSTD_ASSERT(prefix.find_first_of("dz") == cstd::string_view::npos, "[test] string_view find_first_of respects view bound");
}

static void test_string_search()
{
	cstd::string text("abracadabra");

	CSTD_ASSERT(text.find(cstd::string_view("cad")) == 4 && text.find("dab") == 6, "[test] string find substring");
	CSTD_ASSERT(text.find('d') == 6 && text.find("zzz") == cstd::string::npos, "[test] string find char/miss");
	CSTD_ASSERT(text.rfind("abra") == 7 && text.rfind('a') == 10, "[test] string rfind");
	CSTD_ASSERT(text.find_first_of("rc") == 2 && text.find_last_of("rc") == 9, "[test] string find_first/last_of");
	CSTD_ASSERT(text.find_first_not_of("abr") == 4 && text.find_last_not_of("abr") == 6, "[test] string find_first/last_not_of");
	CSTD_ASSERT(text.contains("cada") && text.contains('c'), "[test] string contains");
	CSTD_ASSERT(!text.contains("qqq") && !text.contains('q'), "[test] string contains miss");
	CSTD_ASSERT(text.front() == 'a' && text.back() == 'a' && text.at(4) == 'c', "[test] string front/back/at");
}

static void test_optional()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] probe baseline before optional");

	cstd::optional<lifetime_probe> empty1;
	cstd::optional<lifetime_probe> empty2(cstd::nullopt);
	CSTD_ASSERT(!empty1.has_value() && !empty2 && lifetime_probe::live == 0, "[test] disengaged optional holds no T");

	{
		cstd::optional<lifetime_probe> o;
		o = lifetime_probe(42);
		CSTD_ASSERT(o.has_value() && o->value == 42 && lifetime_probe::live == 1, "[test] optional assign engages one T");

		o.reset();
		CSTD_ASSERT(!o.has_value() && lifetime_probe::live == 0, "[test] optional reset destroys the payload");
	}

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

	{
		cstd::optional<lifetime_probe> src(lifetime_probe(9));
		cstd::optional<lifetime_probe> dst = cstd::move(src);
		CSTD_ASSERT(dst.has_value() && dst->value == 9 && !src.has_value(), "[test] optional move disengages source");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] optional destructors balanced after move");

	{
		cstd::optional<lifetime_probe> o(lifetime_probe(1));
		lifetime_probe& r = o.emplace(2);
		CSTD_ASSERT(o.has_value() && o->value == 2 && &r == &o.value() && lifetime_probe::live == 1,
			"[test] optional emplace replaces in place");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[test] optional destructors balanced after emplace");

	cstd::optional<cstd::int32_t> some(5);
	cstd::optional<cstd::int32_t> none;
	CSTD_ASSERT(some.value_or(0) == 5 && none.value_or(99) == 99, "[test] optional value_or");

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

	CSTD_ASSERT(cstd::find(first, last, 4) == values + 2, "[test] find present");
	CSTD_ASSERT(cstd::find(first, last, 7) == last, "[test] find absent returns last");

	auto is_even = [](cstd::int32_t x) { return x % 2 == 0; };
	CSTD_ASSERT(cstd::find_if(first, last, is_even) == values + 2, "[test] find_if");
	CSTD_ASSERT(cstd::find_if_not(first, last, is_even) == values + 0, "[test] find_if_not");

	CSTD_ASSERT(cstd::count(first, last, 1) == 2, "[test] count");
	CSTD_ASSERT(cstd::count_if(first, last, is_even) == 3, "[test] count_if");

	CSTD_ASSERT(cstd::any_of(first, last, is_even) && !cstd::all_of(first, last, is_even)
		&& !cstd::none_of(first, last, is_even), "[test] any/all/none_of");

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

static void test_ranges_extra()
{
	cstd::array<cstd::int32_t, 5> data{ 1, 2, 3, 4, 5 };
	cstd::span<cstd::int32_t> range(data);

	cstd::int32_t destination[5] = {};
	cstd::int32_t* copy_end = cstd::ranges::copy(range, destination);
	CSTD_ASSERT(copy_end == destination + 5 && destination[0] == 1 && destination[4] == 5, "[test] ranges::copy");

	cstd::int32_t squares[5] = {};
	cstd::ranges::transform(range, squares, [](cstd::int32_t x) { return x * x; });
	CSTD_ASSERT(squares[1] == 4 && squares[4] == 25, "[test] ranges::transform unary");

	cstd::array<cstd::int32_t, 5> other{ 10, 20, 30, 40, 50 };
	cstd::span<cstd::int32_t> other_range(other);
	cstd::int32_t sums[5] = {};
	cstd::ranges::transform(range, other_range, sums, [](cstd::int32_t a, cstd::int32_t b) { return a + b; });
	CSTD_ASSERT(sums[0] == 11 && sums[4] == 55, "[test] ranges::transform binary");

	auto same_parity = [](cstd::int32_t a, cstd::int32_t b) { return (a % 2) == (b % 2); };
	cstd::array<cstd::int32_t, 5> parity{ 3, 4, 5, 6, 7 };
	cstd::span<cstd::int32_t> parity_range(parity);
	CSTD_ASSERT(cstd::ranges::equal(range, parity_range, same_parity), "[test] ranges::equal with predicate");

	cstd::int32_t evens[3] = {};
	cstd::int32_t* copy_if_end = cstd::ranges::copy_if(range, evens, [](cstd::int32_t x) { return x % 2 == 0; });
	CSTD_ASSERT(copy_if_end == evens + 2 && evens[0] == 2 && evens[1] == 4, "[test] ranges::copy_if");

	cstd::array<cstd::int32_t, 4> targets{ 9, 8, 3, 7 };
	cstd::span<cstd::int32_t> targets_range(targets);
	CSTD_ASSERT(cstd::ranges::find_first_of(range, targets_range) == range.begin() + 2, "[test] ranges::find_first_of");

	cstd::array<cstd::int32_t, 6> dups{ 1, 1, 2, 2, 3, 3 };
	cstd::span<cstd::int32_t> dups_range(dups);
	CSTD_ASSERT(cstd::ranges::adjacent_find(dups_range) == dups_range.begin(), "[test] ranges::adjacent_find");
	cstd::int32_t* unique_end = cstd::ranges::unique(dups_range);
	CSTD_ASSERT(unique_end == dups_range.begin() + 3 && dups[0] == 1 && dups[1] == 2 && dups[2] == 3, "[test] ranges::unique");

	cstd::array<cstd::int32_t, 5> rep{ 1, 2, 1, 2, 1 };
	cstd::span<cstd::int32_t> rep_range(rep);
	cstd::ranges::replace(rep_range, 1, 9);
	CSTD_ASSERT(rep[0] == 9 && rep[2] == 9 && rep[1] == 2, "[test] ranges::replace");
	cstd::ranges::replace_if(rep_range, [](cstd::int32_t x) { return x == 2; }, 0);
	CSTD_ASSERT(rep[1] == 0 && rep[3] == 0, "[test] ranges::replace_if");

	cstd::array<cstd::int32_t, 5> sorted{ 1, 2, 3, 4, 5 };
	cstd::span<cstd::int32_t> sorted_range(sorted);
	CSTD_ASSERT(cstd::ranges::lower_bound(sorted_range, 3, cstd::less{}) == sorted_range.begin() + 2, "[test] ranges::lower_bound with compare");
	CSTD_ASSERT(cstd::ranges::upper_bound(sorted_range, 3, cstd::less{}) == sorted_range.begin() + 3, "[test] ranges::upper_bound with compare");
	CSTD_ASSERT(cstd::ranges::binary_search(sorted_range, 4, cstd::less{}) && cstd::ranges::is_sorted(sorted_range, cstd::less{}),
		"[test] ranges::binary_search/is_sorted with compare");
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

	cstd::int32_t all_twos[] = { 2, 2, 2 };
	CSTD_ASSERT(cstd::remove(all_twos, all_twos + 3, 2) == all_twos, "[test] remove all collapses to begin");
	cstd::int32_t no_match[] = { 1, 2, 3 };
	CSTD_ASSERT(cstd::remove(no_match, no_match + 3, 9) == no_match + 3, "[test] remove none keeps end");

	cstd::array<cstd::int32_t, 4> arr{ 1, 2, 3, 4 };
	cstd::span<cstd::int32_t> arr_span(arr);
	cstd::ranges::reverse(arr_span);
	CSTD_ASSERT(arr[0] == 4 && arr[3] == 1, "[test] ranges::reverse");
	cstd::ranges::fill(arr_span, 7);
	CSTD_ASSERT(arr[0] == 7 && arr[3] == 7, "[test] ranges::fill");
}

static void test_algorithm_sorted()
{
	cstd::int32_t sorted[] = { 1, 2, 2, 2, 3 };
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 2) == sorted + 1, "[test] lower_bound first match");
	CSTD_ASSERT(cstd::upper_bound(sorted, sorted + 5, 2) == sorted + 4, "[test] upper_bound past last match");
	CSTD_ASSERT(cstd::upper_bound(sorted, sorted + 5, 2) - cstd::lower_bound(sorted, sorted + 5, 2) == 3,
		"[test] bound multiplicity");
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 0) == sorted, "[test] lower_bound below all");
	CSTD_ASSERT(cstd::lower_bound(sorted, sorted + 5, 9) == sorted + 5, "[test] lower_bound above all");
	CSTD_ASSERT(cstd::binary_search(sorted, sorted + 5, 3) && !cstd::binary_search(sorted, sorted + 5, 4),
		"[test] binary_search present/absent");

	cstd::int32_t small[] = { 5, 2, 8, 1, 9, 3, 7, 4 };
	cstd::sort(small, small + 8);
	CSTD_ASSERT(cstd::is_sorted(small, small + 8) && small[0] == 1 && small[7] == 9, "[test] sort small (insertion)");
	CSTD_ASSERT(cstd::count(small, small + 8, 5) == 1, "[test] sort preserves elements");

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

	cstd::int32_t descending[] = { 3, 1, 4, 1, 5, 9, 2, 6 };
	cstd::sort(descending, descending + 8, cstd::greater{});
	CSTD_ASSERT(descending[0] == 9 && descending[7] == 1 && cstd::is_sorted(descending, descending + 8, cstd::greater{}),
		"[test] sort descending with comparator");

	cstd::int32_t one[] = { 42 };
	CSTD_ASSERT(cstd::is_sorted(one, one + 1) && cstd::is_sorted(one, one), "[test] is_sorted single/empty");
	cstd::int32_t disordered[] = { 1, 3, 2 };
	CSTD_ASSERT(!cstd::is_sorted(disordered, disordered + 3), "[test] is_sorted detects disorder");

	cstd::array<cstd::int32_t, 5> ra{ 4, 2, 5, 1, 3 };
	cstd::span<cstd::int32_t> ra_span(ra);
	cstd::ranges::sort(ra_span);
	CSTD_ASSERT(cstd::ranges::is_sorted(ra_span) && cstd::ranges::binary_search(ra_span, 3)
		&& !cstd::ranges::binary_search(ra_span, 9), "[test] ranges sort + binary_search");
}

static void test_algorithm_extra()
{
	cstd::int32_t values[] = { 1, 2, 3, 4, 5, 6 };
	cstd::int32_t* const first = values;
	cstd::int32_t* const last = values + 6;

	cstd::int32_t targets[] = { 9, 4, 7 };
	CSTD_ASSERT(cstd::find_first_of(first, last, targets, targets + 3) == values + 3, "[test] find_first_of");
	cstd::int32_t no_targets[] = { 99, 98 };
	CSTD_ASSERT(cstd::find_first_of(first, last, no_targets, no_targets + 2) == last, "[test] find_first_of miss returns last");

	cstd::int32_t dups[] = { 1, 2, 2, 3, 3, 3 };
	CSTD_ASSERT(cstd::adjacent_find(dups, dups + 6) == dups + 1, "[test] adjacent_find");
	CSTD_ASSERT(cstd::adjacent_find(first, last) == last, "[test] adjacent_find none returns last");

	cstd::int32_t to_replace[] = { 1, 2, 1, 3, 1 };
	cstd::replace(to_replace, to_replace + 5, 1, 0);
	CSTD_ASSERT(to_replace[0] == 0 && to_replace[2] == 0 && to_replace[4] == 0 && to_replace[1] == 2, "[test] replace");
	cstd::replace_if(to_replace, to_replace + 5, [](cstd::int32_t x) { return x > 1; }, 7);
	CSTD_ASSERT(to_replace[1] == 7 && to_replace[3] == 7 && to_replace[0] == 0, "[test] replace_if");

	cstd::int32_t evens[3] = {};
	cstd::int32_t* copy_end = cstd::copy_if(values, last, evens, [](cstd::int32_t x) { return x % 2 == 0; });
	CSTD_ASSERT(copy_end == evens + 3 && evens[0] == 2 && evens[1] == 4 && evens[2] == 6, "[test] copy_if");

	cstd::int32_t filled[4] = {};
	cstd::int32_t* fill_end = cstd::fill_n(filled, 3, 5);
	CSTD_ASSERT(fill_end == filled + 3 && filled[0] == 5 && filled[2] == 5 && filled[3] == 0, "[test] fill_n");

	cstd::int32_t unique_src[] = { 1, 1, 2, 3, 3, 3, 4 };
	cstd::int32_t* unique_end = cstd::unique(unique_src, unique_src + 7);
	CSTD_ASSERT(unique_end == unique_src + 4 && unique_src[0] == 1 && unique_src[1] == 2 && unique_src[2] == 3 && unique_src[3] == 4,
		"[test] unique collapses consecutive runs");
}

static void test_algorithm_iterators()
{
	cstd::single_linked_list<cstd::int32_t> slist;
	slist.push_back(1);
	slist.push_back(2);
	slist.push_back(3);
	slist.push_back(4);
	slist.push_back(5);

	CSTD_ASSERT(cstd::any_of(slist, [](cstd::int32_t x) { return x == 3; }),
		"[test] any_of with range");
	CSTD_ASSERT(!cstd::any_of(slist, [](cstd::int32_t x) { return x == 9; }),
		"[test] any_of miss with range");
	CSTD_ASSERT(cstd::all_of(slist, [](cstd::int32_t x) { return x > 0; }),
		"[test] all_of with range");
	CSTD_ASSERT(cstd::none_of(slist, [](cstd::int32_t x) { return x > 10; }),
		"[test] none_of with range");

	auto it = cstd::find(slist, 4);
	CSTD_ASSERT(it != slist.end() && *it == 4, "[test] find with range");
	CSTD_ASSERT(cstd::find(slist, 99) == slist.end(), "[test] find miss with range");

	auto pred_it = cstd::find_if(slist, [](cstd::int32_t x) { return x > 3; });
	CSTD_ASSERT(pred_it != slist.end() && *pred_it == 4, "[test] find_if with range");

	CSTD_ASSERT(cstd::count(slist, 3) == 1, "[test] count with range");
	CSTD_ASSERT(cstd::count_if(slist, [](cstd::int32_t x) { return x % 2 == 0; }) == 2,
		"[test] count_if with range");

	cstd::int32_t sum = 0;
	cstd::for_each(slist, [&sum](cstd::int32_t x) { sum += x; });
	CSTD_ASSERT(sum == 15, "[test] for_each with range");

	cstd::double_linked_list<cstd::int32_t> dlist;
	dlist.push_back(5);
	dlist.push_back(4);
	dlist.push_back(3);
	dlist.push_back(2);
	dlist.push_back(1);

	CSTD_ASSERT(cstd::equal(slist, dlist) == false, "[test] equal with range");

	cstd::reverse(dlist);

	CSTD_ASSERT(cstd::equal(slist, dlist), "[test] equal after reverse with range");

	auto adj = cstd::adjacent_find(slist);
	CSTD_ASSERT(adj == slist.end(), "[test] adjacent_find no dups with range");

	auto min_it = cstd::min_element(slist);
	CSTD_ASSERT(*min_it == 1, "[test] min_element with range");
	auto max_it = cstd::max_element(slist);
	CSTD_ASSERT(*max_it == 5, "[test] max_element with range");

	cstd::vector<cstd::int32_t> vec;
	vec.push_back(5);
	vec.push_back(3);
	vec.push_back(1);
	vec.push_back(4);
	vec.push_back(2);

	cstd::sort(vec);
	CSTD_ASSERT(cstd::is_sorted(vec), "[test] sort + is_sorted with range");
	CSTD_ASSERT(*cstd::lower_bound(vec, 3) == 3, "[test] lower_bound with range");
	CSTD_ASSERT(cstd::binary_search(vec, 4), "[test] binary_search with range");
	CSTD_ASSERT(!cstd::binary_search(vec, 9), "[test] binary_search miss with range");
}

static void test_algorithm_projected()
{
	struct entry
	{
		cstd::int32_t id;
		cstd::int32_t score;
	};

	cstd::vector<entry> entries;
	entries.push_back(entry{ 1, 80 });
	entries.push_back(entry{ 2, 95 });
	entries.push_back(entry{ 3, 60 });
	entries.push_back(entry{ 4, 100 });
	entries.push_back(entry{ 5, 70 });

	auto it = cstd::find(entries, 3, &entry::id);
	CSTD_ASSERT(it != entries.end() && it->score == 60, "[test] find with projection");
	CSTD_ASSERT(cstd::find(entries, 99, &entry::id) == entries.end(),
		"[test] find miss with projection");

	auto high = cstd::find_if(entries, [](cstd::int32_t s) { return s >= 100; }, &entry::score);
	CSTD_ASSERT(high != entries.end() && high->id == 4, "[test] find_if with projection");

	CSTD_ASSERT(cstd::any_of(entries, [](cstd::int32_t s) { return s > 90; }, &entry::score),
		"[test] any_of with projection");
	CSTD_ASSERT(!cstd::any_of(entries, [](cstd::int32_t s) { return s > 200; }, &entry::score),
		"[test] any_of miss with projection");

	CSTD_ASSERT(cstd::all_of(entries, [](cstd::int32_t s) { return s > 0; }, &entry::score),
		"[test] all_of with projection");
	CSTD_ASSERT(cstd::none_of(entries, [](cstd::int32_t s) { return s < 0; }, &entry::score),
		"[test] none_of with projection");

	CSTD_ASSERT(cstd::count(entries, 80, &entry::score) == 1, "[test] count with projection");
	CSTD_ASSERT(cstd::count_if(entries, [](cstd::int32_t s) { return s >= 80; }, &entry::score) == 3,
		"[test] count_if with projection");

	cstd::int32_t id_sum = 0;
	cstd::for_each(entries, [&id_sum](cstd::int32_t id) { id_sum += id; }, &entry::id);
	CSTD_ASSERT(id_sum == 15, "[test] for_each with projection");

	auto it2 = cstd::find(entries.begin(), entries.end(), 95, &entry::score);
	CSTD_ASSERT(it2 != entries.end() && it2->id == 2, "[test] find with projection (iterator-pair)");

	CSTD_ASSERT(cstd::contains(entries, 3, &entry::id), "[test] contains with projection");
	CSTD_ASSERT(!cstd::contains(entries, 99, &entry::id), "[test] contains miss with projection");

	cstd::vector<cstd::int32_t> nums;
	nums.push_back(10);
	nums.push_back(20);
	nums.push_back(30);
	CSTD_ASSERT(cstd::contains(nums, 20), "[test] contains with range");
	CSTD_ASSERT(!cstd::contains(nums, 99), "[test] contains miss with range");
	CSTD_ASSERT(cstd::contains(nums.begin(), nums.end(), 30), "[test] contains with iterators");

	struct thing
	{
		cstd::int32_t id;
	};

	cstd::vector<cstd::shared_ptr<thing>> ptrs;
	ptrs.push_back(cstd::make_shared<thing>(thing{ 1 }));
	ptrs.push_back(cstd::make_shared<thing>(thing{ 2 }));
	ptrs.push_back(cstd::make_shared<thing>(thing{ 3 }));

	CSTD_ASSERT(cstd::contains(ptrs, 2, &thing::id), "[test] contains through shared_ptr");
	CSTD_ASSERT(!cstd::contains(ptrs, 9, &thing::id), "[test] contains miss through shared_ptr");

	auto found = cstd::find(ptrs, 3, &thing::id);
	CSTD_ASSERT(found != ptrs.end() && (*found)->id == 3, "[test] find through shared_ptr");

	CSTD_ASSERT(cstd::any_of(ptrs, [](cstd::int32_t id) { return id > 2; }, &thing::id),
		"[test] any_of through shared_ptr");
	CSTD_ASSERT(cstd::count(ptrs, 1, &thing::id) == 1, "[test] count through shared_ptr");
}

static void test_flat_map()
{
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

	cstd::mutex guarded;
	{
		cstd::lock_guard<cstd::mutex> lock(guarded);
		CSTD_ASSERT(!guarded.try_lock(), "[mutex] lock_guard holds during scope");
	}
	CSTD_ASSERT(guarded.try_lock(), "[mutex] lock_guard releases at scope exit");
	guarded.unlock();

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

	cstd::unique_lock<cstd::mutex> empty;
	CSTD_ASSERT(!empty.owns_lock(), "[unique_lock] default does not own");
	CSTD_ASSERT(empty.mutex() == nullptr, "[unique_lock] default has no mutex");
	CSTD_ASSERT(!static_cast<bool>(empty), "[unique_lock] default is falsy");

	{
		cstd::unique_lock<cstd::mutex> lock(m);
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] ctor locks");
		CSTD_ASSERT(lock.mutex() == &m, "[unique_lock] ctor records mutex");
		CSTD_ASSERT(static_cast<bool>(lock), "[unique_lock] owning lock is truthy");
		CSTD_ASSERT(!m.try_lock(), "[unique_lock] mutex held during scope");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] dtor unlocks");
	m.unlock();

	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::defer_lock);
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] defer does not lock");
		lock.lock();
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] lock() acquires");
		lock.unlock();
		CSTD_ASSERT(!lock.owns_lock(), "[unique_lock] unlock() releases");
	}

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

	m.lock();
	{
		cstd::unique_lock<cstd::mutex> lock(m, cstd::adopt_lock);
		CSTD_ASSERT(lock.owns_lock(), "[unique_lock] adopt owns");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] adopt releases at scope exit");
	m.unlock();

	{
		cstd::unique_lock<cstd::mutex> source(m);
		cstd::unique_lock<cstd::mutex> sink(cstd::move(source));
		CSTD_ASSERT(!source.owns_lock() && source.mutex() == nullptr, "[unique_lock] move empties source");
		CSTD_ASSERT(sink.owns_lock() && sink.mutex() == &m, "[unique_lock] move transfers ownership");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] moved-to lock unlocks at scope exit");
	m.unlock();

	{
		cstd::unique_lock<cstd::mutex> a(m);
		cstd::unique_lock<cstd::mutex> b;
		b = cstd::move(a);
		CSTD_ASSERT(b.owns_lock() && !a.owns_lock(), "[unique_lock] move-assign transfers");
	}
	CSTD_ASSERT(m.try_lock(), "[unique_lock] move-assign target unlocks at scope exit");
	m.unlock();

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

static void test_function()
{
	cstd::function<cstd::int32_t()> empty;
	CSTD_ASSERT(!empty, "[function] default is empty");

	cstd::function<cstd::int32_t()> null_fn(nullptr);
	CSTD_ASSERT(!null_fn, "[function] nullptr is empty");

	cstd::function<cstd::int32_t(cstd::int32_t, cstd::int32_t)> add =
		[](cstd::int32_t a, cstd::int32_t b) { return a + b; };
	CSTD_ASSERT(static_cast<bool>(add), "[function] lambda is non-empty");
	CSTD_ASSERT(add(3, 4) == 7, "[function] stateless lambda");

	cstd::int32_t captured = 42;
	cstd::function<cstd::int32_t()> by_value = [captured]() { return captured; };
	CSTD_ASSERT(by_value() == 42, "[function] capture by value");

	cstd::int32_t ref_target = 5;
	cstd::function<void()> by_ref = [&ref_target]() { ref_target = 99; };
	by_ref();
	CSTD_ASSERT(ref_target == 99, "[function] capture by reference");

	cstd::function<cstd::int32_t()> original = by_value;
	cstd::function<cstd::int32_t()> copied = original;
	CSTD_ASSERT(original() == 42 && copied() == 42, "[function] copy preserves both");

	cstd::function<cstd::int32_t()> moved = cstd::move(copied);
	CSTD_ASSERT(moved() == 42, "[function] move transfers target");
	CSTD_ASSERT(!copied, "[function] move empties source");

	moved = nullptr;
	CSTD_ASSERT(!moved, "[function] nullptr assignment empties");

	cstd::function<cstd::int32_t()> sa = []() { return 1; };
	cstd::function<cstd::int32_t()> sb = []() { return 2; };
	sa.swap(sb);
	CSTD_ASSERT(sa() == 2 && sb() == 1, "[function] swap");

	cstd::int32_t side_effect = 0;
	cstd::function<void()> void_fn = [&side_effect]() { side_effect = 7; };
	void_fn();
	CSTD_ASSERT(side_effect == 7, "[function] void return");
}

static void test_function_lifetime()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[function] probe baseline");

	{
		lifetime_probe probe(10);
		cstd::function<cstd::int32_t()> fn = [probe]() { return probe.value; };
		CSTD_ASSERT(fn() == 10, "[function] captured probe");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[function] probes destroyed with function");

	{
		cstd::function<cstd::int32_t()> fn = [p = lifetime_probe(1)]() { return p.value; };
		CSTD_ASSERT(lifetime_probe::live == 1 && fn() == 1, "[function] one probe in closure");

		fn = [p = lifetime_probe(2)]() { return p.value; };
		CSTD_ASSERT(lifetime_probe::live == 1 && fn() == 2, "[function] reassign destroys old probe");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[function] all probes cleaned up");

	{
		cstd::function<cstd::int32_t()> fn = [p = lifetime_probe(3)]() { return p.value; };
		lifetime_probe::copies = 0;
		cstd::function<cstd::int32_t()> copy = fn;
		CSTD_ASSERT(copy() == 3 && lifetime_probe::copies >= 1, "[function] copy duplicates capture");
		CSTD_ASSERT(lifetime_probe::live == 2, "[function] copy has independent probe");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[function] copies cleaned up");

	{
		cstd::int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
		cstd::function<cstd::int64_t()> big = [a, b, c, d, e]() { return a + b + c + d + e; };
		CSTD_ASSERT(big() == 15, "[function] large capture (heap path)");

		cstd::function<cstd::int64_t()> big_copy = big;
		CSTD_ASSERT(big_copy() == 15, "[function] copy of heap-allocated");
	}
}

static void test_single_linked_list()
{
	cstd::single_linked_list<cstd::int32_t> l;
	CSTD_ASSERT(l.empty() && l.size() == 0, "[slist] default is empty");

	l.push_back(1);
	l.push_back(2);
	l.push_back(3);
	CSTD_ASSERT(l.size() == 3, "[slist] push_back grows size");
	CSTD_ASSERT(l.front() == 1 && l.back() == 3, "[slist] push_back order");

	l.push_front(0);
	CSTD_ASSERT(l.size() == 4 && l.front() == 0, "[slist] push_front");

	l.pop_front();
	CSTD_ASSERT(l.size() == 3 && l.front() == 1, "[slist] pop_front");

	cstd::int32_t sum = 0;
	for (auto x : l)
	{
		sum += x;
	}
	CSTD_ASSERT(sum == 6, "[slist] range-for");

	cstd::single_linked_list<cstd::int32_t> copy = l;
	CSTD_ASSERT(copy.size() == 3 && copy.front() == 1 && copy.back() == 3, "[slist] copy ctor");
	copy.push_back(9);
	CSTD_ASSERT(l.size() == 3, "[slist] copy is independent");

	cstd::single_linked_list<cstd::int32_t> moved = cstd::move(copy);
	CSTD_ASSERT(moved.size() == 4 && moved.back() == 9, "[slist] move ctor transfers");
	CSTD_ASSERT(copy.empty(), "[slist] move ctor empties source");

	moved.clear();
	CSTD_ASSERT(moved.empty(), "[slist] clear");

	cstd::single_linked_list<cstd::int32_t> a;
	a.push_back(1); a.push_back(2);
	cstd::single_linked_list<cstd::int32_t> b;
	b.push_back(1); b.push_back(2);
	CSTD_ASSERT(a == b, "[slist] equality");
	b.push_back(3);
	CSTD_ASSERT(a != b && a < b, "[slist] less by length");
}

static void test_single_linked_list_iterator()
{
	cstd::single_linked_list<cstd::int32_t> l;
	l.push_back(10);
	l.push_back(20);
	l.push_back(30);

	auto inserted = l.insert_after(l.before_begin(), 5);
	CSTD_ASSERT(*inserted == 5 && l.size() == 4 && l.front() == 5, "[slist] insert_after before_begin");

	auto it = l.begin();
	l.insert_after(it, 15);
	CSTD_ASSERT(l.size() == 5, "[slist] insert_after middle");
	it = l.begin();
	++it;
	CSTD_ASSERT(*it == 15, "[slist] insert_after order");

	l.erase_after(l.before_begin());
	CSTD_ASSERT(l.size() == 4 && l.front() == 15, "[slist] erase_after before_begin");

	l.erase_after(l.begin());
	CSTD_ASSERT(l.size() == 3, "[slist] erase_after middle");
	it = l.begin();
	++it;
	CSTD_ASSERT(*it == 20, "[slist] erase_after removes correct node");

	l.reverse();
	CSTD_ASSERT(l.front() == 30 && l.back() == 15, "[slist] reverse");

	l.push_back(20);
	cstd::size_t removed = l.remove(20);
	CSTD_ASSERT(removed == 2 && l.size() == 2, "[slist] remove count");
	CSTD_ASSERT(l.front() == 30 && l.back() == 15, "[slist] remove keeps others");

	l.push_back(5);
	l.push_back(40);
	l.remove_if([](cstd::int32_t x) { return x >= 20; });
	CSTD_ASSERT(l.size() == 2 && l.front() == 15 && l.back() == 5, "[slist] remove_if");

	cstd::single_linked_list<cstd::int32_t> x;
	x.push_back(1);
	cstd::single_linked_list<cstd::int32_t> y;
	y.push_back(8);
	y.push_back(9);
	x.swap(y);
	CSTD_ASSERT(x.size() == 2 && x.front() == 8 && y.size() == 1 && y.front() == 1, "[slist] swap");

	cstd::single_linked_list<cstd::int32_t> empty;
	cstd::single_linked_list<cstd::int32_t> full;
	full.push_back(7);
	empty.swap(full);
	CSTD_ASSERT(empty.size() == 1 && empty.front() == 7 && full.empty(), "[slist] swap with empty");

	cstd::single_linked_list<cstd::int32_t> e;
	cstd::int32_t& ref = e.emplace_back(42);
	CSTD_ASSERT(ref == 42 && e.back() == 42, "[slist] emplace_back");
	e.emplace_front(7);
	CSTD_ASSERT(e.front() == 7, "[slist] emplace_front");
}

static void test_single_linked_list_lifetime()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[slist] probe baseline");

	{
		cstd::single_linked_list<lifetime_probe> l;
		l.emplace_back(1);
		l.emplace_back(2);
		l.emplace_back(3);
		CSTD_ASSERT(lifetime_probe::live == 3, "[slist] emplace creates probes");

		l.pop_front();
		CSTD_ASSERT(lifetime_probe::live == 2, "[slist] pop_front destroys one");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[slist] destructor cleans up all");

	{
		cstd::single_linked_list<lifetime_probe> l;
		l.emplace_back(1);
		l.emplace_back(2);
		l.emplace_back(3);
		l.clear();
		CSTD_ASSERT(lifetime_probe::live == 0, "[slist] clear destroys all");
	}
}

static void test_double_linked_list()
{
	cstd::double_linked_list<cstd::int32_t> l;
	CSTD_ASSERT(l.empty() && l.size() == 0, "[dlist] default is empty");

	l.push_back(1);
	l.push_back(2);
	l.push_back(3);
	CSTD_ASSERT(l.size() == 3, "[dlist] push_back grows size");
	CSTD_ASSERT(l.front() == 1 && l.back() == 3, "[dlist] push_back order");

	l.push_front(0);
	CSTD_ASSERT(l.size() == 4 && l.front() == 0, "[dlist] push_front");

	l.pop_back();
	CSTD_ASSERT(l.size() == 3 && l.back() == 2, "[dlist] pop_back");

	l.pop_front();
	CSTD_ASSERT(l.size() == 2 && l.front() == 1, "[dlist] pop_front");

	cstd::int32_t sum = 0;
	for (auto x : l)
	{
		sum += x;
	}
	CSTD_ASSERT(sum == 3, "[dlist] range-for");

	cstd::double_linked_list<cstd::int32_t> copy = l;
	CSTD_ASSERT(copy.size() == 2 && copy.front() == 1 && copy.back() == 2, "[dlist] copy ctor");
	copy.push_back(9);
	CSTD_ASSERT(l.size() == 2, "[dlist] copy is independent");

	cstd::double_linked_list<cstd::int32_t> moved = cstd::move(copy);
	CSTD_ASSERT(moved.size() == 3 && moved.back() == 9, "[dlist] move ctor transfers");
	CSTD_ASSERT(copy.empty(), "[dlist] move ctor empties source");

	moved.clear();
	CSTD_ASSERT(moved.empty(), "[dlist] clear");

	cstd::double_linked_list<cstd::int32_t> a;
	a.push_back(1); a.push_back(2);
	cstd::double_linked_list<cstd::int32_t> b;
	b.push_back(1); b.push_back(2);
	CSTD_ASSERT(a == b, "[dlist] equality");
	b.push_back(3);
	CSTD_ASSERT(a != b && a < b, "[dlist] less by length");
}

static void test_double_linked_list_iterator()
{
	cstd::double_linked_list<cstd::int32_t> l;
	l.push_back(10);
	l.push_back(20);
	l.push_back(30);

	auto it = l.begin();
	++it;
	auto inserted = l.insert(it, 15);
	CSTD_ASSERT(*inserted == 15 && l.size() == 4, "[dlist] insert returns new element");

	it = l.begin();
	CSTD_ASSERT(*it == 10, "[dlist] insert order[0]");
	++it;
	CSTD_ASSERT(*it == 15, "[dlist] insert order[1]");
	++it;
	CSTD_ASSERT(*it == 20, "[dlist] insert order[2]");
	++it;
	CSTD_ASSERT(*it == 30, "[dlist] insert order[3]");

	it = l.begin();
	++it;
	auto after = l.erase(it);
	CSTD_ASSERT(*after == 20 && l.size() == 3, "[dlist] erase returns next");
	CSTD_ASSERT(l.front() == 10, "[dlist] erase keeps other elements");

	l.reverse();
	CSTD_ASSERT(l.front() == 30 && l.back() == 10, "[dlist] reverse");

	l.push_back(20);
	cstd::size_t removed = l.remove(20);
	CSTD_ASSERT(removed == 2 && l.size() == 2, "[dlist] remove count");
	CSTD_ASSERT(l.front() == 30 && l.back() == 10, "[dlist] remove keeps others");

	l.push_back(5);
	l.push_back(40);
	l.remove_if([](cstd::int32_t x) { return x >= 20; });
	CSTD_ASSERT(l.size() == 2 && l.front() == 10 && l.back() == 5, "[dlist] remove_if");

	cstd::double_linked_list<cstd::int32_t> x;
	x.push_back(1);
	cstd::double_linked_list<cstd::int32_t> y;
	y.push_back(8);
	y.push_back(9);
	x.swap(y);
	CSTD_ASSERT(x.size() == 2 && x.front() == 8 && y.size() == 1 && y.front() == 1, "[dlist] swap");

	cstd::double_linked_list<cstd::int32_t> e;
	cstd::int32_t& ref = e.emplace_back(42);
	CSTD_ASSERT(ref == 42 && e.back() == 42, "[dlist] emplace_back");
	e.emplace_front(7);
	CSTD_ASSERT(e.front() == 7, "[dlist] emplace_front");
}

static void test_double_linked_list_lifetime()
{
	CSTD_ASSERT(lifetime_probe::live == 0, "[dlist] probe baseline");

	{
		cstd::double_linked_list<lifetime_probe> l;
		l.emplace_back(1);
		l.emplace_back(2);
		l.emplace_back(3);
		CSTD_ASSERT(lifetime_probe::live == 3, "[dlist] emplace creates probes");

		l.pop_back();
		CSTD_ASSERT(lifetime_probe::live == 2, "[dlist] pop_back destroys one");

		l.pop_front();
		CSTD_ASSERT(lifetime_probe::live == 1, "[dlist] pop_front destroys one");
	}
	CSTD_ASSERT(lifetime_probe::live == 0, "[dlist] destructor cleans up all");

	{
		cstd::double_linked_list<lifetime_probe> l;
		l.emplace_back(1);
		l.emplace_back(2);
		l.emplace_back(3);
		l.clear();
		CSTD_ASSERT(lifetime_probe::live == 0, "[dlist] clear destroys all");
	}
}

static void test_pointer_comparisons()
{
	cstd::unique_ptr<cstd::int32_t> u1 = cstd::make_unique<cstd::int32_t>(1);
	cstd::unique_ptr<cstd::int32_t> u2 = cstd::make_unique<cstd::int32_t>(2);
	cstd::unique_ptr<cstd::int32_t> u_null;

	CSTD_ASSERT(u1 == u1 && u1 != u2, "[unique_ptr] self-equality and inequality");
	CSTD_ASSERT(u_null == nullptr && nullptr == u_null, "[unique_ptr] null == nullptr");
	CSTD_ASSERT(u1 != nullptr && nullptr != u1, "[unique_ptr] non-null != nullptr");
	CSTD_ASSERT((u1 < u2) == (u1.get() < u2.get()), "[unique_ptr] ordering matches raw pointer");
	CSTD_ASSERT((u1 <= u1) && (u1 >= u1), "[unique_ptr] reflexive <= and >=");

	cstd::shared_ptr<cstd::int32_t> s1 = cstd::make_shared<cstd::int32_t>(10);
	cstd::shared_ptr<cstd::int32_t> s2 = cstd::make_shared<cstd::int32_t>(20);
	cstd::shared_ptr<cstd::int32_t> s_copy = s1;
	cstd::shared_ptr<cstd::int32_t> s_null;

	CSTD_ASSERT(s1 == s_copy && s1 != s2, "[shared_ptr] equality with alias");
	CSTD_ASSERT(s_null == nullptr && nullptr == s_null, "[shared_ptr] null == nullptr");
	CSTD_ASSERT(s1 != nullptr && nullptr != s1, "[shared_ptr] non-null != nullptr");
	CSTD_ASSERT((s1 < s2) == (s1.get() < s2.get()), "[shared_ptr] ordering matches raw pointer");
	CSTD_ASSERT((s1 <= s1) && (s1 >= s1), "[shared_ptr] reflexive <= and >=");
}

static void test_optional_comparisons()
{
	cstd::optional<cstd::int32_t> a(3);
	cstd::optional<cstd::int32_t> b(5);
	cstd::optional<cstd::int32_t> a2(3);
	cstd::optional<cstd::int32_t> empty;
	cstd::optional<cstd::int32_t> empty2;

	CSTD_ASSERT(a == a2 && a != b, "[optional] value equality/inequality");
	CSTD_ASSERT(a < b && b > a && a <= b && b >= a, "[optional] value ordering");
	CSTD_ASSERT(a <= a2 && a >= a2, "[optional] equal values have <= and >=");
	CSTD_ASSERT(empty == empty2, "[optional] two empties are equal");
	CSTD_ASSERT(empty < a && a > empty, "[optional] empty < engaged");
	CSTD_ASSERT(!(empty > a) && !(empty == a), "[optional] empty not > engaged and not ==");

	CSTD_ASSERT(empty == cstd::nullopt && cstd::nullopt == empty, "[optional] empty == nullopt");
	CSTD_ASSERT(a != cstd::nullopt && cstd::nullopt != a, "[optional] engaged != nullopt");

	CSTD_ASSERT(a == 3 && 3 == a, "[optional] value comparison ==");
	CSTD_ASSERT(a != 5 && 5 != a, "[optional] value comparison !=");
	CSTD_ASSERT(!(empty == 3) && 3 != empty, "[optional] empty != any value");
}

static void test_function_comparisons()
{
	cstd::function<cstd::int32_t()> empty;
	cstd::function<cstd::int32_t()> active = []() { return 1; };

	CSTD_ASSERT(empty == nullptr && nullptr == empty, "[function] empty == nullptr");
	CSTD_ASSERT(active != nullptr && nullptr != active, "[function] active != nullptr");
}

static void test_unordered_map()
{
	cstd::unordered_map<int, cstd::string> map;

	CSTD_ASSERT(map.empty(), "[unordered_map] default not empty");
	CSTD_ASSERT(map.size() == 0, "[unordered_map] default size != 0");

	map[1] = "one";
	map[2] = "two";
	map[3] = "three";

	CSTD_ASSERT(map.size() == 3, "[unordered_map] size after insert");
	CSTD_ASSERT(!map.empty(), "[unordered_map] not empty after insert");

	CSTD_ASSERT(map[1] == "one", "[unordered_map] operator[] read 1");
	CSTD_ASSERT(map[2] == "two", "[unordered_map] operator[] read 2");
	CSTD_ASSERT(map[3] == "three", "[unordered_map] operator[] read 3");

	auto found = map.find(2);
	CSTD_ASSERT(found != nullptr, "[unordered_map] find existing");
	CSTD_ASSERT(found->first == 2, "[unordered_map] find key");
	CSTD_ASSERT(found->second == "two", "[unordered_map] find value");

	auto missing = map.find(99);
	CSTD_ASSERT(missing == nullptr, "[unordered_map] find missing");

	CSTD_ASSERT(map.contains(1), "[unordered_map] contains existing");
	CSTD_ASSERT(!map.contains(99), "[unordered_map] contains missing");
	CSTD_ASSERT(map.count(1) == 1, "[unordered_map] count existing");
	CSTD_ASSERT(map.count(99) == 0, "[unordered_map] count missing");

	CSTD_ASSERT(map.at(3) == "three", "[unordered_map] at() existing");

	map[1] = "ONE";
	CSTD_ASSERT(map[1] == "ONE", "[unordered_map] overwrite via operator[]");
	CSTD_ASSERT(map.size() == 3, "[unordered_map] size unchanged after overwrite");

	CSTD_ASSERT(map.erase(2) == 1, "[unordered_map] erase existing");
	CSTD_ASSERT(map.size() == 2, "[unordered_map] size after erase");
	CSTD_ASSERT(!map.contains(2), "[unordered_map] erased key gone");
	CSTD_ASSERT(map.erase(2) == 0, "[unordered_map] erase missing");

	map.clear();
	CSTD_ASSERT(map.empty(), "[unordered_map] empty after clear");
	CSTD_ASSERT(map.size() == 0, "[unordered_map] size 0 after clear");
}

static void test_unordered_map_iteration()
{
	cstd::unordered_map<int, int> map;

	auto b = map.begin();
	auto e = map.end();
	CSTD_ASSERT(b == e, "[unordered_map] empty begin == end");

	map[10] = 100;
	map[20] = 200;
	map[30] = 300;

	cstd::size_t count = 0;
	int key_sum = 0;
	int val_sum = 0;

	for (auto it = map.begin(); it != map.end(); ++it)
	{
		key_sum += it->first;
		val_sum += it->second;
		++count;
	}

	CSTD_ASSERT(count == 3, "[unordered_map] iteration count");
	CSTD_ASSERT(key_sum == 60, "[unordered_map] iteration key sum");
	CSTD_ASSERT(val_sum == 600, "[unordered_map] iteration val sum");

	const auto& cmap = map;
	cstd::size_t ccount = 0;

	for (auto it = cmap.begin(); it != cmap.end(); ++it)
	{
		++ccount;
	}

	CSTD_ASSERT(ccount == 3, "[unordered_map] const iteration count");
}

static void test_unordered_map_modifiers()
{
	cstd::unordered_map<int, cstd::string> a;

	auto r1 = a.insert(cstd::pair<int, cstd::string>(1, "one"));
	CSTD_ASSERT(r1.second == true, "[unordered_map] insert new");
	CSTD_ASSERT(r1.first->second == "one", "[unordered_map] insert value");

	auto r2 = a.insert(cstd::pair<int, cstd::string>(1, "ONE"));
	CSTD_ASSERT(r2.second == false, "[unordered_map] insert duplicate");
	CSTD_ASSERT(r2.first->second == "one", "[unordered_map] insert kept original");

	auto r3 = a.insert(2, "two");
	CSTD_ASSERT(r3.second == true, "[unordered_map] insert(k,v) new");

	auto r4 = a.emplace(3, "three");
	CSTD_ASSERT(r4.second == true, "[unordered_map] emplace new");
	CSTD_ASSERT(a[3] == "three", "[unordered_map] emplace value");

	auto r5 = a.emplace(3, "THREE");
	CSTD_ASSERT(r5.second == false, "[unordered_map] emplace duplicate");

	cstd::unordered_map<int, cstd::string> b;
	b[10] = "ten";
	b[20] = "twenty";

	a.swap(b);
	CSTD_ASSERT(a.size() == 2, "[unordered_map] swap a size");
	CSTD_ASSERT(b.size() == 3, "[unordered_map] swap b size");
	CSTD_ASSERT(a.contains(10), "[unordered_map] swap a has 10");
	CSTD_ASSERT(b.contains(1), "[unordered_map] swap b has 1");

	cstd::unordered_map<int, cstd::string> copy(b);
	CSTD_ASSERT(copy.size() == b.size(), "[unordered_map] copy size");
	CSTD_ASSERT(copy[1] == "one", "[unordered_map] copy value");

	cstd::unordered_map<int, cstd::string> moved(cstd::move(copy));
	CSTD_ASSERT(moved.size() == 3, "[unordered_map] move ctor size");
	CSTD_ASSERT(copy.empty(), "[unordered_map] move ctor source empty");

	cstd::unordered_map<int, int> eq1;
	cstd::unordered_map<int, int> eq2;
	eq1[1] = 10;
	eq1[2] = 20;
	eq2[2] = 20;
	eq2[1] = 10;
	CSTD_ASSERT(eq1 == eq2, "[unordered_map] equality");
	eq2[3] = 30;
	CSTD_ASSERT(eq1 != eq2, "[unordered_map] inequality size");
}

static void test_unordered_map_rehash()
{
	cstd::unordered_map<int, int> map(4);
	CSTD_ASSERT(map.bucket_count() == 4, "[unordered_map] custom bucket count");

	for (int i = 0; i < 20; ++i)
	{
		map[i] = i * 10;
	}

	CSTD_ASSERT(map.size() == 20, "[unordered_map] size after many inserts");
	CSTD_ASSERT(map.bucket_count() > 4, "[unordered_map] rehashed");

	for (int i = 0; i < 20; ++i)
	{
		CSTD_ASSERT(map[i] == i * 10, "[unordered_map] values survived rehash");
	}

	cstd::size_t bc = map.bucket_count();
	map.rehash(bc * 2);
	CSTD_ASSERT(map.bucket_count() >= bc * 2, "[unordered_map] manual rehash");
	CSTD_ASSERT(map.size() == 20, "[unordered_map] size after manual rehash");

	for (int i = 0; i < 20; ++i)
	{
		CSTD_ASSERT(map.contains(i), "[unordered_map] contains after rehash");
	}

	map.reserve(100);
	CSTD_ASSERT(map.bucket_count() >= 100, "[unordered_map] reserve");

	CSTD_ASSERT(map.max_load_factor() == 1, "[unordered_map] default mlf");
	map.max_load_factor(2);
	CSTD_ASSERT(map.max_load_factor() == 2, "[unordered_map] set mlf");
}

static void test_ring_buffer()
{
	cstd::ring_buffer<cstd::int32_t> rb(3);
	CSTD_ASSERT(rb.empty(), "[ring_buffer] should be empty after construction");
	CSTD_ASSERT(rb.capacity() == 3, "[ring_buffer] capacity mismatch");
	CSTD_ASSERT(rb.size() == 0, "[ring_buffer] size should be 0");

	rb.push_back(1);
	rb.push_back(2);
	rb.push_back(3);
	CSTD_ASSERT(rb.full(), "[ring_buffer] should be full");
	CSTD_ASSERT(rb.front() == 1, "[ring_buffer] front mismatch");
	CSTD_ASSERT(rb.back() == 3, "[ring_buffer] back mismatch");
	CSTD_ASSERT(rb[0] == 1 && rb[1] == 2 && rb[2] == 3, "[ring_buffer] indexed access");

	rb.push_back(4);
	CSTD_ASSERT(rb.size() == 3, "[ring_buffer] size stays at capacity after overwrite");
	CSTD_ASSERT(rb.front() == 2, "[ring_buffer] oldest dropped on overwrite");
	CSTD_ASSERT(rb.back() == 4, "[ring_buffer] newest is 4");
	CSTD_ASSERT(rb[0] == 2 && rb[1] == 3 && rb[2] == 4, "[ring_buffer] logical order after wrap");

	rb.push_back(5);
	rb.push_back(6);
	CSTD_ASSERT(rb[0] == 4 && rb[1] == 5 && rb[2] == 6, "[ring_buffer] multiple overwrites");

	rb.pop_front();
	CSTD_ASSERT(rb.size() == 2, "[ring_buffer] size after pop_front");
	CSTD_ASSERT(rb.front() == 5, "[ring_buffer] front after pop_front");

	rb.pop_back();
	CSTD_ASSERT(rb.size() == 1, "[ring_buffer] size after pop_back");
	CSTD_ASSERT(rb.front() == 5 && rb.back() == 5, "[ring_buffer] single element");

	rb.clear();
	CSTD_ASSERT(rb.empty(), "[ring_buffer] empty after clear");
	CSTD_ASSERT(rb.capacity() == 3, "[ring_buffer] capacity unchanged after clear");
}

static void test_ring_buffer_push_front()
{
	cstd::ring_buffer<cstd::int32_t> rb(3);
	rb.push_front(10);
	rb.push_front(20);
	rb.push_front(30);
	CSTD_ASSERT(rb.front() == 30, "[ring_buffer] front after push_fronts");
	CSTD_ASSERT(rb.back() == 10, "[ring_buffer] back after push_fronts");
	CSTD_ASSERT(rb[0] == 30 && rb[1] == 20 && rb[2] == 10, "[ring_buffer] push_front order");

	rb.push_front(40);
	CSTD_ASSERT(rb.size() == 3, "[ring_buffer] size after overwriting push_front");
	CSTD_ASSERT(rb.front() == 40, "[ring_buffer] front after overwrite push_front");
	CSTD_ASSERT(rb.back() == 20, "[ring_buffer] back dropped by push_front");
	CSTD_ASSERT(rb[0] == 40 && rb[1] == 30 && rb[2] == 20, "[ring_buffer] push_front overwrite order");
}

static void test_ring_buffer_iteration()
{
	cstd::ring_buffer<cstd::int32_t> rb(4);
	rb.push_back(10);
	rb.push_back(20);
	rb.push_back(30);

	cstd::int32_t sum = 0;
	for (auto it = rb.begin(); it != rb.end(); ++it)
	{
		sum += *it;
	}
	CSTD_ASSERT(sum == 60, "[ring_buffer] iterator sum");

	sum = 0;
	for (const auto& val : rb)
	{
		sum += val;
	}
	CSTD_ASSERT(sum == 60, "[ring_buffer] range-for sum");

	rb.push_back(40);
	rb.push_back(50);

	sum = 0;
	for (const auto& val : rb)
	{
		sum += val;
	}
	CSTD_ASSERT(sum == 140, "[ring_buffer] wrapped iteration sum");
	CSTD_ASSERT(rb[0] == 20 && rb[1] == 30 && rb[2] == 40 && rb[3] == 50,
		"[ring_buffer] wrapped logical order");

	cstd::ring_buffer<cstd::int32_t> empty_rb(2);
	cstd::int32_t count = 0;
	for ([[maybe_unused]] const auto& val : empty_rb)
	{
		++count;
	}
	CSTD_ASSERT(count == 0, "[ring_buffer] empty iteration");
}

static void test_ring_buffer_copy_move()
{
	cstd::ring_buffer<cstd::int32_t> rb(3);
	rb.push_back(1);
	rb.push_back(2);
	rb.push_back(3);
	rb.push_back(4);

	cstd::ring_buffer<cstd::int32_t> copy(rb);
	CSTD_ASSERT(copy.size() == 3, "[ring_buffer] copy size");
	CSTD_ASSERT(copy.capacity() == 3, "[ring_buffer] copy capacity");
	CSTD_ASSERT(copy[0] == 2 && copy[1] == 3 && copy[2] == 4, "[ring_buffer] copy values");
	CSTD_ASSERT(copy == rb, "[ring_buffer] copy equality");

	cstd::ring_buffer<cstd::int32_t> moved(cstd::move(copy));
	CSTD_ASSERT(moved.size() == 3, "[ring_buffer] moved size");
	CSTD_ASSERT(moved[0] == 2 && moved[1] == 3 && moved[2] == 4, "[ring_buffer] moved values");
	CSTD_ASSERT(copy.empty() && copy.capacity() == 0, "[ring_buffer] source empty after move");

	cstd::ring_buffer<cstd::int32_t> assigned(5);
	assigned = rb;
	CSTD_ASSERT(assigned == rb, "[ring_buffer] copy assignment");
	CSTD_ASSERT(assigned.capacity() == 3, "[ring_buffer] copy assignment capacity");

	cstd::ring_buffer<cstd::int32_t> move_assigned(5);
	move_assigned = cstd::move(assigned);
	CSTD_ASSERT(move_assigned.size() == 3, "[ring_buffer] move assignment size");
	CSTD_ASSERT(assigned.empty(), "[ring_buffer] source empty after move assignment");

	cstd::ring_buffer<cstd::int32_t> a(2), b(3);
	a.push_back(1);
	b.push_back(2);
	b.push_back(3);

	a.swap(b);
	CSTD_ASSERT(a.size() == 2 && a.capacity() == 3, "[ring_buffer] swap a");
	CSTD_ASSERT(b.size() == 1 && b.capacity() == 2, "[ring_buffer] swap b");
	CSTD_ASSERT(a[0] == 2 && a[1] == 3, "[ring_buffer] swap a values");
	CSTD_ASSERT(b[0] == 1, "[ring_buffer] swap b values");
}

static void test_ring_buffer_emplace()
{
	cstd::ring_buffer<cstd::int32_t> rb(2);
	cstd::int32_t& ref1 = rb.emplace_back(100);
	CSTD_ASSERT(ref1 == 100, "[ring_buffer] emplace_back return");
	CSTD_ASSERT(rb.front() == 100, "[ring_buffer] emplace_back value");

	cstd::int32_t& ref2 = rb.emplace_front(200);
	CSTD_ASSERT(ref2 == 200, "[ring_buffer] emplace_front return");
	CSTD_ASSERT(rb.front() == 200, "[ring_buffer] emplace_front value");
	CSTD_ASSERT(rb.back() == 100, "[ring_buffer] emplace_front preserved back");

	cstd::int32_t& ref3 = rb.emplace_back(300);
	CSTD_ASSERT(ref3 == 300, "[ring_buffer] emplace_back overwrite return");
	CSTD_ASSERT(rb.front() == 100, "[ring_buffer] emplace_back overwrite front");
	CSTD_ASSERT(rb.back() == 300, "[ring_buffer] emplace_back overwrite back");
}

static void test_ring_buffer_comparisons()
{
	cstd::ring_buffer<cstd::int32_t> a(3), b(5);
	a.push_back(1);
	a.push_back(2);
	b.push_back(1);
	b.push_back(2);
	CSTD_ASSERT(a == b, "[ring_buffer] equal despite different capacity");
	CSTD_ASSERT(!(a != b), "[ring_buffer] not unequal");

	b.push_back(3);
	CSTD_ASSERT(a < b, "[ring_buffer] shorter is less");
	CSTD_ASSERT(a <= b, "[ring_buffer] shorter is less-equal");
	CSTD_ASSERT(b > a, "[ring_buffer] longer is greater");
	CSTD_ASSERT(b >= a, "[ring_buffer] longer is greater-equal");
	CSTD_ASSERT(a != b, "[ring_buffer] different sizes unequal");

	cstd::ring_buffer<cstd::int32_t> c(3), d(3);
	c.push_back(1);
	c.push_back(3);
	d.push_back(1);
	d.push_back(2);
	CSTD_ASSERT(d < c, "[ring_buffer] lexicographic less");
}

static void test_ring_buffer_lifetime()
{
	lifetime_probe::live = 0;
	lifetime_probe::copies = 0;
	lifetime_probe::moves = 0;

	{
		cstd::ring_buffer<lifetime_probe> rb(3);
		rb.emplace_back(1);
		rb.emplace_back(2);
		rb.emplace_back(3);
		CSTD_ASSERT(lifetime_probe::live == 3, "[ring_buffer] live after 3 emplace");

		rb.emplace_back(4);
		CSTD_ASSERT(lifetime_probe::live == 3, "[ring_buffer] live after overwrite");
		CSTD_ASSERT(rb.front().value == 2, "[ring_buffer] front value after overwrite");

		rb.pop_front();
		CSTD_ASSERT(lifetime_probe::live == 2, "[ring_buffer] live after pop_front");

		rb.pop_back();
		CSTD_ASSERT(lifetime_probe::live == 1, "[ring_buffer] live after pop_back");
	}

	CSTD_ASSERT(lifetime_probe::live == 0, "[ring_buffer] all destroyed after scope");

	lifetime_probe::live = 0;
	lifetime_probe::copies = 0;

	{
		cstd::ring_buffer<lifetime_probe> rb(2);
		rb.emplace_back(10);
		rb.emplace_back(20);

		cstd::ring_buffer<lifetime_probe> copy(rb);
		CSTD_ASSERT(lifetime_probe::live == 4, "[ring_buffer] live after copy");
		CSTD_ASSERT(lifetime_probe::copies == 2, "[ring_buffer] copy count");

		copy.clear();
		CSTD_ASSERT(lifetime_probe::live == 2, "[ring_buffer] live after clear copy");
	}

	CSTD_ASSERT(lifetime_probe::live == 0, "[ring_buffer] all destroyed after copy test");

	lifetime_probe::live = 0;
	lifetime_probe::moves = 0;

	{
		cstd::ring_buffer<lifetime_probe> rb(2);
		rb.emplace_back(1);

		cstd::ring_buffer<lifetime_probe> moved(cstd::move(rb));
		CSTD_ASSERT(lifetime_probe::live == 1, "[ring_buffer] live after move");
		CSTD_ASSERT(rb.empty(), "[ring_buffer] source empty after move");
	}

	CSTD_ASSERT(lifetime_probe::live == 0, "[ring_buffer] all destroyed after move test");
}

static void test_ring_buffer_capacity_one()
{
	cstd::ring_buffer<cstd::int32_t> rb(1);
	rb.push_back(10);
	CSTD_ASSERT(rb.front() == 10 && rb.back() == 10, "[ring_buffer] cap-1 push_back");

	rb.push_back(20);
	CSTD_ASSERT(rb.size() == 1, "[ring_buffer] cap-1 still size 1");
	CSTD_ASSERT(rb.front() == 20, "[ring_buffer] cap-1 overwritten");

	rb.push_front(30);
	CSTD_ASSERT(rb.front() == 30, "[ring_buffer] cap-1 push_front overwrite");

	rb.pop_front();
	CSTD_ASSERT(rb.empty(), "[ring_buffer] cap-1 empty after pop");
}

static void test_ring_buffer_try()
{
	cstd::ring_buffer<cstd::int32_t> rb(3);

	CSTD_ASSERT(rb.try_push_back(10), "[ring_buffer] try_push_back into empty");
	CSTD_ASSERT(rb.try_push_back(20), "[ring_buffer] try_push_back 2nd");
	CSTD_ASSERT(rb.try_push_back(30), "[ring_buffer] try_push_back 3rd");
	CSTD_ASSERT(!rb.try_push_back(40), "[ring_buffer] try_push_back full");
	CSTD_ASSERT(rb.size() == 3, "[ring_buffer] try_push_back size unchanged on full");
	CSTD_ASSERT(rb.front() == 10, "[ring_buffer] try_push_back preserved front");

	CSTD_ASSERT(!rb.try_push_front(5), "[ring_buffer] try_push_front full");

	cstd::int32_t out = 0;
	CSTD_ASSERT(rb.try_pop_front(out), "[ring_buffer] try_pop_front non-empty");
	CSTD_ASSERT(out == 10, "[ring_buffer] try_pop_front got front");
	CSTD_ASSERT(rb.size() == 2, "[ring_buffer] try_pop_front decremented size");

	CSTD_ASSERT(rb.try_pop_back(out), "[ring_buffer] try_pop_back non-empty");
	CSTD_ASSERT(out == 30, "[ring_buffer] try_pop_back got back");
	CSTD_ASSERT(rb.size() == 1, "[ring_buffer] try_pop_back decremented size");
	CSTD_ASSERT(rb.front() == 20, "[ring_buffer] middle element remains");

	rb.pop_front();
	CSTD_ASSERT(!rb.try_pop_front(out), "[ring_buffer] try_pop_front empty");
	CSTD_ASSERT(!rb.try_pop_back(out), "[ring_buffer] try_pop_back empty");

	cstd::ring_buffer<cstd::int32_t> zero;
	CSTD_ASSERT(!zero.try_push_back(1), "[ring_buffer] try_push_back zero-cap");
	CSTD_ASSERT(!zero.try_pop_front(out), "[ring_buffer] try_pop_front zero-cap");

	CSTD_ASSERT(rb.try_push_front(99), "[ring_buffer] try_push_front after drain");
	CSTD_ASSERT(rb.front() == 99, "[ring_buffer] try_push_front value");
}

static void test_static_ring_buffer()
{
	cstd::static_ring_buffer<cstd::int32_t, 4> rb = {};
	CSTD_ASSERT(rb.empty(), "[static_ring_buffer] starts empty");
	CSTD_ASSERT(rb.capacity() == 4, "[static_ring_buffer] capacity is N");

	rb.push_back(10);
	rb.push_back(20);
	rb.push_back(30);
	CSTD_ASSERT(rb.size() == 3, "[static_ring_buffer] size after pushes");
	CSTD_ASSERT(rb.front() == 10, "[static_ring_buffer] front");
	CSTD_ASSERT(rb.back() == 30, "[static_ring_buffer] back");
	CSTD_ASSERT(rb[1] == 20, "[static_ring_buffer] operator[]");

	rb.push_back(40);
	CSTD_ASSERT(rb.full(), "[static_ring_buffer] full at capacity");

	rb.push_back(50);
	CSTD_ASSERT(rb.size() == 4, "[static_ring_buffer] size stays at cap after overwrite");
	CSTD_ASSERT(rb.front() == 20, "[static_ring_buffer] front advanced after overwrite");
	CSTD_ASSERT(rb.back() == 50, "[static_ring_buffer] back is new element");

	rb.pop_front();
	CSTD_ASSERT(rb.front() == 30, "[static_ring_buffer] pop_front");
	rb.pop_back();
	CSTD_ASSERT(rb.back() == 40, "[static_ring_buffer] pop_back");
	CSTD_ASSERT(rb.size() == 2, "[static_ring_buffer] size after pops");

	rb.clear();
	CSTD_ASSERT(rb.empty(), "[static_ring_buffer] empty after clear");
}

static void test_static_ring_buffer_try()
{
	cstd::static_ring_buffer<cstd::int32_t, 3> rb = {};

	CSTD_ASSERT(rb.try_push_back(10), "[static_ring_buffer] try_push_back 1");
	CSTD_ASSERT(rb.try_push_back(20), "[static_ring_buffer] try_push_back 2");
	CSTD_ASSERT(rb.try_push_back(30), "[static_ring_buffer] try_push_back 3");
	CSTD_ASSERT(!rb.try_push_back(40), "[static_ring_buffer] try_push_back full");
	CSTD_ASSERT(!rb.try_push_front(5), "[static_ring_buffer] try_push_front full");
	CSTD_ASSERT(rb.front() == 10, "[static_ring_buffer] elements intact after failed push");

	cstd::int32_t out = 0;
	CSTD_ASSERT(rb.try_pop_front(out), "[static_ring_buffer] try_pop_front");
	CSTD_ASSERT(out == 10, "[static_ring_buffer] try_pop_front got 10");

	CSTD_ASSERT(rb.try_pop_back(out), "[static_ring_buffer] try_pop_back");
	CSTD_ASSERT(out == 30, "[static_ring_buffer] try_pop_back got 30");

	CSTD_ASSERT(rb.try_pop_front(out), "[static_ring_buffer] drain last");
	CSTD_ASSERT(out == 20, "[static_ring_buffer] got 20");

	CSTD_ASSERT(!rb.try_pop_front(out), "[static_ring_buffer] try_pop_front empty");
	CSTD_ASSERT(!rb.try_pop_back(out), "[static_ring_buffer] try_pop_back empty");

	CSTD_ASSERT(rb.try_push_front(99), "[static_ring_buffer] try_push_front after drain");
	CSTD_ASSERT(rb.front() == 99, "[static_ring_buffer] try_push_front value");
}

static void test_static_ring_buffer_iteration()
{
	cstd::static_ring_buffer<cstd::int32_t, 4> rb = {};

	for (cstd::int32_t i = 0; i < 6; ++i)
	{
		rb.push_back(i * 10);
	}

	CSTD_ASSERT(rb.size() == 4, "[static_ring_buffer] iter size");
	CSTD_ASSERT(rb.front() == 20, "[static_ring_buffer] iter front after wrap");

	cstd::int32_t expected = 20;
	for (auto it = rb.begin(); it != rb.end(); ++it)
	{
		CSTD_ASSERT(*it == expected, "[static_ring_buffer] forward iteration");
		expected += 10;
	}

	auto it = rb.end();
	expected = 50;
	while (it != rb.begin())
	{
		--it;
		CSTD_ASSERT(*it == expected, "[static_ring_buffer] reverse iteration");
		expected -= 10;
	}

	const auto& crb = rb;
	cstd::int32_t sum = 0;
	for (auto ci = crb.begin(); ci != crb.end(); ++ci)
	{
		sum += *ci;
	}
	CSTD_ASSERT(sum == 20 + 30 + 40 + 50, "[static_ring_buffer] const iteration sum");
}

static void test_static_ring_buffer_copy_move()
{
	cstd::static_ring_buffer<cstd::int32_t, 4> a = {};
	a.push_back(1);
	a.push_back(2);
	a.push_back(3);

	cstd::static_ring_buffer<cstd::int32_t, 4> b(a);
	CSTD_ASSERT(b.size() == 3, "[static_ring_buffer] copy ctor size");
	CSTD_ASSERT(b[0] == 1 && b[1] == 2 && b[2] == 3, "[static_ring_buffer] copy ctor values");

	cstd::static_ring_buffer<cstd::int32_t, 4> c = {};
	c = a;
	CSTD_ASSERT(c.size() == 3 && c[0] == 1, "[static_ring_buffer] copy assign");

	cstd::static_ring_buffer<cstd::int32_t, 4> d(cstd::move(a));
	CSTD_ASSERT(d.size() == 3 && d[0] == 1, "[static_ring_buffer] move ctor values");
	CSTD_ASSERT(a.empty(), "[static_ring_buffer] source empty after move");

	cstd::static_ring_buffer<cstd::int32_t, 4> e = {};
	e.push_back(99);
	e = cstd::move(d);
	CSTD_ASSERT(e.size() == 3 && e[2] == 3, "[static_ring_buffer] move assign");
	CSTD_ASSERT(d.empty(), "[static_ring_buffer] source empty after move assign");

	b.swap(c);
	CSTD_ASSERT(b[0] == 1 && c[0] == 1, "[static_ring_buffer] swap");

	cstd::static_ring_buffer<cstd::int32_t, 4> f = {};
	f.push_back(100);
	cstd::swap(e, f);
	CSTD_ASSERT(f.size() == 3 && e.size() == 1, "[static_ring_buffer] free swap");
}

static void test_static_ring_buffer_comparisons()
{
	cstd::static_ring_buffer<cstd::int32_t, 4> a = {};
	cstd::static_ring_buffer<cstd::int32_t, 4> b = {};

	a.push_back(1);
	a.push_back(2);
	b.push_back(1);
	b.push_back(2);
	CSTD_ASSERT(a == b, "[static_ring_buffer] equal");
	CSTD_ASSERT(!(a != b), "[static_ring_buffer] not not-equal");
	CSTD_ASSERT(a <= b, "[static_ring_buffer] le when equal");
	CSTD_ASSERT(a >= b, "[static_ring_buffer] ge when equal");

	b.push_back(3);
	CSTD_ASSERT(a < b, "[static_ring_buffer] shorter < longer");
	CSTD_ASSERT(b > a, "[static_ring_buffer] longer > shorter");
	CSTD_ASSERT(a != b, "[static_ring_buffer] different sizes not equal");

	cstd::static_ring_buffer<cstd::int32_t, 4> c = {};
	cstd::static_ring_buffer<cstd::int32_t, 4> d = {};
	c.push_back(1);
	c.push_back(3);
	d.push_back(1);
	d.push_back(2);
	CSTD_ASSERT(d < c, "[static_ring_buffer] element-wise less");
}

static void test_static_ring_buffer_lifetime()
{
	lifetime_probe::live = 0;
	lifetime_probe::copies = 0;
	lifetime_probe::moves = 0;

	{
		cstd::static_ring_buffer<lifetime_probe, 4> rb = {};
		rb.push_back(lifetime_probe(1));
		rb.push_back(lifetime_probe(2));
		rb.push_back(lifetime_probe(3));
		CSTD_ASSERT(lifetime_probe::live == 3, "[static_ring_buffer] 3 live after push");

		rb.push_back(lifetime_probe(4));
		rb.push_back(lifetime_probe(5));
		CSTD_ASSERT(lifetime_probe::live == 4, "[static_ring_buffer] still 4 live after overwrite");
		CSTD_ASSERT(rb.front().value == 2, "[static_ring_buffer] front after overwrite");
	}

	CSTD_ASSERT(lifetime_probe::live == 0, "[static_ring_buffer] all destroyed after scope");

	{
		cstd::static_ring_buffer<lifetime_probe, 4> rb = {};
		rb.push_back(lifetime_probe(10));
		rb.push_back(lifetime_probe(20));

		cstd::static_ring_buffer<lifetime_probe, 4> copy(rb);
		CSTD_ASSERT(copy.size() == 2, "[static_ring_buffer] copy size");
		CSTD_ASSERT(copy[0].value == 10, "[static_ring_buffer] copy value");

		cstd::static_ring_buffer<lifetime_probe, 4> moved(cstd::move(rb));
		CSTD_ASSERT(moved.size() == 2, "[static_ring_buffer] move size");
		CSTD_ASSERT(rb.empty(), "[static_ring_buffer] source empty after move");
	}

	CSTD_ASSERT(lifetime_probe::live == 0, "[static_ring_buffer] all destroyed after copy/move scope");
}

static void test_bitset()
{
	cstd::bitset<130> bits;

	CSTD_ASSERT(bits.size() == 130 && bits.count() == 0, "[test] default bitset must be empty");
	CSTD_ASSERT(bits.none() && !bits.any() && !bits.all(), "[test] empty bitset none/any/all");

	bits.set(0).set(65).set(129);

	CSTD_ASSERT(bits.count() == 3 && bits.any() && !bits.none(), "[test] bitset set() must count");
	CSTD_ASSERT(bits.test(65) && bits[129] && !bits.test(64), "[test] bitset test() per position");

	bits.reset(65);
	CSTD_ASSERT(!bits.test(65) && bits.count() == 2, "[test] bitset reset() clears one bit");

	bits.flip(65);
	CSTD_ASSERT(bits.test(65) && bits.count() == 3, "[test] bitset flip() toggles one bit");

	bits.set();
	CSTD_ASSERT(bits.all() && bits.count() == 130, "[test] bitset set() must set every bit");

	bits.flip();
	CSTD_ASSERT(bits.none() && bits.count() == 0, "[test] bitset flip() must clear a full bitset");

	// Iteration visits only the set bits, in ascending order.
	bits.reset().set(3).set(70).set(128);

	cstd::size_t visited[4] = { 0, 0, 0, 0 };
	cstd::size_t seen = 0;

	for (const cstd::size_t index : bits)
	{
		CSTD_ASSERT(seen < 4, "[test] bitset iteration must visit only the set bits");
		visited[seen++] = index;
	}

	CSTD_ASSERT(seen == 3 && visited[0] == 3 && visited[1] == 70 && visited[2] == 128,
		"[test] bitset iteration must yield ascending set-bit indices");

	cstd::bitset<8> empty;
	for (const cstd::size_t index : empty)
	{
		(void)index;
		CSTD_ASSERT(false, "[test] iteration over an empty bitset must not visit anything");
	}

	cstd::bitset<16> runs;
	runs.set(4).set(5).set(6).set(12);

	CSTD_ASSERT(runs.find_first_run(0) == 0, "[test] a zero-length run starts where asked");
	CSTD_ASSERT(runs.find_first_run(1) == 4, "[test] find_first_run finds the first set bit");
	CSTD_ASSERT(runs.find_first_run(3) == 4, "[test] find_first_run finds an exact-length run");
	CSTD_ASSERT(runs.find_first_run(4) == 16, "[test] find_first_run reports N when no run fits");
	CSTD_ASSERT(runs.find_first_run(1, 7) == 12, "[test] find_first_run honours the start position");

	cstd::bitset<64> left;
	cstd::bitset<64> right;

	left.set(1).set(2);
	right.set(2).set(3);

	CSTD_ASSERT((left & right).count() == 1 && (left & right).test(2), "[test] bitset operator&");
	CSTD_ASSERT((left | right).count() == 3, "[test] bitset operator|");
	CSTD_ASSERT((left ^ right).count() == 2 && !(left ^ right).test(2), "[test] bitset operator^");
	CSTD_ASSERT((~left).count() == 62 && !(~left).test(1), "[test] bitset operator~");
	CSTD_ASSERT(left != right && left == left, "[test] bitset comparisons");

	left &= right;
	CSTD_ASSERT(left.count() == 1 && left.test(2), "[test] bitset operator&=");

	// The unused high bits of the final word must stay clear, or count() would over-report.
	cstd::bitset<70> partial;
	partial.set();
	CSTD_ASSERT(partial.count() == 70, "[test] bitset must not count bits past N");
}

static void test_bitset_constexpr()
{
	// The whole type has to work in constant evaluation, which is what rules out the
	// non-constexpr popcount/bit-scan intrinsics MSVC offers.
	constexpr cstd::size_t counted = []
	{
		cstd::bitset<200> bits;
		bits.set(0).set(63).set(64).set(199);
		return bits.count();
	}();

	constexpr cstd::size_t first = []
	{
		cstd::bitset<200> bits;
		bits.set(137);

		cstd::size_t found = 200;
		for (const cstd::size_t index : bits)
		{
			found = index;
			break;
		}
		return found;
	}();

	static_assert(counted == 4, "[test] bitset::count() must be usable in constant evaluation");
	static_assert(first == 137, "[test] bitset iteration must be usable in constant evaluation");
}

static void test_from_chars()
{
	const char decimal[] = "1234rest";
	cstd::uint32_t value = 0;
	cstd::from_chars_result result = cstd::from_chars(decimal, decimal + 8, value);

	CSTD_ASSERT(result.ec == 0 && value == 1234, "[test] from_chars parses decimal digits");
	CSTD_ASSERT(result.ptr == decimal + 4, "[test] from_chars stops at the first non-digit");

	const char negative[] = "-42";
	cstd::int32_t signed_value = 0;
	result = cstd::from_chars(negative, negative + 3, signed_value);

	CSTD_ASSERT(result.ec == 0 && signed_value == -42, "[test] from_chars parses a negative value");
	CSTD_ASSERT(result.ptr == negative + 3, "[test] from_chars consumes the whole negative value");

	const char hex[] = "ffFF";
	cstd::uint32_t hex_value = 0;
	result = cstd::from_chars(hex, hex + 4, hex_value, 16);

	CSTD_ASSERT(result.ec == 0 && hex_value == 0xffff, "[test] from_chars honours the base");

	const char binary[] = "1011";
	cstd::uint32_t binary_value = 0;
	result = cstd::from_chars(binary, binary + 4, binary_value, 2);

	CSTD_ASSERT(result.ec == 0 && binary_value == 11, "[test] from_chars parses base 2");

	// A digit the base does not allow is not a digit at all.
	const char limited[] = "129";
	cstd::uint32_t limited_value = 0;
	result = cstd::from_chars(limited, limited + 3, limited_value, 8);

	CSTD_ASSERT(result.ec == 0 && limited_value == 10 && result.ptr == limited + 2,
		"[test] from_chars stops at a digit the base does not allow");

	const char junk[] = "zzz";
	cstd::uint32_t junk_value = 7;
	result = cstd::from_chars(junk, junk + 3, junk_value);

	CSTD_ASSERT(result.ec != 0 && result.ptr == junk && junk_value == 7,
		"[test] from_chars reports failure without touching the value");

	const char empty[] = "";
	result = cstd::from_chars(empty, empty, junk_value);
	CSTD_ASSERT(result.ec != 0 && result.ptr == empty, "[test] from_chars on an empty range fails");

	const char lone_sign[] = "-";
	result = cstd::from_chars(lone_sign, lone_sign + 1, signed_value);
	CSTD_ASSERT(result.ec != 0 && result.ptr == lone_sign, "[test] from_chars rejects a lone sign");

	// Overflow is reported rather than wrapping.
	const char too_big[] = "300";
	cstd::uint8_t narrow = 0;
	result = cstd::from_chars(too_big, too_big + 3, narrow);

	CSTD_ASSERT(result.ec != 0, "[test] from_chars reports unsigned overflow");

	const char too_positive[] = "200";
	cstd::int8_t narrow_signed = 0;
	result = cstd::from_chars(too_positive, too_positive + 3, narrow_signed);

	CSTD_ASSERT(result.ec != 0, "[test] from_chars reports signed overflow");

	const char most_negative[] = "-128";
	result = cstd::from_chars(most_negative, most_negative + 4, narrow_signed);

	CSTD_ASSERT(result.ec == 0 && narrow_signed == -128,
		"[test] from_chars accepts the most negative value of a signed type");

	constexpr cstd::uint32_t parsed = []
	{
		const char text[] = "4096";
		cstd::uint32_t out = 0;
		(void)cstd::from_chars(text, text + 4, out);
		return out;
	}();

	static_assert(parsed == 4096, "[test] from_chars must work in constant evaluation");
}

static void test_views()
{
	cstd::int32_t total = 0;
	for (const cstd::int32_t value : cstd::views::iota(1, 5))
	{
		total += value;
	}

	CSTD_ASSERT(total == 10, "[test] bounded iota yields [start, bound)");

	cstd::int32_t taken = 0;
	cstd::int32_t count = 0;
	for (const cstd::int32_t value : cstd::views::iota(10))
	{
		taken += value;
		if (++count == 3)
		{
			break;
		}
	}

	CSTD_ASSERT(taken == 33 && count == 3, "[test] unbounded iota keeps incrementing");

	cstd::vector<cstd::int32_t> numbers;
	for (cstd::int32_t i = 0; i < 10; ++i)
	{
		numbers.push_back(i);
	}

	cstd::int32_t doubled = 0;
	for (const cstd::int32_t value : numbers | cstd::views::transform([](const cstd::int32_t x) { return x * 2; }))
	{
		doubled += value;
	}

	CSTD_ASSERT(doubled == 90, "[test] transform view maps every element");

	cstd::int32_t evens = 0;
	for (const cstd::int32_t value : numbers | cstd::views::filter([](const cstd::int32_t x) { return x % 2 == 0; }))
	{
		evens += value;
	}

	CSTD_ASSERT(evens == 20, "[test] filter view keeps only matching elements");

	cstd::int32_t prefix = 0;
	for (const cstd::int32_t value : numbers | cstd::views::take_while([](const cstd::int32_t x) { return x < 4; }))
	{
		prefix += value;
	}

	CSTD_ASSERT(prefix == 6, "[test] take_while view stops at the first failure");

	cstd::int32_t composed = 0;
	for (const cstd::int32_t value : numbers
		| cstd::views::filter([](const cstd::int32_t x) { return x % 2 == 0; })
		| cstd::views::transform([](const cstd::int32_t x) { return x * 3; }))
	{
		composed += value;
	}

	CSTD_ASSERT(composed == 60, "[test] piped views compose");

	// A closure composed before it ever sees a range behaves the same way.
	const auto pipeline = cstd::views::transform([](const cstd::int32_t x) { return x + 1; })
		| cstd::views::filter([](const cstd::int32_t x) { return x % 3 == 0; });

	cstd::int32_t through_closure = 0;
	for (const cstd::int32_t value : numbers | pipeline)
	{
		through_closure += value;
	}

	CSTD_ASSERT(through_closure == 18, "[test] closures compose before being piped a range");

	cstd::vector<cstd::vector<cstd::int32_t>> nested;

	cstd::vector<cstd::int32_t> first;
	first.push_back(1);
	first.push_back(2);
	nested.push_back(first);

	nested.push_back(cstd::vector<cstd::int32_t>());		// an empty inner range must be skipped

	cstd::vector<cstd::int32_t> third;
	third.push_back(3);
	nested.push_back(third);

	cstd::int32_t flattened = 0;
	cstd::int32_t flattened_count = 0;
	for (const cstd::int32_t value : nested | cstd::views::join)
	{
		flattened += value;
		++flattened_count;
	}

	CSTD_ASSERT(flattened == 6 && flattened_count == 3, "[test] join view flattens and skips empties");

	// Views are lazy: nothing runs until iteration.
	cstd::int32_t calls = 0;
	const auto lazy = numbers | cstd::views::transform([&calls](const cstd::int32_t x) { ++calls; return x; });

	CSTD_ASSERT(calls == 0, "[test] building a view must not invoke the function");

	for (const cstd::int32_t value : lazy)
	{
		(void)value;
	}

	CSTD_ASSERT(calls == 10, "[test] iterating a view invokes the function once per element");
}

static void test_shared_mutex()
{
	cstd::shared_mutex device;

	// Readers share; a writer has to wait for all of them.
	device.lock_shared();
	CSTD_ASSERT(device.try_lock_shared(), "[test] a second reader may take a shared lock");
	CSTD_ASSERT(!device.try_lock(), "[test] a writer must not get in while readers hold the lock");

	device.unlock_shared();
	device.unlock_shared();

	CSTD_ASSERT(device.try_lock(), "[test] a writer takes the lock once the readers leave");
	CSTD_ASSERT(!device.try_lock_shared(), "[test] a reader must not get in while a writer holds the lock");
	device.unlock();

	device.lock();
	device.unlock();

	{
		cstd::shared_lock<cstd::shared_mutex> reader(device);

		CSTD_ASSERT(reader.owns_lock() && static_cast<bool>(reader), "[test] shared_lock locks on construction");
		CSTD_ASSERT(reader.mutex() == &device, "[test] shared_lock remembers its mutex");
		CSTD_ASSERT(!device.try_lock(), "[test] shared_lock must hold off writers");
	}

	CSTD_ASSERT(device.try_lock(), "[test] shared_lock must release on destruction");
	device.unlock();

	{
		cstd::shared_lock<cstd::shared_mutex> deferred(device, cstd::defer_lock_t{});

		CSTD_ASSERT(!deferred.owns_lock(), "[test] defer_lock must not acquire");
		CSTD_ASSERT(deferred.try_lock() && deferred.owns_lock(), "[test] shared_lock try_lock acquires");

		deferred.unlock();
		CSTD_ASSERT(!deferred.owns_lock(), "[test] shared_lock unlock releases");

		deferred.lock();
		CSTD_ASSERT(deferred.owns_lock(), "[test] shared_lock lock re-acquires");

		cstd::shared_lock<cstd::shared_mutex> moved(cstd::move(deferred));

		CSTD_ASSERT(moved.owns_lock() && !deferred.owns_lock(), "[test] shared_lock move transfers ownership");
		CSTD_ASSERT(deferred.mutex() == nullptr, "[test] a moved-from shared_lock holds no mutex");

		cstd::shared_mutex* const released = moved.release();

		CSTD_ASSERT(released == &device && !moved.owns_lock(), "[test] shared_lock release hands back the mutex");
		released->unlock_shared();
	}

	{
		cstd::shared_lock<cstd::shared_mutex> empty;
		CSTD_ASSERT(!empty.owns_lock() && empty.mutex() == nullptr, "[test] default shared_lock owns nothing");

		cstd::shared_lock<cstd::shared_mutex> reader(device);
		empty.swap(reader);

		CSTD_ASSERT(empty.owns_lock() && !reader.owns_lock(), "[test] shared_lock swap exchanges ownership");
	}

	// lock_guard drives the exclusive side of a shared_mutex.
	{
		cstd::lock_guard<cstd::shared_mutex> writer(device);
		CSTD_ASSERT(!device.try_lock_shared(), "[test] lock_guard takes a shared_mutex exclusively");
	}

	CSTD_ASSERT(device.try_lock(), "[test] lock_guard releases a shared_mutex");
	device.unlock();
}

static void test_static_ring_buffer_zero_init()
{
	// Value-initialization (= {}) zero-fills all members, producing a valid empty buffer.
	// At file scope this is what the linker does for .bss globals.
	cstd::static_ring_buffer<cstd::int32_t, 8> buf = {};

	CSTD_ASSERT(buf.empty(), "[static_ring_buffer] zero-init is empty");
	CSTD_ASSERT(buf.size() == 0, "[static_ring_buffer] zero-init size 0");
	CSTD_ASSERT(buf.capacity() == 8, "[static_ring_buffer] capacity");

	buf.push_back(42);
	CSTD_ASSERT(buf.front() == 42, "[static_ring_buffer] usable after zero-init");
}

cstd::int32_t main()
{
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

	cstd::atomic<cstd::uint16_t> tiny(0);

	if (tiny.fetch_add(300) != 0)
	{
		return 9;
	}

	if (tiny.load() != 300)
	{
		return 10;
	}

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

	if (*owner != 42)
	{
		return 20;
	}

	test_bug_fixes();
	test_shared_ptr_conversion();
	test_pointer_casts_shared();
	test_pointer_casts_unique();
	test_foundational_utilities();
	test_numeric_limits();
	test_type_traits();
	test_vector_emplace_erase();
	test_vector_assign_insert();
	test_vector_capacity_swap();
	test_leaf_types();
	test_string();
	test_string_conversion();
	test_string_view_search();
	test_string_search();
	test_optional();
	test_expected();
	test_algorithm_nonmodifying();
	test_ranges();
	test_ranges_extra();
	test_algorithm_modifying();
	test_algorithm_sorted();
	test_algorithm_extra();
	test_algorithm_iterators();
	test_algorithm_projected();
	test_flat_map();
	test_mutex();
	test_unique_lock();
	test_function();
	test_function_lifetime();
	test_single_linked_list();
	test_single_linked_list_iterator();
	test_single_linked_list_lifetime();
	test_double_linked_list();
	test_double_linked_list_iterator();
	test_double_linked_list_lifetime();
	test_pointer_comparisons();
	test_optional_comparisons();
	test_function_comparisons();
	test_unordered_map();
	test_unordered_map_iteration();
	test_unordered_map_modifiers();
	test_unordered_map_rehash();
	test_ring_buffer();
	test_ring_buffer_push_front();
	test_ring_buffer_iteration();
	test_ring_buffer_copy_move();
	test_ring_buffer_emplace();
	test_ring_buffer_comparisons();
	test_ring_buffer_lifetime();
	test_ring_buffer_capacity_one();
	test_ring_buffer_try();
	test_static_ring_buffer();
	test_static_ring_buffer_try();
	test_static_ring_buffer_iteration();
	test_static_ring_buffer_copy_move();
	test_static_ring_buffer_comparisons();
	test_static_ring_buffer_lifetime();
	test_static_ring_buffer_zero_init();
	test_bitset();
	test_bitset_constexpr();
	test_from_chars();
	test_views();
	test_shared_mutex();

	*owner = 7;

	return *owner - 7;
}
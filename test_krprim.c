#include "krprim.h"
#include "test.h"
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stddef.h>

TEST_CASE(this_test_always_fails)
{
	//TEST(false);
}

//----------------------------------------------------------------------
// Macros

TEST_CASE(macro_tests)
{
#define TEST_CONCAT_LONG_TOKEN  true
	TEST( CONCAT(TEST_CONCAT_, LONG_TOKEN) );

	const char str[] = {"This is a String"};
	TEST( !strcmp(STRINGIFY(This is a String), str) );

#define EXPAND_THIS_TOKEN   XYZZY

	const char token_not_expanded[] = {"EXPAND_THIS_TOKEN"};
	TEST( !strcmp(STRINGIFY(EXPAND_THIS_TOKEN), token_not_expanded) );

	const char token_expanded[] = {"XYZZY"};
	TEST( !strcmp(STRINGIFY_EXPAND(EXPAND_THIS_TOKEN), token_expanded) );

	int a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	TEST( lengthof(a) == 10 );

	TEST( lengthof("xyzzy") == 6 );

	int nargs = 5;
	TEST( VA_NARGS(1,2,'a',5.5,"hi") == nargs );
}

//----------------------------------------------------------------------
// Debugging & Assertions

TEST_CASE(capture_debug_context_info)
{
	SourceLine dbi = SRC_HERE;
	TEST(!strcmp(dbi.file_name, "test_krprim.c"));
	TEST(dbi.line_num == (__LINE__-2));
}

TEST_CASE(test_assertions)
{
	char errmsg[101] = "";

	int x = 1;
	assert(x == 1);

	// This assert fails
	//assert(x == 2);
}

TEST_CASE(check_index_out_of_bounds)
{
	TEST( check(  0,  22) ==   0 );
	TEST( check(  9, 109) ==   9 );
	TEST( check( 32,  64) ==  32 );
	TEST( check( -1, 128) == 127 );
	TEST( check(-32,  32) ==   0 );

	// These will crash
	int length = 32;
//	check( 32, length);
//	check(-33, length);
//	check(0, 0);
}

//----------------------------------------------------------------------
// String

TEST_CASE(String_default_state)
{
	String s = {0};

	TEST( s.length == 0 );
	TEST( Str_eq(s, Str("")) );
	TEST( Str_empty(s) );

	TEST( Str_empty(Str("")) );
}

TEST_CASE(String_does_not_include_null)
{
	String c = Str("This is a literal String.");
	TEST( c.length == 25 );

	String d = Str( (Utf8[]){"This is a char array."} );
	TEST( d.length == 21 );

	TEST( Str("How long is this?").length == 17 );
}

TEST_CASE(String_slice)
{
	//             0123456789
	String s = Str("Hello, World!");
	//                -987654321
	TEST( s.length == 13 );

	String t = Str_slice(s, 3, 7);
	TEST( t.length == 5 );
	TEST( String_equals(t, Str("lo, W")) );

	// Assert fails!
//	Str_slice(s, 0, 14);
//	Str_slice(s, -20, 12);

	String u = Str_slice(s, 8, 2);
	TEST( u.length == 7 );
	TEST( String_equals(u, Str("llo, Wo")) );

	TEST( String_equals( Str_slice(s, 9, -8), Str(", Wor")) );
	TEST( String_equals( Str_slice(s, -2, -8), Str(", World")) );

	String v = Str_slice(s, -3, s.length-1);
	TEST( v.length == 3 );
	TEST( String_equals(v, Str("ld!")) );
	TEST( String_equals( Str_slice(s,5,-5), Str(", Wo") ) );

	String w = Str_first(s, 5);
	TEST( w.length == 5 );
	TEST( String_equals(w, Str("Hello")) );
	TEST( Str_first(s,0).length == 0 );
	TEST( String_equals( Str_first(s,13), Str("Hello, World!") ) );

	String x = Str_last(s, 6);
	TEST( x.length == 6); 
	TEST( String_equals(x, Str("World!")) );
	TEST( Str_last(s,0).length == 0);
	TEST( String_equals( Str_last(s,13), Str("Hello, World!") ) );
}

TEST_CASE(String_equals)
{
	char hello_array[] = "hello";
	char world_array[] = "world";

	String hello = Str(hello_array);
	String world = Str(world_array);

	String a = Str("hello");
	String b = Str("world");

	TEST( String_equals(a, hello) );
	TEST( String_equals(a, a) );
	TEST( String_equals(b, world) );
	TEST(!String_equals(a, world) );
	TEST(!String_equals(b, hello) );
}

TEST_CASE(String_equals_empty_null_)
{
	String a_null = {0};
	String b_null = {0};
	TEST( String_equals(a_null,b_null) );

	String c_empty = Str("");
	String d_empty = Str("");
	TEST( String_equals(c_empty,d_empty) );
}

//----------------------------------------------------------------------
// Error Handling 

TEST_CASE(Status_to_string_test)
{
	TEST( Str_eq(Status_string(STAT_OK),    Str("STAT_OK")) );
	TEST( Str_eq(Status_string(STAT_ERROR), Str("STAT_ERROR")) );
}

static bool fail_throw(Exception *ex, SourceLine loc)
{
	throw(ex, STAT_ERROR, Str("throw up"), loc);
	return false;
}

TEST_CASE(assertion_throws_exception)
{
	Exception ex = {0};

	switch (EX_BEGIN(ex))
	{
		case EX_TRY:
			TEST( fail_throw(&ex, SRC_HERE) );
			TEST(!"Exception should have thrown!");
			break;

		case STAT_ERROR:
			// We should get here
			TEST( ex.status == STAT_ERROR );
			TEST( Str_eq(ex.message, Str("throw up")) );
			TEST( Str_eq(String_init(ex.location.file_name), Str("test_krprim.c")) );
			TEST( ex.location.line_num == __LINE__-9 );
			break;

		default:
			TEST(!"Wrong exception code");
	}

	// This crashes with "Unhandled Exception"
	//throw(NULL, STAT_ERROR, Str("oops"), SRC_HERE);
}

//----------------------------------------------------------------------
// Numbers

TEST_CASE(int_functions)
{
	TEST(int_min(0, 10) ==  0);
	TEST(int_max(0, 10) == 10);

	TEST( in_range( 55,  1, 99));
	TEST(!in_range(  0,  1, 99));
	TEST(!in_range(100,  1, 99));
}

TEST_CASE(num_equals_with_epsilon)
{
	double a = 9.2;
	double b = 0.000001;

	TEST(!num_eq(a, b, 0.0));
	TEST(!num_eq(a, 9.3-0.1, 0.0));
	TEST( num_eq(a, 9.3-0.1, 0.00001));
}

//----------------------------------------------------------------------
// Pointer Stuff

TEST_CASE(ptr_and_returns_pointer_or_alternate)
{
	char ok_p[] = "hello";
	char alt_p[] = "world";
	TEST(ptr_and(ok_p, alt_p) == ok_p);
	TEST(ptr_and(NULL, alt_p) == alt_p);
	TEST(ptr_and(NULL, NULL)  == NULL);
}

//----------------------------------------------------------------------
// Intervals & Vectors

TEST_CASE(closed_intervals)
{
	Interval itvl = { 1.0, 2.0 };
	TEST( includes(itvl, 1.0) );
	TEST( includes(itvl, 2.0) );
	TEST( includes(itvl, 1.2) );
	TEST( includes(itvl, 1.9999999) );
	TEST(!includes(itvl, 0.1) );
	TEST(!includes(itvl, 2.1) );
	TEST(!includes(itvl, nextafter(1.0, 0.0)) );
	TEST(!includes(itvl, nextafter(2.0, 3.0)) );
}

TEST_CASE(intervals_left_less_than_right)
{
	Interval invl = interval(5.5, 6.6);
	TEST(invl.left <= invl.right);

	Interval invl2 = interval(99.0, -1.0);
	TEST(invl2.left <= invl2.right);
}

TEST_CASE(right_unbounded_intervals)
{
	Interval r_inf = { 99.0, INFINITY };
	TEST( includes(r_inf, DBL_MAX) );
	TEST(!includes(r_inf, -DBL_MAX) );
}

TEST_CASE(left_unbounded_intervals)
{
	Interval l_inf = { -INFINITY, 999.9 };
	TEST( includes(l_inf, -DBL_MAX) );
	TEST(!includes(l_inf, DBL_MAX) );
}

TEST_CASE(interval_R_positive_numbers)
{
	TEST(  includes(R_positive,  DBL_MIN) );
	TEST(  includes(R_positive,  DBL_MAX) );

	TEST( !includes(R_positive,  0.0) );
	TEST( !includes(R_positive, -DBL_MIN) );
	TEST( !includes(R_positive, -DBL_MAX) );
}

TEST_CASE(interval_R_negative_range)
{
	TEST(  includes(R_negative, -DBL_MIN) );
	TEST(  includes(R_negative, -DBL_MAX) );

	TEST( !includes(R_negative,  0.0) );
	TEST( !includes(R_negative,  DBL_MIN) );
	TEST( !includes(R_negative,  DBL_MAX) );
}

TEST_CASE(interval_R_all_range)
{
	TEST( includes(R_all,  DBL_MIN) );
	TEST( includes(R_all,  DBL_MAX) );
	TEST( includes(R_all, -DBL_MIN) );
	TEST( includes(R_all, -DBL_MAX) );
	TEST( includes(R_all,  0.0) );
}

TEST_CASE(interval_R_zero_range)
{
	TEST(  includes(R_zero, 0.0)  );

	TEST( !includes(R_zero,  DBL_MIN)  );
	TEST( !includes(R_zero,  DBL_MAX)  );
	TEST( !includes(R_zero, -DBL_MIN)  );
	TEST( !includes(R_zero, -DBL_MAX)  );
}

TEST_CASE(interval_R_unit_range)
{
	TEST(  includes(R_unit, 0.0)  );
	TEST(  includes(R_unit, 0.567)  );
	TEST(  includes(R_unit, 1.0)  );
	TEST(  includes(R_unit, DBL_MIN) );

	TEST( !includes(R_unit, nextafter(1.0,  2.0)) );
	TEST( !includes(R_unit, nextafter(0.0, -1.0)) );
	TEST( !includes(R_unit,  DBL_MAX) );
	TEST( !includes(R_unit, -DBL_MAX) );
	TEST( !includes(R_unit, -DBL_MIN) );
}

TEST_CASE(interval_R_empty_range)
{
	TEST( !includes(R_empty,  0.0) );
	TEST( !includes(R_empty,  DBL_MIN) );
	TEST( !includes(R_empty, -DBL_MIN) );
	TEST( !includes(R_empty,  DBL_MAX) );
	TEST( !includes(R_empty, -DBL_MAX) );
	TEST( !includes(R_empty, nextafter(0.0,  1.0)) );
	TEST( !includes(R_empty, nextafter(0.0, -1.0)) );
}

TEST_CASE(interval_clamp)
{
	TEST( clamp(interval(1.0, 2.0),  1.5) == 1.5 );
	TEST( clamp(interval(1.0, 2.0), -0.5) == 1.0 );
	TEST( clamp(interval(1.0, 2.0),  3.0) == 2.0);
	// Todo: extreme ranges of float
}

TEST_CASE(linear_interpolation)
{
	TEST( lerp(interval(1.0, 6.0), 0.5) == 3.5 );
	TEST( lerp(interval(1.0, 6.0), 0.0) == 1.0 );
	TEST( lerp(interval(1.0, 6.0), 1.0) == 6.0 );
	// Todo: extreme ranges of float
}

TEST_CASE(vectors)
{
	VecXY v1 = { 1.5, 100.25 };
	TEST( v1.v[0] == 1.5 );
	TEST( v1.v[1] == 100.25 );

	TEST( VEC_LENGTH(v1) == 2 );
}

//-----------------------------------------------------------------------------
// Span

TEST_CASE(create_span_from_literal)
{
	nuspan n2 = $N(1.1, 1.2, 1.3, 1.4, 1.5, 1.6);
	TEST( n2.length == 6 );
}

TEST_CASE(slice_returns_partial_span)
{
	nuspan a = $N(5.342, 3.234, 7.274, 9.232, 1.623, 2.031, 4.0, 6.101, 8.731);
	TEST( a.length == 9 );

	nuspan s1 = num_slice(a, 3, -2);
	TEST( s1.length == 5 );
	TEST( s1.data[0] == 9.232 );
	TEST( s1.data[4] == 6.101 );

	nuspan s2 = num_slice(a, 4, 7);
	TEST( s2.length == 4 );

	TEST( Str_slice( Str("This String is 29 chars long."), -24, 16).length == 12 );
}


//=============================================================================
// Experimental Stuff 


//-----------------------------------------------------------------------------
// Arenas

TEST_CASE(Arena_allocator_empty)
{
	Arena arena = {0};

	TEST( Arena_cap(arena) == 0 );

}

TEST_CASE(Arena_allocator)
{
	Byte storage[1<<10];
	Arena arena = { storage, storage+lengthof(storage) };

	Ispan is = Vspan_cast(new(&arena, int, 100), Ispan);

	// This will crash
//	new(&arena, int, 256);
}

TEST_CASE(Arena_allocator_throws)
{
	Exception ex = {0};
	Byte storage[1<<10];
	Arena arena = { storage, storage+lengthof(storage), &ex };

	switch (EX_BEGIN(ex)) {
		case EX_TRY:
			alloc(&arena, 8, 4, 1<<10, 0, SRC_HERE); // alloc too much space
			TEST(!"Exception was not thrown.");
			break;
		case STAT_OUT_OF_MEM:
			printf("Here!\n");
			break;
		default:
			TEST(!"Wrong exception type");
	}
}


//-----------------------------------------------------------------------------
// Dynamic Array

// Private

struct DynArrayBase
{
	uint32_t lock;
	int item_size;
	int capacity, end;
	max_align_t data[];
};

struct DynArray
{
	uint32_t key;
	struct DynArrayBase *base;
};

#define DYNARRAY_STRUCT(T_)  struct { struct DynArrayBase base; T_ data[]; }
#define NEW_DYNARRAY(T_)  dyn_create(sizeof(T_))


struct DynArrayBase* dyn_private_inc(struct DynArrayBase *a)
{
	if (a->end >= a->capacity)
	{
		int new_size = a->capacity * 2;
		a = realloc(a, sizeof(*a) + a->item_size * new_size);
		a->capacity = new_size;
	}
	a->end++; 
	return a;
}

struct DynArrayBase* dyn_private_base(const struct DynArray da)
{
	assert( da.key == da.base->lock );
	return da.base;
}

void *dyn_private_item(const struct DynArrayBase *base, int i)
{
	if (i < 0)  i += base->end;
	assert(0 <= i && i < base->end);
	return (Byte*)base->data + base->item_size * i; 
}

// Public

struct DynArray dyn_create(int item_size)
{
	static uint32_t lock = 101u;
	// TODO: handle malloc failure
	struct DynArrayBase *a = malloc(sizeof(*a) + item_size * 8);
	a->lock = lock++;
	a->item_size = item_size;
	a->capacity = 8;
	a->end  = 0;
	return (struct DynArray){ .key = a->lock, .base = a };
}

int dyn_length(const struct DynArray da)
{
	return dyn_private_base(da)->end;
}

void dyn_add(struct DynArray *a, const void* p)
{
	dyn_private_base(*a); // Just check the pointer
	a->base = dyn_private_inc(a->base);
	memcpy(dyn_private_item(a->base, -1), p, a->base->item_size);
}

const void* dyn_get(const struct DynArray da, int i, void* p)
{
	struct DynArrayBase* base = dyn_private_base(da);
	return memcpy(p, dyn_private_item(base, i), base->item_size);
}

void dyn_put(struct DynArray da, int i, const void* item)
{
	struct DynArrayBase* base = dyn_private_base(da);
	memcpy(dyn_private_item(base, i), item, base->item_size);
}

struct TestRec
{
	double number;
	char name[10];
};

TEST_CASE(dynamic_array_tests)
{
	struct DynArray da = NEW_DYNARRAY(struct TestRec);
	TEST( dyn_length(da) == 0 );

	dyn_add(&da, &(struct TestRec){ 101, "Jack" });
	TEST( dyn_length(da) == 1 );
	
	dyn_add(&da, &(struct TestRec){ 202, "Sandy" });
	dyn_add(&da, &(struct TestRec){ 303, "Mike" });
	dyn_add(&da, &(struct TestRec){ 404, "Anne" });
	dyn_add(&da, &(struct TestRec){ 505, "John" });
	dyn_add(&da, &(struct TestRec){ 606, "Beth" });
	dyn_add(&da, &(struct TestRec){ 707, "Charles" });
	dyn_add(&da, &(struct TestRec){ 808, "Dabne" });
	TEST( dyn_length(da) == 8 );

	struct TestRec rec = {0};

	dyn_get(da, 0, &rec);
	TEST( rec.number == 101 );
	TEST( !strcmp(rec.name, "Jack") );

	dyn_get(da, 4, &rec);
	TEST( rec.number == 505 );
	TEST( !strcmp(rec.name, "John") );

	dyn_put(da, 5, &(struct TestRec){ 999, "Smith" });
	TEST( dyn_length(da) == 8 );

	// This add will cause a resize
	dyn_add(&da, &(struct TestRec){ 1010, "Bob" });
	TEST( dyn_length(da) == 9 );

	dyn_get(da, 7, &rec);
	TEST( rec.number == 808 );
	TEST( dyn_length(da) == 9 );

	dyn_get(da, 5, &rec);
	TEST( rec.number == 999 );
	TEST( !strcmp(rec.name, "Smith") );
	TEST( dyn_length(da) == 9 );

//	struct { char c; } too_small = { 'x' };
//	dyn_add(&da, STRUCT_P(&too_small));
//	dyn_put(a, 7, STRUCT_P(too_big));

}



double mult(double a, double b) { return a * b; }

struct FFClosure {
	double (*fn)(double a, double b);
	double a;
};

double f_invoke_f(struct FFClosure context, double b)
{
	return context.fn(context.a, b);
}

TEST_CASE(invoking_closures)
{
	struct FFClosure closure = { mult, 0.5 };

	double n = f_invoke_f(closure, 10.0);
	TEST( n == 5.0 );
}


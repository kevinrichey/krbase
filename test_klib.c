#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>
#include <ctype.h>

#define USING_KR_NAMESPACE
#include "krclib.h"
#include "krstring.h"

TEST_CASE(this_test_always_fails)
{
	//TEST(false);
}

//-----------------------------------------------------------------------------
// Testing Language Assumptions
//

_Static_assert(CHAR_BIT == 8, "chars must be 8 bits");

TEST_CASE(empty_struct_adds_no_size)
{
	struct empty {};
	struct full  { void *a, *b; };

	struct no_head    { int n; char s[]; };
	struct empty_head { struct empty head; int n; char s[]; };
	struct full_head  { struct full  head; int n; char s[]; };

	TEST(sizeof(struct no_head) == sizeof(struct empty_head));
	TEST(sizeof(struct full_head) > sizeof(struct empty_head));
}

TEST_CASE(assume_numeric_limits)
{
	TEST(CHAR_BIT == 8);
}

//-----------------------------------------------------------------------------
// Preprocessor Macros
//

TEST_CASE(UNUSED_prevents_compiler_warnings)
{
	// Unused local variable should cause compiler warning
	int x = 1;

	// UNUSED macro stops the warning
	UNUSED(x);
}

TEST_CASE(stringify_makes_literal_strings)
{
	const char *s = STRINGIFY(hello world);
	const char hello[] =  "hello world";
	TEST(!strcmp(s, hello));
}

TEST_CASE(num_arguments_passed_to_va_macro)
{
#define  GET_NUM_VA_ARGS(...)   VA_NARGS(__VA_ARGS__)

	TEST(GET_NUM_VA_ARGS(a) == 1);
	TEST(GET_NUM_VA_ARGS(a, b, c) == 3);
	TEST(GET_NUM_VA_ARGS(a, b, c, d, e, f, g, h, i) == 9);
	TEST(GET_NUM_VA_ARGS(a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a) == 16);

	// Doesn't work with zero args though
	TEST(GET_NUM_VA_ARGS() != 0);
}

TEST_CASE(get_va_macro_param_n)
{
	TEST(VA_PARAM_0(1, 2, 3, 4, 5, 6, 7, 8, 9) == 1);
	TEST(VA_PARAM_1(1, 2, 3, 4, 5, 6, 7, 8, 9) == 2);
	TEST(VA_PARAM_2(1, 2, 3, 4, 5, 6, 7, 8, 9) == 3);
	TEST(VA_PARAM_3(1, 2, 3, 4, 5, 6, 7, 8, 9) == 4);
	TEST(VA_PARAM_4(1, 2, 3, 4, 5, 6, 7, 8, 9) == 5);
	TEST(VA_PARAM_5(1, 2, 3, 4, 5, 6, 7, 8, 9) == 6);
	TEST(VA_PARAM_6(1, 2, 3, 4, 5, 6, 7, 8, 9) == 7);
	TEST(VA_PARAM_7(1, 2, 3, 4, 5, 6, 7, 8, 9) == 8);
}

TEST_CASE(compute_array_size)
{
	int a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	TEST(ARRAY_SIZE(a) == 10);
}

TEST_CASE(convert_member_ptr_to_struct)
{
	typedef struct {
		int i;
		double d;
		char s[];
	} xyzzy;

	xyzzy *zork = malloc(sizeof(xyzzy) + 10);

	int *ip = &zork->i;
	TEST(MEMBER_TO_STRUCT_PTR(ip, xyzzy, i) == zork);

	double *dp = &zork->d;
	TEST(MEMBER_TO_STRUCT_PTR(dp, xyzzy, d) == zork);

	char *sp = zork->s;
	TEST(MEMBER_TO_STRUCT_PTR(sp, xyzzy, s) == zork);

	free(zork);
}

TEST_CASE(size_of_flexible_array_struct)
{
	typedef struct {
		int x, y;
		double dd[];
	} xyzzy;

	xyzzy *p;
	size_t xyzzy_size = sizeof(xyzzy);
	size_t array_length = 10;
	size_t fam_size  = sizeof(double);
	size_t total = xyzzy_size + (fam_size * array_length);

	TEST(FAMSIZE(*p, dd, 10) == total);
}

TEST_CASE(num_chars_to_store_numbers)
{
	TEST((int)NUM_STR_LEN(int) >= snprintf(NULL, 0, "%d", INT_MAX));
	TEST((int)NUM_STR_LEN(char) >= snprintf(NULL, 0, "%d", CHAR_MAX));
	TEST((int)NUM_STR_LEN(long) >= snprintf(NULL, 0, "%ld", LONG_MAX));
}

//-----------------------------------------------------------------------------
// Primitive Utilities

_Static_assert(sizeof(byte) == 1, "byte must have sizeof 1");

TEST_CASE(if_null_default_pointer)
{
	char s[] = "default ptr";
	TEST(if_null(NULL, s) == s);

	void *d = &(int){100};
	void *p = s;
	TEST(if_null(p, d) == s);

	TEST(if_null_const(NULL, "not null"));
}

TEST_CASE(type_safe_max)
{
	TEST(ch_max('a', 'z') == 'z');
	TEST(ch_max('n', 'm') == 'n');
	
	TEST(int_max(6,7) == 7);
	TEST(int_max(14,9) == 14);
	TEST(int_max(45,45) == 45);

	TEST(fl_max(3.1, 5.6) == 5.6);
	TEST(fl_max(3.14159, 2.71828) == 3.14159);
	TEST(fl_max(3.14159, 3.14159) == 3.14159);
}

TEST_CASE(type_safe_min)
{
	TEST(ch_min('e', 'i') == 'e');
	TEST(ch_min('u', 'j') == 'j');

	TEST(int_min(203984, 109234) == 109234);
	TEST(int_min(-234, -4432) == -4432);

	TEST(fl_min(-23.34, 7243.7234) == -23.34);
	TEST(fl_min(0.02341, 1.2345) == 0.02341);
}

TEST_CASE(type_safe_swap)
{
	int x = 320, y = 200;
	int_swap(&x, &y);
	TEST(x == 200 && y == 320);

	char a = 'a', b = 'b';
	ch_swap(&a, &b);
	TEST(a == 'b' && b == 'a');

	double pi = 3.14159, e = 2.71828;
	fl_swap(&pi, &e);
	TEST(pi == 2.71828 && e == 3.14159);

	bool yes = true, no = false;
	bool_swap(&yes, &no);
	TEST(yes == false && no == true);
}

TEST_CASE(type_safe_const_cast)
{
	const char words[] = "can't change this";
	char *s = ch_deconst(words);
	TEST( ! strcmp(s, "can't change this"));

	const int numbers[] = { 1,2,3,4,5 };
	int *n = int_deconst(numbers);
	TEST(n[3] == 4);

	const double decimals[] = { 3.14, 2.718, 4.6692, 1.6180339887 };
	double *d = fl_deconst(decimals);
	TEST(d[1] == 2.718);
}

TEST_CASE(signed_int_overflow_detection)
{
	int big_a = INT_MAX / 2;
	int big_b = INT_MAX / 3;

	TEST( !int_mult_overflows(-1999, -2999) );
	TEST(  int_mult_overflows(-big_a, -big_b) );

	TEST( !int_mult_overflows( 1999, -2999) );
	TEST(  int_mult_overflows(big_a, -big_b) );

	TEST( !int_mult_overflows(-1999,  2999) );
	TEST(  int_mult_overflows(-big_a, big_b) );

	TEST( !int_mult_overflows(1999,  2999) );
	TEST(  int_mult_overflows(big_a, big_b) );
}


//-----------------------------------------------------------------------------
// Debug module
//

TEST_CASE(convert_status_enum_to_str)
{
	TEST( !strcmp(status_string(STATUS_FIRST), "OK") );
	TEST( !strcmp(status_string(STATUS_OK), "OK") );
	TEST( !strcmp(status_string(STATUS_ASSERT_FAILURE), "Assertion failed") );
	TEST( !strcmp(status_string(STATUS_END), "Unknown Status") );
	TEST( !strcmp(status_string(-1), "Unknown Status") );
	TEST( !strcmp(status_string(100000), "Unknown Status") );
}

TEST_CASE(capture_debug_context_info)
{
	struct source_location dbi = CURRENT_LOCATION;

	TEST(!strcmp(dbi.file, "test_klib.c"));
	TEST(dbi.line == (__LINE__-3));
	TEST(!strcmp(dbi.func, "TestCase_capture_debug_context_info"));
}


//-----------------------------------------------------------------------------
// exceptions
//

void this_func_throws_up(struct except_frame *xf)
{
	except_try(xf, STATUS_ERROR, CURRENT_LOCATION);
}

TEST_CASE(func_throws_exception)
{
	struct except_frame xf = {0};

	switch (EXCEPT_BEGIN(xf)) 
	{
		case EXCEPT_TRY:
			except_try(&xf, STATUS_OK, CURRENT_LOCATION);
			// it should not throw
			break;
		default:
			TEST(!"Exception was thrown!");
	}
	except_dispose(&xf);

	switch (EXCEPT_BEGIN(xf)) 
	{
		case EXCEPT_TRY:
			this_func_throws_up(&xf);
			TEST(!"Exception not thrown");
			break;
		case STATUS_ERROR:
			// okay - this is where we should be
			break;
		default:
			TEST(!"Wrong exception thrown");
	}
	except_dispose(&xf);
}

//-----------------------------------------------------------------------------
// Arithmetic Overflow Safety
//

TEST_CASE(size_t_overflow_detection)
{
	TEST(!size_t_mult_overflows(200, 300));
	TEST(!size_t_add_overflows(10000, 99));
	TEST( size_t_mult_overflows(SIZE_MAX-200, SIZE_MAX-300));
	TEST( size_t_add_overflows(SIZE_MAX-100, 101));
}

TEST_CASE(cast_ptrdiff_to_int_safely)
{
	struct except_frame xf = {0};
	switch (EXCEPT_BEGIN(xf)) 
	{
		case EXCEPT_TRY: 
		{
			const byte bytes[100];
			ptrdiff_t d = &bytes[90] - &bytes[65];
			int id = try_ptrdiff_to_int(d, &xf, CURRENT_LOCATION);
			TEST(id == 25);
			// it should not throw
			break;
		}
		case STATUS_MATH_OVERFLOW:
			TEST(!"Overflow exception was thrown!");
			break;
		default:
			TEST(!"Unknown exception was thrown!");
			break;
	}
	except_dispose(&xf);
}

TEST_CASE(cast_ptrdiff_to_int_fail_positive)
{
	struct except_frame xf = {0};
	switch (EXCEPT_BEGIN(xf)) 
	{
		case EXCEPT_TRY: 
		{
			ptrdiff_t big_diff = (ptrdiff_t)INT_MAX + 100;
			int id = try_ptrdiff_to_int(big_diff, &xf, CURRENT_LOCATION);
			TEST(!"Exception was not thrown!");
			break;
		}
		case STATUS_MATH_OVERFLOW:
			// OK - exception was expected.
			break;
		default:
			TEST(!"Unknown exception was thrown!");
			break;
	}
	except_dispose(&xf);
}

TEST_CASE(cast_ptrdiff_to_int_fail_negative)
{
	struct except_frame xf = {0};
	switch (EXCEPT_BEGIN(xf)) 
	{
		case EXCEPT_TRY: 
		{
			ptrdiff_t neg_diff = (ptrdiff_t)-INT_MAX - 100;
			int id = try_ptrdiff_to_int(neg_diff, &xf, CURRENT_LOCATION);
			TEST(!"Exception was not thrown!");
			break;
		}
		case STATUS_MATH_OVERFLOW:
			// OK - exception was expected.
			break;
		default:
			TEST(!"Unknown exception was thrown!");
			break;
	}
	except_dispose(&xf);
}

//-----------------------------------------------------------------------------
// range
//

TEST_CASE(ints_in_range)
{
	struct range r = { 0, 10 };
	
	TEST(!range_has(r, -1));
	TEST( range_has(r,  0));
	TEST( range_has(r,  5));
	TEST( range_has(r,  9));
	TEST(!range_has(r, 10));
}

//-----------------------------------------------------------------------------
// Span Template
//

TEST_CASE(properties_of_null_span)
{
	struct int_span null_span = {0};

	TEST(int_span_is_null(null_span));
	TEST(int_span_is_empty(null_span));
	TEST(int_span_length(null_span) == 0);
}

TEST_CASE(init_span_from_arrays)
{
	char letters[] = "abcdefghijklmnopqrstuvwxyz";
	struct char_span cs = char_span_init_n(letters, ARRAY_SIZE(letters)-1);
	TEST(char_span_length(cs) == 26);
	TEST(cs.front[5] == 'f');

	// Make a span from the array
	int fibs[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
	struct int_span nspan = int_span_init_n(fibs, 8);
	TEST(int_span_length(nspan) == 8);
	int i = 0;
	TEST(nspan.front[i++] ==  0);
	TEST(nspan.front[i++] ==  1);
	TEST(nspan.front[i++] ==  1);
	TEST(nspan.front[i++] ==  2);
	TEST(nspan.front[i++] ==  3);
	TEST(nspan.front[i++] ==  5);
	TEST(nspan.front[i++] ==  8);
	TEST(nspan.front[i++] == 13);
}

TEST_CASE(slice_span)
{
	//             0  1  2  3  4  5  6   7   8   9  10  11
	int fibs[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
	struct int_span fibspan = int_span_init_n(fibs, ARRAY_SIZE(fibs));

	struct int_span s1 = int_span_slice(fibspan, 3, 8);
	TEST(int_span_length(s1) == 5);
	TEST(s1.front[0] == 2);
	TEST(s1.front[1] == 3);
	TEST(s1.front[2] == 5);
	TEST(s1.front[3] == 8);
	TEST(s1.front[4] == 13);

	struct int_span s2 = int_span_slice(fibspan, 6, -3);
	TEST(int_span_length(s2) == 3);
	TEST(s2.front[0] == 8);
	TEST(s2.front[1] == 13);
	TEST(s2.front[2] == 21);
}

TEST_CASE(vector_init)
{
	TVector(int, x, y, z) point = { 10, 20, 30 };

	TEST(point.x == 10);
	TEST(point.y == 20);
	TEST(point.z == 30);
	TEST(point.at[0] == 10);
	TEST(point.at[1] == 20);
	TEST(point.at[2] == 30);
	TEST(VECT_LENGTH(point) == 3);
}


//-----------------------------------------------------------------------------
// strand
//

TEST_CASE(properties_of_null_strand)
{
	struct strand nullstr = { NULL, NULL };

	struct strand empty   = STR("");
	struct strand xyzzy   = STR("xyzzy");

	TEST(   strand_is_null(nullstr));
	TEST(   strand_is_empty(nullstr));
	TEST(   strand_length(nullstr) == 0);
	TEST(   strand_equals(nullstr, (struct strand){NULL,NULL}));
	TEST(   strand_equals(nullstr, empty));
	TEST(   strand_equals(empty, nullstr));
	TEST( ! strand_equals(nullstr, xyzzy));
	TEST( ! strand_equals(xyzzy, nullstr));
}

TEST_CASE(properties_of_empty_strand)
{
	struct strand empty   = STR("");

	struct strand nullstr = { NULL, NULL };
	struct strand xyzzy   = STR("xyzzy");

	TEST( ! strand_is_null(empty));
	TEST(   strand_is_empty(empty));
	TEST(   strand_length(empty) == 0);
	TEST(   strand_equals(empty, nullstr));
	TEST(   strand_equals(empty, empty));
	TEST( ! strand_equals(empty, xyzzy));
}

TEST_CASE(properties_of_strand)
{
	struct strand xyzzy   = STR("xyzzy");

	char   xyzzy_array[] = "xyzzy";
	struct strand xyzzy_comp = strand_init_n(xyzzy_array, strlen(xyzzy_array));

	struct strand empty   = STR("");
	struct strand nullstr = { NULL, NULL };
	struct strand zork    = STR("zork");
	struct strand xyzzy_longer = STR("xyzzy_longer");
	char sub[] = "abxyzzycd";
	struct strand sub_xyzzy = { sub+2, sub+7 };

	TEST(!strand_is_null(xyzzy));
	TEST(!strand_is_empty(xyzzy));
	TEST( strand_length(xyzzy) == 5);
	TEST( strand_equals(xyzzy,  xyzzy_comp));
	TEST( strand_equals(xyzzy_comp,  xyzzy));
	TEST( strand_equals(xyzzy,  sub_xyzzy));
	TEST( strand_equals(sub_xyzzy, xyzzy));

	TEST(!strand_equals(xyzzy,  nullstr));
	TEST(!strand_equals(xyzzy,  empty));
	TEST(!strand_equals(xyzzy,  zork));
	TEST(!strand_equals(zork,   xyzzy));
	TEST(!strand_equals(xyzzy, xyzzy_longer));
	TEST(!strand_equals(xyzzy_longer, xyzzy));
}

TEST_CASE(return_trimmed_span)
{
	char text[] = " \t\v\r\n  Trim trailing whitespace  \t\v\r\n   ";
	struct strand s = STR(text);

	struct strand trimmed = strand_trim_back(s, isspace);
	TEST( strand_equals(trimmed, STR(" \t\v\r\n  Trim trailing whitespace")) );

	trimmed = strand_trim_front(s, isspace);
	TEST( strand_equals(trimmed, STR("Trim trailing whitespace  \t\v\r\n   ")) );

	trimmed = strand_trim(s, isspace);
	TEST( strand_equals(trimmed, STR("Trim trailing whitespace")) );

	char empty_text[] = "";
	struct strand empty = STR(empty_text);
	TEST( strand_equals(strand_trim_back(empty, isspace), STR("")) );
	TEST( strand_equals(strand_trim_front(empty, isspace), STR("")) );
	TEST( strand_equals(strand_trim(empty, isspace), STR("")) );

	char spaces_text[] = "  \t\v\r\n   ";
	struct strand spaces = STR(spaces_text);
	TEST( strand_equals(strand_trim_back(spaces, isspace), STR("")) );
	TEST( strand_equals(strand_trim_front(spaces, isspace), STR("")) );
	TEST( strand_equals(strand_trim(spaces, isspace), STR("")) );
}

TEST_CASE(null_strbuf_properties)
{
	struct strbuf *buf = NULL;

	TEST(strbuf_length(buf) == 0);
	TEST(strbuf_cap(buf) == 0);
}

TEST_CASE(empty_strbuf_properties)
{
	struct strbuf buf = STRBUF_INIT((char[32]){});

	TEST(buf.size == 32);
	TEST(strbuf_length(&buf) == 0);
	TEST(strbuf_cap(&buf) == 32);
}

enum status strbuf_cat(struct strbuf *buf, struct strand str)
{
	int len = strand_length(str);
	if (len > strbuf_cap(buf))
		return STATUS_OUT_OF_SPACE;

	memcpy(buf->back, str.front, len);
	buf->back += len;

	return STATUS_OK;
}

struct strand strbuf_strand(struct strbuf buf)
{
	return (struct strand){ .front = buf.front, .back = buf.back };

}
TEST_CASE(concat_strand_to_strbuf)
{
	struct strbuf buf = STRBUF_INIT((char[100]){});

	char hello[] = "Hello, world.";
	struct strand s1 = STR(hello);

	enum status stat = strbuf_cat(&buf, s1); 
	struct strand res = strbuf_strand(buf);

	TEST(stat == STATUS_OK);
	TEST(strbuf_length(&buf) == 13);
	TEST(strbuf_cap(&buf) == 100-13);
	TEST(strand_equals(res, STR("Hello, world.")));


	char bye[] = " Goodbye, Pluto!";
	stat = strbuf_cat(&buf, STR(bye));
	res = strbuf_strand(buf);

	TEST(stat == STATUS_OK);
	TEST(strbuf_length(&buf) == 13+16);
	TEST(strbuf_cap(&buf) == 100-13-16);
	TEST(strand_equals(res, STR("Hello, world. Goodbye, Pluto!")));

}

//-----------------------------------------------------------------------------
// Doubly linked List
//

TEST_CASE(attach_two_links)
{
	// Given two empty links
	struct link a = LINK_INIT();
	struct link b = LINK_INIT();
	TEST(link_not_attached(&a));
	TEST(link_not_attached(&b));
	
	// When they're linked
	link_attach(&a, &b);

	// They are attached and adjacent
	TEST(links_are_attached(&a, &b));
	TEST(link_next(&a) == &b);
	TEST(link_prev(&b) == &a);
}

TEST_CASE(insert_link)
{
	// Given two linked and one solo nodes
	struct link a = LINK_INIT();
	struct link b = LINK_INIT();
	struct link c = LINK_INIT();
	link_attach(&a, &b);
	TEST(links_are_attached(&a, &b));
	TEST(link_not_attached(&c) );

	// When node c inserted before b
	link_insert(&c, &b);

	// Then a.right is c and c.right is b
	TEST(links_are_attached(&a, &c) );
	TEST(links_are_attached(&c, &b) );
}

TEST_CASE(link_remove_node)
{
	// Given chain a:b:c
	struct link a = LINK_INIT();
	struct link b = LINK_INIT();
	struct link c = LINK_INIT();
	link_attach(&a, &b);
	link_attach(&b, &c);

	// When b is removed
	link_remove(&b);

	// Then a is linked to c and b is unlinked
	TEST(link_not_attached(&b));
	TEST(links_are_attached(&a, &c));
}

TEST_CASE(chain_multiple_links)
{
	// Given empty chain and several links
	Chain chain = CHAIN_INIT(chain);
	struct link a = LINK_INIT(), 
	     b = LINK_INIT(),
	     c = LINK_INIT(),
	     d = LINK_INIT(),
	     e = LINK_INIT(),
	     f = LINK_INIT();

	// When all appended to chain
	Chain_appends(&chain, &a, &b, &c, &d, &e, &f, NULL);

	// Each attached to the next
	struct link *l = Chain_first(&chain);
	TEST(l == &a);
	TEST((l = link_next(l)) == &b);
	TEST((l = link_next(l)) == &c);
	TEST((l = link_next(l)) == &d);
	TEST((l = link_next(l)) == &e);
	TEST((l = link_next(l)) == &f);
	TEST(l == Chain_last(&chain));
}

TEST_CASE(link_foreach)
{
	Chain chain = CHAIN_INIT(chain);

	struct test_node {
		struct link link;
		int i;
	} a = {.i = 1}, 
	  b = {.i = 2},
	  c = {.i = 3},
	  d = {.i = 4},
	  e = {.i = 5},
	  f = {.i = 6};

	// Given chain a:b:c:d:e:f
	Chain_appends(&chain, &a, &b, &c, &d, &e, &f, NULL);

	// When we sum all node values with foreach
	int total = 0;
	int i_offset = offsetof(struct test_node, i);
	Chain_foreach(&chain, sum_ints, &total, i_offset);

	// Then we get a total
	TEST(total == 21);
}

TEST_CASE(add_links_to_empty_chain)
{
	// Given an empty chain
	Chain chain = CHAIN_INIT(chain);
	TEST(Chain_empty(&chain));

	// Append links to chain
	struct link a = LINK_INIT();
	Chain_append(&chain, &a);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &chain.head));

	struct link b = LINK_INIT();
	Chain_append(&chain, &b);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &b));
	TEST(links_are_attached(&b, &chain.head));

	struct link c = LINK_INIT();
	Chain_append(&chain, &c);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &b));
	TEST(links_are_attached(&b, &c));
	TEST(links_are_attached(&c, &chain.head));
}

TEST_CASE(prepent_links_to_chain)
{
	// Given an empty chain
	Chain chain = CHAIN_INIT(chain);
	TEST(Chain_empty(&chain));

	// Prepend links to chain
	struct link a = LINK_INIT();
	Chain_prepend(&chain, &a);
	TEST(links_are_attached(&a, &chain.head));
	TEST(links_are_attached(&chain.head, &a));

	struct link b = LINK_INIT();
	Chain_prepend(&chain, &b);
	TEST(links_are_attached(&chain.head, &b));
	TEST(links_are_attached(&b, &a));
	TEST(links_are_attached(&a, &chain.head));

	struct link c = LINK_INIT();
	Chain_prepend(&chain, &c);
	TEST(links_are_attached(&chain.head, &c));
	TEST(links_are_attached(&c, &b));
	TEST(links_are_attached(&b, &a));
	TEST(links_are_attached(&a, &chain.head));
}


//-----------------------------------------------------------------------------
// Error Module


//-----------------------------------------------------------------------------
// Logging





TEST_CASE(Fib_iterate_the_fibonacci_sequence)
{
	// This is an example for sequence iterators

	Fibonacci fib = Fib_begin();

	TEST(Fib_get(fib) == 0);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 1);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 1);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 2);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 3);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 5);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 8);

	fib = Fib_next(fib);
	TEST(Fib_get(fib) == 13);
}

TEST_CASE(declare_null_list_is_empty)
{
	// When you declare a null list pointer
	LIST(int) *l = NULL;

	// The list has no capacity, no length, is empty, and is full
	TEST(List_capacity(l) == 0);
	TEST(List_length(l) == 0);
	TEST(List_is_empty(l));
	TEST(List_is_full(l));
	TEST(!List_in_bounds(l, 0));

	// You can dispose a null list and that's fine.
	List_dispose(l);
}

TEST_CASE(add_item_to_null_list)
{
	// When you add an item to a null list
	LIST(int) *l = NULL;
	int value = 101;
	LIST_PUSH(l, value);

	// The list grows to hold it.
	TEST(l != NULL);
	TEST(List_length(l) == 1);
	TEST(List_capacity(l) >= List_length(l));
	TEST(List_in_bounds(l, 0));
	TEST(!List_in_bounds(l, 1));
	TEST(LIST_LAST(l) == 101);
	
	List_dispose(l);
}

TEST_CASE(resize_null_list)
{
	// When you resize a null list
	LIST(int) *s = NULL;
	int new_size = 3;
	LIST_ADD(s, new_size);

	// The list has new un-initialized items
	TEST(s != NULL);
	TEST(List_length(s) == new_size);
	TEST(List_capacity(s) >= List_length(s));
	TEST(!List_is_empty(s));
	TEST(List_in_bounds(s, 0));
	TEST(List_in_bounds(s, 1));
	TEST(List_in_bounds(s, 2));
	TEST(!List_in_bounds(s, 3));
	
	List_dispose(s);
}

TEST_CASE(reserve_space_null_list)
{
	// When you reserve space for a null list
	LIST(int) *s = NULL;
	int new_cap = 5;
	LIST_RESERVE(s, new_cap);

	// Then at least that much space is allocated,
	// and list is still empty
	TEST(s != NULL);
	TEST(List_is_empty(s));
	TEST(List_length(s) == 0);
	TEST(List_capacity(s) >= new_cap);
	TEST(!List_is_full(s));
	TEST(!List_in_bounds(s, 0));

	List_dispose(s);
}

TEST_CASE(add_item_to_empty_list)
{
	// Given an empty non-null list
	LIST(int) *l = NULL;
	LIST_RESERVE(l, 5);
	TEST(List_is_empty(l));
	TEST(!List_is_full(l));
	ListDims old = l->head;

	// When an element is added to the list
	LIST_PUSH(l, 202);
	
	// Length increases but cap does not
	TEST(l != NULL);
	TEST(!List_is_empty(l));
	TEST(List_length(l) == 1);
	TEST(LIST_LAST(l) == 202);
	TEST(!List_is_full(&old) || List_capacity(l) > old.cap);

	List_dispose(l);
}



TEST_CASE(Xorshift_random_numbers)
{
	return;

	uint32_t seed = 0xAFAFAFAF;
	Xorshifter rng = { .x = seed, .a = 13, .b = 17, .c = 5 };

	int n = 10;
	for (int i = 0; i < n; ++i)
		printf("xorshift: %u\n", Xorshift_rand(&rng));

	puts("---");

	uint32_t x = seed;
	for (int i = 0; i < n; ++i) {
		x ^= x << 13,
		x ^= x >> 17,
		x ^= x <<  5;
		printf("xorshift: %u\n", x);
	}
}


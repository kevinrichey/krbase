#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>

#define USING_KR_NAMESPACE
#include "test.h"

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
	// All test cases have a hidden parameter named "test_counter".
	UNUSED(test_counter);

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

TEST_CASE(bytes_are_size_one)
{
	TEST(sizeof(byte) == 1);
}

TEST_CASE(in_bounds_enums_and_arrays)
{
	TEST(in_bounds(3, 1, 5));
	TEST(in_bounds(1, 1, 5));
	TEST(in_bounds(5, 1, 5));
	TEST(!in_bounds(0, 1, 5));
	TEST(!in_bounds(6, 1, 5));

	enum TestBoundsEnum {
		Bounds_One,
		Bounds_Two,
		Bounds_Three,
		STANDARD_ENUM_VALUES(Bounds)
	};

	TEST(in_enum_bounds(Bounds_One, Bounds));
	TEST(in_enum_bounds(Bounds_Two, Bounds));
	TEST(in_enum_bounds(Bounds_Three, Bounds));
	TEST(!in_enum_bounds(-1, Bounds));
	TEST(!in_enum_bounds(99, Bounds));

	int array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	TEST(in_array_bounds(0, array));
	TEST(in_array_bounds(5, array));
	TEST(in_array_bounds(8, array));
	TEST(!in_array_bounds(9, array));
	TEST(!in_array_bounds(-1, array));
}

TEST_CASE(length_of_fixed_size_string)
{
	char same_len[] = "xyzzy";
	TEST(strnlen(same_len, sizeof(same_len)) == 5);

	char longer_str[100] = "less than one hundred";
	TEST(strnlen(longer_str, sizeof(longer_str)) == 21);

	char shorter[] = "send a shorter max length";
	TEST(strnlen(shorter, 10) == 10);
	
	char empty[] = "";
	TEST(strnlen(empty, sizeof(empty)) == 0);
}

TEST_CASE(reverse_copy_a_string)
{
	char str[] = "Reverse this string";
	char rev[25];
	char small[10];

	TEST( !strcmp(strnrev(rev, str, ARRAY_SIZE(rev)), "gnirts siht esreveR") );
	TEST( !strcmp(strnrev(small, str, ARRAY_SIZE(small)), "gnirts si") );
}

TEST_CASE(convert_int_to_chars)
{
	char s[NUM_STR_LEN(int)];
	int  len = ARRAY_SIZE(s);

	TEST(!strcmp(itoa(0, s, len), "0"));
	TEST(!strcmp(itoa(1, s, len), "1"));
	TEST(!strcmp(itoa(100, s, len), "100"));
	TEST(!strcmp(itoa(INT_MAX, s, len), "2147483647"));
	TEST(!strcmp(itoa(-0, s, len), "0"));
	TEST(!strcmp(itoa(-1, s, len), "-1"));
	TEST(!strcmp(itoa(-100, s, len), "-100"));
	TEST(!strcmp(itoa(INT_MIN, s, len), "-2147483648"));
}


//-----------------------------------------------------------------------------
// Span Template
//

TEST_CASE(empty_spans)
{
	ispan is = SPAN_INIT((int[0]){});
	TEST(SPAN_IS_EMPTY(is));
}

TEST_CASE(init_span_from_arrays)
{
	char letters[] = "abcdefghijklmnopqrstuvwxyz";
	cspan cs = SPAN_INIT(letters);
	TEST(SPAN_LENGTH(cs) == 27); // don't forget the null-terminator
	TEST(cs.begin[5] == 'f');

	// Make a span from the array
	int fibs[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
	ispan nspan = SPAN_INIT(fibs, 8);
	TEST(SPAN_LENGTH(nspan) == 8);
	int i = 0;
	TEST(nspan.begin[i++] ==  0);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  2);
	TEST(nspan.begin[i++] ==  3);
	TEST(nspan.begin[i++] ==  5);
	TEST(nspan.begin[i++] ==  8);
	TEST(nspan.begin[i++] == 13);
}

bool pass_string_span_rvalue(cspan cs)
{
	return !strcmp(cs.begin, "xyzzy");
}

#define LSTR(S_)   (cspan)SPAN_INIT(S_)
#define PSTR(S_)   (cspan)SPAN_INIT(S_, strlen(S_))

TEST_CASE(string_span_rvalue)
{
	TEST( pass_string_span_rvalue(LSTR("xyzzy")) );
	TEST( pass_string_span_rvalue(PSTR("xyzzy")) );
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
// string
//

TEST_CASE(null_string_is_empty)
{
	string *s = NULL;

	TEST(string_is_empty(s));
	TEST(string_length(s) == 0);
	TEST(string_equals(s, NULL));
	TEST(!string_equals(s, ""));
	TEST(!string_equals(s, "xyzzy"));

	string_dispose(s);
}

TEST_CASE(empty_strings)
{
	string *s = string_copy("");

	TEST(string_is_empty(s));
	TEST(string_length(s) == 0);
	TEST(string_equals(s, ""));
	TEST(!string_equals(s, NULL));
	TEST(!string_equals(s, "xyzzy"));

	string_dispose(s);
}

TEST_CASE(string_lifecycle)
{
	string *s = string_copy("Hello, world.");

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 13);
	TEST(string_equals(s, "Hello, world."));
	TEST(!string_equals(s, "xyzzy"));
	TEST(!string_equals(s, NULL));

	int i = 0;
	cspan schars = string_span(s);
	TEST(schars.begin[i++] == 'H');
	TEST(schars.begin[i++] == 'e');
	TEST(schars.begin[i++] == 'l');
	TEST(schars.begin[i++] == 'l');
	TEST(schars.begin[i++] == 'o');
	TEST(SPAN_LAST(schars) == '.');

	string_dispose(s);
}

TEST_CASE(create_formatted_string)
{
	const char *answer = "the answer";
	string *s = string_format("%s to %d/%d is %.5f", answer, 2, 3, 2.0/3.0);

	TEST(string_equals(s, "the answer to 2/3 is 0.66667"));

	string_dispose(s);
}

//-----------------------------------------------------------------------------
// Doubly Linked List
//

TEST_CASE(attach_two_links)
{
	// Given two empty links
	Link a = LINK_INIT();
	Link b = LINK_INIT();
	TEST(Link_not_attached(&a));
	TEST(Link_not_attached(&b));
	
	// When they're linked
	Link_attach(&a, &b);

	// They are attached and adjacent
	TEST(Links_are_attached(&a, &b));
	TEST(Link_next(&a) == &b);
	TEST(Link_prev(&b) == &a);
}

TEST_CASE(insert_link)
{
	// Given two linked and one solo nodes
	Link a = LINK_INIT();
	Link b = LINK_INIT();
	Link c = LINK_INIT();
	Link_attach(&a, &b);
	TEST(Links_are_attached(&a, &b));
	TEST(Link_not_attached(&c) );

	// When node c inserted before b
	Link_insert(&c, &b);

	// Then a.right is c and c.right is b
	TEST(Links_are_attached(&a, &c) );
	TEST(Links_are_attached(&c, &b) );
}

TEST_CASE(link_remove_node)
{
	// Given chain a:b:c
	Link a = LINK_INIT();
	Link b = LINK_INIT();
	Link c = LINK_INIT();
	Link_attach(&a, &b);
	Link_attach(&b, &c);

	// When b is removed
	Link_remove(&b);

	// Then a is linked to c and b is unlinked
	TEST(Link_not_attached(&b));
	TEST(Links_are_attached(&a, &c));
}

TEST_CASE(chain_multiple_links)
{
	// Given empty chain and several links
	Chain chain = CHAIN_INIT(chain);
	Link a = LINK_INIT(), 
	     b = LINK_INIT(),
	     c = LINK_INIT(),
	     d = LINK_INIT(),
	     e = LINK_INIT(),
	     f = LINK_INIT();

	// When all appended to chain
	Chain_appends(&chain, &a, &b, &c, &d, &e, &f, NULL);

	// Each attached to the next
	Link *l = Chain_first(&chain);
	TEST(l == &a);
	TEST((l = Link_next(l)) == &b);
	TEST((l = Link_next(l)) == &c);
	TEST((l = Link_next(l)) == &d);
	TEST((l = Link_next(l)) == &e);
	TEST((l = Link_next(l)) == &f);
	TEST(l == Chain_last(&chain));
}

TEST_CASE(link_foreach)
{
	Chain chain = CHAIN_INIT(chain);

	struct test_node {
		Link link;
		int i;
	} a = LINK_INIT(.i = 1), 
	  b = LINK_INIT(.i = 2),
	  c = LINK_INIT(.i = 3),
	  d = LINK_INIT(.i = 4),
	  e = LINK_INIT(.i = 5),
	  f = LINK_INIT(.i = 6);

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
	Link a = LINK_INIT();
	Chain_append(&chain, &a);
	TEST(Links_are_attached(&chain.head, &a));
	TEST(Links_are_attached(&a, &chain.head));

	Link b = LINK_INIT();
	Chain_append(&chain, &b);
	TEST(Links_are_attached(&chain.head, &a));
	TEST(Links_are_attached(&a, &b));
	TEST(Links_are_attached(&b, &chain.head));

	Link c = LINK_INIT();
	Chain_append(&chain, &c);
	TEST(Links_are_attached(&chain.head, &a));
	TEST(Links_are_attached(&a, &b));
	TEST(Links_are_attached(&b, &c));
	TEST(Links_are_attached(&c, &chain.head));
}

TEST_CASE(prepent_links_to_chain)
{
	// Given an empty chain
	Chain chain = CHAIN_INIT(chain);
	TEST(Chain_empty(&chain));

	// Prepend links to chain
	Link a = LINK_INIT();
	Chain_prepend(&chain, &a);
	TEST(Links_are_attached(&a, &chain.head));
	TEST(Links_are_attached(&chain.head, &a));

	Link b = LINK_INIT();
	Chain_prepend(&chain, &b);
	TEST(Links_are_attached(&chain.head, &b));
	TEST(Links_are_attached(&b, &a));
	TEST(Links_are_attached(&a, &chain.head));

	Link c = LINK_INIT();
	Chain_prepend(&chain, &c);
	TEST(Links_are_attached(&chain.head, &c));
	TEST(Links_are_attached(&c, &b));
	TEST(Links_are_attached(&b, &a));
	TEST(Links_are_attached(&a, &chain.head));
}


//-----------------------------------------------------------------------------
// Error Module

TEST_CASE(Status_to_string)
{
	TEST( !strcmp(Status_string(Status_First), "OK") );
	TEST( !strcmp(Status_string(Status_OK), "OK") );
	TEST( !strcmp(Status_string(Status_Error), "Error") );
	TEST( !strcmp(Status_string(Status_End), "Unknown Status") );
	TEST( !strcmp(Status_string(-1), "Unknown Status") );
	TEST( !strcmp(Status_string(100000), "Unknown Status") );
}

static void 
source_info_test_fn(TestCounter *test_counter, int line, SourceInfo source)
{
	TEST(!strcmp(source.file, "test_klib.c"));
	TEST(source.line == line);
}

TEST_CASE(pass_source_info_parameter)
{
	int line = __LINE__ + 1;
	source_info_test_fn(test_counter, line, SOURCE_HERE);
}


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
	UNUSED(test_counter);
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


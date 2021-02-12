#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "klib.h"


TEST_CASE(this_always_fails)
{
	UNUSED(test_counter);
//	TEST(!"This test case always fails");
}

//-----------------------------------------------------------------------------
// Testing Language Assumptions
//

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

	// Does not work on plain pointers tho
	int *b = a;
	TEST(ARRAY_SIZE(b) != 10);
}


//-----------------------------------------------------------------------------
// Fun Variadic Functions
//

void unpack_i(int *a, int n, ...)
{
	va_list args;
	va_start(args, n);

	int *i;
	while (n --> 0) {
		if ((i = va_arg(args, int*)))
			*i = *a++;
		else
			break;
	}

	va_end(args);
}

TEST_CASE(unpack_entire_array)
{
	int arr[] = { 10, 11, 12, 13, 14 };
	int a, b, c, d, e;

	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, &d, &e);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
	TEST(d == 13);
	TEST(e == 14);
}

TEST_CASE(unpack_partial_array)
{
	int arr[] = { 10, 11, 12, 13, 14 };
	int a, b, c;

	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, NULL);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
}

TEST_CASE(unpack_shorter_array)
{
	int arr[] = { 10, 11, 12 };
	int a, b, c;
	int d = 99;
	int e = 88;

	// Since array is shorter, only 3 variables will be updated
	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, &d, &e);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
	TEST(d == 99);
	TEST(e == 88);
}



TEST_CASE(Status_to_string)
{
	TEST( !strcmp(Status_string(Status_First), "OK") );
	TEST( !strcmp(Status_string(Status_OK), "OK") );
	TEST( !strcmp(Status_string(Status_Error), "Error") );
	TEST( !strcmp(Status_string(Status_End), "Unknown Status") );
}

TEST_CASE(Error_declare_literal)
{
	ErrorInfo e = {0};
	int error_line = __LINE__ + 1;
	e = ERROR_LITERAL(Status_Error, "Oops!");

	TEST(Error_status(&e) == Status_Error);
	TEST(!strcmp(e.filename, "test_klib.c"));
	TEST(e.fileline == error_line);
	TEST(!strcmp(e.message, "Oops!"));
}

TEST_CASE(Error_init)
{
	int error_line = __LINE__ + 1;
	ErrorInfo e = ERROR_INIT(Status_Error, "Uh-oh!!");

	TEST(Error_status(&e) == Status_Error);
	TEST(!strcmp(e.filename, "test_klib.c"));
	TEST(e.fileline == error_line);
	TEST(!strcmp(e.message, "Uh-oh!!"));
}

static StatusCode test_error_return_code(ErrorInfo *err)
{
	return ERROR(*err, Status_Error, "DOH!");
}

TEST_CASE(Error_return_value)
{
	ErrorInfo e = {0};

	StatusCode ret_stat = test_error_return_code(&e);

	TEST(ret_stat == Status_Error);
	TEST(Error_status(&e) == Status_Error);
	TEST(!strcmp(e.filename, "test_klib.c"));
	TEST(!strcmp(e.message, "DOH!"));
}

TEST_CASE(Error_check_bad_param)
{
	UNUSED(test_counter);

	//int param = 1;
	//CHECK(param == 0);
}

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

TEST_CASE(VECTOR_init)
{
	VECTOR(int, x, y, z) point = { 10, 20, 30 };

	TEST(point.x == 10);
	TEST(point.y == 20);
	TEST(point.z == 30);
	TEST(point.at[0] == 10);
	TEST(point.at[1] == 20);
	TEST(point.at[2] == 30);
	TEST(VECT_LENGTH(point) == 3);
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

TEST_CASE(Binode_link_2_solo_nodes)
{
	struct {
		Binode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  *p;

	// Given two unlinked nodes
	TEST(a.link.right == NULL);
	TEST(a.link.left  == NULL);
	TEST(Binode_not_linked(&a.link));
	TEST(Binode_not_linked(&b.link));
	
	// When they're linked
	Binode_link(&a.link, &b.link);

	// Then a.right == b and b is still unlinked
	TEST(Binodes_are_linked(&a.link, &b.link));
	TEST(Binode_next(&a.link) == (void*)&b);
	TEST(Binode_next(&b.link) == NULL);
	TEST(Binode_is_linked(&a.link));
	TEST(Binode_is_linked(&b.link));

	// and advancing to a right is b
	p = &a;
	TEST(p->q == 'a');
	p = Binode_next((Binode*)p);
	TEST(p->q == 'b');
}

TEST_CASE(Binode_insert_between_2_links)
{
	struct {
		Binode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  c = { .q = 'c' },
	  *p;

	// Given two linked and one solo nodes
	Binode_link(&a.link, &b.link);
	TEST( Binodes_are_linked(&a.link, &b.link) );
	TEST( Binode_not_linked(&c.link) );
	p = NULL;

	// When solo node c inserted after a
	Binode_insert(&a.link, &c.link);

	// Then a.right is c and c.right is b
	p = &a;
	TEST(p->q == 'a');
	p = Binode_next(&p->link);
	TEST(p->q == 'c');
	p = Binode_next(&p->link);
	TEST(p->q == 'b');
}

TEST_CASE(Binode_link_to_null)
{
	struct {
		Binode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  *p;

	p = NULL;

	// Given a and b are linked
	Binode_link(&a.link, &b.link);
	TEST( Binodes_are_linked(&a.link, &b.link) );
	
	// When a is linked to NULL
	Binode_link(&a.link, NULL);

	// Then a is no longer linked to b
	// and b is still linked to a
	TEST( !Binodes_are_linked(&a.link, &b.link) );
	TEST( Binode_not_linked(&a.link) );
	TEST( Binode_next(&a.link) != &b.link );
	TEST( Binode_prev(&b.link) == &a.link );
}

TEST_CASE(Binode_remove_node)
{
	struct {
		Binode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  c = { .q = 'c' };

	// Given chain a:b:c
	Binode_link(&a.link, &b.link);
	Binode_link(&b.link, &c.link);

	// When b is removed
	Binode_remove(&b.link);

	// Then a is linked to c and b is unlinked
	TEST(Binode_not_linked(&b.link));
	TEST(Binodes_are_linked(&a.link, &c.link));
}

TEST_CASE(Binode_remove_last_node)
{
	struct {
		Binode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  c = { .q = 'c' };

	// Given chain a:b:c
	Binode_link(&a.link, &b.link);
	Binode_link(&b.link, &c.link);

	// When c is removed
	Binode_remove(&c.link);

	// Then a is linked to b and c is unlinked
	TEST(Binode_not_linked(&c.link));
	TEST(Binodes_are_linked(&a.link, &b.link));
	TEST(!Binodes_are_linked(&b.link, &c.link));
}

TEST_CASE(Binode_foreach)
{
	struct test_node {
		Binode link;
		int i;
	} a = { .i = 1 }, 
	  b = { .i = 2 },
	  c = { .i = 3 },
	  d = { .i = 4 },
	  e = { .i = 5 },
	  f = { .i = 6 };

	// Given chain a:b:c:d:e:f
	Binode_link(&a.link, &b.link);
	Binode_link(&b.link, &c.link);
	Binode_link(&c.link, &d.link);
	Binode_link(&d.link, &e.link);
	Binode_link(&e.link, &f.link);

	// When we sum all node values with foreach
	int total = 0;
	int i_offset = offsetof(struct test_node,i);
	Binode_foreach(&a.link, sum_ints, &total, i_offset);

	// Then we get a total
	TEST(total == 21);
}

TEST_CASE(Binode_make_chain)
{
	struct test_node {
		Binode link;
		int i;
	} a = { .i = 1 }, 
	  b = { .i = 2 },
	  c = { .i = 3 },
	  d = { .i = 4 },
	  e = { .i = 5 },
	  f = { .i = 6 };

	// Given several unlinked nodes
	TEST(Binode_not_linked(&a.link));
	TEST(Binode_not_linked(&b.link));
	TEST(Binode_not_linked(&c.link));
	TEST(Binode_not_linked(&d.link));
	TEST(Binode_not_linked(&e.link));
	TEST(Binode_not_linked(&f.link));

	// When chained together
	Chain x = BINODE_CHAIN(&a, &b, &c, &d, &e, &f);

	// Then they're all linked
	TEST(Chain_head(&x) == (Binode*)&a);
	TEST(Chain_tail(&x) == (Binode*)&f);
	TEST(Binode_prev(x.head) == NULL);
	TEST(Binode_next(x.tail) == NULL);
	TEST(Binodes_are_linked(&a.link, &b.link));
	TEST(Binodes_are_linked(&b.link, &c.link));
	TEST(Binodes_are_linked(&c.link, &d.link));
	TEST(Binodes_are_linked(&d.link, &e.link));
	TEST(Binodes_are_linked(&e.link, &f.link));
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

//----------------------------------------------------------------------
// Everything below is experimental work in progress


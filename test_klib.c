#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include "test.h"

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
}


//-----------------------------------------------------------------------------
// Primitive Utilities

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

//-----------------------------------------------------------------------------
// span
//

TEST_CASE(init_span_from_arrays)
{
	char letters[] = "abcdefghijklmnopqrstuvwxyz";
	str_span cspan = span_init(letters);
	TEST(cspan.size == 27); // don't forget the null-terminator
	TEST(cspan.ptr[5] == 'f');

	// Make a span from the array
	int fibs[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
	span(int) nspan = span_init(fibs, 12);
	TEST(nspan.size == 12);
	int i = 0;
	TEST(nspan.ptr[i++] ==  0);
	TEST(nspan.ptr[i++] ==  1);
	TEST(nspan.ptr[i++] ==  1);
	TEST(nspan.ptr[i++] ==  2);
	TEST(nspan.ptr[i++] ==  3);
	TEST(nspan.ptr[i++] ==  5);
	TEST(nspan.ptr[i++] ==  8);
	TEST(nspan.ptr[i++] == 13);
	TEST(nspan.ptr[i++] == 21);
	TEST(nspan.ptr[i++] == 34);
	TEST(nspan.ptr[i++] == 55);
	TEST(nspan.ptr[i++] == 89);
}

//-----------------------------------------------------------------------------
// Assertions Module

typedef void (*AssertHandlerFn)(SourceInfo source, const char *message, void *baggage);

typedef struct {
	AssertHandlerFn func;
	void *baggage;
} AssertionHandler;

static AssertionHandler assert_handler;

AssertionHandler Assert_swap_handler(AssertionHandler new_handler)
{
	AssertionHandler old = assert_handler;
	assert_handler = new_handler;
	return old;
}

void Assert_set_handler(AssertHandlerFn new_hand, void *bag)
{
	assert_handler.func = new_hand;
	assert_handler.baggage = bag;
}

void Assert_failed(SourceInfo source, const char *message)
{
	if (assert_handler.func)
		assert_handler.func(source, message, assert_handler.baggage);
}

#define CHECK(CONDITION_)   \
	do{ if (CONDITION_); else Assert_failed(SOURCE_HERE, #CONDITION_); } while(0)

void test_assert_handler(SourceInfo source, const char *message, void *baggage)
{
	UNUSED(message);
	UNUSED(source);
	*((int*)baggage) = 5; 
	printf("%s:%d: Assertion failed: %s\n", source.file, source.line, message);
}

TEST_CASE(custom_assert_handler)
{
	return;

	int asserted = 0;

	Assert_set_handler(test_assert_handler, &asserted);

	bool oops = false;
	CHECK(oops);

	TEST(asserted == 5);
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


static StatusCode test_error_handler(Error *err)
{
	bool *handler_called = err->baggage;
	*handler_called = true;
	return err->status;
}

TEST_CASE(function_fails_call_error_handler)
{
	// Given an error handler
	ErrorHandler oldh = Error_set_handler(Status_Error, test_error_handler);
	bool handler_called = false;
	Error err = { .baggage = &handler_called };

	// When a function fails
	int lineno = __LINE__ + 1;
	StatusCode stat = Error_fail(&err, Status_Error, "Error message here", SOURCE_HERE);

	// Then error status is set and the handler was called
	TEST(stat == Status_Error);
	TEST(err.status == Status_Error);
	TEST(!strcmp(err.source.file, "test_klib.c"));
	TEST(err.source.line == lineno);
	TEST(!strcmp(err.message,  "Error message here"));
	TEST(handler_called);

	Error_set_handler(Status_Error, oldh);
}

TEST_CASE(clear_error_status)
{
	// Given an error handler with an error status
	Error err = {0};
	Error_fail(&err, Status_Error, "Testing clear error", SOURCE_HERE);
	TEST(err.status == Status_Error);

	// Clear the error
	Error_clear(&err);

	TEST(err.status == Status_OK);
	TEST(err.source.file == NULL);
	TEST(err.source.line == 0);
	TEST(err.message == NULL);
	TEST(err.baggage == NULL);
}

TEST_CASE(print_error_handler)
{
	UNUSED(test_counter);

//	ErrorHandler old = Error_set_handler(Status_Error, Error_print);
//	Error err = {0};
//	Error_fail(&err, Status_Error, "testing 1, 2, 3", SOURCE_HERE);
//
//	Error_set_handler(Status_Error, old);
}


StatusCode Error_abort(Error *e)
{
	Error_fprintf(e, stderr, NULL);
	abort();
}

TEST_CASE(abort_error_handler)
{
	UNUSED(test_counter);

//	ErrorHandler old = Error_set_handler(Status_Error, Error_abort);
//	Error err = {0};
//	Error_fail(&err, Status_Error, "aborting in 3..2..1", SOURCE_HERE);

//	Error_set_handler(Status_Error, old);
}


TEST_CASE(error_return_status_handler)
{
	ErrorHandler old = Error_set_handler(Status_Error, Error_status);
	Error err = {0};
	StatusCode stat = Error_fail(&err, Status_Error, "return code", SOURCE_HERE);
	TEST(stat == Status_Error);
	Error_set_handler(Status_Error, old);
}

StatusCode Error_jump(Error *e)
{
	if (e) {
		jmp_buf *buf = e->baggage;
		longjmp(*buf, e->status);
	}
	else {
		return Status_Unknown_Error;
	}
}

TEST_CASE(longjmp_error_handler)
{
	ErrorHandler old = Error_set_handler(Status_Error, Error_status);
	Error err = {0};
	jmp_buf buf;
	err.baggage = &buf;

	switch (setjmp(buf)) {
		case 0:
			Error_fail(&err, Status_Error, "return code", SOURCE_HERE);
			// fallthrough
		case Status_Error:
			printf("Jumped. sizeof buf: %d\n", (int)sizeof(buf));
			break;
		default:
			TEST(false);
	}

	Error_set_handler(Status_Error, old);
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


//-----------------------------------------------------------------------------
// Doubly Linked List
//

TEST_CASE(attach_two_links)
{
	// Given two empty links
	link a = link_init();
	link b = link_init();
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
	link a = link_init();
	link b = link_init();
	link c = link_init();
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
	link a = link_init();
	link b = link_init();
	link c = link_init();
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
	chain chain = chain_init(chain);
	link a = link_init(), 
	     b = link_init(),
	     c = link_init(),
	     d = link_init(),
	     e = link_init(),
	     f = link_init();

	// When all appended to chain
	chain_appends(&chain, &a, &b, &c, &d, &e, &f, NULL);

	// Each attached to the next
	link *l = chain_first(&chain);
	TEST(l == &a);
	TEST((l = link_next(l)) == &b);
	TEST((l = link_next(l)) == &c);
	TEST((l = link_next(l)) == &d);
	TEST((l = link_next(l)) == &e);
	TEST((l = link_next(l)) == &f);
	TEST(l == chain_last(&chain));
}

TEST_CASE(link_foreach)
{
	chain chain = chain_init(chain);

	struct test_node {
		link link;
		int i;
	} a = link_init(.i = 1), 
	  b = link_init(.i = 2),
	  c = link_init(.i = 3),
	  d = link_init(.i = 4),
	  e = link_init(.i = 5),
	  f = link_init(.i = 6);

	// Given chain a:b:c:d:e:f
	chain_appends(&chain, &a, &b, &c, &d, &e, &f, NULL);

	// When we sum all node values with foreach
	int total = 0;
	int i_offset = offsetof(struct test_node, i);
	chain_foreach(&chain, sum_ints, &total, i_offset);

	// Then we get a total
	TEST(total == 21);
}

TEST_CASE(add_links_to_empty_chain)
{
	// Given an empty chain
	chain chain = chain_init(chain);
	TEST(chain_empty(&chain));

	// Append links to chain
	link a = link_init();
	chain_append(&chain, &a);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &chain.head));

	link b = link_init();
	chain_append(&chain, &b);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &b));
	TEST(links_are_attached(&b, &chain.head));

	link c = link_init();
	chain_append(&chain, &c);
	TEST(links_are_attached(&chain.head, &a));
	TEST(links_are_attached(&a, &b));
	TEST(links_are_attached(&b, &c));
	TEST(links_are_attached(&c, &chain.head));
}

TEST_CASE(prepent_links_to_chain)
{
	// Given an empty chain
	chain chain = chain_init(chain);
	TEST(chain_empty(&chain));

	// Prepend links to chain
	link a = link_init();
	chain_prepend(&chain, &a);
	TEST(links_are_attached(&a, &chain.head));
	TEST(links_are_attached(&chain.head, &a));

	link b = link_init();
	chain_prepend(&chain, &b);
	TEST(links_are_attached(&chain.head, &b));
	TEST(links_are_attached(&b, &a));
	TEST(links_are_attached(&a, &chain.head));

	link c = link_init();
	chain_prepend(&chain, &c);
	TEST(links_are_attached(&chain.head, &c));
	TEST(links_are_attached(&c, &b));
	TEST(links_are_attached(&b, &a));
	TEST(links_are_attached(&a, &chain.head));
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


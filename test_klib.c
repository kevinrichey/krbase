#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "klib.h"

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

TEST_CASE(unused_prevents_compiler_warnings)
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


//-----------------------------------------------------------------------------
// Error Handling
//

TEST_CASE(collect_source_debug_info)
{
	SourceInfo source = SOURCE_INFO_INIT;
	int line = __LINE__ - 1; // Get the previous line #

	TEST(!strcmp(source.file, "test_klib.c"));
	TEST(source.line == line);
}

static void source_info_test_fn(TestCounter *test_counter, int line, SourceInfo source)
{
	TEST(!strcmp(source.file, "test_klib.c"));
	TEST(source.line == line);
}

TEST_CASE(pass_source_info_parameter)
{
	int line = __LINE__ + 1;
	source_info_test_fn(test_counter, line, SOURCE_HERE);
}



typedef struct ErrHand ErrHand;
typedef StatusCode (*ErrorHandler)(ErrHand *errh);

typedef struct ErrHand {
	ErrorInfo error;
	ErrorHandler handlers[Status_End];
} ErrHand;


StatusCode 
Error_failure(ErrHand *errh, StatusCode code, const char *message, SourceInfo source)
{
	errh->error.status  = code;
	errh->error.source  = source;
	errh->error.message = message;

	if (code > Status_First && code < Status_End) {
		ErrorHandler handler = errh->handlers[code];
		if (handler)
			return handler(errh);
	}

	return code;
}

bool test_error_handler_called = false;

static StatusCode test_error_handler(ErrHand *errh)
{
	test_error_handler_called = true;
	return errh->error.status;
}

int this_function_fails_line;

static StatusCode this_function_fails(ErrHand *errh)
{
	this_function_fails_line = __LINE__ + 1;
	return Error_failure(errh, Status_Error, "Testing function failure", SOURCE_HERE);
}

TEST_CASE(function_fails)
{
	// Given an error handler
	ErrHand errh = {0};
	errh.handlers[Status_Error] = test_error_handler;

	// When a function returns failure
	StatusCode stat = this_function_fails(&errh);

	// Then error status is set and the handler was called
	TEST(stat == Status_Error);
	TEST(errh.error.status == Status_Error);
	TEST(!strcmp(errh.error.source.file, "test_klib.c"));
	TEST(errh.error.source.line == this_function_fails_line);
	TEST(!strcmp(errh.error.message, "Testing function failure"));
	TEST(test_error_handler_called);
}

void Error_clear(ErrHand *errh)
{
	errh->error = (ErrorInfo){0};
}

TEST_CASE(clear_error_status)
{
	// Given an error handler with an error status
	ErrHand errh = {0};
	Error_failure(&errh, Status_Error, "Testing clear error", SOURCE_HERE);
	TEST(errh.error.status == Status_Error);

	// Clear the error
	Error_clear(&errh);

	TEST(errh.error.status == Status_OK);
	TEST(errh.error.source.file == NULL);
	TEST(errh.error.source.line == 0);
	TEST(errh.error.message == NULL);
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
	TEST_M(List_capacity(l) == 0,  "List has no capacity.");
	TEST_M(List_length(l) == 0,    "List has zero length");
	TEST_M(List_is_empty(l),       "List is empty (length == 0).");
	TEST_M(List_is_full(l),        "List is 'full' (capacity == 0, can't add more elements).");
	TEST_M(!List_in_bounds(l, 0),  "0 is not in range.");

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


TEST_CASE(make_span_from_arrays)
{
	char letters[] = "abcdefghijklmnopqrstuvwxyz";
	Span_t(char) cspan = Span_init(letters);
	TEST(cspan.size == 27); // don't forget the null-terminator
	TEST(cspan.begin[5] == 'f');

	// An array of fibonacci numbers
	Fibonacci fib = Fib_begin();
	int numbers[20];
	int length  = ARRAY_SIZE(numbers);
	for (int *n = numbers; length--; ++n) {
		*n = Fib_get(fib);
		fib = Fib_next(fib);
	}

	// Make a span from the array
	Span_t(int) nspan = Span_init(numbers, 20);
	TEST(nspan.size == 20);
	int i = 0;
	TEST(nspan.begin[i++] ==  0);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  2);
	TEST(nspan.begin[i++] ==  3);
	TEST(nspan.begin[i++] ==  5);
	TEST(nspan.begin[i++] ==  8);
	TEST(nspan.begin[i++] == 13);
	TEST(nspan.begin[i++] == 21);
	TEST(nspan.begin[i++] == 34);
	TEST(nspan.begin[i++] == 55);
	TEST(nspan.begin[i++] == 89);
}


//@module String

//typedef Span_t(char)  String;

typedef struct { char *begin; int size; } String;

static inline bool String_not_empty(String s)
{
	return s.size && s.begin && *s.begin;
}

static inline bool String_is_empty(String s)
{
	return !String_not_empty(s);
}

int String_length(String s)
{
	int length = 0;
	while (s.size-- && *s.begin++)  
		++length;
	return length;
}

#define String_init(...)  Span_init(__VA_ARGS__)

#define STR(...)    (String)String_init(__VA_ARGS__)

String String_create(int size)
{
	return STR(calloc(size, sizeof(char)), size);
}

int vprintf_size(const char *format, va_list args)
{
	va_list args2;
	va_copy(args2, args);
	int size = 1 + vsnprintf(NULL, 0, format, args2);
	va_end(args2);
	return size;
}

String String_create_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int size = vprintf_size(format, args);
	//String s = String_create(size);
	char *s = malloc(size * sizeof(char));
	if (s)
		vsnprintf(s, size, format, args);

	va_end(args);
	return STR(s, size);
}

void String_destroy(String *s)
{
	if (s) {
		free((void*)s->begin);
		*s = (String){0};
	}
}

bool String_equals(String s1, String s2)
{
	return !strncmp(s1.begin, s2.begin, int_min(s1.size, s2.size));
}

TEST_CASE(init_string_to_null)
{
	String s_null = {0};

	TEST(s_null.size == 0);
	TEST(s_null.begin == NULL);
	TEST(String_is_empty(s_null));
}

TEST_CASE(init_string_from_literal)
{
	String s_lit = String_init("xyzzy");

	TEST(s_lit.size == 6);
	TEST(String_length(s_lit) == 5);
	TEST(String_not_empty(s_lit));
}

TEST_CASE(init_string_from_empty_array)
{
	char words[40] = {0};
	String s_arr = String_init(words);
	TEST(s_arr.size == 40);
	TEST(String_is_empty(s_arr));
	TEST(String_length(s_arr) == 0);
}

TEST_CASE(init_string_from_pointer_and_size)
{
	char *ptr = "Hello, world";
	String s_ptr = String_init(ptr, 13);

	TEST(s_ptr.size = 13);
	TEST(String_length(s_ptr) == 12);
	TEST(String_not_empty(s_ptr));
}

TEST_CASE(init_string_from_compound_literal)
{
	String s_acl = String_init((char[15]){ "Foobar"});
	TEST(s_acl.size == 15);
	//TEST(String_is_empty(s_acl));

	//strncpy(s_acl.begin, "Foobar", s_acl.size);
	TEST(String_length(s_acl) == 6);
	TEST(String_not_empty(s_acl));
}

TEST_CASE(create_string_on_heap)
{
	String s = String_create(100);
	TEST(s.size == 100);
	TEST(String_is_empty(s));

	//strncpy(s.begin, "abcdefghijklmnopqrstuvwxyz", s.size);
	//TEST(String_length(s) == 26);
	//TEST(String_not_empty(s));

	String_destroy(&s);
	TEST(s.size  == 0);
	TEST(String_is_empty(s));
	TEST(String_length(s) == 0);
}

TEST_CASE(create_formatted_string_on_heap)
{
	String s = String_create_f("Test %d\n", 123);
	TEST(String_not_empty(s));
	TEST(String_length(s) == 9);
	TEST(s.size == 10);
	TEST(String_equals(s, STR("Test 123\n")));
	
	String_destroy(&s);
}

TEST_CASE(convert_things_to_bytes)
{
	int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	Bytes ns1 = Bytes_init_array(numbers);
	TEST(ns1.size == (10 * sizeof(int)));

	int i = 101;
	Bytes ns2 = Bytes_init_var(i);
	TEST(ns2.size == sizeof(int));

	double d = 3.14159;
	Bytes ns3 = Bytes_init_var(d);
	TEST(ns3.size == sizeof(double));

	char s[100] = "Hello, World";
	Bytes ns4 = Bytes_init_str(s);
	TEST(ns4.size == 12);
}

TEST_CASE(FNV_hash_test)
{
	int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	uint64_t h = hash(Bytes_init_array(numbers));
	TEST(h == 6902647252728264142LLU);

	char s[] = "Hello, World!";
	h = hash(Bytes_init_str(s));
	TEST(h == 7993990320990026836LLU);

	int i = 101;
	h = hash(Bytes_init_var(i));
	TEST(h == 3212644748862486336LLU);

	double d = 3.12159;
	h = hash(Bytes_init_var(d));
	TEST(h == 8148618316659391402LLU);
}

void fill(void *data, int length, void *set, int e_size)
{
	Byte_t *b = data;

	while (length --> 0) {
		memcpy(b, set, e_size);
		b += e_size;
	}
}

#define Array_fill(A_, V_)   \
	fill((A_), ARRAY_SIZE(A_), &(V_), sizeof(V_))

TEST_CASE(hash_table)
{

	typedef struct {
		uint64_t hash;
		char     value;
	} table_record;
	
	table_record data[16];
	int   size = ARRAY_SIZE(data);

	Array_fill(data, ((table_record){ .hash=0, .value=' ' }));

	char s1[] = "Hello";

	uint64_t s1_hash = hash(Bytes_init_str(s1));
	unsigned mask = size - 1;
	int i = mask & s1_hash;
	data[i].hash = s1_hash;
	data[i].value = 'a';

	for (int f = 0; f < size; ++f) {
//		printf("%3d: hash = %llu, value = %c\n", f, data[f].hash, data[f].value);
		if (data[f].hash > 0) {
			TEST(data[f].value == 'a');
		}
	}

}

int timestamp_now(String *out_time)
{
	time_t now = time(NULL);
	struct tm *now_tm = localtime(&now);
	if (now_tm)
		return strftime(out_time->begin, out_time->size, "%Y-%m-%d %H:%M:%S", now_tm);
	else
		return 0;
}

TEST_CASE(log_to_file)
{
	UNUSED(test_counter);

	FILE *log_file = fopen("test.log", "w");

	String stamp = String_init((char[25]){0});
	timestamp_now(&stamp); 
	fprintf(log_file, "%s DEBUG 1 This is a test.\n", stamp.begin);

	fclose(log_file);
}


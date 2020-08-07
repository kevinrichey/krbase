#include "klib.h"
#include <stdlib.h>
#include <stddef.h>

void assertion(bool condition, const char *file, int line, const char *func, const char *msg)
{
	if (!condition) {
		ErrorInfo err = {
			.stat     = Status_Test_Failure,
			.filename = file,
			.fileline = line,
			.funcname = func,
			.message  = msg,
		};
		Error_print(stdout, &err);
	}
}

#define requires(cond_)   assertion((cond_), __FILE__, __LINE__, __func__, #cond_)

TEST_CASE(requires_assertion)
{
	UNUSED(test_counter);
	int x = 0;
	requires(x == 0);
	//requires(x == 1);
}

typedef unsigned char byte;

void print_bytes(byte *b, int size, int cols)
{
	if (!b)
		puts("NULL\n");
	else {
		int c = cols;
		while (size --> 0) {
			printf("%.2X ", *b++);
			if (!--c) {
				puts("");
				c = cols;
			}
		}
		if (c)  puts("");
	}
}

void fill_bytes(byte *b, int size, byte value)
{
	if (!b || !size)  return;
	while (size --> 0)
		*b++ = value;
}


// resizeable array
// dynamic, stretchy, elastic, expanable, list
//  - free (release memory)
//  - add item to end
//  - checked get
//  - length (# elements)
//  - size (current max space)
//  - end (pointer to 1 past end of list)


typedef struct { int size, length; } list_header;

#define list(EL_TYPE)  struct { list_header head; EL_TYPE begin[]; }


void *list_resize_f(list_header *a, int sizeof_base, int sizeof_item, int capacity)
{
	list_header *b = a;
	if ((b = realloc(a, sizeof_base + sizeof_item * capacity))) {
		b->size = capacity;
		if (!a)
			b->length = 0;
		if (b->length > b->size)
			b->length = b->size;
	}
	return b; 
}

#define list_resize(L_, NEW_SIZE)   do{ (L_) = list_resize_f((list_header*)(L_), sizeof(*(L_)), sizeof(*(L_)->begin), (NEW_SIZE)); }while(0)

int list_size(void *list)
{
	return list? ((list_header*)list)->size: 0;
}

int list_length(void *list)
{
	return list? ((list_header*)list)->length: 0;
}

bool list_is_full(void *list)
{
	// assert list -> actual list
	return list && list_length(list) >= list_size(list);
}

bool list_is_empty(void *list)
{
	// assert list is actual list
	return !list || list_length(list) == 0;
}


TEST_CASE(list)
{
	printf("sizeof list(char): %d\n", sizeof(list(char)));
	printf("sizeof list(int): %d\n", sizeof(list(int)));
	printf("sizeof list(double): %d\n", sizeof(list(double)));
	printf("sizeof list(void*): %d\n", sizeof(list(void*)));

	typedef struct { char name[5]; int id; } data;

	list(data) *s = NULL;
	TEST(list_size(s) == 0);     // NULL lists have no size
	TEST(list_length(s) == 0);   // Null lists have no length
	TEST(list_is_empty(s));      // Null lists are empty;
	TEST(!list_is_full(s));      // Null lists are not full;

	int new_size = 3;
	list_resize(s, new_size);
	TEST(list_size(s) == 3);
	TEST(list_length(s) == 0);
	TEST(list_is_empty(s));
	TEST(!list_is_full(s));

	list_header old_head = s->head;
	data x = {"one",101};
	if (s->head.length < s->head.size)
		s->begin[s->head.length++] = x;
	TEST(s->begin[old_head.length].id == 101);
	TEST(s->head.length == old_head.length + 1);
	TEST(!list_is_full(&old_head) || s->head.size > old_head.size);
	TEST(!list_is_empty(s));

	x.id = 202;
	if (s->head.length < s->head.size)
		s->begin[s->head.length++] = x;
	TEST(s->head.length == 2);
	TEST(s->head.size == 3);

	x.id = 303;
	if (s->head.length < s->head.size)
		s->begin[s->head.length++] = x;
	TEST(s->head.length == 3);
	TEST(s->head.size == 3);

	x.id = 404;
	if (list_is_full(s))
		s = list_resize_f((list_header*)s, sizeof(*s), sizeof(*s->begin), s->head.size * 2); 
		// check resize failure...
	s->begin[s->head.length++] = x;
	TEST(s->head.length == 4);
	TEST(s->head.size == 6);

	free(s);
}


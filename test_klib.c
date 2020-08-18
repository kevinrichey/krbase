#include "klib.h"
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

TEST_CASE(vector)
{
	vector(int, x, y, z) point = {10,20,30};

	TEST(point.x == 10);
	TEST(point.y == 20);
	TEST(point.z == 30);
	TEST(point.at[0] == 10);
	TEST(point.at[1] == 20);
	TEST(point.at[2] == 30);
	TEST(vec_length(point) == 3);
}

TEST_CASE(list)
{
	typedef struct { char name[5]; int id; } data;

	list(data) *s = NULL;
	TEST(list_capacity(s) == 0);     // NULL lists have no size
	TEST(list_length(s) == 0);   // Null lists have no length
	TEST(list_is_empty(s));      // Null lists are empty (they have no elements);
	TEST(list_is_full(s));       // Null lists are full (you can't add more elements);

	list_header old_head = { .cap = list_capacity(s), .length = list_length(s) };
	int new_size = 3;
	list_resize(s, new_size);
	TEST(list_capacity(s) == new_size);
	TEST(list_length(s) == old_head.length);
	TEST(list_is_empty(s));
	TEST(!list_is_full(s));

	old_head = s->head;
	data x = {"one",101};
	if (s->head.length < s->head.cap)
		s->begin[s->head.length++] = x;
	TEST(s->begin[old_head.length].id == 101);
	TEST(s->head.length == old_head.length + 1);
	TEST(!list_is_full(&old_head) || s->head.cap > old_head.cap);
	TEST(!list_is_empty(s));

	x.id = 202;
	if (s->head.length < s->head.cap)
		s->begin[s->head.length++] = x;
	TEST(s->head.length == 2);
	TEST(s->head.cap == 3);

	x.id = 303;
	if (s->head.length < s->head.cap)
		s->begin[s->head.length++] = x;
	TEST(s->head.length == 3);
	TEST(s->head.cap == 3);

	x.id = 404;
	Status stat = Status_OK;
	if (list_is_full(s)) {
		void *s2 = list_resize_f((list_header*)s, sizeof(*s), sizeof(*s->begin), list_capacity(s) * 2); 
		if (s2)
			s = s2;
		else
			stat = Status_Alloc_Error;
	}
	s->begin[s->head.length++] = x;
	TEST(s->head.length == 4);
	TEST(s->head.cap == 6);

	list_dispose(s);
}


#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct {
	const char *filename;
	int         fileline;
} debug_info;

#define DEBUG_HERE   (debug_info){ .filename = __FILE__, .fileline = __LINE__ }

typedef struct object_struct {
	struct object_struct *next;
	debug_info  where_created;
} object;

void *object_create(size_t size, object **top, debug_info where)
{
	object *o = malloc(size);
	if (o) {
		o->next     = *top;
		o->where_created = where;
		*top = o;
	}
	return o;
}

#define create(NAME_, TOP_)   object_create(sizeof(*NAME_), &TOP_, DEBUG_HERE)

void object_destroy(object **o, object **top)
{
	object **h;
	for (h = top; *h != *o && *h; h = &(*h)->next) 
		;
	if (*h == *o)
		*h = (*h)->next;
	free(*o);
	*o = NULL;
}

#define destroy(OBJ_, TOP_)   object_destroy((object**)&OBJ_, &TOP_)

void object_print_leaks(object *top)
{
	for ( ; top; top = top->next)
		printf("%s:%d: object leaked.\n", top->where_created.filename, top->where_created.fileline);
}

TEST_CASE(memory_tracker)
{
	typedef struct {
		object base;
		char c;
		double d;
		int i;
	} test_object;

	object *top = NULL;

	object *old_top = top;
	test_object *o1 = create(o1, top);
	TEST(o1 != NULL);
	TEST(o1->base.next == old_top);
	TEST((void*)top == (void*)o1);
	TEST( !strcmp(o1->base.where_created.filename, __FILE__) );
	TEST(o1->base.where_created.fileline == __LINE__-5);

	old_top = top;
	test_object *o2 = create(o2, top);
	TEST(o2 != NULL);
	TEST(o2->base.next == old_top);
	TEST((void*)top == (void*)o2);
	TEST( !strcmp(o2->base.where_created.filename, __FILE__) );
	TEST(o2->base.where_created.fileline == __LINE__-5);

	old_top = top;
	test_object *o3 = create(o3, top);
	TEST(o3 != NULL);
	TEST(o3->base.next == old_top);
	TEST((void*)top == (void*)o3);
	TEST( !strcmp(o3->base.where_created.filename, __FILE__) );
	TEST(o3->base.where_created.fileline == __LINE__-5);

	object_print_leaks(top);

	puts("Delete o2");
	destroy(o2, top);
	TEST(o2 == NULL);
	TEST((void*)top == (void*)o3);
	TEST((void*)top->next == (void*)o1);
	object_print_leaks(top);

	puts("Delete o3");
	destroy(o3, top);
	TEST(o3 == NULL);
	TEST((void*)top == (void*)o1);
	TEST((void*)top->next == NULL);
	object_print_leaks(top);

	puts("Delete o1");
	destroy(o1, top);
	TEST(o1 == NULL);
	TEST(top == NULL);
	object_print_leaks(top);

}




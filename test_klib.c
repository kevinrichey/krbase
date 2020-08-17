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

TEST_CASE(memory_tracker)
{
	typedef struct {
		char c;
		double d;
		int i;
	} test_object;

	typedef struct object_head_struct {
		struct object_head_struct *next;
		const char *filename;
		int fileline;
		size_t size;
	} object_head;

	typedef struct {
		object_head head;
		test_object object;
	} object;


	object_head *top = NULL;

	object_head *old_top = top;
	object *o1 = malloc(sizeof(*o1));
	if (o1) {
		o1->head.next = top;
		top = (object_head*)o1;
	}
	TEST(o1 != NULL);
	TEST(o1->head.next == old_top);
	TEST((void*)top == (void*)o1);


	old_top = top;
	object *o2 = malloc(sizeof(*o2));
	if (o2) {
		o2->head.next = top;
		o2->head.size = sizeof(*o2);
		top = (object_head*)o2;
	}
	TEST(o2 != NULL);
	TEST(o2->head.next == old_top);
	TEST((void*)top == (void*)o2);

	old_top = top;
	object *o3 = malloc(sizeof(*o3));
	if (o3) {
		o3->head.next = top;
		o3->head.size = sizeof(*o3);
		top = (object_head*)o3;
	}
	TEST(o3 != NULL);
	TEST(o3->head.next == old_top);
	TEST((void*)top == (void*)o3);

	object_head *o_ = (object_head*)o2;
	object_head **h;
	for (h = &top; *h != o_ && *h; h = &(*h)->next) 
		;
	if (*h == o_) {
		*h = (*h)->next;
	}
	free(o_);
	TEST((void*)top == (void*)o3);
	TEST((void*)top->next == (void*)o1);


	free(o1);


	free(o3);
}




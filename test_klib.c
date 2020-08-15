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


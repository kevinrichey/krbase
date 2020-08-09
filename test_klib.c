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
	TEST(list_size(s) == 0);     // NULL lists have no size
	TEST(list_length(s) == 0);   // Null lists have no length
	TEST(list_is_empty(s));      // Null lists are empty (they have no elements);
	TEST(list_is_full(s));       // Null lists are full (you can't add more elements);

	list_header old_head = { .size = list_size(s), .length = list_length(s) };
	int new_size = 3;
	list_resize(s, new_size);
	TEST(list_size(s) == new_size);
	TEST(list_length(s) == old_head.length);
	TEST(list_is_empty(s));
	TEST(!list_is_full(s));

	old_head = s->head;
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

	list_dispose(s);
}


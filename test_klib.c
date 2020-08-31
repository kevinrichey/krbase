#include "klib.h"
#include <stddef.h>
#include <string.h>

TEST_CASE(StatusCode_to_string)
{
	TEST( !strcmp(Status_string(Status_First), "OK") );
	TEST( !strcmp(Status_string(Status_OK), "OK") );
	TEST( !strcmp(Status_string(Status_Error), "Error") );
	TEST( !strcmp(Status_string(Status_End), "Unknown Status") );
}

TEST_CASE(Error_declare_literal)
{
	int error_line = __LINE__ + 1;
	ErrorInfo e = {0};
	e = ERROR_LITERAL(Status_Error, "Oops!");

	TEST(e.stat == Status_Error);
	TEST(!strcmp(e.filename, "test_klib.c"));
	TEST(e.fileline == error_line);
	TEST(!strcmp(e.message, "\"Oops!\""));
}

TEST_CASE(Error_init)
{
	int error_line = __LINE__ + 1;
	ErrorInfo e = ERROR_INIT(Status_Error, "Uh-oh!!");

	TEST(e.stat == Status_Error);
	TEST(!strcmp(e.filename, "test_klib.c"));
	TEST(e.fileline == error_line);
	TEST(!strcmp(e.message, "\"Uh-oh!!\""));
}

void Error_init(ErrorStat *err)
{
	err->error = (ErrorInfo){ .stat=Status_OK };
	err->handlers = NULL;
	err->num_handlers = 0;
}

TEST_CASE(ErrorStat_init_default)
{
	ErrorStat err;
	Error_init(&err);
	TEST(err.error.stat == Status_OK);
	TEST(err.handlers == NULL);
	TEST(err.num_handlers == 0);
}

Status Error_handle(ErrorStat *err, ErrorCat category, ErrorInfo *error)
{
	err->error = *error;
	if (err->handlers && (int)category < err->num_handlers) {
		ErrorHandler_fn fn = err->handlers[category].handler;
		void *param        = err->handlers[category].param;
		if (fn)  fn(error, param);
	}

	return error->stat;
}

TEST_CASE(ErrorStat_handle_new_error)
{
	ErrorStat err;
	Error_init(&err);

	Status stat = Error_handle(&err, ErrorCat_Runtime_Error, &ERROR_LITERAL(Status_Error, "Error testing"));

	TEST(stat == Status_Error);
	TEST(err.error.stat == Status_Error);
}

Status Error_report(ErrorInfo *err, void *none)
{
	if (err) {
		FILE *out = none? none: stderr;
		Error_print(out, err);
		return err->stat;
	}
	else
		return Status_OK;
}

TEST_CASE(ErrorStat_handle_report)
{
	ErrorStat err;
	Error_init(&err);

	ErrorHandler handlers[] = {
		[ErrorCat_Runtime_Error] = (ErrorHandler){ .handler = Error_report, .param = stderr }
	};

	err.handlers = handlers;
	err.num_handlers = ARRAY_LENGTH(handlers);

	Status stat = Error_handle(&err, ErrorCat_Runtime_Error, &ERROR_LITERAL(Status_Error, "Test error 2"));

	TEST(stat == Status_Error);
	TEST(err.error.stat == Status_Error);

}

TEST_CASE(requires_assertion)
{
	UNUSED(test_counter);

	int x = 0;

	FILE *out = stderr;
	if ( !(x > 0) ) do {
		Error_print(out, &ERROR_LITERAL(Status_Assert_Failure, "x > 0" ));
	} while(0);
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
	TEST(x.head == (Binode*)&a);
	TEST(x.tail == (Binode*)&f);
	TEST(Binode_prev(x.head) == NULL);
	TEST(Binode_next(x.tail) == NULL);
	TEST(Binodes_are_linked(&a.link, &b.link));
	TEST(Binodes_are_linked(&b.link, &c.link));
	TEST(Binodes_are_linked(&c.link, &d.link));
	TEST(Binodes_are_linked(&d.link, &e.link));
	TEST(Binodes_are_linked(&e.link, &f.link));
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

#define CREATE(TYPE_, TOP_)   object_create(sizeof(TYPE_), &TOP_, DEBUG_HERE)

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

#define DESTROY(OBJ_, TOP_)   object_destroy((object**)&OBJ_, &TOP_)

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
	test_object *o1 = CREATE(*o1, top);
	TEST(o1 != NULL);
	TEST(o1->base.next == old_top);
	TEST((void*)top == (void*)o1);
	TEST( !strcmp(o1->base.where_created.filename, __FILE__) );
	TEST(o1->base.where_created.fileline == __LINE__-5);

	old_top = top;
	test_object *o2 = CREATE(*o2, top);
	TEST(o2 != NULL);
	TEST(o2->base.next == old_top);
	TEST((void*)top == (void*)o2);
	TEST( !strcmp(o2->base.where_created.filename, __FILE__) );
	TEST(o2->base.where_created.fileline == __LINE__-5);

	old_top = top;
	test_object *o3 = CREATE(*o3, top);
	TEST(o3 != NULL);
	TEST(o3->base.next == old_top);
	TEST((void*)top == (void*)o3);
	TEST( !strcmp(o3->base.where_created.filename, __FILE__) );
	TEST(o3->base.where_created.fileline == __LINE__-5);

	//object_print_leaks(top);

	puts("Delete o2");
	DESTROY(o2, top);
	TEST(o2 == NULL);
	TEST((void*)top == (void*)o3);
	TEST((void*)top->next == (void*)o1);

	//object_print_leaks(top);

	puts("Delete o3");
	DESTROY(o3, top);
	TEST(o3 == NULL);
	TEST((void*)top == (void*)o1);
	TEST((void*)top->next == NULL);

	//object_print_leaks(top);

	puts("Delete o1");
	DESTROY(o1, top);
	TEST(o1 == NULL);
	TEST(top == NULL);
	//object_print_leaks(top);

}




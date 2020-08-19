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

typedef struct Unode_struct {
	struct Unode_struct *next;
} Unode;

void Unode_link(Unode *a, Unode *b)
{
	if (a) {
		if (b)
			b->next = a->next;
		a->next = b;
	}
}

_Bool Unode_has_next(Unode *n)
{
	return n && n->next;
}

void *Unode_next(Unode *n)
{
	return n? n->next: NULL;
}

TEST_CASE(Unode_link)
{
	struct {
		Unode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  c = { .q = 'c' },
	  *p;

	TEST(a.link.next == NULL);
	TEST(b.link.next == NULL);
	TEST(c.link.next == NULL);

	// Link two unlinked nodes a and b
	Unode_link(&a.link, &b.link); 
	TEST(a.link.next == &b.link);
	p = Unode_next(&a.link);
	TEST(p && p->q == 'b');
	
	// Link node c between a and b
	Unode_link(&a.link, &c.link);
	p = Unode_next(&a.link);
	TEST(p && p->q == 'c');
	p = Unode_next(&p->link);
	TEST(p && p->q == 'b');

	// Link node a to NULL
	Unode_link(&a.link, NULL);
	p = Unode_next(&a.link);
	TEST(p == NULL);
}

void *Unode_unlink_next(Unode *n)
{
	Unode *p = Unode_next(n);
	if (p) {
		Unode_link(n, Unode_next(p));
		Unode_link(p, NULL);
	}
	return p;
}

void *Unode_unlink_self(Unode *n)
{
	Unode *p = Unode_next(n);
	Unode_link(n, NULL);
	return p;
}

TEST_CASE(Unode_unlink)
{
	struct {
		Unode link;
		char q;
	} a = { .q = 'a' }, 
	  b = { .q = 'b' },
	  c = { .q = 'c' },
	  d = { .q = 'd' },
	  e = { .q = 'e' },
	  f = { .q = 'f' },
	  g = { .q = 'g' },
	  *p;

	Unode_link(&a.link, &b.link);
	Unode_link(&b.link, &c.link);
	Unode_link(&c.link, &d.link);
	Unode_link(&d.link, &e.link);
	Unode_link(&e.link, &f.link);

	// Unlink first node
	p = Unode_unlink_self(&a.link);
	TEST(p->q == 'b');
	TEST(!Unode_has_next(&a.link));

	// Unlink middle node
	p = Unode_unlink_next(&b.link);
	TEST(p->q == 'c');
	TEST(!Unode_has_next(&p->link));
	TEST(Unode_next(&b.link) == &d.link);

	// Unlink last node
	p = Unode_unlink_next(&e.link);
	TEST(p->q == 'f');
	TEST(!Unode_has_next(&e.link));
	TEST(!Unode_has_next(&p->link));

	// Unlink sole node
	p = Unode_unlink_self(&g.link);
	TEST(p == NULL);
	TEST(!Unode_has_next(&g.link));

	p = Unode_unlink_next(&g.link);
	TEST(p == NULL);
	TEST(!Unode_has_next(&g.link));

	// Unlink no node
	p = Unode_unlink_self(NULL);
	TEST(p == NULL);

	p = Unode_unlink_next(NULL);
	TEST(p == NULL);
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




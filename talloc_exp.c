#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "klib.h"

struct object {
	link siblings;
	struct object *down;
	const char *id;
	const char *file;
	int         line;
};

bool object_is_child_of(struct object *node, struct object *parent)
{
	return node && parent && parent->down == node;
}

void object_link_down(struct object *node, struct object *sub)
{
	if (node)  node->down = sub;
	if (sub)   sub->siblings.prev = &node->siblings;
}

#define WATCH(VAR_)      printf(#VAR_ " = %d\n", (int)(VAR_))
#define WATCH_STR(VAR_)  printf(#VAR_ " = %s\n", (char*)(VAR_))

struct string {
	struct object head;
	size_t size;
	char p[];
};

struct person {
	struct object head;
	struct string *name;
	struct string *addy;
	int  age;
};

void print_mem(struct object *h, int indent)
{
	// print self and all siblings
	
	for (; h != NULL; h = (struct object*)h->siblings.next) {
		for (int i = 0; i < indent; ++i)
			putchar('\t');
		printf("%p\t%s\t%s:%d\n", (void*)h, h->id, h->file, h->line);

		if (h->down)
			print_mem(h->down, indent+1);
	}
}

void *tmalloc(struct object *parent, size_t size, const char *id, const char *file, int line)
{
	struct object *h = malloc(size);
	if (h) {
		h->siblings.next = h->siblings.prev = NULL;
		h->down = NULL;
		if (parent) {
			link_attach(&h->siblings, (link*)parent->down);
			object_link_down(parent, h);
		}
		h->id = id;
		h->file = file;
		h->line = line;
	}
	return h;
}

struct string *tmalloc_str(struct object *parent, size_t str_size, const char *id, const char *file, int line)
{
	size_t size = sizeof(struct string) + sizeof(char) * str_size;
	struct string *s = tmalloc(parent, size, id, file, line);
	if (s)
		s->size = str_size;
	return s;
}


#define CREATE(PTR_, PARENT_)  \
	tmalloc((PARENT_), sizeof(*(PTR_)), #PTR_, __FILE__, __LINE__)

#define CREATE_STR(PTR_, LEN_, PARENT_)  \
	tmalloc_str((PARENT_), (LEN_), #PTR_, __FILE__, __LINE__)


void tfree_children(struct object *head)
{
	struct object *child = head->down;
	while (child) {
		struct object *next = (struct object*)child->siblings.next;
		if (child->down)
			tfree_children(child);
		printf("freed %s\n", child->id);
		free(child);
		child = next;
	}
}

void tfree(void *p)
{
	struct object *h = (struct object*)p;

	if (h) {
		tfree_children(h);

		if (object_is_child_of(h, (struct object*)h->siblings.prev)) 
			object_link_down((struct object*)h->siblings.prev, (struct object*)h->siblings.next);
		else
			link_attach(h->siblings.prev, h->siblings.next);

		printf("freed %s\n", h->id);
		free(p);
	}
}

int main(int argc, char *argv[])
{
	WATCH((int)sizeof(struct object));
	WATCH((int)sizeof(struct string));

	struct person *me = CREATE(me, NULL); 
	me->age = 46;
	print_mem((struct object*)me, 0);

	// Add child 
	me->name = CREATE_STR(me->name, 100, &me->head);
	strncpy(me->name->p, "Kevin Richey", me->name->size);
	print_mem((struct object*)me, 0);

	me->addy = CREATE_STR(me->addy, 100, &me->head);
	strncpy(me->addy->p, "660 Independence blvd, Christiansburg VA, 24073", me->addy->size-1);
	print_mem((struct object*)me, 0);

	WATCH(me->age);
	WATCH_STR(me->name->p);
	WATCH_STR(me->addy->p);

//	tfree(me->addy);
//	print_mem((struct header*)me, 0);
//	tfree(me->name);
//	print_mem((struct header*)me, 0);

	tfree(me);

    return 0;
}



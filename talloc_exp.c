#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "krclib.h"

struct object {
	link siblings;
	chain children;
	const char *id;
	const char *file;
	int         line;
};

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
	for (; h != NULL; h = (struct object*)link_next(&h->siblings)) {
		for (int i = 0; i < indent; ++i)
			putchar('\t');
		printf("%p\t%s\t%s:%d\n", (void*)h, h->id, h->file, h->line);

		if (!chain_empty(&h->children))
			print_mem((struct object*)chain_first(&h->children), indent+1);
	}
}

void *tmalloc(struct object *parent, size_t size, const char *id, const char *file, int line)
{
	struct object *h = malloc(size);
	if (h) {
		if (parent)
			chain_append(&parent->children, &h->siblings);
		else
			h->siblings.next = h->siblings.prev = NULL;
		h->children.head.next = h->children.head.prev = &h->children.head;
		h->id   = id;
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
	struct object *child = (struct object*)chain_first(&head->children);
	while (child) {
		struct object *next = (struct object*)link_next(&child->siblings);
		if (!chain_empty(&child->children))
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
		link_remove(&h->siblings);
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
	fprintf(stderr, "name string added\n");

	me->addy = CREATE_STR(me->addy, 100, &me->head);
	fprintf(stderr, "addy string created\n");
	strncpy(me->addy->p, "660 Independence blvd, Christiansburg VA, 24073", me->addy->size-1);
	fprintf(stderr, "addy string copied\n");
	print_mem((struct object*)me, 0);
	fprintf(stderr, "print_mem with addy string\n");

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



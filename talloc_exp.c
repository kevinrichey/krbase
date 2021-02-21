#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct header {
	struct header *back, *next, *down;
	const char *id;
	const char *file;
	int         line;
};

bool header_is_child_of(struct header *node, struct header *parent)
{
	return node && parent && parent->down == node;
}

void header_link(struct header *left, struct header *right)
{
	if (left)   left->next  = right;
	if (right)  right->back = left;
}

void header_link_down(struct header *node, struct header *sub)
{
	if (node)  node->down = sub;
	if (sub)   sub->back  = node;
}

#define WATCH(VAR_)      printf(#VAR_ " = %d\n", (int)(VAR_))
#define WATCH_STR(VAR_)  printf(#VAR_ " = %s\n", (char*)(VAR_))

struct string {
	struct header head;
	size_t size;
	char p[];
};

struct person {
	struct header head;
	struct string *name;
	struct string *addy;
	int  age;
};

void print_mem(struct header *h, int indent)
{
	// print self and all siblings
	
	for (; h != NULL; h = h->next) {
		for (int i = 0; i < indent; ++i)
			putchar('\t');
		printf("%p\t%s\t%s:%d\n", (void*)h, h->id, h->file, h->line);

		if (h->down)
			print_mem(h->down, indent+1);
	}
}

void *tmalloc(struct header *parent, size_t size, const char *id, const char *file, int line)
{
	struct header *h = malloc(size);
	if (h) {
		h->next = h->back = h->down = NULL;
		if (parent) {
			header_link(h, parent->down);
			header_link_down(parent, h);
		}
		h->id = id;
		h->file = file;
		h->line = line;
	}
	return h;
}

struct string *tmalloc_str(struct header *parent, size_t str_size, const char *id, const char *file, int line)
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


void tfree_children(struct header *head)
{
	struct header *child = head->down;
	while (child) {
		struct header *next = child->next;
		if (child->down)
			tfree_children(child);
		printf("freed %s\n", child->id);
		free(child);
		child = next;
	}
}

void tfree(void *p)
{
	struct header *h = (struct header*)p;

	if (h) {
		tfree_children(h);

		if (header_is_child_of(h, h->back)) 
			header_link_down(h->back, h->next);
		else
			header_link(h->back, h->next);

		printf("freed %s\n", h->id);
		free(p);
	}
}

int main(int argc, char *argv[])
{
	WATCH((int)sizeof(struct header));
	WATCH((int)sizeof(struct string));

	struct person *me = CREATE(me, NULL); 
	me->age = 46;
	print_mem((struct header*)me, 0);

	// Add child 
	me->name = CREATE_STR(me->name, 100, &me->head);
	strncpy(me->name->p, "Kevin Richey", me->name->size);
	print_mem((struct header*)me, 0);

	me->addy = CREATE_STR(me->addy, 100, &me->head);
	strncpy(me->addy->p, "660 Independence blvd, Christiansburg VA, 24073", me->addy->size-1);
	print_mem((struct header*)me, 0);

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



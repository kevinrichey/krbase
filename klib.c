#include <stdlib.h>
#include <stdio.h>
#include "klib.h"

const char *Status_string(Status stat)
{
	if (stat < Status_First && stat > Status_Last) 
		return "Unknown Status";

#define X(EnumName)  [Status_##EnumName] = #EnumName,
	return (const char*[]) { STATUS_X_TABLE } [stat];
#undef X
}

void Error_printf(FILE *out, ErrorInfo *e, const char *fmt, ...)
{
	fprintf(out, "%s:%d: %s: %s", 
	        e->filename, 
			e->fileline, 
			Status_string(e->stat), 
			e->message);

	if (fmt && fmt[0]) {
		va_list args;
		va_start(args, fmt);
		vfprintf(out, fmt, args);
		va_end(args);
	}

	putc('\n', out);
}

void Error_print(FILE *out, ErrorInfo *e)
{
	Error_printf(out, e, "");
}

void Test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg)
{
	++counter->test_count;

	if (!test_condition) {
		ErrorInfo err = {
			.stat = Status_Test_Failure,
			.filename = file,
			.fileline = line,
			.funcname = NULL,
			.message  = msg,
		};
		Error_printf(stdout, &err, " in test %s()", counter->test_name);
		counter->failure_count++;
	}
}



void *list_resize_f(list_header *a, int sizeof_base, int sizeof_item, int capacity)
{
	list_header *b = NULL;
	if ( (b = realloc(a, sizeof_base + sizeof_item * capacity)) ) {
		b->cap = capacity;
		if (!a)
			b->length = 0;
		if (b->length > b->cap)
			b->length = b->cap;
	}
	return b; 
}

void list_dispose(void *l)
{
	free(l);
}



void sum_ints(void *total, void *next_i)
{
	*(int*)total += *(int*)next_i;
}



void *Binode_next(Binode *n)
{
	return n? n->right: NULL;
}

void *Binode_prev(Binode *n)
{
	return n? n->left: NULL;
}

_Bool Binode_is_linked(Binode *n)
{
	return n && (n->right || n->left);
}

_Bool Binode_not_linked(Binode *n)
{
	return n && !n->right && !n->left;
}

_Bool Binodes_are_linked(Binode *l, Binode *r)
{
	return l && l->right == r && r && r->left == l;
}

void Binode_link(Binode *a, Binode *b)
{
	if (a)  a->right = b;
	if (b)  b->left  = a;
}

void Binode_insert(Binode *l, Binode *n)
{
	Binode_link(n, Binode_next(l));
	Binode_link(l, n);
}

void Binode_remove(Binode *n)
{
	Binode_link(Binode_prev(n), Binode_next(n));
	Binode_link(n, NULL);
	Binode_link(NULL, n);
}


void *Binode_foreach(Binode *node, closure_fn fn, void *closure, int offset)
{
	for ( ; node != NULL; node = Binode_next(node))
		fn(closure, (byte*)node + offset);
	return closure;
}

Chain Binode_chain_va(void *first, ...)
{
	Chain c = { .head = first, .tail = first };

	va_list args;
	va_start(args, first);
	Binode *n = NULL;
	while ( (n = va_arg(args, Binode*)) ) {
		Binode_link(c.tail, n);
		c.tail = n;
	}
	va_end(args);

	Binode_link(NULL, c.head);
	Binode_link(c.tail, NULL);

	return c;
}


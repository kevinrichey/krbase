#include <stdlib.h>
#include <stdio.h>
#include "klib.h"

const char *Status_string(StatusCode stat)
{
	if (stat < Status_First || stat > Status_Last) 
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
			Status_string(e->status), 
			e->message);

	if (fmt && fmt[0]) {
		va_list args;
		va_start(args, fmt);
		vfprintf(out, fmt, args);
		va_end(args);
	}

	putc('\n', out);
	fflush(out);
}

void Error_print(ErrorInfo *e, FILE *out)
{
	Error_printf(out, e, "");
}

StatusCode Error_fail(ErrorInfo *e)
{
	Error_print(e, stderr);
	fflush(stderr);
	abort();
	return Error_status(e);
}

void Test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg)
{
	++counter->test_count;

	if (!test_condition) {
		ErrorInfo err = {
			.status = Status_Test_Failure,
			.filename = file,
			.fileline = line,
			.message  = msg,
		};
		Error_printf(stdout, &err, " in test %s()", counter->test_name);
		counter->failure_count++;
	}
}

int int_min_n(int n, ...)
{
	va_list args;
	va_start(args, n);

	int min = va_arg(args, int);
	while (--n)
		min = int_min(min, va_arg(args, int));

	va_end(args);
	return min;
}

#define LIST_MIN_CAPACITY 8

void *List_grow(void *l, int sizeof_base, int sizeof_item, int min_cap, int add_length)
{
	CHECK(min_cap || add_length);

	ListDims *b = l;

	int new_length = List_length(l) + add_length;
	min_cap = int_max(min_cap, new_length);

	if (List_capacity(l) < min_cap) {
		min_cap = int_max(min_cap, List_capacity(l) * 2);
		min_cap = int_max(min_cap, LIST_MIN_CAPACITY);
		b = realloc(l, sizeof_base + sizeof_item * min_cap);
		b->cap = min_cap;
	}

	if (b)  b->length = new_length;

	return b;
}

void List_dispose(void *l)
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


#define X(A,B,C)  {A,B,C},
static const int XORSHIFT_PARAM_LIST[][3] = {
	XORSHIFT_PARAMS
};
#undef X

void Xorshift_init(Xorshifter *state, uint32_t seed, int params_num)
{
	params_num = params_num % ARRAY_LENGTH(XORSHIFT_PARAM_LIST);
    const int *params = XORSHIFT_PARAM_LIST[params_num];

    *state = (Xorshifter)
    { 
        .a = params[0], 
        .b = params[1], 
        .c = params[2], 
        .x = seed
    };
}

uint32_t Xorshift_rand(Xorshifter *state)
{
	uint32_t x = state->x;
	x ^= x << state->a;
	x ^= x >> state->b;
	x ^= x << state->c;
	return state->x = x;
}



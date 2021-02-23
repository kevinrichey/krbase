#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "klib.h"

//----------------------------------------------------------------------
// Primitive Utilities

bool in_bounds(int n, int lower, int upper)
{
	return (lower <= n) && (n <= upper);
}

//----------------------------------------------------------------------
// Unit Testing Module

void Test_fail(TestCounter *counter, const char *file, int line, const char *msg)
{
	++counter->test_count;
	printf("%s:%d: Test Failure: %s in test case %s()\n", file, line, msg, counter->test_name);
	++counter->failure_count;
}


//----------------------------------------------------------------------
// Error Module

const char *Status_string(StatusCode stat)
{
	if (!in_enum_bounds(stat, Status))
		return "Unknown Status";

#define X(EnumName)  [Status_##EnumName] = #EnumName,
	return (const char*[]) { STATUS_X_TABLE } [stat];
#undef X
}

StatusCode Error_status(Error *e)
{
	return e ? e->status : Status_Unknown_Error;
}

void Error_fprintf(Error *e, FILE *out, const char *fmt, ...)
{
	if (!e)
		return;

	fprintf(out, "%s:%d: %s: %s", 
	        e->source.file, 
			e->source.line, 
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

StatusCode Error_print(Error *e)
{
	Error_fprintf(e, stderr, NULL);
	return Error_status(e);
}

static ErrorHandler private_error_handlers[Status_End] = { 0 };

ErrorHandler Error_set_handler(StatusCode code, ErrorHandler h)
{
	ErrorHandler old = NULL;
	if (in_array_bounds(code, private_error_handlers)) {
		old = private_error_handlers[code];
		private_error_handlers[code] = h;
	}
	return old;
}

StatusCode Error_fail(Error *err, StatusCode code, const char *message, SourceInfo source)
{
	err->status  = code;
	err->source  = source;
	err->message = message;

	if (in_array_bounds(code, private_error_handlers)) {
		ErrorHandler handler = private_error_handlers[code];
		if (handler)
			code = handler(err);
	}

	return code;
}

void Error_clear(Error *err)
{
    if (err)
        *err = (Error){0};
}





Bytes Bytes_init_str(char *s)
{
	return (Bytes)Span_init((Byte_t*)s, strlen(s));
}



#define LIST_MIN_CAPACITY 8

void *List_grow(void *l, int sizeof_base, int sizeof_item, int min_cap, int add_length)
{
//	CHECK(min_cap || add_length);

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



void *Binode_next(struct link *n)
{
	return n? n->right: NULL;
}

void *Binode_prev(struct link *n)
{
	return n? n->left: NULL;
}

_Bool Binode_is_linked(struct link *n)
{
	return n && (n->right || n->left);
}

_Bool Binode_not_linked(struct link *n)
{
	return n && !n->right && !n->left;
}

_Bool Binodes_are_linked(struct link *l, struct link *r)
{
	return l && l->right == r && r && r->left == l;
}

void Binode_link(struct link *a, struct link *b)
{
	if (a)  a->right = b;
	if (b)  b->left  = a;
}

void Binode_insert(struct link *l, struct link *n)
{
	Binode_link(n, Binode_next(l));
	Binode_link(l, n);
}

void Binode_remove(struct link *n)
{
	Binode_link(Binode_prev(n), Binode_next(n));
	Binode_link(n, NULL);
	Binode_link(NULL, n);
}

void *Binode_foreach(struct link *node, closure_fn fn, void *closure, int offset)
{
	for ( ; node != NULL; node = Binode_next(node))
		fn(closure, (Byte_t*)node + offset);
	return closure;
}

Chain Binode_chain_va(void *first, ...)
{
	Chain c = { .head = first, .tail = first };

	va_list args;
	va_start(args, first);

	struct link *n = NULL;
	while ( (n = va_arg(args, struct link*)) ) {
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
	params_num = params_num % ARRAY_SIZE(XORSHIFT_PARAM_LIST);
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

uint64_t hash_fnv_1a_64bit(Bytes data, uint64_t hash)
{
	const uint64_t fnv_1a_64bit_prime = 0x100000001B3;

	const Byte_t *end = data.begin + data.size;
	for (const Byte_t *b = data.begin; b != end; ++b) {
		hash ^= (uint64_t)*b;
		hash *= fnv_1a_64bit_prime;
	}

	return hash;
}

uint64_t hash(Bytes data)
{
	const uint64_t fnv_1a_64bit_offset_basis = 14695981039346656037llu;
	return hash_fnv_1a_64bit(data, fnv_1a_64bit_offset_basis);
}



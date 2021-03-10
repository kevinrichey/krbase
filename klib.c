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
	return (Bytes)span_init((Byte_t*)s, strlen(s));
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



link *link_next(link *n)
{
	return n ? n->next : NULL;
}

link *link_prev(link *n)
{
	return n ? n->prev : NULL;
}

_Bool link_is_attached(link *n)
{
	return n && (n->next || n->prev);
}

_Bool link_not_attached(link *n)
{
	return n && !n->next && !n->prev;
}

_Bool links_are_attached(link *l, link *r)
{
	return l && l->next == r && r && r->prev == l;
}

link *link_attach(link *a, link *b)
{
	if (a)  a->next = b;
	if (b)  b->prev  = a;
	return b;
}

void link_insert(link *new_link, link *before_this)
{
	link_attach(link_prev(before_this), new_link);
	link_attach(new_link, before_this);
}

void link_append(link *after_this, link *new_link)
{
	link_attach(new_link, link_next(after_this));
	link_attach(after_this, new_link);
}

void link_remove(link *n)
{
	link_attach(link_prev(n), link_next(n));
	n->next = n->prev = NULL;
}

bool chain_empty(const chain * const chain)
{
	return !chain || chain->head.next == &chain->head;
}

link *chain_first(chain *chain)
{
	return chain_empty(chain) ? NULL : chain->head.next;
}

link *chain_last(chain *chain)
{
	return chain_empty(chain) ? NULL : chain->head.prev;
}

void  chain_prepend(chain *c, link *l)
{
	link_append(&c->head, l);
}

void chain_append(chain *c, link *l)
{
	link_insert(l, &c->head);
}

void chain_appends(chain *chain, ...)
{
	va_list args;
	va_start(args, chain);

	link *prev = &chain->head;
	link *next = NULL;
	while ((next = va_arg(args, link*)))
		prev = link_attach(prev, next);
	link_attach(prev, &chain->head);

	va_end(args);
}

void *chain_foreach(chain *chain, closure_fn fn, void *closure, int offset)
{
	for (link *n = chain->head.next; n && n != &chain->head; n = n->next)
		fn(closure, (Byte_t*)n + offset);
	return closure;
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

	const Byte_t *end = data.ptr + data.size;
	for (const Byte_t *b = data.ptr; b != end; ++b) {
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "klib.h"

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
	return (Bytes)SPAN_INIT((Byte_t*)s, strlen(s));
}



#define LIST_MIN_CAPACITY 8

void *List_grow(void *l, int sizeof_base, int sizeof_item, int min_cap, int add_length)
{
//	CHECK(min_cap || add_length);

	ListDims *b = l;

	int new_length = List_length(l) + add_length;
	min_cap = max_i(min_cap, new_length);

	if (List_capacity(l) < min_cap) {
		min_cap = max_i(min_cap, List_capacity(l) * 2);
		min_cap = max_i(min_cap, LIST_MIN_CAPACITY);
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



Link *Link_next(Link *n)
{
	return n ? n->next : NULL;
}

Link *Link_prev(Link *n)
{
	return n ? n->prev : NULL;
}

_Bool Link_is_attached(Link *n)
{
	return n && (n->next || n->prev);
}

_Bool Link_not_attached(Link *n)
{
	return n && !n->next && !n->prev;
}

_Bool Links_are_attached(Link *l, Link *r)
{
	return l && l->next == r && r && r->prev == l;
}

Link *Link_attach(Link *a, Link *b)
{
	if (a)  a->next = b;
	if (b)  b->prev  = a;
	return b;
}

void Link_insert(Link *new_link, Link *before_this)
{
	Link_attach(Link_prev(before_this), new_link);
	Link_attach(new_link, before_this);
}

void Link_append(Link *after_this, Link *new_link)
{
	Link_attach(new_link, Link_next(after_this));
	Link_attach(after_this, new_link);
}

void Link_remove(Link *n)
{
	Link_attach(Link_prev(n), Link_next(n));
	n->next = n->prev = NULL;
}

bool Chain_empty(const Chain * const chain)
{
	return !chain || chain->head.next == &chain->head;
}

Link *Chain_first(Chain *chain)
{
	return Chain_empty(chain) ? NULL : chain->head.next;
}

Link *Chain_last(Chain *chain)
{
	return Chain_empty(chain) ? NULL : chain->head.prev;
}

void  Chain_prepend(Chain *c, Link *l)
{
	Link_append(&c->head, l);
}

void Chain_append(Chain *c, Link *l)
{
	Link_insert(l, &c->head);
}

void Chain_appends(Chain *chain, ...)
{
	va_list args;
	va_start(args, chain);

	Link *prev = &chain->head;
	Link *next = NULL;
	while ((next = va_arg(args, Link*)))
		prev = Link_attach(prev, next);
	Link_attach(prev, &chain->head);

	va_end(args);
}

void *Chain_foreach(Chain *chain, void (*fn)(void*,void*), void *baggage, int offset)
{
	for (Link *n = chain->head.next; n && n != &chain->head; n = n->next)
		fn(baggage, (Byte_t*)n + offset);
	return baggage;
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



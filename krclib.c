#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>

#include "krclib.h"

//----------------------------------------------------------------------
// Primitive Utilities

bool in_bounds(int n, int lower, int upper)
{
	return (lower <= n) && (n <= upper);
}

int check_index(int i, int length)
{
	if (i < 0)  i += length;
	CHECK(i >= 0);
	return i;
}

int max_i(int a, int b)
{
	return (a > b) ? a: b;
}

int min_i(int a, int b)
{
	return (a < b) ? a: b;
}

void cswap(char *a, char *b)
{
	char t = *a;
	*a = *b;
	*b = t;
}


static char *kr_int_to_str_back(int n, char *ps)
{
	int sign = n < 0 ? -1 : 1;

	*ps-- = '\0';
	do *ps-- = sign * (n % 10) + '0';
	while ((n /= 10) != 0);

	if (sign < 0)  *ps-- = '-';

	return ++ps;
}

//----------------------------------------------------------------------
// Error Module

void Assert_failed(SourceInfo source, const char *message)
{
	fprintf(stderr, "%s:%d: %s: %s\n", source.file, source.line, Status_string(Status_Assert_Failure), message); 
	abort();
}

const char *Status_string(StatusCode stat)
{
	if (!in_enum_bounds(stat, Status))
		return "Unknown Status";

#define X(EnumName)  [Status_##EnumName] = #EnumName,
	return (const char*[]) { STATUS_X_TABLE } [stat];
#undef X
}



//----------------------------------------------------------------------
// strand Module

bool strand_is_null(strand s)
{
	return s.front == NULL;
}

bool strand_is_empty(strand s)
{
	return s.front == s.back;
}

int strand_length(strand s)
{
	return s.back - s.front;
}

bool strand_equals(strand a, strand b)
{
	if (strand_length(a) != strand_length(b))
		return false;

	return !strncmp(a.front, b.front, strand_length(a));
}

strand strand_copy(strand from, strbuf out)
{
	char *stop = out.front + out.size - 1;
	while (from.front < from.back && out.back < stop)
		*out.back++ = *from.front++;
	return (strand){ .front = out.front, .back = out.back };
}

strand strand_reverse(strand str, strbuf out)
{
	char *stop = out.front + out.size - 1;

	--str.back;
	while (str.back >= str.front && out.back < stop)
		*out.back++ = *str.back--;

	*out.back = '\0';
	return (strand){ .front = out.front, .back = out.back };
}

strand strand_itoa(int n, strbuf out)
{
	char sbuf[NUM_STR_LEN(int)] = "**********";
	char *istr = kr_int_to_str_back(n, sbuf + ARRAY_SIZE(sbuf)-1);
	return strand_copy((strand){ .front = istr, .back = sbuf+sizeof(sbuf)-1 }, out);
}

void strand_fputs(FILE *out, strand str)
{
	fprintf(out, "%.*s\n", strand_length(str), str.front);
}



//----------------------------------------------------------------------
// string Module

string *string_create(size_t size)
{
	string *s = calloc(sizeof(string) + (sizeof(*s->front) * size), 1);
	if (s) {
		*deconst_size_t(&s->size) = size;
		s->back = s->front;
	}
	return s;
}

string *string_copy(const char *from)
{
	size_t length = strlen(from);
	string *s = string_create(length + 1);
	if (s) {
		strncpy(deconst_char(s->front), from, length+1);
		s->back = s->front + length;
	}
	return s;
}

void string_dispose(string *s)
{
	free(s);
}

size_t string_length(string *s)
{
	return s ? (s->back - s->front) : 0;
}

bool string_equals(string *s, const char *cstr)
{
	if (s && cstr)
		return !strcmp(s->front, cstr);
	else
		return (!s && !cstr);
}

void string_puts(string *s)
{
	puts(s ? s->front : "empty string");
}

bool string_is_empty(string *s) 
{
	return !s  ||  string_length(s) == 0;
}

string *string_format(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	va_list n_args;
	va_copy(n_args, args);
	int length = vsnprintf(NULL, 0, format, n_args) + 1;
	va_end(n_args);

	string *s = string_create(length + 1);
	if (s && s->size > 1) {
		vsnprintf(deconst_char(s->front), s->size, format, args);
		s->back = s->front + length;
	}

	va_end(args);
	return s;
}


bspan Bytes_init_str(char *s)
{
	return (bspan)SPAN_INIT((byte*)s, strlen(s));
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
		fn(baggage, (byte*)n + offset);
	return baggage;
}


//----------------------------------------------------------------------
//@module Logging
//

char *timestamp(char *s, size_t num, struct tm *(*totime)(const time_t*))
{
	time_t now = time(NULL);
	strftime(s, num, "%Y-%m-%d %H:%M:%S %Z", totime(&now));
	return s;
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

uint64_t hash_fnv_1a_64bit(bspan data, uint64_t hash)
{
	const uint64_t fnv_1a_64bit_prime = 0x100000001B3;

	const byte *end = data.back;
	for (const byte *b = data.front; b != end; ++b) {
		hash ^= (uint64_t)*b;
		hash *= fnv_1a_64bit_prime;
	}

	return hash;
}

uint64_t hash(bspan data)
{
	const uint64_t fnv_1a_64bit_offset_basis = 14695981039346656037llu;
	return hash_fnv_1a_64bit(data, fnv_1a_64bit_offset_basis);
}



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <ctype.h>
#include <limits.h>

#include "krclib.h"

//----------------------------------------------------------------------
// Primitive Utilities

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

//static const enum debug_level DEBUG_LEVEL_DEFAULT = DEBUG_LEVEL_LOW;

static enum debug_level ASSERT_VOLUME = DEBUG_LEVEL_MEDIUM;

void debug_set_volume(enum debug_level level)
{
	ASSERT_VOLUME = level;
}

const char *status_string(enum status stat)
{
	static const struct range status_range = {
		.start = STATUS_FIRST, .stop = STATUS_END };

	if (!range_has(status_range, stat))
		return "Unknown Status";

#define X(Enum_, Name_)  [STATUS_##Enum_] = Name_,
	static const char *status_names[] = { KR_STATUS_X_TABLE };
#undef X

	return status_names[stat];
}

void debug_print_abort(FILE *out, enum status status, struct SourceLocation source, const char *format, ...)
{
	va_list args;
	debug_vprint(out, source, format, args);
	va_end(args);
	abort();
}

void assert_failure(
		struct  SourceLocation  source,
		enum    debug_level      level,
		const   char             *msg)
{
	if (level <= ASSERT_VOLUME)
		debug_print_abort(stderr, STATUS_ASSERT_FAILURE, source, msg);
}



void error_fprint(FILE *out, const struct error *error)
{
	if (error)
	{
		debug_print(out, error->source, error->message);
	}
}

void error_fatal(const struct error *error)
{
	error_fprint(stderr, error);
	abort();
}

int check_index(int i, int length, struct SourceLocation dbg)
{
	if (i < -length || i >= length) {
		char buf[100] = "";
		snprintf(buf, 100, "%d is out of bounds [%d..%d)", i, -length, length);
		assert_failure(dbg, DEBUG_LEVEL_MIN, buf);
	}

	return (i < 0) ? (i += length) : i;
}


void except_throw_error(struct except_frame *frame, struct error *error)
{
	if (!frame)
		error_fatal(error);
//		error_fatal(error, "Unhandled exception");

	frame->error = error;
	longjmp(frame->env, (int)error->status);
}

void except_throw(struct except_frame *frame, enum status status, struct SourceLocation source)
{
	struct error *error = malloc(sizeof(struct error));
	if (!error)
		FAILURE(STATUS_MALLOC_FAIL, "Failed to malloc error object.");

	*error = (struct error){
		.source = source,
		.status = status,
	};

	except_throw_error(frame, error);
}

void except_try(struct except_frame *frame, enum status status, struct SourceLocation source)
{
	if (status != STATUS_OK)
		except_throw(frame, status, source);
}

void except_dispose(struct except_frame *frame)
{
	if (frame && frame->error)
	{
		free(frame->error);
		frame->error = NULL;
	}
}

bool size_t_mult_overflows(size_t a, size_t b)
{
	return b != 0 && a > SIZE_MAX / b;
}

bool size_t_add_overflows(size_t a, size_t b)
{
    return a > SIZE_MAX - b;
}

size_t try_size_mult(size_t a, size_t b, struct except_frame *xf, struct SourceLocation loc)
{
    if (size_t_mult_overflows(a, b))
		except_throw(xf, STATUS_MATH_OVERFLOW, loc);

	return a * b;
}

size_t try_size_add(size_t a, size_t b, struct except_frame *xf, struct SourceLocation loc)
{
    if (size_t_add_overflows(a, b))
		except_throw(xf, STATUS_MATH_OVERFLOW, loc);

	return a + b;
}

bool int_mult_overflows(int a, int b)
{
	if (a > 0  &&  b > 0)
		return b > INT_MAX / a;

	else if (a > 0  &&  b < 0)
		return b < INT_MIN / a;

	else if (a < 0  &&  b > 0)
		return a < INT_MIN / b;

	else
		return b < INT_MAX / a;
}

int try_int_mult(int a, int b, struct except_frame *xf, struct SourceLocation loc)
{
	if (int_mult_overflows(a, b))
		except_throw(xf, STATUS_MATH_OVERFLOW, loc);

	return a * b;
}

bool ptrdiff_to_int_overflows(ptrdiff_t d)
{
	return (d > (ptrdiff_t)INT_MAX) || (d < (ptrdiff_t)-INT_MAX);
}

int try_ptrdiff_to_int(ptrdiff_t d, struct except_frame *xf, struct SourceLocation loc)
{
	if (ptrdiff_to_int_overflows(d))
		except_throw(xf, STATUS_MATH_OVERFLOW, loc);

	return (int)d;
}
void *try_malloc(size_t size, struct except_frame *xf, struct SourceLocation source)
{
	void *mem = malloc(size);
	if (!mem)
		except_throw(xf, STATUS_MALLOC_FAIL, source);
	return mem;
}

void *fam_alloc(size_t head_size, size_t elem_size, size_t array_length, struct except_frame *xf)
{
	size_t size = 0;
	size = try_size_mult(elem_size, array_length, xf, CURRENT_LOCATION);
	size = try_size_add(size, head_size, xf, CURRENT_LOCATION);
	return try_malloc(size, xf, CURRENT_LOCATION);
}

//----------------------------------------------------------------------
// strand Module

char *strbuf_end(strbuf buf)
{
	return buf.front + buf.size - 1;
}


bool strand_equals(struct strand a, struct strand b)
{
	if (strand_length(a) != strand_length(b))
		return false;

	return !strncmp(a.front, b.front, strand_length(a));
}

void strand_fputs(FILE *out, struct strand str)
{
	fprintf(out, "%.*s\n", strand_length(str), str.front);
}

struct strand strand_trim_back(struct strand s, int (*istype)(int))
{
	while (s.back > s.front && istype(*(s.back-1))) 
		--s.back;
	return s;
}

struct strand strand_trim_front(struct strand s, int (*istype)(int))
{
	while (s.front < s.back && istype(*s.front)) 
		++s.front;
	return s;
}

struct strand strand_trim(struct strand s, int (*istype)(int))
{
	return strand_trim_back( strand_trim_front(s, istype), istype);
}




struct byte_span Bytes_init_str(char *s)
{
	return (struct byte_span)byte_span_init_n((byte*)s, strlen(s));
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



struct link *link_next(struct link *n)
{
	return n ? n->next : NULL;
}

struct link *link_prev(struct link *n)
{
	return n ? n->prev : NULL;
}

_Bool link_is_attached(struct link *n)
{
	return n && (n->next || n->prev);
}

_Bool link_not_attached(struct link *n)
{
	return n && !n->next && !n->prev;
}

_Bool links_are_attached(struct link *l, struct link *r)
{
	return l && l->next == r && r && r->prev == l;
}

struct link *link_attach(struct link *a, struct link *b)
{
	if (a)  a->next = b;
	if (b)  b->prev  = a;
	return b;
}

void link_insert(struct link *new_link, struct link *before_this)
{
	link_attach(link_prev(before_this), new_link);
	link_attach(new_link, before_this);
}

void link_append(struct link *after_this, struct link *new_link)
{
	link_attach(new_link, link_next(after_this));
	link_attach(after_this, new_link);
}

void link_remove(struct link *n)
{
	link_attach(link_prev(n), link_next(n));
	n->next = n->prev = NULL;
}

bool Chain_empty(const Chain * const chain)
{
	return !chain || chain->head.next == &chain->head;
}

struct link *Chain_first(Chain *chain)
{
	return Chain_empty(chain) ? NULL : chain->head.next;
}

struct link *Chain_last(Chain *chain)
{
	return Chain_empty(chain) ? NULL : chain->head.prev;
}

void  Chain_prepend(Chain *c, struct link *l)
{
	link_append(&c->head, l);
}

void Chain_append(Chain *c, struct link *l)
{
	link_insert(l, &c->head);
}

void Chain_appends(Chain *chain, ...)
{
	va_list args;
	va_start(args, chain);

	struct link *prev = &chain->head;
	struct link *next = NULL;
	while ((next = va_arg(args, struct link*)))
		prev = link_attach(prev, next);
	link_attach(prev, &chain->head);

	va_end(args);
}

void *Chain_foreach(Chain *chain, void (*fn)(void*,void*), void *baggage, int offset)
{
	for (struct link *n = chain->head.next; n && n != &chain->head; n = n->next)
		fn(baggage, (byte*)n + offset);
	return baggage;
}


//----------------------------------------------------------------------
//@module Logging
//

char *timestamp(char *s, size_t num, struct tm *(*totime)(const time_t*))
{
	time_t now = time(NULL);
	strftime(s, num, "%Y-%m-%d %H.%M.%S %Z", totime(&now));
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

uint64_t hash_fnv_1a_64bit(struct byte_span data, uint64_t hash)
{
	const uint64_t fnv_1a_64bit_prime = 0x100000001B3;

	const byte *end = data.back;
	for (const byte *b = data.front; b != end; ++b) {
		hash ^= (uint64_t)*b;
		hash *= fnv_1a_64bit_prime;
	}

	return hash;
}

uint64_t hash(struct byte_span data)
{
	const uint64_t fnv_1a_64bit_offset_basis = 14695981039346656037llu;
	return hash_fnv_1a_64bit(data, fnv_1a_64bit_offset_basis);
}



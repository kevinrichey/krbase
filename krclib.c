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

const char *DebugCategory_string(DebugCategory cat)
{
	if (!in_bounds_enum(cat, DEBUG))
		return "Unknown DebugCategory";

#define X(EnumName)  [DEBUG_##EnumName] = #EnumName,
	return (const char*[]) { KR_DEBUG_CAT_X_TABLE } [cat];
#undef X
}

const char *StatusCode_string(StatusCode stat)
{
	if (!in_bounds_enum(stat, STATUS))
		return "Unknown Status";

#define X(EnumName)  [STATUS_##EnumName] = #EnumName,
	return (const char*[]) { STATUS_X_TABLE } [stat];
#undef X
}

void debug_vfprint(FILE *out, DebugCategory cat, DebugInfo db, const char *message, va_list args)
{
	if (!out)  out = stderr;

	fprintf(out, "%s:%d: %s: ", db.file, db.line, DebugCategory_string(cat));
	if (message)
		vfprintf(out, message, args);
	fputc('\n', out);
}

void debug_print(FILE *out, DebugCategory cat, DebugInfo db, const char *message, ...)
{
	if (!out)  out = stderr;

	fprintf(out, "%s:%d: %s: ", db.file, db.line, DebugCategory_string(cat));
	if (message) {
		va_list args;
		va_start(args, message);
		vfprintf(out, message, args);
		va_end(args);
	}
	fputc('\n', out);
}

int assert_nop(DebugInfo db, const char *s)
{
	UNUSED(db);
	UNUSED(s);
	return false;
}

int assert_exit(DebugInfo db, const char *s)
{
	debug_print(stderr, DEBUG_ASSERT, db, s);
	exit(EXIT_FAILURE);
	return false;
}

static AssertHandler_fp KR_ASSERT_HANDLER = assert_exit;

AssertHandler_fp set_assert_handler(AssertHandler_fp new_handler)
{
	AssertHandler_fp old_handler = KR_ASSERT_HANDLER;
	KR_ASSERT_HANDLER = new_handler;
	return old_handler;
}

void assert_failure(DebugInfo source, const char *s)
{
	KR_ASSERT_HANDLER(source, s);
}

static ExceptFrame *top_frame = NULL;

void except_begin(ExceptFrame *frame)
{
	frame->back = top_frame;
	frame->str  = NULL;

	top_frame = frame;
}

void except_end(void)
{
	free(top_frame->str);
	top_frame = top_frame->back;
}

void except_throw(DebugInfo dbi, char *str)
{
	if (top_frame == NULL) {
		debug_print(stderr, DEBUG_ABORT, dbi, "unhandled exception: %s.\n", str);
		abort();
	}

	top_frame->source = dbi;
	top_frame->str = str;
	longjmp(top_frame->env, 1);
}

void fail(DebugInfo dbg, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	int len = vsnprintf(NULL, 0, message, args) + 1;
	char *str = malloc(len * sizeof(char));
	vsnprintf(str, len, message, args);
//	debug_vfprint(stderr, DEBUG_ERROR, dbg, message, args);
	va_end(args);

	except_throw(dbg, str);
}

int check_index(int i, int length, DebugInfo dbg)
{
	if (i < -length || i >= length) {
		fail(dbg, "index %d out of bounds, length %d", i, length);
	}

	if (i < 0)  i += length;
	return i;
}



//----------------------------------------------------------------------
// strand Module

char *strbuf_end(strbuf buf)
{
	return buf.front + buf.size - 1;
}

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
	char *end = strbuf_end(out);
	while (from.front < from.back && out.back < end)
		*out.back++ = *from.front++;
	return (strand){ .front = out.front, .back = out.back };
}

strand strand_reverse(strand str, strbuf *out)
{
	char *end = strbuf_end(*out);

	--str.back;
	while (str.back >= str.front && out->back < end)
		*out->back++ = *str.back--;

	*out->back = '\0';
	return (strand){ .front = out->front, .back = out->back };
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

strand strand_trim_back(strand s, int (*istype)(int))
{
	while (s.back > s.front && istype(*(s.back-1))) 
		--s.back;
	return s;
}

strand strand_trim_front(strand s, int (*istype)(int))
{
	while (s.front < s.back && istype(*s.front)) 
		++s.front;
	return s;
}

strand strand_trim(strand s, int (*istype)(int))
{
	return strand_trim_back( strand_trim_front(s, istype), istype);
}




byte_span Bytes_init_str(char *s)
{
	return (byte_span)SPAN_INIT((Byte*)s, strlen(s));
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
		fn(baggage, (Byte*)n + offset);
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

uint64_t hash_fnv_1a_64bit(byte_span data, uint64_t hash)
{
	const uint64_t fnv_1a_64bit_prime = 0x100000001B3;

	const Byte *end = data.back;
	for (const Byte *b = data.front; b != end; ++b) {
		hash ^= (uint64_t)*b;
		hash *= fnv_1a_64bit_prime;
	}

	return hash;
}

uint64_t hash(byte_span data)
{
	const uint64_t fnv_1a_64bit_offset_basis = 14695981039346656037llu;
	return hash_fnv_1a_64bit(data, fnv_1a_64bit_offset_basis);
}



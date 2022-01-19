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

const char *debug_cat_string(enum debug_cat cat)
{
	if (!in_bounds_enum(cat, DEBUG))
		return "Unknown DebugCategory";

#define X(Enum_, Name_)  [DBCAT_##Enum_] = Name_,
	return (const char*[]) { KR_DEBUG_CAT_X_TABLE } [cat];
#undef X
}

const char *status_string(enum status stat)
{
	if (!in_bounds_enum(stat, STATUS))
		return "Unknown Status";

#define X(EnumName)  [STATUS_##EnumName] = #EnumName,
	return (const char*[]) { KR_STATUS_X_TABLE } [stat];
#undef X
}


void debug_print(FILE *out, struct debug_info db)
{
	fprintf(out ? out : stderr, "%s:%d: %s(): ", db.file, db.line, db.funcname);
}


void assert_abort(struct debug_info db, const char *s)
{
	debug_print(stderr, db);
	fprintf(stderr, "Assertion failed: '%s'\n", s);
	abort();
}

void assert_exit(struct debug_info db, const char *s)
{
	debug_print(stderr, db);
	fprintf(stderr, "Assertion failed: '%s'\n", s);
	exit(EXIT_FAILURE);
}

static AssertHandler KR_ASSERT_HANDLER = assert_exit;

AssertHandler set_assert_handler(AssertHandler new_handler)
{
	AssertHandler old_handler = KR_ASSERT_HANDLER;
	KR_ASSERT_HANDLER = new_handler;
	return old_handler;
}

void assert_fail(struct debug_info source, const char *s)
{
	if (KR_ASSERT_HANDLER)
		KR_ASSERT_HANDLER(source, s);
}

int check_index(int i, int length, const char *v, struct debug_info dbg)
{
	if (i < -length || i >= length) {
		char buf[100] = "";
		snprintf(buf, 100, "%s = %d is out of bounds [%d..%d)", v, i, -length, length);
		assert_fail(dbg, buf);
	}

	return (i > 0) ? i : (i += length);
}


static struct except_stack {
	struct except_frame *top;
	enum status status;
	struct debug_info source;
} except_stack = { .top=NULL };

static void except_stack_push(struct except_frame *frame)
{
	if (frame)  frame->back = except_stack.top;
	except_stack.top = frame;
}

static struct except_frame *except_stack_pop(void)
{
	struct except_frame *popped = except_stack.top;
	if (popped)  except_stack.top = popped->back;
	return popped;
}

void except_begin(struct except_frame *frame)
{
	except_stack_push(frame);
}

void except_throw(enum status status, struct debug_info dbi)
{
	except_stack.source = dbi;

	struct except_frame *frame = except_stack_pop();
	if (frame == NULL) {
		debug_print(stderr, except_stack.source);
		fprintf(stderr, "%s: %s\n", debug_cat_string(DBCAT_FATAL), "unhandled exception.\n");
		abort();
	}
	longjmp(frame->env, (int)status);
}

void except_end(struct except_frame *frame)
{
	except_stack.top = frame->back;
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



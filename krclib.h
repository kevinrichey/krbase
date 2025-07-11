#ifndef KRCLIB_H_INCLUDED
#define KRCLIB_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <setjmp.h>

#include "krbase.h"

//@library Kevin Richey's C Library

//----------------------------------------------------------------------
//@module Utility Macros

#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_END,  \
  EnumName_##_COUNT = EnumName_##_END,  \
  EnumName_##_LAST  = EnumName_##_END - 1, \
  EnumName_##_FIRST = 0



#define MEMBER_TO_STRUCT_PTR(PTR_, TYPE_, MEMBER_)  \
	(TYPE_*)((byte*)(PTR_) - offsetof(TYPE_, MEMBER_))

#define FAMSIZE(OBJ_, FAM_, LENGTH_)  (sizeof((OBJ_)) + sizeof(*(OBJ_).FAM_) * (LENGTH_))


//----------------------------------------------------------------------
//@module Primitive Utilities

// Type-safe swap
#define KR_SWAP_TEMPLATE(TYPE_, PRE_)  \
	static inline void PRE_##_swap(TYPE_ *a, TYPE_ *b) { \
		TYPE_ c = *a; *a = *b; *b = c; }

KR_SWAP_TEMPLATE(char, ch)
KR_SWAP_TEMPLATE(int, int)
KR_SWAP_TEMPLATE(unsigned, uint)
KR_SWAP_TEMPLATE(double, fl)
KR_SWAP_TEMPLATE(bool, bool)


// Type-safe const casting
#define DEFINE_DECONST_FUNC(TYPE_, PRE_)  \
static inline TYPE_ *PRE_##_deconst(const TYPE_ *i) { return (TYPE_*)i; }

DEFINE_DECONST_FUNC(char, ch)
DEFINE_DECONST_FUNC(int, int)
DEFINE_DECONST_FUNC(long, long)
DEFINE_DECONST_FUNC(unsigned, uint)
DEFINE_DECONST_FUNC(double, fl)
DEFINE_DECONST_FUNC(bool, bool)
DEFINE_DECONST_FUNC(size_t, size_t)

//----------------------------------------------------------------------
//@module Debugging & Error Checking

enum debug_level 
{
	DEBUG_LEVEL_ALWAYS = 0,
	DEBUG_LEVEL_MIN, 

	DEBUG_LEVEL_LOW,
	DEBUG_LEVEL_LOW_1,
	DEBUG_LEVEL_LOW_2,
	DEBUG_LEVEL_LOW_3,
	DEBUG_LEVEL_LOW_4,
	DEBUG_LEVEL_LOW_5,
	DEBUG_LEVEL_LOW_6,
	DEBUG_LEVEL_LOW_7,
	DEBUG_LEVEL_LOW_8,
	DEBUG_LEVEL_LOW_9,

	DEBUG_LEVEL_MEDIUM,
	DEBUG_LEVEL_MEDIUM_1,
	DEBUG_LEVEL_MEDIUM_2,
	DEBUG_LEVEL_MEDIUM_3,
	DEBUG_LEVEL_MEDIUM_4,
	DEBUG_LEVEL_MEDIUM_5,
	DEBUG_LEVEL_MEDIUM_6,
	DEBUG_LEVEL_MEDIUM_7,
	DEBUG_LEVEL_MEDIUM_8,
	DEBUG_LEVEL_MEDIUM_9,

	DEBUG_LEVEL_HIGH,
	DEBUG_LEVEL_HIGH_1,
	DEBUG_LEVEL_HIGH_2,
	DEBUG_LEVEL_HIGH_3,
	DEBUG_LEVEL_HIGH_4,
	DEBUG_LEVEL_HIGH_5,
	DEBUG_LEVEL_HIGH_6,
	DEBUG_LEVEL_HIGH_7,
	DEBUG_LEVEL_HIGH_8,
	DEBUG_LEVEL_HIGH_9,

	DEBUG_LEVEL_MAX,
};

void debug_set_volume(enum debug_level level);


#define KR_STATUS_X_TABLE \
			X(OK,               "OK") \
			X(DEBUG,            "Debug" ) \
			X(WATCH,            "Watch" ) \
			X(ERROR,            "Error") \
			X(FATAL,            "Fatal error") \
			X(ASSERT_FAILURE,   "Assertion failed") \
			X(PRECON_FAIL,      "Precondition failed") \
			X(TEST_FAILURE,     "Test failed") \
            X(MATH_OVERFLOW,    "Arithmetic overflow") \
			X(MALLOC_FAIL,      "Memory allocation failed") \
			X(OUT_OF_SPACE,     "Not enough space to copy data") \
			X(EXCEPTION,        "Exception thrown") 

#define X(EnumName_, _)  STATUS_##EnumName_,
enum status {
    KR_STATUS_X_TABLE
    STANDARD_ENUM_VALUES(STATUS)
};
#undef X

const char *status_string(enum status stat);



#define WATCH_INT(Val_)  debug_print(stderr, STATUS_WATCH, CURRENT_LOCATION, STRINGIFY(Val_) " = %d\n", Val_)

struct error 
{ 
	struct  SourceLocation  source;
	enum    status           status;
	const   char             *message;
};

void error_fprint(FILE *out, const struct error *error);
void error_fatal(const struct error *error);
void assert_failure(struct SourceLocation source, enum debug_level level, const char *msg);

#define PRECON(Condition_, Level_) \
	do{ if (Condition_); else assert_failure(CURRENT_LOCATION, (Level_), #Condition_); } while(0)

#define REQUIRE(Condition_)  PRECON(Condition_, DEBUG_LEVEL_LOW) 

#define FAILURE(Status_, Message_)   \
	error_fatal(&(struct error){ .source=CURRENT_LOCATION, .status=(Status_), .message=(Message_) })

#define CHECK(I_, LEN_)  check_index((I_), (LEN_), CURRENT_LOCATION)
int check_index(int i, int length, struct SourceLocation dbg);


struct except_frame 
{
	volatile struct { jmp_buf env; };
	struct error *error;
};

#define  EXCEPT_BEGIN(Xf_)  setjmp((Xf_).env)
void except_throw_error(struct except_frame *frame, struct error *error);
void except_throw(struct except_frame *frame, enum status status, struct SourceLocation dbi);
void except_try(struct except_frame *frame, enum status status, struct SourceLocation dbi);
void except_dispose(struct except_frame *frame);

#define  EXCEPT_TRY  0

//----------------------------------------------------------------------
// Arithmetic Overflow Safety

bool size_t_mult_overflows(size_t a, size_t b);
bool size_t_add_overflows(size_t a, size_t b);
size_t try_size_mult(size_t a, size_t b, struct except_frame *xf, struct SourceLocation loc);
size_t try_size_add(size_t a, size_t b, struct except_frame *xf, struct SourceLocation loc);

bool   int_mult_overflows(int a, int b);
int    try_int_mult(int a, int b, struct except_frame *xf, struct SourceLocation loc);

bool ptrdiff_to_int_overflows(ptrdiff_t d);
int try_ptrdiff_to_int(ptrdiff_t d, struct except_frame *xf, struct SourceLocation loc);

//----------------------------------------------------------------------
// Memory tools

void *try_malloc(size_t size, struct except_frame *xf, struct SourceLocation source);
void *fam_alloc(size_t head_size, size_t elem_size, size_t array_length, struct except_frame *xf);


//----------------------------------------------------------------------
//@module Vector - tuple with named and random access

#define TVector(TYPE_, ...) \
	union { \
		struct { TYPE_ __VA_ARGS__; }; \
		TYPE_ at[VA_NARGS(__VA_ARGS__)]; \
	}

#define VECT_LENGTH(V_)    (int)(ARRAY_SIZE((V_).at))

//----------------------------------------------------------------------
//@module range

struct range
{
	int start, stop;
};

static inline bool range_has(struct range r, int i)
{
	return (r.start <= i  &&  i < r.stop);
}

//----------------------------------------------------------------------
//@module Span Template

#define SPAN_TEMPLATE(Type_, Name_)  \
	struct Name_ { const Type_* front; const Type_* back; }; \
	static inline struct Name_ CONCAT(Name_,_init_n)(const Type_ a[], int n) { \
		return (struct Name_){ .front = a, .back = a+n }; } \
	static inline int CONCAT(Name_,_length)(struct Name_ span) { \
		return span.front ? (span.back - span.front) : 0; } \
	static inline bool CONCAT(Name_,_is_null)(struct Name_ span) { \
		return !span.front; } \
	static inline bool CONCAT(Name_,_is_empty)(struct Name_ span) { \
		return span.front == span.back; } \
	static inline struct Name_ CONCAT(Name_,_slice)(struct Name_ span, int start, int stop) { \
		int length = CONCAT(Name_,_length)(span);  \
		return (struct Name_){ \
			.front = span.front + CHECK(start, length), \
			.back  = span.front + CHECK(stop,  length) };  }

SPAN_TEMPLATE(char, char_span)
SPAN_TEMPLATE(char, strand)
SPAN_TEMPLATE(int, int_span)
SPAN_TEMPLATE(double, dub_span)
SPAN_TEMPLATE(Byte, byte_span)


//----------------------------------------------------------------------
//@module strand

// Create strand from string literal or fixed-length char array.
#define STR(LIT_)    strand_init_n((LIT_), sizeof(LIT_)-1)

bool   strand_equals(struct strand a, struct strand b);
void   strand_fputs(FILE *out, struct strand str);
struct strand strand_trim_back(struct strand s, int (*istype)(int));
struct strand strand_trim_front(struct strand s, int (*istype)(int));
struct strand strand_trim(struct strand s, int (*istype)(int));


//----------------------------------------------------------------------
//@module strbuf

typedef struct strbuf { 
	size_t size;
	char   *front, *back;
} strbuf;

static inline strbuf strbuf_init(char *buf, size_t size)
{
	return (strbuf){ .size = size, .front = buf, .back = buf };
}

#define STRBUF_INIT(BUF_)  strbuf_init((BUF_), sizeof(BUF_))

static inline int strbuf_length(const struct strbuf *buf)
{
	return buf ?  (buf->back - buf->front) : 0;
}

static inline int strbuf_cap(const struct strbuf *buf)
{
	return buf ? (buf->size - strbuf_length(buf)) : 0;
}

char *strbuf_end(strbuf buf);


//----------------------------------------------------------------------
//@module Chain - Double Linked List

struct link {
	struct link *next, *prev;
};

#define LINK_INIT(...)   {0, __VA_ARGS__ }

struct link *link_next(struct link *n);
struct link *link_prev(struct link *b);
bool         link_is_attached(struct link *n);
bool         link_not_attached(struct link *n);
bool         links_are_attached(struct link *a, struct link *b);
struct link *link_attach(struct link *a, struct link *b);
void         link_insert(struct link *new_link, struct link *before_this);
void         link_append(struct link *after_this, struct link *new_link);
void         link_remove(struct link *n);

typedef struct Chain {
	struct link head;
} Chain;

// Use: 
//      Chain c = CHAIN_INIT(c);
//
#define CHAIN_INIT(C_)   { .head = { .next = &(C_).head, .prev = &(C_).head } }

bool   Chain_empty(const Chain * const chain);
struct link  *Chain_first(Chain *chain);
struct link  *Chain_last(Chain *chain);
void   Chain_prepend(Chain *c, struct link *l);
void   Chain_append(Chain *c, struct link *l);
void   Chain_appends(Chain *chain, ...);
void  *Chain_foreach(Chain *chain, void (*fn)(void*,void*), void *baggage, int offset);


//----------------------------------------------------------------------
//@module Logging
//

#define TIMESTAMP_FORMAT  "YYYY-MM-DD HH.MM.SS TMZ"
#define TIMESTAMP_SIZE    sizeof(TIMESTAMP_FORMAT)

char *timestamp(char *s, size_t num, struct tm *(*totime)(const time_t*));

#define TIMESTAMP_GMT()  timestamp((char[TIMESTAMP_SIZE]){}, TIMESTAMP_SIZE, gmtime)
#define TIMESTAMP_LOC()  timestamp((char[TIMESTAMP_SIZE]){}, TIMESTAMP_SIZE, localtime)












//@module List - Dynamic Resizeable Arrays

typedef struct { int cap, length; } ListDims;

#define LIST(EL_TYPE)  struct { ListDims head; EL_TYPE front[]; }

#define LIST_BASE(L_)  ((ListDims*)L_)

void *List_grow(void *a, int sizeof_base, int sizeof_item, int min_cap, int add_length);

#define LIST_GROW(L_, CAP_, ADD_)     \
	do{ (L_) = List_grow(             \
				(L_),                 \
				sizeof(*(L_)),        \
				sizeof(*(L_)->front), \
				(CAP_),               \
				(ADD_));              \
	}while(0)

#define LIST_ADD(L_, ADD_SIZE_)       \
	LIST_GROW(L_, 0, ADD_SIZE_)

#define LIST_RESERVE(L_, NEW_CAP_)     \
	LIST_GROW(L_, NEW_CAP_, 0)

#define LIST_PUSH(L_, VAL_) \
	do{ \
		LIST_ADD(L_, 1); \
		(L_)->front[(L_)->head.length-1] = (VAL_); \
	}while(0)

static inline int List_capacity(void *l)
{
	return l? ((ListDims*)l)->cap: 0;
}

static inline int List_length(void *l)
{
	return l? ((ListDims*)l)->length: 0;
}

static inline bool List_is_full(void *l)
{
	return List_length(l) >= List_capacity(l);
}

static inline bool List_is_empty(void *l)
{
	return List_length(l) == 0;
}

static inline bool List_in_bounds(void *l, int i)
{
	return l && LIST_BASE(l)->length > i;
}

static inline int List_check(void *l, int i)
{
	if (i < 0)
		i = LIST_BASE(l)->length + i;
//	CHECK(l && List_in_bounds(l, i));
	return i;
}

#define LIST_AT(L_, I_)   ((L_)->front[List_check((L_), (I_))])
#define LIST_LAST(L_)     LIST_AT(L_, -1)

void List_dispose(void *l);





void sum_ints(void *total, void *next_i);


//@module Pseudo-Random Number Generation

#define XORSHIFT_TEMPLATE(X_, A_, B_, C_)  \
			(X_) ^= ((X_) << (A_)),  \
			(X_) ^= ((X_) >> (B_)),  \
			(X_) ^= ((X_) << (C_))


typedef struct Xorshifter_struct {
    int a, b, c;
    uint32_t x;
} Xorshifter;

void Xorshift_init(Xorshifter *state, uint32_t seed, int params_num);
uint32_t Xorshift_rand(Xorshifter *state);


/* A, B, C values
| 1, 3,10| 1, 5,16| 1, 5,19| 1, 9,29| 1,11, 6| 1,11,16| 1,19, 3| 1,21,20| 1,27,27|
| 2, 5,15| 2, 5,21| 2, 7, 7| 2, 7, 9| 2, 7,25| 2, 9,15| 2,15,17| 2,15,25| 2,21, 9|
| 3, 1,14| 3, 3,26| 3, 3,28| 3, 3,29| 3, 5,20| 3, 5,22| 3, 5,25| 3, 7,29| 3,13, 7|
| 3,23,25| 3,25,24| 3,27,11| 4, 3,17| 4, 3,27| 4, 5,15| 5, 3,21| 5, 7,22| 5, 9,7 |
| 5, 9,28| 5, 9,31| 5,13, 6| 5,15,17| 5,17,13| 5,21,12| 5,27, 8| 5,27,21| 5,27,25|
| 5,27,28| 6, 1,11| 6, 3,17| 6,17, 9| 6,21, 7| 6,21,13| 7, 1, 9| 7, 1,18| 7, 1,25|
| 7,13,25| 7,17,21| 7,25,12| 7,25,20| 8, 7,23| 8,9,23 | 9, 5,1 | 9, 5,25| 9,11,19|
| 9,21,16|10, 9,21|10, 9,25|11, 7,12|11, 7,16|11,17,13|11,21,13|12, 9,23|13, 3,17|
|13, 3,27|13, 5,19|13,17,15|14, 1,15|14,13,15|15, 1,29|17,15,20|17,15,23|17,15,26|
*/

#define XORSHIFT_PARAMS \
	X( 1,  3, 10)  \
	X( 1,  5, 16)  \
	X( 1,  5, 19)  \
	X( 1,  9, 29)  \
	X( 1, 11,  6)  \
	X( 1, 11, 16)  \
	X( 1, 19,  3)  \
	X( 1, 21, 20)  \
	X( 1, 27, 27)  \
	X( 2,  5, 15)  \
	X( 2,  5, 21)  \
	X( 2,  7,  7)  \
	X( 2,  7,  9)  \
	X( 2,  7, 25)  \
	X( 2,  9, 15)  \
	X( 2, 15, 17)  \
	X( 2, 15, 25)  \
	X( 2, 21,  9)  \
	X( 3,  1, 14)  \
	X( 3,  3, 26)  \
	X( 3,  3, 28)  \
	X( 3,  3, 29)  \
	X( 3,  5, 20)  \
	X( 3,  5, 22)  \
	X( 3,  5, 25)  \
	X( 3,  7, 29)  \
	X( 3, 13,  7)  \
	X( 3, 23, 25)  \
	X( 3, 25, 24)  \
	X( 3, 27, 11)  \
	X( 4,  3, 17)  \
	X( 4,  3, 27)  \
	X( 4,  5, 15)  \
	X( 5,  3, 21)  \
	X( 5,  7, 22)  \
	X( 5,  9, 7 )  \
	X( 5,  9, 28)  \
	X( 5,  9, 31)  \
	X( 5, 13,  6)  \
	X( 5, 15, 17)  \
	X( 5, 17, 13)  \
	X( 5, 21, 12)  \
	X( 5, 27,  8)  \
	X( 5, 27, 21)  \
	X( 5, 27, 25)  \
	X( 5, 27, 28)  \
	X( 6,  1, 11)  \
	X( 6,  3, 17)  \
	X( 6, 17,  9)  \
	X( 6, 21,  7)  \
	X( 6, 21, 13)  \
	X( 7,  1,  9)  \
	X( 7,  1, 18)  \
	X( 7,  1, 25)  \
	X( 7, 13, 25)  \
	X( 7, 17, 21)  \
	X( 7, 25, 12)  \
	X( 7, 25, 20)  \
	X( 8,  7, 23)  \
	X( 8, 9, 23 )  \
	X( 9,  5, 1 )  \
	X( 9,  5, 25)  \
	X( 9, 11, 19)  \
	X( 9, 21, 16)  \
	X(10,  9, 21)  \
	X(10,  9, 25)  \
	X(11,  7, 12)  \
	X(11,  7, 16)  \
	X(11, 17, 13)  \
	X(11, 21, 13)  \
	X(12,  9, 23)  \
	X(13,  3, 17)  \
	X(13,  3, 27)  \
	X(13,  5, 19)  \
	X(13, 17, 15)  \
	X(14,  1, 15)  \
	X(14, 13, 15)  \
	X(15,  1, 29)  \
	X(17, 15, 20)  \
	X(17, 15, 23)  \
	X(17, 15, 26) 


//@module Hash Table

uint64_t hash_fnv_1a_64bit(struct byte_span data, uint64_t hash);

uint64_t hash(struct byte_span data);

//@module Fibonacci Sequence Iterator

typedef struct Fibonacci_struct {
	int f0, f1;
} Fibonacci;

#define FIB_LITERAL   (Fibonacci){ .f0 = 0, .f1 = 1 }

static inline Fibonacci Fib_begin(void)
{
	return FIB_LITERAL;
}

static inline int Fib_get(Fibonacci fib)
{
	return fib.f0;
}

static inline Fibonacci Fib_next(Fibonacci fib)
{
	return (Fibonacci){ .f0 = fib.f1, .f1 = fib.f0 + fib.f1 };
}

#endif
// vim: ft=c

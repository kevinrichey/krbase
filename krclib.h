#ifndef KRCLIB_H_INCLUDED
#define KRCLIB_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <setjmp.h>

//@library Kevin Richey's C Library

//----------------------------------------------------------------------
//@module Utility Macros

#define NOOP          ((void)0)
#define UNUSED(VAR_)  (void)(VAR_)

#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_END,  \
  EnumName_##_COUNT = EnumName_##_END,  \
  EnumName_##_LAST  = EnumName_##_END - 1, \
  EnumName_##_FIRST = 0

#define CONCAT(A,B)             A##B
#define STRINGIFY(x)            #x
#define STRINGIFY_EXPAND(x)     STRINGIFY(x)

#define LINE_STR                EXPAND_STRINGIFY(__LINE__)

#define VA_NARGS_N(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, PA, PB, PC, PD, PE, PF, PN, ...) PN
#define VA_NARGS(...) VA_NARGS_N(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define VA_PARAM_0(_0, ...)  _0
#define VA_PARAM_1(_0, _1, ...)  _1
#define VA_PARAM_2(_0, _1, _2, ...)  _2
#define VA_PARAM_3(_0, _1, _2, _3, ...)  _3
#define VA_PARAM_4(_0, _1, _2, _3, _4, ...)  _4
#define VA_PARAM_5(_0, _1, _2, _3, _4, _5, ...)  _5
#define VA_PARAM_6(_0, _1, _2, _3, _4, _5, _6, ...)  _6
#define VA_PARAM_7(_0, _1, _2, _3, _4, _5, _6, _7, ...)  _7

#define ARRAY_SIZE(A_)    (sizeof(A_) / sizeof(*(A_)))

#define MEMBER_TO_STRUCT_PTR(PTR_, TYPE_, MEMBER_)  \
	(TYPE_*)((Byte*)(PTR_) - offsetof(TYPE_, MEMBER_))

#define FAMSIZE(OBJ_, FAM_, LENGTH_)  (sizeof((OBJ_)) + sizeof(*(OBJ_).FAM_) * (LENGTH_))
#define NUM_STR_LEN(T_)  (3*sizeof(T_)+2)


//----------------------------------------------------------------------
//@module Primitive Utilities

typedef unsigned char Byte;

// void function pointer
typedef void (*Void_fp)(void);

bool in_bounds(int n, int lower, int upper);

#define in_bounds_enum(VAL_, ENUM_)    in_bounds((VAL_), (ENUM_##_FIRST), (ENUM_##_LAST))
#define in_bounds_array(I_, ARR_)      in_bounds((I_), 0, ARRAY_SIZE(ARR_)-1)


// Type-safe min/max template
#define DEFINE_MAX_FUNC(TYPE_, PRE_) \
	static inline TYPE_ PRE_##_max(TYPE_ a, TYPE_ b) { return a > b ? a : b; }

DEFINE_MAX_FUNC(char, ch)
DEFINE_MAX_FUNC(int, int)
DEFINE_MAX_FUNC(unsigned, uint)
DEFINE_MAX_FUNC(double, fl)
DEFINE_MAX_FUNC(size_t, size_t)


#define DEFINE_MIN_FUNC(TYPE_, PRE_) \
	static inline TYPE_ PRE_##_min(TYPE_ a, TYPE_ b) { return a < b ? a : b; }

DEFINE_MIN_FUNC(char, ch)
DEFINE_MIN_FUNC(int, int)
DEFINE_MIN_FUNC(unsigned, uint)
DEFINE_MIN_FUNC(double, fl)


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


#define KR_DEBUG_CAT_X_TABLE \
	X(ERROR) \
	X(ASSERT) \
	X(WATCH) \
	X(TRACE) \
	X(TEST)  \
	X(ABORT) 
  

#define X(EnumName_)  DEBUG_##EnumName_,
typedef enum {
	KR_DEBUG_CAT_X_TABLE 
    STANDARD_ENUM_VALUES(DEBUG)
} DebugCategory;
#undef X

const char *DebugCategory_string(DebugCategory cat);


typedef struct DebugInfo {
	const char *file;
	int         line;
} DebugInfo;

#define DEBUG_INFO_INIT   { .file = __FILE__, .line = __LINE__ }
#define DEBUG_INFO_HERE   (DebugInfo)DEBUG_INFO_INIT



#define STATUS_X_TABLE \
  X(OK) \
  X(TEST_FAILURE) \
  X(ERROR)  \
  X(ALLOC_ERROR) \
  X(ASSERT_FAILURE) \
  X(UNKNOWN_ERROR) \

#define X(EnumName_)  STATUS_##EnumName_,
typedef enum {
    STATUS_X_TABLE
    STANDARD_ENUM_VALUES(STATUS)
} StatusCode;
#undef X

const char *StatusCode_string(StatusCode stat);

void debug_print(FILE *out, DebugCategory cat, DebugInfo db, const char *message, ...);

typedef int (*AssertHandler_fp)(DebugInfo, const char *);
AssertHandler_fp set_assert_handler(AssertHandler_fp new_handler);

int  assert_nop(DebugInfo db, const char *s);
int  assert_exit(DebugInfo db, const char *s);
void assert_failure(DebugInfo source, const char *s);

#define REQUIRE(CONDITION_)   \
	do{ if (CONDITION_); else assert_failure(DEBUG_INFO_HERE, #CONDITION_); } while(0)


int check_index(int i, int length, DebugInfo dbg);

#define CHECK(I_, LEN_)  check_index((I_), (LEN_), DEBUG_INFO_HERE)

typedef struct ExceptFrame {
	struct ExceptFrame *back;
	jmp_buf env;
	DebugInfo source;
	char *str;
} ExceptFrame;

void except_begin(ExceptFrame *frame);
void except_end(void);

//----------------------------------------------------------------------
//@module Span Template

#define TSPAN(T_)  struct { T_* front; T_* back; }

#define SPAN_INIT_N(PTR_, LEN_, ...)  { .front=(PTR_), .back=(PTR_)+(LEN_) }
#define SPAN_INIT(...)   SPAN_INIT_N(__VA_ARGS__, ARRAY_SIZE(VA_PARAM_0(__VA_ARGS__)))

#define SPAN_LENGTH(SPAN_)    (int)((SPAN_).back - (SPAN_).front)
#define SPAN_IS_EMPTY(SPAN_)  (SPAN_LENGTH(SPAN_) <= 1)
#define SPAN_IS_NULL(SPAN_)   ((SPAN_).front == NULL)

#define SLICE(SPAN_, START_, STOP_)  { \
	.front = (SPAN_).front + CHECK(START_, SPAN_LENGTH(SPAN_)),   \
	.back  = (SPAN_).front + CHECK(STOP_,  SPAN_LENGTH(SPAN_)) }

typedef TSPAN(char)     char_span;
typedef TSPAN(int)      int_span;
typedef TSPAN(double)   dub_span;
typedef TSPAN(void)     void_span;
typedef TSPAN(Byte)     byte_span;
typedef TSPAN(char*)    str_span;


//----------------------------------------------------------------------
//@module Vector - tuple with named and random access

#define TVector(TYPE_, ...) \
	union { \
		struct { TYPE_ __VA_ARGS__; }; \
		TYPE_ at[VA_NARGS(__VA_ARGS__)]; \
	}

#define VECT_LENGTH(V_)    (int)(ARRAY_SIZE((V_).at))

//----------------------------------------------------------------------
//@module strand

typedef struct {
	size_t size;
	char   *front, *back;
} strbuf;

char *strbuf_end(strbuf buf);

static inline strbuf strbuf_init(char buf[], size_t size)
{
	return (strbuf){ .size = size, .front = buf, .back = buf };
}
#define STRBUF_INIT(BUF_)  strbuf_init((BUF_), sizeof(BUF_))


typedef TSPAN(char) strand;

static inline strand strand_init(char *s, int length)
{
	return (strand){ .front = s, .back = s + length };
}

#define STR(LIT_)    strand_init((LIT_), sizeof(LIT_)-1)

bool   strand_is_null(strand s);
bool   strand_is_empty(strand s);
int    strand_length(strand s);
bool   strand_equals(strand a, strand b);
strand strand_copy(strand from, strbuf out);
strand strand_reverse(strand str, strbuf *out);
strand strand_itoa(int n, strbuf out);
void   strand_fputs(FILE *out, strand str);
strand strand_trim_back(strand s, int (*istype)(int));
strand strand_trim_front(strand s, int (*istype)(int));
strand strand_trim(strand s, int (*istype)(int));


//----------------------------------------------------------------------
//@module Chain - Double Linked List

typedef struct Link {
	struct Link *next, *prev;
} Link;

#define LINK_INIT(...)   {0, __VA_ARGS__ }

Link  *Link_next(Link *n);
Link  *Link_prev(Link *b);
bool   Link_is_attached(Link *n);
bool   Link_not_attached(Link *n);
bool   Links_are_attached(Link *a, Link *b);
Link  *Link_attach(Link *a, Link *b);
void   Link_insert(Link *new_link, Link *before_this);
void   Link_append(Link *after_this, Link *new_link);
void   Link_remove(Link *n);

typedef struct Chain {
	Link head;
} Chain;

// Use: 
//      Chain c = CHAIN_INIT(c);
//
#define CHAIN_INIT(C_)   { .head = { .next = &(C_).head, .prev = &(C_).head } }

bool   Chain_empty(const Chain * const chain);
Link  *Chain_first(Chain *chain);
Link  *Chain_last(Chain *chain);
void   Chain_prepend(Chain *c, Link *l);
void   Chain_append(Chain *c, Link *l);
void   Chain_appends(Chain *chain, ...);
void  *Chain_foreach(Chain *chain, void (*fn)(void*,void*), void *baggage, int offset);


//----------------------------------------------------------------------
//@module Logging
//

#define TIMESTAMP_FORMAT  "YYYY-MM-DD HH.MM.SS TMZ"
#define TIMESTAMP_SIZE    sizeof(TIMESTAMP_FORMAT)

char *timestamp(char *s, size_t num, struct tm *(*totime)(const time_t*));

#define TIMESTAMP_GMT  timestamp((char[TIMESTAMP_SIZE]){}, TIMESTAMP_SIZE, gmtime)
#define TIMESTAMP_LOC  timestamp((char[TIMESTAMP_SIZE]){}, TIMESTAMP_SIZE, localtime)












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

uint64_t hash_fnv_1a_64bit(byte_span data, uint64_t hash);

uint64_t hash(byte_span data);

//@module Fibonacci Sequence Iterator

typedef struct Fibonacci_struct {
	int f0, f1;
} Fibonacci;

#define FIB_LITERAL   (Fibonacci){ .f0 = 0, .f1 = 1 }

static inline Fibonacci Fib_begin()
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

#ifndef KLIBC_H_INCLUDED
#define KLIBC_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

//@library klib - Core Library

//@module Utility Macros

#define UNUSED(VAR_)  (void)(VAR_)

#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_End,  \
  EnumName_##_Last  = EnumName_##_End - 1, \
  EnumName_##_First = 0,

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


//@module Unit Testing

typedef struct TestCounter_struct {
	int test_count, failure_count;
	const char *test_name;
} TestCounter;

typedef void (*TestCase_fn)(TestCounter*);

#define TEST_CASE(TEST_NAME_) \
	void TestCase_##TEST_NAME_(TestCounter *test_counter)

void Test_fail(TestCounter *counter, const char *file, int line, const char *msg);

#define TEST(CONDITION_) \
	do { if (CONDITION_); else Test_fail(test_counter, __FILE__, __LINE__, #CONDITION_); } while(0)






static inline int int_max(int a, int b)
{
	return (a > b)? a: b;
}

static inline int int_min(int a, int b)
{
	return (a < b)? a: b;
}




//@module Debugging & Error Checking

typedef struct SourceInfo_struct {
	const char *file;
	int line;
} SourceInfo;

#define SOURCE_INFO_INIT   { .file = __FILE__, .line = __LINE__ }
#define SOURCE_HERE        (SourceInfo)SOURCE_INFO_INIT   

#define STATUS_X_TABLE \
  X(OK) \
  X(Test_Failure) \
  X(Error)  \
  X(Alloc_Error) \
  X(Assert_Failure) \
  X(Unknown_Error) \

#define X(EnumName_)  Status_##EnumName_,
typedef enum {
    STATUS_X_TABLE
    STANDARD_ENUM_VALUES(Status)
} StatusCode;
#undef X

const char *Status_string(StatusCode stat);

typedef struct Error_struct {
	StatusCode     status;
	SourceInfo     source;
	const char    *filename; // delete
	int            fileline; // delete
	const char    *message;
} ErrorInfo;

#define ERROR_INIT(STAT_, MSG_) {  \
			.status   = (STAT_),   \
			.filename = __FILE__,  \
			.fileline = __LINE__,  \
			.message  = (MSG_) }

#define ERROR_LITERAL(STAT_, MSG_)  \
			(ErrorInfo)ERROR_INIT(STAT_, MSG_)

#define ERROR(ERR_, STAT_, MSG_) \
			((ERR_) = ERROR_LITERAL((STAT_), (MSG_))).status;

static inline StatusCode Error_status(ErrorInfo *e)
{
	return e? e->status: Status_Unknown_Error;
}

void Error_print(ErrorInfo *e, FILE *out);

StatusCode Error_fail(ErrorInfo *e);

#define CHECK_HERE(ASSERTION_, FILE_, LINE_, ...) \
	do{ if(!(ASSERTION_)) \
		Error_fail(&ERROR_LITERAL(Status_Assert_Failure, #ASSERTION_)); \
	}while(0) \

#define CHECK(...) \
  CHECK_HERE(__VA_ARGS__, __FILE__, __LINE__)



//@module Vector - tuple with named and random access

#define VECTOR(TYPE_, ...)   \
  union { \
      struct { TYPE_ __VA_ARGS__; }; \
      TYPE_ at[VA_NARGS(__VA_ARGS__)]; \
  }

#define VECT_LENGTH(V_)    (int)(ARRAY_SIZE((V_).at))


//@module Span

#define Span_t(T_)        struct { T_ *begin; int size; }

#define Span_init_n(PTR_, LEN_, ...) \
			{ .begin = (PTR_), .size = (LEN_) }

#define Span_init(...) \
			Span_init_n(__VA_ARGS__, ARRAY_SIZE(VA_PARAM_0(__VA_ARGS__)))



//@module Bytes

typedef unsigned char Byte_t;

typedef Span_t(Byte_t)  Bytes;

#define Bytes_init_array(ARR_)  \
			(Bytes)Span_init((Byte_t*)(ARR_), sizeof(ARR_))

#define Bytes_init_var(VAR_)   \
			(Bytes)Span_init((Byte_t*)&(VAR_), sizeof(VAR_))

Bytes Bytes_init_str(char *s);



//@module List - Dynamic Resizeable Arrays

typedef struct { int cap, length; } ListDims;

#define LIST(EL_TYPE)  struct { ListDims head; EL_TYPE begin[]; }

#define LIST_BASE(L_)  ((ListDims*)L_)

void *List_grow(void *a, int sizeof_base, int sizeof_item, int min_cap, int add_length);

#define LIST_GROW(L_, CAP_, ADD_)     \
	do{ (L_) = List_grow(             \
				(L_),                 \
				sizeof(*(L_)),        \
				sizeof(*(L_)->begin), \
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
		(L_)->begin[(L_)->head.length-1] = (VAL_); \
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
	CHECK(l && List_in_bounds(l, i));
	return i;
}

#define LIST_AT(L_, I_)   ((L_)->begin[List_check((L_), (I_))])
#define LIST_LAST(L_)     LIST_AT(L_, -1)

void List_dispose(void *l);




typedef void (*closure_fn)(void*, void*);

void sum_ints(void *total, void *next_i);


//@module Binode - Double linked list

typedef struct Binode_struct {
	struct Binode_struct *right, *left;
} Binode;

void *Binode_next(Binode *n);
void *Binode_prev(Binode *b);
_Bool Binode_is_linked(Binode *n);
_Bool Binode_not_linked(Binode *n);
_Bool Binodes_are_linked(Binode *a, Binode *b);

void Binode_link(Binode *a, Binode *b);
void Binode_insert(Binode *l, Binode *n);
void Binode_remove(Binode *n);
void *Binode_foreach(Binode *node, closure_fn fn, void *closure, int offset);

typedef struct {
	Binode *head, *tail;
} Chain;

Chain Binode_chain_va(void *first, ...);

#define BINODE_CHAIN(...)   Binode_chain_va(__VA_ARGS__, NULL)

static inline Binode *Chain_head(Chain *c) 
{
	return c? c->head: NULL;
}

static inline Binode *Chain_tail(Chain *c) 
{
	return c? c->tail: NULL;
}

//@module Pseudo-Random Number Generation

#define XORSHIFT_TEMPLATE(X_, A_, B_, C_)  \
			X_ ^= (X_ << A_),  \
			X_ ^= (X_ >> B_),  \
			X_ ^= (X_ << C_)


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

uint64_t hash_fnv_1a_64bit(Bytes data, uint64_t hash);

uint64_t hash(Bytes data);

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

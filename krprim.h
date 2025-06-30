#ifndef KR_KRPRIM_H_INCLUDED
#define KR_KRPRIM_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <setjmp.h>

//----------------------------------------------------------------------
// Primitive Types

typedef unsigned char      Byte;
typedef uint8_t            Octet;
typedef uint8_t            Utf8;
typedef ptrdiff_t          Size;
typedef void               (*VoidFP)(void);


//----------------------------------------------------------------------
// Macros

#define NOOP                        ((void)0)
#define UNUSED(VAR_)                (void)(VAR_)
#define CONCAT(A,B)                 A##B
#define STRINGIFY(TOKEN_)           #TOKEN_
#define STRINGIFY_EXPAND(TOKEN_)    STRINGIFY(TOKEN_)
#define NUM_STR_LEN(T_)             (Size)(3*sizeof(T_)+2)

// Variable Argument Macro Support

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
#define VA_PARAM_TAIL(_0, ...)  __VA_ARGS__

#define sizeof(A_)         (Size)sizeof(A_)
#define lengthof(A_)       (Size)(sizeof(A_) / sizeof(*(A_)))

//----------------------------------------------------------------------
// Debugging & Assertions

// Tiny C's backtrace printer.
int tcc_backtrace(const char *fmt, ...); 

#define assertf(C_, ...)  while(!(C_)) { tcc_backtrace(__VA_ARGS__); abort(); }
#define assert(C_)        assertf(C_, "Assert Failed: " STRINGIFY_EXPAND(C_))
#define crash(...)        assertf(false, __VA_ARGS__)

int check(int len, int i);


typedef struct {
	const char *file_name;
	int         line_num;
} SourceLine;

#define SRC_HERE  (SourceLine){ .file_name=__FILE__, .line_num=__LINE__ }

void debug_vprint(FILE *out, SourceLine source, const char *format, va_list args);
void debug_print (FILE *out, SourceLine source, const char *format, ...);   


//----------------------------------------------------------------------
// String

typedef struct { 
	const Utf8 *data;
	int         length; 
} String;

#define Str(A_)   (String){ .data=(const Utf8*)(A_), .length=lengthof(A_)-1 }

String String_init(const char *c_str);
bool   String_is_empty(String s);
bool   String_equals(String a, String b);
void   String_fprint(String s, FILE *out);
String String_slice(String span, int first, int last); 
String String_first(String s, int n);
String String_last(String s, int n);

// String function abbreviations
#define Str_print(S_)  String_fprint((S_), stdout )
#define Str_empty(S_)  String_is_empty((S_))
#define Str_eq         String_equals
#define Str_slice      String_slice
#define Str_first      String_first
#define Str_last       String_last


//----------------------------------------------------------------------
// Error Handling 

#define STATUS_X_TABLE \
	X(OK,                    "OK") \
	X(ERROR,                 "Error")  \
	X(OUT_OF_MEM,            "Out of memory") 

#define X(NAME_, _)   CONCAT(STAT_, NAME_), 
typedef enum {
	STATUS_X_TABLE 
	STAT_END, 
	STAT_FIRST = 0
} Status;
#undef X

const char *Status_cstr(Status stat);
String Status_string(Status stat);

typedef struct {
	volatile struct { jmp_buf env; };
	Status          status;
	String          message;
	SourceLine      location;
} Exception;

#define  EX_BEGIN(Xf_)  setjmp((Xf_).env)
#define  EX_TRY  0

void throw(Exception *e, Status status, String message, SourceLine loc); 

//----------------------------------------------------------------------
// Numbers

int   int_min(int a, int b);      
int   int_max(int a, int b);      
bool  in_range(int n, int low, int hi);
int   int_swap(int *a, int *b);

bool  Double_equals(double a, double b, double epsilon);

#define  num_eq   Double_equals



//----------------------------------------------------------------------
// Pointer Stuff

void *ptr_and(void *p, void *alt);

//=============================================================================
// Compound Types

//----------------------------------------------------------------------
// Span

#define SPAN(T_)  struct { T_* data; Size length; }
typedef SPAN(double) nuspan;
#define $N(...)   (nuspan){ .data=( (double[]){__VA_ARGS__}), .length=VA_NARGS(__VA_ARGS__) };
nuspan num_slice(nuspan span, int first, int last); 

typedef SPAN(int)   Ispan;

typedef SPAN(void)  Vspan;
#define Vspan_cast(VS_, TO_)   (TO_){ VS_.data, VS_.length }

//----------------------------------------------------------------------
// Vector

#define VECTOR(...)   union { struct { double __VA_ARGS__; }; double v[VA_NARGS(__VA_ARGS__)]; }

#define VEC_LENGTH(V_)    (lengthof( (V_).v ))

typedef VECTOR(x,y)       VecXY;
typedef VECTOR(x,y,z)     VecXYZ;
typedef VECTOR(r,g,b)     VecRGB;
typedef VECTOR(row,col)   VecRowCol;

//----------------------------------------------------------------------
// Interval

//#define INTERVAL(T_) struct { T_ left, right; }

//typedef INTERVAL(double) Interval;

typedef VECTOR(left,right) Interval;

Interval interval(double left, double right);

bool   includes (Interval invl, double n);
double clamp    (Interval invl, double n);
double lerp     (Interval invl, double t);
void Interval_fprint (FILE *out, Interval invl); 

// Common intervals on the real number line. 
extern const Interval R_positive;
extern const Interval R_negative;
extern const Interval R_all;
extern const Interval R_zero;
extern const Interval R_unit;
extern const Interval R_empty;

//======================================================================
// Memory management

//-----------------------------------------------------------------------------
// Arenas
//
// Derived from skeeto's arena: https://nullprogram.com/blog/2023/09/27/

typedef struct Arena_struct {
	Byte *beg, *end;
	Exception *ex;
} Arena;

enum {
	ARENA_NO_FILL      = -1, 
	ARENA_FILL_ZERO    = 0, 
	ARENA_FILL_DEBUG   = 0xA
};

Size Arena_cap(Arena a);
Vspan alloc(Arena *arena, Size unit, Size align, Size count, int fill, SourceLine loc); 
#define new(A_, T_, N_)   (T_*)alloc(A_, sizeof(T_), _Alignof(T_), N_, ARENA_FILL_ZERO, SRC_HERE)

#endif

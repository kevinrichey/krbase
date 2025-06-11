#ifndef KR_KRBASE_H_INCLUDED
#define KR_KRBASE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>

#define NOOP                        ((void)0)
#define UNUSED(VAR_)                (void)(VAR_)
#define CONCAT(A,B)                 A##B
#define STRINGIFY(TOKEN_)           #TOKEN_
#define STRINGIFY_EXPAND(TOKEN_)    STRINGIFY(TOKEN_)
#define arraylen(A_)                (Size)(sizeof(A_) / sizeof(*(A_)))
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

//----------------------------------------------------------------------
// Custom Types

typedef char      Byte;
typedef uint8_t   Octet;
typedef ptrdiff_t Size;
typedef void      (*VoidFn)(void);

#define SPAN(T_)  struct { T_* data; int length; }

typedef SPAN(const char) String;
#define s(A_)   (String){ .data=(A_), .length=arraylen(A_)-1 }
#define $s(A_)   (String){ .data=(A_), .length=arraylen(A_)-1 }
bool String_equals(String a, String b);
void String_fprint(String s, FILE *out);
#define String_print(S_)  String_fprint((S_), stdout )

int int_swap(int *a, int *b);

//----------------------------------------------------------------------
// Debugging

typedef struct {
	String      file_name;
	int         line_num;
} SourceLine;

#define SRC_HERE  (SourceLine){ .file_name=$s(__FILE__), .line_num=__LINE__ }


void debug_vprint(FILE *out, SourceLine source, const char *format, va_list args);
void debug_print (FILE *out, SourceLine source, const char *format, ...);   
bool crash(SourceLine source, const char* format, ...);

//----------------------------------------------------------------------
// Assertions

int tcc_backtrace(const char *fmt, ...); 
#define precond(C_, ...)  while(!(C_)) { tcc_backtrace("Precondition Failed: " __VA_ARGS__); abort(); }
#define require(C_)       precond(C_, STRINGIFY_EXPAND(C_))
int check(int len, int i);

//----------------------------------------------------------------------
// Math Stuff

int int_min (int a, int b);      
int int_max (int a, int b);      
bool in_range (int n, int low, int hi);
bool feq (double a, double b, double epsilon);

//----------------------------------------------------------------------
// Pointer Stuff

void *ptr_and(void *p, void *alt);

//=============================================================================
// Compound Types

//----------------------------------------------------------------------
// Span

String str_slice(String span, int first, int last); 
String str_first(String s, int n);
String str_last(String s, int n);

typedef SPAN(double) nuspan;
#define $N(...)   (nuspan){ .data=( (double[]){__VA_ARGS__}), .length=VA_NARGS(__VA_ARGS__) };
nuspan num_slice(nuspan span, int first, int last); 

//----------------------------------------------------------------------
// Interval

struct Interval { double left, right; };

struct Interval interval (double left, double right);
bool   includes (struct Interval invl, double n);
double clamp    (struct Interval invl, double n);
double lerp     (struct Interval invl, double t);
void Interval_fprint (FILE *out, struct Interval invl); 

// Common intervals on the real number line. 
extern const struct Interval R_positive;
extern const struct Interval R_negative;
extern const struct Interval R_all;
extern const struct Interval R_zero;
extern const struct Interval R_unit;
extern const struct Interval R_empty;

//----------------------------------------------------------------------
// Vector

#define VECTOR(...)   union { struct { double __VA_ARGS__; }; double v[VA_NARGS(__VA_ARGS__)]; }

#define VEC_LENGTH(V_)    (int)(arraylen( (V_).v ))

typedef VECTOR(x,y)       VectorXY;
typedef VECTOR(x,y,z)     VectorXYZ;
typedef VECTOR(r,g,b)     VectorRGB;
typedef VECTOR(row,col)   VectorRowCol;

#endif

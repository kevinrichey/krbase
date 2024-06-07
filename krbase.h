#ifndef KR_KRBASE_H_INCLUDED
#define KR_KRBASE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define NOOP                        ((void)0)
#define UNUSED(VAR_)                (void)(VAR_)
#define CONCAT(A,B)                 A##B
#define STRINGIFY(TOKEN_)           #TOKEN_
#define STRINGIFY_EXPAND(TOKEN_)    STRINGIFY(TOKEN_)
#define ARRAY_LENGTH(A_)            (sizeof(A_) / sizeof(*(A_)))
#define NUM_STR_LEN(T_)             (3*sizeof(T_)+2)

#define VA_NARGS_N(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, PA, PB, PC, PD, PE, PF, PN, ...) PN
#define VA_NARGS(...) VA_NARGS_N(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

//----------------------------------------------------------------------
// Custom Types

typedef unsigned char byte;
typedef void (*voidfn)(void);

//----------------------------------------------------------------------
// Debugging

struct SourceLocation 
{
	const char *file_name;
	unsigned    line_num;
};

#define SRCLOC  (struct SourceLocation){ .file_name=__FILE__, .line_num=__LINE__ }
#define CURRENT_LOCATION   SRCLOC


void debug_vprint(FILE* out, struct SourceLocation source, const char* format, va_list args);
void debug_print (FILE* out, struct SourceLocation source, const char* format, ...);   
bool debug_abort (void* out, struct SourceLocation source, const char* format, ...);

//----------------------------------------------------------------------
// Math Stuff

int int_min (int a, int b);      
int int_max (int a, int b);      
bool in_range (int n, int low, int hi);
bool feq (double a, double b, double epsilon);

//----------------------------------------------------------------------
// Pointer Stuff

void*       ptr_and(void *p, void *alt);
const void* const_ptr_and(const void *p, const void *d);

//----------------------------------------------------------------------
// Assertions

typedef bool (*FailFn)(void*, struct SourceLocation, const char*, ...);
struct AssertHandler
{
	FailFn fail;
	void  *bag;                                   
	struct AssertHandler *back;                  
};
void AssertHandler_push(struct AssertHandler *handler);
void AssertHandler_pop(void);
bool fail(const char *m, struct SourceLocation loc);
#define ASSERTION(T_)   ((T_)? true: fail("ASSERT Failed: " STRINGIFY_EXPAND(T_), SRCLOC))
bool assert_equal(const char* an, int av, const char* bn, int bv, struct SourceLocation loc);
#define ASSERT_INT_EQ(A_, B_) assert_equal(#A_, (A_), #B_, (B_), CURRENT_LOCATION)
bool assert_streq(const char* a, const char* b, struct SourceLocation loc);
int check_index(int len, int i, struct SourceLocation source);
#define CHECK(S_, I_)  check_index((S_).length, (I_), SRCLOC)

//=============================================================================
// Compound Types

//----------------------------------------------------------------------
// Span

#define SPAN(T_)  struct { T_* data; int length; }
#define SPAN_INIT(A_)   { .data=(A_), .length=ARRAY_LENGTH(A_) }

typedef SPAN(const char) strand;
#define $(A_)   (strand){ .data=(A_), .length=ARRAY_LENGTH(A_)-1 }
strand str_slice(strand span, int first, int last); 

typedef SPAN(double)  nuspan;
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

#define VEC_LENGTH(V_)    (int)(ARRAY_LENGTH( (V_).v ))

typedef VECTOR(x,y)       VectorXY;
typedef VECTOR(x,y,z)     VectorXYZ;
typedef VECTOR(r,g,b)     VectorRGB;
typedef VECTOR(row,col)   VectorRowCol;

#endif

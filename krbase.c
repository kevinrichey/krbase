#include "krbase.h"
#include <math.h>
#include <float.h>

//----------------------------------------------------------------------
// Debugging

void debug_vprint(FILE *out, struct SourceLocation source, const char *format, va_list args)
{
	out = ptr_and(out, stderr);
	fprintf(out, "%s:%d: ", source.file_name, source.line_num);
	if (format)
		vfprintf(out, format, args);
	fputc('\n', out);
}

void debug_print(FILE *out, struct SourceLocation source, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	debug_vprint(out, source, format, args);
	va_end(args);
}

bool debug_abort(void* out, struct SourceLocation source, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	debug_vprint(out, source, format, args);
	va_end(args);

	abort();
	return false; // To satisfy compiler warnings. 
}
//----------------------------------------------------------------------
// Math Stuff

int int_min(int a, int b)  { return (a < b) ? a : b; }
int int_max(int a, int b)  { return (a > b) ? a : b; }

bool in_range(int n, int low, int hi)
{
	return (low <= n) && (n <= hi);
}

bool feq(double a, double b, double epsilon)
{
	return fabs(a - b) <= epsilon;
}

//----------------------------------------------------------------------
// Pointer Stuff

void *ptr_and(void *p, void *alt)
{
	return p ? p : alt;
}

const void *const_ptr_and(const void *p, const void *d)
{
	return p ? p : d;
}

//----------------------------------------------------------------------
// Assertions

struct AssertHandler default_assert_handler = { .fail = debug_abort };
struct AssertHandler *handler_top = &default_assert_handler;
void AssertHandler_push(struct AssertHandler *handler)
{
	handler->back = handler_top;
	handler_top = handler;
}
void AssertHandler_pop(void)
{
	if (handler_top)
		handler_top = handler_top->back;
	if (!handler_top)
		handler_top = &default_assert_handler;
}
bool fail(const char *m, struct SourceLocation loc)
{
	return handler_top->fail(handler_top->bag, loc, m);
}
bool assert_equal(const char* an, int av, const char* bn, int bv, struct SourceLocation loc)
{
	if (av != bv)
	{
		return handler_top->fail(handler_top->bag, loc, 
		                         "ASSERT Failed: %s(%d) == %s(%d)",
								 an, av, bn, bv);
	}
	return true;
}
bool assert_streq(const char* a, const char* b, struct SourceLocation loc)
{
	if (!strcmp(a,b)) {
		return handler_top->fail(handler_top->bag, loc, "ASSERT Failed: \"%s\" == \"%s\"", a, b);
	}
	return true;
}
int check_index(int len, int i, struct SourceLocation source)
{
	i += len * (i < 0);

	if (i < 0  ||  i >= len)
		handler_top->fail(handler_top->bag, source, "Array index %d out of bounds [%d,%d).", i, 0, len);

	return i;
}


//----------------------------------------------------------------------
// Compound Types

nuspan num_slice(nuspan span, int first, int last)
{
	return (nuspan){
		.p = span.p + check_index(span.length, first, SRCLOC),
		.length = check_index(span.length, last, SRCLOC) - first + 1
	};
}

strand str_slice(strand span, int first, int last) 
{
	first = check_index(span.length, first, SRCLOC);
	last  = check_index(span.length, last, SRCLOC);
	return (strand){
		.p = span.p + first, 
		.length = last - first + 1
	};
}

const struct Interval R_positive = {  DBL_MIN,   INFINITY };
const struct Interval R_negative = { -INFINITY, -DBL_MIN  };
const struct Interval R_all      = { -INFINITY,  INFINITY };
const struct Interval R_zero     = {  0.0,       0.0      };
const struct Interval R_unit     = {  0.0,       1.0      };
const struct Interval R_empty    = {  NAN,       NAN      };

struct Interval interval(double left, double right)
{
	return 
		(struct Interval) {
			.left = fmin(left, right), 
			.right = fmax(left, right)
		};
}
bool includes(struct Interval invl, double n)
{
	return invl.left <= n && n <= invl.right;
}
double clamp(struct Interval invl, double n)
{
	if (n < invl.left)  
		return invl.left;
	else if (n > invl.right) 
		return invl.right;
	else
		return n;
}
double lerp(struct Interval invl, double t) 
{
	return invl.left * (1.0 - t) + invl.right * t; 
}
void Interval_fprint(FILE *out, struct Interval invl)
{
	fprintf(out, "[%g,%g]", invl.left, invl.right);
}


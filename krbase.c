#include "krbase.h"
#include <math.h>
#include <float.h>
#include <signal.h>


//----------------------------------------------------------------------
// Strings

bool String_equals(String a, String b)
{
	return 
		(!a.data && !b.data) ||
		(a.length == b.length) && 
		!strncmp(a.data, b.data, a.length);
}

void String_fprint(String s, FILE *out)
{
	require(s.data && s.length);
	require(out);
	while (s.length--)  fputc(*s.data++, out); 
}

int int_swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
	return t;
}

//----------------------------------------------------------------------
// Debugging

void debug_vprint(FILE *out, SourceLine source, const char *format, va_list args)
{
	out = ptr_and(out, stderr);
	String_fprint(source.file_name, out);
	fprintf(out, ":%d: ", source.line_num);
	if (format)
		vfprintf(out, format, args);
	fputc('\n', out);
}

void debug_print(FILE *out, SourceLine source, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	debug_vprint(out, source, format, args);
	va_end(args);
}


bool crash(SourceLine source, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	debug_vprint(stderr, source, format, args);
	va_end(args);

	String_fprint($s("Begin backtrace.\n"), stderr);
	tcc_backtrace("");
	String_fprint($s("End backtrace.\n"), stderr);

	abort();
	return false; 
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


//----------------------------------------------------------------------
// Assertions

int check(int i, int len)
{
	int r = i + len * (i < 0);
	precond((0 <= r  &&  r < len), "Array index %d out of bounds [0,%d).", i, len);
	return r;
}


//----------------------------------------------------------------------
// Compound Types

nuspan num_slice(nuspan span, int first, int last)
{
	return (nuspan){
		.data = span.data + check(first, span.length),
		.length = check(last, span.length) - first + 1
	};
}

String str_slice(String str, int from, int to) 
{
	from = check(from, str.length);
	to  = check(to, str.length);

	if (from > to) 
		int_swap(&from, &to);

	return (String){
		.data = str.data + from, 
		.length = to - from + 1
	};
}

String str_first(String s, int n)
{
	require(n >= 0);
	require(n <= s.length);
	s.length = n;
	return s;
}

String str_last(String s, int n)
{
	require(n >= 0);
	require(n <= s.length);
	s.data += (s.length - n);
	s.length = n;
	return s;
}

const struct Interval R_positive = {  DBL_MIN,   INFINITY };
const struct Interval R_negative = { -INFINITY, -DBL_MIN  };
const struct Interval R_all      = { -INFINITY,  INFINITY };
const struct Interval R_zero     = {  0.0,       0.0      };
const struct Interval R_unit     = {  0.0,       1.0      };
const struct Interval R_empty    = {  NAN,       NAN      };

struct Interval interval(double left, double right)
{
	return (struct Interval) {
		.left  = fmin(left, right), 
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


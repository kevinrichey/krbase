#include "krprim.h"
#include <math.h>
#include <float.h>
#include <signal.h>


//----------------------------------------------------------------------
// Debugging & Assertions

int check(int i, int len)
{
	int r = i + len * (i < 0);
	assertf((0 <= r  &&  r < len), "Array index %d out of bounds [0,%d).", i, len);
	return r;
}

void debug_vprint(FILE *out, SourceLine source, const char *format, va_list args)
{
	out = ptr_and(out, stderr);
	fprintf(out, "%s:%d: ", source.file_name, source.line_num);
	if (format)  vfprintf(out, format, args);
	fputc('\n', out);
}

void debug_print(FILE *out, SourceLine source, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	debug_vprint(out, source, format, args);
	va_end(args);
}


//----------------------------------------------------------------------
// String

String String_init(const char *c_str)
{
	return (String){ .data=c_str, .length=strlen(c_str) };
}

bool String_is_empty(String s)
{
	return !s.data || !s.length;
}

bool String_equals(String a, String b)
{
	return 
		(!a.data && !b.data) ||
		(a.length == b.length) && 
		!strncmp(a.data, b.data, a.length);
}

void String_fprint(String s, FILE *out)
{
	assert(s.data && s.length);
	assert(out);
	while (s.length--)  fputc(*s.data++, out); 
}

String String_slice(String str, int from, int to) 
{
	from = check(from, str.length);
	to   = check(to,   str.length);

	if (from > to) 
		int_swap(&from, &to);

	return (String){
		.data = str.data + from, 
		.length = to - from + 1
	};
}

String String_first(String s, int n)
{
	assert(n >= 0);
	assert(n <= s.length);
	s.length = n;
	return s;
}

String String_last(String s, int n)
{
	assert(n >= 0);
	assert(n <= s.length);
	s.data += (s.length - n);
	s.length = n;
	return s;
}

//----------------------------------------------------------------------
// Error Handling

const char *Status_cstr(Status stat)
{
	assert(in_range(stat, STAT_FIRST, STAT_END-1));

#define X(NAME_, _)  [CONCAT(STAT_, NAME_)] = STRINGIFY_EXPAND(CONCAT(STAT_, NAME_)),
	static const char *status_names[] = {
		STATUS_X_TABLE
	};
#undef X

	return status_names[stat];
}

String Status_string(Status stat)
{
	return String_init(Status_cstr(stat));
}

void throw(Exception *e, Status status, String message, SourceLine loc)
{
	if (e)
	{
		e->status   = status;
		e->message  = message;
		e->location = loc;
		longjmp(e->env, status);
	}

	crash("Unhandled exception %s (%d)", Status_cstr(status), (int)status);
}

//----------------------------------------------------------------------
// Numbers

int int_min(int a, int b)  { return (a < b) ? a : b; }
int int_max(int a, int b)  { return (a > b) ? a : b; }

bool in_range(int n, int low, int hi)
{
	assert(low <= hi);
	return (low <= n) && (n <= hi);
}

int int_swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
	return t;
}

bool Double_equals(double a, double b, double epsilon)
{
	return fabs(a - b) <= epsilon;
}

//----------------------------------------------------------------------
// Pointer Stuff

void *ptr_and(void *p, void *alt)
{
	return p ? p : alt;
}


//=============================================================================
// Compound Types

//----------------------------------------------------------------------
// Span

nuspan num_slice(nuspan span, int from, int to)
{
	from = check(from, span.length);
	to   = check(to,   span.length);

	if (from > to) 
		int_swap(&from, &to);

	return (nuspan){
		.data = span.data + from,
		.length = to - from + 1
	};
}

//----------------------------------------------------------------------
// Interval

static void interval_check(Interval invl)
{
	if (!isnan(invl.left) && !isnan(invl.right))
		assert(invl.left <= invl.right);
}

Interval interval(double left, double right)
{
	return (Interval) {
		.left  = fmin(left, right), 
		.right = fmax(left, right)
	};
}

bool includes(Interval invl, double n)
{
	interval_check(invl);
	return invl.left <= n && n <= invl.right;
}

double clamp(Interval invl, double n)
{
	interval_check(invl);
	if (n < invl.left)  
		return invl.left;
	else if (n > invl.right) 
		return invl.right;
	else
		return n;
}

double lerp(Interval invl, double t) 
{
	interval_check(invl);
	return invl.left * (1.0 - t) + invl.right * t; 
}

void Interval_fprint(FILE *out, Interval invl)
{
	interval_check(invl);
	fprintf(out, "[%g,%g]", invl.left, invl.right);
}

const Interval R_positive = {  DBL_MIN,   INFINITY };
const Interval R_negative = { -INFINITY, -DBL_MIN  };
const Interval R_all      = { -INFINITY,  INFINITY };
const Interval R_zero     = {  0.0,       0.0      };
const Interval R_unit     = {  0.0,       1.0      };
const Interval R_empty    = {  NAN,       NAN      };

//======================================================================
// Memory management

//-----------------------------------------------------------------------------
// Arenas

Size Arena_cap(Arena a)
{
	return a.end - a.beg;
}

Vspan alloc(Arena *arena, Size unit, Size align, Size count, int fill, SourceLine loc)
{
	Size padding   = -(Size)arena->beg & (align - 1);
	Size available = arena->end - arena->beg - padding;

	if (available < 0 || count > available/unit) {
		throw(arena->ex, STAT_OUT_OF_MEM, Str("Out of memory"), loc);
		return (Vspan){0};
	}

	void *p = arena->beg + padding;
	arena->beg += padding + count*unit;

	if (fill >= 0)  memset(p, fill, count*unit);

//	return (fill<0) ? p : memset(p, fill, count*unit);
	return (Vspan){ p, count };
}



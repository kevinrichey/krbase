#include "krbase.h"
#include <math.h>

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

int int_min(int a, int b)  { return (a < b) ? a : b; }
int int_max(int a, int b)  { return (a > b) ? a : b; }

int int_clamp(int n, int low, int hi)
{
	return (n < low) ? low :
	       (n > hi)  ? hi  :
           n;
}

int int_in_range(int n, int low, int hi)
{
	return (low <= n) && (n <= hi);
}

double float_min(double a, double b) { return (a < b) ? a : b; }
double float_max(double a, double b) { return (a > b) ? a : b; }

double float_clamp(double n, double low, double hi)
{
	return (n < low) ? low :
	       (n > hi)  ? hi  :
           n;
}

double float_in_range(double n, double low, double hi)
{
	return (low <= n) && (n <= hi);
}

double float_equals(double a, double b, double epsilon)
{
	return fabs(a - b) <= epsilon;
}

double lerp(double a, double b, double t) 
{
	return a * (1-t) + b * t; 
}


void *ptr_and(void *p, void *alt)
{
	return p ? p : alt;
}

const void *const_ptr_and(const void *p, const void *d)
{
	return p ? p : d;
}


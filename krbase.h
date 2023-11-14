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

typedef unsigned char Byte;

typedef void (*VoidFunc)(void);

struct SourceLocation 
{
	const char *file_name;
	unsigned    line_num;
};

#define CURRENT_LOCATION   (struct SourceLocation){ .file_name=__FILE__, .line_num=__LINE__ }

void debug_vprint(FILE *out, struct SourceLocation source, const char *format, va_list args);
void debug_print(FILE *out, struct SourceLocation source, const char *format, ...);

int    int_min(int a, int b);
int    int_max(int a, int b);
int    int_clamp(int n, int low, int hi);
int    int_in_range(int n, int low, int hi);

double float_min(double a, double b);
double float_max(double a, double b);
double float_clamp(double n, double low, double hi);
double float_in_range(double n, double low, double hi);
double float_equals(double a, double b, double epsilon);

double lerp(double a, double b, double t);

void       *ptr_and(void *p, void *alt);
const void *const_ptr_and(const void *p, const void *d);


#endif

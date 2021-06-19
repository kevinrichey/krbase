#include <string.h>

#include "krstring.h"
#include "krclib.h"

typedef struct string {
	size_t size;
	char  *back;
	char   front[];
} string;

string *string_create(const char *from)
{
	size_t length = strlen(from);
	string *s = string_reserve(NULL, length + 1);
	if (s) {
		strncpy(deconst(char*, s->front), from, length+1);
		s->back = s->front + length;
	}
	return s;
}

void string_dispose(string *s)
{
	free(s);
}

string *string_reserve(string *s, size_t bigger)
{
	if (s && bigger == 0)
		bigger = s->size * 2;

	else if (s && s->size >= bigger)
		return s;

	bigger = max(bigger, 8);

	size_t length = string_length(s);
	string *new_s = realloc(s, sizeof(string) + bigger);
	
	if (!new_s) {
		fprintf(stderr, "string_reserve() failed to allocate %zu bytes.\n", bigger);
		exit(1);
	}

	new_s->size = bigger;
	new_s->back = new_s->front + length;

	return new_s;
}

string *string_pushc(string *s, int c)
{
	if (string_is_full(s))
		s = string_reserve(s, 0);
	*s->back++ = c;
	return s;
}



size_t string_length(const string *s)
{
	return s ? (s->back - s->front) : 0;
}

size_t  string_size(const string *s)
{
	return (s == NULL) ? 0 : s->size;
}

bool string_is_empty(const string *s) 
{
	return !s  ||  string_length(s) == 0;
}

bool string_is_full(const string *s)
{
	return (s == NULL) || (s->back == (s->front + s->size));
}

const char *string_cstr(const string *s)
{
	return s ? s->front : "";
}

bool string_equals(const string *s, const char *cstr)
{
	if (s && cstr)
		return !strcmp(s->front, cstr);
	else
		return (!s && !cstr);
}


void string_puts(const string *s)
{
	puts(s ? s->front : "empty string");
}



string *string_format(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	va_list n_args;
	va_copy(n_args, args);
	int length = vsnprintf(NULL, 0, format, n_args) + 1;
	va_end(n_args);

	string *s = string_reserve(NULL, length + 1);
	if (s && s->size > 1) {
		vsnprintf(deconst(char*, s->front), s->size, format, args);
		s->back = s->front + length;
	}

	va_end(args);
	return s;
}

string *string_clear(string *s)
{
	if (s)  s->back = s->front;
	return s;
}

string *string_fgetline(FILE *in, string *s)
{
	if (feof(in))
		return NULL;

	if (s)  s->back = s->front;

	int c;
	while ((c = fgetc(in)) != EOF && c != '\n')
		s = string_pushc(s, c);
	
	s = string_pushc(s, '\0');

	return s;
}

void string_swap(string **a, string **b)
{
	string *t = *a;
	*a = *b;
	*b = t;
}


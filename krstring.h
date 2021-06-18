#ifndef KRSTRING_H_INCLUDED
#define KRSTRING_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct string string;

string     *string_create(const char *str);
string     *string_reserve(string *s, size_t bigger);
string     *string_pushc(string *s, int c);
void        string_dispose(string *s);

size_t      string_length(const string *s);
size_t      string_size(const string *s);
bool        string_is_full(const string *s);
bool        string_is_empty(const string *s);
const char *string_cstr(const string *s);
bool        string_equals(const string *s, const char *cstr);

string     *string_clear(string *s);
string     *string_copy(const char *from);
string     *string_format(const char *format, ...);
string     *string_fgetline(FILE *in, string *s);
void        string_swap(string **a, string **b);

void        string_puts(const string *s);

#endif

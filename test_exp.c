#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include "test.h"

//
// This is all experimental stuff
//


//@module string

struct string_buf {
	size_t size;
	char   strbuf[];
};

typedef struct {
	const char *start, *end;
	char mode;
} string;

#define STRING_MODE_ALLOC   'x'
#define STRING_MODE_SCOPED  'i'
#define STRING_MODE_ERROR   'e'

string string_init_x(const char *s, size_t n, int mode)
{
	return (string){ .start = s, .end = s + n, .mode = mode };
}

string string_init_n(const char *s, size_t n)
{
	return string_init_x(s, n, STRING_MODE_SCOPED);
}

string string_init(const char *s)
{
	return string_init_n(s, s? strlen(s): 0);
}

#define STR(S_)   string_init_n((S_), ARRAY_SIZE(S_))

struct string_buf *stringbuf_alloc(int length)
{
	size_t size = sizeof(char) * length + 1;
	struct string_buf *str = malloc(sizeof(*str) + size);
	if (str)
		str->size = size;
	return str;
}

int string_length(string s) 
{
	return s.end - s.start;
}

string string_copy(string from)
{
	int length = string_length(from);
	string str = { .mode = STRING_MODE_ERROR };

	struct string_buf *buf = stringbuf_alloc(length);
	if (buf) {
		if (length > 0)
			strncpy(buf->strbuf, from.start, length);
		buf->strbuf[length] = '\0';
		str = string_init_x(buf->strbuf, length, STRING_MODE_ALLOC);
	}

	return str;
}

bool string_is_empty(string s)
{
	return !s.start || !string_length(s);
}

void string_destroy(string *s)
{
	if (s && s->mode == STRING_MODE_ALLOC) {
		struct string_buf *buf = (struct string_buf *)(s->start - sizeof(*buf));
		free(buf);
	}
	s->start = s->end = NULL;
	s->mode = '\0';
}

bool strings_equal(string a, string b)
{
	return !strcmp(a.start, b.start);
}

TEST_CASE(create_string_buffer)
{
	const char *cs = "Initial String Value";
	string s = string_init(NULL);
	TEST(string_length(s) == 0);
	TEST(string_is_empty(s));

	s = string_copy(string_init(cs));

	TEST(!string_is_empty(s));
	TEST(string_length(s) == strlen("Initial String Value"));
	TEST(strings_equal(s, STR("Initial String Value")));

	string_destroy(&s);
}

TEST_CASE(copy_null_string)
{
	string nullstr = string_init(NULL);
	string s = string_copy(nullstr);
	TEST(string_length(s) == 0);
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;
	while (*s++ && ++len < maxlen) ;
	return len;
}

TEST_CASE(length_of_fixed_size_string)
{
	char same_len[] = "xyzzy";
	TEST(strnlen(same_len, sizeof(same_len)) == 5);

	char longer_str[100] = "less than one hundred";
	TEST(strnlen(longer_str, sizeof(longer_str)) == 21);

	char shorter[] = "send a shorter max length";
	TEST(strnlen(shorter, 10) == 10);
	
	char empty[] = "";
	TEST(strnlen(empty, sizeof(empty)) == 0);
}




void swap_c(char *a, char *b)
{
	char t = *a;
	*a = *b;
	*b = t;
}



typedef struct {
	Link link;
	void (*dispose)(void*);
} Resource;

typedef struct {
	Resource res;
	char name[32];
} Object;

void Object_dispose(void *object)
{
	Object *o = object;
	fprintf(stderr, "Disposing %s\n", o->name);
	free(o);
}

Object *Object_create(const char *n)
{
	Object *o = malloc(sizeof(*o));
	strncpy(o->name, n, sizeof(o->name));
	o->res.dispose = Object_dispose;
	fprintf(stderr, "init %s\n", o->name);
	return o;
}

void code(Chain *res_chain, jmp_buf *jmp)
{
	Object *a = Object_create("A");
	Chain_append(res_chain, &a->res.link);

	Object *b = Object_create("B");
	Chain_append(res_chain, &b->res.link);

	longjmp(*jmp, 99);

	Object *c = Object_create("C");
	Chain_append(res_chain, &c->res.link);
}


TEST_CASE(destructor_chain)
{
	jmp_buf jmp;

	Chain res_chain = CHAIN_INIT(res_chain);
	if (!setjmp(jmp)) {

		code(&res_chain, &jmp);

		Object *first = (Object*)Chain_first(&res_chain);
		TEST(!strcmp(first->name, "A"));
		Object *second = (Object*)Link_next(&first->res.link);
		TEST(!strcmp(second->name, "B"));
		Object *third = (Object*)Link_next(&second->res.link);
		TEST(!strcmp(third->name, "C"));
	}

	Resource *this = (Resource*)Chain_last(&res_chain);
	while (&this->link != &res_chain.head) {
		Resource *prev = (Resource*)Link_prev(&this->link);
		this->dispose(this);
		this = prev;
	}

}


TEST_CASE(convert_things_to_bytes)
{
	int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	Bytes ns1 = Bytes_init_array(numbers);
	TEST(ns1.size == (10 * sizeof(int)));

	int i = 101;
	Bytes ns2 = Bytes_init_var(i);
	TEST(ns2.size == sizeof(int));

	double d = 3.14159;
	Bytes ns3 = Bytes_init_var(d);
	TEST(ns3.size == sizeof(double));

	char s[100] = "Hello, World";
	Bytes ns4 = Bytes_init_str(s);
	TEST(ns4.size == 12);
}

TEST_CASE(FNV_hash_test)
{
	int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	uint64_t h = hash(Bytes_init_array(numbers));
	TEST(h == 6902647252728264142LLU);

	char s[] = "Hello, World!";
	h = hash(Bytes_init_str(s));
	TEST(h == 7993990320990026836LLU);

	int i = 101;
	h = hash(Bytes_init_var(i));
	TEST(h == 3212644748862486336LLU);

	double d = 3.12159;
	h = hash(Bytes_init_var(d));
	TEST(h == 8148618316659391402LLU);
}

void fill(void *data, int length, void *set, int e_size)
{
	Byte_t *b = data;

	while (length --> 0) {
		memcpy(b, set, e_size);
		b += e_size;
	}
}

#define Array_fill(A_, V_)   \
	fill((A_), ARRAY_SIZE(A_), &(V_), sizeof(V_))

TEST_CASE(hash_table)
{

	typedef struct {
		uint64_t hash;
		char     value;
	} table_record;
	
	table_record data[16];
	int   size = ARRAY_SIZE(data);

	Array_fill(data, ((table_record){ .hash=0, .value=' ' }));

	char s1[] = "Hello";

	uint64_t s1_hash = hash(Bytes_init_str(s1));
	unsigned mask = size - 1;
	int i = mask & s1_hash;
	data[i].hash = s1_hash;
	data[i].value = 'a';

	for (int f = 0; f < size; ++f) {
//		printf("%3d: hash = %llu, value = %c\n", f, data[f].hash, data[f].value);
		if (data[f].hash > 0) {
			TEST(data[f].value == 'a');
		}
	}

}



//-----------------------------------------------------------------------------
// Fun Variadic Functions
//

void unpack_i(int *a, int n, ...)
{
	va_list args;
	va_start(args, n);

	int *i;
	while (n --> 0) {
		if ((i = va_arg(args, int*)))
			*i = *a++;
		else
			break;
	}

	va_end(args);
}

TEST_CASE(unpack_entire_array)
{
	int arr[] = { 10, 11, 12, 13, 14 };
	int a, b, c, d, e;

	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, &d, &e);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
	TEST(d == 13);
	TEST(e == 14);
}

TEST_CASE(unpack_partial_array)
{
	int arr[] = { 10, 11, 12, 13, 14 };
	int a, b, c;

	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, NULL);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
}

TEST_CASE(unpack_shorter_array)
{
	int arr[] = { 10, 11, 12 };
	int a, b, c;
	int d = 99;
	int e = 88;

	// Since array is shorter, only 3 variables will be updated
	unpack_i(arr, ARRAY_SIZE(arr), &a, &b, &c, &d, &e);
	TEST(a == 10);
	TEST(b == 11);
	TEST(c == 12);
	TEST(d == 99);
	TEST(e == 88);
}


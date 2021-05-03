#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include "test.h"

//
// This is all experimental stuff
//

#define member_to_struct_ptr(PTR_, TYPE_, MEMBER_)  \
	(TYPE_*)((byte*)(PTR_) - offsetof(TYPE_, MEMBER_))

TEST_CASE(convert_member_ptr_to_struct)
{
	typedef struct {
		int i;
		double d;
		char s[];
	} xyzzy;

	xyzzy *zork = malloc(sizeof(xyzzy) + 10);

	int *ip = &zork->i;
	TEST(member_to_struct_ptr(ip, xyzzy, i) == zork);

	double *dp = &zork->d;
	TEST(member_to_struct_ptr(dp, xyzzy, d) == zork);

	char *sp = zork->s;
	TEST(member_to_struct_ptr(sp, xyzzy, s) == zork);

	free(zork);
}

//@module string

#define FAMSIZE(OBJ_, FAM_, LENGTH_)  (sizeof((OBJ_)) + sizeof(*(OBJ_).FAM_) * (LENGTH_))

struct string_buf {
	size_t size;
	char   strbuf[];
};

struct allocator {
	void *(*alloc)(size_t);
	void  (*free)(void*);
	void  (*fail)(SourceInfo, size_t);
};

void *alloc_null(size_t size) { UNUSED(size); return NULL; }
void  free_nop(void *ptr) { UNUSED(ptr); }
void  nop(void) {}

void  alloc_fail_print(SourceInfo source, size_t size)
{
	fprintf(stderr, "%s:%d: Allocation failed, %lu bytes\n", 
			source.file, source.line, size);
}

struct string_buf *stringbuf_alloc(int length, struct allocator *allocator, SourceInfo source)
{
	size_t size = 0;
	struct string_buf *str = allocator->alloc(size = FAMSIZE(*str, strbuf, length));
	if (str)
		str->size = length;
	else
		allocator->fail(source, size);
	return str;
}

struct string_buf *stringbuf_create_dbg(int length, SourceInfo source)
{
	return stringbuf_alloc(length, &(struct allocator){ malloc, free, alloc_fail_print }, source);
}

#define stringbuf_create(LENGTH_)   stringbuf_create_dbg((LENGTH_), SOURCE_HERE)


typedef struct {
	const char *start, *end;
	char mode;
} string;

#define STRING_MODE_NULL    '\0'
#define STRING_MODE_ALLOC   'a'
#define STRING_MODE_SCOPED  's'
#define STRING_MODE_ERROR   'e'

string string_init_x(const char *s, size_t length, int mode)
{
	return (string){ .start = s, .end = s + length, .mode = mode };
}

string string_init_n(const char *s, size_t length)
{
	return string_init_x(s, length, STRING_MODE_SCOPED);
}

string string_init(const char *s)
{
	return string_init_n(s, s? strlen(s): 0);
}

#define STR(S_)   string_init_n((S_), ARRAY_SIZE(S_)-1)

int string_length(string s) 
{
	return s.end - s.start;
}

bool string_is_empty(string s)
{
	return !s.start || !string_length(s);
}

bool strings_equal(string a, string b)
{
	if (string_is_empty(a) && string_is_empty(b))
		return true;

	if (string_length(a) != string_length(b))
		return false;

	return !strcmp(a.start, b.start);
}

string string_copy_dbg(string from, SourceInfo source)
{
	int length = string_length(from);

	if (length == 0)
		return string_init(NULL);

	struct string_buf *buf = stringbuf_alloc(length+1, &(struct allocator){ malloc, free, alloc_fail_print }, source);

	if (buf == NULL)
		return (string){ .mode = STRING_MODE_ERROR };

	strncpy(buf->strbuf, from.start, length);
	buf->strbuf[length] = '\0';
	return string_init_x(buf->strbuf, length, STRING_MODE_ALLOC);
}

#define string_copy(FROMSTR_)  string_copy_dbg((FROMSTR_), SOURCE_HERE)

void string_destroy(string *s)
{
	if (s) {
		if (s->mode == STRING_MODE_ALLOC)
			free(member_to_struct_ptr(s->start, struct string_buf, strbuf));
		*s = (string){0};
	}
}

TEST_CASE(null_string_is_empty)
{
	string s = string_init(NULL);
	TEST(string_length(s) == 0);
	TEST(string_is_empty(s));

	string t = string_copy(s);
	TEST(string_length(t) == 0);
	TEST(string_is_empty(t));

	string_destroy(&s);
	string_destroy(&t);
}

TEST_CASE(create_string_from_literal)
{
	string s = STR("hello world");

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 11);
	TEST(strings_equal(s, STR("hello world")));

	string_destroy(&s);
}

TEST_CASE(create_string_from_array)
{
	char sa[32] = "this is 31 chars long";

	// STR() gets length from the array, not the string!
	// String length is the array length 32 minus one for null terminator
	string s = STR(sa);
	TEST(!string_is_empty(s));
	TEST(string_length(s) == 31);
	// Not equal because the literal is 21 and the array is 31
	TEST(!strings_equal(s, STR("this is 31 chars long")));

	// It's probably better to use string_init() for arrays.
	string t = string_init(sa);
	TEST(!string_is_empty(t));
	TEST(string_length(t) == 21);
	TEST(strings_equal(t, STR("this is 31 chars long")));

	// Technically unnecessary since arrays and literals are not 
	// allocated on heap. But probably good practice anyway.
	string_destroy(&s);
	string_destroy(&t);
}

TEST_CASE(create_string_from_pointer)
{
	char sa[] = "xyzzy";
	char *sp = sa;

	string s = string_init(sp);

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 5);
	TEST(strings_equal(s, STR("xyzzy")));

	string_destroy(&s);
}

TEST_CASE(create_string_from_compound_literal)
{
	// this is mostly useless since strings are immutable
	string s = STR((char[16]){0});

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 15);

	string_destroy(&s);
}

TEST_CASE(copy_literal_to_string)
{
	string s = string_copy(STR("Initial String Value"));

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 20);
	TEST(strings_equal(s, STR("Initial String Value")));

	string_destroy(&s);
}

TEST_CASE(copy_null_string)
{
	string nullstr = string_init(NULL);
	string s = string_copy(nullstr);
	TEST(string_length(s) == 0);
	TEST(strings_equal(s, nullstr));

	string_destroy(&s);
}

string string_format(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	string newstr = string_init(NULL);

	va_list n_args;
	va_copy(n_args, args);
	int length = vsnprintf(NULL, 0, format, n_args);
	va_end(n_args);

	if (length < 0)
		newstr.mode = STRING_MODE_ERROR;

	if (length > 0) {
		struct string_buf *buf = stringbuf_create(length+1);
		if (buf) {
			int num = vsnprintf(buf->strbuf, buf->size, format, args);
			newstr = string_init_n(buf->strbuf, length);
			if (num < 0)
				newstr.mode = STRING_MODE_ERROR;
		}
		else
			newstr.mode = STRING_MODE_ERROR;
	}

	va_end(args);
	return newstr;
}


TEST_CASE(create_formatted_string)
{
	const char *answer = "the answer";
	string s = string_format("%s to %d/%d is %f", answer, 2, 3, 2.0/3.0);

	string sc = STR("the answer to 2/3 is 0.666667");
	TEST(strings_equal(s, sc));
}

static bool TEST_STRINGBUF_FAILED = false;

static void test_stringbuf_alloc_failed (SourceInfo source, size_t size)
{
	alloc_fail_print(source, size);
	TEST_STRINGBUF_FAILED = true;
}

TEST_CASE(create_string_buf_fails)
{
	struct allocator stralloc = { alloc_null, free_nop, test_stringbuf_alloc_failed };
	struct string_buf *str = stringbuf_alloc(10, &stralloc, SOURCE_HERE);
	TEST(str == NULL);
	TEST(TEST_STRINGBUF_FAILED);
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


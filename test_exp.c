#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "klib.h"

//
// This is all experimental stuff
//

TEST_CASE(make_span_from_arrays)
{
	char letters[] = "abcdefghijklmnopqrstuvwxyz";
	Span_t(char) cspan = Span_init(letters);
	TEST(cspan.size == 27); // don't forget the null-terminator
	TEST(cspan.begin[5] == 'f');

	// An array of fibonacci numbers
	Fibonacci fib = Fib_begin();
	int numbers[20];
	int length  = ARRAY_SIZE(numbers);
	for (int *n = numbers; length--; ++n) {
		*n = Fib_get(fib);
		fib = Fib_next(fib);
	}

	// Make a span from the array
	Span_t(int) nspan = Span_init(numbers, 20);
	TEST(nspan.size == 20);
	int i = 0;
	TEST(nspan.begin[i++] ==  0);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  1);
	TEST(nspan.begin[i++] ==  2);
	TEST(nspan.begin[i++] ==  3);
	TEST(nspan.begin[i++] ==  5);
	TEST(nspan.begin[i++] ==  8);
	TEST(nspan.begin[i++] == 13);
	TEST(nspan.begin[i++] == 21);
	TEST(nspan.begin[i++] == 34);
	TEST(nspan.begin[i++] == 55);
	TEST(nspan.begin[i++] == 89);
}


//@module String

//typedef Span_t(char)  String;

typedef struct { char *begin; int size; } String;

static inline bool String_not_empty(String s)
{
	return s.size && s.begin && *s.begin;
}

static inline bool String_is_empty(String s)
{
	return !String_not_empty(s);
}

int String_length(String s)
{
	int length = 0;
	while (s.size-- && *s.begin++)  
		++length;
	return length;
}

#define String_init(...)  Span_init(__VA_ARGS__)

#define STR(...)    (String)String_init(__VA_ARGS__)

String String_create(int size)
{
	return STR(calloc(size, sizeof(char)), size);
}

int vprintf_size(const char *format, va_list args)
{
	va_list args2;
	va_copy(args2, args);
	int size = 1 + vsnprintf(NULL, 0, format, args2);
	va_end(args2);
	return size;
}

String String_create_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int size = vprintf_size(format, args);
	//String s = String_create(size);
	char *s = malloc(size * sizeof(char));
	if (s)
		vsnprintf(s, size, format, args);

	va_end(args);
	return STR(s, size);
}

void String_destroy(String *s)
{
	if (s) {
		free((void*)s->begin);
		*s = (String){0};
	}
}

bool String_equals(String s1, String s2)
{
	return !strncmp(s1.begin, s2.begin, int_min(s1.size, s2.size));
}

TEST_CASE(init_string_to_null)
{
	String s_null = {0};

	TEST(s_null.size == 0);
	TEST(s_null.begin == NULL);
	TEST(String_is_empty(s_null));
}

TEST_CASE(init_string_from_literal)
{
	String s_lit = String_init("xyzzy");

	TEST(s_lit.size == 6);
	TEST(String_length(s_lit) == 5);
	TEST(String_not_empty(s_lit));
}

TEST_CASE(init_string_from_empty_array)
{
	char words[40] = {0};
	String s_arr = String_init(words);
	TEST(s_arr.size == 40);
	TEST(String_is_empty(s_arr));
	TEST(String_length(s_arr) == 0);
}

TEST_CASE(init_string_from_pointer_and_size)
{
	char *ptr = "Hello, world";
	String s_ptr = String_init(ptr, 13);

	TEST(s_ptr.size == 13);
	TEST(String_length(s_ptr) == 12);
	TEST(String_not_empty(s_ptr));
}

TEST_CASE(init_string_from_compound_literal)
{
	String s_acl = String_init((char[15]){ "Foobar"});
	TEST(s_acl.size == 15);
	//TEST(String_is_empty(s_acl));

	//strncpy(s_acl.begin, "Foobar", s_acl.size);
	TEST(String_length(s_acl) == 6);
	TEST(String_not_empty(s_acl));
}

TEST_CASE(create_string_on_heap)
{
	String s = String_create(100);
	TEST(s.size == 100);
	TEST(String_is_empty(s));

	//strncpy(s.begin, "abcdefghijklmnopqrstuvwxyz", s.size);
	//TEST(String_length(s) == 26);
	//TEST(String_not_empty(s));

	String_destroy(&s);
	TEST(s.size  == 0);
	TEST(String_is_empty(s));
	TEST(String_length(s) == 0);
}

TEST_CASE(create_formatted_string_on_heap)
{
	String s = String_create_f("Test %d\n", 123);
	TEST(String_not_empty(s));
	TEST(String_length(s) == 9);
	TEST(s.size == 10);
	TEST(String_equals(s, STR("Test 123\n")));
	
	String_destroy(&s);
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

int timestamp_now(String *out_time)
{
	time_t now = time(NULL);
	struct tm *now_tm = localtime(&now);
	if (now_tm)
		return strftime(out_time->begin, out_time->size, "%Y-%m-%d %H:%M:%S", now_tm);
	else
		return 0;
}

TEST_CASE(log_to_file)
{
	UNUSED(test_counter);

	FILE *log_file = fopen("test.log", "w");

	String stamp = String_init((char[25]){0});
	timestamp_now(&stamp); 
	fprintf(log_file, "%s DEBUG 1 This is a test.\n", stamp.begin);

	fclose(log_file);
}


#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <assert.h>

#include "krclib.h"

//
// This is all experimental stuff
//


#define  VAR_SEL1(TYPE_)         TYPE_:1
#define  VAR_SEL2(TYPE_, ...)    VAR_SEL1(__VA_ARGS__), TYPE_:2

//#define  VAR_SELECT(TYPE_)  _Generic((TYPE_){0}, int:1, double:2)
#define  VAR_SELECT(TYPE_)  _Generic((TYPE_){0}, VAR_SEL2(int, double))

#define  VAR_MEMBER(TYPE_)      TYPE_##_val
#define  VAR_MEM1(TYPE_)        TYPE_ VAR_MEMBER(TYPE_);
#define  VAR_MEM2(TYPE_, ...)   VAR_MEM1(__VA_ARGS__) TYPE_ VAR_MEMBER(TYPE_);

#define VARIANT_2(T1_, T2_)  \
struct { union { VAR_MEM2(T1_, T2_) }; int type; }

typedef VARIANT_2(int, double) var;


#define var_init(TYPE_, VAL_)  { { .VAR_MEMBER(TYPE_) = (VAL_) }, .type = VAR_SELECT(TYPE_) }

var var_int(int i)
{
	return (var){ { .VAR_MEMBER(int) = i }, .type = VAR_SELECT(int) };
}

bool var_is_set(var *v)
{
	return v && v->type > 0 && v->type <= 2;
}

#define var_is(V_, TYPE_)  ((V_).type == VAR_SELECT(TYPE_))

bool var_type_is(var *v, int t)
{
	return v && v->type == t;
}

#define var_get(V_, TYPE_)   (assert((V_).type == VAR_SELECT(TYPE_)), (V_).VAR_MEMBER(TYPE_))

int var_get_int(var *v)
{
	assert (v && v->type == 1);
	return v->VAR_MEMBER(int);
}

double var_get_double(var *v)
{
	assert (v && v->type == 2);
	return v->VAR_MEMBER(double);
}

TEST_CASE(set_value_in_variant)
{
	var v = var_init(int, 11);

	TEST(var_is_set(&v));
	TEST(var_is(v, int));
	TEST(!var_is(v, double));
	TEST(var_get(v, int) == 11);

//	double d = var_get(v, double);
//	TEST(d == 11.0);
}




struct Object;

typedef struct {
	const char *name;
	void (*dispose)(struct Object*);
} TypeInfo;

#define DECLARE_TYPE(TYPE_)  TypeInfo * TYPE_##_type_info = &(TypeInfo){ .name=#TYPE_, .dispose=(TYPE_##_dispose) }


typedef struct Object {
	TypeInfo *type;
	struct Object *next;
	struct source_location source;
	const char *name;
} Object;

void Object_dispose(Object *object)
{
	free(object);
}

DECLARE_TYPE(Object);


Object *Object_create_dbg(const char *name, Object *top, struct source_location source)
{
	Object *o = malloc(sizeof(*o));
	o->next = top;
	o->source = source;
	o->type = Object_type_info;
	o->name = name;
	return o;
}

#define Object_create(NAME_, TOP_)   Object_create_dbg((NAME_), (TOP_), CURRENT_LOCATION)

void dispose(Object *object)
{
	if (object)  object->type->dispose(object);
}

void Object_dispose_all(Object *top)
{
	while (top) {
		Object *n = top->next;
		dispose(top);
		top = n;
	}
}

TEST_CASE(object_life_cycle)
{
	Object *top = NULL;

	Object *one = top = Object_create("one", top);
	TEST(!strcmp(one->name, "one"));
	TEST(one->next == NULL);

	Object *two = top = Object_create("two", top);
	TEST(!strcmp(two->name, "two"));
	TEST(two->next == one);

	Object *three = top = Object_create("three", top);
	TEST(!strcmp(three->name, "three"));
	TEST(three->next == two);

	Object_dispose_all(top);
}


#define Bytes_init_array(ARR_)  \
			(byte_span)SPAN_INIT((Byte*)(ARR_), sizeof(ARR_))

#define Bytes_init_var(VAR_)   \
			(byte_span)SPAN_INIT((Byte*)&(VAR_), sizeof(VAR_))

byte_span Bytes_init_str(char *s);

TEST_CASE(convert_things_to_bytes)
{
	int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	byte_span ns1 = Bytes_init_array(numbers);
	TEST(SPAN_LENGTH(ns1) == (10 * sizeof(int)));

	int i = 101;
	byte_span ns2 = Bytes_init_var(i);
	TEST(SPAN_LENGTH(ns2) == sizeof(int));

	double d = 3.14159;
	byte_span ns3 = Bytes_init_var(d);
	TEST(SPAN_LENGTH(ns3) == sizeof(double));

	char s[100] = "Hello, World";
	byte_span ns4 = Bytes_init_str(s);
	TEST(SPAN_LENGTH(ns4) == 12);
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
	Byte *b = data;

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


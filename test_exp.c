#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <assert.h>

#include "test.h"
#include "krclib.h"
#include "krstring.h"

//
// This is all experimental stuff
//


#define VA_ITER_1(WHAT_, VAL_, ...)    WHAT_(VAL_, 1)
#define VA_ITER_2(WHAT_, VAL_, ...)    WHAT_(VAL_, 2) VA_ITER_1(WHAT_, __VA_ARGS__)
#define VA_ITER_3(WHAT_, VAL_, ...)    WHAT_(VAL_, 3) VA_ITER_2(WHAT_, __VA_ARGS__)
#define VA_ITER_4(WHAT_, VAL_, ...)    WHAT_(VAL_, 4) VA_ITER_3(WHAT_, __VA_ARGS__)
#define VA_ITER_5(WHAT_, VAL_, ...)    WHAT_(VAL_, 5) VA_ITER_4(WHAT_, __VA_ARGS__)
#define VA_ITER_6(WHAT_, VAL_, ...)    WHAT_(VAL_, 6) VA_ITER_5(WHAT_, __VA_ARGS__)
#define VA_ITER_7(WHAT_, VAL_, ...)    WHAT_(VAL_, 7) VA_ITER_6(WHAT_, __VA_ARGS__)
#define VA_ITER_8(WHAT_, VAL_, ...)    WHAT_(VAL_, 8) VA_ITER_7(WHAT_, __VA_ARGS__)
#define VA_ITER_9(WHAT_, VAL_, ...)    WHAT_(VAL_, 9) VA_ITER_8(WHAT_, __VA_ARGS__)

#define VA_FOREACH_N(N_, WHAT_, ...)   CONCAT(VA_ITER_,N_)(WHAT_, __VA_ARGS__)
#define VA_FOREACH(WHAT_, ...)         VA_FOREACH_N(VA_NARGS(__VA_ARGS__), WHAT_, __VA_ARGS__)


#define VAL_MEMBER_X(VAL_, I_)   VAL_ val##I_;
#define VAL_MEMBER(VAL_, I_)     VAL_MEMBER_X(VAL_, I_)
#define VARIANT_T(...)  struct { \
	int     type; \
	union { VA_FOREACH(VAL_MEMBER, __VA_ARGS__) } value; }


#define VAR_TYPE_ID_X(TYPE_, I_)   TYPE_: I_,
#define VAR_TYPE_ID(TYPE_, I_)     VAR_TYPE_ID_X(TYPE_, I_)
#define VARIANT_TYPE_ID_T(VAL_, ...)     _Generic(((VAL_){0}), VA_FOREACH(VAR_TYPE_ID, __VA_ARGS__) default:-1 )




typedef struct { int x; } dummy;

#define ATTRIB_TYPES   int, char*, dummy

typedef VARIANT_T(ATTRIB_TYPES) attrib;
#define attrib_type(TYPE_)     VARIANT_TYPE_ID_T(TYPE_, ATTRIB_TYPES)



#define attrib_init(TYPE_)  { .type = attrib_type(TYPE_) }

#define attrib_is(ATTR_, TYPE_)   (attrib_type(TYPE_) == (ATTR_).type)


#define attrib_val(ATTR_, TYPE_)  \
	(*(attrib_assert_type(&(ATTR_), attrib_type(TYPE_)), ((TYPE_*)&(ATTR_).value)))

bool attrib_assert_type(const attrib *a, int typeid)
{
	assert(a && a->type == typeid);
	return true;
}

bool attrib_is_void(const attrib *attr)
{
	return !attr || attr->type == 0;
}


TEST_CASE(create_attribute)
{
	attrib a = {0};
	TEST(attrib_is_void(&a));

	a = (attrib)attrib_init(dummy);

	TEST(!attrib_is_void(&a));
	TEST(attrib_is(a, dummy));
	TEST(!attrib_is(a, int));

	attrib_val(a, dummy).x = 99;

	switch(a.type) {
		case attrib_type(char*):
			fprintf(stderr, "a.value = %s\n", attrib_val(a, char*));
			// ok!
			break;
		case attrib_type(dummy):
			fprintf(stderr, "a.dummy.x = %d\n", attrib_val(a, dummy).x);
			break;
		default:
			TEST(!"Wrong attrib type!");
			break;
	}

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
	SourceInfo source;
	const char *name;
} Object;

void Object_dispose(Object *object)
{
	free(object);
}

DECLARE_TYPE(Object);


Object *Object_create_dbg(const char *name, Object *top, SourceInfo source)
{
	Object *o = malloc(sizeof(*o));
	o->next = top;
	o->source = source;
	o->type = Object_type_info;
	o->name = name;
	return o;
}

#define Object_create(NAME_, TOP_)   Object_create_dbg((NAME_), (TOP_), SOURCE_HERE)

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
			(byte_span)SPAN_INIT((byte*)(ARR_), sizeof(ARR_))

#define Bytes_init_var(VAR_)   \
			(byte_span)SPAN_INIT((byte*)&(VAR_), sizeof(VAR_))

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
	byte *b = data;

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


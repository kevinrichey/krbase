#include "krbase.h"
#include "test.h"

TEST_CASE(this_test_always_fails)
{
	//TEST(false);
}

TEST_CASE(helper_macros)
{
#define TEST_CONCAT_LONG_TOKEN  true
	TEST(CONCAT(TEST_CONCAT_, LONG_TOKEN));

	const char str[] = {"This is a string"};
	TEST(!strcmp(STRINGIFY(This is a string), str));

#define EXPAND_THIS_TOKEN   XYZZY

	const char token_not_expanded[] = {"EXPAND_THIS_TOKEN"};
	TEST(!strcmp(STRINGIFY(EXPAND_THIS_TOKEN), token_not_expanded));

	const char token_expanded[] = {"XYZZY"};
	TEST(!strcmp(STRINGIFY_EXPAND(EXPAND_THIS_TOKEN), token_expanded));

	int a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	TEST(ARRAY_LENGTH(a) == 10);

	TEST(ARRAY_LENGTH("xyzzy") == 6);
}

TEST_CASE(capture_debug_context_info)
{
	struct SourceLocation dbi = CURRENT_LOCATION;

	TEST(!strcmp(dbi.file_name, "test_krbase.c"));
	TEST(dbi.line_num == (__LINE__-3));
}

TEST_CASE(int_functions)
{
	TEST(int_min(0, 10) ==  0);
	TEST(int_max(0, 10) == 10);

	TEST(int_clamp( 5, 1, 9) == 5);
	TEST(int_clamp(-4, 1, 9) == 1);
	TEST(int_clamp(99, 1, 9) == 9);

	TEST( int_in_range( 55, 1, 99));
	TEST(!int_in_range(  0, 1, 99));
	TEST(!int_in_range(100, 1, 99));
}

TEST_CASE(float_functions)
{
	TEST(float_min(-0.125, 3.14) == -0.125);
	TEST(float_max(0, 10) == 10);

	TEST(float_clamp( 1.5, 1.0, 2.0) == 1.5);
	TEST(float_clamp(-0.5, 1.0, 2.0) == 1.0);
	TEST(float_clamp( 3.0, 1.0, 2.0) == 2.0);
	// Test extreme ranges of float

	TEST( float_in_range( 55, 1, 99));
	TEST(!float_in_range(  0, 1, 99));
	TEST(!float_in_range(100, 1, 99));
}

TEST_CASE(ptr_and_returns_pointer_or_alternate)
{
	char ok_p[] = "hello";
	char alt_p[] = "world";
	TEST(ptr_and(ok_p, alt_p) == ok_p);
	TEST(ptr_and(NULL, alt_p) == alt_p);
	TEST(ptr_and(NULL, NULL)  == NULL);

	const char *const_ok_ptr = "const hello";
	const char *const_alt_ptr = "const hello";
	TEST(const_ptr_and(const_ok_ptr, const_alt_ptr) == const_ok_ptr);
	TEST(const_ptr_and(NULL, const_alt_ptr) == const_alt_ptr);
}

TEST_CASE(lerp_interpolation)
{
	TEST(lerp(1.0, 6.0, 0.5) == 3.5);
	TEST(lerp(1.0, 6.0, 0.0) == 1.0);
	TEST(lerp(1.0, 6.0, 1.0) == 6.0);
}

TEST_CASE(float_epsilon_compare)
{
	double a = 9.2;
	double b = 0.000001;

	TEST(!float_equals(a, b, 0.0));
	TEST( float_equals(a, 9.3-0.1, 0.00001));
}


#define USING_KR_NAMESPACE
#include "test.h"
#include "krstring.h"

TEST_CASE(null_string_is_empty)
{
	string *s = NULL;

	TEST(string_length(s) == 0);
	TEST(string_size(s) == 0);
	TEST(string_is_empty(s));
	TEST(string_is_full(s));

	TEST(string_equals(s, NULL));
	TEST(!string_equals(s, ""));
	TEST(!string_equals(s, "xyzzy"));

	string_dispose(s);
}

TEST_CASE(empty_strings)
{
	string *s = string_create("");

	TEST(string_length(s) == 0);
	TEST(string_size(s) == 8);
	TEST(!string_is_full(s));
	TEST(string_is_empty(s));

	TEST(string_equals(s, ""));
	TEST(!string_equals(s, NULL));
	TEST(!string_equals(s, "xyzzy"));

	string_dispose(s);
}

TEST_CASE(string_lifecycle)
{
	char hello[] = "Hello, world.";
	string *s = string_create(hello);

	TEST(!string_is_empty(s));
	TEST(string_length(s) == 13);
	TEST(string_equals(s, hello));
	TEST(!string_equals(s, "xyzzy"));
	TEST(!string_equals(s, NULL));

	string_dispose(s);
}

TEST_CASE(create_formatted_string)
{
	const char *answer = "the answer";
	string *s = string_format("%s to %d/%d is %.5f", answer, 2, 3, 2.0/3.0);

	TEST(string_equals(s, "the answer to 2/3 is 0.66667"));

	string_dispose(s);
}

TEST_CASE(grow_string_from_null)
{
	string *s = NULL;

	s = string_reserve(s, 0);
	TEST(string_size(s) == 8);

	s = string_reserve(s, 0);
	TEST(string_size(s) == 16);

	s = string_reserve(s, 20);
	TEST(string_size(s) == 20);

	s = string_reserve(s, 18);
	TEST(string_size(s) == 20);

	string_dispose(s);
}


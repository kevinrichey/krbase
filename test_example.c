#include "klib.h"

TEST_CASE(pass_example_0)
{
	UNUSED(test_counter);
	TEST(true);
}

TEST_CASE(fail_example_1)
{
	UNUSED(test_counter);
	int x = 0;

	TEST(x == 0);
	TEST(x == 1);
}


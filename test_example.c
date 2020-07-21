#include "klib.h"

TEST_CASE(pass_example_0)
{
	test(true);
}

TEST_CASE(fail_example_1)
{
	int x = 0;

	test(x == 0);
	test(x == 1);
}


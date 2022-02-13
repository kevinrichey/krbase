#include "krclib.h"

TEST_CASE(pass_example_0)
{
	TEST(true);
}

TEST_CASE(fail_example_1)
{
	int x = 0;

	TEST(x == 0);
	//TEST(x == 1);
}


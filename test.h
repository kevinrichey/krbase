#ifndef KR_TEST_H_INCLUDED
#define KR_TEST_H_INCLUDED

#include "krbase.h"

#define TEST_CASE(TEST_NAME_)  void TestCase_##TEST_NAME_(void)
#define TEST(CONDITION_)       test_assert((CONDITION_), SRC_HERE, "'" #CONDITION_ "'")
void test_assert(bool condition, SourceLine source, const char *msg);

#endif

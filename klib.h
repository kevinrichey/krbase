#include <stdbool.h>
#include <stdio.h>

/** 
 * # klib - Core Library
 */

/** --------------------------------------------------
 *
 * ## Code Utility 
 */

#define UNUSED(var)  (void)(var)
/**
 * Suppress "unused parameter" warnings for variable *var*.
 */

/** ### STANDARD_ENUM_VALUES(EnumName_)
 * Add useful enum values, prefixed with *EnumName_*:
 *
 * - *EnumName_*First - equals 0.
 * - *EnumName_*Last - equals last enum value.
 * - *EnumName_*End - equals last enum value + 1.
 */
#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_End,  \
  EnumName_##_Last  = EnumName_##_End - 1, \
  EnumName_##_First = 0,

/**
 * ## Debugging & Error Checking
 */

#define ERROR_CODE_X_TABLE \
  X(OK) \
  X(Failure) \
  X(Error) 

#define X(EnumName_)  ErrorCode_##EnumName_,
typedef enum {
    ERROR_CODE_X_TABLE
    STANDARD_ENUM_VALUES(ErrorCode)
} ErrorCode;
#undef X

const char *ErrorCode_String(ErrorCode error);
/** 
 * Return string rep of *error* code.
 */

#define DEBUG_CATEGORY_X_TABLE \
  X(Test) 

#define X(EnumName_)  DebugCategory_##EnumName_,
typedef enum {
	DEBUG_CATEGORY_X_TABLE 
    STANDARD_ENUM_VALUES(DebugCategory)
} DebugCategory;
#undef X

const char *DebugCategory_String(DebugCategory category);
/** 
 * Return string rep of *category*.
 */

void error_fprint(FILE *out, const char *file, int line, DebugCategory category, ErrorCode errcode, const char *msg, const char *func);
/** 
 * Print error info to file stream *out*. 
 */

/**--------------------------------------------------
 * ## Unit Testing
 */

typedef struct TestCounter_struct {
	int test_count, failure_count;
} TestCounter;

typedef void (*TestCase_fn)(TestCounter*);

void test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg, const char *func);

#define TEST_CASE(test_name_)  void TestCase_##test_name_(TestCounter *test_counter)
/** 
 * Define new test case *test_name_*. 
 */

#define test(condition_)  test_assert(test_counter, (condition_), __FILE__, __LINE__, #condition_, __func__)
/** 
 * Test that the *condition_* is true. If it is false, the test fails.
 */



#include <stdbool.h>
#include <stdio.h>

// Suppress "unused parameter" warnings.
#define UNUSED(x)  (void)(x)

// Declare standard enum values.
//  - _End: last enum value +1
//  - _Last:  equal to value of last enum 
//  - _First:  equal to 0
#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_End,  \
  EnumName_##_Last  = EnumName_##_End - 1, \
  EnumName_##_First = 0,

//--------------------------------------------------
// Debugging & Error Checking

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


#define DEBUG_CATEGORY_X_TABLE \
  X(Test) 

#define X(EnumName_)  DebugCategory_##EnumName_,
typedef enum {
	DEBUG_CATEGORY_X_TABLE 
    STANDARD_ENUM_VALUES(DebugCategory)
} DebugCategory;
#undef X

const char *DebugCategory_String(DebugCategory category);

void error_fprint(FILE *out, const char *file, int line, DebugCategory category, ErrorCode errcode, const char *msg, const char *func);

//--------------------------------------------------
// Unit Testing

typedef struct TestCounter_struct {
	int test_count, failure_count;
} TestCounter;

typedef void (*TestCase_fn)(TestCounter*);

void test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg, const char * func);

#define TEST_CASE(test_name_)  void TestCase_##test_name_(TestCounter *test_counter)
#define test(condition_)  test_assert(test_counter, (condition_), __FILE__, __LINE__, #condition_, __func__)

